#include "DrawDataPack.h"
#include "../../../common.h"
#include "hash.h"
#include "types.h"
#include <algorithm>
#include <cstring>

namespace nxdb {

    constexpr int sMaxDrawLists = 128;
    static u32 sPrevDrawListHashes[sMaxDrawLists] { 0 };
    static int sNumPrevDrawLists = 0;

    size packImDrawData(void* out, ImDrawData* data) {
        uintptr_t cursor = 0;

        const auto getDataAtCursor = [&]<typename Type>(Type) {
            return reinterpret_cast<Type*>(uintptr_t(out) + cursor);
        };
        const auto write = [&]<typename WriteType>(const WriteType& value) {
            std::memcpy(getDataAtCursor(WriteType()), &value, sizeof(WriteType));
            cursor += sizeof(WriteType);
        };

        ImDrawDataPackHeader header {
            ImGui::GetMouseCursor(), data->CmdListsCount, data->TotalVtxCount, data->TotalIdxCount
        };

        write(header);

        const auto& displaySize = ImGui::GetIO().DisplaySize;

        for (int cmdListIdx = 0; cmdListIdx < data->CmdListsCount; cmdListIdx++) {
            ImDrawList* drawList = data->CmdLists[cmdListIdx];
            u32 hash = hashImDrawList(drawList);
            u32 prevHash = sPrevDrawListHashes[cmdListIdx];

            if (hash == prevHash) {
                write(int(0));
                write(int(0));
                write(int(0));
                continue;
            }

            sPrevDrawListHashes[cmdListIdx] = hash;

            write(drawList->CmdBuffer.size());
            write(drawList->VtxBuffer.size());
            write(drawList->IdxBuffer.size());

            for (int cmdIdx = 0; cmdIdx < drawList->CmdBuffer.size(); cmdIdx++) {
                auto& srcCmd = drawList->CmdBuffer[cmdIdx];

                write(PackedImDrawCmd { {
                                            u16((srcCmd.ClipRect.x / displaySize.x) * PackedImDrawCmd::sBoundClip),
                                            u16((srcCmd.ClipRect.y / displaySize.y) * PackedImDrawCmd::sBoundClip),
                                            u16((srcCmd.ClipRect.z / displaySize.x) * PackedImDrawCmd::sBoundClip),
                                            u16((srcCmd.ClipRect.w / displaySize.y) * PackedImDrawCmd::sBoundClip),
                                        },
                    u16(srcCmd.VtxOffset), u16(srcCmd.IdxOffset), u16(srcCmd.ElemCount) });
            }

            for (int i = 0; i < drawList->VtxBuffer.size(); i++) {
                auto& srcVtx = drawList->VtxBuffer.Data[i];

                PackedImDrawVert vert;
                vert.col = srcVtx.col;
                srcVtx.pos.x = std::clamp(srcVtx.pos.x, 0.0f, displaySize.x);
                srcVtx.pos.y = std::clamp(srcVtx.pos.y, 0.0f, displaySize.y);
                vert.posX = (srcVtx.pos.x / displaySize.x) * PackedImDrawVert::sBoundPos;
                vert.posY = (srcVtx.pos.y / displaySize.y) * PackedImDrawVert::sBoundPos;
                vert.uvX = srcVtx.uv.x * PackedImDrawVert::sBoundUv;
                vert.uvY = srcVtx.uv.y * PackedImDrawVert::sBoundUv;
                write(vert);
            }

            std::memcpy(getDataAtCursor(ImDrawIdx()), drawList->IdxBuffer.Data, sizeof(ImDrawIdx) * drawList->IdxBuffer.size());
            cursor += sizeof(ImDrawIdx) * drawList->IdxBuffer.size();
        }

        sNumPrevDrawLists = data->CmdListsCount;

        return size(cursor);
    }

    u32 hashImDrawList(ImDrawList* list) {
        u32 hash = 0;
        hash += hk::util::hashMurmur(reinterpret_cast<u8*>(list->VtxBuffer.Data), list->VtxBuffer.size_in_bytes());
        hash += hk::util::hashMurmur(reinterpret_cast<u8*>(list->IdxBuffer.Data), list->IdxBuffer.size_in_bytes());
        hash += hk::util::hashMurmur(reinterpret_cast<u8*>(list->CmdBuffer.Data), list->CmdBuffer.size_in_bytes());
        return hash;
    }

} // namespace nxdb
