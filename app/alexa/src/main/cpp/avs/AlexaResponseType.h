#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ALEXARESPONSETYPE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ALEXARESPONSETYPE_H_

#include <ostream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class AlexaResponseType {
                SUCCESS,
                ALREADY_IN_OPERATION,
                BRIDGE_UNREACHABLE,
                ENDPOINT_BUSY,
                ENDPOINT_LOW_POWER,
                ENDPOINT_UNREACHABLE,
                FIRMWARE_OUT_OF_DATE,
                HARDWARE_MALFUNCTION,
                INSUFFICIENT_PERMISSIONS,
                INTERNAL_ERROR,
                INVALID_VALUE,
                NOT_CALIBRATED,
                NOT_SUPPORTED_IN_CURRENT_MODE,
                NOT_IN_OPERATION,
                POWER_LEVEL_NOT_SUPPORTED,
                RATE_LIMIT_EXCEEDED,
                TEMPERATURE_VALUE_OUT_OF_RANGE,
                VALUE_OUT_OF_RANGE
            };
            inline std::ostream& operator<<(std::ostream& stream, AlexaResponseType responseType) {
                switch (responseType) {
                    case AlexaResponseType::SUCCESS: return stream << "SUCCESS";
                    case AlexaResponseType::ALREADY_IN_OPERATION: return stream << "ALREADY_IN_OPERATION";
                    case AlexaResponseType::BRIDGE_UNREACHABLE: return stream << "BRIDGE_UNREACHABLE";
                    case AlexaResponseType::ENDPOINT_BUSY: return stream << "ENDPOINT_BUSY";
                    case AlexaResponseType::ENDPOINT_LOW_POWER: return stream << "ENDPOINT_LOW_POWER";
                    case AlexaResponseType::ENDPOINT_UNREACHABLE: return stream << "ENDPOINT_UNREACHABLE";
                    case AlexaResponseType::FIRMWARE_OUT_OF_DATE: return stream << "FIRMWARE_OUT_OF_DATE";
                    case AlexaResponseType::HARDWARE_MALFUNCTION: return stream << "HARDWARE_MALFUNCTION";
                    case AlexaResponseType::INSUFFICIENT_PERMISSIONS: return stream << "INSUFFICIENT_PERMISSIONS";
                    case AlexaResponseType::INTERNAL_ERROR: return stream << "INTERNAL_ERROR";
                    case AlexaResponseType::INVALID_VALUE: return stream << "INVALID_VALUE";
                    case AlexaResponseType::NOT_CALIBRATED: return stream << "NOT_CALIBRATED";
                    case AlexaResponseType::NOT_SUPPORTED_IN_CURRENT_MODE: return stream << "NOT_SUPPORTED_IN_CURRENT_MODE";
                    case AlexaResponseType::NOT_IN_OPERATION: return stream << "NOT_IN_OPERATION";
                    case AlexaResponseType::POWER_LEVEL_NOT_SUPPORTED: return stream << "POWER_LEVEL_NOT_SUPPORTED";
                    case AlexaResponseType::RATE_LIMIT_EXCEEDED: return stream << "RATE_LIMIT_EXCEEDED";
                    case AlexaResponseType::TEMPERATURE_VALUE_OUT_OF_RANGE: return stream << "TEMPERATURE_VALUE_OUT_OF_RANGE";
                    case AlexaResponseType::VALUE_OUT_OF_RANGE: return stream << "VALUE_OUT_OF_RANGE";
                }
                return stream << "Unknown AlexaResponseType";
            }
        }
    }
}
#endif