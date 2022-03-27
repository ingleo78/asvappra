#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_DIALOGUXSTATEAGGREGATOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_DIALOGUXSTATEAGGREGATOR_H_

#include <atomic>
#include <chrono>
#include <unordered_set>
#include <vector>
#include <sdkinterfaces/AudioInputProcessorObserverInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <sdkinterfaces/InteractionModelRequestProcessingObserverInterface.h>
#include <sdkinterfaces/MessageObserverInterface.h>
#include <sdkinterfaces/SpeechSynthesizerObserverInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include <threading/Executor.h>
#include <timing/Timer.h>
#include <media_player/MediaPlayerState.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace chrono;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace audioAnalyzer;
            using namespace metrics;
            using namespace mediaPlayer;
            using namespace threading;
            using namespace timing;
            class DialogUXStateAggregator : public AudioInputProcessorObserverInterface, public SpeechSynthesizerObserverInterface, public MessageObserverInterface,
                                            public ConnectionStatusObserverInterface, public InteractionModelRequestProcessingObserverInterface {
            public:
                DialogUXStateAggregator(shared_ptr<MetricRecorderInterface> metricRecorder = nullptr, milliseconds timeoutForThinkingToIdle = (milliseconds)seconds(8).count(),
                                        milliseconds timeoutForListeningToIdle = (milliseconds)seconds{8}.count());
                void addObserver(shared_ptr<DialogUXStateObserverInterface> observer);
                void removeObserver(shared_ptr<DialogUXStateObserverInterface> observer);
                void onStateChanged(AudioInputProcessorObserverInterface::State state) override;
                void onStateChanged(SpeechSynthesizerObserverInterface::SpeechSynthesizerState state, const MediaPlayerInterface::SourceId mediaSourceId,
                                    const Optional<MediaPlayerState>& mediaPlayerState, const vector<AudioAnalyzerState>& audioAnalyzerState) override;
                void receive(const string& contextId, const string& message) override;
                void onRequestProcessingStarted() override;
                void onRequestProcessingCompleted() override;
            private:
                void notifyObserversOfState();
                void setState(sdkInterfaces::DialogUXStateObserverInterface::DialogUXState newState);
                void tryEnterIdleState();
                void transitionFromThinkingTimedOut();
                void transitionFromListeningTimedOut();
                void tryEnterIdleStateOnTimer();
                void onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status, const ConnectionStatusObserverInterface::ChangedReason reason) override;
                void onActivityStarted();
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                unordered_set<shared_ptr<DialogUXStateObserverInterface>> m_observers;
                DialogUXStateObserverInterface::DialogUXState m_currentState;
                const milliseconds m_timeoutForThinkingToIdle;
                Timer m_thinkingTimeoutTimer;
                Timer m_multiturnSpeakingToListeningTimer;
                const microseconds m_timeoutForListeningToIdle;
                Timer m_listeningTimeoutTimer;
                Executor m_executor;
                atomic<SpeechSynthesizerObserverInterface::SpeechSynthesizerState> m_speechSynthesizerState;
                atomic<AudioInputProcessorObserverInterface::State> m_audioInputProcessorState;
            };
        }
    }
}
#endif