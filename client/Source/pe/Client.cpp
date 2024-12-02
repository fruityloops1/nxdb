#include "pe/Client.h"
#include "pe/NetworkInput.h"
#include "pe/Render.h"
#include <GLFW/glfw3.h>
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace nxdb {

    static Client* sInstance = nullptr;
    Client*& Client::instance() { return sInstance; }

    Client::Client()
        : mPort(34039) {
        sInstance = this;
        start();
    }

    int Client::start() {
        if (mFd != -1) {
            printf("m_Fd already exists, closing\n");
            close(mFd);
        }

        mFd = socket(AF_INET, SOCK_STREAM, 0);
        if (mFd < 0) {
            printf("Socket creation failed because %d\n", errno);
            return 1;
        }

        memset(&mServAddr, 0, sizeof(mServAddr));
        memset(&mCliAddr, 0, sizeof(mCliAddr));

        mServAddr.sin_family = AF_INET;
        mServAddr.sin_addr.s_addr = inet_addr("192.168.213.202");
        mServAddr.sin_port = htons(mPort);

        if (connect(mFd, (struct sockaddr*)&mServAddr, sizeof(mServAddr))) {
            printf("Socket connection failed because %d\n", errno);
            return 2;
        }

        mHasSentHello = false;

        mState = State_Connecting;

        return 0;
    }

    void Client::handlePacket(PacketType type, void* data, size_t size) {
        switch (type) {
        case FontTexture: {
            struct {
                int width;
                int height;
                u8 data[];
            }* header((decltype(header))data);

            printf("width %d height %d\n", header->width, header->height);

            pe::loadFontTextureRGBA32(header->data, header->width, header->height);
            break;
        }
        case DrawData: {
            pe::handlePackedDrawData(data, size);
            break;
        }
        case SetClipboardText: {
            glfwSetClipboardString(pe::getGlfwWindow(), reinterpret_cast<const char*>(data));
            break;
        }
        default:
            break;
        }
    }

    void Client::sendPacketImpl(PacketType type, const void* buffer, size_t size) {
        struct {
            u32 size;
            u32 type;
        } hdr { u32(size), u32(type) };

        int ret = write(mFd, &hdr, sizeof(hdr));
        if (ret == -1)
            mState = State(State_NoSocket);

        size_t sizeLeft
            = size;
        while (sizeLeft > 0) {
            size_t sendSize = 0x3000;
            if (sendSize > sizeLeft)
                sendSize = sizeLeft;
            size_t offset = size - sizeLeft;

            ret = send(mFd, ((u8*)buffer) + offset, sendSize, 0);
            if (ret == -1)
                mState = State(State_NoSocket);
            sizeLeft -= sendSize;
        }
    }

    using namespace std::chrono_literals;
    std::chrono::high_resolution_clock::time_point sLastAliveTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point sLastInputUpdateTime = std::chrono::high_resolution_clock::now();

    void Client::run() {
        fd_set set;
        memset(mBuffer, 0, sizeof(mBuffer));

        while (!mDead) {
            if (mState == State_NoSocket) {
                printf("No socket, retrying and sleeping\n");
                mHasSentHello = false;
                start();
                continue;
            }

            FD_ZERO(&set);
            FD_SET(mFd, &set);

            timeval time = { 0, 8000 /* 8ms */ };
            select(mFd + 1, &set, nullptr, nullptr, &time);

            if (!mHasSentHello) {
                sendPacket(PacketType::Hello, 0);
                mHasSentHello = true;
            }

            std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

            {
                auto elapsed = now - sLastAliveTime;
                if (elapsed > 5s) {
                    sendPacket((PacketType)PacketType::Alive, 0);
                    sLastAliveTime = now;
                    printf("Alive\n");
                }
            }

            {
                auto elapsed = now - sLastInputUpdateTime;
                if (elapsed > 15ms) {
                    pe::updateNetworkInput();
                    sLastInputUpdateTime = now;
                }
            }

            /*printf("m_Buffer %d %d %d %d %d %d %d",
                m_Buffer[0],
                m_Buffer[1],
                m_Buffer[2],
                m_Buffer[3],
                m_Buffer[4],
                m_Buffer[5],
                m_Buffer[6],
                m_Buffer[7]);*/
            // printf("m_NextPacketType %d m_NextPackeSize %zd m_BufferSize %zu", m_NextPacketType, m_NextPacketSize, m_BufferSize);
            if (mNextPacketType == NoPacket && mNextPacketSize == 0 && mBufferSize >= 8) {
                mNextPacketSize = *reinterpret_cast<u32*>(mBuffer);
                mNextPacketType = *reinterpret_cast<PacketType*>(&mBuffer[sizeof(u32)]);
            }

            size_t wholeSize = mNextPacketSize + 8;
            if (mBufferSize >= wholeSize) {
                void* data = mBuffer + 8;
                size_t size = mNextPacketSize;
                // printf("b m_NextPacketType %d m_NextPackeSize %zd m_BufferSize %zu", m_NextPacketType, m_NextPacketSize, m_BufferSize);
                handlePacket(mNextPacketType, data, size);
                // printf("a m_NextPacketType %d m_NextPackeSize %zd m_BufferSize %zu", m_NextPacketType, m_NextPacketSize, m_BufferSize);

                size_t toMove = mBufferSize - wholeSize;
                if (toMove > sizeof(mBuffer)) {

                    mState = State_NoSocket;
                    mBufferSize = 0;
                    mNextPacketSize = 0;
                    mNextPacketType = NoPacket;
                    memset(mBuffer, 0, sizeof(mBuffer));
                    continue;
                }
                memmove(mBuffer, mBuffer + wholeSize, toMove);
                mBufferSize -= wholeSize;

                mNextPacketSize = 0;
                mNextPacketType = NoPacket;
            }

            ssize_t rem = recv(mFd, mBuffer + mBufferSize, sizeof(mBuffer) - mBufferSize, MSG_DONTWAIT);
            if (rem == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    ;
                else
                    mState = State_NoSocket;
            } else if (rem > 0) {
                mBufferSize += rem;
            }
        }
    }

} // namespace nxdb
