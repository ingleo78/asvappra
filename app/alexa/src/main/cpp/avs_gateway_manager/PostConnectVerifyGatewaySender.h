#ifndef ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_POSTCONNECTVERIFYGATEWAYSENDER_H_
#define ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_POSTCONNECTVERIFYGATEWAYSENDER_H_

#include <memory>
#include <mutex>
#include <functional>
#include <sdkinterfaces/PostConnectOperationInterface.h>
#include <avs/WaitableMessageRequest.h>
#include <util/WaitEvent.h>
#include "GatewayVerifyState.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        class PostConnectVerifyGatewaySender : public PostConnectOperationInterface, public enable_shared_from_this<PostConnectVerifyGatewaySender> {
        public:
            static shared_ptr<PostConnectVerifyGatewaySender> create(function<void(const shared_ptr<PostConnectVerifyGatewaySender>&)> gatewayVerifiedCallback);
            unsigned int getOperationPriority() override;
            bool performOperation(const shared_ptr<MessageSenderInterface>& messageSender) override;
            void abortOperation() override;
        private:
            enum class VerifyGatewayReturnCode {
                GATEWAY_VERIFIED,
                CHANGING_GATEWAY,
                FATAL_ERROR,
                RETRIABLE_ERROR
            };
            PostConnectVerifyGatewaySender(function<void(const shared_ptr<PostConnectVerifyGatewaySender>&)> gatewayVerifiedCallback);
            VerifyGatewayReturnCode sendVerifyGateway(const shared_ptr<MessageSenderInterface>& messageSender);
            bool isStopping();
            function<void(const shared_ptr<PostConnectVerifyGatewaySender>&)> m_gatewayVerifiedCallback;
            mutex m_mutex;
            bool m_isStopping;
            shared_ptr<WaitableMessageRequest> m_postConnectRequest;
            WaitEvent m_wakeEvent;
        };
    }
}
#endif