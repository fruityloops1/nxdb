#pragma once

#include "pe/Enet/DataPacket.h"
#include "pe/Enet/Request.h"
#include "pe/Enet/Types.h"

namespace pe {
    namespace enet {

        class Client;

        struct ToS_Hello : DataPacket<ToS_Hello> { };
        struct Test : DataPacket<Test> { };

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
                Process processes[sMaxPids];
            };

            static void handleRequest(Request* req, Client* client, u32 requestId);
        };

        REQUEST_DEFINES(ProcessList)

        struct StartDebugging_ {
            struct Request {
                u64 processIdToDebug;
            };
            struct Response {
                u64 sessionId;
                char _[0x10];
            };

            static void handleRequest(Request* req, Client* client, u32 requestId);
        };

        REQUEST_DEFINES(StartDebugging)

        struct GetDebuggingSessionInfo_ {
            struct Request {
                u64 sessionId;
            };
            struct Response {
                struct Module {
                    u64 base;
                    u64 size;
                    u16 nameOffsetIntoStringTable = -1;
                } __attribute__((packed));

                u64 processId;
                u64 programId;
                char processName[12] { 0 };
                s8 numModules = 0;
                Module modules[13];
                char moduleNameStringTable[0x300];
            };

            static void handleRequest(Request* req, Client* client, u32 requestId);
        };

        REQUEST_DEFINES(GetDebuggingSessionInfo)

        struct EditSubscription_ {
            struct Request {
                u64 sessionId;

                u64 subscriptionId = 0; // 0 = create new, other = edit
                u64 addr = 0; // -1 = delete
                size_t size = 0;
                int frequencyHz = 10;
            };
            struct Response {
                u64 subscriptionId = 0; // 0 if fail
                char _[0x10];
            };

            static void handleRequest(Request* req, Client* client, u32 requestId);
        };

        REQUEST_DEFINES(EditSubscription)

    } // namespace enet
} // namespace pe
