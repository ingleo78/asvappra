#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTS_H_

#include <memory>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include "A2DPRole.h"
#include "MediaStreamingState.h"

typedef unsigned int size_t;

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                using namespace std;
                using namespace sdkInterfaces;
                using namespace sdkInterfaces::bluetooth;
                using namespace services;
                enum class BluetoothEventType {
                    DEVICE_DISCOVERED,
                    DEVICE_REMOVED,
                    DEVICE_STATE_CHANGED,
                    STREAMING_STATE_CHANGED,
                    MEDIA_COMMAND_RECEIVED,
                    BLUETOOTH_DEVICE_MANAGER_INITIALIZED,
                    SCANNING_STATE_CHANGED,
                    TOGGLE_A2DP_PROFILE_STATE_CHANGED
                };
                struct BluetoothEventTypeHash {
                    template <typename T> size_t operator()(T t) const {
                        return static_cast<size_t>(t);
                    }
                };
                class BluetoothEvent {
                public:
                    virtual ~BluetoothEvent() = default;
                    BluetoothEventType getType() const;
                    std::shared_ptr<BluetoothDeviceInterface> getDevice() const;
                    DeviceState getDeviceState() const;
                    MediaStreamingState getMediaStreamingState() const;
                    shared_ptr<A2DPRole> getA2DPRole() const;
                    shared_ptr<MediaCommand> getMediaCommand() const;
                    bool isScanning() const;
                    bool isA2DPEnabled() const;
                protected:
                    BluetoothEvent(BluetoothEventType type, shared_ptr<BluetoothDeviceInterface> device = nullptr, DeviceState deviceState = DeviceState::IDLE,
                                   MediaStreamingState mediaStreamingState = MediaStreamingState::IDLE, shared_ptr<A2DPRole> role = nullptr,
                                   shared_ptr<MediaCommand> mediaCommand = nullptr, bool scanningState = false, bool a2dpEnable = false);
                private:
                    BluetoothEventType m_type;
                    shared_ptr<BluetoothDeviceInterface> m_device;
                    DeviceState m_deviceState;
                    MediaStreamingState m_mediaStreamingState;
                    shared_ptr<A2DPRole> m_a2dpRole;
                    shared_ptr<MediaCommand> m_mediaCommand;
                    bool m_scanningState;
                    bool m_a2dpEnable;
                };
                inline BluetoothEvent::BluetoothEvent(BluetoothEventType type, shared_ptr<BluetoothDeviceInterface> device, DeviceState deviceState,
                                                      MediaStreamingState mediaStreamingState, shared_ptr<A2DPRole> a2dpRole, shared_ptr<MediaCommand> mediaCommand,
                                                      bool scanningState, bool a2dpEnable) : m_type{type}, m_device{device}, m_deviceState{deviceState},
                                                      m_mediaStreamingState{mediaStreamingState}, m_a2dpRole{a2dpRole}, m_mediaCommand{mediaCommand},
                                                      m_scanningState{scanningState}, m_a2dpEnable{a2dpEnable} {}
                inline BluetoothEventType BluetoothEvent::getType() const {
                    return m_type;
                }
                inline shared_ptr<BluetoothDeviceInterface> BluetoothEvent::getDevice() const {
                    return m_device;
                }
                inline DeviceState BluetoothEvent::getDeviceState() const {
                    return m_deviceState;
                }
                inline MediaStreamingState BluetoothEvent::getMediaStreamingState() const {
                    return m_mediaStreamingState;
                }
                inline shared_ptr<A2DPRole> BluetoothEvent::getA2DPRole() const {
                    return m_a2dpRole;
                }
                inline shared_ptr<MediaCommand> BluetoothEvent::getMediaCommand() const {
                    return m_mediaCommand;
                }
                inline bool BluetoothEvent::isScanning() const {
                    return m_scanningState;
                }
                inline bool BluetoothEvent::isA2DPEnabled() const {
                    return m_a2dpEnable;
                }
                class DeviceDiscoveredEvent : public BluetoothEvent {
                public:
                    explicit DeviceDiscoveredEvent(const shared_ptr<BluetoothDeviceInterface>& device);
                };
                inline DeviceDiscoveredEvent::DeviceDiscoveredEvent(const shared_ptr<BluetoothDeviceInterface>& device) :
                                                                    BluetoothEvent(BluetoothEventType::DEVICE_DISCOVERED, device,DeviceState::IDLE,
                                                                                   MediaStreamingState::IDLE) {}
                class DeviceRemovedEvent : public BluetoothEvent {
                public:
                    explicit DeviceRemovedEvent(const shared_ptr<BluetoothDeviceInterface>& device);
                };
                inline DeviceRemovedEvent::DeviceRemovedEvent(const shared_ptr<BluetoothDeviceInterface>& device) :
                                                              BluetoothEvent(BluetoothEventType::DEVICE_REMOVED, device,DeviceState::IDLE,
                                                                             MediaStreamingState::IDLE) {}
                class DeviceStateChangedEvent : public BluetoothEvent {
                public:
                    DeviceStateChangedEvent(shared_ptr<BluetoothDeviceInterface> device, DeviceState newState);
                };
                inline DeviceStateChangedEvent::DeviceStateChangedEvent(shared_ptr<BluetoothDeviceInterface> device, DeviceState newState) :
                                                                        BluetoothEvent(BluetoothEventType::DEVICE_STATE_CHANGED, device, newState,
                                                                                       MediaStreamingState::IDLE) {}
                class MediaStreamingStateChangedEvent : public BluetoothEvent {
                public:
                    explicit MediaStreamingStateChangedEvent(MediaStreamingState newState, A2DPRole role, std::shared_ptr<BluetoothDeviceInterface> device);
                };
                inline MediaStreamingStateChangedEvent::MediaStreamingStateChangedEvent(MediaStreamingState newState, A2DPRole role,
                                                                                        std::shared_ptr<BluetoothDeviceInterface> device) :
                                                                                        BluetoothEvent(BluetoothEventType::STREAMING_STATE_CHANGED,
                                                                                        device,DeviceState::IDLE, newState,
                                                                                        shared_ptr<A2DPRole>(&role)){};
                class MediaCommandReceivedEvent : public BluetoothEvent {
                public:
                    explicit MediaCommandReceivedEvent(MediaCommand command);
                };
                inline MediaCommandReceivedEvent::MediaCommandReceivedEvent(MediaCommand command) : BluetoothEvent(BluetoothEventType::MEDIA_COMMAND_RECEIVED,
                                                                           nullptr,DeviceState::IDLE,MediaStreamingState::IDLE,
                                                                           nullptr, shared_ptr<MediaCommand>(&command)) {}
                class BluetoothDeviceManagerInitializedEvent : public BluetoothEvent {
                public:
                    explicit BluetoothDeviceManagerInitializedEvent();
                };
                inline BluetoothDeviceManagerInitializedEvent::BluetoothDeviceManagerInitializedEvent() :
                       BluetoothEvent(BluetoothEventType::BLUETOOTH_DEVICE_MANAGER_INITIALIZED) {}
                class ScanningStateChangedEvent : public BluetoothEvent {
                public:
                    explicit ScanningStateChangedEvent(bool isScanning);
                };
                inline ScanningStateChangedEvent::ScanningStateChangedEvent(bool isScanning) : BluetoothEvent(BluetoothEventType::SCANNING_STATE_CHANGED,
                                                                                                              nullptr,DeviceState::IDLE,
                                                                                                              MediaStreamingState::IDLE,
                                                                                                              nullptr,nullptr, isScanning) {}
                class ToggleA2DPProfileStateChangedEvent : public BluetoothEvent {
                public:
                    explicit ToggleA2DPProfileStateChangedEvent(bool a2dpEnable);
                };
                inline ToggleA2DPProfileStateChangedEvent::ToggleA2DPProfileStateChangedEvent(bool a2dpEnable) :
                       BluetoothEvent(BluetoothEventType::TOGGLE_A2DP_PROFILE_STATE_CHANGED, nullptr,DeviceState::IDLE,
                                     MediaStreamingState::IDLE,nullptr,nullptr,false, a2dpEnable) {}
            }
        }
    }
}
#endif