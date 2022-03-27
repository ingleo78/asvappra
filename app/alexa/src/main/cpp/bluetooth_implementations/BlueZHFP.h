#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZHFP_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZHFP_H_

#include <memory>
#include <sdkinterfaces/Bluetooth/Services/HFPInterface.h>
#include <util/bluetooth/SDPRecords.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            using namespace utils::bluetooth;
            class BlueZHFP : public HFPInterface {
            public:
                static shared_ptr<BlueZHFP> create();
                shared_ptr<SDPRecordInterface> getRecord() override;
                void setup() override;
                void cleanup() override;
            private:
                BlueZHFP();
                shared_ptr<HFPRecord> m_record;
            };
        }
    }
}
#endif