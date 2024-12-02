#include "pe/DrawData.h"
#include "../../../common.h"
#include <cstdint>
#include <cstring>

namespace pe {
    static ImDrawData* sDrawDataPrev = nullptr;
    static ImDrawData* sDrawData = nullptr;

    ImDrawData* unpackDrawData(void* data, size_t size, ImGuiMouseCursor* outCursor) {
        uintptr_t cursor = 0;
        const auto getDataAtCursor = [&]<typename Type>(Type) {
            return reinterpret_cast<Type*>(uintptr_t(data) + cursor);
        };
        const auto read = [&]<typename ReadType>(ReadType) -> ReadType {
            ReadType instance;
            std::memcpy(&instance, getDataAtCursor(ReadType()), sizeof(ReadType));
            cursor += sizeof(ReadType);
            return instance;
        };

        const ImDrawDataPackHeader header = read(ImDrawDataPackHeader());
        *outCursor = header.curCursor;

        if (sDrawData == nullptr) {
            sDrawData = new ImDrawData;
        } else {
            for (ImDrawList* cmdList : sDrawData->CmdLists)
                delete cmdList;
        }

        auto& displaySize = ImGui::GetIO().DisplaySize;

        sDrawData->CmdListsCount = header.CmdListsCount;
        sDrawData->CmdLists.resize(header.CmdListsCount);
        for (int cmdListIdx = 0; cmdListIdx < header.CmdListsCount; cmdListIdx++) {
            sDrawData->CmdLists.Data[cmdListIdx] = new ImDrawList(nullptr);
            ImDrawList* list = sDrawData->CmdLists[cmdListIdx];
            list->_Data = ImGui::GetDrawListSharedData();

            int cmdBufferSize = read(int());
            int vtxBufferSize = read(int());
            int idxBufferSize = read(int());

            if (cmdBufferSize == 0 && vtxBufferSize == 0 && idxBufferSize == 0 && sDrawDataPrev && cmdListIdx < sDrawDataPrev->CmdLists.size()) {
                ImDrawList* srcList = sDrawDataPrev->CmdLists[cmdListIdx];
                list->CmdBuffer = srcList->CmdBuffer;
                list->VtxBuffer = srcList->VtxBuffer;
                list->IdxBuffer = srcList->IdxBuffer;
                list->Flags = srcList->Flags;
                continue;
            }

            list->CmdBuffer.resize(cmdBufferSize);
            list->VtxBuffer.resize(vtxBufferSize);
            list->IdxBuffer.resize(idxBufferSize);

            for (int cmdIdx = 0; cmdIdx < cmdBufferSize; cmdIdx++) {
                PackedImDrawCmd srcCmd = read(PackedImDrawCmd());
                auto& cmd = list->CmdBuffer[cmdIdx];
                auto& srcRect = srcCmd.ClipRect;
                cmd.ClipRect = {
                    srcRect.x1 / PackedImDrawCmd::sBoundClip * displaySize.x,
                    srcRect.y1 / PackedImDrawCmd::sBoundClip * displaySize.y,
                    srcRect.x2 / PackedImDrawCmd::sBoundClip * displaySize.x,
                    srcRect.y2 / PackedImDrawCmd::sBoundClip * displaySize.y,
                };
                cmd.VtxOffset = srcCmd.VtxOffset;
                cmd.IdxOffset = srcCmd.IdxOffset;
                cmd.ElemCount = srcCmd.ElemCount;
                cmd.ElemCount = srcCmd.ElemCount;
                cmd.TextureId = ImGui::GetIO().Fonts->TexID;
                cmd.UserCallbackData = nullptr;
                cmd.UserCallback = nullptr;
                cmd.UserCallbackDataOffset = 0;
                cmd.UserCallbackDataSize = 0;
            }

            for (int i = 0; i < vtxBufferSize; i++) {
                auto& dstVtx = list->VtxBuffer.Data[i];
                PackedImDrawVert vert = read(PackedImDrawVert());
                dstVtx.col = vert.col;
                dstVtx.pos = {
                    vert.posX / PackedImDrawVert::sBoundPos * displaySize.x,
                    vert.posY / PackedImDrawVert::sBoundPos * displaySize.y,
                };
                dstVtx.uv = {
                    vert.uvX / PackedImDrawVert::sBoundUv,
                    vert.uvY / PackedImDrawVert::sBoundUv,
                };
            }

            std::memcpy(list->IdxBuffer.Data, getDataAtCursor(ImDrawIdx()), sizeof(ImDrawIdx) * idxBufferSize);
            cursor += sizeof(ImDrawIdx) * idxBufferSize;
        }

        {
            if (sDrawDataPrev)
                sDrawDataPrev->CmdLists.clear_delete();
            else
                sDrawDataPrev = new ImDrawData;

            sDrawDataPrev->CmdLists.resize(sDrawData->CmdListsCount);
            for (int cmdListIdx = 0; cmdListIdx < sDrawData->CmdListsCount; cmdListIdx++) {
                sDrawDataPrev->CmdLists.Data[cmdListIdx] = new ImDrawList(nullptr);
                ImDrawList* list = sDrawDataPrev->CmdLists[cmdListIdx];
                ImDrawList* srcList = sDrawData->CmdLists[cmdListIdx];
                list->_Data = ImGui::GetDrawListSharedData();
                list->CmdBuffer = srcList->CmdBuffer;
                list->VtxBuffer = srcList->VtxBuffer;
                list->IdxBuffer = srcList->IdxBuffer;
                list->Flags = srcList->Flags;
            }
        }

        return sDrawData;
    }

} // namespace pe
