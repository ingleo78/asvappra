#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERINTERFACE_H_

#include <gmock/gmock.h>
#include "EqualizerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    class MockEqualizerInterface : public EqualizerInterface {
                    public:
                        MOCK_METHOD1(setEqualizerBandLevels, void(EqualizerBandLevelMap));
                        MOCK_METHOD0(getMinimumBandLevel, int());
                        MOCK_METHOD0(getMaximumBandLevel, int());
                    };
                }
            }
        }
    }
}
#endif