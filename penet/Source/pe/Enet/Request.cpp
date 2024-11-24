#include "pe/Enet/Request.h"

namespace pe {
    namespace enet {
        static RequestMgr sInstance;
        RequestMgr& RequestMgr::instance() { return sInstance; }

        void RequestMgr::registerEntry(u32 requestId, IRequestFunctor* callback)
        {
            mEntries[requestId] = callback;
        }

        bool RequestMgr::findAndCallEntry(u32 requestId, void* requestData)
        {
            if (mEntries.contains(requestId))
            {
                auto* callback = mEntries[requestId];
                callback->call(requestData);
                mEntries.erase(requestId);
                delete callback;
                return true;
            }
            return false;
        }

    } // namespace enet
} // namespace pe
