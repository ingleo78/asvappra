#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_POSTCONNECTCAPABILITIESPUBLISHER_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_POSTCONNECTCAPABILITIESPUBLISHER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <avs/WaitableMessageRequest.h>
#include <sdkinterfaces/AlexaEventProcessedObserverInterface.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/AuthObserverInterface.h>
#include <sdkinterfaces/PostConnectOperationInterface.h>
#include <util/WaitEvent.h>
#include "DiscoveryEventSenderInterface.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        class PostConnectCapabilitiesPublisher : public PostConnectOperationInterface, public enable_shared_from_this<PostConnectCapabilitiesPublisher> {
        public:
            static shared_ptr<PostConnectCapabilitiesPublisher> create(const shared_ptr<DiscoveryEventSenderInterface>& discoveryEventSender);
            ~PostConnectCapabilitiesPublisher();
            unsigned int getOperationPriority() override;
            bool performOperation(const shared_ptr<MessageSenderInterface>& messageSender) override;
            void abortOperation() override;
        private:
            PostConnectCapabilitiesPublisher(shared_ptr<DiscoveryEventSenderInterface> discoveryEventSender);
            mutex m_mutex;
            bool m_isPerformOperationInvoked;
            shared_ptr<DiscoveryEventSenderInterface> m_discoveryEventSender;
        };
    }
}
#endif