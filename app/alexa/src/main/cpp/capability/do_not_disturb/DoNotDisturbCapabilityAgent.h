#ifndef ACSDKDONOTDISTURB_DONOTDISTURBCAPABILITYAGENT_H_
#define ACSDKDONOTDISTURB_DONOTDISTURBCAPABILITYAGENT_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <json/document.h>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/ExceptionErrorType.h>
#include <sdkinterfaces/CapabilitiesDelegateInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Setting.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSenderInterface.h>
#include <settings/Storage/DeviceSettingStorageInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace doNotDisturb {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace json;
            using namespace logger;
            using namespace settings;
            using namespace storage;
            using namespace threading;
            using namespace rapidjson;
            class DoNotDisturbCapabilityAgent : public enable_shared_from_this<DoNotDisturbCapabilityAgent>, public CapabilityAgent, public CapabilityConfigurationInterface,
                                                public ConnectionStatusObserverInterface, public RequiresShutdown, public SettingEventSenderInterface {
            public:
                ~DoNotDisturbCapabilityAgent() override = default;
                static shared_ptr<DoNotDisturbCapabilityAgent> create(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                      shared_ptr<MessageSenderInterface> messageSender,
                                                                      shared_ptr<DeviceSettingStorageInterface> settingsStorage);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
                void doShutdown() override;
                shared_future<bool> sendChangedEvent(const string& value) override;
                shared_future<bool> sendReportEvent(const string& value) override;
                shared_future<bool> sendStateReportEvent(const string& payload) override;
                void cancel() override;
                void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;
                shared_ptr<DoNotDisturbSetting> getDoNotDisturbSetting() const;
                static SettingEventMetadata getDoNotDisturbEventsMetadata();
            private:
                DoNotDisturbCapabilityAgent(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender, shared_ptr<MessageSenderInterface> messageSender);
                bool initialize(shared_ptr<DeviceSettingStorageInterface> settingsStorage);
                shared_future<MessageRequestObserverInterface::Status> sendDNDEvent(const string& eventName, const string& value);
                void generateCapabilityConfiguration();
                bool handleSetDoNotDisturbDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& document);
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<Setting<bool>> m_dndModeSetting;
                bool m_isConnected;
                mutex m_connectedStateMutex;
                atomic_bool m_hasOfflineChanges;
                Executor m_executor;
            };
        }
    }
}
#endif