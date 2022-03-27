#include <logger/Logger.h>
#include "BlueZHFP.h"
#include "BlueZDeviceManager.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            using namespace utils::bluetooth;
            static const string TAG{"BlueZHFP"};
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<BlueZHFP> BlueZHFP::create() {
                return shared_ptr<BlueZHFP>(new BlueZHFP());
            }
            void BlueZHFP::setup() {}
            void BlueZHFP::cleanup() {}
            shared_ptr<SDPRecordInterface> BlueZHFP::getRecord() {
                return m_record;
            }
            BlueZHFP::BlueZHFP() : m_record{make_shared<HFPRecord>("")} {}
        }
    }
}