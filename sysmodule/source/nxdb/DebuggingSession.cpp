#include "DebuggingSession.h"
#include "LogServer.h"

extern "C" {
#include "../ams/pm_ams.os.horizon.h"
#include "switch/arm/counter.h"
#include "switch/kernel/svc.h"
#include "switch/result.h"
}

namespace nxdb {

    DebuggingSession::DebuggingSession(u64 pid, pe::enet::Client* owner)
        : mOwner(owner)
        , mProcessId(pid) {
        Result rc = svcDebugActiveProcess(&mDebugHandle, pid);
        if (R_SUCCEEDED(rc)) {
            R_ABORT_UNLESS(pmdmntAtmosphereGetProcessInfo(&mProcessHandle, nullptr, nullptr, pid));

            findModules();
            findName();

            mSessionId = armGetSystemTick();
        }

        nxdb::log("New session %zu owned by %x", mSessionId, mOwner->getHash());
    }

    DebuggingSession::~DebuggingSession() {
        nxdb::log("Deleting session %zu owned by %x", mSessionId, mOwner->getHash());
        if (mDebugHandle)
            svcCloseHandle(mDebugHandle);
    }

    Result DebuggingSession::readMemorySlow(void* out, paddr addr, size_t size) {
        return svcReadDebugProcessMemory(out, mDebugHandle, addr, size);
    }

    Result DebuggingSession::writeMemorySlow(const void* buffer, paddr addr, size_t size) {
        return svcWriteDebugProcessMemory(mDebugHandle, buffer, addr, size);
    }

    Result DebuggingSession::queryMemory(MemoryInfo* outMemInfo, u32* outPageInfo, Handle debugHandle, paddr addr) {
        MemoryInfo dummyMemInfo;
        u32 dummyPageInfo;

        return svcQueryDebugProcessMemory(outMemInfo ?: &dummyMemInfo, outPageInfo ?: &dummyPageInfo, debugHandle, addr);
    }

    void DebuggingSession::findModules() {
        mNumModules = 0;

        paddr curAddr = 0;
        while (true) {
            MemoryInfo memoryInfo;

            if (R_SUCCEEDED(queryMemory(&memoryInfo, nullptr, mDebugHandle, curAddr))) {
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
    }

    u64 DebuggingSessionMgr::registerNew(u64 pid, pe::enet::Client* owner) {
        DebuggingSession* session = new DebuggingSession(pid, owner);
        if (session->isValid()) {
            mSessions.push_back(session);
            return session->mSessionId;
        }

        return 0;
    }

    DebuggingSession* DebuggingSessionMgr::getById(u64 sessionId) {
        for (int i = 0; i < mSessions.size(); i++) {
            if (mSessions[i]->mSessionId == sessionId) {
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
