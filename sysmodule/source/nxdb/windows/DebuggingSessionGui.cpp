#include "DebuggingSession.h"
#include "imgui.h"

namespace nxdb {

    class ModuleList : public Component {
        DebuggingSession& mSession;

    public:
        ModuleList(DebuggingSession& session)
            : mSession(session) { }

        void update() override {
            ImGui::Begin("Module List");

            if (ImGui::BeginTable("modulelist", 3)) {

                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Start");
                ImGui::TableSetupColumn("End");
                ImGui::TableHeadersRow();

                for (int i = 0; i < mSession.mNumModules; i++) {
                    auto& mod = mSession.mModules[i];
                    ImGui::TableNextRow();

                    std::string modName(mod.mod0Name);
                    size_t offs = modName.rfind('\\');

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", offs != std::string::npos ? mod.mod0Name + offs : mod.mod0Name);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%016lx", mod.base);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%016lx", mod.base + mod.size);
                }
                ImGui::EndTable();
            }

            ImGui::End();
        }
    };

    void DebuggingSession::update() {
        char title[48];
        snprintf(title, sizeof(title), "Debugging %s - PID: %zu", mName, mProcessId);
        bool opened = true;
        ImGui::Begin(title, &opened);

        if (!opened)
            queueForDeletion();

        ImGui::Text("Modules: %d", mNumModules);
        ImGui::End();
    }

    void DebuggingSession::createComponents() {
        mModuleList = new ModuleList(*this);
        getSharedData().registerComponent(mModuleList);
    }

    void DebuggingSession::queueForDeletion() {
        Component::queueForDeletion();
        mModuleList->queueForDeletion();
    }

} // namespace nxdb
