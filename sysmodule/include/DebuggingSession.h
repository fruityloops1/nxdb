#pragma once

#include "Client.h"
#include "types.h"
#include <vector>

namespace nxdb {

    struct Module {
        paddr base = 0;
        size_t size = 0;

        char mod0Name[0x200] { 0 };
    };

    constexpr static size_t sMaxModules = 13;
    struct DebuggingSession {
        u64 mSessionId = 0;
        pe::enet::Client* mOwner = nullptr;

        u64 mProcessId = 0;
        u64 mProgramId = 0;
        Handle mDebugHandle = 0;
        Handle mProcessHandle = 0;

        s32 mNumModules = 0;
        Module mModules[sMaxModules];
        char mName[12] { 0 };

        DebuggingSession(u64 pid, pe::enet::Client* owner);
        ~DebuggingSession();
        void findModules();
        void findName();

        bool isValid() { return mDebugHandle; }

        Result readMemorySlow(void* out, paddr addr, size_t size);
        Result writeMemorySlow(const void* buffer, paddr addr, size_t size);
        Result queryMemory(MemoryInfo* outMemInfo, u32* outPageInfo, Handle debugHandle, paddr addr);
    };

    class DebuggingSessionMgr {
        std::vector<DebuggingSession*> mSessions;

    public:
        static DebuggingSessionMgr& instance();

        u64 registerNew(u64 pid, pe::enet::Client* owner);
        DebuggingSession* getById(u64 sessionId);
        void deleteOwnedBy(pe::enet::Client* owner);
        void deleteById(u64 sessionId);
    };

} // namespace nxdb
