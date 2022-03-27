#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONTIMINGADAPTER_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONTIMINGADAPTER_H_

#include <gmock/gmock.h>
#include "TimingAdapterFactory.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace testing;
            class MockCaptionTimingAdapter : public CaptionTimingAdapterInterface {
            public:
                //MOCK_METHOD2(queueForDisplay, void(const CaptionFrame& captionFrame, bool autostart));
                MOCK_METHOD0(reset, void());
                MOCK_METHOD0(start, void());
                MOCK_METHOD0(stop, void());
                MOCK_METHOD0(pause, void());
            };
        }
    }
}
#endif