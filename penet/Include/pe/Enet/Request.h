#pragma once

#include "DataPacket.h"
#include "Types.h"
#include "pe/Enet/DataPacket.h"
#include <unordered_map>

namespace pe {
    namespace enet {

        struct IRequestFunctor {
            virtual void call(void* request) = 0;
            virtual ~IRequestFunctor() { }
        };

        template <typename Request, typename Lambda>
        struct RequestFunctor : IRequestFunctor {
            Lambda func;

            RequestFunctor(Lambda l)
                : func(l) { }
            ~RequestFunctor() override = default;

            virtual void call(void* request) {
                func(static_cast<Request*>(request));
            }
        };

        class RequestMgr {
            std::unordered_map<u32, IRequestFunctor*> mEntries;

        public:
            void registerEntry(u32 requestId, IRequestFunctor* callback);
            bool findAndCallEntry(u32 requestId, void* requestData);

            static RequestMgr& instance();
        };

        template <typename T>
        struct Request {
            using RequestType = T::Request;
            using ResponseType = T::Response;

            struct RequestPacketType : DataPacket<RequestPacketType> {
                u32 requestId;
                RequestType data;
            };

            struct ResponsePacketType : DataPacket<ResponsePacketType> {
                ResponsePacketType() = default;
                ResponsePacketType(u32 requestId)
                    : requestId(requestId) { }

                u32 requestId;
                ResponseType data;
            };

            template <typename ClientType>
            static void handleRequest(RequestPacketType* packet, ClientType* client) {
                T::handleRequest(&packet->data, client, packet->requestId);
            }

            static void handleResponse(ResponsePacketType* packet) {
                RequestMgr::instance().findAndCallEntry(packet->requestId, &packet->data);
            }

            using Req = RequestPacketType;
            using Res = ResponsePacketType;
        };

#define REQUEST_DEFINES(REQ)                 \
    using REQ = ::pe::enet::Request<REQ##_>; \
    using REQ##Req = REQ::Req;         \
    using REQ##Res = REQ::Res;

    } // namespace enet
} // namespace pe
