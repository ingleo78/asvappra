#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZA2DPSOURCE_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZA2DPSOURCE_H_

#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <util/bluetooth/SDPRecords.h>
#include <util/bluetooth/FormattedAudioStreamAdapter.h>

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
            class BlueZDeviceManager;
            class BlueZA2DPSource : public A2DPSourceInterface {
            public:
                static shared_ptr<BlueZA2DPSource> create(shared_ptr<BlueZDeviceManager> deviceManager);
                shared_ptr<FormattedAudioStreamAdapter> getSourceStream() override;
                shared_ptr<SDPRecordInterface> getRecord() override;
                void setup() override;
                void cleanup() override;
            private:
                BlueZA2DPSource(shared_ptr<BlueZDeviceManager> deviceManager);
                shared_ptr<A2DPSourceRecord> m_record;
                shared_ptr<BlueZDeviceManager> m_deviceManager;
            };
        }
    }
}
#endif