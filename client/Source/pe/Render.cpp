#include "pe/Render.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "pe/Client.h"
#include "pe/DrawData.h"
#include "pe/NetworkInput.h"
#include "pe/Util.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdio.h>
#include <thread>
#define GL_SILENCE_DEPRECATION
#include "../../../common.h"

namespace pe {

    static GLFWwindow* sWindow = nullptr;

    GLFWwindow* getGlfwWindow() { return sWindow; }

    static void glfw_error_callback(int error, const char* description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    static bool sWindowInited = false;

    static void initWindow() {
        glfwSetErrorCallback(glfw_error_callback);
        assert(glfwInit());

        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

        sWindow = glfwCreateWindow(1280, 720, "nxdbg", nullptr, nullptr);
        assert(sWindow);
        glfwMakeContextCurrent(sWindow);
        glfwSwapInterval(1);

        ImGui::StyleColorsDark();

        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void render(ImDrawData* data) {
        if (!sWindowInited) {
            initWindow();
            initNetworkInput();
            sWindowInited = true;
        }

        if (glfwWindowShouldClose(sWindow))
            nxdb::Client::instance()->kill();

        ImGuiIO& io = ImGui::GetIO();

        if (!data)
            return;

        glfwPollEvents();
        if (glfwGetWindowAttrib(sWindow, GLFW_ICONIFIED) != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return;
        }

        ImGui_ImplOpenGL3_NewFrame();

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        float display_w = io.DisplaySize.x, display_h = io.DisplaySize.y;

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        data->DisplaySize = { display_w, display_h };
        data->FramebufferScale = { 1, 1 };

        ImGui_ImplOpenGL3_RenderDrawData(data);
        glfwSwapBuffers(sWindow);
    }

    void handlePackedDrawData(void* data, size_t size) {
        ImGuiMouseCursor cursor;
        auto* drawData = unpackDrawData(data, size, &cursor);

        if (drawData) {
            render(drawData);
            updateMouseCursor(cursor);
        }
    }

    struct ImGui_ImplOpenGL3_Data {
        GLuint GlVersion; // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
        char GlslVersionString[32]; // Specified by user or detected based on compile time GL settings.
        bool GlProfileIsES2;
        bool GlProfileIsES3;
        bool GlProfileIsCompat;
        GLint GlProfileMask;
        GLuint FontTexture;
        GLuint ShaderHandle;
        GLint AttribLocationTex; // Uniforms location
        GLint AttribLocationProjMtx;
        GLuint AttribLocationVtxPos; // Vertex attributes location
        GLuint AttribLocationVtxUV;
        GLuint AttribLocationVtxColor;
        unsigned int VboHandle, ElementsHandle;
        GLsizeiptr VertexBufferSize;
        GLsizeiptr IndexBufferSize;
        bool HasPolygonMode;
        bool HasClipOrigin;
        bool UseBufferSubData;

        ImGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
    };

    static ImGui_ImplOpenGL3_Data* ImGui_ImplOpenGL3_GetBackendData() {
        return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL3_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
    }

    void loadFontTextureRGBA32(void* data, int width, int height) {
        printf("init font\n");
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

        unsigned char* pixels = reinterpret_cast<unsigned char*>(data);

        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGenTextures(1, &bd->FontTexture);
        glBindTexture(GL_TEXTURE_2D, bd->FontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->FontTexture);

        glBindTexture(GL_TEXTURE_2D, last_texture);
    }

} // namespace pe
