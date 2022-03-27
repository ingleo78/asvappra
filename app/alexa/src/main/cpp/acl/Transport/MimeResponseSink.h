#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MIMERESPONSESINK_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MIMERESPONSESINK_H_

#include <memory>
#include <avs/attachment/AttachmentManager.h>
#include <avs/attachment/AttachmentWriter.h>
#include <util/HTTP2/HTTP2MimeResponseSinkInterface.h>
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
        class MimeResponseSink : public HTTP2MimeResponseSinkInterface {
        public:
            MimeResponseSink(shared_ptr<MimeResponseStatusHandlerInterface> handler, shared_ptr<MessageConsumerInterface> messageConsumer,
                             shared_ptr<AttachmentManager> attachmentManager, string attachmentContextId);
            virtual ~MimeResponseSink() = default;
            bool onReceiveResponseCode(long responseCode) override;
            bool onReceiveHeaderLine(const string& line) override;
            bool onBeginMimePart(const multimap<string, string>& headers) override;
            HTTP2ReceiveDataStatus onReceiveMimeData(const char* bytes, size_t size) override;
            bool onEndMimePart() override;
            HTTP2ReceiveDataStatus onReceiveNonMimeData(const char* bytes, size_t size) override;
            void onResponseFinished(HTTP2ResponseFinishedStatus status) override;
        private:
            enum ContentType {
                NONE,
                JSON,
                ATTACHMENT
            };
            HTTP2ReceiveDataStatus writeToAttachment(const char* bytes, size_t size);
            shared_ptr<MimeResponseStatusHandlerInterface> m_handler;
            shared_ptr<MessageConsumerInterface> m_messageConsumer;
            shared_ptr<AttachmentManager> m_attachmentManager;
            ContentType m_contentType;
            string m_attachmentContextId;
            string m_directiveBeingReceived;
            string m_attachmentIdBeingReceived;
            unique_ptr<AttachmentWriter> m_attachmentWriter;
            string m_nonMimeBody;
        };
    }
}
#endif