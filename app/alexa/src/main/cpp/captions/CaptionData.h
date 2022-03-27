#ifndef ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONDATA_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONDATA_H_

#include <string>
#include <utility>
#include "CaptionFormat.h"

namespace alexaClientSDK {
    namespace captions {
        struct CaptionData {
            CaptionData(const CaptionFormat format = CaptionFormat::UNKNOWN, const std::string& content = "") : format{format}, content{content} {}
            bool isValid() const;
            bool operator==(const CaptionData& rhs) const {
                return (format == rhs.format) && (content == rhs.content);
            }
            bool operator!=(const CaptionData& rhs) const {
                return !(rhs == *this);
            }
            CaptionFormat format;
            std::string content;
        };
    }
}
#endif