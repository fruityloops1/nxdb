#pragma once

#include "al/Nerve/NerveExecutor.h"
#include "enet/enet.h"
#include "pe/Enet/Types.h"
#include <chrono>
#include <mutex>
#include <thread>
#include "pe/Util.h"
#include "pe/Enet/Request.h"

namespace pe {
    namespace enet {
        template <typename ClientType>
        class PacketHandler;
        class IPacket;

        class NetClient : public al::NerveExecutor {
            bool mIsDead = false;
            ENetHost* mClient = nullptr;
            std::recursive_mutex mClientCS;
            std::thread mThread;
            ENetPeer* mServerPeer = nullptr;
            const char* mIp = nullptr;
            u16 mPort = 0;
            PacketHandler<void>* mPacketHandler = nullptr;
            s64 mSyncClockStartTick = 0;
            int mPacketsHandledInCurrentTick = 0;
            std::chrono::high_resolution_clock::time_point mClock;

            void threadFunc();

        public:
            NetClient(PacketHandler<void>* handler);
            void kill();

            void connect(const char* ip, u16 port);
            bool isConnected() const;
            void disconnect();

            void sendPacket(const IPacket* packet, bool reliable = true);

            template <typename Request, typename L>
            void makeRequest(const typename Request::RequestType& data, L callback)
            {
                typename Request::RequestPacketType packet;
                packet.data = data;
                packet.requestId = pe::getRandom() >> 32;

                RequestMgr::instance().registerEntry(packet.requestId, new RequestFunctor<typename Request::ResponseType, decltype(callback)>(callback));

                sendPacket(&packet, true);
            }

            void flush() {
                if (mClient)
                    enet_host_flush(mClient);
            }

            void join() {
                if (mThread.joinable())
                    mThread.join();
            }

            PacketHandler<void>* getPacketHandler() const { return mPacketHandler; }
            ENetPeer* getPeer() const { return mServerPeer; }
            void printStatusMsg(char* buf);
            void printStatusMsgPretty(char* buf, float secondsElapsed);
            void updateSyncClockStartTick(u64 microseconds);
            s64 getSyncClockTicks() const;
            double getSyncClock() const;

            void exeStall();
            void exeConnect();
            void exeService();

            bool checkStall();

            constexpr static int sMaxPacketsPerStep = 32;
            constexpr static int sStepInterval = 48;
        };

        void setNetClient(NetClient* client);
        NetClient* getNetClient();
        inline s64 getSyncClockTicks() { return getNetClient()->getSyncClockTicks(); }
        inline double getSyncClock() { return getNetClient()->getSyncClock(); }
        inline u64 getSyncClockFrames() { return getSyncClock() * 60.0; }

    } // namespace enet
} // namespace pe
