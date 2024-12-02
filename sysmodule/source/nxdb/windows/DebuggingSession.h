#pragma once

#include "../Component.h"
#include "../MemoryMapMgr.h"
#include "Server.h"
#include "types.h"
#include <thread>
#include <vector>

extern "C" {
#include "switch/kernel/svc.h"
}

namespace nxdb {

    struct Module {
        paddr base = 0;
        size_t size = 0;

        char mod0Name[0x200] { 0 };
    };

    constexpr static size_t sMaxModules = 13;
    struct DebuggingSession : public Component {
        u64 mSessionId = 0;

        u64 mProcessId = 0;
        u64 mProgramId = 0;
        Handle mDebugHandle = 0;
        Handle mProcessHandle = 0;

        s32 mNumModules = 0;
        Module mModules[sMaxModules];
        char mName[12] { 0 };

        std::thread mDebugEventHandlerThread;
        bool mKillThread = false;
        bool mShouldContinue = true;

        MemoryMapMgr* mMapMgr = nullptr;

        Component* mModuleList = nullptr;

        DebuggingSession(u64 pid);
        ~DebuggingSession() override;

        void findModules();
        void findName();

        bool tryContinueImpl();
        void continue_() { mShouldContinue = true; }
        void handleDebugEvent(const nxdb::svc::DebugEventInfo& d);
        void handleDebugEvents();
        void debugEventHandlerThreadFunc();
        void dataTransterThreadFunc();

        bool isValid() { return mDebugHandle && mSessionId; }

        Result readMemorySlow(void* out, paddr addr, size_t size);
        Result writeMemorySlow(const void* buffer, paddr addr, size_t size);
        Result queryMemory(MemoryInfo* outMemInfo, u32* outPageInfo, paddr addr);

        void update() override;
        void createComponents();

        void queueForDeletion() override;
    };

} // namespace nxdb