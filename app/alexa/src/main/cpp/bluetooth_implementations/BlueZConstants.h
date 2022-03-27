#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZCONSTANTS_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZCONSTANTS_H_

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class BlueZConstants {
            public:
                static constexpr auto BLUEZ_SERVICE_NAME = "org.bluez";
                static constexpr auto BLUEZ_MEDIATRANSPORT_INTERFACE = "org.bluez.MediaTransport1";
                static constexpr auto BLUEZ_DEVICE_INTERFACE = "org.bluez.Device1";
                static constexpr auto BLUEZ_DEVICE_INTERFACE_ADDRESS = "Address";
                static constexpr auto BLUEZ_DEVICE_INTERFACE_ALIAS = "Alias";
                static constexpr auto BLUEZ_ADAPTER_INTERFACE = "org.bluez.Adapter1";
                static constexpr auto BLUEZ_MEDIA_INTERFACE = "org.bluez.Media1";
                static constexpr auto BLUEZ_MEDIA_PLAYER_INTERFACE = "org.bluez.MediaPlayer1";
                static constexpr auto BLUEZ_AGENTMANAGER_INTERFACE = "org.bluez.AgentManager1";
                static constexpr auto OBJECT_MANAGER_INTERFACE = "org.freedesktop.DBus.ObjectManager";
                static constexpr auto PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";
            };
        }
    }
}
#endif