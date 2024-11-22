#pragma once

#include "Client.h"
#include "pe/Enet/PacketHandler.h"

namespace pe {
    namespace enet {

        class ProjectPacketHandler : public PacketHandler<Client> {
        public:
            ProjectPacketHandler();

            virtual void handlePacket(ChannelType type, const void* data, size_t len, Client* client = nullptr) override;

            friend struct Handlers;
        };

    } // namespace enet
} // namespace pe
