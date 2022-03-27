#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMESSAGEREQUEST_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMESSAGEREQUEST_H_

#include <memory>
#include <string>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include <gmock/gmock.h>

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            class MockMessageRequest : public avsCommon::avs::MessageRequest {
            public:
                MockMessageRequest() : avsCommon::avs::MessageRequest{""} {}
                MOCK_METHOD1(exceptionReceived, void(const std::string& exceptionMessage));
                MOCK_METHOD1(sendCompleted, void(avsCommon::sdkInterfaces::MessageRequestObserverInterface::Status status));
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMESSAGEREQUEST_H_
