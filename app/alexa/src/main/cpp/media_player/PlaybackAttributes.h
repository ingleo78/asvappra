#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_PLAYBACKATTRIBUTES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_PLAYBACKATTRIBUTES_H_

#include <chrono>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                struct PlaybackAttributes {
                    std::string name;
                    std::string codec;
                    long samplingRateInHertz;
                    long dataRateInBitsPerSecond;
                };
            }
        }
    }
}
#endif