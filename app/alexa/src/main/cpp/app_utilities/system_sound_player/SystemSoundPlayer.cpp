#include <logger/Logger.h>
#include "SystemSoundPlayer.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace systemSoundPlayer {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace audio;
            using namespace utils;
            using namespace logger;
            using namespace mediaPlayer;
            static const string TAG("SystemSoundPlayer");
            #define LX(event) LogEntry(TAG, event)
            static shared_future<bool> getFalseFuture() {
                auto errPromise = promise<bool>();
                errPromise.set_value(false);
                return errPromise.get_future();
            }
            shared_ptr<SystemSoundPlayer> SystemSoundPlayer::create(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<SystemSoundAudioFactoryInterface> soundPlayerAudioFactory) {
                if (!mediaPlayer) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMediaPlayer"));
                    return nullptr;
                }
                if (!soundPlayerAudioFactory) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullSoundPlayerAudioFactory"));
                    return nullptr;
                }
                auto systemSoundPlayer = shared_ptr<SystemSoundPlayer>(new SystemSoundPlayer(mediaPlayer, soundPlayerAudioFactory));
                mediaPlayer->addObserver(systemSoundPlayer);
                return systemSoundPlayer;
            }
            shared_future<bool> SystemSoundPlayer::playTone(Tone tone) {
                lock_guard<mutex> lock(m_mutex);
                if (m_sharedFuture.valid()) {
                    ACSDK_ERROR(LX("playToneFailed").d("reason", "Already Playing a Tone"));
                    return getFalseFuture();
                }
                m_sourceId = MediaPlayerInterface::ERROR;
                shared_ptr<istream> stream;
                MediaType streamFormat = MediaType::UNKNOWN;
                switch(tone) {
                    case Tone::WAKEWORD_NOTIFICATION:
                        tie(stream, streamFormat) = m_soundPlayerAudioFactory->wakeWordNotificationTone()();
                        m_sourceId = m_mediaPlayer->setSource(stream, false, emptySourceConfig(), streamFormat);
                        break;
                    case Tone::END_SPEECH:
                        tie(stream, streamFormat) = m_soundPlayerAudioFactory->endSpeechTone()();
                        m_sourceId = m_mediaPlayer->setSource(stream, false, mediaPlayer::emptySourceConfig(), streamFormat);
                        break;
                }
                if (MediaPlayerInterface::ERROR == m_sourceId) {
                    ACSDK_ERROR(LX("playToneFailed").d("reason", "setSourceFailed").d("type", "attachment"));
                    return getFalseFuture();
                }
                if (!m_mediaPlayer->play(m_sourceId)) {
                    ACSDK_ERROR(LX("playToneFailed").d("reason", "playSourceFailed"));
                    return getFalseFuture();
                }
                m_sharedFuture = m_playTonePromise.get_future();
                return m_sharedFuture;
            }
            void SystemSoundPlayer::onPlaybackStarted(SourceId id, const MediaPlayerState&) {
                ACSDK_DEBUG5(LX(__func__).d("SourceId", id));
            }
            void SystemSoundPlayer::onPlaybackFinished(SourceId id, const MediaPlayerState&) {
                lock_guard<mutex> lock(m_mutex);
                ACSDK_DEBUG5(LX(__func__).d("SourceId", id));
                if (m_sourceId != id) { ACSDK_ERROR(LX(__func__).d("SourceId", id).d("reason", "sourceId doesn't match played file")); }
                finishPlayTone(true);
            }
            void SystemSoundPlayer::onPlaybackError(
                SourceId id,
                const ErrorType& type,
                string error,
                const MediaPlayerState&) {
                lock_guard<mutex> lock(m_mutex);
                ACSDK_ERROR(LX(__func__).d("SourceId", id).d("error", error));
                if (m_sourceId != id) {
                    ACSDK_ERROR(LX("UnexpectedSourceId").d("expectedSourceId", m_sourceId).d("reason", "sourceId doesn't match played file"));
                }
                finishPlayTone(false);
            }
            void SystemSoundPlayer::onFirstByteRead(SourceId id, const MediaPlayerState&) {}
            void SystemSoundPlayer::finishPlayTone(bool result) {
                m_playTonePromise.set_value(result);
                m_playTonePromise = promise<bool>();
                m_sharedFuture = shared_future<bool>();
            }
            SystemSoundPlayer::SystemSoundPlayer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<SystemSoundAudioFactoryInterface> soundPlayerAudioFactory) :
                                                 m_mediaPlayer{mediaPlayer}, m_soundPlayerAudioFactory{soundPlayerAudioFactory} {}
        }
    }
}