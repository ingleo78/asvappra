#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERCONFIGURATIONINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERCONFIGURATIONINTERFACE_H_

#include <gmock/gmock.h>
#include <set>
#include "EqualizerConfigurationInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    class MockEqualizerConfigurationInterface : public EqualizerConfigurationInterface {
                    public:
                        MOCK_CONST_METHOD0(getSupportedBands, std::set<EqualizerBand>());
                        MOCK_CONST_METHOD0(getSupportedModes, std::set<EqualizerMode>());
                        MOCK_CONST_METHOD0(getMinBandLevel, int());
                        MOCK_CONST_METHOD0(getMaxBandLevel, int());
                        MOCK_CONST_METHOD0(getDefaultBandDelta, int());
                        MOCK_CONST_METHOD0(getDefaultState, EqualizerState());
                        MOCK_CONST_METHOD1(isBandSupported, bool(EqualizerBand));
                        MOCK_CONST_METHOD1(isModeSupported, bool(EqualizerMode));
                    };
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERCONFIGURATIONINTERFACE_H_
