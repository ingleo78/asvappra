#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_ALEXA_INCLUDE_ALEXA_ALEXAINTERFACEMESSAGESENDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_ALEXA_INCLUDE_ALEXA_ALEXAINTERFACEMESSAGESENDER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <capability_agents/Alexa/AlexaInterfaceMessageSenderInternalInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ContextRequesterInterface.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace alexa {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace threading;
            class AlexaInterfaceMessageSender : public AlexaInterfaceMessageSenderInternalInterface, public ContextManagerObserverInterface,
                                                public ContextRequesterInterface, public MessageRequestObserverInterface, public RequiresShutdown,
                                                public enable_shared_from_this<AlexaInterfaceMessageSender> {
            public:
                ~AlexaInterfaceMessageSender();
                static shared_ptr<AlexaInterfaceMessageSender> create(shared_ptr<ContextManagerInterface> contextManager,
                                                                      shared_ptr<MessageSenderInterface> messageSender);
                void onStateChanged(const CapabilityTag& identifier, const CapabilityState& state, const AlexaStateChangeCauseType cause) override;
                void onContextAvailable(const string& endpointId, const AVSContext& endpointContext, ContextRequestToken token) override;
                void onContextFailure(const ContextRequestError error, ContextRequestToken token) override;
                void onSendCompleted(MessageRequestObserverInterface::Status status) override;
                void onExceptionReceived(const string& exceptionMessage) override;
                virtual bool sendStateReportEvent(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint) override;
                virtual bool sendResponseEvent(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                               const string& jsonPayload = "{}");
                virtual bool sendErrorResponseEvent(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                                    const ErrorResponseType errorType, const string& errorMessage = "");
                virtual bool sendDeferredResponseEvent(const string& instance, const string& correlationToken, const int estimatedDeferralInSeconds = 0) override;
            private:
                struct ResponseData {
                    ResponseData(const string& type, const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                 const string& jsonPayload);
                    const string type;
                    const string instance;
                    const string correlationToken;
                    const AVSMessageEndpoint endpoint;
                    const string jsonPayload;
                };
                struct ChangeReportData {
                    ChangeReportData(const CapabilityTag& tag, const CapabilityState& state, const AlexaStateChangeCauseType& cause);
                    const CapabilityTag tag;
                    const CapabilityState state;
                    const AlexaStateChangeCauseType cause;
                };
                AlexaInterfaceMessageSender(shared_ptr<ContextManagerInterface> contextManager, shared_ptr<MessageSenderInterface> messageSender);
                bool initialize();
                void doShutdown() override;
                bool sendCommonResponseEvent(const string& type, const string& instance, const string& correlationToken,
                                             const AVSMessageEndpoint& endpoint, const string& jsonPayload = "{}");
                void completeResponseEvent(const shared_ptr<ResponseData>& event, const Optional<AVSContext>& context = Optional<AVSContext>());
                void completeChangeReportEvent(const shared_ptr<ChangeReportData>& event, const AVSContext& context);
                void sendEvent(const string& eventJson);
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<MessageSenderInterface> m_messageSender;
                map<ContextRequestToken, shared_ptr<ResponseData>> m_pendingResponses;
                map<ContextRequestToken, shared_ptr<ChangeReportData>> m_pendingChangeReports;
                Executor m_executor;
            };
        }
    }
}
#endif