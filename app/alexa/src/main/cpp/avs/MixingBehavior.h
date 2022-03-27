#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_MIXINGBEHAVIOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_MIXINGBEHAVIOR_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class MixingBehavior {
                PRIMARY,
                MAY_DUCK,
                MUST_PAUSE,
                MUST_STOP,
                UNDEFINED
            };
            inline std::string mixingBehaviorToString(MixingBehavior behavior) {
                switch(behavior) {
                    case MixingBehavior::PRIMARY: return "PRIMARY";
                    case MixingBehavior::MAY_DUCK: return "MAY_DUCK";
                    case MixingBehavior::MUST_PAUSE: return "MUST_PAUSE";
                    case MixingBehavior::MUST_STOP: return "MUST_STOP";
                    case MixingBehavior::UNDEFINED: return "UNDEFINED";
                }
                return "UNDEFINED";
            }
            inline MixingBehavior getMixingBehavior(const std::string& input) {
                MixingBehavior behavior = MixingBehavior::UNDEFINED;
                if (mixingBehaviorToString(MixingBehavior::PRIMARY) == input) behavior = MixingBehavior::PRIMARY;
                else if (mixingBehaviorToString(MixingBehavior::MAY_DUCK) == input) behavior = MixingBehavior::MAY_DUCK;
                else if (mixingBehaviorToString(MixingBehavior::MUST_PAUSE) == input) behavior = MixingBehavior::MUST_PAUSE;
                else if (mixingBehaviorToString(MixingBehavior::MUST_STOP) == input) behavior = MixingBehavior::MUST_STOP;
                return behavior;
            }
            inline std::ostream& operator<<(std::ostream& stream, const MixingBehavior& behavior) {
                return stream << mixingBehaviorToString(behavior);
            }
        }
    }
}
#endif