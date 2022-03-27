#include <iostream>
#include <string>
#include <logger/Logger.h>
#include "BlueZConstants.h"
#include "BlueZDeviceManager.h"
#include "PairingAgent.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            static const string TAG{"PairingAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string AGENT_OBJECT_PATH = "/ACSDK/Bluetooth/Agent";
            static const string CAPABILITY = "NoInputNoOutput";
            static const string RELEASE = "Release";
            static const string REQUEST_PIN_CODE = "RequestPinCode";
            static const string DISPLAY_PIN_CODE = "DisplayPinCode";
            static const string REQUEST_PASSKEY = "RequestPasskey";
            static const string DISPLAY_PASSKEY = "DisplayPasskey";
            static const string REQUEST_CONFIRMATION = "RequestConfirmation";
            static const string REQUEST_AUTHORIZATION = "RequestAuthorization";
            static const string AUTHORIZE_SERVICE = "AuthorizeService";
            static const string CANCEL = "Cancel";
            static const string BLUEZ_OBJECT_PATH = "/org/bluez";
            const uint32_t DEFAULT_PASSKEY = 0;
            const char* DEFAULT_PIN_CODE = "0000";
            const char INTROSPECT_XML[] = R"(<!DOCTYPE node PUBLIC -//freedesktop//DTD D-BUS Object Introspection 1.0//EN
                                          "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd";><node><interface name="org.bluez.Agent1">
                                          <method name="Release"></method><method name="RequestPinCode"><arg type="s" direction="out"/>
                                          <arg name="device" type="o" direction="in"/></method><method name="DisplayPinCode"><arg type="s" direction="out"/>
                                          <arg name="device" type="o" direction="in"/><arg name="pincode" type="s" direction="in"/></method>
                                          <method name="RequestPasskey"><arg type="u" direction="out"/><arg name="device" type="o" direction="in"/></method>
                                          <method name="DisplayPasskey"><arg name="device" type="o" direction="in"/><arg name="passkey" type="u" direction="in"/>
                                          <arg name="entered" type="q" direction="in"/></method><method name="RequestConfirmation">
                                          <arg name="device" type="o" direction="in"/><arg name="passkey" type="u" direction="in"/></method>
                                          <method name="RequestAuthorization"><arg name="device" type="o" direction="in"/></method>
                                          <method name="AuthorizeService"><arg name="device" type="o" direction="in"/><arg name="uuid" type="s" direction="in"/>
                                          </method><method name="Cancel"></method></interface></node>)";
            unique_ptr<PairingAgent> PairingAgent::create(shared_ptr<DBusConnection> connection) {
                if (!connection) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullConnection"));
                    return nullptr;
                }
                auto pairingAgent = unique_ptr<PairingAgent>(new PairingAgent(connection));
                if (!pairingAgent->init()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "initFailed"));
                    return nullptr;
                }
                return pairingAgent;
            }
            PairingAgent::PairingAgent(shared_ptr<DBusConnection> connection) : DBusObject(connection, INTROSPECT_XML,AGENT_OBJECT_PATH,{
                                                                                    {RELEASE, &PairingAgent::release},
                                                                                    {REQUEST_PIN_CODE, &PairingAgent::requestPinCode},
                                                                                    {DISPLAY_PIN_CODE, &PairingAgent::displayPinCode},
                                                                                    {REQUEST_PASSKEY, &PairingAgent::requestPasskey},
                                                                                    {DISPLAY_PASSKEY, &PairingAgent::displayPasskey},
                                                                                    {REQUEST_CONFIRMATION, &PairingAgent::requestConfirmation},
                                                                                    {REQUEST_AUTHORIZATION, &PairingAgent::requestAuthorization},
                                                                                    {AUTHORIZE_SERVICE, &PairingAgent::authorizeService},
                                                                                    {CANCEL, &PairingAgent::cancel},
                                                                                }) {
            }
            bool PairingAgent::init() {
                if (!registerWithDBus()) return false;
                m_agentManager = DBusProxy::create(BlueZConstants::BLUEZ_AGENTMANAGER_INTERFACE, BLUEZ_OBJECT_PATH);
                if (!m_agentManager) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullAgentManager"));
                    return false;
                }
                return registerAgent() && requestDefaultAgent();
            }
            PairingAgent::~PairingAgent() {
                ACSDK_DEBUG5(LX(__func__));
                unregisterAgent();
            }
            void PairingAgent::release(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void PairingAgent::requestPinCode(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                ACSDK_INFO(LX(__func__).d("pinCode", DEFAULT_PIN_CODE));
                auto parameters = g_variant_new("(s)", DEFAULT_PIN_CODE);
                g_dbus_method_invocation_return_value(invocation, parameters);
            }
            void PairingAgent::displayPinCode(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void PairingAgent::requestPasskey(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                ACSDK_INFO(LX(__func__).d("passKey", DEFAULT_PASSKEY));
                auto parameters = g_variant_new("(u)", DEFAULT_PASSKEY);
                g_dbus_method_invocation_return_value(invocation, parameters);
            }
            void PairingAgent::displayPasskey(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void PairingAgent::requestConfirmation(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void PairingAgent::requestAuthorization(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void PairingAgent::authorizeService(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void PairingAgent::cancel(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            bool PairingAgent::requestDefaultAgent() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGError error;
                auto parameters = g_variant_new("(o)", AGENT_OBJECT_PATH.c_str());
                m_agentManager->callMethod("RequestDefaultAgent", parameters, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "requestDefaultAgentFailed").d("error", error.getMessage()));
                    return false;
                }
                ACSDK_DEBUG5(LX(__func__).m("requestDefaultAgentSuccessful"));
                return true;
            }
            bool PairingAgent::registerAgent() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGError error;
                auto parameters = g_variant_new("(os)", AGENT_OBJECT_PATH.c_str(), CAPABILITY.c_str());
                m_agentManager->callMethod("RegisterAgent", parameters, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "registerAgentFailed").d("error", error.getMessage()));
                    return false;
                }
                ACSDK_DEBUG5(LX(__func__).m("registerAgentDone"));
                return true;
            }
            bool PairingAgent::unregisterAgent() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGError error;
                auto parameters = g_variant_new("(o)", AGENT_OBJECT_PATH.c_str());
                m_agentManager->callMethod("UnregisterAgent", parameters, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "unregisterAgentFailed").d("error", error.getMessage()));
                    return false;
                }
                ACSDK_DEBUG5(LX(__func__).m("unregisterAgentDone"));
                return true;
            }
        }
    }
}