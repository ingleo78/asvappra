#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_APIGATEWAY_INCLUDE_APIGATEWAY_APIGATEWAYCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_APIGATEWAY_INCLUDE_APIGATEWAY_APIGATEWAYCAPABILITYAGENT_H_

#include <memory>
#include <avs/CapabilityAgent.h>
#include <sdkinterfaces/AVSGatewayManagerInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace apiGateway {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace threading;
            class ApiGatewayCapabilityAgent : public CapabilityAgent, public CapabilityConfigurationInterface, public RequiresShutdown {
            public:
                static shared_ptr<ApiGatewayCapabilityAgent> create(shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                                                                    shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                unordered_set<shared_ptr<avsCommon::avs::CapabilityConfiguration>> getCapabilityConfigurations() override;
                void doShutdown() override;
            private:
                ApiGatewayCapabilityAgent(shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                                          shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                void executeHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info);
                void executeSetHandlingCompleted(shared_ptr<CapabilityAgent::DirectiveInfo> info);
                void removeDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info);
                void executeSendExceptionEncountered(shared_ptr<DirectiveInfo> info, const string& errorMessage, ExceptionErrorType errorType);
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                shared_ptr<AVSGatewayManagerInterface> m_avsGatewayManager;
                Executor m_executor;
            };
        }
    }
}
#endif