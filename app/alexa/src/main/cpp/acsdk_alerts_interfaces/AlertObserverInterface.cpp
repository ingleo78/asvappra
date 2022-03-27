#include "AlertObserverInterface.h"
#include "AudioPlayerInterface.h"
#include "AudioPlayerObserverInterface.h"
#include "NotificationRendererInterface.h"
#include "NotificationsObserverInterface.h"
#include "NotificationsStorageInterface.h"

namespace alexaClientSDK {
    namespace acsdkAlertsInterfaces {
        inline string_view AlertObserverInterface::stateToString(State state) {
            switch(state) {
                case State::READY: return "READY";
                case State::STARTED: return "STARTED";
                case State::STOPPED: return "STOPPED";
                case State::SNOOZED: return "SNOOZED";
                case State::COMPLETED: return "COMPLETED";
                case State::PAST_DUE: return "PAST_DUE";
                case State::FOCUS_ENTERED_FOREGROUND: return "FOCUS_ENTERED_FOREGROUND";
                case State::FOCUS_ENTERED_BACKGROUND: return "FOCUS_ENTERED_BACKGROUND";
                case State::ERROR: return "ERROR";
                case State::DELETED: return "DELETED";
                case State::SCHEDULED_FOR_LATER: return "SCHEDULED_FOR_LATER";
            }
            return "unknown State";
        }
        inline ostream& operator<<(ostream& stream, const AlertObserverInterface::State& state) {
            return stream << AlertObserverInterface::stateToString(state);
        }
    }
}