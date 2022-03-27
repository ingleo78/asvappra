#ifndef ACSDKALERTSINTERFACES_ALERTOBSERVERINTERFACE_H_
#define ACSDKALERTSINTERFACES_ALERTOBSERVERINTERFACE_H_

#include <string>
#include <iostream>

namespace alexaClientSDK {
    namespace acsdkAlertsInterfaces {
        using namespace std;
        class AlertObserverInterface {
        public:
            enum class State {
                READY,
                STARTED,
                STOPPED,
                SNOOZED,
                COMPLETED,
                PAST_DUE,
                FOCUS_ENTERED_FOREGROUND,
                FOCUS_ENTERED_BACKGROUND,
                ERROR,
                DELETED,
                SCHEDULED_FOR_LATER
            };
            virtual ~AlertObserverInterface() = default;
            virtual void onAlertStateChange(const string& alertToken, const string& alertType, State state, const string_view& reason = "");
            static string_view stateToString(State state);
        };
    }
}
#endif