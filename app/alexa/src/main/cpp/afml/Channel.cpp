#include <logger/Logger.h>
#include "Channel.h"

namespace alexaClientSDK {
    namespace afml {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        static const string TAG("Channel");
        #define LX(event) LogEntry(TAG, event)
        Channel::State::State(const std::string& name) : name{name}, focusState{FocusState::NONE}, timeAtIdle{steady_clock::now()} {}
        Channel::State::State() : focusState{FocusState::NONE}, timeAtIdle{steady_clock::now()} {}
        Channel::Channel(const string& name, const unsigned int priority) : m_priority{priority}, m_isVirtual{false}, m_state{name}{}
        Channel::Channel(const string& name, const unsigned int priority, bool isVirtual) : m_priority{priority}, m_isVirtual{isVirtual}, m_state{name} {}
        const std::string& Channel::getName() const {
            return m_state.name;
        }
        unsigned int Channel::getPriority() const {
            return m_priority;
        }
        bool Channel::setFocus(FocusState focus, MixingBehavior behavior, bool forceUpdate) {
            unique_lock<mutex> lock(m_mutex);
            bool focusChanged = (m_state.focusState != focus);
            auto primaryActivity = getPrimaryActivityLocked();
            bool mixingBehaviorChanged = primaryActivity && (primaryActivity->getMixingBehavior() != behavior);
            if (!forceUpdate && !focusChanged && !mixingBehaviorChanged) return false;
            ACSDK_DEBUG5(LX(__func__).d("name", m_state.name).d("newfocusState", focus).d("prevfocusState", m_state.focusState).d("newMixingBehavior", behavior)
                .d("forceUpdate", forceUpdate));
            m_state.focusState = focus;
            if (m_state.focusState == FocusState::NONE) m_state.timeAtIdle = steady_clock::now();
            addToChannelUpdatesLocked(m_state.interfaceName, m_state.focusState);
            lock.unlock();
            notifyActivities(behavior, focus);
            return true;
        }
        void Channel::setPrimaryActivity(shared_ptr<FocusManagerInterface::Activity> activity) {
            unique_lock<mutex> lock(m_mutex);
            if (!activity) {
                ACSDK_ERROR(LX("setPrimaryActivityFailed").m("Null Activity"));
                return;
            }
            ACSDK_DEBUG5(LX(__func__).d("Interface", activity->getInterface()));
            if (!m_activities.empty()) processPolicyLocked(activity, m_activities.front());
            m_activities.push_front(activity);
            updateChannelInterfaceLocked();
        }
        bool Channel::releaseActivity(shared_ptr<ChannelObserverInterface> observer) {
            if (observer == nullptr) {
                ACSDK_ERROR(LX("releaseActivityFailed").d("reason", "observer is null."));
                return false;
            }
            unique_lock<mutex> lock(m_mutex);
            for (auto it = m_activities.begin(); it != m_activities.end(); ++it) {
                if ((*it)->getChannelObserver() == observer) {
                    bool success = removeActivityHelperLocked(it);
                    if (success) addToChannelUpdatesLocked(m_state.interfaceName, m_state.focusState);
                    return success;
                }
            }
            ACSDK_DEBUG0(LX("releaseActivityFailed").m("Observer not found"));
            return false;
        }
        bool Channel::releaseActivity(const string& interfaceName) {
            unique_lock<mutex> lock(m_mutex);
            for (auto it = m_activities.begin(); it != m_activities.end(); ++it) {
                if ((*it)->getInterface() == interfaceName) {
                    bool success = removeActivityHelperLocked(it);
                    if (success) addToChannelUpdatesLocked(m_state.interfaceName, m_state.focusState);
                    return success;
                }
            }
            return false;
        }
        bool Channel::isActive() {
            unique_lock<mutex> lock(m_mutex);
            return !m_activities.empty();
        }
        bool Channel::operator>(const Channel& rhs) const {
            return m_priority < rhs.getPriority();
        }
        string Channel::getInterface() const {
            lock_guard<mutex> lock(m_mutex);
            return m_state.interfaceName;
        }
        Channel::State Channel::getState() const {
            lock_guard<mutex> lock(m_mutex);
            return m_state;
        }
        vector<Channel::State> Channel::getActivityUpdates() {
            unique_lock<mutex> lock(m_mutex);
            auto activityUpdatesRet = m_activityUpdates;
            m_activityUpdates.clear();
            return activityUpdatesRet;
        }
        shared_ptr<FocusManagerInterface::Activity> Channel::getPrimaryActivity() {
            unique_lock<mutex> lock(m_mutex);
            return getPrimaryActivityLocked();
        }
        shared_ptr<FocusManagerInterface::Activity> Channel::getPrimaryActivityLocked() const {
            if (m_activities.empty()) return nullptr;
            return *(m_activities.begin());
        }
        shared_ptr<FocusManagerInterface::Activity> Channel::getActivity(const string& interfaceName) {
            unique_lock<mutex> lock(m_mutex);
            for (const auto& it : m_activities) {
                if (it->getInterface() == interfaceName) return it;
            }
            return nullptr;
        }
        vector<string> Channel::getInterfaceList() const {
            vector<string> listOfInterface = {};
            for (const auto& activity : m_activities) {
                listOfInterface.push_back(activity->getInterface());
            }
            return listOfInterface;
        }
        void Channel::notifyActivities(MixingBehavior behavior, FocusState focusState) {
            unique_lock<mutex> lock(m_mutex);
            if (m_activities.empty()) {
                ACSDK_WARN(LX("notifyActivitiesFailed").m("No Associated Activities Found"));
                return;
            }
            auto activitiesCopy = m_activities;
            lock.unlock();
            auto activityIt = activitiesCopy.begin();
            (*activityIt)->notifyObserver(focusState, behavior);
            activityIt++;
            for (; activityIt != activitiesCopy.end(); activityIt++) {
                (*activityIt)->notifyObserver(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
            }
        }
        bool Channel::releaseActivityLocked(shared_ptr<FocusManagerInterface::Activity> activityToRelease) {
            if (!activityToRelease) {
                ACSDK_ERROR(LX("releaseActivityLockedFailed").m("Null activityToRelease"));
                return false;
            }
            auto priorActivityIt = find(m_activities.begin(), m_activities.end(), activityToRelease);
            if (priorActivityIt != m_activities.end()) return removeActivityHelperLocked(priorActivityIt);
            return false;
        }
        bool Channel::removeActivityHelperLocked(list<shared_ptr<FocusManagerInterface::Activity>>::iterator activityToRemoveIt) {
            ACSDK_DEBUG5(LX(__func__).d("interface", (*activityToRemoveIt)->getInterface()));
            auto isRemovingPatienceReceiver = false;
            if (m_patienceReceiver == *activityToRemoveIt) isRemovingPatienceReceiver = true;
            if (m_patienceTimer.isActive() &&
                (m_patienceInitiator == *activityToRemoveIt || m_patienceReceiver == *activityToRemoveIt)) {
                m_patienceTimer.stop();
                ACSDK_DEBUG9(LX(__func__).d("status", "Patience Timer Stopped"));
                m_patienceInitiator = nullptr;
                m_patienceReceiver = nullptr;
            }
            if (m_activities.size() == 1) m_state.timeAtIdle = std::chrono::steady_clock::now();
            if (!isRemovingPatienceReceiver) addToChannelUpdatesLocked((*activityToRemoveIt)->getInterface(), FocusState::NONE);
            (*activityToRemoveIt)->notifyObserver(FocusState::NONE, MixingBehavior::MUST_STOP);
            m_activities.erase(activityToRemoveIt);
            updateChannelInterfaceLocked();
            return true;
        }
        void Channel::addToChannelUpdatesLocked(const string& interfaceName, FocusState focusState) {
            if (m_state.interfaceName.empty()) return;
            if (!m_isVirtual) {
                auto state = m_state;
                state.focusState = focusState;
                state.interfaceName = interfaceName;
                m_activityUpdates.push_back(state);
                ACSDK_DEBUG0(LX(__func__).d("interface", state.interfaceName).d("focusState", state.focusState));
            }
        }
        void Channel::processPolicyLocked(shared_ptr<FocusManagerInterface::Activity> incomingActivity, shared_ptr<FocusManagerInterface::Activity> currentActivity) {
            if (!incomingActivity || !currentActivity) {
                ACSDK_ERROR(LX("processPolicyLockedFailed").m("Null Activities"));
                return;
            }
            if (incomingActivity->getInterface() == currentActivity->getInterface()) {
                releaseActivityLocked(currentActivity);
                return;
            }
            if (m_patienceTimer.isActive()) {
                m_patienceTimer.stop();
                ACSDK_DEBUG9(LX(__func__).d("status", "Patience Release Timer Stopped"));
                releaseActivityLocked(m_patienceReceiver);
                m_patienceReceiver = nullptr;
            }
            if (incomingActivity->getPatienceDuration().count() > 0) {
                ACSDK_DEBUG9(LX(__func__).d("status", "Patience Timer Started"));
                addToChannelUpdatesLocked(currentActivity->getInterface(), FocusState::NONE);
                auto patienceDuration = incomingActivity->getPatienceDuration();
                m_patienceTimer.start(patienceDuration, std::bind(&Channel::patienceTimerCallback, this, currentActivity));
                m_patienceInitiator = incomingActivity;
                m_patienceReceiver = currentActivity;
            } else {
                if (m_patienceInitiator != nullptr) {
                    releaseActivityLocked(m_patienceInitiator);
                    m_patienceInitiator = nullptr;
                }
                releaseActivityLocked(currentActivity);
            }
        }
        void Channel::patienceTimerCallback(shared_ptr<FocusManagerInterface::Activity> activity) {
            unique_lock<mutex> lock(m_mutex);
            ACSDK_DEBUG9(LX(__func__).d("status", "Patience Release Timer Triggered"));
            releaseActivityLocked(std::move(activity));
        }
        void Channel::updateChannelInterfaceLocked() {
            if (!m_activities.empty()) m_state.interfaceName = m_activities.front()->getInterface();
            else m_state.interfaceName = "";
        }
    }
}