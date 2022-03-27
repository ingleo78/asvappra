#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_SPEAKERCONSTANTS_SPEAKERCONSTANTS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_SPEAKERCONSTANTS_SPEAKERCONSTANTS_H_

#include <cstdint>
#include <stdint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace speakerConstants {
                const int8_t AVS_SET_VOLUME_MIN = 0;
                const int8_t AVS_SET_VOLUME_MAX = 100;
                const int8_t AVS_ADJUST_VOLUME_MIN = -100;
                const int8_t AVS_ADJUST_VOLUME_MAX = 100;
                const int8_t MIN_UNMUTE_VOLUME = 10;
            }
        }
    }
}
#endif