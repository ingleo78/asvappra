#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_DOWNCHANNELHANDLER_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_DOWNCHANNELHANDLER_H_

#include <memory>
#include <avs/attachment/AttachmentManager.h>
#include <util/HTTP2/HTTP2RequestSourceInterface.h>
#include "ExchangeHandler.h"
#include "MessageConsumerInterface.h"
#include "MimeResponseStatusHandlerInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace attachment;
        using namespace http2;
        class DownchannelHandler;
        class DownchannelHandler : public ExchangeHandler , public HTTP2RequestSourceInterface , public MimeResponseStatusHandlerInterface,
                                   public enable_shared_from_this<DownchannelHandler> {
        public:
            static shared_ptr<DownchannelHandler> create(
                shared_ptr<ExchangeHandlerContextInterface> context,
                const string& authToken,
                shared_ptr<MessageConsumerInterface> messageConsumer,
                shared_ptr<AttachmentManager> attachmentManager);
        private:
            DownchannelHandler(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken);
            vector<string> getRequestHeaderLines() override;
            HTTP2SendDataResult onSendData(char* bytes, size_t size) override;
            void onActivity() override;
            bool onReceiveResponseCode(long responseCode) override;
            void onResponseFinished(HTTP2ResponseFinishedStatus status, const string& nonMimeBody) override;
        };
    }
}
#endif