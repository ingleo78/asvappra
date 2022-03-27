#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZBLUETOOTHDEVICE_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZBLUETOOTHDEVICE_H_

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <gio/gio.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <threading/Executor.h>
#include <util/bluetooth/SDPRecords.h>
#include "DBusPropertiesProxy.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            using namespace utils;
            using namespace utils::bluetooth;
            using namespace threading;
            class BlueZDeviceManager;
            class BlueZBluetoothDevice : public BluetoothDeviceInterface, public enable_shared_from_this<BlueZBluetoothDevice> {
            public:
                enum class BlueZDeviceState {
                    FOUND,
                    UNPAIRED,
                    PAIRED,
                    IDLE,
                    DISCONNECTED,
                    CONNECTED,
                    CONNECTION_FAILED
                };
                ~BlueZBluetoothDevice() override;
                string getMac() const override;
                string getFriendlyName() const override;
                DeviceState getDeviceState() override;
                MetaData getDeviceMetaData() override;
                bool isPaired() override;
                future<bool> pair() override;
                future<bool> unpair() override;
                bool isConnected() override;
                future<bool> connect() override;
                future<bool> disconnect() override;
                vector<shared_ptr<SDPRecordInterface>> getSupportedServices() override;
                shared_ptr<BluetoothServiceInterface> getService(string uuid) override;
                MediaStreamingState getStreamingState() override;
                bool toggleServiceConnection(bool enabled, shared_ptr<BluetoothServiceInterface> service) override;
                static shared_ptr<BlueZBluetoothDevice> create(const string& mac, const string& objectPath, shared_ptr<BlueZDeviceManager> deviceManager);
                string getObjectPath() const;
                void onPropertyChanged(const GVariantMapReader& changesMap);
            private:
                BlueZBluetoothDevice(const string& mac, const string& objectPath, shared_ptr<BlueZDeviceManager> deviceManager);
                bool init();
                bool updateFriendlyName();
                unordered_set<string> getServiceUuids();
                unordered_set<string> getServiceUuids(GVariant* array);
                bool initializeServices(const std::unordered_set<std::string>& uuids);
                bool executePair();
                bool executeUnpair();
                bool executeConnect();
                bool executeDisconnect();
                bool executeIsPaired();
                bool executeIsConnected();
                bool executeToggleServiceConnection(bool enabled, shared_ptr<BluetoothServiceInterface> service);
                bool queryDeviceProperty(const string& name, bool* value);
                DeviceState convertToDeviceState(BlueZDeviceState bluezDeviceState);
                void transitionToState(BlueZDeviceState newState, bool sendEvent);
                bool executeIsConnectedToRelevantServices();
                bool serviceExists(const string& uuid);
                bool insertService(shared_ptr<BluetoothServiceInterface> service);
                template <typename ServiceType> shared_ptr<ServiceType> getService();
                template <typename BlueZServiceType> bool initializeService();
                shared_ptr<DBusProxy> m_deviceProxy;
                shared_ptr<DBusPropertiesProxy> m_propertiesProxy;
                const string m_mac;
                const string m_objectPath;
                string m_friendlyName;
                mutex m_servicesMapMutex;
                unordered_map<string, shared_ptr<BluetoothServiceInterface>> m_servicesMap;
                BlueZDeviceState m_deviceState;
                unique_ptr<BluetoothDeviceInterface::MetaData> m_metaData;
                shared_ptr<BlueZDeviceManager> m_deviceManager;
                Executor m_executor;
            };
            inline string deviceStateToString(BlueZBluetoothDevice::BlueZDeviceState state) {
                switch(state) {
                    case BlueZBluetoothDevice::BlueZDeviceState::FOUND: return "FOUND";
                    case BlueZBluetoothDevice::BlueZDeviceState::UNPAIRED: return "UNPAIRED";
                    case BlueZBluetoothDevice::BlueZDeviceState::PAIRED: return "PAIRED";
                    case BlueZBluetoothDevice::BlueZDeviceState::IDLE: return "IDLE";
                    case BlueZBluetoothDevice::BlueZDeviceState::DISCONNECTED: return "DISCONNECTED";
                    case BlueZBluetoothDevice::BlueZDeviceState::CONNECTED: return "CONNECTED";
                    case BlueZBluetoothDevice::BlueZDeviceState::CONNECTION_FAILED: return "CONNECTION_FAILED";
                }
                return "UNKNOWN";
            }
            inline ostream& operator<<(ostream& stream, const BlueZBluetoothDevice::BlueZDeviceState& state) {
                return stream << deviceStateToString(state);
            }
        }
    }
}
#endif