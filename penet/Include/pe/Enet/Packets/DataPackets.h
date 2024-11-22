#pragma once

#include "pe/Enet/DataPacket.h"
#include "pe/Enet/Types.h"

namespace pe {
    namespace enet {

        struct ToS_Hello : DataPacket<ToS_Hello> { };
        struct Test : DataPacket<Test> {
            int d;
        };

    } // namespace enet
} // namespace pe
