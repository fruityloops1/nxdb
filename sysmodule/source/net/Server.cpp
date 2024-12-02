#include "Server.h"
#include "LogServer.h"
#include "imgui.h"
#include "nxdb/ImGuiLoop.h"
#include "switch/runtime/diag.h"
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace nxdb {

    void Client::sendPacketImpl(PacketType type, const void* buffer, size_t size) {
        struct {
            uint32_t size;
            uint32_t type;
        } hdr { uint32_t(size), uint32_t(type) };

        int ret = write(mFd, &hdr, sizeof(hdr));
        if (ret == -1) {
            nxdb::log("write returned %d\n", ret);
        }
        ret = send(mFd, buffer, size, 0);
        if (ret == -1) {
            nxdb::log("send returned %d\n", ret);
        }
    }

    Server::Server(uint16_t port)
        : mPort(port) {
    }

    int Server::start() {
        if (mFd != -1)
            close(mFd);

        mFd = socket(AF_INET, SOCK_STREAM, 0);
        if (mFd < 0) {
            return 1;
        }

        int opt = true;

        if (setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            nxdb::log("setsockopt() failed");
            return 3;
        }

        if (setsockopt(mFd, IPPROTO_TCP, 1 /* TCP_NODELAY (?) */, &opt, sizeof(opt)) < 0) {
            nxdb::log("setsockopt() failed");
            return 3;
        }

        memset(&mServAddr, 0, sizeof(mServAddr));

        mServAddr.sin_family = AF_INET;
        mServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        mServAddr.sin_port = htons(mPort);

        if (bind(mFd, (struct sockaddr*)&mServAddr, sizeof(mServAddr))) {
            nxdb::log("bind() failed");
            return 2;
        }

        if (listen(mFd, SOMAXCONN) < 0) {
            nxdb::log("listen() failed");
            return 4;
        }

        nxdb::log("Created server at %s:%d", inet_ntoa(mServAddr.sin_addr), mPort);

        return 0;
    }

    void Client::handlePacket(PacketType type, void* data, size_t size) {
        switch (type) {
        case NoPacket:
            break;
        case Hello:
            queueRerender();
            break;
        case Alive:
            nxdb::log("Alive");
            mLastAlive = std::chrono::high_resolution_clock::now();
            break;
        case UpdateDisplaySize: {
            auto& displaySize = ImGui::GetIO().DisplaySize;
            displaySize = *reinterpret_cast<ImVec2*>(data);
            nxdb::log("New display size: %.2ff:%.2f", displaySize.x, displaySize.y);
            queueRerender();
            break;
        }
        case UpdateMouse: {
            ImGuiIO& io = ImGui::GetIO();
            MouseEvent& packet = *reinterpret_cast<MouseEvent*>(data);

            io.AddMousePosEvent(packet.pos.x, packet.pos.y);

            if (packet.wheelX || packet.wheelY)
                io.AddMouseWheelEvent(packet.wheelX, packet.wheelY);
            if (packet.button != -1)
                io.AddMouseButtonEvent(packet.button, packet.pressed);

            queueRerender();

            break;
        }
        case UpdateKey: {
            ImGuiIO& io = ImGui::GetIO();
            KeyEvent& packet = *reinterpret_cast<KeyEvent*>(data);

            io.AddKeyEvent(packet.key, packet.pressed);
            io.AddKeyEvent(ImGuiMod_Ctrl, packet.mod.ctrl);
            io.AddKeyEvent(ImGuiMod_Shift, packet.mod.shift);
            io.AddKeyEvent(ImGuiMod_Alt, packet.mod.alt);
            io.AddKeyEvent(ImGuiMod_Super, packet.mod.super);
            break;
        }
        case UpdateChar: {
            ImGuiIO& io = ImGui::GetIO();
            unsigned int c = *reinterpret_cast<unsigned int*>(data);

            io.AddInputCharacter(c);
            break;
        }
        case SetClipboardText: {
            setCurClipboardString(reinterpret_cast<const char*>(data));
            break;
        }
        default:
            break;
        }
    }

    void Client::sendFontTexture() {
        u8* pixels;
        int width;
        int height;

        struct Hdr {
            int width;
            int height;
            u8 data[];
        };

        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        size_t pixelsSize = width * height * sizeof(u32);
        Hdr* header = (Hdr*)malloc(sizeof(Hdr) + pixelsSize);

        header->width = width;
        header->height = height;
        memcpy(header->data, pixels, pixelsSize);

        sendPacketImpl(PacketType::FontTexture, header, sizeof(Hdr) + pixelsSize);

        free(header);

        mHasSentFontTexture = true;
    }

    constexpr int packetHeaderSize = 8;
    bool Client::handleEvent() {
        if (mNextPacketType == NoPacket && mNextPacketSize == 0 && mBufferSize >= packetHeaderSize) {
            mNextPacketSize = *reinterpret_cast<uint32_t*>(mBuffer);
            mNextPacketType = *reinterpret_cast<PacketType*>(&mBuffer[sizeof(uint32_t)]);
        }

        size_t wholeSize = mNextPacketSize + packetHeaderSize;
        if (mBufferSize >= wholeSize) {
            void* data = mBuffer + packetHeaderSize;
            size_t size = mNextPacketSize;
            handlePacket(mNextPacketType, data, size);

            memmove(mBuffer, mBuffer + wholeSize, mBufferSize - wholeSize);
            mBufferSize -= wholeSize;

            mNextPacketSize = 0;
            mNextPacketType = NoPacket;
        }

        ssize_t rem = recv(mFd, mBuffer + mBufferSize, sizeof(mBuffer) - mBufferSize, 0);
        if (rem == 0) {
            close(mFd);
            mFd = 0;

            nxdb::log("Client %zu disconnected", mUuid);
            return true;
        } else if (rem == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                ;
            else
                nxdb::log("recv returned %zd %d", rem, errno);
        } else if (rem > 0) {
            mBufferSize += rem;
        }
        return false;
    }

    void Server::run() {
        while (true) {
            int maxfd = mFd;

            FD_ZERO(&mSet);

            FD_SET(mFd, &mSet);

            for (auto& client : mClients) {
                if (client.mFd > 0)
                    FD_SET(client.mFd, &mSet);
                if (client.mFd > maxfd)
                    maxfd = client.mFd;
            }

            EINTR;

            timeval a = { 0, 0 /* 15ms */ };
            int activity = select(maxfd + 1, &mSet, nullptr, nullptr, &a);

            if (activity < 0) {
                nxdb::log("select error %d %d", activity, errno);

                diagAbortWithResult(0x493423);
            }

            if (FD_ISSET(mFd, &mSet)) {
                Client client(this);

                client.mFd = accept(mFd, (struct sockaddr*)&client.mAddr, &client.mCliAddrLen);
                if (client.mFd < 0) {
                    nxdb::log("accept error ret: %d errno: %d Fd: %d", client.mFd, errno, mFd);

                    diagAbortWithResult(0x493423);
                }

                nxdb::log("New client %s:%d", inet_ntoa(client.mAddr.sin_addr), client.mAddr.sin_port);

                mClients.push_back(client);
            }

            {
                int i = 0;
                for (auto& client : mClients) {
                    if (FD_ISSET(client.mFd, &mSet) or true) {
                        auto end = std::chrono::high_resolution_clock::now();
                        auto duration = end - client.mLastAlive;
                        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

                        if (client.handleEvent() || seconds.count() > 15) // disconnect
                        {
                            nxdb::log("Client %zu timed out", client.mUuid);
                            mClients.erase(mClients.begin() + i);
                        }
                    }
                    i++;
                }
            }
        }
    }

} // namespace mog
