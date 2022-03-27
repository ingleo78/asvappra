#include <string>
#include <logger/Logger.h>
#include <util/bluetooth/BluetoothEvents.h>
#include "BlueZConstants.h"
#include "BlueZDeviceManager.h"
#include "MPRISPlayer.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            using namespace sdkInterfaces;
            using namespace utils::bluetooth;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            static const string TAG{"MPRISPlayer"};
            #define LX(event) LogEntry(TAG, event)
            static const string CAN_GO_NEXT = "CanGoNext";
            static const string CAN_GO_PREVIOUS = "CanGoPrevious";
            static const string CAN_PLAY = "CanPlay";
            static const string CAN_PAUSE = "CanPause";
            static const string CAN_SEEK = "CanSeek";
            static const string CAN_CONTROL = "CanControl";
            static const string PLAY = "Play";
            static const string PAUSE = "Pause";
            static const string NEXT = "Next";
            static const string PREVIOUS = "Previous";
            static const string PLAY_PAUSE = "PlayPause";
            static const string STOP = "Stop";
            static const string SEEK = "Seek";
            static const string SET_POSITION = "SetPosition";
            static const string OPEN_URI = "OpenUri";
            static const string REGISTER_PLAYER = "RegisterPlayer";
            static const string UNREGISTER_PLAYER = "UnregisterPlayer";
            const char INTROSPECT_XML[] = R"(<!DOCTYPE node PUBLIC -//freedesktop//DTD D-BUS Object Introspection 1.0//EN
                                          "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd";><node>
                                          <interface name="org.mpris.MediaPlayer2.Player"><method name="Next"/><method name="Previous"/>
                                          <method name="Pause"/><method name="PlayPause"/><method name="Stop"/><method name="Play"/><method name="Seek">
                                          <arg type="x" direction="in"/></method><method name="SetPosition"><arg type="o" direction="in"/>
                                          <arg type="x" direction="in"/></method><method name="OpenUri"><arg type="s" direction="in"/></method></interface>
                                          </node>)";
            const string MPRISPlayer::MPRIS_OBJECT_PATH = "/org/mpris/MediaPlayer2";
            unique_ptr<MPRISPlayer> MPRISPlayer::create(shared_ptr<DBusConnection> connection, shared_ptr<DBusProxy> media, shared_ptr<BluetoothEventBus> eventBus,
                                                        const string& playerPath) {
                if (!connection) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullConnection"));
                    return nullptr;
                } else if (!media) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullMediaManager"));
                    return nullptr;
                } else if (!eventBus) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullEventBus"));
                    return nullptr;
                }
                auto mediaPlayer = std::unique_ptr<MPRISPlayer>(new MPRISPlayer(connection, media, eventBus, playerPath));
                if (!mediaPlayer->init()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "initFailed"));
                    return nullptr;
                }
                return mediaPlayer;
            }
            MPRISPlayer::MPRISPlayer(shared_ptr<DBusConnection> connection, shared_ptr<DBusProxy> media, shared_ptr<BluetoothEventBus> eventBus,
                                     const string& playerPath) : DBusObject(connection, INTROSPECT_XML, playerPath,{
                                         {NEXT, &MPRISPlayer::toMediaCommand},
                                         {PREVIOUS, &MPRISPlayer::toMediaCommand},
                                         {PAUSE, &MPRISPlayer::toMediaCommand},
                                         {PLAY, &MPRISPlayer::toMediaCommand},
                                         {PLAY_PAUSE, &MPRISPlayer::unsupportedMethod},
                                         {STOP, &MPRISPlayer::unsupportedMethod},
                                         {SEEK, &MPRISPlayer::unsupportedMethod},
                                         {SET_POSITION, &MPRISPlayer::unsupportedMethod},
                                         {OPEN_URI, &MPRISPlayer::unsupportedMethod},
                                     }), m_playerPath{playerPath}, m_media{media}, m_eventBus{eventBus} {
            }
            bool MPRISPlayer::init() {
                if (!registerWithDBus()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "createDBusObjectFailed"));
                    return false;
                }
                return registerPlayer();
            }
            MPRISPlayer::~MPRISPlayer() {
                ACSDK_DEBUG5(LX(__func__));
                unregisterPlayer();
            }
            void MPRISPlayer::unsupportedMethod(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_WARN(LX(__func__).d("methodName", g_dbus_method_invocation_get_method_name(invocation)));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void MPRISPlayer::toMediaCommand(GVariant* arguments, GDBusMethodInvocation* invocation) {
                const char* method = g_dbus_method_invocation_get_method_name(invocation);
                ACSDK_DEBUG5(LX(__func__).d("method", method));
                if (PLAY == method) sendEvent(MediaCommand::PLAY);
                else if (PAUSE == method) sendEvent(MediaCommand::PAUSE);
                else if (NEXT == method) sendEvent(MediaCommand::NEXT);
                else if (PREVIOUS == method) sendEvent(MediaCommand::PREVIOUS);
                else { ACSDK_ERROR(LX(__func__).d("reason", "unsupported").d("method", method)); }
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            void MPRISPlayer::sendEvent(const MediaCommand& command) {
                ACSDK_DEBUG5(LX(__func__).d("command", command));
                MediaCommandReceivedEvent event(command);
                m_eventBus->sendEvent(event);
            }
            bool MPRISPlayer::registerPlayer() {
                ACSDK_DEBUG5(LX(__func__));
                GVariantBuilder builder;
                g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
                g_variant_builder_add(&builder, "{sv}", CAN_GO_NEXT.c_str(), g_variant_new("b", TRUE));
                g_variant_builder_add(&builder, "{sv}", CAN_GO_PREVIOUS.c_str(), g_variant_new("b", TRUE));
                g_variant_builder_add(&builder, "{sv}", CAN_PLAY.c_str(), g_variant_new("b", TRUE));
                g_variant_builder_add(&builder, "{sv}", CAN_PAUSE.c_str(), g_variant_new("b", TRUE));
                g_variant_builder_add(&builder, "{sv}", CAN_SEEK.c_str(), g_variant_new("b", FALSE));
                g_variant_builder_add(&builder, "{sv}", CAN_CONTROL.c_str(), g_variant_new("b", TRUE));
                GVariant* properties = g_variant_builder_end(&builder);
                auto parameters = g_variant_new("(o@a{sv})", m_playerPath.c_str(), properties);
                ManagedGError error;
                m_media->callMethod(REGISTER_PLAYER, parameters, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "registerPlayerFailed").d("error", error.getMessage()));
                    return false;
                }
                ACSDK_DEBUG(LX("registerPlayerSucceeded").d("path", m_playerPath));
                return true;
            }
            bool MPRISPlayer::unregisterPlayer() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGError error;
                auto parameters = g_variant_new("(o)", m_playerPath.c_str());
                m_media->callMethod(UNREGISTER_PLAYER, parameters, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "unregisterPlayerFailed").d("error", error.getMessage()));
                    return false;
                }
                ACSDK_DEBUG(LX(__func__).m("unregisterPlayerSucceeded"));
                return true;
            }
        }
    }
}