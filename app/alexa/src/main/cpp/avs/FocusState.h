#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_FOCUSSTATE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_FOCUSSTATE_H_

#include <sstream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class FocusState {
                FOREGROUND,
                BACKGROUND,
                NONE
            };
            inline std::string focusStateToString(FocusState state) {
                switch(state) {
                    case FocusState::FOREGROUND: return "FOREGROUND";
                    case FocusState::BACKGROUND: return "BACKGROUND";
                    case FocusState::NONE: return "NONE";
                }
                return "Unknown State";
            }
            inline std::ostream& operator<<(std::ostream& stream, const FocusState& state) {
                return stream << focusStateToString(state);
            }
        }
    }
}
#endif