#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKEVENTTRACER_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKEVENTTRACER_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/EventTracerInterface.h>

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            class MockEventTracer : public avsCommon::sdkInterfaces::EventTracerInterface {
            public:
                MOCK_METHOD1(traceEvent, void(const std::string& messageContent));
            };
        }
    }
}
#endif