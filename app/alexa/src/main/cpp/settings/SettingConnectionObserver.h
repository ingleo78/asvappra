#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGCONNECTIONOBSERVER_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGCONNECTIONOBSERVER_H_

#include <functional>
#include <memory>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>

namespace alexaClientSDK {
    namespace settings {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        class SettingConnectionObserver : public ConnectionStatusObserverInterface, public enable_shared_from_this<SettingConnectionObserver> {
        public:
            using ConnectionStatusCallback = function<void(bool isConnected)>;
            static shared_ptr<SettingConnectionObserver> create(ConnectionStatusCallback notifyCallback);
            ~SettingConnectionObserver() = default;
            void onConnectionStatusChanged(ConnectionStatusObserverInterface::Status status, ConnectionStatusObserverInterface::ChangedReason reason) override;
        private:
            SettingConnectionObserver(ConnectionStatusCallback notifyCallback);
            ConnectionStatusCallback m_connectionStatusCallback;
        };
    }
}
#endif