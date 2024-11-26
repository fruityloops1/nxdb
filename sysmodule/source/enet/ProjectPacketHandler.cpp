#include "ProjectPacketHandler.h"
#include "DebuggingSession.h"
#include "LogServer.h"
#include "Server.h"
#include "pe/Enet/Channels.h"
#include "pe/Enet/PacketHandler.h"
#include "pe/Enet/Packets/DataPackets.h"
#include "switch/arm/counter.h"
#include "switch/result.h"
#include "types.h"
#include <chrono>
#include <ratio>
#include <thread>

extern "C" {
#include "switch/kernel/svc.h"
#include "switch/runtime/diag.h"
#include "switch/runtime/env.h"
#include "switch/services/pm.h"
}

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
            { ChannelType::ProcessListReq,
                (PacketHandler<Client>::HandlePacketType)ProcessList::handleRequest<Client> },
            { ChannelType::StartDebuggingReq,
                (PacketHandler<Client>::HandlePacketType)StartDebugging::handleRequest<Client> },
            { ChannelType::GetDebuggingSessionInfoReq,
                (PacketHandler<Client>::HandlePacketType)GetDebuggingSessionInfo::handleRequest<Client> },
            { ChannelType::EditSubscriptionReq,
                (PacketHandler<Client>::HandlePacketType)EditSubscription::handleRequest<Client> },
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

        void ProcessList_::handleRequest(ProcessList_::Request* request, Client* client, u32 requestId) {
            u64 pids[sMaxPids] { 0 };
            s32 numPids = 0;

            nxdb::log("requestId %d", requestId);

            if (false) {
                R_ABORT_UNLESS(svcGetProcessList(&numPids, pids, sMaxPids));
            } else { // ^ this shit doesnt work and the result code cant possibly make any sense so complain to me about the following shitty code
                constexpr int sMaxPidSearchRange = 0x1000;
                for (int i = 0; i < sMaxPidSearchRange; i++) {
                    u64 programId;
                    if (R_SUCCEEDED(pmdmntGetProgramId(&programId, i)))
                        pids[numPids++] = i;
                }
            }

            ProcessList::ResponsePacketType packet(requestId);
            auto& res = packet.data;

            for (int i = 0; i < numPids; i++) {
                auto& p = res.processes[res.num];

                Handle debugHandle;
                p.processId = pids[i];
                if (R_SUCCEEDED(svcDebugActiveProcess(&debugHandle, p.processId))) {
                    nxdb::svc::DebugEventInfo d;
                    R_ABORT_UNLESS(svcGetDebugEvent(&d, debugHandle));
                    p.programId = d.info.create_process.program_id;
                    if (d.type == nxdb::svc::DebugEvent_CreateProcess) {
                        std::memcpy(p.name, d.info.create_process.name, sizeof(p.name));
                    }

                    svcCloseHandle(debugHandle);

                    res.num++;
                }
            }
            nxdb::log("sending process list %d", numPids);

            client->sendPacket(&packet);
        }

        void StartDebugging_::handleRequest(Request* req, Client* client, u32 requestId) {
            auto& mgr = nxdb::DebuggingSessionMgr::instance();
            bool exists = mgr.getByProcessId(req->processIdToDebug) != nullptr;

            StartDebugging::ResponsePacketType packet(requestId);
            auto& res = packet.data;
            res.sessionId = exists ? 0
                                   : mgr.registerNew(req->processIdToDebug, client);

            client->sendPacket(&packet);
        }

        void GetDebuggingSessionInfo_::handleRequest(Request* req, Client* client, u32 requestId) {
            auto* session = nxdb::DebuggingSessionMgr::instance().getBySessionId(req->sessionId);

            nxdb::log("getdebug shit %zu", req->sessionId);

            GetDebuggingSessionInfo::ResponsePacketType packet(requestId);
            auto& res = packet.data;

            if (session == nullptr) {
                res.programId = 0x0;
            } else {
                res.programId = session->mProgramId;
                res.processId = session->mProcessId;
                std::memcpy(res.processName, session->mName, sizeof(res.processName));
                res.numModules = session->mNumModules;

                u16 stringTableOffset = 0;
                for (int i = 0; i < session->mNumModules; i++) {
                    auto& dst = res.modules[i];
                    auto& src = session->mModules[i];

                    dst.base = src.base;
                    dst.size = src.size;

                    size_t strSize = std::strlen(src.mod0Name) + sizeof('\0');

                    if (stringTableOffset + strSize < sizeof(res.moduleNameStringTable)) {
                        std::memcpy(res.moduleNameStringTable + stringTableOffset, src.mod0Name, strSize);

                        dst.nameOffsetIntoStringTable = stringTableOffset;
                        stringTableOffset += strSize;
                    }
                }
            }

            client->sendPacket(&packet);
        }

        void EditSubscription_::handleRequest(Request* req, Client* client, u32 requestId) {
            auto* session = nxdb::DebuggingSessionMgr::instance().getBySessionId(req->sessionId);

            EditSubscription::ResponsePacketType packet(requestId);
            auto& res = packet.data;

            nxdb::log("EditSubscription_: %zu %p %p %d Hz added", req->subscriptionId, req->addr, req->size, req->frequencyHz);

            if (session == nullptr) {
                nxdb::log("EditSubscription_: invalid session %zu", req->sessionId);
            } else {
                void* mappedAddr = session->mMapMgr->get<void>(req->addr);

                auto& subs = session->mSubscriptions;
                if (mappedAddr) {
                    if (req->subscriptionId == 0) {
                        nxdb::DebuggingSession::MemorySubscription sub;
                        sub.id = armGetSystemTick();
                        sub.sendFrequencyHz = req->frequencyHz;
                        sub.mem = mappedAddr;
                        sub.size = req->size;
                        res.subscriptionId = sub.id;
                        subs.push_back(sub);
                    } else {
                        auto* sub = session->findSubscriptionById(res.subscriptionId);
                        if (sub != nullptr) {
                            if (req->addr == -1) {
                                session->removeSubscriptionById(res.subscriptionId);
                            } else {
                                sub->size = req->size;
                                sub->mem = mappedAddr;
                                res.subscriptionId = sub->id;
                            }
                        } else
                            nxdb::log("EditSubscription_: invalid subscription %zu", req->subscriptionId);
                    }
                } else {
                    nxdb::log("EditSubscription_: %p unmapped addr", req->addr);
                }
            }
            nxdb::log("EditSubscription_: sent subid %zu", res.subscriptionId);

            client->sendPacket(&packet);
        }

    } // namespace enet
} // namespace pe
