#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYREQUESTOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYREQUESTOR_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            struct PlayRequestor {
                std::string type;
                std::string id;
            };
            inline bool operator==(const PlayRequestor& playRequestorA, const PlayRequestor& playRequestorB) {
                bool state1 = playRequestorA.type == playRequestorB.type;
                bool state2 = playRequestorA.id == playRequestorB.id;
                return state1 && state2;
            }
        }
    }
}
#endif