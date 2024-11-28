#include "DebuggingSession.h"
#include "LogServer.h"
#include "Server.h"
#include "pe/Enet/Hash.h"
#include "pe/Enet/Packets/MemorySubscriptionData.h"
#include "switch/runtime/diag.h"
#include "types.h"

extern "C" {
#include "../ams/pm_ams.os.horizon.h"
#include "switch/arm/counter.h"
#include "switch/kernel/svc.h"
#include "switch/result.h"
}

namespace nxdb {

    DebuggingSession::DebuggingSession(u64 pid, pe::enet::Client* owner)
        : mOwner(owner)
        , mProcessId(pid)
        , mMapMgr(new MemoryMapMgr(*this)) {
        Result rc = svcDebugActiveProcess(&mDebugHandle, pid);
        if (R_SUCCEEDED(rc)) {
            R_ABORT_UNLESS(pmdmntAtmosphereGetProcessInfo(&mProcessHandle, nullptr, nullptr, pid));

            findModules();
            findName();

            mSessionId = armGetSystemTick();
            mMapMgr->mapBlocks();

            mDebugEventHandlerThread = std::thread(&DebuggingSession::debugEventHandlerThreadFunc, this);
            mDataTransferThread = std::thread(&DebuggingSession::dataTransterThreadFunc, this);

            mSubscriptions.push_back({ 44, mMapMgr->get<void>(mModules[0].base), 128, armGetSystemTick(), 60 });
        }

        nxdb::log("New session %zu owned by %x", mSessionId, mOwner->getHash());
    }

    DebuggingSession::~DebuggingSession() {
        nxdb::log("Deleting session %zu owned by %x", mSessionId, mOwner->getHash());
        delete mMapMgr;

        if (mDebugHandle)
            svcCloseHandle(mDebugHandle);
        if (mProcessHandle)
            svcCloseHandle(mProcessHandle);

        mKillThread = true;
        if (mDebugEventHandlerThread.joinable())
            mDebugEventHandlerThread.join();
        if (mDataTransferThread.joinable())
            mDataTransferThread.join();
    }

    Result DebuggingSession::readMemorySlow(void* out, paddr addr, size_t size) {
        return svcReadDebugProcessMemory(out, mDebugHandle, addr, size);
    }

    Result DebuggingSession::writeMemorySlow(const void* buffer, paddr addr, size_t size) {
        return svcWriteDebugProcessMemory(mDebugHandle, buffer, addr, size);
    }

    Result DebuggingSession::queryMemory(MemoryInfo* outMemInfo, u32* outPageInfo, paddr addr) {
        MemoryInfo dummyMemInfo;
        u32 dummyPageInfo;

        return svcQueryDebugProcessMemory(outMemInfo ?: &dummyMemInfo, outPageInfo ?: &dummyPageInfo, mDebugHandle, addr);
    }

    void DebuggingSession::findModules() {
        mNumModules = 0;

        paddr curAddr = 0;
        while (true) {
            MemoryInfo memoryInfo;

            if (R_SUCCEEDED(queryMemory(&memoryInfo, nullptr, curAddr))) {
                if (memoryInfo.perm == Perm_Rx && (memoryInfo.type == MemType_CodeStatic || memoryInfo.type == MemType_ModuleCodeStatic)) {
                    auto& module = mModules[mNumModules++];
                    module.base = memoryInfo.addr;
                    module.size = memoryInfo.size;

                    struct {
                        u32 zero;
                        s32 pathLength;
                        char path[0x200];
                    } modulePath;

                    if (R_SUCCEEDED(readMemorySlow(&modulePath, memoryInfo.addr + memoryInfo.size, sizeof(modulePath))) && modulePath.zero == 0 && modulePath.pathLength > 0 && modulePath.pathLength <= 0x200) {
                        std::memcpy(module.mod0Name, modulePath.path, modulePath.pathLength);
                        modulePath.path[modulePath.pathLength - 1] = '\0';
                    }
                }
            }

            const paddr next = memoryInfo.addr + memoryInfo.size;
            if (memoryInfo.type == MemType_Reserved)
                break;

            if (next < curAddr)
                break;
            curAddr = next;
        }
    }

    void DebuggingSession::findName() {
        nxdb::svc::DebugEventInfo d;
        R_ABORT_UNLESS(svcGetDebugEvent(&d, mDebugHandle));
        mProgramId = d.info.create_process.program_id;
        if (d.type == nxdb::svc::DebugEvent_CreateProcess) {
            std::memcpy(mName, d.info.create_process.name, sizeof(mName));
        }

        if (tryContinueImpl() == false)
            diagAbortWithResult(0xfefef0);
    }

    bool DebuggingSession::tryContinueImpl() {
        for (int core = 0; core < 4; core++) {
            R_ABORT_UNLESS(svcSetThreadCoreMask(Handle(nxdb::svc::PseudoHandle::CurrentThread), core, 1 << core));
            u64 tids[] { 0 };

            Result rc = svcContinueDebugEvent(mDebugHandle, nxdb::svc::ContinueFlag_ContinueAll, tids, 1);
            while (rc == 0xf401) {
                nxdb::svc::DebugEventInfo d;
                R_ABORT_UNLESS(svcGetDebugEvent(&d, mDebugHandle));
                // do nothing with it
                rc = svcContinueDebugEvent(mDebugHandle, nxdb::svc::ContinueFlag_ContinueAll, tids, 1);
            }

            if (R_FAILED(rc))
                return false;
        }
        return true;
    }

