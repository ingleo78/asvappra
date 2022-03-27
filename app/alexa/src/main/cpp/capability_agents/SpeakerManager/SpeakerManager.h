#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_SPEAKERMANAGER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_SPEAKERMANAGER_H_

#include <future>
#include <map>
#include <memory>
#include <unordered_set>
#include <vector>
#include <avs/AVSDirective.h>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/SpeakerManagerInterface.h>
#include <sdkinterfaces/SpeakerManagerObserverInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <util/RetryTimer.h>
#include <util/WaitEvent.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace metrics;
            using namespace threading;
            using namespace rapidjson;
            using Type = ChannelVolumeInterface::Type;
            using Source = SpeakerManagerObserverInterface::Source;
            using SpeakerSettings = SpeakerInterface::SpeakerSettings;
            class SpeakerManager : public CapabilityAgent, public CapabilityConfigurationInterface, public SpeakerManagerInterface,
                                   public RequiresShutdown {
            public:
                using DirectiveInfo = CapabilityAgent::DirectiveInfo;
                static shared_ptr<SpeakerManager> create(const vector<shared_ptr<ChannelVolumeInterface>>& volumeInterfaces,
                                                         shared_ptr<ContextManagerInterface> contextManager,
                                                         shared_ptr<MessageSenderInterface> messageSender,
                                                         shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                         shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                void doShutdown() override;
                future<bool> setVolume(Type type, int8_t volume, const NotificationProperties& properties) override;
                future<bool> adjustVolume(Type type, int8_t delta, const NotificationProperties& properties) override;
                future<bool> setMute(Type type, bool mute, const NotificationProperties& properties) override;
                future<bool> setVolume(Type type, int8_t volume, bool forceNoNotifications = false, Source source = Source::LOCAL_API) override;
                future<bool> adjustVolume(Type type, int8_t delta, bool forceNoNotifications = false, Source source = Source::LOCAL_API) override;
                future<bool> setMute(Type type, bool mute, bool forceNoNotifications = false, Source source = Source::LOCAL_API) override;
            #ifndef ENABLE_MAXVOLUME_SETTING
                future<bool> setMaximumVolumeLimit(const int8_t maximumVolumeLimit);
            #endif
                future<bool> getSpeakerSettings(Type type, SpeakerSettings* settings) override;
                void addSpeakerManagerObserver(shared_ptr<SpeakerManagerObserverInterface> observer) override;
                void removeSpeakerManagerObserver(shared_ptr<SpeakerManagerObserverInterface> observer) override;
                void addChannelVolumeInterface(shared_ptr<ChannelVolumeInterface> channelVolumeInterface) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
            private:
                SpeakerManager(const vector<shared_ptr<ChannelVolumeInterface>>& groupVolumeInterfaces, shared_ptr<ContextManagerInterface> contextManager,
                               shared_ptr<MessageSenderInterface> messageSender, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                               const int minUnmuteVolume, shared_ptr<MetricRecorderInterface> metricRecorder);
                bool parseDirectivePayload(string payload, Document* document);
                void executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info);
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void sendExceptionEncountered(shared_ptr<DirectiveInfo> info, const string& message, ExceptionErrorType type);
                bool updateContextManager(const Type& type, const SpeakerSettings& settings);
                void executeSendSpeakerSettingsChangedEvent(const string& eventName, SpeakerSettings settings);
                bool executeSetVolume(Type type, int8_t volume, const NotificationProperties& properties);
                bool executeRestoreVolume(Type type, Source source);
                bool executeAdjustVolume(Type type, int8_t delta, const NotificationProperties& properties);
                bool executeSetMute(Type type, bool mute, const NotificationProperties& properties);
            #ifndef ENABLE_MAXVOLUME_SETTING
                bool executeSetMaximumVolumeLimit(const int8_t maximumVolumeLimit);
            #endif
                bool executeGetSpeakerSettings(Type type, SpeakerSettings* settings);
                void executeNotifySettingsChanged(const SpeakerSettings& settings, const string& eventName, const Source& source, const Type& type);
                void executeNotifyObserver(const Source& source, const Type& type, const SpeakerSettings& settings);
                bool validateSpeakerSettingsConsistency(Type type, SpeakerSettings* settings);
                int8_t getMaximumVolumeLimit();
                template <typename Task, typename... Args> void retryAndApplySettings(Task task, Args&&... args);
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<MessageSenderInterface> m_messageSender;
                const int m_minUnmuteVolume;
                multimap<Type, shared_ptr<ChannelVolumeInterface>> m_speakerMap;
                unordered_set<shared_ptr<SpeakerManagerObserverInterface>> m_observers;
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                WaitEvent m_waitCancelEvent;
                RetryTimer m_retryTimer;
                const size_t m_maxRetries;
                int8_t m_maximumVolumeLimit;
                Executor m_executor;
            };
        }
    }
}
#endif