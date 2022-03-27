#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEINTERFACE_H_

#include <future>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <util/bluetooth/MediaStreamingState.h>
#include <util/Optional.h>
#include "Services/BluetoothServiceInterface.h"
#include "Services/SDPRecordInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                enum class DeviceState {
                    FOUND,
                    UNPAIRED,
                    PAIRED,
                    IDLE,
                    DISCONNECTED,
                    CONNECTED
                };
                inline std::string_view deviceStateToString(DeviceState state) {
                    switch (state) {
                        case DeviceState::FOUND: return "FOUND";
                        case DeviceState::UNPAIRED: return "UNPAIRED";
                        case DeviceState::PAIRED: return "PAIRED";
                        case DeviceState::IDLE: return "IDLE";
                        case DeviceState::DISCONNECTED: return "DISCONNECTED";
                        case DeviceState::CONNECTED: return "CONNECTED";
                    }
                    return "UNKNOWN";
                }
                inline std::ostream& operator<<(std::ostream& stream, const DeviceState state) {
                    return stream << deviceStateToString(state);
                }
                class BluetoothDeviceInterface {
                public:
                    struct MetaData {
                        static const int UNDEFINED_CLASS_VALUE = 0;
                        utils::Optional<int> vendorId;
                        utils::Optional<int> productId;
                        int classOfDevice;
                        utils::Optional<int> vendorDeviceSigId;
                        utils::Optional<std::string> vendorDeviceId;
                        MetaData(utils::Optional<int> vendorId, utils::Optional<int> productId, int classOfDevice, utils::Optional<int> vendorDeviceSigId,
                                 utils::Optional<std::string> vendorDeviceId) : vendorId(vendorId), productId(productId), classOfDevice(classOfDevice),
                                 vendorDeviceSigId(vendorDeviceSigId), vendorDeviceId(vendorDeviceId) {}
                    };
                    virtual ~BluetoothDeviceInterface() = default;
                    virtual std::string getMac() const = 0;
                    virtual std::string getFriendlyName() const = 0;
                    virtual DeviceState getDeviceState() = 0;
                    virtual MetaData getDeviceMetaData() = 0;
                    virtual bool isPaired() = 0;
                    virtual std::future<bool> pair() = 0;
                    virtual std::future<bool> unpair() = 0;
                    virtual bool isConnected() = 0;
                    virtual std::future<bool> connect() = 0;
                    virtual std::future<bool> disconnect() = 0;
                    virtual std::vector<std::shared_ptr<services::SDPRecordInterface>> getSupportedServices() = 0;
                    virtual std::shared_ptr<services::BluetoothServiceInterface> getService(std::string uuid) = 0;
                    virtual utils::bluetooth::MediaStreamingState getStreamingState() = 0;
                    virtual bool toggleServiceConnection(bool enabled, std::shared_ptr<services::BluetoothServiceInterface> service) = 0;
                };
            }
        }
    }
}
#endif