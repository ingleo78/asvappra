#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_PLAYBACKREPORT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_PLAYBACKREPORT_H_

#include <chrono>
#include "PlaybackAttributes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                struct PlaybackReport {
                    std::chrono::milliseconds startOffset;
                    std::chrono::milliseconds endOffset;
                    PlaybackAttributes playbackAttributes;
                };
            }
        }
    }
}
#endif