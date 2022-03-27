#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_UUIDGENERATION_UUIDGENERATION_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_UUIDGENERATION_UUIDGENERATION_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace uuidGeneration {
                const std::string generateUUID();
                void setSalt(const std::string& newSalt);
            }
        }
    }
}
#endif