#include <logger/Logger.h>
#include "Settings/MockDeviceSettingStorage.h"
#include "Settings/MockSetting.h"
#include "Settings/MockSettingEventSender.h"
#include "CloudControlledSettingProtocol.h"

namespace alexaClientSDK {
    namespace settings {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace storage;
        static const string TAG("CloudControlledSettingProtocol");
        #define LX(event) LogEntry(TAG, event)
        unique_ptr<CloudControlledSettingProtocol> CloudControlledSettingProtocol::create(const SettingEventMetadata& metadata,
                                                                                          shared_ptr<SettingEventSenderInterface> eventSender,
                                                                                          shared_ptr<DeviceSettingStorageInterface> settingStorage,
                                                                                          shared_ptr<AVSConnectionManagerInterface> connectionManager) {
            auto sharedProtocol = SharedAVSSettingProtocol::create(metadata, eventSender, settingStorage, connectionManager, true);
            if (!sharedProtocol) {
                ACSDK_ERROR(LX("createFailed").d("reason", "cannot create shared Protocol"));
                return nullptr;
            }
            return unique_ptr<CloudControlledSettingProtocol>(new CloudControlledSettingProtocol(move(sharedProtocol)));
        }
        CloudControlledSettingProtocol::CloudControlledSettingProtocol(unique_ptr<SharedAVSSettingProtocol> sharedProtocol) : m_protocolImpl{move(sharedProtocol)} {}
        SetSettingResult CloudControlledSettingProtocol::localChange(SettingProtocolInterface::ApplyChangeFunction applyChange,
                                                                     SettingProtocolInterface::RevertChangeFunction revertChange,
                                                                     SettingProtocolInterface::SettingNotificationFunction notifyObservers) {
            ACSDK_ERROR(LX("localChangeFailed").d("reason", "unsupportedOperation"));
            return SetSettingResult::UNSUPPORTED_OPERATION;
        }
        bool CloudControlledSettingProtocol::avsChange(SettingProtocolInterface::ApplyChangeFunction applyChange,
                                                       SettingProtocolInterface::RevertChangeFunction revertChange,
                                                       SettingProtocolInterface::SettingNotificationFunction notifyObservers) {
            return m_protocolImpl->avsChange(applyChange, revertChange, notifyObservers);
        }
        bool CloudControlledSettingProtocol::restoreValue(SettingProtocolInterface::ApplyDbChangeFunction applyChange,
                                                          SettingProtocolInterface::SettingNotificationFunction notifyObservers) {
            return m_protocolImpl->restoreValue(applyChange, notifyObservers);
        }
        bool CloudControlledSettingProtocol::clearData() {
            return m_protocolImpl->clearData();
        }
    }
}