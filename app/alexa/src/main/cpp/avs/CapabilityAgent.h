#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYAGENT_H_

#include <atomic>
#include <unordered_map>
#include <string>
#include <memory>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/ChannelObserverInterface.h>
#include <sdkinterfaces/ContextRequesterInterface.h>
#include <sdkinterfaces/StateProviderInterface.h>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <sdkinterfaces/DirectiveHandlerResultInterface.h>
#include "NamespaceAndName.h"
#include "ExceptionErrorType.h"
#include "FocusState.h"
#include "MixingBehavior.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace avs;
            using namespace sdkInterfaces;
            class CapabilityAgent : public DirectiveHandlerInterface, public ChannelObserverInterface, public StateProviderInterface, public ContextRequesterInterface {
            public:
                CapabilityAgent(const std::string& nameSpace, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                virtual ~CapabilityAgent() = default;
                void preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result) override final;
                bool handleDirective(const string& messageId) override final;
                void cancelDirective(const string& messageId) override final;
                void onDeregistered() override;
                void onFocusChanged(FocusState newFocus, MixingBehavior behavior);
            protected:
                class DirectiveInfo {
                public:
                    DirectiveInfo(shared_ptr<AVSDirective> directiveIn, unique_ptr<DirectiveHandlerResultInterface> resultIn);
                    virtual ~DirectiveInfo() = default;
                    shared_ptr<AVSDirective> directive;
                    shared_ptr<DirectiveHandlerResultInterface> result;
                    atomic<bool> isCancelled;
                };
                virtual shared_ptr<DirectiveInfo> createDirectiveInfo(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result);
                virtual void preHandleDirective(shared_ptr<DirectiveInfo> info);
                virtual void handleDirective(shared_ptr<DirectiveInfo> info);
                virtual void cancelDirective(shared_ptr<DirectiveInfo> info);
                void removeDirective(const string& messageId);
                void sendExceptionEncounteredAndReportFailed(shared_ptr<DirectiveInfo> info, const string& message, ExceptionErrorType type = ExceptionErrorType::INTERNAL_ERROR);
                const pair<string, string> buildJsonEventString(const string& eventName, const string& dialogRequestIdString = "", const string& payload = "{}",
                                                                const string& context = "") const;
                const string m_namespace;
                shared_ptr<ExceptionEncounteredSenderInterface> m_exceptionEncounteredSender;
            private:
                shared_ptr<DirectiveInfo> getDirectiveInfo(const string& messageId);
                unordered_map<string, shared_ptr<DirectiveInfo>> m_directiveInfoMap;
                mutex m_mutex;
            };
        }
    }
}
#endif