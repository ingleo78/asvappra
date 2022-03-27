#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHHOSTCONTROLLER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHHOSTCONTROLLER_H_

#include <gmock/gmock.h>
#include "BluetoothHostControllerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace test {
                    static const std::string MOCK_MAC_ADDRESS = "XX:XX:XX:XX";
                    static const std::string MOCK_FRIENDLY_NAME = "MockBluetoothHostControllerName";
                    class MockBluetoothHostController : public BluetoothHostControllerInterface {
                    public:
                        std::string getMac() const override;
                        std::string getFriendlyName() const override;
                        bool isDiscoverable() const override;
                        bool isScanning() const override;
                        std::future<bool> startScan() override;
                        std::future<bool> stopScan() override;
                        std::future<bool> enterDiscoverableMode() override;
                        std::future<bool> exitDiscoverableMode() override;
                    protected:
                        bool m_isDiscoverable;
                        bool m_isScanning;
                    };
                    inline std::future<bool> MockBluetoothHostController::startScan() {
                        std::promise<bool> scanPromise;
                        scanPromise.set_value(true);
                        m_isScanning = true;
                        return scanPromise.get_future();
                    }
                    inline std::future<bool> MockBluetoothHostController::stopScan() {
                        std::promise<bool> scanPromise;
                        scanPromise.set_value(true);
                        m_isScanning = false;
                        return scanPromise.get_future();
                    }
                    std::future<bool> MockBluetoothHostController::enterDiscoverableMode() {
                        std::promise<bool> discoverPromise;
                        discoverPromise.set_value(true);
                        m_isDiscoverable = true;
                        return discoverPromise.get_future();
                    }
                    std::future<bool> MockBluetoothHostController::exitDiscoverableMode() {
                        std::promise<bool> discoverPromise;
                        discoverPromise.set_value(true);
                        m_isDiscoverable = false;
                        return discoverPromise.get_future();
                    }
                    bool MockBluetoothHostController::isScanning() const {
                        return m_isScanning;
                    }
                    bool MockBluetoothHostController::isDiscoverable() const {
                        return m_isDiscoverable;
                    }
                    std::string MockBluetoothHostController::getMac() const {
                        return MOCK_MAC_ADDRESS;
                    }
                    std::string MockBluetoothHostController::getFriendlyName() const {
                        return MOCK_FRIENDLY_NAME;
                    }
                }
            }
        }
    }
}
#endif