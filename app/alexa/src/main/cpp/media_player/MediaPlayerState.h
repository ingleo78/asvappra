#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERSTATE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERSTATE_H_

#include <chrono>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                using namespace std;
                using namespace chrono;
                const milliseconds DURATION_UNKNOWN = milliseconds(-1);
                struct MediaPlayerState {
                    MediaPlayerState() : offset(milliseconds::zero()), duration(DURATION_UNKNOWN) {}
                    MediaPlayerState(milliseconds offsetInMs, milliseconds duration_ = DURATION_UNKNOWN) : offset(offsetInMs),
                                                                                                           duration(duration_) {}
                    milliseconds offset;
                    milliseconds duration;
                    bool operator==(const MediaPlayerState& other) const {
                        return offset == other.offset;
                    }
                };
            }
        }
    }
}
#endif