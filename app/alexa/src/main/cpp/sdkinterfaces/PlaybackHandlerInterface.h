#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_PLAYBACKHANDLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_PLAYBACKHANDLERINTERFACE_H_

#include <avs/PlaybackButtons.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class PlaybackHandlerInterface {
            public:
                virtual ~PlaybackHandlerInterface() = default;
                virtual void onButtonPressed(alexaClientSDK::avsCommon::avs::PlaybackButton button);
                virtual void onTogglePressed(alexaClientSDK::avsCommon::avs::PlaybackToggle toggle, bool action);
            };
        }
    }
}
#endif