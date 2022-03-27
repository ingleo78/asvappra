#include <logger/Logger.h>
#include "BlueZSPP.h"
#include "BlueZDeviceManager.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            using namespace sdkInterfaces;
            using namespace utils::bluetooth;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            static const string TAG{"BlueZSPP"};
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<BlueZSPP> BlueZSPP::create() {
                return shared_ptr<BlueZSPP>(new BlueZSPP());
            }
            void BlueZSPP::setup() {}
            void BlueZSPP::cleanup() {}
            shared_ptr<SDPRecordInterface> BlueZSPP::getRecord() {
                return m_record;
            }
            BlueZSPP::BlueZSPP() : m_record{std::make_shared<SPPRecord>("")} {}
        }
    }
}