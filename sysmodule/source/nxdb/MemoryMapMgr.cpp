#include "MemoryMapMgr.h"
#include "DebuggingSession.h"
#include "LogServer.h"
#include "switch/kernel/svc.h"
#include "switch/kernel/virtmem.h"
#include "switch/result.h"
#include "types.h"

namespace nxdb {

    static void perm(char* out, u32 perm) {
        if (perm & Perm_R)
            *out++ = 'R';
        if (perm & Perm_W)
            *out++ = 'W';
        if (perm & Perm_X)
            *out++ = 'X';
        *out = '\0';
    }

    void MemoryMapMgr::MapBlock::unmap(Handle processHandle) {
        MemoryInfo info;
        u32 pageinfo;
        uintptr_t addr = startMapped;

        while (R_SUCCEEDED(svcQueryMemory(&info, &pageinfo, addr))) {
            if (addr + info.size == 0x0000008000000000 || info.type == 0 || addr >= startMapped + size)
                break;

            nxdb::log("svcUnmapProcessMemory(%p, %d, %p, %p)", reinterpret_cast<void*>(addr), processHandle, addr, info.size);
            R_ABORT_UNLESS(svcUnmapProcessMemory(reinterpret_cast<void*>(addr), processHandle, addr, info.size))

            addr += info.size;
        }
    }

    void MemoryMapMgr::mapBlocks() {
        paddr addr = 0;

        MemoryInfo* mapBuffer = new MemoryInfo[256];
        int numMaps = 0;

        auto mapBlock = [this](const MemoryInfo* maps, int numMaps) {
            auto& first = maps[0];
            auto& last = maps[numMaps - 1];
            size_t blockSize = last.addr - first.addr + last.size;

            virtmemLock();

            void* found = virtmemFindAslr(blockSize, 0x10000);
            if (found == nullptr)
                diagAbortWithResult(0xfefef8ef);
            auto* reservation = virtmemAddReservation(found, blockSize);
            uintptr_t mapped = reinterpret_cast<uintptr_t>(found);

            virtmemUnlock();

            nxdb::log("\n\n\nblock: 0x%016lx-0x%016lx", maps[0].addr, maps[numMaps - 1].addr + maps[numMaps - 1].size);
            for (int i = 0; i < numMaps; i++) {
                auto& info = maps[i];
                paddr base = info.addr;
                paddr end = info.addr + info.size;

                uintptr_t offsetIntoBlock = info.addr - first.addr;

                char perms[8];
                perm(perms, info.perm);

                nxdb::log("map 0x%016lx-0x%016lx state:%d perm:%s", base, end, info.type, perms);

                if (info.type == MemType_SharedMem || info.type == MemType_MappedMemory || info.type == MemType_ThreadLocal) {
                    continue;
                }

                nxdb::log("svcMapProcessMemory(%p, %d, %p, %p)", mapped + offsetIntoBlock, mSession.mProcessHandle, info.addr, info.size);
                R_ABORT_UNLESS(svcMapProcessMemory(reinterpret_cast<void*>(mapped + offsetIntoBlock), mSession.mProcessHandle, info.addr, info.size));
                R_DESCRIPTION(0);
            }

            mBlocks.push_back({ first.addr, mapped, blockSize, reservation });
        };

        MemoryInfo info;
        while (R_SUCCEEDED(mSession.queryMemory(&info, nullptr, addr))) {
            if (addr + info.size == 0x0000008000000000)
                break;

            if (info.type == 0) {
                if (numMaps > 0) {
                    mapBlock(mapBuffer, numMaps);
                }
                numMaps = 0;
            } else
                mapBuffer[numMaps++] = info;

            addr += info.size;
        }

        delete[] mapBuffer;
    }

    MemoryMapMgr::~MemoryMapMgr() {
        for (auto& block : mBlocks)
            block.unmap(mSession.mProcessHandle);
    }

} // namespace nxdb
