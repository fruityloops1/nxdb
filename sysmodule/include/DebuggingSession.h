#pragma once

#include "Client.h"
#include "MemoryMapMgr.h"
#include "types.h"
#include <thread>
#include <vector>

namespace nxdb {

    struct Module {
        paddr base = 0;
        size_t size = 0;

        char mod0Name[0x200] { 0 };
    };

    constexpr static size_t sMaxModules = 13;
    struct DebuggingSession {
        struct MemorySubscription {
            u64 id = 0;
            void* mem = nullptr;
            size_t size = 0;

            u64 lastSendTick = 0;
            u32 lastHash = 0;
            int sendFrequencyHz = 0;
        };

        static constexpr int sSubscriptionsMax = 32;

        u64 mSessionId = 0;
        pe::enet::Client* mOwner = nullptr;

        u64 mProcessId = 0;
        u64 mProgramId = 0;
        Handle mDebugHandle = 0;
        Handle mProcessHandle = 0;

        s32 mNumModules = 0;
        Module mModules[sMaxModules];
        char mName[12] { 0 };

        std::thread mDebugEventHandlerThread;
        std::thread mDataTransferThread;
        bool mKillThread = false;
        bool mShouldContinue = true;

        std::vector<MemorySubscription> mSubscriptions;

        MemoryMapMgr* mMapMgr = nullptr;

        DebuggingSession(u64 pid, pe::enet::Client* owner);
        ~DebuggingSession();

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

        MemorySubscription* findSubscriptionById(u64 id);
        void removeSubscriptionById(u64 id);
    };

    class DebuggingSessionMgr {
        std::vector<DebuggingSession*> mSessions;

    public:
        static DebuggingSessionMgr& instance();

        u64 registerNew(u64 pid, pe::enet::Client* owner);
        DebuggingSession* getBySessionId(u64 sessionId);
        DebuggingSession* getByProcessId(u64 processId);
        void deleteOwnedBy(pe::enet::Client* owner);
        void deleteById(u64 sessionId);
    };

} // namespace nxdb
