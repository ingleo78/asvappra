#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKALERTSAUDIOFACTORY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKALERTSAUDIOFACTORY_H_

#include <gmock/gmock.h>
#include "AlertsAudioFactoryInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    class MockAlertsAudioFactory : public AlertsAudioFactoryInterface {
                        MOCK_CONST_METHOD0(alarmDefault, std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()>());
                        MOCK_CONST_METHOD0(alarmShort, std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()>());
                        MOCK_CONST_METHOD0(timerDefault, std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()>());
                        MOCK_CONST_METHOD0(timerShort, std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()>());
                        MOCK_CONST_METHOD0(reminderDefault, std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()>());
                        MOCK_CONST_METHOD0(reminderShort, std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()>());
                    };
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKALERTSAUDIOFACTORY_H_
