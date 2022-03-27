#include <algorithm>
#include <functional>
#include <unordered_map>
#include <util/HTTP/HttpResponseCode.h>
#include <util/HTTP2/HTTP2MimeRequestEncoder.h>
#include <util/HTTP2/HTTP2MimeResponseDecoder.h>
#include <logger/Logger.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/DataPointStringBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include "HTTP2Transport.h"
#include "MimeResponseSink.h"
#include "MessageRequestHandler.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace attachment;
        using namespace http;
        using namespace http2;
        using namespace logger;
        using namespace metrics;
        const static string AVS_EVENT_URL_PATH_EXTENSION = "/v20160207/events";
        const static string MIME_BOUNDARY = "WhooHooZeerOoonie!";
        static const seconds STREAM_PROGRESS_TIMEOUT = std::chrono::seconds(15);
        static const vector<string> JSON_MIME_PART_HEADER_LINES = { "Content-Disposition: form-data; name=\"metadata\"", "Content-Type: application/json"};
        static const string CONTENT_DISPOSITION_PREFIX = "Content-Disposition: form-data; name=\"";
        static const string CONTENT_DISPOSITION_SUFFIX = "\"";
        static const string ATTACHMENT_CONTENT_TYPE = "Content-Type: application/octet-stream";
        static const string MESSAGEREQUEST_ID_PREFIX = "AVSEvent-";
        static const string TAG("MessageRequestHandler");
        static const string ACL_METRIC_SOURCE_PREFIX = "ACL-";
        static const string SEND_DATA_ERROR = "ERROR.SEND_DATA_ERROR";
        static const string READ_STATUS_TAG = "READ_STATUS";
        static const string ERROR_READ_OVERRUN = "READ_OVERRUN";
        static const string ERROR_INTERNAL = "INTERNAL_ERROR";
        static const string SEND_COMPLETED = "SEND_COMPLETED";
        static const string HTTP_KEY_VALUE_SEPARATOR = ": ";
        #define LX(event) LogEntry(TAG, event)
        static void collectSendDataResultMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, int count, const string& readStatus) {
            recordMetric(metricRecorder,MetricEventBuilder{}.setActivityName(ACL_METRIC_SOURCE_PREFIX + SEND_DATA_ERROR)
                                                   .addDataPoint(DataPointCounterBuilder{}.setName(SEND_DATA_ERROR).increment(count).build())
                                                   .addDataPoint(DataPointStringBuilder{}.setName(READ_STATUS_TAG).setValue(readStatus).build())
                                                   .build());
        }
        MessageRequestHandler::~MessageRequestHandler() {
            reportMessageRequestAcknowledged();
            reportMessageRequestFinished();
        }
        shared_ptr<MessageRequestHandler> MessageRequestHandler::create(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken,
                                                                        shared_ptr<MessageRequest> messageRequest,
                                                                        shared_ptr<MessageConsumerInterface> messageConsumer,
                                                                        shared_ptr<AttachmentManager> attachmentManager,
                                                                        shared_ptr<MetricRecorderInterface> metricRecorder,
                                                                        shared_ptr<EventTracerInterface> eventTracer) {
            ACSDK_DEBUG7(LX(__func__).d("context", context.get()).d("messageRequest", messageRequest.get()));
            if (!context) {
                ACSDK_CRITICAL(LX("MessageRequestHandlerCreateFailed").d("reason", "nullHttp2Transport"));
                return nullptr;
            }
            if (authToken.empty()) {
                ACSDK_DEBUG9(LX("createFailed").d("reason", "emptyAuthToken"));
                return nullptr;
            }
            shared_ptr<MessageRequestHandler> handler(new MessageRequestHandler(context, authToken, messageRequest, move(metricRecorder)));
            auto url = context->getAVSGateway();
            if (messageRequest->getUriPathExtension().empty()) url += AVS_EVENT_URL_PATH_EXTENSION;
            else url += messageRequest->getUriPathExtension();
            HTTP2RequestConfig cfg{HTTP2RequestType::POST, url, MESSAGEREQUEST_ID_PREFIX};
            cfg.setRequestSource(make_shared<HTTP2MimeRequestEncoder>(MIME_BOUNDARY, handler));
            cfg.setResponseSink(make_shared<HTTP2MimeResponseDecoder>(
                std::make_shared<MimeResponseSink>(handler, messageConsumer, attachmentManager, cfg.getId())));
            cfg.setActivityTimeout(STREAM_PROGRESS_TIMEOUT);
            context->onMessageRequestSent(messageRequest);
            auto request = context->createAndSendRequest(cfg);
            if (!request) {
                handler->reportMessageRequestAcknowledged();
                handler->reportMessageRequestFinished();
                ACSDK_ERROR(LX("MessageRequestHandlerCreateFailed").d("reason", "createAndSendRequestFailed"));
                return nullptr;
            }
            if (eventTracer) eventTracer->traceEvent(messageRequest->getJsonContent());
            ACSDK_DEBUG0(LX("EventSent").sensitive("url", messageRequest->getUriPathExtension()).sensitive("jsonContent", messageRequest->getJsonContent()));
            return handler;
        }
        MessageRequestHandler::MessageRequestHandler(
            std::shared_ptr<ExchangeHandlerContextInterface> context,
            const std::string& authToken,
            std::shared_ptr<avsCommon::avs::MessageRequest> messageRequest,
            std::shared_ptr<MetricRecorderInterface> metricRecorder) :
                ExchangeHandler{context, authToken},
                m_messageRequest{messageRequest},
                m_json{messageRequest->getJsonContent()},
                m_jsonNext{m_json.c_str()},
                m_countOfJsonBytesLeft{m_json.size()},
                m_countOfPartsSent{0},
                m_metricRecorder{metricRecorder},
                m_wasMessageRequestAcknowledgeReported{false},
                m_wasMessageRequestFinishedReported{false},
                m_responseCode{0} {
            ACSDK_DEBUG7(LX(__func__).d("context", context.get()).d("messageRequest", messageRequest.get()));
        }
        void MessageRequestHandler::reportMessageRequestAcknowledged() {
            ACSDK_DEBUG7(LX(__func__));
            if (!m_wasMessageRequestAcknowledgeReported) {
                m_wasMessageRequestAcknowledgeReported = true;
                m_context->onMessageRequestAcknowledged(m_messageRequest);
            }
        }
        void MessageRequestHandler::reportMessageRequestFinished() {
            ACSDK_DEBUG7(LX(__func__));
            if (!m_wasMessageRequestFinishedReported) {
                m_wasMessageRequestFinishedReported = true;
                m_context->onMessageRequestFinished();
            }
        }
        std::vector<std::string> MessageRequestHandler::getRequestHeaderLines() {
            ACSDK_DEBUG9(LX(__func__));
            m_context->onActivity();
            vector<string> result({m_authHeader});
            auto headers = m_messageRequest->getHeaders();
            for (const auto& header : headers) result.emplace_back(header.first + HTTP_KEY_VALUE_SEPARATOR + header.second);
            return result;
        }
        HTTP2GetMimeHeadersResult MessageRequestHandler::getMimePartHeaderLines() {
            ACSDK_DEBUG9(LX(__func__));
            m_context->onActivity();
            if (0 == m_countOfPartsSent) return HTTP2GetMimeHeadersResult(JSON_MIME_PART_HEADER_LINES);
            else if (static_cast<int>(m_countOfPartsSent) <= m_messageRequest->attachmentReadersCount()) {
                m_namedReader = m_messageRequest->getAttachmentReader(m_countOfPartsSent - 1);
                if (m_namedReader) {
                    return HTTP2GetMimeHeadersResult({ CONTENT_DISPOSITION_PREFIX + m_namedReader->name + CONTENT_DISPOSITION_SUFFIX,
                                                     ATTACHMENT_CONTENT_TYPE });
                } else {
                    ACSDK_ERROR(LX("getMimePartHeaderLinesFailed").d("reason", "nullReader").d("index", m_countOfPartsSent));
                    return HTTP2GetMimeHeadersResult::ABORT;
                }
            } else return HTTP2GetMimeHeadersResult::COMPLETE;
        }
        HTTP2SendDataResult MessageRequestHandler::onSendMimePartData(char* bytes, size_t size) {
            ACSDK_DEBUG9(LX(__func__).d("size", size));
            m_context->onActivity();
            if (0 == m_countOfPartsSent) {
                if (m_countOfJsonBytesLeft != 0) {
                    size_t countToCopy = (m_countOfJsonBytesLeft <= size) ? m_countOfJsonBytesLeft : size;
                    copy(m_jsonNext, m_jsonNext + countToCopy, bytes);
                    m_jsonNext += countToCopy;
                    m_countOfJsonBytesLeft -= countToCopy;
                    return HTTP2SendDataResult(countToCopy);
                } else {
                    m_countOfPartsSent++;
                    return HTTP2SendDataResult::COMPLETE;
                }
            } else if (m_namedReader) {
                auto readStatus = AttachmentReader::ReadStatus::OK;
                auto bytesRead = m_namedReader->reader->read(bytes, size, &readStatus);
                ACSDK_DEBUG9(LX("attachmentRead").d("readStatus", (int)readStatus).d("bytesRead", bytesRead));
                switch (readStatus) {
                    case AttachmentReader::ReadStatus::OK: case AttachmentReader::ReadStatus::OK_WOULDBLOCK: case AttachmentReader::ReadStatus::OK_TIMEDOUT:
                        return bytesRead != 0 ? HTTP2SendDataResult(bytesRead) : HTTP2SendDataResult::PAUSE;
                    case AttachmentReader::ReadStatus::OK_OVERRUN_RESET: return HTTP2SendDataResult::ABORT;
                    case AttachmentReader::ReadStatus::CLOSED:
                        m_namedReader.reset();
                        m_countOfPartsSent++;
                        collectSendDataResultMetric(m_metricRecorder, 0, SEND_COMPLETED);
                        return HTTP2SendDataResult::COMPLETE;
                    case AttachmentReader::ReadStatus::ERROR_OVERRUN:
                        collectSendDataResultMetric(m_metricRecorder, 1, ERROR_READ_OVERRUN);
                        return HTTP2SendDataResult::ABORT;
                    case AttachmentReader::ReadStatus::ERROR_INTERNAL:
                        collectSendDataResultMetric(m_metricRecorder, 1, ERROR_INTERNAL);
                        return HTTP2SendDataResult::ABORT;
                    case AttachmentReader::ReadStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE: return HTTP2SendDataResult::PAUSE;
                }
            }
            ACSDK_ERROR(LX("onSendMimePartDataFailed").d("reason", "noMoreAttachments"));
            return HTTP2SendDataResult::ABORT;
        }
        void MessageRequestHandler::onActivity() {
            m_context->onActivity();
        }
        bool MessageRequestHandler::onReceiveResponseCode(long responseCode) {
            ACSDK_DEBUG7(LX(__func__).d("responseCode", responseCode));
            reportMessageRequestAcknowledged();
            if (HTTPResponseCode::CLIENT_ERROR_FORBIDDEN == intToHTTPResponseCode(responseCode)) m_context->onForbidden(m_authToken);
            m_responseCode = responseCode;
            return true;
        }
        void MessageRequestHandler::onResponseFinished(HTTP2ResponseFinishedStatus status, const string& nonMimeBody) {
            ACSDK_DEBUG7(LX(__func__).d("status", status).d("responseCode", m_responseCode));
            if (HTTP2ResponseFinishedStatus::TIMEOUT == status) m_context->onMessageRequestTimeout();
            reportMessageRequestAcknowledged();
            reportMessageRequestFinished();
            if ((intToHTTPResponseCode(m_responseCode) != HTTPResponseCode::SUCCESS_OK) && !nonMimeBody.empty()) {
                m_messageRequest->exceptionReceived(nonMimeBody);
            }
            struct statusHash {
                size_t operator()(const HTTP2ResponseFinishedStatus& key) const {
                    return static_cast<size_t>(key);
                }
            };
            static const unordered_map<HTTP2ResponseFinishedStatus, MessageRequestObserverInterface::Status, statusHash>
                statusToResult = {
                    {HTTP2ResponseFinishedStatus::INTERNAL_ERROR, MessageRequestObserverInterface::Status::INTERNAL_ERROR},
                    {HTTP2ResponseFinishedStatus::CANCELLED, MessageRequestObserverInterface::Status::CANCELED},
                    {HTTP2ResponseFinishedStatus::TIMEOUT, MessageRequestObserverInterface::Status::TIMEDOUT}};
            static const unordered_map<long, MessageRequestObserverInterface::Status> responseToResult = {
                {HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED, MessageRequestObserverInterface::Status::INTERNAL_ERROR},
                {HTTPResponseCode::SUCCESS_OK, MessageRequestObserverInterface::Status::SUCCESS},
                {HTTPResponseCode::SUCCESS_ACCEPTED, MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED},
                {HTTPResponseCode::SUCCESS_NO_CONTENT, MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT},
                {HTTPResponseCode::CLIENT_ERROR_BAD_REQUEST, MessageRequestObserverInterface::Status::BAD_REQUEST},
                {HTTPResponseCode::CLIENT_ERROR_FORBIDDEN, MessageRequestObserverInterface::Status::INVALID_AUTH},
                {HTTPResponseCode::SERVER_ERROR_INTERNAL, MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2}};
            auto result = MessageRequestObserverInterface::Status::INTERNAL_ERROR;
            if (HTTP2ResponseFinishedStatus::COMPLETE == status) {
                auto responseIterator = responseToResult.find(m_responseCode);
                if (responseIterator != responseToResult.end()) result = responseIterator->second;
                else result = MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR;
            } else {
                auto statusIterator = statusToResult.find(status);
                if (statusIterator != statusToResult.end()) result = statusIterator->second;
            }
            m_messageRequest->sendCompleted(result);
        }
    }
}