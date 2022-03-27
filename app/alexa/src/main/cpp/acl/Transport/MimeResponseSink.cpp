#include <logger/Logger.h>
#include "MockMessageRequest.h"
#include "MimeResponseSink.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace attachment;
        using namespace utils;
        using namespace logger;
        using namespace http2;
    #ifdef DEBUG
        static const char CR = 0x0D;
    #endif
        static const string X_AMZN_REQUESTID_PREFIX = "x-amzn-requestid:";
        static const string MIME_CONTENT_TYPE_FIELD_NAME = "Content-Type";
        static const string MIME_CONTENT_ID_FIELD_NAME = "Content-ID";
        static const string MIME_JSON_CONTENT_TYPE = "application/json";
        static const string MIME_OCTET_STREAM_CONTENT_TYPE = "application/octet-stream";
        static const size_t NON_MIME_BODY_MAX_SIZE = 4096;
        static const string TAG("MimeResponseSink");
        #define LX(event) LogEntry(TAG, event)
        static string sanitizeContentId(const string& mimeContentId) {
            string sanitizedContentId;
            if (mimeContentId.empty()) {
                ACSDK_ERROR(LX("sanitizeContentIdFailed").d("reason", "emptyMimeContentId"));
            } else if (('<' == mimeContentId.front()) && ('>' == mimeContentId.back())) sanitizedContentId = mimeContentId.substr(1,mimeContentId.size() - 2);
            else sanitizedContentId = mimeContentId;
            return sanitizedContentId;
        }
        MimeResponseSink::MimeResponseSink(shared_ptr<MimeResponseStatusHandlerInterface> handler, shared_ptr<MessageConsumerInterface> messageConsumer,
                                           shared_ptr<AttachmentManager> attachmentManager, string attachmentContextId) : m_handler{handler},
                                           m_messageConsumer{messageConsumer}, m_attachmentManager{attachmentManager},
                                           m_attachmentContextId{move(attachmentContextId)} {
            ACSDK_DEBUG9(LX(__func__).d("handler", handler.get()));
        }
        bool MimeResponseSink::onReceiveResponseCode(long responseCode) {
            ACSDK_DEBUG9(LX(__func__).d("responseCode", responseCode));
            if (m_handler) {
                m_handler->onActivity();
                return m_handler->onReceiveResponseCode(responseCode);
            }
            return false;
        }
        bool MimeResponseSink::onReceiveHeaderLine(const string& line) {
            ACSDK_DEBUG9(LX(__func__).d("line", line));
            if (m_handler) m_handler->onActivity();
        #ifdef DEBUG
            if (0 == line.find(X_AMZN_REQUESTID_PREFIX)) {
                auto end = line.find(CR);
                ACSDK_DEBUG0(LX("receivedRequestId").d("value", line.substr(0, end)));
            }
        #endif
            return true;
        }
        bool MimeResponseSink::onBeginMimePart(const multimap<string, string>& headers) {
            ACSDK_DEBUG9(LX(__func__));
            if (m_handler) m_handler->onActivity();
            auto it = headers.find(MIME_CONTENT_TYPE_FIELD_NAME);
            if (headers.end() == it) {
                ACSDK_WARN(LX("noContent-Type"));
                return true;
            }
            auto contentType = it->second;
            if (contentType.find(MIME_JSON_CONTENT_TYPE) != string::npos) {
                m_contentType = ContentType::JSON;
                ACSDK_DEBUG9(LX("JsonContentDetected"));
            } else if (m_attachmentManager && contentType.find(MIME_OCTET_STREAM_CONTENT_TYPE) != string::npos && 1 == headers.count(MIME_CONTENT_ID_FIELD_NAME)) {
                auto iy = headers.find(MIME_CONTENT_ID_FIELD_NAME);
                auto contentId = sanitizeContentId(iy->second);
                auto attachmentId = m_attachmentManager->generateAttachmentId(m_attachmentContextId, contentId);
                if (!m_attachmentWriter && attachmentId != m_attachmentIdBeingReceived) {
                    m_attachmentWriter = m_attachmentManager->createWriter(attachmentId);
                    if (!m_attachmentWriter) {
                        ACSDK_ERROR(LX("onBeginMimePartFailed").d("reason", "createWriterFailed").d("attachmentId", attachmentId));
                        return false;
                    }
                    ACSDK_DEBUG9(LX("attachmentContentDetected").d("contentId", contentId));
                }
                m_contentType = ContentType::ATTACHMENT;
            } else {
                ACSDK_WARN(LX("unhandledContent-Type").d("Content-Type", contentType));
                m_contentType = ContentType::NONE;
            }
            return true;
        }
        HTTP2ReceiveDataStatus MimeResponseSink::onReceiveMimeData(const char* bytes, size_t size) {
            ACSDK_DEBUG9(LX(__func__).d("size", size));
            if (m_handler) m_handler->onActivity();
            switch (m_contentType) {
                case ContentType::JSON:
                    m_directiveBeingReceived.append(bytes, size);
                    return HTTP2ReceiveDataStatus::SUCCESS;
                case ContentType::ATTACHMENT: return writeToAttachment(bytes, size);
                case ContentType::NONE: break;
            }
            return HTTP2ReceiveDataStatus::SUCCESS;
        }
        bool MimeResponseSink::onEndMimePart() {
            ACSDK_DEBUG9(LX(__func__));
            if (m_handler) m_handler->onActivity();
            switch (m_contentType) {
                case ContentType::JSON:
                    if (!m_messageConsumer) {
                        ACSDK_ERROR(LX("onEndMimePartFailed").d("reason", "nullMessageConsumer"));
                        break;
                    }
                    if (!m_directiveBeingReceived.empty()) {
                        m_messageConsumer->consumeMessage(m_attachmentContextId, m_directiveBeingReceived);
                        m_directiveBeingReceived.clear();
                    }
                    break;
                case ContentType::ATTACHMENT:
                    m_attachmentIdBeingReceived.clear();
                    m_attachmentWriter.reset();
                    m_contentType = ContentType::NONE;
                    break;
                default:
                    ACSDK_ERROR(LX("partEndCallbackFailed").d("reason", "unsupportedContentType"));
                    break;
            }
            return true;
        }
        HTTP2ReceiveDataStatus MimeResponseSink::onReceiveNonMimeData(const char* bytes, size_t size) {
            ACSDK_DEBUG9(LX(__func__).d("size", size));
            if (m_handler) m_handler->onActivity();
            auto total = m_nonMimeBody.size() + size;
            if (total <= NON_MIME_BODY_MAX_SIZE) m_nonMimeBody.append(bytes, size);
            else {
                auto spaceLeft = NON_MIME_BODY_MAX_SIZE - m_nonMimeBody.size();
                m_nonMimeBody.append(bytes, spaceLeft);
                ACSDK_ERROR(LX("nonMimeBodyTruncated").d("total", total).d("maxSize", NON_MIME_BODY_MAX_SIZE));
            }
            return HTTP2ReceiveDataStatus::SUCCESS;
        }
        void MimeResponseSink::onResponseFinished(HTTP2ResponseFinishedStatus status) {
            ACSDK_DEBUG9(LX(__func__).d("status", status));
            if (m_handler) m_handler->onResponseFinished(status, m_nonMimeBody);
        }
        HTTP2ReceiveDataStatus MimeResponseSink::writeToAttachment(const char* bytes, size_t size) {
            if (!m_attachmentWriter) {
                ACSDK_ERROR(LX("writeToAttachmentFailed").d("reason", "nullAttachmentWriter"));
                return HTTP2ReceiveDataStatus::ABORT;
            }
            auto writeStatus = AttachmentWriter::WriteStatus::OK;
            auto numWritten = m_attachmentWriter->write(const_cast<char*>(bytes), size, &writeStatus);
            switch (writeStatus) {
                case AttachmentWriter::WriteStatus::OK:
                    if (numWritten != size) {
                        ACSDK_ERROR(LX("writeDataToAttachmentFailed").d("reason", "writeTruncated"));
                        return HTTP2ReceiveDataStatus::ABORT;
                    }
                    return HTTP2ReceiveDataStatus::SUCCESS;
                case AttachmentWriter::WriteStatus::OK_BUFFER_FULL: return HTTP2ReceiveDataStatus::PAUSE;
                case AttachmentWriter::WriteStatus::CLOSED:
                    ACSDK_WARN(LX("writeDataToAttachmentFailed").d("reason", "attachmentWriterIsClosed"));
                    return HTTP2ReceiveDataStatus::ABORT;
                case AttachmentWriter::WriteStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE:
                case AttachmentWriter::WriteStatus::ERROR_INTERNAL:
                    ACSDK_ERROR(LX("writeDataToAttachmentFailed").d("reason", "attachmentWriterInternalError"));
                    return HTTP2ReceiveDataStatus::ABORT;
                case AttachmentWriter::WriteStatus::TIMEDOUT:
                    ACSDK_ERROR(LX("writeDataToAttachmentFailed").d("reason", "unexpectedTimedoutStatus"));
                    return HTTP2ReceiveDataStatus::ABORT;
            }
            ACSDK_ERROR(LX("writeDataToAttachmentFailed").d("reason", "unhandledStatus").d("status", static_cast<int>(writeStatus)));
            return HTTP2ReceiveDataStatus::ABORT;
        }
    }
}