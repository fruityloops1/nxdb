#include "Client.h"
#include "DebuggingSession.h"
#include "Server.h"
#include "enet/enet.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/Packets/DataPackets.h"
#include <cstdio>
#include <thread>

namespace pe {
    namespace enet {

        Client::Client(Server* parent, ENetPeer* peer)
            : mServer(parent)
            , mPeer(peer) {
            ENetAddress address = peer->address;

            in_addr addr { address.host };
            nxdb::log("New client connected from %s:%u (%zx)", inet_ntoa(addr), address.port, getHash());
        }

        void Client::sendPacket(IPacket* packet, bool reliable) {
            if (mPeer->state != ENET_PEER_STATE_CONNECTED)
                return;

            u32 flags = ENET_PACKET_FLAG_NO_ALLOCATE;
            if (reliable)
                flags |= ENET_PACKET_FLAG_RELIABLE;
            size_t packetSize = packet->calcSize();
            u8* buf = new u8[packetSize];
            size_t len = packet->build(buf);

            ENetPacket* pak = enet_packet_create(buf, len, flags);
            enet_peer_send(mPeer, (int)identifyType(packet), pak);
            delete[] buf;
        }

        void Client::disconnect() {
        }

        Client::~Client() {
            nxdb::DebuggingSessionMgr::instance().deleteOwnedBy(this);
        }

    } // namespace enet
} // namespace pe
