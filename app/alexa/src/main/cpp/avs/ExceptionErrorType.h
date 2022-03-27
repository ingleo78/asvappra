#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EXCEPTIONERRORTYPE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EXCEPTIONERRORTYPE_H_

#include <iostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class ExceptionErrorType {
                UNEXPECTED_INFORMATION_RECEIVED,
                UNSUPPORTED_OPERATION,
                INTERNAL_ERROR
            };
            inline std::ostream& operator<<(std::ostream& stream, ExceptionErrorType type) {
                switch(type) {
                    case ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED: stream << "UNEXPECTED_INFORMATION_RECEIVED"; break;
                    case ExceptionErrorType::UNSUPPORTED_OPERATION: stream << "UNSUPPORTED_OPERATION"; break;
                    case ExceptionErrorType::INTERNAL_ERROR: stream << "INTERNAL_ERROR"; break;
                }
                return stream;
            }
        }
    }
}
#endif