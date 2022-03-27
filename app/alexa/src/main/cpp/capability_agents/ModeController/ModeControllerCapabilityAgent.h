#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_MODECONTROLLER_INCLUDE_MODECONTROLLER_MODECONTROLLERCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_MODECONTROLLER_INCLUDE_MODECONTROLLER_MODECONTROLLERCAPABILITYAGENT_H_

#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/CapabilityState.h>
#include <avs/CapabilityResources.h>
#include <sdkinterfaces/AlexaInterfaceMessageSenderInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ModeController/ModeControllerAttributes.h>
#include <sdkinterfaces/ModeController/ModeControllerInterface.h>
#include <sdkinterfaces/ModeController/ModeControllerObserverInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace modeController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace endpoints;
            using namespace utils;
            using namespace rapidjson;
            using namespace threading;
            using namespace sdkInterfaces::modeController;
            class ModeControllerCapabilityAgent : public CapabilityAgent, public ModeControllerObserverInterface, public RequiresShutdown,
                                                  public enable_shared_from_this<ModeControllerCapabilityAgent> {
            public:
                using EndpointIdentifier = EndpointIdentifier;
                using ModeState = ModeControllerInterface::ModeState;
                static shared_ptr<ModeControllerCapabilityAgent> create(const EndpointIdentifier& endpointId, const string& instance,
                                                                        const ModeControllerAttributes& modeControllerAttributes,
                                                                        shared_ptr<ModeControllerInterface> modeController,
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
                void onModeChanged(const ModeState& mode, AlexaStateChangeCauseType cause) override;
            private:
                ModeControllerCapabilityAgent(const EndpointIdentifier& endpointId, const string& instance, const ModeControllerAttributes& modeControllerAttributes,
                                              shared_ptr<ModeControllerInterface> modeController, shared_ptr<ContextManagerInterface> contextManager,
                                              shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender, shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                              bool isProactivelyReported, bool isRetrievable, bool isNonControllable);
                void doShutdown() override;
                bool initialize();
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info);
                std::string buildModeConfigurationJson();
                void executeSetModeDirective(shared_ptr<DirectiveInfo> info, Document& payload);
                void executeAdjustModeDirective(shared_ptr<DirectiveInfo> info, Document& payload);
                void executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type);
                bool validateMode(const string& mode);
                void executeSetMode(const shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& payload);
                void executeAdjustMode(const shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& payload);
                void executeProvideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken);
                void executeSendResponseEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info, pair<AlexaResponseType, std::string> result);
                CapabilityState buildCapabilityState(const ModeState& modeState);
                EndpointIdentifier m_endpointId;
                string m_instance;
                bool m_isProactivelyReported;
                bool m_isRetrievable;
                bool m_isNonControllable;
                ModeControllerAttributes m_modeControllerAttributes;
                ModeControllerInterface::ModeControllerConfiguration m_modeControllerConfiguration;
                shared_ptr<ModeControllerInterface> m_modeController;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<AlexaInterfaceMessageSenderInterface> m_responseSender;
                using ModeAndCauseTypePair = pair<ModeState, AlexaStateChangeCauseType>;
                map<ContextRequestToken, ModeAndCauseTypePair> m_pendingChangeReports;
                Executor m_executor;
            };
        }
    }
}
#endif