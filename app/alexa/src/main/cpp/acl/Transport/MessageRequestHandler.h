#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEREQUESTHANDLER_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEREQUESTHANDLER_H_

#include <memory>
#include <avs/attachment/AttachmentManager.h>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/EventTracerInterface.h>
#include <util/HTTP2/HTTP2MimeRequestSourceInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include "ExchangeHandler.h"
#include "MessageConsumerInterface.h"
#include "MimeResponseStatusHandlerInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace attachment;
        using namespace http2;
        using namespace metrics;
        class MessageRequestHandler : public ExchangeHandler, public HTTP2MimeRequestSourceInterface, public MimeResponseStatusHandlerInterface,
                                      public enable_shared_from_this<MessageRequestHandler> {
        public:
            ~MessageRequestHandler() override;
            static shared_ptr<MessageRequestHandler> create(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken,
                                                            shared_ptr<MessageRequest> messageRequest, shared_ptr<MessageConsumerInterface> messageConsumer,
                                                            shared_ptr<AttachmentManager> attachmentManager, shared_ptr<MetricRecorderInterface> metricRecorder,
                                                            shared_ptr<EventTracerInterface> eventTracer = nullptr);
            HTTP2GetMimeHeadersResult getMimePartHeaderLines() override;
            vector<string> getRequestHeaderLines() override;
            HTTP2SendDataResult onSendMimePartData(char* bytes, size_t size) override;
            void onActivity() override;
            bool onReceiveResponseCode(long responseCode) override;
            void onResponseFinished(HTTP2ResponseFinishedStatus status, const string& nonMimeBody) override;
        private:
            MessageRequestHandler(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken, shared_ptr<MessageRequest> messageRequest,
                                  shared_ptr<MetricRecorderInterface> metricRecorder);
            void reportMessageRequestAcknowledged();
            void reportMessageRequestFinished();
            shared_ptr<MessageRequest> m_messageRequest;
            string m_json;
            const char* m_jsonNext;
            size_t m_countOfJsonBytesLeft;
            size_t m_countOfPartsSent;
            shared_ptr<MessageRequest::NamedReader> m_namedReader;
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            bool m_wasMessageRequestAcknowledgeReported;
            bool m_wasMessageRequestFinishedReported;
            long m_responseCode;
        };
    }
}
#endif