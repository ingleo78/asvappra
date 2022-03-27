#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETSETTINGRESULT_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETSETTINGRESULT_H_

#include <ostream>

enum class SetSettingResult {
    NO_CHANGE,
    ENQUEUED,
    BUSY,
    UNAVAILABLE_SETTING,
    INVALID_VALUE,
    INTERNAL_ERROR,
    UNSUPPORTED_OPERATION
};
inline std::ostream& operator<<(std::ostream& stream, const SetSettingResult& value) {
    switch (value) {
        case SetSettingResult::NO_CHANGE:
            stream << "NO_CHANGE";
            return stream;
        case SetSettingResult::ENQUEUED:
            stream << "ENQUEUED";
            return stream;
        case SetSettingResult::BUSY:
            stream << "BUSY";
            return stream;
        case SetSettingResult::UNAVAILABLE_SETTING:
            stream << "UNAVAILABLE_SETTING";
            return stream;
        case SetSettingResult::INVALID_VALUE:
            stream << "INVALID_VALUE";
            return stream;
        case SetSettingResult::INTERNAL_ERROR:
            stream << "INTERNAL_ERROR";
            return stream;
        case SetSettingResult::UNSUPPORTED_OPERATION:
            stream << "UNSUPPORTED_OPERATION";
            return stream;
    }
    stream.setstate(std::ios_base::failbit);
    return stream;
}
#endif