#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIATYPE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIATYPE_H_

#include <iostream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            enum class MediaType {
                MPEG,
                WAV,
                UNKNOWN
            };
            inline MediaType MimeTypeToMediaType(const std::string& mimetype) {
                if (mimetype == "audio/mpeg" || mimetype == "audio/x-mpeg") return MediaType::MPEG;
                else if (mimetype == "audio/wav" || mimetype == "audio/x-wav") return MediaType::WAV;
                return MediaType::UNKNOWN;
            }
            inline std::ostream& operator<<(std::ostream& stream, const MediaType& mediatype) {
                switch (mediatype) {
                    case MediaType::MPEG: stream << "MPEG"; break;
                    case MediaType::WAV: stream << "WAV"; break;
                    case MediaType::UNKNOWN:
                    default: stream << "UNKNOWN"; break;
                }
                return stream;
            }
        }
    }
}
#endif