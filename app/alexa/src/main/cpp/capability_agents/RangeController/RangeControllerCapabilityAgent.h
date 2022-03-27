#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_RANGECONTROLLER_INCLUDE_RANGECONTROLLER_RANGECONTROLLERCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_RANGECONTROLLER_INCLUDE_RANGECONTROLLER_RANGECONTROLLERCAPABILITYAGENT_H_

#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/CapabilityState.h>
#include <avs/CapabilityResources.h>
#include <sdkinterfaces/AlexaInterfaceMessageSenderInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/RangeController/RangeControllerAttributeBuilderInterface.h>
#include <sdkinterfaces/RangeController/RangeControllerInterface.h>
#include <sdkinterfaces/RangeController/RangeControllerObserverInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace rangeController {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace endpoints;
            using namespace utils;
            using namespace logger;
            using namespace threading;
            using namespace timing;
            using namespace rapidjson;
            using namespace sdkInterfaces::rangeController;
            class RangeControllerCapabilityAgent : public CapabilityAgent, public RangeControllerObserverInterface, public RequiresShutdown,
                                                   public enable_shared_from_this<RangeControllerCapabilityAgent> {
            public:
                using RangeState = RangeControllerInterface::RangeState;
                static shared_ptr<RangeControllerCapabilityAgent> create(const EndpointIdentifier& endpointId, const string& instance,
                                                                         const RangeControllerAttributes& rangeControllerAttributes,
                                                                         shared_ptr<RangeControllerInterface> rangeController,
                                                                         shared_ptr<ContextManagerInterface> contextManager,
                                                                         shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                         shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                         bool isProactivelyReported, bool isRetrievable,
                                                                         bool isNonControllable = false);
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                void provideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken) override;
                bool canStateBeRetrieved() override;
                bool hasReportableStateProperties() override;
                CapabilityConfiguration getCapabilityConfiguration();
                void onRangeChanged(const RangeState& rangeState, AlexaStateChangeCauseType causeType) override;
            private:
                RangeControllerCapabilityAgent(const EndpointIdentifier& endpointId, const string& instance,
                                               const RangeControllerAttributes& rangeControllerAttributes,
                                               shared_ptr<RangeControllerInterface> rangeController,
                                               shared_ptr<ContextManagerInterface> contextManager,
                                               shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                               shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                               bool isProactivelyReported, bool isRetrievable, bool isNonControllable = false);
                void doShutdown() override;
                bool initialize();
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info);
                string buildRangeConfigurationJson();
                void executeSetRangeValueDirective(shared_ptr<DirectiveInfo> info, Document& payload);
                void executeAdjustRangeValueDirective(shared_ptr<DirectiveInfo> info, Document& payload);
                void executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type);
                bool validateRangeValue(double rangeValue);
                void executeProvideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken);
                void executeSendResponseEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info, pair<AlexaResponseType, string> result);
                CapabilityState buildCapabilityState(const RangeState& rangeState);
                EndpointIdentifier m_endpointId;
                string m_instance;
                bool m_isProactivelyReported;
                bool m_isRetrievable;
                bool m_isNonControllable;
                RangeControllerAttributes m_rangeControllerAttributes;
                shared_ptr<RangeControllerInterface> m_rangeController;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<AlexaInterfaceMessageSenderInterface> m_responseSender;
                RangeControllerInterface::RangeControllerConfiguration m_rangeControllerConfiguration;
                Executor m_executor;
            };
        }
    }
}
#endif