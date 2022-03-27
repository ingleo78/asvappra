#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_CONNECTIONRETRYTRIGGER_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_CONNECTIONRETRYTRIGGER_H_

#include <memory>
#include <capability_agents/AIP/AudioInputProcessor.h>
#include <sdkinterfaces/AudioInputProcessorObserverInterface.h>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace avsCommon::sdkInterfaces;
        using namespace capabilityAgents::aip;
        class ConnectionRetryTrigger : public AudioInputProcessorObserverInterface {
        public:
            static shared_ptr<ConnectionRetryTrigger> create(shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                                             shared_ptr<AudioInputProcessor> audioInputProcessor);
        private:
            ConnectionRetryTrigger(shared_ptr<AVSConnectionManagerInterface> connectionManager);
            void onStateChanged(AudioInputProcessorObserverInterface::State state) override;
            AudioInputProcessorObserverInterface::State m_state;
            shared_ptr<AVSConnectionManagerInterface> m_connectionManager;
        };
    }
}
#endif