#pragma once

#include "LogServer.h"
#include "switch/arm/counter.h"
#include "types.h"

namespace nxdb {

    class TimeProfiler {
        u64 mStartTick = 0;
        const char* mName = "";

    public:
        TimeProfiler(const char* name)
            : mStartTick(armGetSystemTick())
            , mName(name) { }

        void print() {
            u64 now = armGetSystemTick();
            u64 ns = armTicksToNs(now - mStartTick);

            if (ns < 1_us)
                nxdb::log("%[%s]: %zuns", mName, ns);
            else if (ns < 1_ms)
                nxdb::log("%[%s]: %4.2fµs", mName, ns / float(1_us));
            else if (ns < 1_s)
                nxdb::log("%[%s]: %4.3fms", mName, ns / float(1_ms));
            else
                nxdb::log("%[%s]: %4.4fs", mName, ns / float(1_s));
        }
    };

} // namespace nxdb
