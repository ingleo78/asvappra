#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIALOGUXSTATEOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIALOGUXSTATEOBSERVERINTERFACE_H_

#include <ostream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class DialogUXStateObserverInterface {
            public:
                DialogUXStateObserverInterface(){}
                enum class DialogUXState {
                    IDLE,
                    LISTENING,
                    EXPECTING,
                    THINKING,
                    SPEAKING,
                    FINISHED
                };
                virtual ~DialogUXStateObserverInterface() = default;
                virtual void onDialogUXStateChanged(DialogUXState newState);
                static std::string stateToString(DialogUXState state);
            };
            inline void onDialogUXStateChanged(DialogUXStateObserverInterface::DialogUXState newState) {}
            inline std::string DialogUXStateObserverInterface::stateToString(DialogUXState state) {
                switch(state) {
                    case DialogUXState::IDLE: return "IDLE";
                    case DialogUXState::LISTENING: return "LISTENING";
                    case DialogUXState::EXPECTING: return "EXPECTING";
                    case DialogUXState::THINKING: return "THINKING";
                    case DialogUXState::SPEAKING: return "SPEAKING";
                    case DialogUXState::FINISHED: return "FINISHED";
                }
                return "Unknown State";
            }
            inline std::ostream& operator<<(std::ostream& stream, const DialogUXStateObserverInterface::DialogUXState& state) {
                return stream << DialogUXStateObserverInterface::stateToString(state);
            }
        }
    }
}
#endif