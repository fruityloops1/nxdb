#pragma once

#include "pe/Enet/Types.h"
#include <string>
#include <vector>

namespace nxdb {

    struct Process {
        struct Module {
            u64 base;
            u64 size;
            std::string name;
        };

        u64 processId;
        u64 programId;
        std::string processName;
        std::vector<Module> modules;
    };

} // namespace nxdb
