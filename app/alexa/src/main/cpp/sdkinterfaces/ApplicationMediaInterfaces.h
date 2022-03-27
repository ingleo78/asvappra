#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_APPLICATIONMEDIAINTERFACES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_APPLICATIONMEDIAINTERFACES_H_

#include "Audio/EqualizerInterface.h"
#include "SpeakerInterface.h"
#include "../media_player/MediaPlayerInterface.h"
#include "../util/RequiresShutdown.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace utils;
            using namespace mediaPlayer;
            using namespace audio;
            struct ApplicationMediaInterfaces {
                ApplicationMediaInterfaces(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<SpeakerInterface> speaker,
                                           shared_ptr<EqualizerInterface> equalizer, shared_ptr<RequiresShutdown> requiresShutdown);
                std::shared_ptr<MediaPlayerInterface> mediaPlayer;
                std::shared_ptr<SpeakerInterface> speaker;
                std::shared_ptr<EqualizerInterface> equalizer;
                std::shared_ptr<RequiresShutdown> requiresShutdown;
            };
            inline ApplicationMediaInterfaces::ApplicationMediaInterfaces(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<SpeakerInterface> speaker,
                                                                          shared_ptr<EqualizerInterface> equalizer, shared_ptr<RequiresShutdown> requiresShutdown) :
                                                                          mediaPlayer{mediaPlayer}, speaker{speaker}, equalizer{equalizer},
                                                                          requiresShutdown{requiresShutdown} {}
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_APPLICATIONMEDIAINTERFACES_H_
