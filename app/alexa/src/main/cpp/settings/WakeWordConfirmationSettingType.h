#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_WAKEWORDCONFIRMATIONSETTINGTYPE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_WAKEWORDCONFIRMATIONSETTINGTYPE_H_

#include <string>
#include <ostream>
#include <istream>

namespace alexaClientSDK {
    namespace settings {
        enum class WakeWordConfirmationSettingType {
            NONE,
            TONE
        };
        inline WakeWordConfirmationSettingType getWakeWordConfirmationDefault() {
            return WakeWordConfirmationSettingType::NONE;
        }
        inline std::ostream& operator<<(std::ostream& stream, const WakeWordConfirmationSettingType& value) {
            switch (value) {
                case WakeWordConfirmationSettingType::NONE:
                    stream << "NONE";
                    return stream;
                case WakeWordConfirmationSettingType::TONE:
                    stream << "TONE";
                    return stream;
            }
            stream.setstate(std::ios_base::failbit);
            return stream;
        }
        inline std::istream& operator>>(std::istream& is, WakeWordConfirmationSettingType& value) {
            std::string str;
            is >> str;
            if ("NONE" == str) value = WakeWordConfirmationSettingType::NONE;
            else if ("TONE" == str) value = WakeWordConfirmationSettingType::TONE;
            else is.setstate(std::ios_base::failbit);
            return is;
        }
    }
}
#endif