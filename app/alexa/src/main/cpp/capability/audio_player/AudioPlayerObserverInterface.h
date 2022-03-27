#ifndef ACSDKAUDIOPLAYERINTERFACES_AUDIOPLAYEROBSERVERINTERFACE_H_
#define ACSDKAUDIOPLAYERINTERFACES_AUDIOPLAYEROBSERVERINTERFACE_H_

#include <chrono>
#include <string>
#include <avs/PlayerActivity.h>
#include <avs/PlayRequestor.h>

namespace alexaClientSDK {
    namespace acsdkAudioPlayerInterfaces {
        class AudioPlayerObserverInterface {
        public:
            virtual ~AudioPlayerObserverInterface() = default;
            struct Context {
                std::string audioItemId;
                std::chrono::milliseconds offset;
                avsCommon::avs::PlayRequestor playRequestor;
            };
            virtual void onPlayerActivityChanged(avsCommon::avs::PlayerActivity state, const Context& context) = 0;
        };
    }
}
#endif