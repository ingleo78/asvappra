#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_LOCALPLAYBACKHANDLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_LOCALPLAYBACKHANDLERINTERFACE_H_

#include <chrono>
#include <avs/PlaybackButtons.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class LocalPlaybackHandlerInterface {
            public:
                virtual ~LocalPlaybackHandlerInterface() = default;
                enum PlaybackOperation { STOP_PLAYBACK, PAUSE_PLAYBACK, RESUME_PLAYBACK };
                virtual bool localOperation(PlaybackOperation op);
                virtual bool localSeekTo(std::chrono::milliseconds location, bool fromStart);
            };
        }
    }
}
#endif