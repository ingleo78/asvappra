#include <metrics/DataPointCounterBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include "DialogUXStateAggregator.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace chrono;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace audioAnalyzer;
            using namespace mediaPlayer;
            using namespace metrics;
            static const string TAG("DialogUXStateAggregator");
            #define LX(event) LogEntry(TAG, event)
            static const milliseconds SHORT_TIMEOUT{200};
            static const string CUSTOM_METRIC_PREFIX = "CUSTOM-";
            static const string LISTENING_TIMEOUT_EXPIRES = "LISTENING_TIMEOUT_EXPIRES";
            static const string THINKING_TIMEOUT_EXPIRES = "THINKING_TIMEOUT_EXPIRES";
            static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const string& eventName) {
                if (!metricRecorder) return;
                auto metricEvent = MetricEventBuilder{}.setActivityName(CUSTOM_METRIC_PREFIX + eventName).addDataPoint(DataPointCounterBuilder{}
                                       .setName(eventName).increment(1).build()).build();
                if (metricEvent == nullptr) {
                    ACSDK_ERROR(LX("Error creating metric."));
                    return;
                }
                recordMetric(metricRecorder, metricEvent);
            }
            DialogUXStateAggregator::DialogUXStateAggregator(shared_ptr<MetricRecorderInterface> metricRecorder, milliseconds timeoutForThinkingToIdle,
                                                             milliseconds timeoutForListeningToIdle) : m_metricRecorder{metricRecorder},
                                                             m_currentState{DialogUXStateObserverInterface::DialogUXState::IDLE},
                                                             m_timeoutForThinkingToIdle{timeoutForThinkingToIdle}, m_timeoutForListeningToIdle{timeoutForListeningToIdle},
                                                             m_speechSynthesizerState{SpeechSynthesizerObserverInterface::SpeechSynthesizerState::FINISHED},
                                                             m_audioInputProcessorState{AudioInputProcessorObserverInterface::State::IDLE} {}
            void DialogUXStateAggregator::addObserver(shared_ptr<DialogUXStateObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                m_executor.submit([this, observer]() {
                    m_observers.insert(observer);
                    observer->onDialogUXStateChanged(m_currentState);
                });
            }
            void DialogUXStateAggregator::removeObserver(shared_ptr<DialogUXStateObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                m_executor.submit([this, observer]() { m_observers.erase(observer); }).wait();
            }
            void DialogUXStateAggregator::onStateChanged(AudioInputProcessorObserverInterface::State state) {
                m_audioInputProcessorState = state;
                m_executor.submit([this, state]() {
                    switch (state) {
                        case AudioInputProcessorObserverInterface::State::IDLE:
                            tryEnterIdleState();
                            return;
                        case AudioInputProcessorObserverInterface::State::RECOGNIZING:
                            onActivityStarted();
                            setState(DialogUXStateObserverInterface::DialogUXState::LISTENING);
                            return;
                        case AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH:
                            onActivityStarted();
                            setState(DialogUXStateObserverInterface::DialogUXState::EXPECTING);
                            return;
                        case AudioInputProcessorObserverInterface::State::BUSY:
                            setState(DialogUXStateObserverInterface::DialogUXState::LISTENING);
                            if (!m_listeningTimeoutTimer.start(m_timeoutForListeningToIdle,bind(&DialogUXStateAggregator::transitionFromListeningTimedOut,
                             this)).valid()) {
                                ACSDK_ERROR(LX("failedToStartTimerFromListeningToIdle"));
                            }
                            return;
                    }
                    ACSDK_ERROR(LX("unknownAudioInputProcessorState"));
                });
            }
            void DialogUXStateAggregator::onStateChanged(SpeechSynthesizerObserverInterface::SpeechSynthesizerState state, const MediaPlayerInterface::SourceId mediaSourceId,
                                                         const Optional<MediaPlayerState>& mediaPlayerState, const vector<AudioAnalyzerState>& audioAnalyzerState) {
                m_speechSynthesizerState = state;
                m_executor.submit([this, state]() {
                    switch(state) {
                        case SpeechSynthesizerObserverInterface::SpeechSynthesizerState::PLAYING:
                            onActivityStarted();
                            setState(DialogUXStateObserverInterface::DialogUXState::SPEAKING);
                            return;
                        case SpeechSynthesizerObserverInterface::SpeechSynthesizerState::FINISHED: case SpeechSynthesizerObserverInterface::SpeechSynthesizerState::INTERRUPTED:
                            tryEnterIdleState();
                            return;
                        case SpeechSynthesizerObserverInterface::SpeechSynthesizerState::LOSING_FOCUS: return;
                        case SpeechSynthesizerObserverInterface::SpeechSynthesizerState::GAINING_FOCUS:
                            onActivityStarted();
                            return;
                    }
                    ACSDK_ERROR(LX("unknownSpeechSynthesizerState"));
                });
            }
            void DialogUXStateAggregator::receive(const string& contextId, const string& message) {
                m_executor.submit([this]() {
                    if (DialogUXStateObserverInterface::DialogUXState::THINKING == m_currentState &&
                        SpeechSynthesizerObserverInterface::SpeechSynthesizerState::GAINING_FOCUS != m_speechSynthesizerState) {
                        m_thinkingTimeoutTimer.stop();
                        m_thinkingTimeoutTimer.start(SHORT_TIMEOUT, bind(&DialogUXStateAggregator::transitionFromThinkingTimedOut, this));
                    }
                });
            }
            void DialogUXStateAggregator::onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status,
                                                                    const ConnectionStatusObserverInterface::ChangedReason reason) {
                m_executor.submit([this, status]() {
                    if (status != ConnectionStatusObserverInterface::Status::CONNECTED) setState(DialogUXStateObserverInterface::DialogUXState::IDLE);
                });
            }
            void DialogUXStateAggregator::onRequestProcessingStarted() {
                ACSDK_DEBUG(LX("onRequestProcessingStarted"));
                m_executor.submit([this]() {
                    m_listeningTimeoutTimer.stop();
                    ACSDK_DEBUG0(LX("onRequestProcessingStartedLambda").d("currentState", m_currentState));
                    switch(m_currentState) {
                        case DialogUXStateObserverInterface::DialogUXState::IDLE:
                            ACSDK_WARN(LX("onRequestProcessingStartedLambda").d("reason", "transitioningFromIdle"));
                        case DialogUXStateObserverInterface::DialogUXState::LISTENING:
                            setState(DialogUXStateObserverInterface::DialogUXState::THINKING);
                            if (!m_thinkingTimeoutTimer.start(m_timeoutForThinkingToIdle, std::bind(&DialogUXStateAggregator::transitionFromThinkingTimedOut, this)).valid()) {
                                ACSDK_ERROR(LX("failedToStartTimerFromThinkingToIdle"));
                            }
                            break;
                        default: ACSDK_ERROR(LX("onRequestProcessingStartedLambda").d("reason", "invalidState").d("currentState", m_currentState));
                    }
                });
            }
            void DialogUXStateAggregator::onRequestProcessingCompleted() {}
            void DialogUXStateAggregator::notifyObserversOfState() {
                for (auto observer : m_observers) {
                    if (observer) observer->onDialogUXStateChanged(m_currentState);
                }
            }
            void DialogUXStateAggregator::transitionFromThinkingTimedOut() {
                m_executor.submit([this]() {
                    if (DialogUXStateObserverInterface::DialogUXState::THINKING == m_currentState) {
                        ACSDK_DEBUG(LX("transitionFromThinkingTimedOut"));
                        setState(DialogUXStateObserverInterface::DialogUXState::IDLE);
                        submitMetric(m_metricRecorder, THINKING_TIMEOUT_EXPIRES);
                    }
                });
            }
            void DialogUXStateAggregator::transitionFromListeningTimedOut() {
                m_executor.submit([this]() {
                    if (DialogUXStateObserverInterface::DialogUXState::LISTENING == m_currentState) {
                        ACSDK_DEBUG(LX("transitionFromListeningTimedOut"));
                        setState(DialogUXStateObserverInterface::DialogUXState::IDLE);
                        submitMetric(m_metricRecorder, LISTENING_TIMEOUT_EXPIRES);
                    }
                });
            }
            void DialogUXStateAggregator::tryEnterIdleStateOnTimer() {
                m_executor.submit([this]() {
                    if (m_currentState != sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE &&
                        m_audioInputProcessorState == AudioInputProcessorObserverInterface::State::IDLE &&
                        (m_speechSynthesizerState == SpeechSynthesizerObserverInterface::SpeechSynthesizerState::FINISHED ||
                         m_speechSynthesizerState == SpeechSynthesizerObserverInterface::SpeechSynthesizerState::INTERRUPTED)) {
                        setState(sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE);
                    }
                });
            }
            void DialogUXStateAggregator::setState(sdkInterfaces::DialogUXStateObserverInterface::DialogUXState newState) {
                if (newState == m_currentState) return;
                m_listeningTimeoutTimer.stop();
                m_thinkingTimeoutTimer.stop();
                m_multiturnSpeakingToListeningTimer.stop();
                ACSDK_DEBUG(LX("setState").d("from", m_currentState).d("to", newState));
                m_currentState = newState;
                notifyObserversOfState();
            }
            void DialogUXStateAggregator::tryEnterIdleState() {
                ACSDK_DEBUG5(LX(__func__));
                m_thinkingTimeoutTimer.stop();
                m_multiturnSpeakingToListeningTimer.stop();
                if (!m_multiturnSpeakingToListeningTimer.start(SHORT_TIMEOUT, std::bind(&DialogUXStateAggregator::tryEnterIdleStateOnTimer, this)).valid()) {
                    ACSDK_ERROR(LX("failedToStartTryEnterIdleStateTimer"));
                }
            }
            void DialogUXStateAggregator::onActivityStarted() {
                m_listeningTimeoutTimer.stop();
                m_thinkingTimeoutTimer.stop();
            }
        }
    }
}