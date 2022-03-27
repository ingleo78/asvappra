#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_PLAYBACKCONTEXT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_PLAYBACKCONTEXT_H_

#include <map>
#include <string>
#include "util/PlatformDefinitions.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                typedef std::map<std::string, std::string> HeaderConfig;
                struct PlaybackContext {
                    static const avscommon_EXPORT std::string HTTP_HEADERS;
                    static const avscommon_EXPORT std::string HTTP_KEY_HEADERS;
                    static const avscommon_EXPORT std::string HTTP_MANIFEST_HEADERS;
                    static const avscommon_EXPORT std::string HTTP_AUDIOSEGMENT_HEADERS;
                    static const avscommon_EXPORT std::string HTTP_ALL_HEADERS;
                    HeaderConfig keyConfig;
                    HeaderConfig manifestConfig;
                    HeaderConfig audioSegmentConfig;
                    HeaderConfig allConfig;
                };
                bool validatePlaybackContextHeaders(PlaybackContext* playbackContext);
            }
        }
    }
}
#endif