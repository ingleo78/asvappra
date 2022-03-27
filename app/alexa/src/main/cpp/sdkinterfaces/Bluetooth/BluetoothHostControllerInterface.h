#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHHOSTCONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHHOSTCONTROLLERINTERFACE_H_

#include <future>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                class BluetoothHostControllerInterface {
                public:
                    virtual ~BluetoothHostControllerInterface() = default;
                    virtual std::string getMac() const = 0;
                    virtual std::string getFriendlyName() const = 0;
                    virtual bool isDiscoverable() const = 0;
                    virtual std::future<bool> enterDiscoverableMode() = 0;
                    virtual std::future<bool> exitDiscoverableMode() = 0;
                    virtual bool isScanning() const = 0;
                    virtual std::future<bool> startScan() = 0;
                    virtual std::future<bool> stopScan() = 0;
                };
            }
        }
    }
}
#endif