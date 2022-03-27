#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_PINGHANDLER_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_PINGHANDLER_H_

#include <memory>
#include <util/HTTP2/HTTP2RequestSourceInterface.h>
#include <util/HTTP2/HTTP2ResponseSinkInterface.h>
#include "ExchangeHandler.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace http2;
        class PingHandler : public ExchangeHandler, public HTTP2RequestSourceInterface, public HTTP2ResponseSinkInterface,
                            public enable_shared_from_this<PingHandler> {
        public:
            static shared_ptr<PingHandler> create(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken);
        private:
            PingHandler(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken);
            void reportPingAcknowledged();
            vector<string> getRequestHeaderLines() override;
            HTTP2SendDataResult onSendData(char* bytes, size_t size) override;
            bool onReceiveResponseCode(long responseCode) override;
            bool onReceiveHeaderLine(const string& line) override;
            HTTP2ReceiveDataStatus onReceiveData(const char* bytes, size_t size) override;
            void onResponseFinished(HTTP2ResponseFinishedStatus status) override;
            bool m_wasPingAcknowledgedReported;
            long m_responseCode;
        };
    }
}
#endif