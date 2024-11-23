#include "pe/Render.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "pe/Enet/Hash.h"
#include "pe/Enet/Packets/ImGuiDrawDataPacket.h"
#include "pe/Util.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <thread>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

namespace pe {

    static GLFWwindow* window = nullptr;

    static void glfw_error_callback(int error, const char* description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    static bool inited = false;

    static void initWindow() {
        glfwSetErrorCallback(glfw_error_callback);
        assert(glfwInit());

        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

        window = glfwCreateWindow(1280, 720, "nxdbg", nullptr, nullptr);
        assert(window);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        ImGui::StyleColorsDark();

        // ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        /*while (!glfwWindowShouldClose(window)) {

            glfwPollEvents();
            if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                return;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();
            ImGui::Render();

            pe::enet::ImGuiDrawDataPacket packet(ImGui::GetDrawData());
            void* datsdfa = malloc(packet.calcSize());
            packet.build(datsdfa);
            char name[32];
            static int i = 0;
            static std::vector<u32> hashes;
            u32 hash = nxdb::util::hashMurmur((u8*)datsdfa, packet.calcSize());
            bool exists = false;
            for (int i = 0; i < hashes.size(); i++) {
                if (hashes[i] == hash) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                snprintf(name, sizeof(name), "fs/frame%d.bin", i++);
                FILE* f = fopen(name, "wb");
                fwrite(datsdfa, packet.calcSize(), 1, f);
                fclose(f);
                hashes.push_back(hash);
            }
            free(datsdfa);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }*/
    }

    /*class Sex {
    public:
        Sex() {
            {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = { 1280, 720 };
                io.DisplayFramebufferScale = { 1, 1 };
                (void)io;
                unsigned char* a;
                int w, h;
                io.Fonts->GetTexDataAsRGBA32(&a, &w, &h);
            }

            initWindow();
        }
    };
    Sex sexdsf;*/

    void render(ImDrawData* data) {
        if (!inited) {
            initWindow();
            inited = true;
        }

        if (!data)
            return;

        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return;
        }

        ImGui_ImplOpenGL3_NewFrame();

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        float display_w = 1280, display_h = 720;
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        data->DisplaySize = { display_w, display_h };
        data->FramebufferScale = { 1, 1 };

        ImGui_ImplOpenGL3_RenderDrawData(data);
        glfwSwapBuffers(window);
    }

    static ZSTD_DDict* sDDict = nullptr;
    static ZSTD_CDict* sCDict = nullptr;
    constexpr char sDictFile[] = "dictionary.zsdic";

    ZSTD_DDict* getZstdDDict() {
        if (sDDict == nullptr) {
            size_t dictSize;
            printf("loading dictionary %s\n", sDictFile);

            void* const dictBuffer = pe::readBytesFromFile(sDictFile, &dictSize);
            sDDict = ZSTD_createDDict(dictBuffer, dictSize);
            assert(sDDict != nullptr);
            free(dictBuffer);
        }
        return sDDict;
    }

    ZSTD_CDict* getZstdCDict() {
        if (sCDict == nullptr) {
            size_t dictSize;
            printf("loading dictionary %s\n", sDictFile);
            void* const dictBuffer = readBytesFromFile(sDictFile, &dictSize);
            sCDict = ZSTD_createCDict(dictBuffer, dictSize, 3);
            assert(sCDict != nullptr);
            free(dictBuffer);
        }
        return sCDict;
    }

} // namespace pe
