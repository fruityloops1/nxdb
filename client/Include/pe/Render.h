#pragma once

#include "imgui.h"
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

namespace pe {

    GLFWwindow* getGlfwWindow();
    void render(ImDrawData* data);
    void handlePackedDrawData(void* data, size_t size);
    void loadFontTextureRGBA32(void* data, int width, int height);

} // namespace pesDictFile
