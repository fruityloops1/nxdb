#include "ImGuiLoop.h"
#include "Component.h"
#include "DrawDataPack.h"
#include "LogServer.h"
#include "Server.h"
#include "TimeProfiler.h"
#include "imgui_internal.h"
#include "windows/MainWindow.h"
#include <cmath>
#include <cstring>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

extern "C" {
#include "switch/arm/counter.h"
#include "switch/kernel/svc.h"
#include "switch/runtime/diag.h"
#include "switch/services/hid.h"
}

#include "hash.h"
#include "imgui.h"

namespace nxdb {

    static bool sRerender = true;

    static u32 calcDrawDataHash(const ImDrawData* data) {
        u32 hash = 0;
        for (ImDrawList* cmdList : data->CmdLists)
            hash += hashImDrawList(cmdList);
        return hash;
    }

    constexpr u64 sFrameRate = 60;
    const u64 sFrameTimeTicks = (1.0 / sFrameRate) * armGetSystemTickFreq();

    static u8 sDrawDataBuffer[512_KB];
    static std::string sCurClipboardString;
    static nxdb::Server sServer;

    static void showImGui() {

        const ImGuiWindowFlags dockWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* main = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(main->WorkPos);
        ImGui::SetNextWindowSize(main->WorkSize);
        ImGui::SetNextWindowViewport(main->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("sdffksdg", nullptr, dockWindowFlags);

        ImGui::PopStyleVar();

        ImGui::DockSpace(ImGui::GetID("MainDockspace"), { 0, 0 }, ImGuiDockNodeFlags_None);

        Component::getSharedData().handleDeadComponents();
        for (Component* component : Component::getSharedData().components)
            component->update();

        ImGui::End();

        // ImGui::GetForegroundDrawList()->AddRectFilled({ 0, 400 }, { 400, 400 + std::sin(armGetSystemTick() / float(armGetSystemTickFreq()) * 3) * 50 }, IM_COL32(255, 0, 0, 255));
    }

    void render() {
        ImGuiIO& io = ImGui::GetIO();
        /*HidMouseState mouseState { 0 };
        hidGetMouseStates(&mouseState, 1);

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

        io.MouseDrawCursor = true;*/

        ImGui::NewFrame();

        showImGui();

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();

        /*if (data->CmdLists.size()) {
            auto& d = data->CmdLists[0]->CmdBuffer[1];
            nxdb::log("%d %d %d %llu %f\n", d.VtxOffset, d.IdxOffset, d.ElemCount, d.TextureId, data->CmdLists[0]->VtxBuffer[5].pos.y);
        }*/

        {
            static u32 lastDrawDataHash = 0;
            u32 drawDataHash = calcDrawDataHash(drawData);
            if (sRerender || lastDrawDataHash != drawDataHash) {
                sRerender = false;
                TimeProfiler clock("PackDrawData");
                size_t written = packImDrawData(sDrawDataBuffer, drawData);
                clock.print();

                clock = "SendDrawData";
                // whatever about the undefined behavior
                IM_ASSERT(written <= sizeof(sDrawDataBuffer));
                sServer.iterateClients([written](nxdb::Client& client) {
                    client.sendPacketImpl(PacketType::DrawData, sDrawDataBuffer, written);
                    if (!client.hasSentFontTexture())
                        client.sendFontTexture();
                });
                clock.print();

                size_t cmdCount = 0;
                for (auto& cmd : drawData->CmdLists) {
                    cmdCount += cmd->CmdBuffer.size();
                }
                nxdb::log("Packet size: %3.2fKB TotalVtxCount %zu TotalIdxCount %zu TotalCmdCount %zu", written / 1024.f, drawData->TotalVtxCount, drawData->TotalIdxCount, cmdCount);

                {
                    static u64 diffs[10];
                    static u64 count = 0;

                    if (count >= 10)
                        count = 0;

                    static u64 lastTick = 0;
                    u64 now = armGetSystemTick();
                    u64 diff = now - lastTick;
                    diffs[count++] = diff;

                    u64 avgDiff = 0;
                    for (int i = 0; i < 10; i++)
                        avgDiff += diffs[i];
                    avgDiff /= 10;

                    float fps = 1 / (float(avgDiff) / armGetSystemTickFreq());
                    // nxdb::log("FPS: %.2f", fps);

                    lastTick = now;
                }
            }
            lastDrawDataHash = drawDataHash;
        }
    }

    static void serverThread() {
        sServer.run();
    }

    static void initComponents() {
        auto& components = Component::getSharedData().components;

        components.push_back(new MainWindow);
    }

    void runImguiLoop() {

        hidInitializeMouse();

        sServer.start();
        std::thread a(serverThread);

        IMGUI_CHECKVERSION();

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        auto& io = ImGui::GetIO();
        io.DisplaySize = { 1280, 720 };
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.BackendPlatformName = "Horizon";
        io.BackendRendererName = "network";

        auto& platformIO = ImGui::GetPlatformIO();
        platformIO.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) {
            sServer.iterateClients([text](nxdb::Client& client) {
                size_t textSize = std::strlen(text) + sizeof('\0');
                client.sendPacketImpl(PacketType::SetClipboardText, text, textSize);
            });
        };
        platformIO.Platform_GetClipboardTextFn = [](ImGuiContext*) -> const char* { return sCurClipboardString.c_str(); };

        io.Fonts->AddFontFromFileTTF("font.otf", 16);

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

        initComponents();

        u64 lastFrame = armGetSystemTick();
        while (true) {
            if (sServer.getNumClients() != 0)
                render();

            u64 diff = armGetSystemTick() - lastFrame;
            u64 ns = armTicksToNs(sFrameTimeTicks - diff);
            if (ns < 1e+9)
                svcSleepThread(ns);
            io.DeltaTime = float(armGetSystemTick() - lastFrame) / armGetSystemTickFreq();

            lastFrame = armGetSystemTick();
        }
    }

    void queueRerender() {
        sRerender = true;
    }

    void setCurClipboardString(const char* text) {
        sCurClipboardString = text;
    }

} // namespace nxdb
