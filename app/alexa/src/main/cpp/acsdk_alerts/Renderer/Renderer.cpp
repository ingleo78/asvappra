#include <algorithm>
#include <chrono>
#include <fstream>
#include <sdkinterfaces/SpeakerInterface.h>
#include <logger/Logger.h>
#include <metrics/MetricEventBuilder.h>
#include <metrics/DataPointCounterBuilder.h>
#include "Renderer.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace renderer {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace mediaPlayer;
            using namespace metrics;
            static const string TAG("Renderer");
            static const string ALERT_RENDERER_METRIC_SOURCE_PREFIX = "ALERT_RENDERER-";
            #define LX(event) LogEntry(TAG, event)
            static const auto ALARM_VOLUME_RAMP_TIME = minutes(1);
            static bool isSourceIdOk(MediaPlayerInterface::SourceId sourceId) {
                return sourceId != MediaPlayerInterface::ERROR;
            }
            static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const string& eventName, int count) {
                if (!metricRecorder) return;
                auto metricEvent = MetricEventBuilder{}.setActivityName(ALERT_RENDERER_METRIC_SOURCE_PREFIX + eventName)
                                   .addDataPoint(DataPointCounterBuilder{}.setName(eventName).increment(count).build()).build();
                if (!metricEvent) {
                    ACSDK_ERROR(LX("Error creating metric."));
                    return;
                }
                recordMetric(metricRecorder, metricEvent);
            }
            shared_ptr<Renderer> Renderer::create(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<MetricRecorderInterface> metricRecorder) {
                if (!mediaPlayer) {
                    ACSDK_ERROR(LX("createFailed").m("mediaPlayer parameter was nullptr."));
                    return nullptr;
                }
                auto renderer = shared_ptr<Renderer>(new Renderer{mediaPlayer, metricRecorder});
                mediaPlayer->addObserver(renderer);
                return renderer;
            }
            void Renderer::start(shared_ptr<RendererObserverInterface> observer, function<pair<unique_ptr<istream>, const MediaType>()> audioFactory,
                                 bool volumeRampEnabled, const vector<string>& urls, int loopCount, milliseconds loopPause, bool startWithPause) {
                ACSDK_DEBUG5(LX(__func__));
                pair<unique_ptr<istream>, const MediaType> defaultAudio = audioFactory();
                if (!defaultAudio.first) {
                    ACSDK_ERROR(LX("startFailed").m("default audio is nullptr"));
                    return;
                }
                if (loopCount < 0) {
                    ACSDK_ERROR(LX("startInvalidParam").m("loopCount less than zero - adjusting to acceptable minimum."));
                    loopCount = 0;
                }
                if (loopPause.count() < 0) {
                    ACSDK_ERROR(LX("startInvalidParam").m("loopPause less than zero - adjusting to acceptable minimum."));
                    loopPause = milliseconds{0};
                }
                m_executor.submit([this, observer, audioFactory, volumeRampEnabled, urls, loopCount, loopPause, startWithPause]() {
                    executeStart(observer, audioFactory, volumeRampEnabled, urls, loopCount, loopPause, startWithPause);
                });
            }
            void Renderer::stop() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_waitMutex);
                m_isStopping = true;
                m_waitCondition.notify_all();
                m_executor.submit([this]() { executeStop(); });
            }
            void Renderer::onFirstByteRead(SourceId id, const MediaPlayerState&) {
                ACSDK_DEBUG(LX(__func__).d("id", id));
            }
            void Renderer::onPlaybackStarted(SourceId sourceId, const MediaPlayerState&) {
                m_executor.submit([this, sourceId]() { executeOnPlaybackStarted(sourceId); });
            }
            void Renderer::onPlaybackStopped(SourceId sourceId, const MediaPlayerState&) {
                m_executor.submit([this, sourceId]() { executeOnPlaybackStopped(sourceId); });
            }
            void Renderer::onPlaybackFinished(SourceId sourceId, const MediaPlayerState&) {
                m_executor.submit([this, sourceId]() { executeOnPlaybackFinished(sourceId); });
            }
            void Renderer::onPlaybackError(SourceId sourceId, const ErrorType& type, string error, const MediaPlayerState&) {
                m_executor.submit([this, sourceId, type, error]() { executeOnPlaybackError(sourceId, type, error); });
            }
            Renderer::Renderer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<MetricRecorderInterface> metricRecorder) :
                    m_mediaPlayer{mediaPlayer}, m_metricRecorder{metricRecorder}, m_observer{nullptr}, m_numberOfStreamsRenderedThisLoop{0},
                    m_remainingLoopCount{0}, m_directiveLoopCount{0}, m_loopPause{milliseconds{0}}, m_shouldPauseBeforeRender{false},
                    m_isStopping{false}, m_isStartPending{false}, m_volumeRampEnabled{false} { resetSourceId(); }
            bool Renderer::shouldPlayDefault() {
                ACSDK_DEBUG9(LX("shouldPlayDefault"));
                return m_urls.empty();
            }
            bool Renderer::shouldMediaPlayerRepeat() {
                ACSDK_DEBUG9(LX("shouldMediaPlayerRepeat"));
                return m_urls.empty() && (0 == m_directiveLoopCount) && (0 == m_loopPause.count());
            }
            bool Renderer::isLastSource() {
                ACSDK_DEBUG9(LX("isLastSource"));
                return isLastSourceInLoop() && m_remainingLoopCount <= 0;
            }
            bool Renderer::isLastSourceInLoop() {
                ACSDK_DEBUG9(LX("isLastSourceInLoop"));
                return m_numberOfStreamsRenderedThisLoop >= static_cast<int>(m_urls.size());
            }
            bool Renderer::shouldRenderNext() {
                ACSDK_DEBUG9(LX("shouldRenderNext"));
                if (shouldMediaPlayerRepeat()) return false;
                if (m_directiveLoopCount > 0) return (m_remainingLoopCount > 0);
                if (shouldPlayDefault() && (0 != m_loopPause.count())) return true;
                return false;
            }
            bool Renderer::shouldPause() {
                ACSDK_DEBUG9(LX("shouldPause"));
                if (0 == m_loopPause.count()) return false;
                if (m_directiveLoopCount == 0) return true;
                if (m_remainingLoopCount > 0) return true;
                return false;
            }
            bool Renderer::pause(milliseconds duration) {
                ACSDK_DEBUG9(LX(__func__).d("duration", duration.count()));
                if (duration.count() <= 0) {
                    ACSDK_WARN(LX(__func__).m("duration is a non-positive value.  Returning."));
                    return false;
                }
                unique_lock<mutex> lock(m_waitMutex);
                return !m_waitCondition.wait_for(lock, duration, [this]() { return m_isStopping; });
            }
            SourceConfig Renderer::generateMediaConfiguration() {
                if (!m_volumeRampEnabled) return emptySourceConfig();
                auto timeDifference = steady_clock::now().time_since_epoch() - m_renderStartTime.time_since_epoch();
                auto timePlayedDuration = duration_cast<milliseconds>(timeDifference);
                auto startVolumeGain = (timePlayedDuration.count() * MAX_GAIN) / duration_cast<milliseconds>(ALARM_VOLUME_RAMP_TIME).count();
                if (timePlayedDuration.count() < 0) {
                    ACSDK_ERROR(LX("generateMediaConfigurationFailed").d("reason", "invalidDuration"));
                    return emptySourceConfig();
                }
                return SourceConfig::createWithFadeIn(startVolumeGain, MAX_GAIN, ALARM_VOLUME_RAMP_TIME);
            }
            void Renderer::play() {
                auto mediaConfig = generateMediaConfiguration();
                ACSDK_DEBUG9(LX(__func__).d("fadeEnabled", m_volumeRampEnabled).d("startGain", mediaConfig.fadeInConfig.startGain));
                m_isStartPending = false;
                if (shouldPlayDefault()) {
                    shared_ptr<std::istream> stream;
                    MediaType streamFormat = avsCommon::utils::MediaType::UNKNOWN;
                    tie(stream, streamFormat) = m_defaultAudioFactory();
                    m_currentSourceId = m_mediaPlayer->setSource(stream, shouldMediaPlayerRepeat(), mediaConfig, streamFormat);
                } else {
                    m_currentSourceId = m_mediaPlayer->setSource(m_urls[m_numberOfStreamsRenderedThisLoop], milliseconds::zero(), mediaConfig, false);
                }
                if (!isSourceIdOk(m_currentSourceId)) {
                    ACSDK_ERROR(LX("executeStartFailed").d("m_currentSourceId", m_currentSourceId).m("SourceId response was invalid."));
                    return;
                }
                if (m_shouldPauseBeforeRender) {
                    ACSDK_DEBUG5(LX(__func__).m("Performing initial pause before beginning loop rendering."));
                    m_shouldPauseBeforeRender = false;
                    pause(m_loopPause);
                }
                if (0 == m_numberOfStreamsRenderedThisLoop) m_loopStartTime = steady_clock::now();
                if (!m_mediaPlayer->play(m_currentSourceId)) {
                    const string errorMessage{"MediaPlayer play request failed."};
                    ACSDK_ERROR(LX("executeStartFailed").d("m_currentSourceId", m_currentSourceId).m(errorMessage));
                    handlePlaybackError(errorMessage);
                }
            }
            void Renderer::executeStart(shared_ptr<RendererObserverInterface> observer, function<pair<std::unique_ptr<istream>, const MediaType>()> audioFactory,
                                        bool volumeRampEnabled, const vector<string>& urls, int loopCount, milliseconds loopPause, bool startWithPause) {
                ACSDK_DEBUG1(LX(__func__).d("rampEnabled", volumeRampEnabled).d("urls.size", urls.size()).d("loopCount", loopCount)
                                 .d("loopPause (ms)", duration_cast<std::chrono::milliseconds>(loopPause).count()).d("startWithPause", startWithPause));
                m_observer = observer;
                m_urls = urls;
                m_remainingLoopCount = loopCount;
                m_directiveLoopCount = loopCount;
                m_loopPause = loopPause;
                m_shouldPauseBeforeRender = startWithPause;
                m_defaultAudioFactory = audioFactory;
                m_volumeRampEnabled = volumeRampEnabled;
                m_numberOfStreamsRenderedThisLoop = 0;
                ACSDK_DEBUG9(LX("executeStart").d("m_urls.size", m_urls.size()).d("m_remainingLoopCount", m_remainingLoopCount)
                        .d("m_loopPause (ms)", duration_cast<milliseconds>(m_loopPause).count()));
                unique_lock<mutex> lock(m_waitMutex);
                if (m_isStopping) {
                    lock.unlock();
                    ACSDK_DEBUG5(LX(__func__).m("Being stopped. Will start playing once fully stopped."));
                    m_isStartPending = true;
                    return;
                }
                lock.unlock();
                m_renderStartTime = steady_clock::now();
                play();
            }
            void Renderer::executeStop() {
                ACSDK_DEBUG1(LX("executeStop"));
                m_isStartPending = false;
                if (MediaPlayerInterface::ERROR == m_currentSourceId) {
                    ACSDK_DEBUG5(LX(__func__).m("Nothing to stop, no media playing."));
                    {
                        lock_guard<mutex> lock(m_waitMutex);
                        m_isStopping = false;
                        m_observer = nullptr;
                    }
                    return;
                }
                if (!m_mediaPlayer->stop(m_currentSourceId)) {
                    string errorMessage = "mediaPlayer stop request failed.";
                    ACSDK_ERROR(LX("executeStopFailed").d("SourceId", m_currentSourceId).m(errorMessage));
                }
            }
            void Renderer::executeOnPlaybackStarted(SourceId sourceId) {
                ACSDK_DEBUG1(LX("executeOnPlaybackStarted").d("sourceId", sourceId));
                if (m_currentSourceId != sourceId) {
                    ACSDK_DEBUG9(LX("executeOnPlaybackStarted").d("m_currentSourceId", m_currentSourceId).m("Ignoring - different from expected source id."));
                    return;
                }
                notifyObserver(RendererObserverInterface::State::STARTED);
            }
            void Renderer::executeOnPlaybackStopped(SourceId sourceId) {
                ACSDK_DEBUG1(LX("executeOnPlaybackStopped").d("sourceId", sourceId));
                if (m_currentSourceId != sourceId) {
                    ACSDK_DEBUG9(LX("executeOnPlaybackStopped").d("m_currentSourceId", m_currentSourceId).m("Ignoring - different from expected source id."));
                    return;
                }
                {
                    lock_guard<mutex> lock(m_waitMutex);
                    m_isStopping = false;
                }
                notifyObserver(RendererObserverInterface::State::STOPPED);
                if (m_isStartPending) {
                    ACSDK_DEBUG5(LX(__func__).m("Resuming pending play."));
                    play();
                } else {
                    m_observer = nullptr;
                    resetSourceId();
                }
            }
            void Renderer::executeOnPlaybackFinished(SourceId sourceId) {
                ACSDK_DEBUG1(LX("executeOnPlaybackFinished").d("sourceId", sourceId));
                if (m_currentSourceId != sourceId) {
                    ACSDK_DEBUG9(LX("executeOnPlaybackFinished").d("m_currentSourceId", m_currentSourceId).m("Ignoring - different from expected source id."));
                    return;
                }
                RendererObserverInterface::State finalState = RendererObserverInterface::State::STOPPED;
                ++m_numberOfStreamsRenderedThisLoop;
                auto localIsStopping = false;
                {
                    lock_guard<std::mutex> lock(m_waitMutex);
                    localIsStopping = m_isStopping;
                    m_isStopping = false;
                }
                if (!localIsStopping && shouldRenderNext()) {
                    bool pauseWasInterrupted = false;
                    if (renderNextAudioAsset(&pauseWasInterrupted)) return;
                    else {
                        if (!pauseWasInterrupted) finalState = RendererObserverInterface::State::COMPLETED;
                    }
                }
                resetSourceId();
                notifyObserver(finalState);
                m_observer = nullptr;
            }
            bool Renderer::renderNextAudioAsset(bool* pauseInterruptedOut) {
                if (pauseInterruptedOut) *pauseInterruptedOut = false;
                if (isLastSourceInLoop()) {
                    m_remainingLoopCount--;
                    m_numberOfStreamsRenderedThisLoop = 0;
                    ACSDK_DEBUG5(LX("renderNextAudioAsset").d("remainingLoopCount", m_remainingLoopCount).d("nextAudioIndex", m_numberOfStreamsRenderedThisLoop)
                        .m("Preparing the audio loop counters."));
                    if (shouldPause()) {
                        auto loopRenderDuration = duration_cast<milliseconds>(steady_clock::now() - m_loopStartTime);
                        auto pauseDuration = m_loopPause - loopRenderDuration;
                        if (pauseDuration.count() > 0) {
                            bool pauseWasInterrupted = !pause(pauseDuration);
                            if (pauseInterruptedOut) *pauseInterruptedOut = pauseWasInterrupted;
                            if (pauseWasInterrupted) {
                                ACSDK_DEBUG5(LX(__func__).m("Pause has been interrupted, not proceeding with the loop."));
                                return false;
                            }
                        }
                    }
                }
                if (!shouldRenderNext()) return false;
                play();
                return true;
            }
            void Renderer::executeOnPlaybackError(SourceId sourceId, const ErrorType& type, const string& error) {
                ACSDK_DEBUG1(LX("executeOnPlaybackError").d("sourceId", sourceId).d("type", type).d("error", error));
                submitMetric(m_metricRecorder, errorTypeToString(type).data(), 1);
                if (m_currentSourceId != sourceId) {
                    ACSDK_DEBUG9(LX("executeOnPlaybackError").d("m_currentSourceId", m_currentSourceId).m("Ignoring - different from expected source id."));
                    return;
                }
                handlePlaybackError(error);
            }
            void Renderer::notifyObserver(RendererObserverInterface::State state, const string& message) {
                if (m_observer) m_observer->onRendererStateChange(state, message);
            }
            void Renderer::resetSourceId() {
                ACSDK_DEBUG5(LX(__func__));
                m_currentSourceId = MediaPlayerInterface::ERROR;
            }
            void Renderer::handlePlaybackError(const string& error) {
                unique_lock<mutex> lock(m_waitMutex);
                m_isStopping = false;
                lock.unlock();
                m_isStartPending = false;
                resetSourceId();
                notifyObserver(RendererObserverInterface::State::ERROR, error);
                m_observer = nullptr;
            }
        }
    }
}