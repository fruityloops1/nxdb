#pragma once

#include "imgui.h"

struct __attribute__((packed)) ImDrawDataPackHeader {
    ImGuiMouseCursor curCursor;
    int CmdListsCount;
    int TotalVtxCount;
    int TotalIdxCount;
};

enum PacketType
{
    NoPacket,
    Hello,
    Alive,
    FontTexture,
    DrawData,
    UpdateDisplaySize,
    UpdateMouse,
    UpdateKey,
    UpdateChar,
    SetClipboardText,
};

struct __attribute__((packed)) PackedImDrawCmd {
    struct __attribute__((packed)) { // quantized 0-1 * width/height
        unsigned short x1 : 12;
        unsigned short y1 : 12;
        unsigned short x2 : 12;
        unsigned short y2 : 12;
    } ClipRect;
    static constexpr float sBoundClip = 4095;
    unsigned short VtxOffset; // 2    // Start offset in vertex buffer. ImGuiBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned short IdxOffset; // 2    // Start offset in index buffer.
    unsigned short ElemCount; // 2    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee ImDrawList's vtx_buffer[]
};

struct __attribute__((packed)) PackedImDrawVert {
    unsigned short posX : 12;
    unsigned short posY : 12;
    // ^ quantized 0-1 * width/height
    unsigned short uvX : 12;
    unsigned short uvY : 12;
    // ^ quantized 0-1
    ImU32 col;

    static constexpr float sBoundPos = 4095;
    static constexpr float sBoundUv = 4095;
};

struct MouseEvent {
    ImVec2 pos;
    int button;
    bool pressed;
    float wheelX;
    float wheelY;
};

struct KeyEvent {
    struct {
        bool ctrl : 1;
        bool shift : 1;
        bool alt : 1;
        bool super : 1;
    } mod;
    ImGuiKey key;
    bool pressed;
};
