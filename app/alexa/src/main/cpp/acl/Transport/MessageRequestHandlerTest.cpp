#include <gtest/gtest.h>
#include <util/HTTP2/HTTP2RequestInterface.h>
#include "MessageRequestHandler.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                using namespace std;
                using namespace avsCommon;
                using namespace utils;
                using namespace avs;
                using namespace attachment;
                using namespace http2;
                using namespace metrics;
                using namespace testing;
                static const string AUTHORIZATION_HEADER = "Authorization: Bearer ";
                static const string AUTH_TOKEN = "authToken";
                class MessageRequestHandlerTest : public Test {};
                class MockExchangeHandlerContext : public ExchangeHandlerContextInterface {
                public:
                    class HTTP2Request : public HTTP2RequestInterface {
                    public:
                        bool cancel() override {
                            return false;
                        }
                        string getId() const override {
                            return "TestId";
                        }
                    };
                    void onDownchannelConnected() override {}
                    void onDownchannelFinished() override {}
                    void onMessageRequestSent(const shared_ptr<MessageRequest>& request) override {}
                    void onMessageRequestTimeout() override {}
                    void onMessageRequestAcknowledged(const shared_ptr<MessageRequest>& request) override {}
                    void onMessageRequestFinished() override {}
                    void onPingRequestAcknowledged(bool success) override {}
                    void onPingTimeout() override {}
                    void onActivity() override {}
                    void onForbidden(const string& authToken) override {}
                    shared_ptr<HTTP2RequestInterface> createAndSendRequest(
                        const HTTP2RequestConfig& cfg) override {
                        return make_shared<HTTP2Request>();
                    }
                    string getAVSGateway() override {
                        return "";
                    }
                };
                TEST_F(MessageRequestHandlerTest, test_headers) {
                    auto messageRequest = shared_ptr<MessageRequest>(new MessageRequest("{}", true, "",
                                                             {{"k1", "v1"}, {"k2", "v2"}}));
                    auto classUnderTest = MessageRequestHandler::create(make_shared<MockExchangeHandlerContext>(), AUTH_TOKEN, messageRequest,
                                                         shared_ptr<MessageConsumerInterface>(),shared_ptr<AttachmentManager>(),
                                                            shared_ptr<MetricRecorderInterface>());
                    const vector<string> actual = classUnderTest->getRequestHeaderLines();
                    vector<string> expected{AUTHORIZATION_HEADER + AUTH_TOKEN, "k1: v1", "k2: v2"};
                    EXPECT_EQ(actual, expected);
                }
            }
        }
    }
}