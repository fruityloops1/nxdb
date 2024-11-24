#include "loop.h"
#include "LogServer.h"
#include "ProjectPacketHandler.h"
#include "Server.h"
#include "enet/enet.h"
#include "pe/Enet/Packets/DataPackets.h"
#include <cmath>
#include <stdlib.h>

extern "C" {
#include "switch/arm/counter.h"
#include "switch/kernel/svc.h"
#include "switch/runtime/diag.h"
#include "switch/services/hid.h"
}

static pe::enet::Server* server = nullptr;

static void no_memory() {
    nxdb::log("ENet ran out of memory", 0);
    diagAbortWithResult(0x0);
}

void shit() {
    const ENetCallbacks callbacks { malloc, free, no_memory };
    if (enet_initialize_with_callbacks(ENET_VERSION, &callbacks) != 0) {
        nxdb::log("ENet initialize failed", 0);
        diagAbortWithResult(0x0);
    }

    pe::enet::ProjectPacketHandler handler;
    pe::enet::Server server({ ENET_HOST_ANY, 3450 }, handler, callbacks);
    ::server = &server;
    server.start();

    while (true)
        svcSleepThread(1000000 * 1000);
}
