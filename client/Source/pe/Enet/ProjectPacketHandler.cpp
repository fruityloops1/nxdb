#include "pe/Enet/ProjectPacketHandler.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/NetClient.h"
#include "pe/Enet/PacketHandler.h"
#include "pe/Enet/Packets/DataPackets.h"
#include "pe/Render.h"

namespace pe {
    namespace enet {

        static void handleTest(Test* packet) {
            printf(":3 %d\n", packet->d);
        }

        static void handleImGuiDrawData(ImGuiDrawDataPacket* packet) {
            pe::render(packet->getDrawData());
        }

        const static PacketHandler<void>::PacketHandlerEntry sEntries[] {
            { ChannelType::Test,
                (PacketHandler<void>::HandlePacketType)handleTest },
            { ChannelType::ImGuiDrawDataPacket,
                (PacketHandler<void>::HandlePacketType)handleImGuiDrawData },
        };

        ProjectPacketHandler::ProjectPacketHandler()
            : PacketHandler(sEntries) {
        }

    } // namespace enet
} // namespace pe
