#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEECHINTERACTIONHANDLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEECHINTERACTIONHANDLERINTERFACE_H_

#include <stdlib.h>
#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <stdint.h>
#include <capability_agents/AIP/AudioProvider.h>
#include <avs/AudioInputStream.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace capabilityAgents::aip;
            using namespace avs;
            using namespace std;
            using namespace chrono;
            class SpeechInteractionHandlerInterface {
            public:
                virtual ~SpeechInteractionHandlerInterface() = default;
                virtual future<bool> notifyOfWakeWord(AudioProvider wakeWordAudioProvider, uint64_t beginIndex, uint64_t endIndex, string keyword,
                                                      steady_clock::time_point startOfSpeechTimestamp, shared_ptr<const vector<char>> KWDMetadata = nullptr) = 0;
                virtual future<bool> notifyOfTapToTalk(AudioProvider tapToTalkAudioProvider, AudioInputStream::Index beginIndex,
                                                       steady_clock::time_point startOfSpeechTimestamp = steady_clock::now()) = 0;
                virtual future<bool> notifyOfHoldToTalkStart(AudioProvider holdToTalkAudioProvider,
                                                             steady_clock::time_point startOfSpeechTimestamp = steady_clock::now()) = 0;
                virtual future<bool> notifyOfHoldToTalkEnd() = 0;
                virtual future<bool> notifyOfTapToTalkEnd() = 0;
            };
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEECHINTERACTIONHANDLERINTERFACE_H_
