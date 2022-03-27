#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKTRANSPORTOBSERVER_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKTRANSPORTOBSERVER_H_

#include <gmock/gmock.h>
#include <memory>
#include "TransportObserverInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                using namespace std;
                using namespace avsCommon::sdkInterfaces;
                class MockTransportObserver : public TransportObserverInterface {
                public:
                    MOCK_METHOD1(onConnected, void(shared_ptr<TransportInterface> transport));
                    //MOCK_METHOD2(onDisconnected, void(shared_ptr<TransportInterface> transport, ConnectionStatusObserverInterface::ChangedReason reason));
                    MOCK_METHOD1(onServerSideDisconnect, void(std::shared_ptr<TransportInterface> transport));
                };
            }
        }
    }
}
#endif