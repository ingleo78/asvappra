#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CONTENTTYPE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CONTENTTYPE_H_

#include <iostream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class ContentType {
                MIXABLE,
                NONMIXABLE,
                UNDEFINED,
                NUM_CONTENT_TYPE
            };
            inline std::string_view contentTypeToString(ContentType contentType) {
                switch (contentType) {
                    case ContentType::MIXABLE: return "MIXABLE";
                    case ContentType::NONMIXABLE: return "NONMIXABLE";
                    default: return "UNDEFINED";
                }
            }
            inline std::ostream& operator<<(std::ostream& stream, const ContentType& contentType) {
                return stream << contentTypeToString(contentType).data();
            }
        }
    }
}
#endif