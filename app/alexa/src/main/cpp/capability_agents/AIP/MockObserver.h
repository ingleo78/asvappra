#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_TEST_MOCKOBSERVER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_TEST_MOCKOBSERVER_H_

#include <gmock/gmock.h>
#include <sdkinterfaces/AudioInputProcessorObserverInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace aip {
            namespace test {
                class MockObserver : public avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface {
                public:
                    MOCK_METHOD1(onStateChanged, void(avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State state));
                };
            }
        }
    }
}
#endif