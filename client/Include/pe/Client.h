#pragma once

#include "../../../common.h"
#include "types.h"
#include <netinet/in.h>

namespace nxdb {

    class Client {
        u16 mPort;
        int mFd = -1;
        struct sockaddr_in mServAddr, mCliAddr;
        u32 mCliAddrLen = sizeof(mCliAddr);
        bool mConnected = false;
        enum State
        {
            State_NoSocket,
            State_Connecting,
            State_Connected,
        } mState
            = State_NoSocket;
        bool mThreadStarted = false;
        u8 mBuffer[1_MB];
        uintptr_t mBufferSize = 0;
        ssize_t mNextPacketSize = 0;
        PacketType mNextPacketType
            = NoPacket;
        bool mHasSentHello = false;
        bool mDead = false;

    public:
        Client();

        static Client*& instance();
        bool isConnected() const { return mConnected; }
        int start();
        void run();

        void handlePacket(PacketType type, void* data, size_t size);

        void sendPacketImpl(PacketType type, const void* buffer, size_t size);

        template <typename T>
        void sendPacket(PacketType type, const T& value) {
            sendPacketImpl(type, (void*)&value, sizeof(T));
        }

        void kill() { mDead = true; }
    };

} // namespace nxdb
