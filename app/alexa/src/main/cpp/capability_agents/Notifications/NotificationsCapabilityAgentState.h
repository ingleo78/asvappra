#ifndef ACSDKNOTIFICATIONS_NOTIFICATIONSCAPABILITYAGENTSTATE_H_
#define ACSDKNOTIFICATIONS_NOTIFICATIONSCAPABILITYAGENTSTATE_H_

#include <ostream>
#include <string>

namespace alexaClientSDK {
    namespace acsdkNotifications {
        enum class NotificationsCapabilityAgentState {
            IDLE,
            PLAYING,
            CANCELING_PLAY,
            SHUTTING_DOWN,
            SHUTDOWN
        };
        inline std::string stateToString(NotificationsCapabilityAgentState state) {
            switch (state) {
                case NotificationsCapabilityAgentState::IDLE: return "IDLE";
                case NotificationsCapabilityAgentState::PLAYING: return "PLAYING";
                case NotificationsCapabilityAgentState::CANCELING_PLAY: return "CANCELING_PLAY";
                case NotificationsCapabilityAgentState::SHUTTING_DOWN: return "SHUTTING_DOWN";
                case NotificationsCapabilityAgentState::SHUTDOWN: return "SHUTDOWN";
            }
            return "Unknown NotificationsCapabilityAgent State";
        }
        inline std::ostream& operator<<(std::ostream& stream, const NotificationsCapabilityAgentState& state) {
            return stream << stateToString(state);
        }
    }
}
#endif