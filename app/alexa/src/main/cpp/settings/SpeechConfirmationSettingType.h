#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SPEECHCONFIRMATIONSETTINGTYPE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SPEECHCONFIRMATIONSETTINGTYPE_H_

#include <istream>
#include <ostream>
#include <string>

namespace alexaClientSDK {
    namespace settings {
        enum class SpeechConfirmationSettingType {
            NONE,
            TONE
        };
        inline SpeechConfirmationSettingType getSpeechConfirmationDefault() {
            return SpeechConfirmationSettingType::NONE;
        }
        inline std::ostream& operator<<(std::ostream& stream, const SpeechConfirmationSettingType& value) {
            switch (value) {
                case SpeechConfirmationSettingType::NONE:
                    stream << "NONE";
                    return stream;
                case SpeechConfirmationSettingType::TONE:
                    stream << "TONE";
                    return stream;
            }
            stream.setstate(std::ios_base::failbit);
            return stream;
        }
        inline std::istream& operator>>(std::istream& is, SpeechConfirmationSettingType& value) {
            std::string str;
            is >> str;
            if ("NONE" == str) value = SpeechConfirmationSettingType::NONE;
            else if ("TONE" == str) value = SpeechConfirmationSettingType::TONE;
            else is.setstate(std::ios_base::failbit);
            return is;
        }
    }
}
#endif