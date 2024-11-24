#include "Server.h"
#include "Client.h"
#include "al/Nerve/Nerve.h"
#include "al/Nerve/NerveUtil.h"
#include "enet/enet.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/Hash.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>

namespace pe {
    namespace enet {

        namespace {
            NERVE_DEF(Server, Stall)
            NERVE_DEF(Server, Startup)
            NERVE_DEF(Server, Service)
        }

        Server::Server(ENetAddress address, PacketHandler<Client>& handler, const ENetCallbacks& callbacks)
            : al::NerveExecutor("Server")
            , mCallbacks(callbacks)
            , mAddress(address)
            , mPacketHandler(handler) {
            initNerve(&nrvServerStall);
            mSyncClockStartTimestamp = std::chrono::high_resolution_clock::now();
        }

        void Server::threadFunc() {
            while (!mIsDead)
                updateNerve();
        }

        void Server::fail(const char* msg) {
            nxdb::log("fail: %s\n");
            kill(1);
        }

        void Server::sendPacketToAll(IPacket* packet, bool reliable) {
            const size_t bufSize = packet->calcSize();

            u8* buf = new u8[bufSize];
            const size_t packetSize = packet->build(buf);

            u32 flags = ENET_PACKET_FLAG_NO_ALLOCATE;
            if (reliable)
                flags |= ENET_PACKET_FLAG_RELIABLE;

            mEnetMutex.lock();
            ENetPacket* pak = enet_packet_create(buf, packetSize, flags);
            ChannelType channel = identifyType(packet);
            for (int i = 0; i < mServer->peerCount; i++)
                mPacketHandler.increaseSentPacketCount(channel);

            enet_host_broadcast(mServer, (int)channel, pak);
            mEnetMutex.unlock();
            delete[] buf;
        }

        void Server::start() {
            al::setNerve(this, &nrvServerStartup);
            mServerThread = std::thread(&Server::threadFunc, this);
        }

        void Server::exeStall() { nxdb::log("Server is Stalling"); }

        void Server::exeStartup() {
            if (al::isFirstStep(this)) {

                mEnetMutex.lock();
                if (enet_initialize_with_callbacks(ENET_VERSION, &mCallbacks) != 0) {
                    fail("An error occurred while initializing ENet.");
                    return;
                }

                mServer = enet_host_create(&mAddress, 12, pe::enet::sChannels.channelCount, 0, 0);
                if (mServer == nullptr) {
                    fail("An error occurred while creating ENet server host.");
                    return;
                }

                mEnetMutex.unlock();
                al::setNerve(this, &nrvServerService);
            }
        }

        void Server::exeService() {
            if (al::isFirstStep(this)) {
                nxdb::log("Server initialized");
            }

            if (mServer == nullptr) {
                al::setNerve(this, &nrvServerStall);
                return;
            }

            mEnetMutex.lock();

            ENetEvent event;
            while (enet_host_service(mServer, &event, 1000) > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    ENetAddress addr = event.peer->address;
                    size_t hash = calcEnetAddressHashCode(addr);

                    if (mClients.contains(hash)) {
                        nxdb::log("Warning: ghost client removed");
                        mClients.erase(hash);
                    }

                    mClients[hash] = Client(this, event.peer);
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    ENetAddress addr = event.peer->address;
                    size_t hash = calcEnetAddressHashCode(addr);

                    if (!mClients.contains(hash)) {
                        nxdb::log("Warning: packet from invalid peer. disconnecting");
                        enet_peer_disconnect(event.peer, 0);
                        break;
                    }

                    Client& client = mClients[hash];
                    ChannelType packetType = (ChannelType)event.channelID;
                    if (packetType != ChannelType::ToS_Hello && !client.hasGreeted()) {
                        nxdb::log("Warning: client %zx tried to send packet without greeting", calcEnetAddressHashCode(addr));
                        enet_packet_destroy(event.packet);
                        break;
                    }

                    mPacketHandler.handlePacket(packetType, event.packet->data, event.packet->dataLength, &client);
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    ENetAddress addr = event.peer->address;
                    size_t hash = calcEnetAddressHashCode(addr);

                    nxdb::log("Client %zx disconnected", calcEnetAddressHashCode(addr));

                    if (mClients.contains(hash)) {
                        {
                            Client& client = mClients[hash];
                            client.disconnect();
                        }
                        mClients.erase(hash);
                    }
                    break;
                }
                case ENET_EVENT_TYPE_NONE:
                default:
                    break;
                }
            }

            mEnetMutex.unlock();
        }

    } // namespace enet
} // namespace pe
