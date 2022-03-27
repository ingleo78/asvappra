#include <util/HTTP/HttpResponseCode.h>
#include <util/HTTP2/HTTP2MimeResponseDecoder.h>
#include <logger/Logger.h>
#include "DownchannelHandler.h"
#include "HTTP2Transport.h"
#include "MimeResponseSink.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace attachment;
        using namespace logger;
        using namespace http;
        using namespace http2;
        const static string AVS_DOWNCHANNEL_URL_PATH_EXTENSION = "/v20160207/directives";
        static const string DOWNCHANNEL_ID_PREFIX = "AVSDownChannel-";
        static const seconds ESTABLISH_CONNECTION_TIMEOUT = seconds(60);
        static const string TAG("DownchannelHandler");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<DownchannelHandler> DownchannelHandler::create(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken,
                                                                  shared_ptr<MessageConsumerInterface> messageConsumer,
                                                                  shared_ptr<AttachmentManager> attachmentManager) {
            ACSDK_DEBUG9(LX(__func__).d("context", context.get()));
            if (!context) {
                ACSDK_CRITICAL(LX("createFailed").d("reason", "nullHttp2Transport"));
                return nullptr;
            }
            if (authToken.empty()) {
                ACSDK_DEBUG9(LX("createFailed").d("reason", "emptyAuthToken"));
                return nullptr;
            }
            shared_ptr<DownchannelHandler> handler(new DownchannelHandler(context, authToken));
            HTTP2RequestConfig cfg{HTTP2RequestType::GET, context->getAVSGateway() + AVS_DOWNCHANNEL_URL_PATH_EXTENSION, DOWNCHANNEL_ID_PREFIX};
            cfg.setRequestSource(handler);
            cfg.setResponseSink(make_shared<HTTP2MimeResponseDecoder>(make_shared<MimeResponseSink>(handler, messageConsumer, attachmentManager, cfg.getId())));
            cfg.setConnectionTimeout(ESTABLISH_CONNECTION_TIMEOUT);
            cfg.setIntermittentTransferExpected();
            auto request = context->createAndSendRequest(cfg);
            if (!request) {
                ACSDK_ERROR(LX("createFailed").d("reason", "createAndSendRequestFailed"));
                return nullptr;
            }
            return handler;
        }
        vector<string> DownchannelHandler::getRequestHeaderLines() {
            ACSDK_DEBUG9(LX(__func__));
            return {m_authHeader};
        }
        HTTP2SendDataResult DownchannelHandler::onSendData(char* bytes, size_t size) {
            ACSDK_DEBUG9(LX(__func__).d("size", size));
            return HTTP2SendDataResult::COMPLETE;
        }
        DownchannelHandler::DownchannelHandler(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken) : ExchangeHandler{context, authToken} {
            ACSDK_DEBUG9(LX(__func__).d("context", context.get()));
        }
        void DownchannelHandler::onActivity() {
            m_context->onActivity();
        }
        bool DownchannelHandler::onReceiveResponseCode(long responseCode) {
            ACSDK_DEBUG5(LX(__func__).d("responseCode", responseCode));
            switch (intToHTTPResponseCode(responseCode)) {
                case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED: case HTTPResponseCode::SUCCESS_CREATED: case HTTPResponseCode::SUCCESS_ACCEPTED:
                case HTTPResponseCode::SUCCESS_NO_CONTENT: case HTTPResponseCode::SUCCESS_PARTIAL_CONTENT: case HTTPResponseCode::SUCCESS_END_CODE:
                case HTTPResponseCode::REDIRECTION_MULTIPLE_CHOICES: case HTTPResponseCode::REDIRECTION_MOVED_PERMANENTLY:
                case HTTPResponseCode::REDIRECTION_FOUND: case HTTPResponseCode::REDIRECTION_SEE_ANOTHER: case HTTPResponseCode::REDIRECTION_TEMPORARY_REDIRECT:
                case HTTPResponseCode::REDIRECTION_PERMANENT_REDIRECT: case HTTPResponseCode::CLIENT_ERROR_BAD_REQUEST:
                case HTTPResponseCode::SERVER_ERROR_INTERNAL: case HTTPResponseCode::SERVER_ERROR_NOT_IMPLEMENTED:
                    break;
                case HTTPResponseCode::SUCCESS_OK: m_context->onDownchannelConnected(); break;
                case HTTPResponseCode::CLIENT_ERROR_FORBIDDEN: m_context->onForbidden(m_authToken); break;
            }
            return true;
        }
        void DownchannelHandler::onResponseFinished(HTTP2ResponseFinishedStatus status, const std::string& nonMimeBody) {
            ACSDK_DEBUG5(LX(__func__).d("status", status).d("nonMimeBody", nonMimeBody));
            m_context->onDownchannelFinished();
        }
    }
}