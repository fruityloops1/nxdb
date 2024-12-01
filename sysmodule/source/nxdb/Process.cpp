#include "Process.h"
#include <algorithm>
#include <cstring>

extern "C" {
#include "switch/kernel/svc.h"
#include "switch/services/pm.h"
}

namespace nxdb {

    s32 getProcessList(Process* out, s32 max) {
        u64 pids[sMaxPids] { 0 };
        s32 numPids = 0;

        if (false) {
            R_ABORT_UNLESS(svcGetProcessList(&numPids, pids, sMaxPids));
        } else { // ^ this shit doesnt work and the result code cant possibly make any sense so complain to me about the following shitty code
            constexpr int sMaxPidSearchRange = 0x1000;
            for (int i = 0; i < sMaxPidSearchRange; i++) {
                u64 programId;
                if (R_SUCCEEDED(pmdmntGetProgramId(&programId, i)))
                    pids[numPids++] = i;
            }
        }

        const s32 numRead = std::min(numPids, max);

        for (int i = 0; i < numRead; i++) {
            auto& p = out[i];

            Handle debugHandle;
            p.processId = pids[i];
            if (R_SUCCEEDED(svcDebugActiveProcess(&debugHandle, p.processId))) {
                nxdb::svc::DebugEventInfo d;
                R_ABORT_UNLESS(svcGetDebugEvent(&d, debugHandle));
                p.programId = d.info.create_process.program_id;
                if (d.type == nxdb::svc::DebugEvent_CreateProcess) {
                    std::memcpy(p.name, d.info.create_process.name, sizeof(p.name));
                }

                svcCloseHandle(debugHandle);
            }
        }

        return numRead;
    }

} // namespace nxdb
