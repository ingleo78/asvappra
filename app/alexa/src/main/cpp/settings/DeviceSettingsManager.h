#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_DEVICESETTINGSMANAGER_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_DEVICESETTINGSMANAGER_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include "Types/AlarmVolumeRampTypes.h"
#include "Types/NetworkInfo.h"
#include "SettingInterface.h"
#include "SettingsManager.h"
#include "SpeechConfirmationSettingType.h"
#include "WakeWordConfirmationSettingType.h"

namespace alexaClientSDK {
    namespace settings {
        using Locale = std::string;
        using DeviceLocales = std::vector<Locale>;
        using WakeWord = std::string;
        using WakeWords = std::set<WakeWord>;
        using DoNotDisturbSetting = SettingInterface<bool>;
        using AlarmVolumeRampSetting = SettingInterface<types::AlarmVolumeRampTypes>;
        using WakeWordConfirmationSetting = SettingInterface<WakeWordConfirmationSettingType>;
        using SpeechConfirmationSetting = SettingInterface<SpeechConfirmationSettingType>;
        using TimeZoneSetting = SettingInterface<std::string>;
        using WakeWordsSetting = SettingInterface<WakeWords>;
        using LocalesSetting = SettingInterface<DeviceLocales>;
        using NetworkInfoSetting = SettingInterface<types::NetworkInfo>;
        enum DeviceSettingsIndex {
            DO_NOT_DISTURB,
            ALARM_VOLUME_RAMP,
            WAKEWORD_CONFIRMATION,
            SPEECH_CONFIRMATION,
            TIMEZONE,
            WAKE_WORDS,
            LOCALE,
            NETWORK_INFO
        };
        using DeviceSettingsManager = SettingsManager<DoNotDisturbSetting, AlarmVolumeRampSetting, WakeWordConfirmationSetting, SpeechConfirmationSetting,
                                                      TimeZoneSetting, WakeWordsSetting, LocalesSetting, NetworkInfoSetting>;
    }
}
#endif