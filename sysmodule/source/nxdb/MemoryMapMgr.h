#pragma once

#include "types.h"
#include <cstdint>
#include <vector>
extern "C" {
#include "switch/kernel/virtmem.h"
}

namespace nxdb {

    struct DebuggingSession;

    class MemoryMapMgr {
        struct MapBlock {
            paddr startOrig;
            uintptr_t startMapped;
            size_t size;
            VirtmemReservation* reservation;

            void unmap(Handle processHandle);

            uintptr_t get(paddr ptr) const {
                return (ptr - startOrig) + startMapped;
            }

            template <typename T>
            T* get(paddr ptr) const {
                return reinterpret_cast<T*>(get(ptr));
            }

            template <typename T>
            T* get(T* ptr) const {
                return get(paddr(ptr));
            }
        };

        DebuggingSession& mSession;
        std::vector<MapBlock> mBlocks;

    public:
        MemoryMapMgr(DebuggingSession& session)
            : mSession(session) { }
        ~MemoryMapMgr();

        void mapBlocks();

        uintptr_t get(paddr ptr) const {
            for (auto& block : mBlocks) {
                if (ptr >= block.startOrig && ptr <= block.startOrig + block.size) {
                    return block.get(ptr);
                }
            }
            return 0;
        }

        template <typename T>
        T* get(paddr ptr) const {
            return reinterpret_cast<T*>(get(ptr));
        }

        template <typename T>
        T* get(T* ptr) const {
            return get(paddr(ptr));
        }

        const MapBlock* findBlock(paddr ptr) const {
            for (auto& block : mBlocks) {
                if (ptr >= block.startOrig && ptr <= block.startOrig + block.size) {
                    return &block;
                }
            }
            return nullptr;
        }

        template <typename T>
        const MapBlock* findBlock(T* ptr) const {
            return findBlock(paddr(ptr));
        }
    };

} // namespace nxdb
