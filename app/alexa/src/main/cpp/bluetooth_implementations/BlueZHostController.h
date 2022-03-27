#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZHOSTCONTROLLER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZHOSTCONTROLLER_H_

#include <string>
#include <memory>
#include <mutex>
#include <gio/gio.h>
#include <sdkinterfaces/Bluetooth/BluetoothHostControllerInterface.h>
#include <util/MacAddressString.h>
#include <threading/Executor.h>
#include "BlueZUtils.h"
#include "DBusPropertiesProxy.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace sdkInterfaces::bluetooth;
            class BlueZHostController : public BluetoothHostControllerInterface {
            public:
                virtual ~BlueZHostController() = default;
                string getMac() const override;
                string getFriendlyName() const override;
                bool isDiscoverable() const override;
                future<bool> enterDiscoverableMode() override;
                future<bool> exitDiscoverableMode() override;
                bool isScanning() const override;
                future<bool> startScan() override;
                future<bool> stopScan() override;
                static unique_ptr<BlueZHostController> create(const string& adapterObjectPath);
                void onPropertyChanged(const GVariantMapReader& changesMap);
            private:
                BlueZHostController(const string& adapterObjectPath);
                bool init();
                future<bool> setDiscoverable(bool discoverable);
                future<bool> changeScanState(bool scanning);
                string m_adapterObjectPath;
                unique_ptr<MacAddressString> m_mac;
                mutable mutex m_adapterMutex;
                string m_friendlyName;
                shared_ptr<DBusPropertiesProxy> m_adapterProperties;
                shared_ptr<DBusProxy> m_adapter;
            };
        }
    }
}
#endif