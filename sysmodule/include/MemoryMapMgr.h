#pragma once

#include "types.h"
#include <cstdint>
#include <vector>
extern "C" {
#include "switch/kernel/virtmem.h"
}

namespace nxdb {

    class DebuggingSession;

    class MemoryMapMgr {
        struct MapBlock {
            paddr startOrig;
            uintptr_t startMapped;
            size_t size;
            VirtmemReservation* reservation;

            void unmap() { }
        };

        DebuggingSession& mSession;
        std::vector<MapBlock> mBlocks;

    public:
        MemoryMapMgr(DebuggingSession& session)
            : mSession(session) { }
        ~MemoryMapMgr();

        void mapBlocks();
    };

} // namespace nxdb
