#include <utility>
#include <logger/Logger.h>
#include <util/RetryTimer.h>
#include "Utils/DiscoveryUtils.h"
#include "DiscoveryEventSender.h"
#include "PostConnectCapabilitiesPublisher.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace avs;
        using namespace utils;
        using namespace logger;
        using namespace capabilitiesDelegate::utils;
        static const string TAG("PostConnectCapabilitiesPublisher");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<PostConnectCapabilitiesPublisher> PostConnectCapabilitiesPublisher::create(
            const shared_ptr<DiscoveryEventSenderInterface>& discoveryEventSender) {
            if (!discoveryEventSender) { ACSDK_ERROR(LX("createFailed").d("reason", "invalid DiscoveryEventSender")); }
            else {
                auto instance = std::shared_ptr<PostConnectCapabilitiesPublisher>(new PostConnectCapabilitiesPublisher(discoveryEventSender));
                return instance;
            }
            return nullptr;
        }
        PostConnectCapabilitiesPublisher::PostConnectCapabilitiesPublisher(shared_ptr<DiscoveryEventSenderInterface> discoveryEventSender) :
                                                                           m_isPerformOperationInvoked{false}, m_discoveryEventSender{move(discoveryEventSender)} {}
        PostConnectCapabilitiesPublisher::~PostConnectCapabilitiesPublisher() {
            ACSDK_DEBUG5(LX(__func__));
            m_discoveryEventSender->stop();
        }
        unsigned int PostConnectCapabilitiesPublisher::getOperationPriority() {
            return ENDPOINT_DISCOVERY_PRIORITY;
        }
        bool PostConnectCapabilitiesPublisher::performOperation(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__));
            if (!messageSender) {
                ACSDK_ERROR(LX("performOperationFailed").d("reason", "nullPostConnectSender"));
                return false;
            }
            {
                lock_guard<mutex> lock{m_mutex};
                if (m_isPerformOperationInvoked) {
                    ACSDK_ERROR(LX("performOperationFailed").d("reason", "performOperation should only be called once."));
                    return false;
                }
                m_isPerformOperationInvoked = true;
            }
            if (!m_discoveryEventSender) {
                ACSDK_ERROR(LX("performOperationFailed").d("reason", "DiscoveryEventSender is null"));
                return false;
            }
            return m_discoveryEventSender->sendDiscoveryEvents(messageSender);
        }
        void PostConnectCapabilitiesPublisher::abortOperation() {
            ACSDK_DEBUG5(LX(__func__));
            m_discoveryEventSender->stop();
        }
    }
}