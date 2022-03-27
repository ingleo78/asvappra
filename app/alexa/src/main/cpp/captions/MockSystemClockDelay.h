#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKSYSTEMCLOCKDELAY_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKSYSTEMCLOCKDELAY_H_

#include <gmock/gmock.h>
#include "DelayInterface.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace testing;
            class MockSystemClockDelay : public DelayInterface {
            public:
                MockSystemClockDelay() = default;
                MOCK_METHOD1(delay, void(milliseconds));
            };
        }
    }
}
#endif