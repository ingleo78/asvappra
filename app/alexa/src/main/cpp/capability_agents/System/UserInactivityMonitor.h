#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_USERINACTIVITYMONITOR_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_USERINACTIVITYMONITOR_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <avs/CapabilityAgent.h>
#include <timing/Timer.h>
#include <util/RequiresShutdown.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/UserInactivityMonitorInterface.h>
#include <sdkinterfaces/UserInactivityMonitorObserverInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace timing;
            class UserInactivityMonitor : public UserInactivityMonitorInterface, public CapabilityAgent, public RequiresShutdown {
            public:
                using DirectiveInfo = CapabilityAgent::DirectiveInfo;
                static shared_ptr<UserInactivityMonitor> create(shared_ptr<MessageSenderInterface> messageSender,
                                                                shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                const milliseconds& sendPeriod = hours(1));
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void onUserActive() override;
                seconds timeSinceUserActivity() override;
                void addObserver(shared_ptr<UserInactivityMonitorObserverInterface> observer) override;
                void removeObserver(shared_ptr<UserInactivityMonitorObserverInterface> observer) override;
            private:
                UserInactivityMonitor(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                      const milliseconds& sendPeriod);
                void doShutdown() override;
                void removeDirectiveGracefully(shared_ptr<DirectiveInfo> info, bool isFailure = false, const string& report = "");
                void sendInactivityReport();
                void startTimer();
                void notifyObservers();
                shared_ptr<MessageSenderInterface> m_messageSender;
                mutex m_mutex;
                Timer m_eventTimer;
                atomic_bool m_recentUpdateBlocked;
                time_point<steady_clock> m_lastTimeActive;
                unordered_set<shared_ptr<UserInactivityMonitorObserverInterface>> m_inactivityObservers;
                const milliseconds m_sendPeriod;
            };
        }
    }
}
#endif