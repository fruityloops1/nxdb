#include "pe/Enet/ProjectPacketHandler.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/NetClient.h"
#include "pe/Enet/PacketHandler.h"
#include "pe/Enet/Packets/DataPackets.h"

namespace pe {
    namespace enet {

        static void handleShit(StartDebuggingRes* packet)
        {
            printf("dflkjsdflksdj %x %zu\n", packet->requestId, packet->data.sessionId);
        }

        const static PacketHandler<void>::PacketHandlerEntry sEntries[] {
            { ChannelType::ProcessListRes,
                (PacketHandler<void>::HandlePacketType)ProcessList::handleResponse },
            { ChannelType::StartDebuggingRes,
                (PacketHandler<void>::HandlePacketType)StartDebugging::handleResponse },
            { ChannelType::GetDebuggingSessionInfoRes,
                (PacketHandler<void>::HandlePacketType)GetDebuggingSessionInfo::handleResponse },
        };

        ProjectPacketHandler::ProjectPacketHandler()
            : PacketHandler(sEntries) {
        }

    } // namespace enet
} // namespace pe
