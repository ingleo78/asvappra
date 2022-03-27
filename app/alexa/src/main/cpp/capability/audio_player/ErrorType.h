#ifndef ACSDKAUDIOPLAYER_ERRORTYPE_H_
#define ACSDKAUDIOPLAYER_ERRORTYPE_H_

#include <ostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                enum class ErrorType {
                    MEDIA_ERROR_UNKNOWN,
                    MEDIA_ERROR_INVALID_REQUEST,
                    MEDIA_ERROR_SERVICE_UNAVAILABLE,
                    MEDIA_ERROR_INTERNAL_SERVER_ERROR,
                    MEDIA_ERROR_INTERNAL_DEVICE_ERROR
                };
                inline std::string errorTypeToString(ErrorType errorType) {
                    switch(errorType) {
                        case ErrorType::MEDIA_ERROR_UNKNOWN: return "MEDIA_ERROR_UNKNOWN";
                        case ErrorType::MEDIA_ERROR_INVALID_REQUEST: return "MEDIA_ERROR_INVALID_REQUEST";
                        case ErrorType::MEDIA_ERROR_SERVICE_UNAVAILABLE: return "MEDIA_ERROR_SERVICE_UNAVAILABLE";
                        case ErrorType::MEDIA_ERROR_INTERNAL_SERVER_ERROR: return "MEDIA_ERROR_INTERNAL_SERVER_ERROR";
                        case ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR: return "MEDIA_ERROR_INTERNAL_DEVICE_ERROR";
                    }
                    return "unknown ErrorType";
                }
                inline std::ostream& operator<<(std::ostream& stream, const ErrorType& errorType) {
                    return stream << errorTypeToString(errorType);
                }
            }
        }
    }
}
#endif