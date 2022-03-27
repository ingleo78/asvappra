#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MPRISPLAYER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MPRISPLAYER_H_

#include <memory>
#include <gio/gio.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include "DBusConnection.h"
#include "DBusObject.h"
#include "DBusProxy.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils::bluetooth;
            using namespace sdkInterfaces::bluetooth::services;
            class MPRISPlayer : public DBusObject<MPRISPlayer> {
            public:
                virtual ~MPRISPlayer();
                static const string MPRIS_OBJECT_PATH;
                static unique_ptr<MPRISPlayer> create(shared_ptr<DBusConnection> connection, shared_ptr<DBusProxy> media, shared_ptr<BluetoothEventBus> eventBus,
                                                      const string& playerPath = MPRIS_OBJECT_PATH);

            private:
                MPRISPlayer(shared_ptr<DBusConnection> connection, shared_ptr<DBusProxy> media, shared_ptr<BluetoothEventBus> eventBus, const string& playerPath);
                bool init();
                bool registerPlayer();
                bool unregisterPlayer();
                void unsupportedMethod(GVariant* arguments, GDBusMethodInvocation* invocation);
                void toMediaCommand(GVariant* arguments, GDBusMethodInvocation* invocation);
                void sendEvent(const MediaCommand& command);
                const string m_playerPath;
                shared_ptr<DBusProxy> m_media;
                shared_ptr<BluetoothEventBus> m_eventBus;
            };
        }
    }
}
#endif