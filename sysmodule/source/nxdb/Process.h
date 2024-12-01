#pragma once

#include "types.h"

namespace nxdb {

    constexpr int sMaxPids = 0x50;

    struct Process {
        u64 processId = 0;
        u64 programId = 0;
        char name[12];
    };

    s32 getProcessList(Process* out, s32 max);

} // namespace nxdb
