#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEECHSYNTHESIZEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEECHSYNTHESIZEROBSERVERINTERFACE_H_

#include <iostream>
#include <vector>
#include <media_player/MediaPlayerInterface.h>
#include <audio_analyzer/AudioAnalyzerState.h>
#include <media_player/MediaPlayerState.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace utils;
            using namespace audioAnalyzer;
            using namespace mediaPlayer;
            class SpeechSynthesizerObserverInterface {
            public:
                using SourceId = MediaPlayerInterface::SourceId ;
                enum class SpeechSynthesizerState {
                    PLAYING,
                    FINISHED,
                    INTERRUPTED,
                    GAINING_FOCUS,
                    LOSING_FOCUS
                };
                virtual ~SpeechSynthesizerObserverInterface() = default;
                virtual void onStateChanged(SpeechSynthesizerState state, const SourceId mediaSourceId, const Optional<MediaPlayerState>& mediaPlayerState,
                                            const vector<AudioAnalyzerState>& audioAnalyzerState);
            };
            using SpeechSynthesizerState = SpeechSynthesizerObserverInterface::SpeechSynthesizerState;
            inline ostream& operator<<(ostream& stream, const SpeechSynthesizerState state) {
                switch(state) {
                    case SpeechSynthesizerState::PLAYING: stream << "PLAYING"; break;
                    case SpeechSynthesizerState::FINISHED: stream << "FINISHED"; break;
                    case SpeechSynthesizerState::INTERRUPTED: stream << "INTERRUPTED"; break;
                    case SpeechSynthesizerState::GAINING_FOCUS: stream << "GAINING_FOCUS"; break;
                    case SpeechSynthesizerState::LOSING_FOCUS: stream << "LOSING_FOCUS"; break;
                }
                return stream;
            }
        }
    }
}
#endif