#pragma once

#include "enet/enet.h"
#include "pe/Enet/Packets/DataPackets.h"
#include "pe/Enet/Types.h"
#include <functional>

namespace pe {
    namespace enet {

        class Server;
        inline size_t calcEnetAddressHashCode(ENetAddress address) {
            size_t hash = std::hash<u32>()(address.host);
            hash ^= std::hash<u16>()(address.port);
            return hash;
        }

        class Client {
            Server* mServer = nullptr;
            ENetPeer* mPeer = nullptr;
            bool mHasGreeting = false;

        public:
            Client() { }
            Client(Server* parent, ENetPeer* peer);

            void sendPacket(IPacket* packet, bool reliable = true);

            bool hasGreeted() const { return mHasGreeting; }
            size_t getHash() const { return calcEnetAddressHashCode(mPeer->address); }
            ENetPeer* getPeer() const { return mPeer; }

            void handleGreet(ToS_Hello* packet);
            void disconnect();

            friend struct Handlers;
            friend class Console;
            friend class Server;
        };

    } // namespace enet
} // namespace pe
