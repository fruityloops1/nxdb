#include "ImGuiLoop.h"
#include "DrawDataPack.h"
#include "LogServer.h"
#include "Server.h"
#include "imgui_internal.h"
#include <cmath>
#include <cstring>
#include <stdlib.h>
#include <string>
#include <thread>

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
        for (ImDrawList* cmdList : data->CmdLists) {
            hash += hk::util::hashMurmur(reinterpret_cast<u8*>(cmdList->VtxBuffer.Data), cmdList->VtxBuffer.size_in_bytes());
            hash += hk::util::hashMurmur(reinterpret_cast<u8*>(cmdList->IdxBuffer.Data), cmdList->IdxBuffer.size_in_bytes());
            hash += hk::util::hashMurmur(reinterpret_cast<u8*>(cmdList->CmdBuffer.Data), cmdList->CmdBuffer.size_in_bytes());
        }
        return hash;
    }

    static nxdb::Server sServer;

    constexpr u64 sFrameRate = 60;
    const u64 sFrameTimeTicks = (1.0 / sFrameRate) * armGetSystemTickFreq();

    float a = 0.0f;

    static u8 sDrawDataBuffer[512_KB];
    static std::string sCurClipboardString;

    void render() {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = false;

        auto& platformIO = ImGui::GetPlatformIO();
        platformIO.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) {
            sServer.iterateClients([text](nxdb::Client& client) {
                size_t textSize = std::strlen(text) + sizeof('\0');
                client.sendPacketImpl(PacketType::SetClipboardText, text, textSize);
            });
        };
        platformIO.Platform_GetClipboardTextFn = [](ImGuiContext*) -> const char* { return sCurClipboardString.c_str(); };

        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();
        ImGui::ShowDemoWindow();
        // ImGui::GetForegroundDrawList()->AddRectFilled({ 0, 200 }, { 200, 200 + (sinf(a) + 1) * 100.0f }, IM_COL32(255, 0, 0, 255));
        a += 0.1f;

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
                size_t size;
                packImDrawData(nullptr, &size, drawData);
                IM_ASSERT(size <= sizeof(sDrawDataBuffer));
                packImDrawData(sDrawDataBuffer, nullptr, drawData);
                sServer.iterateClients([size](nxdb::Client& client) {
                    client.sendPacketImpl(PacketType::DrawData, sDrawDataBuffer, size);
                    if (!client.hasSentFontTexture())
                        client.sendFontTexture();
                });
                size_t cmdCount = 0;
                for (auto& cmd : drawData->CmdLists) {
                    cmdCount += cmd->CmdBuffer.size();
                }
                nxdb::log("Packet size: %3.2fKB TotalVtxCount %zu TotalIdxCount %zu TotalCmdCount %zu", size / 1024.f, drawData->TotalVtxCount, drawData->TotalIdxCount, cmdCount);

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
                    nxdb::log("FPS: %.2f", fps);

                    lastTick = now;
                }
            }
            lastDrawDataHash = drawDataHash;
        }
    }

    static void serverThread() {
        sServer.run();
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
            if (sServer.getNumClients() != 0)
                render();

            u64 diff = armGetSystemTick() - lastFrame;
            u64 ns = armTicksToNs(sFrameTimeTicks - diff);
            if (ns < 1e+9)
                svcSleepThread(ns);
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