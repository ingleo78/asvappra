#ifndef ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_AUDIOACTIVITYTRACKER_H_
#define ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_AUDIOACTIVITYTRACKER_H_

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
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
        class AudioActivityTracker : public RequiresShutdown, public ActivityTrackerInterface, public CapabilityConfigurationInterface, public StateProviderInterface {
        public:
            static shared_ptr<AudioActivityTracker> create(shared_ptr<ContextManagerInterface> contextManager);
            void provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) override;
            void notifyOfActivityUpdates(const vector<Channel::State>& channelStates) override;
            unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
        private:
            AudioActivityTracker(shared_ptr<ContextManagerInterface> contextManager);
            void doShutdown() override;
            void executeNotifyOfActivityUpdates(const vector<Channel::State>& channelStates);
            void executeProvideState(unsigned int stateRequestToken);
            const std::string& executeChannelNameInLowerCase(const std::string& channelName);
            shared_ptr<ContextManagerInterface> m_contextManager;
            unordered_map<std::string, Channel::State> m_channelStates;
            unordered_map<std::string, std::string> m_channelNamesInLowerCase;
            unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
            Executor m_executor;
        };
    }
}
#endif