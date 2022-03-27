#ifndef ACSDKDONOTDISTURB_DNDSETTINGPROTOCOL_H_
#define ACSDKDONOTDISTURB_DNDSETTINGPROTOCOL_H_

#include <memory>
#include <string>
#include <utility>
#include <threading/Executor.h>
#include <settings/SetSettingResult.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSenderInterface.h>
#include <settings/SettingObserverInterface.h>
#include <settings/SettingProtocolInterface.h>
#include <settings/SettingStatus.h>
#include <settings/Storage/DeviceSettingStorageInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace doNotDisturb {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace settings;
            using namespace storage;
            using namespace threading;
            class DNDSettingProtocol : public SettingProtocolInterface {
            public:
                static unique_ptr<DNDSettingProtocol> create(const SettingEventMetadata& metadata, shared_ptr<SettingEventSenderInterface> eventSender,
                                                             shared_ptr<DeviceSettingStorageInterface> settingStorage);
                SetSettingResult localChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) override;
                bool avsChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) override;
                bool restoreValue(ApplyDbChangeFunction applyChange, SettingNotificationFunction notifyObservers) override;
                bool clearData() override;
                ~DNDSettingProtocol() = default;
            private:
                DNDSettingProtocol(const string& settingKey, shared_ptr<SettingEventSenderInterface> eventSender, shared_ptr<DeviceSettingStorageInterface> settingStorage);
                const string m_key;
                shared_ptr<SettingEventSenderInterface> m_eventSender;
                shared_ptr<DeviceSettingStorageInterface> m_storage;
                Executor m_executor;
            };
        }
    }
}
#endif