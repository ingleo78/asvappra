#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTMEDIAPLAYER_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTMEDIAPLAYER_H_

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <iostream>
#include <string>
#include <future>
#include <unordered_set>
#include <media_player/MediaPlayerInterface.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <util/MediaType.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace integration {
        namespace test {

            class TestMediaPlayer : public avsCommon::utils::mediaPlayer::MediaPlayerInterface {
            public:
                ~TestMediaPlayer();
                avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
                    std::shared_ptr<avsCommon::avs::attachment::AttachmentReader> attachmentReader,
                    const avsCommon::utils::AudioFormat* audioFormat = nullptr,
                    const avsCommon::utils::mediaPlayer::SourceConfig& config =
                        avsCommon::utils::mediaPlayer::emptySourceConfig()) override;
                avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
                    std::shared_ptr<std::istream> stream,
                    bool repeat,
                    const avsCommon::utils::mediaPlayer::SourceConfig& config = avsCommon::utils::mediaPlayer::emptySourceConfig(),
                    avsCommon::utils::MediaType format = avsCommon::utils::MediaType::UNKNOWN) override;
                avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
                    const std::string& url,
                    std::chrono::milliseconds offset = std::chrono::milliseconds::zero(),
                    const avsCommon::utils::mediaPlayer::SourceConfig& config = avsCommon::utils::mediaPlayer::emptySourceConfig(),
                    bool repeat = false,
                    const avsCommon::utils::mediaPlayer::PlaybackContext& playbackContext =
                        avsCommon::utils::mediaPlayer::PlaybackContext()) override;
                bool play(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
                bool stop(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
                bool pause(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
                bool resume(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
                std::chrono::milliseconds getOffset(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
                void addObserver(
                    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> playerObserver) override;
                void removeObserver(
                    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> playerObserver) override;
                avsCommon::utils::Optional<avsCommon::utils::mediaPlayer::MediaPlayerState> getMediaPlayerState(
                    avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
                uint64_t getNumBytesBuffered() override;
            private:
                std::unordered_set<std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface>> m_observers;
                bool m_playbackFinished = false;
                std::shared_ptr<avsCommon::avs::attachment::AttachmentReader> m_attachmentReader;
                std::shared_ptr<avsCommon::utils::timing::Timer> m_timer;
                std::shared_ptr<std::istream> m_istream;
            };
        }
    }
}
#endif