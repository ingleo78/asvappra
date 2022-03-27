#include <logger/Logger.h>
#include "BlueZAVRCPController.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace utils::bluetooth;
            using namespace logger;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            static const string TAG{"BlueZAVRCPController"};
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<BlueZAVRCPController> BlueZAVRCPController::create() {
                ACSDK_DEBUG5(LX(__func__));
                return shared_ptr<BlueZAVRCPController>(new BlueZAVRCPController());
            }
            BlueZAVRCPController::BlueZAVRCPController() : m_record{make_shared<AVRCPControllerRecord>("")} {}
            shared_ptr<SDPRecordInterface> BlueZAVRCPController::getRecord() {
                return m_record;
            }
            void BlueZAVRCPController::setup() {}
            void BlueZAVRCPController::cleanup() {}
        }
    }
}