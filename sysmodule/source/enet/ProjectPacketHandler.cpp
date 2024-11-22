#include "ProjectPacketHandler.h"
#include "Server.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/PacketHandler.h"
#include "pe/Enet/Packets/DataPackets.h"
#include <chrono>
#include <ratio>
#include <thread>

namespace pe {
    namespace enet {

        struct Handlers {
            static void handleGreet(ToS_Hello* packet, Client* client) {
                client->handleGreet(packet);
            }
        };

        const static PacketHandler<Client>::PacketHandlerEntry sEntries[] {
            { ChannelType::ToS_Hello,
                (PacketHandler<Client>::HandlePacketType)Handlers::handleGreet },
        };

        ProjectPacketHandler::ProjectPacketHandler()
            : PacketHandler(sEntries) {
        }

        void ProjectPacketHandler::handlePacket(ChannelType type, const void* data, size_t len, Client* client) {
            for (int i = 0; i < mNumEntries; i++) {
                const PacketHandlerEntry& entry = mEntries[i];
                if (entry.type == type) {
                    PacketHandler<Client>::handlePacket(type, data, len, client);
                    return;
                }
            }

            PENET_ABORT("Packet Handler for packet %hhu not found\n", type);
        }

    } // namespace enet
} // namespace pe
