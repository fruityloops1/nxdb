#include "pe/Enet/ProjectPacketHandler.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/NetClient.h"
#include "pe/Enet/PacketHandler.h"
#include "pe/Enet/Packets/DataPackets.h"

namespace pe {
    namespace enet {

        const static PacketHandler<void>::PacketHandlerEntry sEntries[] {
            { ChannelType::ProcessListRes,
                (PacketHandler<void>::HandlePacketType)ProcessList::handleResponse },
        };

        ProjectPacketHandler::ProjectPacketHandler()
            : PacketHandler(sEntries) {
        }

    } // namespace enet
} // namespace pe
