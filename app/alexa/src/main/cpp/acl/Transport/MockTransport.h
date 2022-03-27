#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKTRANSPORT_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKTRANSPORT_H_

#include <memory>
#include <avs/MessageRequest.h>
#include <gmock/gmock.h>
#include "TransportInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                using testing::Return;
                class MockTransport : public TransportInterface {
                public:
                    MockTransport() : m_id{sNewId++} {};
                    MOCK_METHOD0(doShutdown, void());
                    MOCK_METHOD0(connect, bool());
                    MOCK_METHOD0(disconnect, void());
                    MOCK_METHOD0(isConnected, bool());
                    MOCK_METHOD0(isPendingDisconnected, bool());
                    MOCK_METHOD0(onRequestEnqueued, void());
                    MOCK_METHOD0(onWakeConnectionRetry, void());
                    MOCK_METHOD0(onWakeVerifyConnectivity, void());
                    MOCK_METHOD1(sendMessage, void(std::shared_ptr<avsCommon::avs::MessageRequest>));
                    //MOCK_METHOD2(onAttachmentReceived, void(const std::string& contextId, const std::string& message));
                    const int m_id;
                private:
                    static int sNewId;
                };
                int MockTransport::sNewId = 0;
                void initializeMockTransport(MockTransport* transport) {
                    ON_CALL(*transport, connect()).WillByDefault(Return(true));
                    ON_CALL(*transport, isConnected()).WillByDefault(Return(false));
                }
                void connectMockTransport(MockTransport* transport) {
                    initializeMockTransport(transport);
                    ON_CALL(*transport, isConnected()).WillByDefault(Return(true));
                }
                void disconnectMockTransport(MockTransport* transport) {
                    ON_CALL(*transport, isConnected()).WillByDefault(Return(false));
                }
            }
        }
    }
}
#endif