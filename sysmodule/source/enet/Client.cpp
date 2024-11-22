#include "Client.h"
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
            size_t len = packet->calcSize();
            u8* buf = new u8[len];
            packet->build(buf);

            ENetPacket* pak = enet_packet_create(buf, len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
            enet_peer_send(mPeer, (int)identifyType(packet), pak);
            delete[] buf;
        }

        void Client::handleGreet(ToS_Hello* packet) {
        }

        void Client::disconnect() {
        }

    } // namespace enet
} // namespace pe
