#include "loop.h"
#include "LogServer.h"
#include "ProjectPacketHandler.h"
#include "Server.h"
#include "enet/enet.h"
#include "imgui_internal.h"
#include "pe/Enet/Packets/DataPackets.h"
#include "pe/Enet/Packets/ImGuiDrawDataPacket.h"
#include <cmath>
#include <stdlib.h>

extern "C" {
#include "switch/arm/counter.h"
#include "switch/kernel/svc.h"
#include "switch/runtime/diag.h"
#include "switch/services/hid.h"
}

#include "imgui.h"

static pe::enet::Server* server = nullptr;

static void no_memory() {
    nxdb::log("ENet ran out of memory", 0);
    diagAbortWithResult(0x0);
}

constexpr u64 sFrameRate = 60;
const u64 sFrameTimeTicks = (1.0 / sFrameRate) * armGetSystemTickFreq();

float a = 0.0f;

void render() {

    HidMouseState mouseState { 0 };
    hidGetMouseStates(&mouseState, 1);
    ImGuiIO& io = ImGui::GetIO();

    float multiplier = io.DisplaySize.x / 1280.f;
    io.AddMousePosEvent(mouseState.x * multiplier, mouseState.y * multiplier);
    if (mouseState.wheel_delta_x != 0)
        io.AddMouseWheelEvent(0.0f, mouseState.wheel_delta_x > 0 ? 5 : -5);

    constexpr int mouse_mapping[][2] = {
        { ImGuiMouseButton_Left, static_cast<const int>(HidMouseButton_Left) },
        { ImGuiMouseButton_Right, static_cast<const int>(HidMouseButton_Right) },
        { ImGuiMouseButton_Middle, static_cast<const int>(HidMouseButton_Middle) },
    };

    static u32 sLastMouseButtons = 0;

    for (auto [im_k, nx_k] : mouse_mapping) {
        if (mouseState.buttons & nx_k)
            io.AddMouseButtonEvent((ImGuiMouseButton)im_k, true);
        else if (sLastMouseButtons & nx_k)
            io.AddMouseButtonEvent((ImGuiMouseButton)im_k, false);
    }

    sLastMouseButtons = mouseState.buttons;

    io.MouseDrawCursor = true;

    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();
    ImGui::ShowDemoWindow();
    ImGui::GetForegroundDrawList()->AddRectFilled({ 0, 200 }, { 200, 200 + (sinf(a) + 1) * 100.0f }, IM_COL32(255, 0, 0, 255));
    a += 0.1f;

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

    hidInitializeMouse();
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
        u64 ns = armTicksToNs(sFrameTimeTicks - diff);
        if (ns < 1e+9)
            svcSleepThread(ns);
        lastFrame = armGetSystemTick();
    }
}
