#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGOBSERVERINTERFACE_H_

#include <functional>
#include <memory>
#include <ostream>

namespace alexaClientSDK {
    namespace settings {
        enum class SettingNotifications {
            LOCAL_CHANGE_IN_PROGRESS,
            AVS_CHANGE_IN_PROGRESS,
            LOCAL_CHANGE,
            AVS_CHANGE,
            LOCAL_CHANGE_FAILED,
            AVS_CHANGE_FAILED,
            LOCAL_CHANGE_CANCELLED,
            AVS_CHANGE_CANCELLED
        };
        inline std::ostream& operator<<(std::ostream& stream, const SettingNotifications& value) {
            switch (value) {
                case SettingNotifications::LOCAL_CHANGE_IN_PROGRESS:
                    stream << "LOCAL_CHANGE_IN_PROGRESS";
                    return stream;
                case SettingNotifications::AVS_CHANGE_IN_PROGRESS:
                    stream << "AVS_CHANGE_IN_PROGRESS";
                    return stream;
                case SettingNotifications::LOCAL_CHANGE:
                    stream << "LOCAL_CHANGE";
                    return stream;
                case SettingNotifications::AVS_CHANGE:
                    stream << "AVS_CHANGE";
                    return stream;
                case SettingNotifications::LOCAL_CHANGE_FAILED:
                    stream << "LOCAL_CHANGE_FAILED";
                    return stream;
                case SettingNotifications::AVS_CHANGE_FAILED:
                    stream << "AVS_CHANGE_FAILED";
                    return stream;
                case SettingNotifications::LOCAL_CHANGE_CANCELLED:
                    stream << "LOCAL_CHANGE_CANCELLED";
                    return stream;
                case SettingNotifications::AVS_CHANGE_CANCELLED:
                    stream << "AVS_CHANGE_CANCELLED";
                    return stream;
            }
            stream.setstate(std::ios_base::failbit);
            return stream;
        }
        template <typename SettingT> class SettingObserverInterface {
        public:
            virtual ~SettingObserverInterface() = default;
            virtual void onSettingNotification(const typename SettingT::ValueType& value, SettingNotifications notification) = 0;
            friend SettingT;
        };
    }
}

#endif  // ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGOBSERVERINTERFACE_H_
