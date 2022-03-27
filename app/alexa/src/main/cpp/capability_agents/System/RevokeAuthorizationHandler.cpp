#include <string>
#include <json/document.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "RevokeAuthorizationHandler.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace rapidjson;
            static const string TAG{"RevokeAuthorizationHandler"};
            #define LX(event) LogEntry(TAG, event)
            static const string REVOKE_NAMESPACE = "System";
            static const string REVOKE_DIRECTIVE_NAME = "RevokeAuthorization";
            void RevokeAuthorizationHandler::removeDirectiveGracefully(shared_ptr<CapabilityAgent::DirectiveInfo> info, bool isFailure,
                                                                       const string& report) {
                if (info) {
                    if (info->result) {
                        if (isFailure) info->result->setFailed(report);
                        else info->result->setCompleted();
                        if (info->directive) CapabilityAgent::removeDirective(info->directive->getMessageId());
                    }
                }
            }
            shared_ptr<RevokeAuthorizationHandler> RevokeAuthorizationHandler::create(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) {
                ACSDK_DEBUG5(LX(__func__));
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncounteredSender"));
                    return nullptr;
                }
                return shared_ptr<RevokeAuthorizationHandler>(new RevokeAuthorizationHandler(exceptionEncounteredSender));
            }
            RevokeAuthorizationHandler::RevokeAuthorizationHandler(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                                                   CapabilityAgent(REVOKE_NAMESPACE, exceptionEncounteredSender) {}
            DirectiveHandlerConfiguration RevokeAuthorizationHandler::getConfiguration() const {
                return DirectiveHandlerConfiguration {
                           {NamespaceAndName{REVOKE_NAMESPACE, REVOKE_DIRECTIVE_NAME},
                            BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)}
                       };
            }
            void RevokeAuthorizationHandler::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            void RevokeAuthorizationHandler::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                handleDirective(make_shared<CapabilityAgent::DirectiveInfo>(directive, nullptr));
            }
            void RevokeAuthorizationHandler::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveOrDirectiveInfo"));
                    return;
                }
                unique_lock<mutex> lock(m_mutex);
                auto observers = m_revokeObservers;
                lock.unlock();
                for (auto observer : observers) {
                    observer->onRevokeAuthorization();
                }
                removeDirectiveGracefully(info);
            }
            void RevokeAuthorizationHandler::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (!info || !info->directive) {
                    removeDirectiveGracefully(info, true, "nullDirective");
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "nullDirectiveOrDirectiveInfo"));
                    return;
                }
                CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            bool RevokeAuthorizationHandler::addObserver(shared_ptr<RevokeAuthorizationObserverInterface> observer) {
                ACSDK_DEBUG5(LX(__func__));
                if (!observer) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                    return false;
                }
                lock_guard<mutex> lock(m_mutex);
                return m_revokeObservers.insert(observer).second;
            }
            bool RevokeAuthorizationHandler::removeObserver(shared_ptr<RevokeAuthorizationObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                    return false;
                }
                lock_guard<mutex> lock(m_mutex);
                return m_revokeObservers.erase(observer) == 1;
            }
        }
    }
}