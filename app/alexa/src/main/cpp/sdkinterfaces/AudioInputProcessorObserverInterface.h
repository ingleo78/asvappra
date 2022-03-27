#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIOINPUTPROCESSOROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIOINPUTPROCESSOROBSERVERINTERFACE_H_

#include <sstream>
#include <iostream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AudioInputProcessorObserverInterface {
            public:
                enum class State {
                    IDLE,
                    EXPECTING_SPEECH,
                    RECOGNIZING,
                    BUSY
                };
                virtual ~AudioInputProcessorObserverInterface() = default;
                virtual void onStateChanged(State state);
                static std::string_view stateToString(State state) {
                    switch(state) {
                        case State::IDLE: return "IDLE";
                        case State::EXPECTING_SPEECH: return "EXPECTING_SPEECH";
                        case State::RECOGNIZING: return "RECOGNIZING";
                        case State::BUSY: return "BUSY";
                    }
                    return "Unknown State";
                }
            };
            inline std::ostream& operator<<(std::ostream& stream, const AudioInputProcessorObserverInterface::State& state) {
                return stream << AudioInputProcessorObserverInterface::stateToString(state);
            }
        }
    }
}
#endif