#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHDEVICE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHDEVICE_H_

#include <unordered_map>
#include <vector>
#include <gmock/gmock.h>
#include "BluetoothDeviceInterface.h"
#include "Services/BluetoothServiceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace test {
                    using namespace std;
                    using namespace sdkInterfaces::bluetooth::services;
                    using namespace services;
                    using namespace testing;
                    using namespace utils;
                    using namespace utils::bluetooth;
                    class MockBluetoothDevice : public BluetoothDeviceInterface {
                    public:
                        string getMac() const override;
                        string getFriendlyName() const override;
                        DeviceState getDeviceState() override;
                        MockBluetoothDevice::MetaData getDeviceMetaData() override;
                        bool isPaired() override;
                        future<bool> pair() override;
                        future<bool> unpair() override;
                        bool isConnected() override;
                        future<bool> connect() override;
                        future<bool> disconnect() override;
                        vector<shared_ptr<SDPRecordInterface>> getSupportedServices() override;
                        shared_ptr<BluetoothServiceInterface> getService(string uuid) override;
                        MediaStreamingState getStreamingState() override;
                        bool toggleServiceConnection(bool enabled, shared_ptr<BluetoothServiceInterface> service) override;
                        MockBluetoothDevice(const string& mac, const string friendlyName, MetaData metaData,
                                            vector<shared_ptr<BluetoothServiceInterface>> supportedServices);
                    protected:
                        bool m_isPaired;
                        bool m_isConnected;
                        const string m_mac;
                        const string m_friendlyName;
                        MockBluetoothDevice::MetaData m_metaData;
                        unordered_map<string, shared_ptr<BluetoothServiceInterface>> m_supportedServices;
                        DeviceState m_deviceState;
                    };
                    inline string MockBluetoothDevice::getMac() const {
                        return m_mac;
                    }
                    inline string MockBluetoothDevice::getFriendlyName() const {
                        return m_friendlyName;
                    }
                    inline DeviceState MockBluetoothDevice::getDeviceState() {
                        return m_deviceState;
                    }
                    inline MockBluetoothDevice::MetaData MockBluetoothDevice::getDeviceMetaData() {
                        return m_metaData;
                    }
                    inline bool MockBluetoothDevice::isPaired() {
                        return m_isPaired;
                    }
                    inline future<bool> MockBluetoothDevice::pair() {
                        promise<bool> pairPromise;
                        pairPromise.set_value(true);
                        m_isPaired = true;
                        m_deviceState = DeviceState::PAIRED;
                        return pairPromise.get_future();
                    }
                    inline future<bool> MockBluetoothDevice::unpair() {
                        promise<bool> pairPromise;
                        pairPromise.set_value(true);
                        m_isPaired = false;
                        m_deviceState = DeviceState::UNPAIRED;
                        return pairPromise.get_future();
                    }
                    inline bool MockBluetoothDevice::isConnected() {
                        return m_isConnected;
                    }
                    inline future<bool> MockBluetoothDevice::connect() {
                        promise<bool> connectionPromise;
                        connectionPromise.set_value(true);
                        m_isConnected = true;
                        m_deviceState = DeviceState::CONNECTED;
                        return connectionPromise.get_future();
                    }
                    inline future<bool> MockBluetoothDevice::disconnect() {
                        promise<bool> connectionPromise;
                        connectionPromise.set_value(true);
                        m_isConnected = false;
                        m_deviceState = DeviceState::DISCONNECTED;
                        return connectionPromise.get_future();
                    }
                    inline vector<shared_ptr<SDPRecordInterface>> MockBluetoothDevice::getSupportedServices() {
                        vector<shared_ptr<SDPRecordInterface>> services;
                        for (auto service : m_supportedServices) {
                            services.push_back(service.second->getRecord());
                        }
                        return services;
                    }
                    inline shared_ptr<BluetoothServiceInterface> MockBluetoothDevice::getService(string uuid) {
                        auto serviceIt = m_supportedServices.find(uuid);
                        if (serviceIt != m_supportedServices.end()) {
                            return serviceIt->second;
                        }
                        return nullptr;
                    }
                    inline MediaStreamingState MockBluetoothDevice::getStreamingState() {
                        return MediaStreamingState::IDLE;
                    }
                    inline bool MockBluetoothDevice::toggleServiceConnection(
                        bool enabled,
                        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::bluetooth::services::BluetoothServiceInterface> service) {
                        return false;
                    }
                    MockBluetoothDevice::MockBluetoothDevice(const string& mac, const string friendlyName, BluetoothDeviceInterface::MetaData metaData,
                                                             vector<shared_ptr<BluetoothServiceInterface>> supportedServices) : m_mac{mac},
                                                             m_friendlyName{friendlyName}, m_metaData{metaData} {
                        m_isPaired = false;
                        m_isConnected = false;
                        m_deviceState = DeviceState::FOUND;
                        for (unsigned int i = 0; i < supportedServices.size(); ++i) {
                            auto service = supportedServices[i];
                            m_supportedServices.insert({service->getRecord()->getUuid(), service});
                        }
                    }
                }
            }
        }
    }
}
#endif