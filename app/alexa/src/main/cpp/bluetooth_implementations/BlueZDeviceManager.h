#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZDEVICEMANAGER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZDEVICEMANAGER_H_

#include <list>
#include <map>
#include <mutex>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>
#include <sdkinterfaces/Bluetooth/BluetoothHostControllerInterface.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include <util/bluetooth/BluetoothEvents.h>
#include <util/RequiresShutdown.h>
#include <gio/gio.h>
#include "BlueZHostController.h"
#include "BlueZUtils.h"
#include "MPRISPlayer.h"
#include "PairingAgent.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace sdkInterfaces::bluetooth;
            using namespace utils::bluetooth;
            class MediaEndpoint;
            class BlueZBluetoothDevice;
            class BlueZDeviceManager : public BluetoothDeviceManagerInterface , public RequiresShutdown , public enable_shared_from_this<BlueZDeviceManager> {
            public:
                static shared_ptr<BlueZDeviceManager> create(shared_ptr<BluetoothEventBus> eventBus);
                virtual ~BlueZDeviceManager() override;
                shared_ptr<BluetoothHostControllerInterface> getHostController();
                list<std::shared_ptr<BluetoothDeviceInterface>> getDiscoveredDevices() override;
                shared_ptr<BluetoothEventBus> getEventBus() override;
                void doShutdown() override;
                shared_ptr<MediaEndpoint> getMediaEndpoint();
                string getAdapterPath() const;
                MediaStreamingState getMediaStreamingState();
            private:
                explicit BlueZDeviceManager(const shared_ptr<BluetoothEventBus>& eventBus);
                bool init();
                bool initializeMedia();
                bool finalizeMedia();
                shared_ptr<BlueZBluetoothDevice> addDeviceFromDBusObject(const char* objectPath, GVariant* dbusObject);
                static void interfacesAddedCallback(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path, const gchar* interface_name,
                                                    const gchar* signal_name, GVariant* parameters, gpointer data);
                static void interfacesRemovedCallback(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path, const gchar* interface_name,
                                                      const gchar* signal_name, GVariant* parameters, gpointer data);
                static void propertiesChangedCallback(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path, const gchar* interface_name,
                                                      const gchar* signal_name, GVariant* parameters, gpointer data);
                void addDevice(const char* devicePath, std::shared_ptr<BlueZBluetoothDevice> device);
                void removeDevice(const char* devicePath);
                void notifyDeviceAdded(shared_ptr<BlueZBluetoothDevice> device);
                void onInterfaceAdded(const char* objectPath, ManagedGVariant& interfacesChangedMap);
                void onInterfaceRemoved(const char* objectPath);
                void onAdapterPropertyChanged(const string& path, const GVariantMapReader& changesMap);
                void onDevicePropertyChanged(const string& path, const GVariantMapReader& changesMap);
                void onMediaStreamPropertyChanged(const string& path, const GVariantMapReader& changesMap);
                shared_ptr<BlueZHostController> initializeHostController();
                void onPropertiesChanged(const string& propertyOwner, const string& objectPath, const GVariantMapReader& changesMap);
                bool getStateFromBlueZ();
                shared_ptr<BlueZBluetoothDevice> getDeviceByPath(const string& path) const;
                void mainLoopThread();
                string m_adapterPath;
                shared_ptr<DBusProxy> m_objectManagerProxy;
                shared_ptr<DBusProxy> m_mediaProxy;
                map<string, shared_ptr<BlueZBluetoothDevice>> m_devices;
                shared_ptr<MediaEndpoint> m_mediaEndpoint;
                shared_ptr<PairingAgent> m_pairingAgent;
                shared_ptr<MPRISPlayer> m_mediaPlayer;
                shared_ptr<BluetoothEventBus> m_eventBus;
                GMainLoop* m_eventLoop;
                GMainContext* m_workerContext;
                shared_ptr<DBusConnection> m_connection;
                MediaStreamingState m_streamingState;
                mutable mutex m_devicesMutex;
                shared_ptr<BlueZHostController> m_hostController;
                promise<bool> m_mainLoopInitPromise;
                thread m_eventThread;
            };
        }
    }
}
#endif