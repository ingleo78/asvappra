#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMESSAGECONSUMER_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMESSAGECONSUMER_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "MessageConsumerInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            class MockMessageConsumer : public MessageConsumerInterface {
            public:
                //MOCK_METHOD2(consumeMessage, void(const std::string& contextId, const std::string& message));
            };
        }
    }
}
#endif