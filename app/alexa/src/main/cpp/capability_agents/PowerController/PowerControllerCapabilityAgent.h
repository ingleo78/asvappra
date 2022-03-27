#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_POWERCONTROLLER_INCLUDE_POWERCONTROLLER_POWERCONTROLLERCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_POWERCONTROLLER_INCLUDE_POWERCONTROLLER_POWERCONTROLLERCAPABILITYAGENT_H_

#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/CapabilityState.h>
#include <sdkinterfaces/AlexaInterfaceMessageSenderInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/PowerController/PowerControllerInterface.h>
#include <sdkinterfaces/PowerController/PowerControllerObserverInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace powerController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace endpoints;
            using namespace utils;
            using namespace logger;
            using namespace threading;
            using namespace rapidjson;
            using namespace sdkInterfaces::powerController;
            class PowerControllerCapabilityAgent : public CapabilityAgent, public PowerControllerObserverInterface, public RequiresShutdown,
                                                   public enable_shared_from_this<PowerControllerCapabilityAgent> {
            public:
                using PowerState = PowerControllerInterface::PowerState;
                static shared_ptr<PowerControllerCapabilityAgent> create(const EndpointIdentifier& endpointId, shared_ptr<PowerControllerInterface> powerController,
                                                                         shared_ptr<ContextManagerInterface> contextManager,
                                                                         shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                         shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                         bool isProactivelyReported, bool isRetrievable);
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                void provideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken) override;
                bool canStateBeRetrieved() override;
                bool hasReportableStateProperties() override;
                CapabilityConfiguration getCapabilityConfiguration();
                void onPowerStateChanged(const PowerState& powerState, AlexaStateChangeCauseType cause) override;
            private:
                PowerControllerCapabilityAgent(const EndpointIdentifier& endpointId, shared_ptr<PowerControllerInterface> powerController,
                                               shared_ptr<ContextManagerInterface> contextManager,
                                               shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                               shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender, bool isProactivelyReported,
                                               bool isRetrievable);
                void doShutdown() override;
                bool initialize();
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info);
                void executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type);
                void executeProvideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken);
                void executeSendResponseEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info, const pair<AlexaResponseType, string> result);
                CapabilityState buildCapabilityState(const PowerState& powerState);
                EndpointIdentifier m_endpointId;
                bool m_isProactivelyReported;
                bool m_isRetrievable;
                shared_ptr<PowerControllerInterface> m_powerController;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<AlexaInterfaceMessageSenderInterface> m_responseSender;
                Executor m_executor;
            };
        }
    }
}
#endif