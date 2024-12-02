#include "pe/NetworkInput.h"
#include "pe/Client.h"
#include "pe/Hash.h"
#include "pe/Render.h"
#include <cstdio>
#include <cstring>

namespace pe {

    static ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode) {
        IM_UNUSED(scancode);
        switch (keycode) {
        case GLFW_KEY_TAB:
            return ImGuiKey_Tab;
        case GLFW_KEY_LEFT:
            return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT:
            return ImGuiKey_RightArrow;
        case GLFW_KEY_UP:
            return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN:
            return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP:
            return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN:
            return ImGuiKey_PageDown;
        case GLFW_KEY_HOME:
            return ImGuiKey_Home;
        case GLFW_KEY_END:
            return ImGuiKey_End;
        case GLFW_KEY_INSERT:
            return ImGuiKey_Insert;
        case GLFW_KEY_DELETE:
            return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE:
            return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE:
            return ImGuiKey_Space;
        case GLFW_KEY_ENTER:
            return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE:
            return ImGuiKey_Escape;
        case GLFW_KEY_APOSTROPHE:
            return ImGuiKey_Apostrophe;
        case GLFW_KEY_COMMA:
            return ImGuiKey_Comma;
        case GLFW_KEY_MINUS:
            return ImGuiKey_Minus;
        case GLFW_KEY_PERIOD:
            return ImGuiKey_Period;
        case GLFW_KEY_SLASH:
            return ImGuiKey_Slash;
        case GLFW_KEY_SEMICOLON:
            return ImGuiKey_Semicolon;
        case GLFW_KEY_EQUAL:
            return ImGuiKey_Equal;
        case GLFW_KEY_LEFT_BRACKET:
            return ImGuiKey_LeftBracket;
        case GLFW_KEY_BACKSLASH:
            return ImGuiKey_Backslash;
        case GLFW_KEY_RIGHT_BRACKET:
            return ImGuiKey_RightBracket;
        case GLFW_KEY_GRAVE_ACCENT:
            return ImGuiKey_GraveAccent;
        case GLFW_KEY_CAPS_LOCK:
            return ImGuiKey_CapsLock;
        case GLFW_KEY_SCROLL_LOCK:
            return ImGuiKey_ScrollLock;
        case GLFW_KEY_NUM_LOCK:
            return ImGuiKey_NumLock;
        case GLFW_KEY_PRINT_SCREEN:
            return ImGuiKey_PrintScreen;
        case GLFW_KEY_PAUSE:
            return ImGuiKey_Pause;
        case GLFW_KEY_KP_0:
            return ImGuiKey_Keypad0;
        case GLFW_KEY_KP_1:
            return ImGuiKey_Keypad1;
        case GLFW_KEY_KP_2:
            return ImGuiKey_Keypad2;
        case GLFW_KEY_KP_3:
            return ImGuiKey_Keypad3;
        case GLFW_KEY_KP_4:
            return ImGuiKey_Keypad4;
        case GLFW_KEY_KP_5:
            return ImGuiKey_Keypad5;
        case GLFW_KEY_KP_6:
            return ImGuiKey_Keypad6;
        case GLFW_KEY_KP_7:
            return ImGuiKey_Keypad7;
        case GLFW_KEY_KP_8:
            return ImGuiKey_Keypad8;
        case GLFW_KEY_KP_9:
            return ImGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL:
            return ImGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE:
            return ImGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY:
            return ImGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT:
            return ImGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD:
            return ImGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER:
            return ImGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL:
            return ImGuiKey_KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT:
            return ImGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL:
            return ImGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT:
            return ImGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER:
            return ImGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT:
            return ImGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL:
            return ImGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT:
            return ImGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER:
            return ImGuiKey_RightSuper;
        case GLFW_KEY_MENU:
            return ImGuiKey_Menu;
        case GLFW_KEY_0:
            return ImGuiKey_0;
        case GLFW_KEY_1:
            return ImGuiKey_1;
        case GLFW_KEY_2:
            return ImGuiKey_2;
        case GLFW_KEY_3:
            return ImGuiKey_3;
        case GLFW_KEY_4:
            return ImGuiKey_4;
        case GLFW_KEY_5:
            return ImGuiKey_5;
        case GLFW_KEY_6:
            return ImGuiKey_6;
        case GLFW_KEY_7:
            return ImGuiKey_7;
        case GLFW_KEY_8:
            return ImGuiKey_8;
        case GLFW_KEY_9:
            return ImGuiKey_9;
        case GLFW_KEY_A:
            return ImGuiKey_A;
        case GLFW_KEY_B:
            return ImGuiKey_B;
        case GLFW_KEY_C:
            return ImGuiKey_C;
        case GLFW_KEY_D:
            return ImGuiKey_D;
        case GLFW_KEY_E:
            return ImGuiKey_E;
        case GLFW_KEY_F:
            return ImGuiKey_F;
        case GLFW_KEY_G:
            return ImGuiKey_G;
        case GLFW_KEY_H:
            return ImGuiKey_H;
        case GLFW_KEY_I:
            return ImGuiKey_I;
        case GLFW_KEY_J:
            return ImGuiKey_J;
        case GLFW_KEY_K:
            return ImGuiKey_K;
        case GLFW_KEY_L:
            return ImGuiKey_L;
        case GLFW_KEY_M:
            return ImGuiKey_M;
        case GLFW_KEY_N:
            return ImGuiKey_N;
        case GLFW_KEY_O:
            return ImGuiKey_O;
        case GLFW_KEY_P:
            return ImGuiKey_P;
        case GLFW_KEY_Q:
            return ImGuiKey_Q;
        case GLFW_KEY_R:
            return ImGuiKey_R;
        case GLFW_KEY_S:
            return ImGuiKey_S;
        case GLFW_KEY_T:
            return ImGuiKey_T;
        case GLFW_KEY_U:
            return ImGuiKey_U;
        case GLFW_KEY_V:
            return ImGuiKey_V;
        case GLFW_KEY_W:
            return ImGuiKey_W;
        case GLFW_KEY_X:
            return ImGuiKey_X;
        case GLFW_KEY_Y:
            return ImGuiKey_Y;
        case GLFW_KEY_Z:
            return ImGuiKey_Z;
        case GLFW_KEY_F1:
            return ImGuiKey_F1;
        case GLFW_KEY_F2:
            return ImGuiKey_F2;
        case GLFW_KEY_F3:
            return ImGuiKey_F3;
        case GLFW_KEY_F4:
            return ImGuiKey_F4;
        case GLFW_KEY_F5:
            return ImGuiKey_F5;
        case GLFW_KEY_F6:
            return ImGuiKey_F6;
        case GLFW_KEY_F7:
            return ImGuiKey_F7;
        case GLFW_KEY_F8:
            return ImGuiKey_F8;
        case GLFW_KEY_F9:
            return ImGuiKey_F9;
        case GLFW_KEY_F10:
            return ImGuiKey_F10;
        case GLFW_KEY_F11:
            return ImGuiKey_F11;
        case GLFW_KEY_F12:
            return ImGuiKey_F12;
        case GLFW_KEY_F13:
            return ImGuiKey_F13;
        case GLFW_KEY_F14:
            return ImGuiKey_F14;
        case GLFW_KEY_F15:
            return ImGuiKey_F15;
        case GLFW_KEY_F16:
            return ImGuiKey_F16;
        case GLFW_KEY_F17:
            return ImGuiKey_F17;
        case GLFW_KEY_F18:
            return ImGuiKey_F18;
        case GLFW_KEY_F19:
            return ImGuiKey_F19;
        case GLFW_KEY_F20:
            return ImGuiKey_F20;
        case GLFW_KEY_F21:
            return ImGuiKey_F21;
        case GLFW_KEY_F22:
            return ImGuiKey_F22;
        case GLFW_KEY_F23:
            return ImGuiKey_F23;
        case GLFW_KEY_F24:
            return ImGuiKey_F24;
        default:
            return ImGuiKey_None;
        }
    }

    static ImVec2 sLastDisplaySize;
    static ImVec2 sLastCursorPos;
    static u32 sLastClipboardStringHash = 0;
    static GLFWcursor* sMouseCursors[ImGuiMouseCursor_COUNT];

    static void sendMouseEvent(const ImVec2& cursorPos, int button = -1, bool pressed = false, float wheelX = 0, float wheelY = 0) {
        MouseEvent packet(cursorPos, button, pressed, wheelX, wheelY);
        const auto& displaySize = ImGui::GetIO().DisplaySize;

        if (cursorPos.x < 0 || cursorPos.y < 0 || cursorPos.x > displaySize.x || cursorPos.y > displaySize.y)
            return;
        nxdb::Client::instance()->sendPacket(PacketType::UpdateMouse, packet);
    }

    static void sendKeyEvent(ImGuiKey key, bool pressed) {
        auto* window = getGlfwWindow();

        KeyEvent packet;
        packet.mod.ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
        packet.mod.alt = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
        packet.mod.shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
        packet.mod.super = glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS);
        packet.key = key;
        packet.pressed = pressed;
        nxdb::Client::instance()->sendPacket(PacketType::UpdateKey, packet);
    }

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        ImVec2 cursorPos(mouse_x, mouse_y);

        sendMouseEvent(cursorPos, button, action == GLFW_PRESS);
    }

    static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        ImVec2 cursorPos(mouse_x, mouse_y);

        sendMouseEvent(cursorPos, -1, false, float(xoffset), float(yoffset));
    }

    void keyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods) {
        if (action != GLFW_PRESS && action != GLFW_RELEASE)
            return;

        ImGuiKey imguiKey = ImGui_ImplGlfw_KeyToImGuiKey(keycode, scancode);
        sendKeyEvent(imguiKey, action == GLFW_PRESS);
    }

    void charCallback(GLFWwindow* window, unsigned int c) {
        nxdb::Client::instance()->sendPacket(PacketType::UpdateChar, c);
    }

    void initNetworkInput() {
        glfwSetMouseButtonCallback(getGlfwWindow(), mouseButtonCallback);
        glfwSetScrollCallback(getGlfwWindow(), mouseScrollCallback);
        glfwSetKeyCallback(getGlfwWindow(), keyCallback);
        glfwSetCharCallback(getGlfwWindow(), charCallback);

        auto& platformIO = ImGui::GetPlatformIO();

        sMouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        sMouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        sMouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        sMouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        sMouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
        sMouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        sMouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        sMouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        sMouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
        sMouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        sMouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        sMouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        sMouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
    }

    void updateNetworkInput() {
        ImGuiIO& io = ImGui::GetIO();
        auto* window = getGlfwWindow();

        if (window == nullptr)
            return;
        glfwPollEvents();

        int w, h;
        int display_w, display_h;
        glfwGetWindowSize(window, &w, &h);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        io.DisplaySize = ImVec2((float)w, (float)h);
        // if (w > 0 && h > 0)
        //    io.DisplayFramebufferScale = ImVec2((float)display_w / (float)w, (float)display_h / (float)h);

        if (sLastDisplaySize.x != io.DisplaySize.x || sLastDisplaySize.y != io.DisplaySize.y) {
            nxdb::Client::instance()->sendPacket(PacketType::UpdateDisplaySize, io.DisplaySize);
        }

        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        ImVec2 cursorPos(mouse_x, mouse_y);

        if (sLastCursorPos.x != cursorPos.x || sLastCursorPos.y != cursorPos.y) {
            sendMouseEvent(cursorPos);
        }

        const char* clipboard = glfwGetClipboardString(NULL);
        u32 clipboardStringHash = clipboard ? hk::util::hashMurmur(clipboard) : 0;

        if (clipboardStringHash != sLastClipboardStringHash) {
            size_t textSize = std::strlen(clipboard) + sizeof('\0');
            if (textSize <= 512)
                nxdb::Client::instance()->sendPacketImpl(PacketType::SetClipboardText, clipboard, textSize);
        }

        sLastClipboardStringHash = clipboardStringHash;
        sLastCursorPos = cursorPos;
        sLastDisplaySize = io.DisplaySize;
    }

    void updateMouseCursor(ImGuiMouseCursor cursor) {
        auto* window = getGlfwWindow();
        if (cursor == ImGuiMouseCursor_None)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        else {
            glfwSetCursor(window, sMouseCursors[cursor] ? sMouseCursors[cursor] : sMouseCursors[ImGuiMouseCursor_Arrow]);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

} // namespace pe
