#include <vector>
#include <avs/EventBuilder.h>
#include <logger/Logger.h>
#include <util/RetryTimer.h>
#include "PostConnectVerifyGatewaySender.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        static const string TAG("PostConnectVerifyGatewaySender");
        #define LX(event) LogEntry(TAG, event)
        static const string VERIFY_GATEWAY_NAMESPACE = "Alexa.ApiGateway";
        static const string VERIFY_GATEWAY_NAME = "VerifyGateway";
        static const vector<int> RETRY_TABLE = { 1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000, 256000, };
        static RetryTimer RETRY_TIMER{RETRY_TABLE};
        shared_ptr<PostConnectVerifyGatewaySender> PostConnectVerifyGatewaySender::create(function<void(const shared_ptr<PostConnectVerifyGatewaySender>& verifyGatewaySender)> gatewayVerifiedCallback) {
            if (!gatewayVerifiedCallback) { ACSDK_ERROR(LX("createFailed").d("reason", "invalid gatewayVerifiedCallback")); }
            else return shared_ptr<PostConnectVerifyGatewaySender>(new PostConnectVerifyGatewaySender(gatewayVerifiedCallback));
            return nullptr;
        }
        PostConnectVerifyGatewaySender::PostConnectVerifyGatewaySender(function<void(const shared_ptr<PostConnectVerifyGatewaySender>& verifyGatewaySender)> gatewayVerifiedCallback) :
                                                                       m_gatewayVerifiedCallback{gatewayVerifiedCallback}, m_isStopping{false} {}
        unsigned int PostConnectVerifyGatewaySender::getOperationPriority() {
            return VERIFY_GATEWAY_PRIORITY;
        }
        bool PostConnectVerifyGatewaySender::performOperation(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__));
            if (!messageSender) {
                ACSDK_ERROR(LX("performOperationFailed").d("reason", "nullPostConnectSender"));
                return false;
            }
            int retryAttempt = 0;
            while(!isStopping()) {
                auto status = sendVerifyGateway(messageSender);
                switch(status) {
                    case VerifyGatewayReturnCode::GATEWAY_VERIFIED:
                        m_gatewayVerifiedCallback(shared_from_this());
                        return true;
                    case VerifyGatewayReturnCode::CHANGING_GATEWAY: return true;
                    case VerifyGatewayReturnCode::FATAL_ERROR: return false;
                    case VerifyGatewayReturnCode::RETRIABLE_ERROR: break;
                }
                if (m_wakeEvent.wait(RETRY_TIMER.calculateTimeToRetry(retryAttempt++))) {
                    ACSDK_DEBUG5(LX(__func__).m("aborting operation"));
                    return false;
                }
            }
            return false;
        }
        bool PostConnectVerifyGatewaySender::isStopping() {
            lock_guard<std::mutex> lock{m_mutex};
            return m_isStopping;
        }
        void PostConnectVerifyGatewaySender::abortOperation() {
            ACSDK_DEBUG5(LX(__func__));
            std::shared_ptr<WaitableMessageRequest> requestCopy;
            {
                std::lock_guard<std::mutex> lock{m_mutex};
                if (m_isStopping) return;
                m_isStopping = true;
                requestCopy = m_postConnectRequest;
            }
            if (requestCopy) requestCopy->shutdown();
            m_wakeEvent.wakeUp();
        }
        PostConnectVerifyGatewaySender::VerifyGatewayReturnCode PostConnectVerifyGatewaySender::sendVerifyGateway(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__));
            unique_lock<std::mutex> lock{m_mutex};
            auto event = buildJsonEventString(VERIFY_GATEWAY_NAMESPACE, VERIFY_GATEWAY_NAME);
            m_postConnectRequest.reset();
            m_postConnectRequest = make_shared<WaitableMessageRequest>(event.second);
            lock.unlock();
            messageSender->sendMessage(m_postConnectRequest);
            auto status = m_postConnectRequest->waitForCompletion();
            switch(status) {
                case MessageRequestObserverInterface::Status::SUCCESS: return VerifyGatewayReturnCode::CHANGING_GATEWAY;
                case MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT: return VerifyGatewayReturnCode::GATEWAY_VERIFIED;
                case MessageRequestObserverInterface::Status::CANCELED: case MessageRequestObserverInterface::Status::BAD_REQUEST:
                case MessageRequestObserverInterface::Status::PROTOCOL_ERROR: case MessageRequestObserverInterface::Status::REFUSED:
                case MessageRequestObserverInterface::Status::INVALID_AUTH:
                    return VerifyGatewayReturnCode::FATAL_ERROR;
                default: return VerifyGatewayReturnCode::RETRIABLE_ERROR;
            }
            return VerifyGatewayReturnCode::FATAL_ERROR;
        }
    }
}