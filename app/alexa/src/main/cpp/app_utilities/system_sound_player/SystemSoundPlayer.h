#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_SYSTEMSOUNDPLAYER_INCLUDE_SYSTEMSOUNDPLAYER_SYSTEMSOUNDPLAYER_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_SYSTEMSOUNDPLAYER_INCLUDE_SYSTEMSOUNDPLAYER_SYSTEMSOUNDPLAYER_H_

#include <future>
#include <memory>
#include <sdkinterfaces/Audio/SystemSoundAudioFactoryInterface.h>
#include <sdkinterfaces/SystemSoundPlayerInterface.h>
#include <media_player/MediaPlayerInterface.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <util/MediaType.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace systemSoundPlayer {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace audio;
            using namespace mediaPlayer;
            class SystemSoundPlayer : public SystemSoundPlayerInterface, public MediaPlayerObserverInterface {
            public:
                static shared_ptr<SystemSoundPlayer> create(shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                            shared_ptr<SystemSoundAudioFactoryInterface> soundPlayerAudioFactory);
                shared_future<bool> playTone(Tone tone) override;
                void onPlaybackFinished(SourceId id, const MediaPlayerState& state) override;
                void onPlaybackError(SourceId id, const ErrorType& type, string error, const MediaPlayerState& state) override;
                void onPlaybackStarted(SourceId id, const avsCommon::utils::mediaPlayer::MediaPlayerState& state) override;
                void onFirstByteRead(SourceId id, const avsCommon::utils::mediaPlayer::MediaPlayerState& state) override;
            private:
                SystemSoundPlayer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<SystemSoundAudioFactoryInterface> soundPlayerAudioFactory);
                void finishPlayTone(bool result);
                shared_ptr<MediaPlayerInterface> m_mediaPlayer;
                shared_ptr<SystemSoundAudioFactoryInterface> m_soundPlayerAudioFactory;
                shared_future<bool> m_sharedFuture;
                promise<bool> m_playTonePromise;
                mutex m_mutex;
                SourceId m_sourceId;
            };
        }
    }
}
#endif