#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZHID_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZHID_H_

#include <memory>
#include <sdkinterfaces/Bluetooth/Services/HIDInterface.h>
#include <util/bluetooth/SDPRecords.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace utils::bluetooth;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            class BlueZHID : public HIDInterface {
            public:
                static shared_ptr<BlueZHID> create();
                shared_ptr<SDPRecordInterface> getRecord() override;
                void setup() override;
                void cleanup() override;
            private:
                BlueZHID();
                shared_ptr<HIDRecord> m_record;
            };
        }
    }
}
#endif