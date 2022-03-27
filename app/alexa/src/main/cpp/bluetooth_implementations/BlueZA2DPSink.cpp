#include <logger/Logger.h>
#include "BlueZA2DPSink.h"
#include "BlueZDeviceManager.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::bluetooth;
            using namespace utils::bluetooth;
            using namespace services;
            static const string TAG{"BlueZA2DPSink"};
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<BlueZA2DPSink> BlueZA2DPSink::create() {
                return shared_ptr<BlueZA2DPSink>(new BlueZA2DPSink());
            }
            void BlueZA2DPSink::setup() {}
            void BlueZA2DPSink::cleanup() {}
            shared_ptr<SDPRecordInterface> BlueZA2DPSink::getRecord() {
                return m_record;
            }
            BlueZA2DPSink::BlueZA2DPSink() : m_record{make_shared<A2DPSinkRecord>("")} {}
        }
    }
}