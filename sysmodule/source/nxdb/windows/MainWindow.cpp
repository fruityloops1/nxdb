#include "MainWindow.h"
#include "DebuggingSession.h"
#include "LogServer.h"
#include "imgui.h"

namespace nxdb {

    void MainWindow::update() {
        if (mFramesSinceLastProcessListUpdate > 360) {
            mNumProcesses = getProcessList(mProcessList, sMaxPids);
            mFramesSinceLastProcessListUpdate = 0;
        }
        mFramesSinceLastProcessListUpdate++;

        if (ImGui::Begin("nxdb")) {
            ImGui::Text("Hewwo");

            if (ImGui::BeginTable("processes", 3)) {
                ImGui::TableSetupColumn("PID");
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Program");
                ImGui::TableHeadersRow();

                for (int i = 0; i < mNumProcesses; i++) {
                    auto& p = mProcessList[i];
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%zu", p.processId);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Selectable(p.name, false, ImGuiSelectableFlags_SpanAllColumns);

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        getSharedData().registerComponent(new DebuggingSession(p.processId));
                    }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%016lX", p.programId);
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();
    }

} // namespace nxdb
