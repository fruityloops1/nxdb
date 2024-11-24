#pragma once

#include "pe/Enet/DataPacket.h"
#include "pe/Enet/Request.h"
#include "pe/Enet/Types.h"

namespace pe {
    namespace enet {

        class Client;

        struct ToS_Hello : DataPacket<ToS_Hello> { };

        struct ProcessList_ {
            constexpr static size_t sMaxPids = 0x50;

            struct Request {
            };
            struct Response {
                struct Process {
                    u64 processId;
                    u64 programId;
                    char name[12] { 0 };
                } __attribute__((packed));

                s32 num = 0;
                Process processes[80];
            };

            static void handleRequest(Request* req, Client* client, u32 requestId);
        };

        REQUEST_DEFINES(ProcessList)

    } // namespace enet
} // namespace pe
