#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_INITIATOR_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_INITIATOR_H_

#include <string>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace aip {
            enum class Initiator {
                PRESS_AND_HOLD,
                TAP,
                WAKEWORD
            };
            inline std::string initiatorToString(Initiator initiator) {
                switch(initiator) {
                    case Initiator::PRESS_AND_HOLD: return "PRESS_AND_HOLD";
                    case Initiator::TAP: return "TAP";
                    case Initiator::WAKEWORD: return "WAKEWORD";
                }
                return "Unknown Inititator";
            }
        }
    }
}
#endif