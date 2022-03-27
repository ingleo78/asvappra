#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGPROTOCOLINTERFACE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGPROTOCOLINTERFACE_H_

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <threading/Executor.h>
#include "SetSettingResult.h"
#include "SettingObserverInterface.h"

namespace alexaClientSDK {
    namespace settings {
        class SettingProtocolInterface {
        public:
            using ApplyChangeFunction = std::function<std::pair<bool, std::string>()>;
            using ApplyDbChangeFunction = std::function<std::pair<bool, std::string>(const std::string& dbValue)>;
            using SettingNotificationFunction = std::function<void(SettingNotifications notification)>;
            using RevertChangeFunction = std::function<std::string()>;
            virtual SetSettingResult localChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers);
            virtual bool avsChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers);
            virtual bool restoreValue(ApplyDbChangeFunction applyChange, SettingNotificationFunction notifyObservers);
            virtual bool clearData();
            virtual ~SettingProtocolInterface() = default;
        };
    }
}
#endif