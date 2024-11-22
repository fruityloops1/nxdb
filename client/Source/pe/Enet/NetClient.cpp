#include "pe/Enet/NetClient.h"
#include "al/Nerve/Nerve.h"
#include "al/Nerve/NerveUtil.h"
#include "enet/enet.h"
#include "enet/list.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/Hash.h"
#include "pe/Enet/PacketHandler.h"
#include <chrono>
#include <cmath>
#include <thread>

namespace pe {
    namespace enet {

        namespace {

            NERVE_DEF(NetClient, Stall);
            NERVE_DEF(NetClient, Connect);
            NERVE_DEF(NetClient, Service);

        } // namespace

        NetClient::NetClient(PacketHandler<void>* handler)
            : al::NerveExecutor("NetClient")
            , mPacketHandler(handler)
            , mClock(std::chrono::high_resolution_clock::now()) {
            initNerve(&nrvNetClientStall);
            mThread = std::thread(&NetClient::threadFunc, this);
        }

        void NetClient::threadFunc() {
            const auto wait = std::chrono::milliseconds(sStepInterval);

            while (!mIsDead) {
                updateNerve();
                std::this_thread::sleep_for(wait);
            }
        }

        void NetClient::connect(const char* ip, u16 port) {
            mIp = ip;
            mPort = port;

            al::setNerve(this, &nrvNetClientConnect);
        }

        bool NetClient::isConnected() const { return al::isNerve(this, &nrvNetClientService); }

        void NetClient::disconnect() {
            mClientCS.lock();

            if (mServerPeer) {
                enet_peer_disconnect_now(mServerPeer, 0);
                mServerPeer = nullptr;
            }

            if (mClient) {
                enet_host_flush(mClient);
                enet_host_destroy(mClient);
                mClient = nullptr;
            }

            al::setNerve(this, &nrvNetClientStall);

            mClientCS.unlock();
        }

