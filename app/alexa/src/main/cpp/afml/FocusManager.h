#ifndef ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_FOCUSMANAGER_H_
#define ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_FOCUSMANAGER_H_

#include <algorithm>
#include <map>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sdkinterfaces/ChannelObserverInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <threading/Executor.h>
#include <interrupt_model/InterruptModel.h>
#include "Channel.h"
#include "ActivityTrackerInterface.h"

namespace alexaClientSDK {
    namespace afml {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace threading;
        class FocusManager : public FocusManagerInterface {
        public:
            class ChannelConfiguration {
            public:
                ChannelConfiguration(const string& configName, unsigned int configPriority) : name{configName}, priority{configPriority} {}
                string toString() const {
                    return "name:'" + name + "', priority:" + std::to_string(priority);
                }
                string name;
                unsigned int priority;
                static bool readChannelConfiguration(const string& channelTypeKey, vector<FocusManager::ChannelConfiguration>* virtualChannelConfigurations);
            };
            FocusManager(const vector<ChannelConfiguration>& channelConfigurations, shared_ptr<ActivityTrackerInterface> activityTrackerInterface = nullptr,
                         const vector<ChannelConfiguration>& virtualChannelConfigurations = vector<ChannelConfiguration>(),
                         shared_ptr<interruptModel::InterruptModel> interruptModel = nullptr);
            bool acquireChannel(const string& channelName, shared_ptr<ChannelObserverInterface> channelObserver, const string& interfaceName) override;
            bool acquireChannel(const string& channelName, shared_ptr<FocusManagerInterface::Activity> channelActivity) override;
            future<bool> releaseChannel(const string& channelName, shared_ptr<ChannelObserverInterface> channelObserver) override;
            void stopForegroundActivity() override;
            void stopAllActivities() override;
            void addObserver(const shared_ptr<FocusManagerObserverInterface>& observer) override;
            void removeObserver(const shared_ptr<FocusManagerObserverInterface>& observer) override;
            void modifyContentType(const string& channelName, const string& interfaceName, ContentType contentType) override;
            static const vector<FocusManager::ChannelConfiguration> getDefaultAudioChannels();
            static const vector<FocusManager::ChannelConfiguration> getDefaultVisualChannels();
        private:
            using ChannelsToInterfaceNamesMap = map<shared_ptr<Channel>, string>;
            struct ChannelPtrComparator {
                bool operator()(const shared_ptr<Channel>& first, const shared_ptr<Channel>& second) const {
                    return *first > *second;
                }
            };
            void readChannelConfiguration(const vector<ChannelConfiguration>& channelConfigurations, bool isVirtual);
            void setChannelFocus(const shared_ptr<Channel>& channel, FocusState focus, MixingBehavior behavior, bool forceUpdate = false);
            void acquireChannelHelper(shared_ptr<Channel> channelToAcquire, shared_ptr<FocusManagerInterface::Activity> channelActivity);
            void releaseChannelHelper(shared_ptr<Channel> channelToRelease, shared_ptr<ChannelObserverInterface> channelObserver,
                                      shared_ptr<promise<bool>> releaseChannelSuccess, const string& channelName);
            void stopForegroundActivityHelper(shared_ptr<Channel> foregroundChannel, string foregroundChannelInterface);
            void stopAllActivitiesHelper(const ChannelsToInterfaceNamesMap& channelsOwnersMap);
            shared_ptr<Channel> getChannel(const string& channelName) const;
            shared_ptr<Channel> getHighestPriorityActiveChannelLocked() const;
            bool isChannelForegroundedLocked(const shared_ptr<Channel>& channel) const;
            bool doesChannelNameExist(const string& name) const;
            bool doesChannelPriorityExist(const unsigned int priority) const;
            void foregroundHighestPriorityActiveChannel();
            void notifyActivityTracker();
            MixingBehavior getMixingBehavior(shared_ptr<Channel> lowPrioChannel, shared_ptr<Channel> highPrioChannel);
            void setBackgroundChannelMixingBehavior(shared_ptr<Channel> foregroundChannel);
            mutex m_mutex;
            unordered_map<string, shared_ptr<Channel>> m_allChannels;
            set<shared_ptr<Channel>, ChannelPtrComparator> m_activeChannels;
            unordered_set<shared_ptr<FocusManagerObserverInterface>> m_observers;
            vector<Channel::State> m_activityUpdates;
            shared_ptr<ActivityTrackerInterface> m_activityTracker;
            shared_ptr<interruptModel::InterruptModel> m_interruptModel;
            Executor m_executor;
        };
    }
}
#endif