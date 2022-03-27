#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSTATESYNCHRONIZEROBSERVER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSTATESYNCHRONIZEROBSERVER_H_

#include <gmock/gmock.h>
#include "StateSynchronizerObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockStateSynchronizerObserver : public StateSynchronizerObserverInterface {
                public:
                    MOCK_METHOD1(onStateChanged, void(StateSynchronizerObserverInterface::State newstate));
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSTATESYNCHRONIZEROBSERVER_H_
