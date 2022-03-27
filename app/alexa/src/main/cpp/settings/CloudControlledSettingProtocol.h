#include <memory>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include "SetSettingResult.h"
#include "SettingEventMetadata.h"
#include "SettingEventSenderInterface.h"
#include "SettingProtocolInterface.h"
#include "SharedAVSSettingProtocol.h"
#include "Storage/DeviceSettingStorageInterface.h"

#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_CLOUDCONTROLLEDSETTINGPROTOCOL_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_CLOUDCONTROLLEDSETTINGPROTOCOL_H_

namespace alexaClientSDK {
    namespace settings {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace storage;
        class CloudControlledSettingProtocol : public SettingProtocolInterface {
        public:
            static unique_ptr<CloudControlledSettingProtocol> create(const SettingEventMetadata& metadata, shared_ptr<SettingEventSenderInterface> eventSender,
                                                                     shared_ptr<DeviceSettingStorageInterface> settingStorage,
                                                                     shared_ptr<AVSConnectionManagerInterface> connectionManager);
            ~CloudControlledSettingProtocol() = default;
            SetSettingResult localChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) override;
            bool avsChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) override;
            bool restoreValue(ApplyDbChangeFunction applyChange, SettingNotificationFunction notifyObservers) override;
            bool clearData() override;
        private:
            CloudControlledSettingProtocol(unique_ptr<SharedAVSSettingProtocol> sharedProtocol);
            unique_ptr<SharedAVSSettingProtocol> m_protocolImpl;
        };
    }
}

#endif  // ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_CLOUDCONTROLLEDSETTINGPROTOCOL_H_
