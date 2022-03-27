#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTALERTOBSERVER_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTALERTOBSERVER_H_

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <deque>
#include <acsdk_alerts_interfaces/AlertObserverInterface.h>

namespace alexaClientSDK {
    namespace integration {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace acsdkAlertsInterfaces;
            class TestAlertObserver : public AlertObserverInterface {
            public:
                void onAlertStateChange(const string& alertToken, const string& alertType, State state, const string& reason);
                class changedAlert {
                public:
                    State state;
                };
                changedAlert waitForNext(const seconds duration);
            private:
                mutex m_mutex;
                condition_variable m_wakeTrigger;
                deque<changedAlert> m_queue;
                State currentState;
            };
        }
    }
}
#endif