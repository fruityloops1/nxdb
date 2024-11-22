#pragma once

#include "imgui.h"
#include "pe/Enet/IPacket.h"
#include "pe/Enet/Impls.h"
#include <cstring>
#include <type_traits>

namespace pe::enet {

    struct ImDrawCmd {
        ImVec4 ClipRect; // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract ImDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
        unsigned int VtxOffset; // 4    // Start offset in vertex buffer. ImGuiBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
        unsigned int IdxOffset; // 4    // Start offset in index buffer.
        unsigned int ElemCount; // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee ImDrawList's vtx_buffer[]
    };

    class ImGuiDrawDataPacket : public IPacket {
        ImDrawData* mData = nullptr;
        bool mDataOwned = false;

        void destroy() {
            for (ImDrawList* list : mData->CmdLists)
                delete list;
            delete mData;
        }

    public:
        ImGuiDrawDataPacket() = default;
        ImGuiDrawDataPacket(ImDrawData* data)
            : mData(data)
            , mDataOwned(false) { }

        struct __attribute__((packed)) Header {
            int Length;
            int CmdListsCount;
            int TotalVtxCount;
            int TotalIdxCount;
        };

        size_t calcSize() const override {
            size_t packetSize = 0;

            packetSize += sizeof(Header);

            for (ImDrawList* cmdList : mData->CmdLists) {
                packetSize += sizeof(cmdList->CmdBuffer.size());
                packetSize += sizeof(cmdList->VtxBuffer.size());
                packetSize += sizeof(cmdList->IdxBuffer.size());
                packetSize += cmdList->CmdBuffer.size() * sizeof(ImDrawCmd);
                packetSize += cmdList->VtxBuffer.size() * sizeof(ImDrawVert);
                packetSize += cmdList->IdxBuffer.size() * sizeof(ImDrawIdx);
            }

            return packetSize;
        }

        void build(void* outData) const override {
            uintptr_t cursor = 0;

            const auto getDataAtCursor = [&]<typename Type>(Type) {
                return reinterpret_cast<Type*>(uintptr_t(outData) + cursor);
            };
            const auto write = [&]<typename WriteType>(const WriteType& value) {
                std::memcpy(getDataAtCursor(WriteType()), &value, sizeof(WriteType));
                cursor += sizeof(WriteType);
            };

            Header header {
                static_cast<int>(calcSize()), mData->CmdListsCount, mData->TotalVtxCount, mData->TotalIdxCount
            };

            write(header);

            for (int cmdListIdx = 0; cmdListIdx < mData->CmdListsCount; cmdListIdx++) {
                ImDrawList* cmdList = mData->CmdLists[cmdListIdx];
                write(cmdList->CmdBuffer.size());
                write(cmdList->VtxBuffer.size());
                write(cmdList->IdxBuffer.size());

                for (int cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.size(); cmdIdx++) {
                    auto& srcCmd = cmdList->CmdBuffer[cmdIdx];

                    write(ImDrawCmd { srcCmd.ClipRect, srcCmd.VtxOffset, srcCmd.IdxOffset, srcCmd.ElemCount });
                }

                std::memcpy(getDataAtCursor(ImDrawVert()), cmdList->VtxBuffer.Data, sizeof(ImDrawVert) * cmdList->VtxBuffer.size());
                cursor += sizeof(ImDrawVert) * cmdList->VtxBuffer.size();
                std::memcpy(getDataAtCursor(ImDrawIdx()), cmdList->IdxBuffer.Data, sizeof(ImDrawIdx) * cmdList->IdxBuffer.size());
                cursor += sizeof(ImDrawIdx) * cmdList->IdxBuffer.size();
            }
        }

        void read(const void* data, size_t len) override {

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

            const Header header = read(Header());

            if (header.Length != len) {
                PENET_WARN("Corrupted packet %d!=%zu\n", header.Length, len);
                return;
            }

            if (mData == nullptr) {
                mData = new ImDrawData;
                mDataOwned = true;
            }

            mData->CmdListsCount = header.CmdListsCount;
            mData->CmdLists.resize(header.CmdListsCount);
            for (int cmdListIdx = 0; cmdListIdx < header.CmdListsCount; cmdListIdx++) {
                mData->CmdLists.Data[cmdListIdx] = new ImDrawList(nullptr);
                ImDrawList* list = mData->CmdLists[cmdListIdx];
                list->_Data = ImGui::GetDrawListSharedData();

                int cmdBufferSize = read(int());
                int vtxBufferSize = read(int());
                int idxBufferSize = read(int());

                list->CmdBuffer.resize(cmdBufferSize);
                list->VtxBuffer.resize(vtxBufferSize);
                list->IdxBuffer.resize(idxBufferSize);

                for (int cmdIdx = 0; cmdIdx < cmdBufferSize; cmdIdx++) {
                    ImDrawCmd srcCmd = read(ImDrawCmd());
                    auto& cmd = list->CmdBuffer[cmdIdx];
                    cmd.ClipRect = srcCmd.ClipRect;
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

                std::memcpy(list->VtxBuffer.Data, getDataAtCursor(ImDrawVert()), sizeof(ImDrawVert) * vtxBufferSize);
                cursor += sizeof(ImDrawVert) * vtxBufferSize;
                std::memcpy(list->IdxBuffer.Data, getDataAtCursor(ImDrawIdx()), sizeof(ImDrawIdx) * idxBufferSize);
                cursor += sizeof(ImDrawIdx) * idxBufferSize;
            }

            if (cursor != len) {
                // PENET_ABORT("kys %zu %zu", cursor, len);
                PENET_WARN("Corrupted packet %zu!=%zu\n", cursor, len);
                destroy();
                mData = nullptr;
            }
        }

        ~ImGuiDrawDataPacket() {
            if (mData && mDataOwned) {
                destroy();
            }
        }

        ImDrawData* getDrawData() { return mData; }
    };

} // namespace pe::enet
