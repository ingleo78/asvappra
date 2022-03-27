#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_LOCALEHANDLER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_LOCALEHANDLER_H_

#include <memory>
#include <string>
#include <avs/CapabilityAgent.h>
#include <threading/Executor.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/SettingEventMetadata.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace settings;
            using namespace threading;
            class LocaleHandler : public CapabilityAgent {
            public:
                static unique_ptr<LocaleHandler> create(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                        shared_ptr<LocalesSetting> localeSetting);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                static SettingEventMetadata getLocaleEventsMetadata();
            private:
                void executeHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info);
                void sendProcessingDirectiveException(shared_ptr<CapabilityAgent::DirectiveInfo> info, const string& errorMessage);
                void handleSetLocale(shared_ptr<CapabilityAgent::DirectiveInfo> info);
                LocaleHandler(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender, shared_ptr<LocalesSetting> localeSetting);
                shared_ptr<LocalesSetting> m_localeSetting;
                Executor m_executor;
            };
        }
    }
}
#endif