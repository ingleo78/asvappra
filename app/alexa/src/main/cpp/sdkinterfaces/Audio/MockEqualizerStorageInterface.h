#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERSTORAGEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERSTORAGEINTERFACE_H_

#include <gmock/gmock.h>
#include "EqualizerStorageInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    class MockEqualizerStorageInterface : public EqualizerStorageInterface {
                    public:
                        MOCK_METHOD1(saveState, void(const EqualizerState&));
                        MOCK_METHOD0(loadState, avsCommon::utils::error::SuccessResult<EqualizerState>());
                        MOCK_METHOD0(clear, void());
                    };
                }
            }
        }
    }
}
#endif