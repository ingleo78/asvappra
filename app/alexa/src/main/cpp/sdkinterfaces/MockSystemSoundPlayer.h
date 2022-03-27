#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSYSTEMSOUNDPLAYER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSYSTEMSOUNDPLAYER_H_

#include <gmock/gmock.h>
#include "SystemSoundPlayerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockSystemSoundPlayer : public avsCommon::sdkInterfaces::SystemSoundPlayerInterface {
                public:
                    MOCK_METHOD1(playTone, std::shared_future<bool>(Tone tone));
                };
            }
        }
    }
}
#endif