#include "pe/Enet/Request.h"

namespace pe {
    namespace enet {
        static RequestMgr sInstance;
        RequestMgr& RequestMgr::instance() { return sInstance; }

        bool RequestMgr::findAndCallEntry(u32 requestId, void* requestData)
        {
            for (int i = 0; i < mEntries.size(); i++) {
                auto& entry = mEntries[i];
                if (entry.requestId == requestId) {
                    entry.responseCallback->call(requestData);
                    delete entry.responseCallback;
                    mEntries.erase(mEntries.begin() + i);
                    return true;
                }
            }
            return false;
        }

    } // namespace enet
} // namespace pe
