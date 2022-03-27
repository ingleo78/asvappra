#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKPLAYBACKHANDLER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKPLAYBACKHANDLER_H_

#include <gmock/gmock.h>
#include "PlaybackHandlerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockPlaybackHandler : public PlaybackHandlerInterface {
                public:
                    MOCK_METHOD1(onButtonPressed, void(avs::PlaybackButton button));
                    MOCK_METHOD2(onTogglePressed, void(avs::PlaybackToggle toggle, bool action));
                };
            }
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKPLAYBACKHANDLER_H_
