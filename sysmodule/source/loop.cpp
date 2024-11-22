#include "loop.h"
#include "LogServer.h"
#include "ProjectPacketHandler.h"
#include "Server.h"
#include "enet/enet.h"
#include "imgui_internal.h"
#include "pe/Enet/Packets/DataPackets.h"
#include "pe/Enet/Packets/ImGuiDrawDataPacket.h"
#include "switch/arm/counter.h"
#include "switch/kernel/svc.h"
#include <stdlib.h>

extern "C" {
#include "switch/runtime/diag.h"
}

#include "imgui.h"

static pe::enet::Server* server = nullptr;

static void no_memory() {
    nxdb::log("ENet ran out of memory", 0);
    diagAbortWithResult(0x0);
}

constexpr u64 sFrameRate = 15;
const u64 sFrameTimeTicks = (1.0 / sFrameRate) * armGetSystemTickFreq();

void render() {
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();
    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImDrawData* data = ImGui::GetDrawData();

    /*if (data->CmdLists.size()) {
        auto& d = data->CmdLists[0]->CmdBuffer[1];
        nxdb::log("%d %d %d %llu %f\n", d.VtxOffset, d.IdxOffset, d.ElemCount, d.TextureId, data->CmdLists[0]->VtxBuffer[5].pos.y);
    }*/

    pe::enet::ImGuiDrawDataPacket packet(data);
    nxdb::log("packet size: %zu", packet.calcSize());
    server->sendPacketToAll(&packet);
}

void shit() {

    const ENetCallbacks callbacks { malloc, free, no_memory };
    if (enet_initialize_with_callbacks(ENET_VERSION, &callbacks) != 0) {
        nxdb::log("ENet initialize failed", 0);
        diagAbortWithResult(0x0);
    }

    pe::enet::ProjectPacketHandler handler;
    pe::enet::Server server({ ENET_HOST_ANY, 3450 }, handler, callbacks);
    ::server = &server;

    server.start();

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto& io = ImGui::GetIO();
    io.DisplaySize = { 1280, 720 };

    {
        u8* pixels;
        int width, height;
        int size;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &size);

        nxdb::log("Font texture size: %d:%d", width, height);

        FILE* tex = fopen("fonttexture.rgba32", "wb");
        fwrite(pixels, width * height * size, 1, tex);
        fclose(tex);
    }

    u64 lastFrame = armGetSystemTick();
    while (true) {
        render();

        u64 diff = armGetSystemTick() - lastFrame;
        svcSleepThread(armTicksToNs(sFrameTimeTicks - diff));
        lastFrame = armGetSystemTick();
    }
}