        void NetClient::sendPacket(const IPacket* packet, bool reliable) {
            if (mServerPeer == nullptr)
                return;

            if (mServerPeer->state != ENET_PEER_STATE_CONNECTED)
                return;
            size_t bufSize = packet->calcBufSize();
            void* buf = buddyMalloc(bufSize);
            packet->build(buf);

            // ENET_PACKET_FLAG_NO_ALLOCATE is FUCKING broken dont use it
            mClientCS.lock();

            ENetPacket* pak = enet_packet_create(buf, packet->calcSize(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
            ChannelType type = identifyType(packet);
            enet_peer_send(mServerPeer, (int)type, pak);
            mPacketHandler->increaseSentPacketCount(type);

            // enet_host_flush(mClient); // try this later

            mClientCS.unlock();
            buddyFree(buf);
        }

        void NetClient::printStatusMsg(char* buf) {
            if (al::isNerve(this, &nrvNetClientStall))
                snprintf(buf, 256, "Stalling");
            else if (al::isNerve(this, &nrvNetClientConnect))
                snprintf(buf, 256, "Connecting to %s:%u", mIp, mPort);
            else if (al::isNerve(this, &nrvNetClientService))
                snprintf(buf, 256, "Connected to %s:%u - Ping: %dms (avg %dms +- ~%dms) - Clock: %f %lu - Step: %d, Events handled in %dms: %d",
                    mIp, mPort, mServerPeer->lastRoundTripTime, mServerPeer->roundTripTime, mServerPeer->roundTripTimeVariance, getSyncClock(), getSyncClockFrames(), al::getNerveStep(this), sStepInterval, mPacketsHandledInCurrentTick);
        }

        void NetClient::printStatusMsgPretty(char* buf, float secondsElapsed) {
            if (al::isNerve(this, &nrvNetClientConnect))
                snprintf(buf, 256, "Connecting to %s:%u... %ds - Press B to cancel", mIp, mPort, int(secondsElapsed));
            else if (al::isNerve(this, &nrvNetClientService))
                snprintf(buf, 256, "Successfully connected to %s:%u - Ping: %dms ",
                    mIp, mPort, mServerPeer->lastRoundTripTime);
        }

        void NetClient::updateSyncClockStartTick(u64 microseconds) {
            u64 ticks = microseconds;
            mSyncClockStartTick = (std::chrono::high_resolution_clock::now() - mClock).count() - ticks;
        }

        s64 NetClient::getSyncClockTicks() const {
            return (std::chrono::high_resolution_clock::now() - mClock).count() - mSyncClockStartTick;
        }

        double NetClient::getSyncClock() const {
            return std::fmod(double(getSyncClockTicks() / 1000000.0), 14400.0);
        }

        bool NetClient::checkStall() {
            if (mClient == nullptr) {
                al::setNerve(this, &nrvNetClientStall);
                return true;
            }
            return false;
        }

        void NetClient::exeStall() {
            if (al::isFirstStep(this))
                printf("pe::enet::NetClient stalling\n", 0);
        }

        void NetClient::exeConnect() {
            mClientCS.lock();

            if (al::isFirstStep(this)) {
                if (mClient)
                    enet_host_destroy(mClient);

                mClient = enet_host_create(nullptr, 1, 2, 0, 0);
                if (mClient == nullptr) {
                    PENET_WARN("ENet client creation failed", 0);
                }

                ENetAddress addr;
                enet_address_set_host_ip(&addr, mIp);
                addr.port = mPort;

                mServerPeer = enet_host_connect(mClient, &addr, sChannels.channelCount, 0);
                if (mServerPeer == nullptr) {
                    PENET_WARN("No available peers for initiating an ENet connection.", 0);
                    al::setNerve(this, &nrvNetClientStall);
                    return;
                }
                printf("Attempting to connect to server at %s:%u...\n", mIp, mPort);
            }

            if (checkStall()) {
                mClientCS.unlock();
                return;
            }

            ENetEvent event;
            while (enet_host_service(mClient, &event, 0) > 0)
                if (event.type == ENET_EVENT_TYPE_CONNECT)
                    al::setNerve(this, &nrvNetClientService);

            if (al::getNerveStep(this) > 5 * 60)
                al::setNerve(this, &nrvNetClientConnect);
            mClientCS.unlock();
        }

        void NetClient::exeService() {
            if (al::isFirstStep(this)) {
                ToS_Hello greetPacket;

                sendPacket(&greetPacket);
                printf("Connected to server at %s:%u\n", mIp, mPort);
            }

            ENetEvent event;
            if (!mClientCS.try_lock())
                return;

            if (checkStall()) {
                mClientCS.unlock();
                return;
            }

            u32 maxPackets = sMaxPacketsPerStep;
            while (maxPackets-- && enet_host_service(mClient, &event, 0) > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_DISCONNECT: {
                    printf("Disconnected from server\n");
                    // al::setNerve(this, &nrvNetClientStall);
                    al::setNerve(this, &nrvNetClientConnect);
                    mClientCS.unlock();
                    return;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    if (event.channelID > pe::enet::sChannels.channelCount) {
                        printf("Invalid packet type %d\n", event.channelID);
                        break;
                    }

                    u32 hash = nxdb::util::hashMurmur(event.packet->data + sizeof(u32), event.packet->dataLength - sizeof(u32));
                    u32 wantedHash = *reinterpret_cast<u32*>(event.packet->data);

                    if (hash != wantedHash) {
                        PENET_WARN("Dropped packet (corrupted) %x != %x", hash, wantedHash);
                    } else
                        mPacketHandler->handlePacket((ChannelType)event.channelID, event.packet->data + sizeof(u32), event.packet->dataLength - sizeof(u32));

                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_CONNECT:
                    break;
                case ENET_EVENT_TYPE_NONE:
                default:
                    break;
                }
            }
            mPacketsHandledInCurrentTick = sMaxPacketsPerStep - maxPackets;

            mClientCS.unlock();
        }

        static NetClient* sInstance = nullptr;

        void setNetClient(NetClient* client) { sInstance = client; }

        NetClient* getNetClient() {
            return sInstance;
        }

    } // namespace enet
} // namespace pe
