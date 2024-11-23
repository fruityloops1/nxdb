#pragma once

#include "Client.h"
#include "al/Nerve/NerveExecutor.h"
#include "enet/enet.h"
#include "pe/Enet/PacketHandler.h"
#include <chrono>
#include <mutex>
#include <thread>
#include <type_traits>
#include <unordered_map>

namespace pe {
    namespace enet {
        class Client;

        class Server : public al::NerveExecutor {
            bool mIsDead = false;
            std::thread mServerThread;
            std::chrono::high_resolution_clock::time_point mSyncClockStartTimestamp;
            const ENetCallbacks mCallbacks;
            ENetAddress mAddress {};
            ENetHost* mServer = nullptr;
            int mExitCode = 0;
            PacketHandler<Client>& mPacketHandler;
            std::unordered_map<size_t /* hash code of ENetAddress */, Client> mClients;
            std::recursive_mutex mEnetMutex;

            void threadFunc();
            void fail(const char* msg);

        public:
            Server(ENetAddress address, PacketHandler<Client>& handler, const ENetCallbacks& callbacks);

            void exeStall();
            void exeStartup();
            void exeService();

            void sendPacketToAll(IPacket* packet, bool reliable = true);

            void start();
            int join() {
                if (mServerThread.joinable())
                    mServerThread.join();
                return mExitCode;
            }

            void kill(int exitCode) {
                mIsDead = true;
                mExitCode = exitCode;
            }

            bool isAlive() const { return !mIsDead; }

            auto getSyncClockStartTimestamp() { return mSyncClockStartTimestamp; }
            void flush() { enet_host_flush(mServer); }

            static constexpr int sSyncClockInterval = 12000;
            friend struct Handlers;
        };

    } // namespace enet
} // namespace pe
