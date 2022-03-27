#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_PLAYBACKROUTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_PLAYBACKROUTERINTERFACE_H_

#include <memory>
#include <iostream>
#include "PlaybackHandlerInterface.h"
#include "LocalPlaybackHandlerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class PlaybackRouterInterface {
            public:
                virtual ~PlaybackRouterInterface() = default;
                virtual void buttonPressed(avsCommon::avs::PlaybackButton button);
                virtual void togglePressed(avsCommon::avs::PlaybackToggle toggle, bool action);
                virtual void setHandler(std::shared_ptr<PlaybackHandlerInterface> handler, std::shared_ptr<LocalPlaybackHandlerInterface> localHandler = nullptr);
                virtual void switchToDefaultHandler();
                virtual void useDefaultHandlerWith(std::shared_ptr<LocalPlaybackHandlerInterface> localHandler){};
                virtual bool localOperation(LocalPlaybackHandlerInterface::PlaybackOperation op) {
                    return false;
                };
                virtual bool localSeekTo(std::chrono::milliseconds location, bool fromStart) {
                    return false;
                };
            };
        }
    }
}
#endif