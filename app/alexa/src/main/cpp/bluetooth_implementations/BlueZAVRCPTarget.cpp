#include <logger/Logger.h>
#include "BlueZAVRCPTarget.h"

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
            static const string TAG{"BlueZAVRCPTarget"};
            #define LX(event) LogEntry(TAG, event)
            static const string PLAY_CMD = "Play";
            static const string PAUSE_CMD = "Pause";
            static const string NEXT_CMD = "Next";
            static const string PREVIOUS_CMD = "Previous";
            shared_ptr<BlueZAVRCPTarget> BlueZAVRCPTarget::create(shared_ptr<DBusProxy> mediaControlProxy) {
                ACSDK_DEBUG5(LX(__func__));
                if (!mediaControlProxy) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullMediaControlProxy"));
                    return nullptr;
                }
                return shared_ptr<BlueZAVRCPTarget>(new BlueZAVRCPTarget(mediaControlProxy));
            }
            BlueZAVRCPTarget::BlueZAVRCPTarget(shared_ptr<DBusProxy> mediaControlProxy) :
                    m_record{make_shared<AVRCPTargetRecord>("")},
                    m_mediaControlProxy{mediaControlProxy} {
            }
            shared_ptr<SDPRecordInterface> BlueZAVRCPTarget::getRecord() {
                return m_record;
            }
            void BlueZAVRCPTarget::setup() {}
            void BlueZAVRCPTarget::cleanup() {}
            bool BlueZAVRCPTarget::play() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_cmdMutex);
                ManagedGError error;
                m_mediaControlProxy->callMethod(PLAY_CMD, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("error", error.getMessage()));
                    return false;
                }
                return true;
            }
            bool BlueZAVRCPTarget::pause() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_cmdMutex);
                ManagedGError error;
                m_mediaControlProxy->callMethod(PAUSE_CMD, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("error", error.getMessage()));
                    return false;
                }
                return true;
            }
            bool BlueZAVRCPTarget::next() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_cmdMutex);
                ManagedGError error;
                m_mediaControlProxy->callMethod(NEXT_CMD, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("error", error.getMessage()));
                    return false;
                }
                return true;
            }
            bool BlueZAVRCPTarget::previous() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_cmdMutex);
                ManagedGError error;
                m_mediaControlProxy->callMethod(PREVIOUS_CMD, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("error", error.getMessage()));
                    return false;
                }
                return true;
            }
        }
    }
}