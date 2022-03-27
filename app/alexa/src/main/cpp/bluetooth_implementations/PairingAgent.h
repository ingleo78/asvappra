#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_PAIRINGAGENT_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_PAIRINGAGENT_H_

#include <memory>
#include <gio/gio.h>
#include "DBusObject.h"
#include "DBusProxy.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class PairingAgent : public DBusObject<PairingAgent> {
            public:
                virtual ~PairingAgent();
                static std::unique_ptr<PairingAgent> create(std::shared_ptr<DBusConnection> connection);
            private:
                PairingAgent(std::shared_ptr<DBusConnection> connection);
                bool init();
                bool registerAgent();
                bool unregisterAgent();
                bool requestDefaultAgent();
                void release(GVariant* arguments, GDBusMethodInvocation* invocation);
                void requestPinCode(GVariant* arguments, GDBusMethodInvocation* invocation);
                void displayPinCode(GVariant* arguments, GDBusMethodInvocation* invocation);
                void requestPasskey(GVariant* arguments, GDBusMethodInvocation* invocation);
                void displayPasskey(GVariant* arguments, GDBusMethodInvocation* invocation);
                void requestConfirmation(GVariant* arguments, GDBusMethodInvocation* invocation);
                void requestAuthorization(GVariant* arguments, GDBusMethodInvocation* invocation);
                void authorizeService(GVariant* arguments, GDBusMethodInvocation* invocation);
                void cancel(GVariant* arguments, GDBusMethodInvocation* invocation);
                std::shared_ptr<DBusProxy> m_agentManager;
            };
        }
    }
}
#endif