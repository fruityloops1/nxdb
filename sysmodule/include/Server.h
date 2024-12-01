#pragma once

#include "../../common.h"
#include "types.h"
#include <chrono>
#include <netinet/in.h>

namespace nxdb {

    class Server;

    struct Client {
        struct sockaddr_in mAddr;
        uint32_t mCliAddrLen = sizeof(mAddr);
        int mFd = -1;

        uint8_t mBuffer[size_t(4_KB)];
        uintptr_t mBufferSize = 0;
        ssize_t mNextPacketSize = 0;
        PacketType mNextPacketType = NoPacket;
        u64 mUuid;
        Server* mServer = nullptr;
        std::chrono::time_point<std::chrono::high_resolution_clock> mLastAlive = std::chrono::high_resolution_clock::now();

        bool mHasSentFontTexture = false;

        Client(Server* server)
            : mServer(server) { }

        bool handleEvent();
        void handlePacket(PacketType type, void* data, size_t size);

        void sendPacketImpl(PacketType type, const void* buffer, size_t size);
        template <typename T>
        void sendPacket(PacketType type, const T& value) {
            sendPacketImpl(type, (void*)&value, sizeof(T));
        }

        void sendFontTexture();
        bool hasSentFontTexture() const { return mHasSentFontTexture; }
    };

    class Server {
        uint16_t mPort = 34039;
        int mFd = -1;
        struct sockaddr_in mServAddr;
        std::vector<Client> mClients;
        fd_set mSet;

    public:
        Server(uint16_t port = 34039);

        int start();
        void run();

        void handlePacket(PacketType type);

        size_t getNumClients() const { return mClients.size(); }

        template <typename Func>
        void iterateClients(Func func) {
            for (Client& client : mClients)
                func(client);
        }
    };

} // namespace nxdb
