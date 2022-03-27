#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTFACTORY_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTFACTORY_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "PostConnectFactoryInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            class MockPostConnectFactory : public PostConnectFactoryInterface {
            public:
                MOCK_METHOD0(createPostConnect, std::shared_ptr<PostConnectInterface>());
            };
        }
    }
}
#endif