#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_EXCHANGEHANDLERCONTEXTINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_EXCHANGEHANDLERCONTEXTINTERFACE_H_

#include <memory>
#include <string>
#include <avs/MessageRequest.h>
#include <util/HTTP2/HTTP2RequestConfig.h>
#include <util/HTTP2/HTTP2RequestInterface.h>

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace http2;
        class ExchangeHandlerContextInterface {
        public:
            virtual ~ExchangeHandlerContextInterface() = default;
            virtual void onDownchannelConnected() = 0;
            virtual void onDownchannelFinished() = 0;
            virtual void onMessageRequestSent(const shared_ptr<MessageRequest>& request) = 0;
            virtual void onMessageRequestTimeout() = 0;
            virtual void onMessageRequestAcknowledged(const shared_ptr<MessageRequest>& request) = 0;
            virtual void onMessageRequestFinished() = 0;
            virtual void onPingRequestAcknowledged(bool success) = 0;
            virtual void onPingTimeout() = 0;
            virtual void onActivity() = 0;
            virtual void onForbidden(const string& authToken = "") = 0;
            virtual shared_ptr<HTTP2RequestInterface> createAndSendRequest(const HTTP2RequestConfig& cfg) = 0;
            virtual string getAVSGateway() = 0;
        };
    }
}
#endif