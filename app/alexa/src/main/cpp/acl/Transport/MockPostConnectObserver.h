#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTOBSERVER_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECTOBSERVER_H_

#include "PostConnectObserverInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                class MockPostConnectObserver : public PostConnectObserverInterface {
                public:
                    MOCK_METHOD0(onPostConnected, void());
                    MOCK_METHOD0(onUnRecoverablePostConnectFailure, void());
                };
            }
        }
    }
}
#endif