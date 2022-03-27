#ifndef ACSDKAUDIOPLAYERINTERFACES_AUDIOPLAYERINTERFACE_H_
#define ACSDKAUDIOPLAYERINTERFACES_AUDIOPLAYERINTERFACE_H_

#include <chrono>
#include <memory>
#include "AudioPlayerObserverInterface.h"

namespace alexaClientSDK {
    namespace acsdkAudioPlayerInterfaces {
        class AudioPlayerInterface {
        public:
            virtual ~AudioPlayerInterface() = default;
            virtual void addObserver(std::shared_ptr<acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface> observer) = 0;
            virtual void removeObserver(std::shared_ptr<acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface> observer) = 0;
            virtual void stopPlayback() = 0;
        };
    }
}
#endif