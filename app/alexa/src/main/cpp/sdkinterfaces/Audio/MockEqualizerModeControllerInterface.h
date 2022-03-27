#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERMODECONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERMODECONTROLLERINTERFACE_H_

#include <gmock/gmock.h>
#include "EqualizerModeControllerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    class MockEqualizerModeControllerInterface : public EqualizerModeControllerInterface {
                    public:
                        MOCK_METHOD1(setEqualizerMode, bool(avsCommon::sdkInterfaces::audio::EqualizerMode));
                    };
                }
            }
        }
    }
}
#endif