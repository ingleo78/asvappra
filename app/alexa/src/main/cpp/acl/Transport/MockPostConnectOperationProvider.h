#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTOPERATIONPROVIDER_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTOPERATIONPROVIDER_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/PostConnectOperationProviderInterface.h>

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                class MockPostConnectOperationProvider : public avsCommon::sdkInterfaces::PostConnectOperationProviderInterface {
                public:
                    MOCK_METHOD0(createPostConnectOperation, std::shared_ptr<avsCommon::sdkInterfaces::PostConnectOperationInterface>());
                };
            }
        }
    }
}
#endif