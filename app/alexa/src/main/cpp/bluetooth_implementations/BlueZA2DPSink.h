#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZA2DPSINK_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZA2DPSINK_H_

#include <memory>
#include <sdkinterfaces/Bluetooth/Services/A2DPSinkInterface.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include <util/bluetooth/SDPRecords.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            using namespace utils;
            using namespace utils::bluetooth;
            class BlueZA2DPSink : public A2DPSinkInterface {
            public:
                static shared_ptr<BlueZA2DPSink> create();
                shared_ptr<SDPRecordInterface> getRecord() override;
                void setup() override;
                void cleanup() override;
            private:
                BlueZA2DPSink();
                shared_ptr<A2DPSinkRecord> m_record;
            };
        }
    }
}
#endif