#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SHAREDAVSSETTINGPROTOCOL_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SHAREDAVSSETTINGPROTOCOL_H_

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <threading/Executor.h>
#include "Storage/DeviceSettingStorageInterface.h"
#include "SetSettingResult.h"
#include "SettingConnectionObserver.h"
#include "SettingEventMetadata.h"
#include "SettingEventSenderInterface.h"
#include "SettingObserverInterface.h"
#include "SettingProtocolInterface.h"
#include "SettingStatus.h"

namespace alexaClientSDK {
    namespace settings {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace storage;
        using namespace threading;
        class SharedAVSSettingProtocol : public SettingProtocolInterface {
        public:
            static unique_ptr<SharedAVSSettingProtocol> create(const SettingEventMetadata& metadata, shared_ptr<SettingEventSenderInterface> eventSender,
                                                               shared_ptr<DeviceSettingStorageInterface> settingStorage,
                                                               shared_ptr<AVSConnectionManagerInterface> connectionManager, bool isDefaultCloudAuthoritative = false);
            SetSettingResult localChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) override;
            bool avsChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) override;
            bool restoreValue(ApplyDbChangeFunction applyChange, SettingNotificationFunction notifyObservers) override;
            bool clearData() override;
            void connectionStatusChangeCallback(bool isConnected);
            ~SharedAVSSettingProtocol();
        private:
            struct Request {
                Request(ApplyChangeFunction applyFn, RevertChangeFunction revertFn, SettingNotificationFunction notifyFn);
                ApplyChangeFunction applyChange;
                RevertChangeFunction revertChange;
                SettingNotificationFunction notifyObservers;
            };
            SharedAVSSettingProtocol(const string& settingKey, shared_ptr<SettingEventSenderInterface> eventSender,
                                     shared_ptr<DeviceSettingStorageInterface> settingStorage, shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                     bool isDefaultCloudAuthoritative);
            void executeSynchronizeOnConnected();
            const string m_key;
            const bool m_isDefaultCloudAuthoritative;
            shared_ptr<SettingEventSenderInterface> m_eventSender;
            shared_ptr<DeviceSettingStorageInterface> m_storage;
            shared_ptr<AVSConnectionManagerInterface> m_connectionManager;
            shared_ptr<SettingConnectionObserver> m_connectionObserver;
            unique_ptr<Request> m_pendingRequest;
            mutex m_requestLock;
            Executor m_executor;
        };
    }
}
#endif