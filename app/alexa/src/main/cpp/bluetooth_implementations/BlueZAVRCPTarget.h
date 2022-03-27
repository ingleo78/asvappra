#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZAVRCPTARGET_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZAVRCPTARGET_H_

#include <memory>
#include <mutex>
#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include <util/bluetooth/SDPRecords.h>
#include "BlueZBluetoothDevice.h"
#include "BlueZUtils.h"

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
            class BlueZAVRCPTarget : public AVRCPTargetInterface {
            public:
                static shared_ptr<BlueZAVRCPTarget> create(shared_ptr<DBusProxy> mediaControlProxy);
                shared_ptr<SDPRecordInterface> getRecord() override;
                void setup() override;
                void cleanup() override;
                bool play() override;
                bool pause() override;
                bool next() override;
                bool previous() override;
            private:
                BlueZAVRCPTarget(shared_ptr<DBusProxy> mediaControlProxy);
                shared_ptr<AVRCPTargetRecord> m_record;
                mutex m_cmdMutex;
                shared_ptr<DBusProxy> m_mediaControlProxy;
            };
        }
    }
}
#endif