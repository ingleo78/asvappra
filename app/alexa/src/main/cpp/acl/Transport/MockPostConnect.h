#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECT_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKPOSTCONNECT_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "PostConnectInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            using namespace std;
            using namespace avsCommon::sdkInterfaces;
            class MockPostConnect : public PostConnectInterface {
            public:
                //MOCK_METHOD2(doPostConnect, bool(shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver));
                MOCK_METHOD0(onDisconnect, void());
            };
        }
    }
}
#endif