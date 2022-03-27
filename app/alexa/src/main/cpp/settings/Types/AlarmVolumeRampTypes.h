#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_TYPES_ALARMVOLUMERAMPTYPES_H__
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_TYPES_ALARMVOLUMERAMPTYPES_H__

#include <istream>
#include <ostream>
#include <string>

namespace alexaClientSDK {
    namespace settings {
        namespace types {
            enum class AlarmVolumeRampTypes {
                NONE,
                ASCENDING
            };
            constexpr AlarmVolumeRampTypes getAlarmVolumeRampDefault() {
                return AlarmVolumeRampTypes::NONE;
            }
            constexpr bool isEnabled(AlarmVolumeRampTypes volumeRamp) {
                return volumeRamp != AlarmVolumeRampTypes::NONE;
            }
            constexpr AlarmVolumeRampTypes toAlarmRamp(bool enabled) {
                return enabled ? AlarmVolumeRampTypes::ASCENDING : AlarmVolumeRampTypes::NONE;
            }
            inline std::ostream& operator<<(std::ostream& stream, const AlarmVolumeRampTypes& value) {
                switch (value) {
                    case AlarmVolumeRampTypes::NONE:
                        stream << "NONE";
                        return stream;
                    case AlarmVolumeRampTypes::ASCENDING:
                        stream << "ASCENDING";
                        return stream;
                }
                stream.setstate(std::ios_base::failbit);
                return stream;
            }
            inline std::istream& operator>>(std::istream& is, AlarmVolumeRampTypes& value) {
                std::string str;
                is >> str;
                if ("NONE" == str) value = AlarmVolumeRampTypes::NONE;
                else if ("ASCENDING" == str) value = AlarmVolumeRampTypes::ASCENDING;
                else is.setstate(std::ios_base::failbit);
                return is;
            }
        }
    }
}
#endif