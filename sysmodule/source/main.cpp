#include "LogServer.h"
extern "C" {
#include "services/capssc.h"
#include "switch/kernel/event.h"
#include "switch/kernel/svc.h"
#include "switch/kernel/thread.h"
#include "switch/result.h"
#include "switch/runtime/devices/fs_dev.h"
#include "switch/runtime/diag.h"
#include "switch/runtime/pad.h"
#include "switch/services/apm.h"
#include "switch/services/applet.h"
#include "switch/services/fs.h"
#include "switch/services/hid.h"
#include "switch/services/nv.h"
#include "switch/services/pm.h"
#include "switch/services/set.h"
#include "switch/services/spsm.h"
#include "switch/services/vi.h"
#include "switch/types.h"
}
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <switch.h>
#include <sys/socket.h>
#include <unistd.h>

extern "C" {
NvServiceType __nx_nv_service_type = NvServiceType_Factory;
u32 __nx_applet_type = AppletType_None;

u32 __nx_fs_num_sessions = 1;

size_t __nx_heap_size = 0x200000 * 1;

void __appInit(void) {
    Result rc;

    rc = smInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
    }

    rc = setInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(rc);

    static const SocketInitConfig socketInitConfig = {
        .tcp_tx_buf_size = 0x14000,
        .tcp_rx_buf_size = 0x1000,
        .tcp_tx_buf_max_size = 0,
        .tcp_rx_buf_max_size = 0,

        .udp_tx_buf_size = 0x2400,
        .udp_rx_buf_size = 0xA500,

        .sb_efficiency = 2,

        .num_bsd_sessions = 3,
        .bsd_service_type = BsdServiceType_User,
    };

    rc = socketInitialize(&socketInitConfig);
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));

    rc = hidInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

    rc = hidsysInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

    rc = viInitialize(ViServiceType_Manager);
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));

    rc = pmdmntInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));

    rc = fsInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    rc = capsscInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));

    rc = spsmInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));

    smExit();
}

void __appExit(void) {
    viExit();
    hidExit();
    pmdmntExit();
    fsdevUnmountAll();
    fsExit();
}
}

#include "nxdb/ImGuiLoop.h"

extern "C" int main(int argc, char* argv[]) {
    fsdevMountSdmc();

    nxdb::LogServer::instance() = new nxdb::LogServer;
    nxdb::LogServer::instance()->StartServer();

    for (int i = 0; i < 5; i++) {
        nxdb::LogServer::instance()->Poll();
        nxdb::log("Waiting %d", i);
        sleep(1);
    }

    nxdb::runImguiLoop();

    return 0;
}
