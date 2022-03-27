#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_INTERACTIONMODEL_INCLUDE_INTERACTIONMODEL_INTERACTIONMODELCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_INTERACTIONMODEL_INCLUDE_INTERACTIONMODEL_INTERACTIONMODELCAPABILITYAGENT_H_

#include <memory>
#include <set>
#include <string>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <sdkinterfaces/InteractionModelRequestProcessingObserverInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace interactionModel {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace json;
            using namespace rapidjson;
            class InteractionModelCapabilityAgent : public CapabilityAgent, public CapabilityConfigurationInterface {
            public:
                static shared_ptr<InteractionModelCapabilityAgent> create(shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                                          shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                void addObserver(shared_ptr<InteractionModelRequestProcessingObserverInterface> observer);
                void removeObserver(shared_ptr<InteractionModelRequestProcessingObserverInterface> observer);
                ~InteractionModelCapabilityAgent();
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
            private:
                InteractionModelCapabilityAgent(shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                bool handleDirectiveHelper(shared_ptr<CapabilityAgent::DirectiveInfo> info, string* errMessage, ExceptionErrorType* type);
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                shared_ptr<DirectiveSequencerInterface> m_directiveSequencer;
                mutex m_observerMutex;
                set<shared_ptr<InteractionModelRequestProcessingObserverInterface>> m_observers;
            };
        }
    }
}
#endif