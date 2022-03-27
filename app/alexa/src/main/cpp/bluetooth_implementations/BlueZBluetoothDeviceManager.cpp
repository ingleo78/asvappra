#include <logger/Logger.h>
#include "BlueZBluetoothDeviceManager.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace avsCommon::sdkInterfaces::bluetooth;
            using namespace avsCommon::utils::bluetooth;
            static const std::string TAG{"BlueZBluetoothDeviceManager"};
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            std::shared_ptr<BluetoothHostControllerInterface> BlueZBluetoothDeviceManager::getHostController() {
                return m_deviceManager->getHostController();
            }
            std::list<std::shared_ptr<BluetoothDeviceInterface>> BlueZBluetoothDeviceManager::getDiscoveredDevices() {
                return m_deviceManager->getDiscoveredDevices();
            }
            std::unique_ptr<BlueZBluetoothDeviceManager> BlueZBluetoothDeviceManager::create(
                std::shared_ptr<BluetoothEventBus> eventBus) {
                auto deviceManager = BlueZDeviceManager::create(eventBus);
                if (!deviceManager) return nullptr;
                return std::unique_ptr<BlueZBluetoothDeviceManager>(new BlueZBluetoothDeviceManager(deviceManager));
            }
            BlueZBluetoothDeviceManager::BlueZBluetoothDeviceManager(std::shared_ptr<BlueZDeviceManager> deviceManager) : m_deviceManager{deviceManager} {}
            BlueZBluetoothDeviceManager::~BlueZBluetoothDeviceManager() {
                ACSDK_DEBUG5(LX(__func__));
                m_deviceManager->shutdown();
            };
            std::shared_ptr<BluetoothEventBus> BlueZBluetoothDeviceManager::getEventBus() {
                return m_deviceManager->getEventBus();
            }
        }
    }
}