#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONPRESENTER_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONPRESENTER_H_

#include <gmock/gmock.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include "CaptionPresenterInterface.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace std;
            using namespace testing;
            using namespace avsCommon;
            using namespace avs;
            class MockCaptionPresenter : public CaptionPresenterInterface {
            public:
                //MOCK_METHOD2(onCaptionActivity, void(const CaptionFrame&, FocusState));
                MOCK_METHOD1(getWrapIndex, pair<bool, int>(const CaptionLine&));
            };
        }
    }
}
#endif