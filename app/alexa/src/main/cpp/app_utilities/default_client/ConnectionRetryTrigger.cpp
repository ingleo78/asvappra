#include <logger/Logger.h>
#include "ConnectionRetryTrigger.h"

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace capabilityAgents::aip;
        static const string TAG("ConnectionRetryTrigger");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<ConnectionRetryTrigger> ConnectionRetryTrigger::create(shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                                                          shared_ptr<AudioInputProcessor> audioInputProcessor) {
            if (!connectionManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullConnectionManager"));
                return nullptr;
            }
            if (!audioInputProcessor) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullAudioInputProcessor"));
                return nullptr;
            }
            shared_ptr<ConnectionRetryTrigger> connectionRetryTrigger(new ConnectionRetryTrigger(connectionManager));
            audioInputProcessor->addObserver(connectionRetryTrigger);
            return connectionRetryTrigger;
        }
        ConnectionRetryTrigger::ConnectionRetryTrigger(shared_ptr<AVSConnectionManagerInterface> connectionManager) :
                                                       m_state{AudioInputProcessorObserverInterface::State::IDLE}, m_connectionManager{connectionManager} {}
        void ConnectionRetryTrigger::onStateChanged(AudioInputProcessorObserverInterface::State state) {
            ACSDK_DEBUG9(LX(__func__).d("state", state));
            if (AudioInputProcessorObserverInterface::State::IDLE == m_state && state != m_state) m_connectionManager->onWakeConnectionRetry();
            m_state = state;
        }
    }
}