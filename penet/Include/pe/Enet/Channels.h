#pragma once

#include "pe/Enet/IPacket.h"
#include "pe/Enet/Impls.h"
#include "pe/Enet/Packets/DataPackets.h"
#include "pe/Enet/Packets/ImGuiDrawDataPacket.h"
#ifndef NNSDK
#include <new>
#endif

namespace pe {
    namespace enet {

#define PENET_CHANNEL(CLASS)                                      \
    {                                                             \
        channels.createFuncs[i] = readPacket<CLASS>;              \
        CLASS packet;                                             \
        channels.vtables[i] = *reinterpret_cast<void**>(&packet); \
        channels.packetNames[i] = #CLASS;                         \
        i++;                                                      \
    }

#define PENET_CHANNELS       \
    PENET_CHANNEL(ToS_Hello) \
    PENET_CHANNEL(Test)      \
    PENET_CHANNEL(ImGuiDrawDataPacket)

        /*
         *
         *
         *
         *
         *
         *
         *
         *
         * Channel Implementation
         * im putting this big comment so the stuff above is seperated and i dont get
         * information overload from looking at the stuff below this
         *
         *
         *
         *
         *
         *
         *
         *
         */

        using ReadPacketType = IPacket* (*)(const void* data, size_t len);
        template <typename T>
        IPacket* readPacket(const void* data, size_t len) {
            static_assert(std::is_base_of<IPacket, T>::value,
                "T must derive from IPacket");

            T* packet = (T*)PENET_MALLOC(sizeof(T));
            new (packet) T;
            packet->read(data, len);
            return packet;
        }

        static const auto createChannels() {
            int i = 0;
            struct {
                ReadPacketType createFuncs[255] { nullptr };
                void* vtables[255] { nullptr };
                const char* packetNames[255] { nullptr };
                int channelCount;
            } channels;

            PENET_CHANNELS

            channels.channelCount = i;
            return channels;
        }
        const auto sChannels = createChannels();

#undef PENET_CHANNEL
#define PENET_CHANNEL(CLASS) CLASS,

        enum class ChannelType : u8
        {
            PENET_CHANNELS
        };

#undef PENET_CHANNEL
#undef PENET_CHANNELS

        ChannelType identifyType(const IPacket* packet);

    } // namespace enet
} // namespace pe
