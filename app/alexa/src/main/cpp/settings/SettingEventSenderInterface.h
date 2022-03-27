#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGEVENTSENDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGEVENTSENDERINTERFACE_H_

#include <future>
#include <string>

namespace alexaClientSDK {
    namespace settings {
        class SettingEventSenderInterface {
        public:
            virtual ~SettingEventSenderInterface() = default;
            virtual std::shared_future<bool> sendChangedEvent(const std::string& value);
            virtual std::shared_future<bool> sendReportEvent(const std::string& value);
            virtual std::shared_future<bool> sendStateReportEvent(const std::string& payload);
            virtual void cancel();
        };
    }
}
#endif