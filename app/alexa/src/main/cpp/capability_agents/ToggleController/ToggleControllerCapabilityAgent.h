#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_TOGGLECONTROLLER_INCLUDE_TOGGLECONTROLLER_TOGGLECONTROLLERCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_TOGGLECONTROLLER_INCLUDE_TOGGLECONTROLLER_TOGGLECONTROLLERCAPABILITYAGENT_H_

#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/CapabilityState.h>
#include <sdkinterfaces/AlexaInterfaceMessageSenderInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ToggleController/ToggleControllerAttributes.h>
#include <sdkinterfaces/ToggleController/ToggleControllerInterface.h>
#include <sdkinterfaces/ToggleController/ToggleControllerObserverInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace toggleController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace endpoints;
            using namespace logger;
            using namespace threading;
            using namespace sdkInterfaces::toggleController;
            class ToggleControllerCapabilityAgent : public CapabilityAgent, public ToggleControllerObserverInterface, public RequiresShutdown,
                                                    public enable_shared_from_this<ToggleControllerCapabilityAgent> {
            public:
                using ToggleState = ToggleControllerInterface::ToggleState;
                using DirectiveInfo = CapabilityAgent::DirectiveInfo;
                static shared_ptr<ToggleControllerCapabilityAgent> create(const EndpointIdentifier& endpointId, const string& instance,
                                                                          const ToggleControllerAttributes& toggleControllerAttributes,
                                                                          shared_ptr<ToggleControllerInterface> toggleController,
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
                void onToggleStateChanged(const ToggleState& toggleState, AlexaStateChangeCauseType cause) override;
            private:
                ToggleControllerCapabilityAgent(const EndpointIdentifier& endpointId, const string& instance,
                                                const ToggleControllerAttributes& toggleControllerAttributes,
                                                shared_ptr<ToggleControllerInterface> toggleController,
                                                shared_ptr<ContextManagerInterface> contextManager,
                                                shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                bool isProactivelyReported, bool isRetrievable, bool isNonControllable = false);
                void doShutdown() override;
                bool initialize();
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info);
                void executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type);
                void executeProvideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken);
                void executeSendResponseEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info, pair<AlexaResponseType, string> result);
                CapabilityState buildCapabilityState(const ToggleState& toggleState);
                EndpointIdentifier m_endpointId;
                string m_instance;
                bool m_isProactivelyReported;
                bool m_isRetrievable;
                bool m_isNonControllable;
                ToggleControllerAttributes m_toggleControllerAttributes;
                shared_ptr<ToggleControllerInterface> m_toggleController;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<AlexaInterfaceMessageSenderInterface> m_responseSender;
                Executor m_executor;
            };
        }
    }
}
#endif