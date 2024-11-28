#pragma once

#include "pe/Enet/IPacket.h"
#include <cstring>

namespace pe::enet {

    class MemorySubscriptionData : public IPacket {
        u64 mId = 0;
        u8* mData = nullptr;
        size_t mSize = 0;
        enum
        {
            None,
            New,
        } mAllocSource
            = None;

    public:
        MemorySubscriptionData() { }
        ~MemorySubscriptionData() {
            if (mData && mAllocSource == New)
                delete[] mData;
        }

        MemorySubscriptionData(u64 id, void* data, size_t size)
            : mId(id)
            , mData(reinterpret_cast<u8*>(data))
            , mSize(size)
            , mAllocSource(None) { }

        size_t calcSize() const override {
            return mSize + sizeof(mId);
        }

        size_t build(void* outData) const override {
            *reinterpret_cast<u64*>(outData) = mId;
            std::memcpy(reinterpret_cast<void*>(uintptr_t(outData) + sizeof(mId)), mData, mSize);

            return -1;
        }

        void read(const void* data, size_t len) override {
            mId = *reinterpret_cast<const u64*>(data);
            len -= sizeof(mId);

            mData = new u8[len];
            std::memcpy(mData, reinterpret_cast<const void*>(uintptr_t(data) + sizeof(mId)), len);
            mSize = len;
        }

        const u8* data() const { return mData; }
        size_t size() const { return mSize; }
        u64 id() const { return mId; }
    };

} // namespace pe::enet
