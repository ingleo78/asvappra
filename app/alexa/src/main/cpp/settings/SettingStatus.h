#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSTATUS_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSTATUS_H_

namespace alexaClientSDK {
    namespace settings {
        enum class SettingStatus {
            NOT_AVAILABLE,
            LOCAL_CHANGE_IN_PROGRESS,
            AVS_CHANGE_IN_PROGRESS,
            SYNCHRONIZED
        };
        inline std::string settingStatusToString(SettingStatus status) {
            switch (status) {
                case SettingStatus::LOCAL_CHANGE_IN_PROGRESS: return "LOCAL_CHANGE_IN_PROGRESS";
                case SettingStatus::AVS_CHANGE_IN_PROGRESS: return "AVS_CHANGE_IN_PROGRESS";
                case SettingStatus::SYNCHRONIZED: return "SYNCHRONIZED";
                case SettingStatus::NOT_AVAILABLE: return "NOT_AVAILABLE";
            }
            return "UNKNOWN";
        }
        inline SettingStatus stringToSettingStatus(const std::string& statusString) {
            if ("LOCAL_CHANGE_IN_PROGRESS" == statusString) return SettingStatus::LOCAL_CHANGE_IN_PROGRESS;
            else if ("SYNCHRONIZED" == statusString) return SettingStatus::SYNCHRONIZED;
            else if ("AVS_CHANGE_IN_PROGRESS" == statusString) return SettingStatus::AVS_CHANGE_IN_PROGRESS;
            return SettingStatus::NOT_AVAILABLE;
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSTATUS_H_
