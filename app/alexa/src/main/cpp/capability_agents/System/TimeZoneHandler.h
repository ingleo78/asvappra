#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_TIMEZONEHANDLER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_TIMEZONEHANDLER_H_

#include <memory>
#include <string>
#include <avs/CapabilityAgent.h>
#include <sdkinterfaces/SystemTimeZoneInterface.h>
#include <threading/Executor.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/SettingEventMetadata.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace rapidjson;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace threading;
            using namespace settings;
            class TimeZoneHandler : public CapabilityAgent {
            public:
                static unique_ptr<TimeZoneHandler> create(shared_ptr<TimeZoneSetting> timeZoneSetting,
                                                          shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                static SettingEventMetadata getTimeZoneMetadata();
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
            private:
                void executeHandleDirectiveImmediately(shared_ptr<DirectiveInfo> info);
                void sendProcessingDirectiveException(const shared_ptr<AVSDirective>& directive, const string& errorMessage);
                bool handleSetTimeZone(const shared_ptr<AVSDirective>& directive, const Document& payload);
                TimeZoneHandler(shared_ptr<TimeZoneSetting> timeZoneSetting, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                shared_ptr<TimeZoneSetting> m_timeZoneSetting;
                Executor m_executor;
            };
        }
    }
}
#endif