#include <logger/Logger.h>
#include "BlueZHID.h"
#include "BlueZDeviceManager.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            using namespace utils::bluetooth;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            static const string TAG{"BlueZHID"};
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<BlueZHID> BlueZHID::create() {
                return shared_ptr<BlueZHID>(new BlueZHID());
            }
            void BlueZHID::setup() {}
            void BlueZHID::cleanup() {}
            shared_ptr<SDPRecordInterface> BlueZHID::getRecord() {
                return m_record;
            }
            BlueZHID::BlueZHID() : m_record{make_shared<HIDRecord>("")} {}
        }
    }
}