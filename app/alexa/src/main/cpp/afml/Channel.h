#ifndef ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_CHANNEL_H_
#define ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_CHANNEL_H_

#include <algorithm>
#include <chrono>
#include <list>
#include <memory>
#include <string>
#include <avs/ContentType.h>
#include <avs/FocusState.h>
#include <sdkinterfaces/ChannelObserverInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace afml {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace timing;
        class Channel {
        public:
            struct State {
                State(const std::string& name);
                State();
                std::string name;
                FocusState focusState;
                std::string interfaceName;
                steady_clock::time_point timeAtIdle;
            };
            Channel(const std::string& name, const unsigned int priority);
            Channel(const std::string& name, const unsigned int priority, bool isVirtual);
            const std::string& getName() const;
            unsigned int getPriority() const;
            bool setFocus(FocusState focus, MixingBehavior behavior, bool forceUpdate = false);
            void setPrimaryActivity(shared_ptr<FocusManagerInterface::Activity> activity);
            bool releaseActivity(shared_ptr<ChannelObserverInterface> observer);
            bool releaseActivity(const string& interfaceName);
            bool isActive();
            bool operator>(const Channel& rhs) const;
            string getInterface() const;
            Channel::State getState() const;
            vector<Channel::State> getActivityUpdates();
            shared_ptr<FocusManagerInterface::Activity> getPrimaryActivity();
            shared_ptr<FocusManagerInterface::Activity> getActivity(const std::string& interfaceName);
            vector<string> getInterfaceList() const;
        private:
            void notifyActivities(MixingBehavior behavior, FocusState focusState);
            bool releaseActivityLocked(shared_ptr<FocusManagerInterface::Activity> activityToRelease);
            bool removeActivityHelperLocked(list<shared_ptr<FocusManagerInterface::Activity>>::iterator activityToRemoveIt);
            void addToChannelUpdatesLocked(const std::string& interfaceName, FocusState focusState);
            void processPolicyLocked(shared_ptr<FocusManagerInterface::Activity> incomingActivity, shared_ptr<FocusManagerInterface::Activity> currentActivity);
            void patienceTimerCallback(shared_ptr<FocusManagerInterface::Activity> activity);
            void updateChannelInterfaceLocked();
            shared_ptr<FocusManagerInterface::Activity> getPrimaryActivityLocked() const;
        private:
            const unsigned int m_priority;
            bool m_isVirtual;
            State m_state;
            list<shared_ptr<FocusManagerInterface::Activity>> m_activities;
            mutable mutex m_mutex;
            vector<Channel::State> m_activityUpdates;
            Timer m_patienceTimer;
            shared_ptr<FocusManagerInterface::Activity> m_patienceInitiator;
            shared_ptr<FocusManagerInterface::Activity> m_patienceReceiver;
        };
    }
}
#endif