#include "DrawDataPack.h"
#include "../../../common.h"
#include "types.h"
#include <algorithm>
#include <cstring>

namespace nxdb {

    static size_t calcImDrawDataSize(const ImDrawData* data) {
        size_t packetSize = 0;

        packetSize += sizeof(ImDrawDataPackHeader);

        for (ImDrawList* cmdList : data->CmdLists) {
            packetSize += sizeof(cmdList->CmdBuffer.size());
            packetSize += sizeof(cmdList->VtxBuffer.size());
            packetSize += sizeof(cmdList->IdxBuffer.size());
            packetSize += cmdList->CmdBuffer.size() * sizeof(PackedImDrawCmd);
            packetSize += cmdList->VtxBuffer.size() * sizeof(PackedImDrawVert);
            packetSize += cmdList->IdxBuffer.size() * sizeof(ImDrawIdx);
        }

        return packetSize;
    }

    void packImDrawData(void* out, size_t* outSize, ImDrawData* data) {
        const size_t packetSize = calcImDrawDataSize(data);
        if (outSize)
            *outSize = packetSize;
        if (out == nullptr)
            return;

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
            ImDrawList* cmdList = data->CmdLists[cmdListIdx];
            write(cmdList->CmdBuffer.size());
            write(cmdList->VtxBuffer.size());
            write(cmdList->IdxBuffer.size());

            for (int cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.size(); cmdIdx++) {
                auto& srcCmd = cmdList->CmdBuffer[cmdIdx];

                write(PackedImDrawCmd { {
                                            u16((srcCmd.ClipRect.x / displaySize.x) * PackedImDrawCmd::sBoundClip),
                                            u16((srcCmd.ClipRect.y / displaySize.y) * PackedImDrawCmd::sBoundClip),
                                            u16((srcCmd.ClipRect.z / displaySize.x) * PackedImDrawCmd::sBoundClip),
                                            u16((srcCmd.ClipRect.w / displaySize.y) * PackedImDrawCmd::sBoundClip),
                                        },
                    u16(srcCmd.VtxOffset), u16(srcCmd.IdxOffset), u16(srcCmd.ElemCount) });
            }

            for (int i = 0; i < cmdList->VtxBuffer.size(); i++) {
                auto& srcVtx = cmdList->VtxBuffer.Data[i];

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

            std::memcpy(getDataAtCursor(ImDrawIdx()), cmdList->IdxBuffer.Data, sizeof(ImDrawIdx) * cmdList->IdxBuffer.size());
            cursor += sizeof(ImDrawIdx) * cmdList->IdxBuffer.size();
        }
    }

} // namespace nxdb