    void DebuggingSession::handleDebugEvent(const nxdb::svc::DebugEventInfo& d) {
        switch (d.type) {
        case svc::DebugEvent_CreateProcess:
        case svc::DebugEvent_CreateThread:
        case svc::DebugEvent_ExitProcess:
        case svc::DebugEvent_ExitThread:
        case svc::DebugEvent_Exception:
            break; // whatever
        default:
            break;
        }

        continue_();
    }

    void DebuggingSession::handleDebugEvents() {
        Result rc = 0;
        while (R_SUCCEEDED(rc)) {
            nxdb::svc::DebugEventInfo d;
            rc = svcGetDebugEvent(&d, mDebugHandle);

            if (R_SUCCEEDED(rc))
                handleDebugEvent(d);
        }
    }

    void DebuggingSession::debugEventHandlerThreadFunc() {
        while (mKillThread == false) {
            s32 idx;
            Result rc = svcWaitSynchronization(&idx, &mDebugHandle, 1, 250_ms);
            if (rc == 0xea01) // timeout
                continue;
            else
                R_ABORT_UNLESS(rc);

            if (R_SUCCEEDED(rc))
                handleDebugEvents();

            if (mShouldContinue) {
                tryContinueImpl();
                mShouldContinue = false;
            }
        }

        nxdb::log("exited Debug THREAD");
    }

    void DebuggingSession::dataTransterThreadFunc() {
        while (mKillThread == false) {
            svcSleepThread(1_s / 60);
            /**
             * ^ Usually, you'd calculate the least common multiple of the frequencies to
             * wait it out most accurately here, but that would exponentially inflate the
             * frequency for combinations such as [14Hz, 60Hz] -> 840Hz, and we don't really
             * need that accuracy as practically only simple frequencies like 24, 30, or 60
             * will be used.
             */

            for (MemorySubscription& sub : mSubscriptions) {
                const auto cntfrq = armGetSystemTickFreq();
                u64 now = armGetSystemTick();
                u64 nextSend = sub.lastSendTick + cntfrq / sub.sendFrequencyHz;
                if (now >= nextSend || nextSend - now < cntfrq / 120) {
                    u32 hash = util::hashMurmur((u8*)sub.mem, sub.size);
                    if (hash != sub.lastHash) {
                        pe::enet::MemorySubscriptionData packet(sub.id, sub.mem, sub.size);
                        mOwner->sendPacket(&packet, false);

                        sub.lastHash = hash;
                        sub.lastSendTick = armGetSystemTick();
                    }
                }
            }

            // pe::enet::Server::sInstance->flush();
        }

        nxdb::log("exited data Tranfer THREAD");
    }

    DebuggingSession::MemorySubscription* DebuggingSession::findSubscriptionById(u64 id) {
        for (MemorySubscription& sub : mSubscriptions)
            if (sub.id == id)
                return &sub;
        return nullptr;
    }

    void DebuggingSession::removeSubscriptionById(u64 id) {
        for (int i = 0; i < mSubscriptions.size(); i++) {
            MemorySubscription& sub = mSubscriptions[i];
            if (sub.id == id) {
                mSubscriptions.erase(mSubscriptions.begin() + i);
                break;
            }
        }
    }

    u64 DebuggingSessionMgr::registerNew(u64 pid, pe::enet::Client* owner) {
        DebuggingSession* session = new DebuggingSession(pid, owner);
        if (session->isValid()) {
            mSessions.push_back(session);
            return session->mSessionId;
        }

        return 0;
    }

    DebuggingSession* DebuggingSessionMgr::getBySessionId(u64 sessionId) {
        for (int i = 0; i < mSessions.size(); i++) {
            if (mSessions[i]->mSessionId == sessionId) {
                return mSessions[i];
            }
        }
        return nullptr;
    }

    DebuggingSession* DebuggingSessionMgr::getByProcessId(u64 processId) {
        for (int i = 0; i < mSessions.size(); i++) {
            if (mSessions[i]->mProcessId == processId) {
                return mSessions[i];
            }
        }
        return nullptr;
    }

    void DebuggingSessionMgr::deleteOwnedBy(pe::enet::Client* owner) {
        for (int i = 0; i < mSessions.size(); i++) {
            if (mSessions[i]->mOwner == owner) {
                delete mSessions[i];
                mSessions.erase(mSessions.begin() + i);
                i--;
            }
        }
    }

    void DebuggingSessionMgr::deleteById(u64 sessionId) {
        for (int i = 0; i < mSessions.size(); i++) {
            if (mSessions[i]->mSessionId == sessionId) {
                delete mSessions[i];
                mSessions.erase(mSessions.begin() + i);
                i--;
            }
        }
    }

    static DebuggingSessionMgr sInstance;
    DebuggingSessionMgr& DebuggingSessionMgr::instance() { return sInstance; }

} // namespace nxdb
