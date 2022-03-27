#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZAVRCPCONTROLLER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZAVRCPCONTROLLER_H_

#include <memory>
#include <sdkinterfaces/Bluetooth/Services/AVRCPControllerInterface.h>
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
            class BlueZAVRCPController : public AVRCPControllerInterface {
            public:
                static shared_ptr<BlueZAVRCPController> create();
                shared_ptr<SDPRecordInterface> getRecord() override;
                void setup() override;
                void cleanup() override;
            private:
                BlueZAVRCPController();
                shared_ptr<AVRCPControllerRecord> m_record;
            };
        }
    }
}
#endif