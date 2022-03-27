#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_ASRPROFILE_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_ASRPROFILE_H_

#include <ostream>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace aip {
            enum class ASRProfile {
                CLOSE_TALK,
                NEAR_FIELD,
                FAR_FIELD
            };
            inline std::ostream& operator<<(std::ostream& stream, ASRProfile profile) {
                switch (profile) {
                    case ASRProfile::CLOSE_TALK: stream << "CLOSE_TALK"; break;
                    case ASRProfile::NEAR_FIELD: stream << "NEAR_FIELD"; break;
                    case ASRProfile::FAR_FIELD: stream << "FAR_FIELD"; break;
                }
                return stream;
            }
            inline std::string asrProfileToString(ASRProfile profile) {
                switch (profile) {
                    case ASRProfile::CLOSE_TALK: return "CLOSE_TALK";
                    case ASRProfile::NEAR_FIELD: return "NEAR_FIELD";
                    case ASRProfile::FAR_FIELD: return "FAR_FIELD";
                }
                return "";
            }
        }
    }
}
#endif