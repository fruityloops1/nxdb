#include "pe/Render.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <chrono>
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

        ImGui_ImplOpenGL3_Init(glsl_version);
    }

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

} // namespace pe
