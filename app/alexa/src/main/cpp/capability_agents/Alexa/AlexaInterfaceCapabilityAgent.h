#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_ALEXA_INCLUDE_ALEXA_ALEXAINTERFACECAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_ALEXA_INCLUDE_ALEXA_ALEXAINTERFACECAPABILITYAGENT_H_

#include <memory>
#include <mutex>
#include <set>
#include <capability_agents/Alexa/AlexaInterfaceMessageSenderInternalInterface.h>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/AlexaEventProcessedObserverInterface.h>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <util/DeviceInfo.h>
#include <threading/Executor.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace alexa {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace endpoints;
            using namespace utils;
            using namespace threading;
            using namespace logger;
            class AlexaInterfaceCapabilityAgent : public CapabilityAgent {
            public:
                static shared_ptr<AlexaInterfaceCapabilityAgent> create(const DeviceInfo& deviceInfo, const EndpointIdentifier& endpointId,
                                                                        shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                        shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                CapabilityConfiguration getCapabilityConfiguration();
                void addEventProcessedObserver(const shared_ptr<AlexaEventProcessedObserverInterface>& observer);
                void removeEventProcessedObserver(const shared_ptr<AlexaEventProcessedObserverInterface>& observer);
            private:
                AlexaInterfaceCapabilityAgent(const DeviceInfo& deviceInfo, const EndpointIdentifier& endpointId,
                                              shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                              shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender);
                bool executeHandleEventProcessed(const shared_ptr<AVSDirective>& directive);
                void removeDirective(const shared_ptr<CapabilityAgent::DirectiveInfo>& info);
                void executeSendExceptionEncounteredAndReportFailed(const shared_ptr<CapabilityAgent::DirectiveInfo>& info, ExceptionErrorType errorType,
                                                                    const string& errorMessage);
                void executeSendErrorResponse(const shared_ptr<CapabilityAgent::DirectiveInfo>& info, AlexaInterfaceMessageSenderInterface::ErrorResponseType errorType,
                                              const string& errorMessage);
                const DeviceInfo m_deviceInfo;
                EndpointIdentifier m_endpointId;
                shared_ptr<AlexaInterfaceMessageSenderInternalInterface> m_alexaMessageSender;
                mutex m_observerMutex;
                set<shared_ptr<AlexaEventProcessedObserverInterface>> m_observers;
                Executor m_executor;
            };
        }
    }
}
#endif