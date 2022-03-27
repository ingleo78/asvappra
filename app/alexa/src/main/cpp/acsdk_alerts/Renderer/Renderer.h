#ifndef ACSDKALERTS_RENDERER_RENDERER_H_
#define ACSDKALERTS_RENDERER_RENDERER_H_

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>
#include <acsdk_alerts/Renderer/RendererInterface.h>
#include <acsdk_alerts/Renderer/RendererObserverInterface.h>
#include <sdkinterfaces/SpeakerManagerInterface.h>
#include <threading/Executor.h>
#include <media_player/MediaPlayerInterface.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <media_player/SourceConfig.h>
#include <util/MediaType.h>
#include <metrics/MetricRecorderInterface.h>

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace renderer {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace utils;
            using namespace mediaPlayer;
            using namespace metrics;
            using namespace threading;
            class Renderer : public RendererInterface, public MediaPlayerObserverInterface {
            public:
                static shared_ptr<Renderer> create(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
                void start(shared_ptr<RendererObserverInterface> observer, function<std::pair<unique_ptr<std::istream>, const MediaType>()> audioFactory,
                           bool volumeRampEnabled, const vector<string>& urls = vector<string>(), int loopCount = 0, milliseconds loopPause = milliseconds{0},
                           bool startWithPause = false);
                void stop() override;
                void onFirstByteRead(SourceId sourceId, const MediaPlayerState& state) override;
                void onPlaybackStarted(SourceId sourceId, const MediaPlayerState& state) override;
                void onPlaybackStopped(SourceId sourceId, const MediaPlayerState& state) override;
                void onPlaybackFinished(SourceId sourceId, const MediaPlayerState& state) override;
                void onPlaybackError(SourceId sourceId, const ErrorType& type, string error, const MediaPlayerState& state) override;
            private:
                using SourceId = MediaPlayerInterface::SourceId;
                Renderer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<MetricRecorderInterface> metricRecorder);
                void executeStart(shared_ptr<RendererObserverInterface> observer, function<pair<std::unique_ptr<istream>, const MediaType>()> audioFactory,
                                  bool volumeRampEnabled, const vector<string>& urls, int loopCount, milliseconds loopPause, bool startWithPause);
                void executeStop();
                void executeOnPlaybackStarted(SourceId sourceId);
                void executeOnPlaybackStopped(SourceId sourceId);
                void executeOnPlaybackFinished(SourceId sourceId);
                void executeOnPlaybackError(SourceId sourceId, const ErrorType& type, const string& error);
                void notifyObserver(RendererObserverInterface::State state, const string& message = "");
                void resetSourceId();
                bool shouldPlayDefault();
                bool shouldMediaPlayerRepeat();
                bool shouldRenderNext();
                bool shouldPause();
                bool isLastSourceInLoop();
                bool isLastSource();
                bool pause(milliseconds duration);
                void play();
                avsCommon::utils::mediaPlayer::SourceConfig generateMediaConfiguration();
                bool renderNextAudioAsset(bool* pauseInterruptedOut = nullptr);
                void handlePlaybackError(const std::string& error);
                shared_ptr<MediaPlayerInterface> m_mediaPlayer;
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                shared_ptr<RendererObserverInterface> m_observer;
                vector<string> m_urls;
                int m_numberOfStreamsRenderedThisLoop;
                int m_remainingLoopCount;
                int m_directiveLoopCount;
                milliseconds m_loopPause;
                bool m_shouldPauseBeforeRender;
                time_point<steady_clock> m_loopStartTime;
                function<pair<unique_ptr<istream>, const MediaType>()> m_defaultAudioFactory;
                bool m_isStopping;
                bool m_isStartPending;
                condition_variable m_waitCondition;
                mutex m_waitMutex;
                SourceId m_currentSourceId;
                bool m_volumeRampEnabled;
                Executor m_executor;
                steady_clock::time_point m_renderStartTime;
            };
        }
    }
}
#endif