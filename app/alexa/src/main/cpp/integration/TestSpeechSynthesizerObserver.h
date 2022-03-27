#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTSPEECHSYNTHESIZEROBSERVER_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTSPEECHSYNTHESIZEROBSERVER_H_

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <sdkinterfaces/SpeechSynthesizerObserverInterface.h>

namespace alexaClientSDK {
    namespace integration {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace audioAnalyzer;
            using namespace mediaPlayer;
            using namespace sdkInterfaces::SpeechSynthesizerObserverInterface;
            using namespace mediaPlayer::MediaPlayerInterface;
            class TestSpeechSynthesizerObserver : public avsCommon::sdkInterfaces::SpeechSynthesizerObserverInterface {
            public:
                TestSpeechSynthesizerObserver();
                ~TestSpeechSynthesizerObserver() = default;
                void onStateChanged(SpeechSynthesizerState state, const SourceId mediaSourceId, const Optional<MediaPlayerState>& mediaPlayerState,
                                    const vector<AudioAnalyzerState>& audioAnalyzerState) override;
                bool checkState(const SpeechSynthesizerState expectedState, const seconds duration);
                SpeechSynthesizerState waitForNext(const seconds duration);
                SpeechSynthesizerState getCurrentState();
            private:
                SpeechSynthesizerState m_state;
                mutex m_mutex;
                condition_variable m_wakeTrigger;
                deque<SpeechSynthesizerState> m_queue;
            };
        }
    }
}
#endif