#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTOPERATION_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTOPERATION_H_

#include <sdkinterfaces/PostConnectOperationInterface.h>

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                class MockPostConnectOperation : public avsCommon::sdkInterfaces::PostConnectOperationInterface {
                public:
                    MOCK_METHOD0(getOperationPriority, unsigned int());
                    MOCK_METHOD1(performOperation, bool(const std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface>&));
                    MOCK_METHOD0(abortOperation, void());
                };
            }
        }
    }
}
#endif