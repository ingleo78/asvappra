#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERCONTROLLERLISTENERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKEQUALIZERCONTROLLERLISTENERINTERFACE_H_

#include <gmock/gmock.h>
#include "EqualizerControllerListenerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    class MockEqualizerControllerListenerInterface : public EqualizerControllerListenerInterface {
                    public:
                        MOCK_METHOD1(onEqualizerStateChanged, void(const EqualizerState&));
                        MOCK_METHOD1(onEqualizerSameStateChanged, void(const EqualizerState&));
                    };
                }
            }
        }
    }
}
#endif