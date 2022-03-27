#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_MEDIASTREAMINGSTATE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_MEDIASTREAMINGSTATE_H_

#include <string>
#include <iostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                enum class MediaStreamingState {
                    IDLE,
                    PENDING,
                    ACTIVE
                };
                inline std::string_view mediaStreamingStateToString(MediaStreamingState value) {
                    switch (value) {
                        case MediaStreamingState::IDLE: return "IDLE";
                        case MediaStreamingState::PENDING: return "PENDING";
                        case MediaStreamingState::ACTIVE: return "ACTIVE";
                        default: return "UNKNOWN";
                    }
                }
                inline std::ostream& operator<<(std::ostream& stream, const MediaStreamingState state) {
                    return stream << mediaStreamingStateToString(state);
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_MEDIASTREAMINGSTATE_H_
