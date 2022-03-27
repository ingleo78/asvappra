#ifndef ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_VISUALACTIVITYTRACKER_H_
#define ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_VISUALACTIVITYTRACKER_H_

#include <chrono>
#include <memory>
#include <vector>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/StateProviderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include "Channel.h"
#include "ActivityTrackerInterface.h"

namespace alexaClientSDK {
    namespace afml {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace threading;
        class VisualActivityTracker : public RequiresShutdown, public ActivityTrackerInterface, public CapabilityConfigurationInterface,
                                      public StateProviderInterface {
        public:
            static shared_ptr<VisualActivityTracker> create(shared_ptr<ContextManagerInterface> contextManager);
            void provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) override;
            void notifyOfActivityUpdates(const vector<Channel::State>& channelStates) override;
            unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
        private:
            VisualActivityTracker(shared_ptr<ContextManagerInterface> contextManager);
            void doShutdown() override;
            void executeProvideState(unsigned int stateRequestToken);
            shared_ptr<ContextManagerInterface> m_contextManager;
            Channel::State m_channelState;
            unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
            Executor m_executor;
        };
    }
}
#endif