#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_REVOKEAUTHORIZATIONHANDLER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_REVOKEAUTHORIZATIONHANDLER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <avs/CapabilityAgent.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/RevokeAuthorizationObserverInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace json;
            class RevokeAuthorizationHandler : public avsCommon::avs::CapabilityAgent {
            public:
                static std::shared_ptr<RevokeAuthorizationHandler> create(
                    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(std::shared_ptr<avsCommon::avs::AVSDirective> directive) override;
                void preHandleDirective(std::shared_ptr<avsCommon::avs::CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(std::shared_ptr<avsCommon::avs::CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(std::shared_ptr<avsCommon::avs::CapabilityAgent::DirectiveInfo> info) override;
                bool addObserver(std::shared_ptr<avsCommon::sdkInterfaces::RevokeAuthorizationObserverInterface> observer);
                bool removeObserver(std::shared_ptr<avsCommon::sdkInterfaces::RevokeAuthorizationObserverInterface> observer);
            private:
                RevokeAuthorizationHandler(
                    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                void removeDirectiveGracefully(
                    std::shared_ptr<avsCommon::avs::CapabilityAgent::DirectiveInfo> info,
                    bool isFailure = false,
                    const std::string& report = "");
                std::mutex m_mutex;
                std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::RevokeAuthorizationObserverInterface>>
                    m_revokeObservers;
            };
        }
    }
}
#endif