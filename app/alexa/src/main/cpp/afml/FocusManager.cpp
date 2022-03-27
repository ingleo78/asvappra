#include <algorithm>
#include <logger/Logger.h>
#include "FocusManager.h"

namespace alexaClientSDK {
    namespace afml {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace avs;
        using namespace configuration;
        using namespace logger;
        using namespace interruptModel;
        static const string TAG("FocusManager");
        static const string VIRTUAL_CHANNELS_CONFIG_KEY = "virtualChannels";
        static const string CHANNEL_NAME_KEY = "name";
        static const string CHANNEL_PRIORITY_KEY = "priority";
        #define LX(event) LogEntry(TAG, event)
        FocusManager::FocusManager(const vector<ChannelConfiguration>& channelConfigurations, shared_ptr<ActivityTrackerInterface> activityTrackerInterface,
                                   const vector<ChannelConfiguration>& virtualChannelConfigurations, shared_ptr<InterruptModel> interruptModel) :
                                   m_activityTracker{activityTrackerInterface}, m_interruptModel{interruptModel} {
            readChannelConfiguration(channelConfigurations, false);
            readChannelConfiguration(virtualChannelConfigurations, true);
        }
        bool FocusManager::acquireChannel(const string& channelName, shared_ptr<ChannelObserverInterface> channelObserver, const string& interfaceName) {
            ACSDK_DEBUG1(LX("acquireChannel").d("channelName", channelName).d("interface", interfaceName));
            shared_ptr<Channel> channelToAcquire = getChannel(channelName);
            if (!channelToAcquire) {
                ACSDK_ERROR(LX("acquireChannelFailed").d("reason", "channelNotFound").d("channelName", channelName));
                return false;
            }
            auto channelActivity = FocusManagerInterface::Activity::create(interfaceName, channelObserver);
            if (!channelActivity) {
                ACSDK_ERROR(LX("acquireChannelFailed").d("reason", "failedToCreateActivity").d("interface", interfaceName));
                return false;
            }
            m_executor.submit([this, channelToAcquire, channelActivity]() { acquireChannelHelper(channelToAcquire, channelActivity); });
            return true;
        }
        bool FocusManager::acquireChannel(const string& channelName, shared_ptr<FocusManagerInterface::Activity> channelActivity) {
            ACSDK_DEBUG1(LX("acquireChannel").d("channelName", channelName).d("interface", channelActivity->getInterface()));
            shared_ptr<Channel> channelToAcquire = getChannel(channelName);
            if (!channelToAcquire) {
                ACSDK_ERROR(LX("acquireChannelFailed").d("reason", "channelNotFound").d("channelName", channelName));
                return false;
            }
            if (!channelActivity) {
                ACSDK_ERROR(LX("acquireChannelFailed").d("reason", "channelActivityIsNull"));
                return false;
            }
            m_executor.submit([this, channelToAcquire, channelActivity]() { acquireChannelHelper(channelToAcquire, channelActivity); });
            return true;
        }
        future<bool> FocusManager::releaseChannel(const string& channelName, shared_ptr<ChannelObserverInterface> channelObserver) {
            ACSDK_DEBUG1(LX("releaseChannel").d("channelName", channelName));
            auto releaseChannelSuccess = make_shared<std::promise<bool>>();
            future<bool> returnValue = releaseChannelSuccess->get_future();
            shared_ptr<Channel> channelToRelease = getChannel(channelName);
            if (!channelToRelease) {
                ACSDK_ERROR(LX("releaseChannelFailed").d("reason", "channelNotFound").d("channelName", channelName));
                releaseChannelSuccess->set_value(false);
                return returnValue;
            }
            m_executor.submit([this, channelToRelease, channelObserver, releaseChannelSuccess, channelName]() {
                releaseChannelHelper(channelToRelease, channelObserver, releaseChannelSuccess, channelName);
            });
            return returnValue;
        }
        void FocusManager::stopForegroundActivity() {
            unique_lock<mutex> lock(m_mutex);
            shared_ptr<Channel> foregroundChannel = getHighestPriorityActiveChannelLocked();
            if (!foregroundChannel) {
                ACSDK_DEBUG(LX("stopForegroundActivityFailed").d("reason", "noForegroundActivity"));
                return;
            }
            string foregroundChannelInterface = foregroundChannel->getInterface();
            lock.unlock();
            m_executor.submitToFront([this, foregroundChannel, foregroundChannelInterface]() {
                stopForegroundActivityHelper(foregroundChannel, foregroundChannelInterface);
            });
        }
        void FocusManager::stopAllActivities() {
            ACSDK_DEBUG5(LX(__func__));
            if (m_activeChannels.empty()) {
                ACSDK_DEBUG5(LX(__func__).m("no active channels"));
                return;
            }
            ChannelsToInterfaceNamesMap channelOwnersCapture;
            unique_lock<mutex> lock(m_mutex);
            for (const auto& channel : m_activeChannels) {
                for (const auto& interfaceName : channel->getInterfaceList()) {
                    channelOwnersCapture.insert(pair<shared_ptr<Channel>, string>(channel, interfaceName));
                }
            }
            lock.unlock();
            m_executor.submitToFront([this, channelOwnersCapture]() { stopAllActivitiesHelper(channelOwnersCapture); });
        }
        void FocusManager::addObserver(const shared_ptr<FocusManagerObserverInterface>& observer) {
            lock_guard<mutex> lock(m_mutex);
            m_observers.insert(observer);
        }
        void FocusManager::removeObserver(const shared_ptr<FocusManagerObserverInterface>& observer) {
            lock_guard<std::mutex> lock(m_mutex);
            m_observers.erase(observer);
        }
        void FocusManager::readChannelConfiguration(const vector<ChannelConfiguration>& channelConfigurations, bool isVirtual) {
            for (const auto& config : channelConfigurations) {
                if (doesChannelNameExist(config.name)) {
                    ACSDK_ERROR(LX("readChannelConfigurationFailed").d("reason", "channelNameExists").d("config", config.toString()));
                    continue;
                }
                if (doesChannelPriorityExist(config.priority)) {
                    ACSDK_ERROR(LX("readChannelConfigurationFailed").d("reason", "channelPriorityExists").d("config", config.toString()));
                    continue;
                }
                auto channel = make_shared<Channel>(config.name, config.priority, isVirtual);
                m_allChannels.insert({config.name, channel});
            }
        }
        void FocusManager::setChannelFocus(const shared_ptr<Channel>& channel, FocusState focus, MixingBehavior behavior, bool forceUpdate) {
            if (!channel->setFocus(focus, behavior, forceUpdate)) return;
            unique_lock<mutex> lock(m_mutex);
            auto observers = m_observers;
            lock.unlock();
            for (auto& observer : observers) {
                observer->onFocusChanged(channel->getName(), focus);
            }
        }
        MixingBehavior FocusManager::getMixingBehavior(shared_ptr<Channel> lowPrioChannel, shared_ptr<Channel> highPrioChannel) {
            if (!m_interruptModel) {
                ACSDK_ERROR(LX(__func__).m("Null InterruptModel"));
                return MixingBehavior::UNDEFINED;
            }
            if (!lowPrioChannel || !highPrioChannel) {
                ACSDK_ERROR(LX("getMixingBehaviorFailed").d("reason", "nullInputChannels"));
                return MixingBehavior::UNDEFINED;
            }
            if (*lowPrioChannel > *highPrioChannel) {
                ACSDK_ERROR(LX("getMixingBehaviorFailed").d("reason", "Priorities of input channels violate API contract")
                                .d("lowPrioChannel priority", lowPrioChannel->getPriority()).d("highPrioChannel priority", highPrioChannel->getPriority()));
                return MixingBehavior::UNDEFINED;
            }
            auto lowPrioChannelName = lowPrioChannel->getName();
            auto lowPrioChannelPrimaryActivity = lowPrioChannel->getPrimaryActivity();
            if (!lowPrioChannelPrimaryActivity) {
                ACSDK_ERROR(LX("getMixingBehaviorFailed").d("No PrimaryActivity on lowPrioChannel", lowPrioChannelName));
                return MixingBehavior::UNDEFINED;
            }
            auto highPrioChannelName = highPrioChannel->getName();
            auto highPrioChannelPrimaryActivity = highPrioChannel->getPrimaryActivity();
            if (!highPrioChannelPrimaryActivity) {
                ACSDK_ERROR(LX("getMixingBehaviorFailed").d("No PrimaryActivity on highPrioChannel", highPrioChannelName));
                return MixingBehavior::UNDEFINED;
            }
            return m_interruptModel->getMixingBehavior(lowPrioChannelName, lowPrioChannelPrimaryActivity->getContentType(), highPrioChannelName,
                                                       highPrioChannelPrimaryActivity->getContentType());
        }
        void FocusManager::acquireChannelHelper(shared_ptr<Channel> channelToAcquire, shared_ptr<FocusManagerInterface::Activity> channelActivity) {
            unique_lock<mutex> lock(m_mutex);
            shared_ptr<Channel> foregroundChannel = getHighestPriorityActiveChannelLocked();
            m_activeChannels.insert(channelToAcquire);
            lock.unlock();
            ACSDK_DEBUG5(LX(__func__).d("incomingChannel", channelToAcquire->getName()).d("incomingInterface", channelActivity->getInterface()));
            channelToAcquire->setPrimaryActivity(std::move(channelActivity));
            if (!foregroundChannel) setChannelFocus(channelToAcquire, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            else if (foregroundChannel == channelToAcquire) {
                setChannelFocus(channelToAcquire, FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
            } else if (*channelToAcquire > *foregroundChannel) {
                setBackgroundChannelMixingBehavior(channelToAcquire);
                setChannelFocus(channelToAcquire, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            } else {
                auto mixingBehavior = getMixingBehavior(channelToAcquire, foregroundChannel);
                setChannelFocus(channelToAcquire, FocusState::BACKGROUND, mixingBehavior);
            }
            notifyActivityTracker();
        }
        void FocusManager::releaseChannelHelper(shared_ptr<Channel> channelToRelease, shared_ptr<ChannelObserverInterface> channelObserver,
                                                shared_ptr<promise<bool>> releaseChannelSuccess, const std::string& name) {
            ACSDK_DEBUG5(LX(__func__).d("channelToRelease", channelToRelease->getName()));
            bool success = channelToRelease->releaseActivity(std::move(channelObserver));
            releaseChannelSuccess->set_value(success);
            if (!success) {
                ACSDK_ERROR(LX(__func__).d("reason", "releaseActivityFailed").d("channel", channelToRelease).d("interface", channelToRelease->getInterface()));
                return;
            }
            if (!channelToRelease->isActive()) {
                unique_lock<mutex> lock(m_mutex);
                m_activeChannels.erase(channelToRelease);
                lock.unlock();
                setChannelFocus(channelToRelease, FocusState::NONE, MixingBehavior::MUST_STOP);
            }
            foregroundHighestPriorityActiveChannel();
            notifyActivityTracker();
        }
        void FocusManager::stopForegroundActivityHelper(shared_ptr<Channel> foregroundChannel, string foregroundChannelInterface) {
            if (foregroundChannelInterface != foregroundChannel->getInterface()) return;
            ACSDK_DEBUG5(LX(__func__).d("interface", foregroundChannelInterface));
            bool success = foregroundChannel->releaseActivity(foregroundChannel->getInterface());
            if (!success) {
                ACSDK_ERROR(LX(__func__).d("reason", "releaseActivityFailed").d("channel", foregroundChannel).d("interface", foregroundChannel->getInterface()));
            }
            if (!foregroundChannel->isActive()) {
                ACSDK_DEBUG1(LX(__func__).m("Channel is not active ... releasing"));
                unique_lock<mutex> lock(m_mutex);
                m_activeChannels.erase(foregroundChannel);
                lock.unlock();
                setChannelFocus(foregroundChannel, FocusState::NONE, MixingBehavior::MUST_STOP);
            }
            foregroundHighestPriorityActiveChannel();
            notifyActivityTracker();
        }
        void FocusManager::stopAllActivitiesHelper(const ChannelsToInterfaceNamesMap& channelsOwnersMap) {
            ACSDK_DEBUG3(LX(__func__));
            set<shared_ptr<Channel>> channelsToClear;
            unique_lock<mutex> lock(m_mutex);
            for (const auto& channelAndInterface : channelsOwnersMap) {
                auto channel = channelAndInterface.first;
                auto interfaceName = channelAndInterface.second;
                ACSDK_DEBUG3(LX(__func__).d("channel", channel).d("interface", interfaceName));
                bool success = channel->releaseActivity(channelAndInterface.second);
                if (!success) {
                    ACSDK_ERROR(LX(__func__).d("reason", "releaseActivityFailed").d("channel", channel).d("interface", interfaceName));
                }
                channelsToClear.insert(channel);
            }
            lock.unlock();
            for (const auto& channel : channelsToClear) {
                if (!channel->isActive()) {
                    unique_lock<mutex> lock(m_mutex);
                    m_activeChannels.erase(channel);
                    lock.unlock();
                    setChannelFocus(channel, FocusState::NONE, MixingBehavior::MUST_STOP);
                }
            }
            foregroundHighestPriorityActiveChannel();
            notifyActivityTracker();
        }
        shared_ptr<Channel> FocusManager::getChannel(const string& channelName) const {
            auto search = m_allChannels.find(channelName);
            if (search != m_allChannels.end()) return search->second;
            return nullptr;
        }
        shared_ptr<Channel> FocusManager::getHighestPriorityActiveChannelLocked() const {
            if (m_activeChannels.empty()) return nullptr;
            return *m_activeChannels.begin();
        }
        bool FocusManager::isChannelForegroundedLocked(const shared_ptr<Channel>& channel) const {
            return getHighestPriorityActiveChannelLocked() == channel;
        }
        bool FocusManager::doesChannelNameExist(const string& name) const {
            return m_allChannels.find(name) != m_allChannels.end();
        }
        bool FocusManager::doesChannelPriorityExist(const unsigned int priority) const {
            for (const auto& m_allChannel : m_allChannels) {
                if (m_allChannel.second->getPriority() == priority) return true;
            }
            return false;
        }
        void FocusManager::modifyContentType(const string& channelName, const string& interfaceName, ContentType contentType) {
            auto channel = getChannel(channelName);
            if (!channel) {
                ACSDK_ERROR(LX("modifyContentTypeFailed").d("reason", "channelNotFound").d("channel", channelName));
                return;
            }
            auto activity = channel->getActivity(interfaceName);
            if (!activity) {
                ACSDK_ERROR(LX("modifyContentTypeFailed").d("no activity found associated with interfaceName", interfaceName));
                return;
            }
            if (contentType == activity->getContentType()) {
                ACSDK_WARN(LX("modifyContentTypeFailed").d("no contentType to modify it is already identical: ", contentType));
                return;
            }
            activity->setContentType(contentType);
            unique_lock<mutex> lock(m_mutex);
            shared_ptr<Channel> foregroundChannel = getHighestPriorityActiveChannelLocked();
            lock.unlock();
            setBackgroundChannelMixingBehavior(foregroundChannel);
        }
        void FocusManager::setBackgroundChannelMixingBehavior(shared_ptr<Channel> foregroundChannel) {
            unique_lock<mutex> lock(m_mutex);
            auto channelIter = m_activeChannels.find(foregroundChannel);
            if (channelIter == m_activeChannels.end()) {
                ACSDK_ERROR(LX("setBackgroundChannelMixingBehaviorFailed").d("Could not find channel", foregroundChannel->getName()));
                return;
            }
            channelIter++;
            for (; channelIter != m_activeChannels.end(); channelIter++) {
                auto mixingBehavior = getMixingBehavior(*channelIter, foregroundChannel);
                lock.unlock();
                setChannelFocus(*channelIter, FocusState::BACKGROUND, mixingBehavior);
                lock.lock();
            }
        }
        void FocusManager::foregroundHighestPriorityActiveChannel() {
            unique_lock<mutex> lock(m_mutex);
            shared_ptr<Channel> channelToForeground = getHighestPriorityActiveChannelLocked();
            lock.unlock();
            if (channelToForeground) {
                setBackgroundChannelMixingBehavior(channelToForeground);
                setChannelFocus(channelToForeground, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
        }
        void FocusManager::notifyActivityTracker() {
            unique_lock<mutex> lock(m_mutex);
            for (const auto& channel : m_allChannels) {
                auto activityUpdates = channel.second->getActivityUpdates();
                for (const auto& activity : activityUpdates) {
                    m_activityUpdates.push_back(activity);
                    ACSDK_DEBUG1(LX(__func__).d("name", activity.name).d("interfaceName", activity.interfaceName).d("focusState", activity.focusState));
                }
            }
            lock.unlock();
            if (m_activityTracker && !m_activityUpdates.empty()) m_activityTracker->notifyOfActivityUpdates(m_activityUpdates);
            m_activityUpdates.clear();
        }
        const vector<FocusManager::ChannelConfiguration> FocusManager::getDefaultAudioChannels() {
            static const vector<FocusManager::ChannelConfiguration> defaultAudioChannels = {
                {FocusManagerInterface::DIALOG_CHANNEL_NAME, FocusManagerInterface::DIALOG_CHANNEL_PRIORITY},
                {FocusManagerInterface::ALERT_CHANNEL_NAME, FocusManagerInterface::ALERT_CHANNEL_PRIORITY},
                {FocusManagerInterface::COMMUNICATIONS_CHANNEL_NAME, FocusManagerInterface::COMMUNICATIONS_CHANNEL_PRIORITY},
                {FocusManagerInterface::CONTENT_CHANNEL_NAME, FocusManagerInterface::CONTENT_CHANNEL_PRIORITY}
            };
            return defaultAudioChannels;
        }
        const vector<FocusManager::ChannelConfiguration> FocusManager::getDefaultVisualChannels() {
            static const vector<FocusManager::ChannelConfiguration> defaultVisualChannels = {
                {FocusManagerInterface::VISUAL_CHANNEL_NAME, FocusManagerInterface::VISUAL_CHANNEL_PRIORITY}
            };
            return defaultVisualChannels;
        }
        bool FocusManager::ChannelConfiguration::readChannelConfiguration(const string& channelTypeKey,
                                                                          vector<FocusManager::ChannelConfiguration>* virtualChannelConfigurations) {
            if (!virtualChannelConfigurations) {
                ACSDK_ERROR(LX("readChannelConfigurationFailed").d("reason", "nullVirtualChannelConfiguration"));
                return false;
            }
            auto configRoot = ConfigurationNode::getRoot()[VIRTUAL_CHANNELS_CONFIG_KEY];
            if (!configRoot) {
                ACSDK_DEBUG9(LX(__func__).m("noConfigurationRoot"));
                return true;
            }
            bool returnValue = true;
            auto channelArray = configRoot.getArray(channelTypeKey);
            if (!channelArray) {
                ACSDK_DEBUG9(LX(__func__).d("key", channelTypeKey).m("keyNotFoundOrNotAnArray"));
            } else {
                for (size_t i = 0; i < channelArray.getArraySize(); i++) {
                    auto elem = channelArray[i];
                    if (!elem) {
                        ACSDK_ERROR(LX("readChannelConfigurationFailed").d("reason", "noNameKey"));
                        returnValue = false;
                        break;
                    }
                    string name;
                    if (!elem.getString(CHANNEL_NAME_KEY, &name)) {
                        ACSDK_ERROR(LX("readChannelConfigurationFailed").d("reason", "noNameKey"));
                        returnValue = false;
                        break;
                    }
                    int priority = 0;
                    if (!elem.getInt(CHANNEL_PRIORITY_KEY, &priority)) {
                        ACSDK_ERROR(LX("readChannelConfigurationFailed").d("reason", "noPriorityKey"));
                        returnValue = false;
                        break;
                    }
                    if (priority < 0) {
                        ACSDK_ERROR(LX("ChannelConfigurationFailed").d("reason", "invalidPriority").d("priority", priority));
                        returnValue = false;
                        break;
                    }
                    FocusManager::ChannelConfiguration channelConfig{name, static_cast<unsigned int>(priority)};
                    virtualChannelConfigurations->push_back(channelConfig);
                }
            }
            return returnValue;
        }
    }
}