#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZSPP_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZSPP_H_

#include <memory>
#include <sdkinterfaces/Bluetooth/Services/SPPInterface.h>
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
            class BlueZSPP : public SPPInterface {
            public:
                static shared_ptr<BlueZSPP> create();
                shared_ptr<SDPRecordInterface> getRecord() override;
                void setup() override;
                void cleanup() override;
            private:
                BlueZSPP();
                shared_ptr<SPPRecord> m_record;
            };
        }
    }
}
#endif