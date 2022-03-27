#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <json/document.h>
#include "ApiGatewayCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace apiGateway {
            using namespace json;
            using namespace jsonUtils;
            using namespace logger;
            static const string TAG{"ApiGateway"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE = "Alexa.ApiGateway";
            static const NamespaceAndName SET_GATEWAY_DIRECTIVE{NAMESPACE, "SetGateway"};
            static const string PAYLOAD_KEY_GATEWAY = "gateway";
            static const string APIGAETWAY_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string APIGAETWAY_CAPABILITY_INTERFACE_NAME = "Alexa.ApiGateway";
            static const string APIGAETWAY_CAPABILITY_INTERFACE_VERSION = "1.0";
            static shared_ptr<CapabilityConfiguration> getApiGatewayConfigurations() {
                return make_shared<CapabilityConfiguration>(APIGAETWAY_CAPABILITY_INTERFACE_TYPE, APIGAETWAY_CAPABILITY_INTERFACE_NAME,
                                                            APIGAETWAY_CAPABILITY_INTERFACE_VERSION);
            }
            shared_ptr<ApiGatewayCapabilityAgent> ApiGatewayCapabilityAgent::create(shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                                                                                    shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) {
                ACSDK_DEBUG5(LX(__func__));
                if (!avsGatewayManager) { ACSDK_ERROR(LX("createFailed").d("reason", "nullavsGatewayManager")); }
                else if (!exceptionEncounteredSender) { ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncounteredSender")); }
                else {
                    auto instance = shared_ptr<ApiGatewayCapabilityAgent>(new ApiGatewayCapabilityAgent(avsGatewayManager, exceptionEncounteredSender));
                    return instance;
                }
                return nullptr;
            }
            ApiGatewayCapabilityAgent::ApiGatewayCapabilityAgent(shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                                                                 shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                                                 CapabilityAgent{NAMESPACE, exceptionEncounteredSender}, RequiresShutdown("ApiGateway"),
                                                                 m_avsGatewayManager{avsGatewayManager} {
                m_capabilityConfigurations.insert(getApiGatewayConfigurations());
            }
            DirectiveHandlerConfiguration ApiGatewayCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                configuration[SET_GATEWAY_DIRECTIVE] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                return configuration;
            }
            std::unordered_set<std::shared_ptr<CapabilityConfiguration>> ApiGatewayCapabilityAgent::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            void ApiGatewayCapabilityAgent::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
            }
            void ApiGatewayCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
            }
            void ApiGatewayCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                m_executor.submit([this, info] { executeHandleDirective(info); });
            }
            void ApiGatewayCapabilityAgent::executeHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirective"));
                    return;
                }
                if (SET_GATEWAY_DIRECTIVE.name == info->directive->getName()) {
                    string newGateway;
                    if (!retrieveValue(info->directive->getPayload(), PAYLOAD_KEY_GATEWAY, &newGateway)) {
                        executeSendExceptionEncountered(info, "unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    } else {
                        if (!m_avsGatewayManager->setGatewayURL(newGateway)) {
                            ACSDK_ERROR(LX("executeHandleDirectiveFailed").d("reason", "failure to set gateway URL"));
                        }
                    }
                } else {
                    executeSendExceptionEncountered(info, "unknown directive", ExceptionErrorType::UNSUPPORTED_OPERATION);
                    return;
                }
                executeSetHandlingCompleted(info);
            }
            void ApiGatewayCapabilityAgent::removeDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (info && info->directive && info->result) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void ApiGatewayCapabilityAgent::executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (info && info->directive && info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void ApiGatewayCapabilityAgent::executeSendExceptionEncountered(shared_ptr<DirectiveInfo> info, const string& message,
                                                                            ExceptionErrorType errorType) {
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", message).d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));
                const string exceptionMessage = message + " " + info->directive->getNamespace() + ":" + info->directive->getName();
                sendExceptionEncounteredAndReportFailed(info, exceptionMessage, errorType);
            }
            void ApiGatewayCapabilityAgent::cancelDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                removeDirective(info);
            }
            void ApiGatewayCapabilityAgent::doShutdown() {
                m_executor.shutdown();
            }
        }
    }
}