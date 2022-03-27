#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ENDIAN_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ENDIAN_H_

#include <stdint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            inline bool littleEndianMachine() {
                uint16_t original = 1;
                uint8_t* words = reinterpret_cast<uint8_t*>(&original);
                return words[0] == 1;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ENDIAN_H_
