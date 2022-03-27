#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKPLAYBACKROUTER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKPLAYBACKROUTER_H_

#include <gmock/gmock.h>
#include "PlaybackRouterInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace chrono;
                using namespace avs;
                class MockPlaybackRouter : public PlaybackRouterInterface {
                public:
                    MOCK_METHOD1(buttonPressed, void(PlaybackButton button));
                    //MOCK_METHOD2(togglePressed, void(PlaybackToggle toggle, bool action));
                    MOCK_METHOD1(setHandler, void(shared_ptr<PlaybackHandlerInterface> handler));
                    MOCK_METHOD0(switchToDefaultHandler, void());
                    //MOCK_METHOD2(setHandler,void(shared_ptr<PlaybackHandlerInterface> handler, shared_ptr<LocalPlaybackHandlerInterface> localHandler));
                    MOCK_METHOD1(useDefaultHandlerWith, void(shared_ptr<LocalPlaybackHandlerInterface> localHandler));
                    MOCK_METHOD1(localOperation, bool(LocalPlaybackHandlerInterface::PlaybackOperation op));
                    //MOCK_METHOD2(localSeekTo, bool(milliseconds location, bool fromStart));
                };
            }
        }
    }
}
#endif