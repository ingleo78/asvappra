#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGEVENTMETADATA_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGEVENTMETADATA_H_

#include <string>

namespace alexaClientSDK {
    namespace settings {
        struct SettingEventMetadata {
            std::string eventNamespace;
            std::string eventChangedName;
            std::string eventReportName;
            std::string settingName;
        };
    }
}
#endif