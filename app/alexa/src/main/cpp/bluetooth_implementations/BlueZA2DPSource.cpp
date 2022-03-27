#include <logger/Logger.h>
#include "BlueZA2DPSource.h"
#include "BlueZDeviceManager.h"
#include "MediaEndpoint.h"

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
            static const string TAG{"BlueZA2DPSource"};
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<BlueZA2DPSource> BlueZA2DPSource::create(shared_ptr<BlueZDeviceManager> deviceManager) {
                if (nullptr == deviceManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "deviceManager is null"));
                    return nullptr;
                }
                return shared_ptr<BlueZA2DPSource>(new BlueZA2DPSource(deviceManager));
            }
            shared_ptr<FormattedAudioStreamAdapter> BlueZA2DPSource::getSourceStream() {
                auto endpoint = m_deviceManager->getMediaEndpoint();
                if (!endpoint) {
                    ACSDK_ERROR(LX("getSourceStreamFailed").d("reason", "Failed to get media endpoint"));
                    return nullptr;
                }
                return endpoint->getAudioStream();
            }
            void BlueZA2DPSource::setup() {}
            void BlueZA2DPSource::cleanup() {}
            shared_ptr<SDPRecordInterface> BlueZA2DPSource::getRecord() {
                return m_record;
            }
            BlueZA2DPSource::BlueZA2DPSource(std::shared_ptr<BlueZDeviceManager> deviceManager) : m_record{make_shared<A2DPSourceRecord>("")},
                                                                                                  m_deviceManager{deviceManager} {}
        }
    }
}