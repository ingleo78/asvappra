#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ALEXASTATECHANGECAUSETYPE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ALEXASTATECHANGECAUSETYPE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            enum class AlexaStateChangeCauseType {
                ALEXA_INTERACTION,
                APP_INTERACTION,
                PHYSICAL_INTERACTION,
                PERIODIC_POLL,
                RULE_TRIGGER,
                VOICE_INTERACTION
            };
            inline std::string alexaStateChangeCauseTypeToString(AlexaStateChangeCauseType cause) {
                switch(cause) {
                    case AlexaStateChangeCauseType::ALEXA_INTERACTION: return "ALEXA_INTERACTION";
                    case AlexaStateChangeCauseType::APP_INTERACTION: return "APP_INTERACTION";
                    case AlexaStateChangeCauseType::PHYSICAL_INTERACTION: return "PHYSICAL_INTERACTION";
                    case AlexaStateChangeCauseType::PERIODIC_POLL: return "PERIODIC_POLL";
                    case AlexaStateChangeCauseType::RULE_TRIGGER: return "RULE_TRIGGER";
                    case AlexaStateChangeCauseType::VOICE_INTERACTION: return "VOICE_INTERACTION";
                }
                return "UNKNOWN";
            }
        }
    }
}
#endif