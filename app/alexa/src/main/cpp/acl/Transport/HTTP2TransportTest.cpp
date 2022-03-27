#include <future>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <avs/attachment/AttachmentManager.h>
#include <avs/attachment/AttachmentUtils.h>
#include <util/PromiseFuturePair.h>
#include <util/HTTP/HttpResponseCode.h>
#include <util/HTTP2/HTTP2RequestConfig.h>
#include <metrics/MetricRecorderInterface.h>
#include "HTTP2Transport.h"
#include "MockAuthDelegate.h"
#include "MockEventTracer.h"
#include "MockHTTP2Connection.h"
#include "MockHTTP2Request.h"
#include "MockMessageConsumer.h"
#include "MockPostConnect.h"
#include "MockPostConnectFactory.h"
#include "MockTransportObserver.h"
#include "SynchronizedMessageRequestQueue.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                using namespace std;
                using namespace chrono;
                using namespace avsCommon;
                using namespace avs;
                using namespace attachment;
                using namespace sdkInterfaces;
                using namespace utils;
                using namespace http;
                using namespace http2;
                using namespace metrics;
                using namespace testing;
                using namespace acl::test;
                using namespace http2::test;
                static const string TEST_AVS_GATEWAY_STRING = "http://avs-alexa-na.amazon.com";
                static const string AVS_DOWNCHANNEL_URL_PATH_EXTENSION = "/v20160207/directives";
                static const string AVS_PING_URL_PATH_EXTENSION = "/ping";
                static const string FULL_DOWNCHANNEL_URL = TEST_AVS_GATEWAY_STRING + AVS_DOWNCHANNEL_URL_PATH_EXTENSION;
                static const string FULL_PING_URL = TEST_AVS_GATEWAY_STRING + AVS_PING_URL_PATH_EXTENSION;
                static const auto ONE_HUNDRED_MILLISECOND_DELAY = milliseconds(100);
                static const auto TEN_MILLISECOND_DELAY = milliseconds(10);
                static const auto SHORT_DELAY = seconds(1);
                static const auto RESPONSE_TIMEOUT = seconds(5);
                static const auto LONG_RESPONSE_TIMEOUT = seconds(10);
                static const string HTTP_AUTHORIZATION_HEADER_BEARER = "Authorization: Bearer";
                static const string CBL_AUTHORIZATION_TOKEN = "AUTH_TOKEN";
                static const string TEST_ATTACHMENT_ID_STRING_ONE = "testAttachmentId_1";
                static const string TEST_MESSAGE = "aaabbccc";
                static const string TEST_ATTACHMENT_MESSAGE = "MY_A_T_T_ACHMENT";
                static const string TEST_ATTACHMENT_FIELD = "ATTACHMENT";
                static const long NO_CALL = -2;
                static const string NON_MIME_PAYLOAD = "A_NON_MIME_PAYLOAD";
                static const string DIRECTIVE1 = "{\"namespace:\"SpeechSynthesizer\",name:\"Speak\",messageId:\"351df0ff-8041-4891-a925-136f52d54da1\","
                                                 "dialogRequestId:\"58352bb2-7d07-4ba2-944b-10e6df25d193\"}";
                static const string DIRECTIVE2 = "{\"namespace:\"Alerts\",name:\"SetAlert\",messageId:\"ccc005b8-ca8f-4c34-aeb5-73a8dbbd8d37\",dialogRequestId:"
                                                 "\"dca0bece-16a7-44f3-b940-e6c4ecc2b1f5\"}";
                static const string MIME_BOUNDARY = "thisisaboundary";
                static const string MIME_BODY_DIRECTIVE1 = "--" + MIME_BOUNDARY + "\r\nContent-Type: application/json" +
                                                           "\r\n\r\n" + DIRECTIVE1 + "\r\n--" + MIME_BOUNDARY + "--\r\n";
                static const string MIME_BODY_DIRECTIVE2 = "--" + MIME_BOUNDARY + "\r\nContent-Type: application/json" +
                                                           "\r\n\r\n" + DIRECTIVE2 + "\r\n--" + MIME_BOUNDARY + "\r\n";
                static const string HTTP_BOUNDARY_HEADER = "Content-Type: multipart/related; boundary=" + MIME_BOUNDARY + "; type=application/json";
                static const unsigned MAX_PING_STREAMS = 1;
                static const unsigned MAX_DOWNCHANNEL_STREAMS = 1;
                static const unsigned MAX_AVS_STREAMS = 10;
                static const unsigned MAX_POST_STREAMS = MAX_AVS_STREAMS - MAX_DOWNCHANNEL_STREAMS - MAX_PING_STREAMS;
                class HTTP2TransportTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    void setupHandlers(bool sendOnPostConnected, bool expectConnected);
                    void sendAuthStateRefreshed();
                    void authorizeAndConnect();
                    shared_ptr<HTTP2Transport> m_http2Transport;
                    shared_ptr<MockAuthDelegate> m_mockAuthDelegate;
                    shared_ptr<MockHTTP2Connection> m_mockHttp2Connection;
                    shared_ptr<MockMessageConsumer> m_mockMessageConsumer;
                    shared_ptr<AttachmentManager> m_attachmentManager;
                    shared_ptr<MockTransportObserver> m_mockTransportObserver;
                    shared_ptr<MockPostConnectFactory> m_mockPostConnectFactory;
                    shared_ptr<MetricRecorderInterface> m_mockMetricRecorder;
                    shared_ptr<MockEventTracer> m_mockEventTracer;
                    shared_ptr<MockPostConnect> m_mockPostConnect;
                    shared_ptr<SynchronizedMessageRequestQueue> m_synchronizedMessageRequestQueue;
                    PromiseFuturePair<shared_ptr<AuthObserverInterface>> m_authObserverSet;
                    PromiseFuturePair<void> m_createPostConnectCalled;
                    PromiseFuturePair<pair<shared_ptr<MessageSenderInterface>, shared_ptr<PostConnectObserverInterface>>> m_doPostConnected;
                    PromiseFuturePair<void> m_transportConnected;
                };
                class TestMessageRequestObserver : public MessageRequestObserverInterface {
                public:
                    void onSendCompleted(MessageRequestObserverInterface::Status status) {
                        m_status.setValue(status);
                    }
                    void onExceptionReceived(const string& exceptionMessage) {
                        m_exception.setValue(exceptionMessage);
                    }
                    PromiseFuturePair<MessageRequestObserverInterface::Status> m_status;
                    PromiseFuturePair<string> m_exception;
                };
                void HTTP2TransportTest::SetUp() {
                    m_mockAuthDelegate = make_shared<NiceMock<MockAuthDelegate>>();
                    m_mockHttp2Connection = make_shared<NiceMock<MockHTTP2Connection>>(FULL_DOWNCHANNEL_URL, FULL_PING_URL);
                    //m_mockMessageConsumer = make_shared<NiceMock<MockMessageConsumer>>();
                    m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                    //m_mockTransportObserver = make_shared<NiceMock<MockTransportObserver>>();
                    m_mockPostConnectFactory = make_shared<NiceMock<MockPostConnectFactory>>();
                    m_mockEventTracer = make_shared<NiceMock<MockEventTracer>>();
                    //m_mockPostConnect = make_shared<NiceMock<MockPostConnect>>();
                    //m_mockMetricRecorder = make_shared<NiceMock<MetricRecorderInterface>>();
                    m_mockAuthDelegate->setAuthToken(CBL_AUTHORIZATION_TOKEN);
                    HTTP2Transport::Configuration cfg;
                    m_synchronizedMessageRequestQueue = make_shared<SynchronizedMessageRequestQueue>();
                    m_http2Transport = HTTP2Transport::create(m_mockAuthDelegate, TEST_AVS_GATEWAY_STRING, m_mockHttp2Connection, m_mockMessageConsumer,
                                                              m_attachmentManager, m_mockTransportObserver, m_mockPostConnectFactory, m_synchronizedMessageRequestQueue,
                                                              cfg, m_mockMetricRecorder, m_mockEventTracer);
                    ASSERT_NE(m_http2Transport, nullptr);
                }
                void HTTP2TransportTest::TearDown() {
                    m_http2Transport->shutdown();
                }
                void HTTP2TransportTest::setupHandlers(bool sendOnPostConnected, bool expectConnected) {
                    Sequence s1, s2;
                    EXPECT_CALL(*m_mockAuthDelegate, addAuthObserver(_))
                        .InSequence(s1).WillOnce(Invoke([this](std::shared_ptr<avsCommon::sdkInterfaces::AuthObserverInterface> argAuthObserver) {
                            m_authObserverSet.setValue(argAuthObserver);
                        }));
                    {
                        InSequence dummy;
                        EXPECT_CALL(*m_mockPostConnectFactory, createPostConnect()).WillOnce(InvokeWithoutArgs([this] {
                            m_createPostConnectCalled.setValue();
                            return m_mockPostConnect;
                        }));
                        /*EXPECT_CALL(*m_mockPostConnect, doPostConnect(_, _))
                            .InSequence(s2)
                            .WillOnce(Invoke([this, sendOnPostConnected](shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver) {
                                m_doPostConnected.setValue(std::make_pair(postConnectSender, postConnectObserver));
                                if (sendOnPostConnected) postConnectObserver->onPostConnected();
                                return true;
                            }));*/
                    }
                    if (expectConnected) {
                        EXPECT_CALL(*m_mockTransportObserver, onConnected(_)).InSequence(s1, s2)
                            .WillOnce(Invoke([this](std::shared_ptr<TransportInterface> transport) { m_transportConnected.setValue(); }));
                    }
                }
                void HTTP2TransportTest::sendAuthStateRefreshed() {
                    shared_ptr<AuthObserverInterface> authObserver;
                    ASSERT_TRUE(m_authObserverSet.waitFor(RESPONSE_TIMEOUT));
                    authObserver = m_authObserverSet.getValue();
                    ASSERT_TRUE(authObserver != nullptr);
                    ASSERT_EQ(authObserver.get(), m_http2Transport.get());
                    authObserver->onAuthStateChange(AuthObserverInterface::State::REFRESHED, AuthObserverInterface::Error::SUCCESS);
                }
                void HTTP2TransportTest::authorizeAndConnect() {
                    setupHandlers(true, true);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    ASSERT_TRUE(m_mockHttp2Connection->respondToDownchannelRequests(static_cast<long>(HTTPResponseCode::SUCCESS_OK), false, RESPONSE_TIMEOUT));
                    ASSERT_TRUE(m_doPostConnected.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_TRUE(m_transportConnected.waitFor(LONG_RESPONSE_TIMEOUT));
                }
                TEST_F(HTTP2TransportTest, testSlow_emptyAuthToken) {
                    m_mockAuthDelegate->setAuthToken("");
                    setupHandlers(false, false);
                    m_http2Transport->connect();
                    std::this_thread::sleep_for(SHORT_DELAY);
                    ASSERT_TRUE(m_mockHttp2Connection->isRequestQueueEmpty());
                    sendAuthStateRefreshed();
                    ASSERT_EQ(m_mockHttp2Connection->waitForRequest(ONE_HUNDRED_MILLISECOND_DELAY), nullptr);
                }
                TEST_F(HTTP2TransportTest, testSlow_waitAuthDelegateInterface) {
                    setupHandlers(false, false);
                    m_http2Transport->connect();
                    std::this_thread::sleep_for(SHORT_DELAY);
                    ASSERT_TRUE(m_mockHttp2Connection->isRequestQueueEmpty());
                    sendAuthStateRefreshed();
                    ASSERT_NE(m_mockHttp2Connection->waitForRequest(RESPONSE_TIMEOUT), nullptr);
                    auto request = m_mockHttp2Connection->dequeRequest();
                    ASSERT_EQ(request->getUrl(), FULL_DOWNCHANNEL_URL);
                }
                TEST_F(HTTP2TransportTest, test_bearerTokenInRequest) {
                    setupHandlers(false, false);
                    m_mockHttp2Connection->setWaitRequestHeader(HTTP_AUTHORIZATION_HEADER_BEARER);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    ASSERT_TRUE(m_mockHttp2Connection->waitForRequestWithHeader(RESPONSE_TIMEOUT));
                }
                TEST_F(HTTP2TransportTest, test_triggerPostConnectObject) {
                    setupHandlers(false, false);
                    EXPECT_CALL(*m_mockTransportObserver, onConnected(_)).Times(0);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    ASSERT_TRUE(m_mockHttp2Connection->respondToDownchannelRequests(static_cast<long>(HTTPResponseCode::SUCCESS_OK), false, RESPONSE_TIMEOUT));
                    ASSERT_TRUE(m_createPostConnectCalled.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_TRUE(m_doPostConnected.waitFor(RESPONSE_TIMEOUT));
                }
                TEST_F(HTTP2TransportTest, test_connectionStatusOnPostConnect) {
                    setupHandlers(true, true);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    ASSERT_TRUE(m_mockHttp2Connection->respondToDownchannelRequests(static_cast<long>(HTTPResponseCode::SUCCESS_OK), false, RESPONSE_TIMEOUT));
                    ASSERT_TRUE(m_doPostConnected.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_TRUE(m_transportConnected.waitFor(LONG_RESPONSE_TIMEOUT));
                }
                TEST_F(HTTP2TransportTest, testSlow_retryOnDownchannelConnectionFailure) {
                    setupHandlers(false, false);
                    EXPECT_CALL(*m_mockTransportObserver, onConnected(_)).Times(0);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    ASSERT_TRUE(m_mockHttp2Connection->respondToDownchannelRequests(static_cast<long>(HTTPResponseCode::SERVER_ERROR_INTERNAL), false, RESPONSE_TIMEOUT));
                    m_mockHttp2Connection->waitForRequest(LONG_RESPONSE_TIMEOUT, 2);
                }
                TEST_F(HTTP2TransportTest, test_messageRequestContent) {
                    setupHandlers(false, false);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    ASSERT_TRUE(m_mockHttp2Connection->respondToDownchannelRequests(static_cast<long>(HTTPResponseCode::SUCCESS_OK), false, RESPONSE_TIMEOUT));
                    ASSERT_TRUE(m_doPostConnected.waitFor(RESPONSE_TIMEOUT));
                    std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                    m_http2Transport->sendMessage(messageReq);
                    auto postMessage = m_mockHttp2Connection->waitForPostRequest(LONG_RESPONSE_TIMEOUT);
                    ASSERT_NE(postMessage, nullptr);
                    ASSERT_EQ(postMessage->getMimeResponseSink()->getCountOfMimeParts(), 1u);
                    auto mimeMessage = postMessage->getMimeResponseSink()->getMimePart(0);
                    std::string mimeMessageString(mimeMessage.begin(), mimeMessage.end());
                    ASSERT_EQ(TEST_MESSAGE, mimeMessageString);
                }
                TEST_F(HTTP2TransportTest, test_messageRequestWithAttachment) {
                    vector<char> attachment(TEST_ATTACHMENT_MESSAGE.begin(), TEST_ATTACHMENT_MESSAGE.end());
                    shared_ptr<AttachmentReader> attachmentReader = AttachmentUtils::createAttachmentReader(attachment);
                    ASSERT_NE(attachmentReader, nullptr);
                    setupHandlers(false, false);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    m_mockHttp2Connection->respondToDownchannelRequests(static_cast<long>(HTTPResponseCode::SUCCESS_OK), false, RESPONSE_TIMEOUT);
                    ASSERT_TRUE(m_doPostConnected.waitFor(RESPONSE_TIMEOUT));
                    shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                    messageReq->addAttachmentReader(TEST_ATTACHMENT_FIELD, attachmentReader);
                    m_http2Transport->sendMessage(messageReq);
                    auto postMessage = m_mockHttp2Connection->waitForPostRequest(LONG_RESPONSE_TIMEOUT);
                    ASSERT_NE(postMessage, nullptr);
                    ASSERT_EQ(postMessage->getMimeResponseSink()->getCountOfMimeParts(), 2u);
                    auto mimeMessage = postMessage->getMimeResponseSink()->getMimePart(0);
                    string mimeMessageString(mimeMessage.begin(), mimeMessage.end());
                    ASSERT_EQ(TEST_MESSAGE, mimeMessageString);
                    auto mimeAttachment = postMessage->getMimeResponseSink()->getMimePart(1);
                    string mimeAttachmentString(mimeAttachment.begin(), mimeAttachment.end());
                    ASSERT_EQ(TEST_ATTACHMENT_MESSAGE, mimeAttachmentString);
                }
                TEST_F(HTTP2TransportTest, test_pauseSendWhenSDSEmpty) {
                    setupHandlers(false, false);
                    m_http2Transport->connect();
                    sendAuthStateRefreshed();
                    m_mockHttp2Connection->respondToDownchannelRequests(static_cast<long>(HTTPResponseCode::SUCCESS_OK), false, RESPONSE_TIMEOUT);
                    ASSERT_TRUE(m_doPostConnected.waitFor(RESPONSE_TIMEOUT));
                    std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                    AttachmentManager attMgr(AttachmentManager::AttachmentType::IN_PROCESS);
                    std::vector<char> attachment(TEST_ATTACHMENT_MESSAGE.begin(), TEST_ATTACHMENT_MESSAGE.end());
                    std::shared_ptr<AttachmentReader> attachmentReader = attMgr.createReader(TEST_ATTACHMENT_ID_STRING_ONE, avsCommon::utils::sds::ReaderPolicy::NONBLOCKING);
                    ASSERT_NE(attachmentReader, nullptr);
                    messageReq->addAttachmentReader(TEST_ATTACHMENT_FIELD, attachmentReader);
                    m_http2Transport->sendMessage(messageReq);
                    std::thread writerThread([this, &attMgr, &attachment]() {
                        const unsigned chunks = 4;
                        unsigned int chunkSize = (attachment.size() + chunks - 1) / chunks;
                        auto writer = attMgr.createWriter(TEST_ATTACHMENT_ID_STRING_ONE, avsCommon::utils::sds::WriterPolicy::BLOCKING);
                        AttachmentWriter::WriteStatus writeStatus = AttachmentWriter::WriteStatus::OK;
                        unsigned int lastChunkSize = (attachment.size() % chunks == 0) ? chunkSize : attachment.size() % chunks;
                        for (unsigned chunk = 0; chunk < chunks; chunk++) {
                            writer->write(&attachment[chunk * chunkSize], (chunk == chunks - 1) ? lastChunkSize : chunkSize, &writeStatus);
                            ASSERT_EQ(writeStatus, AttachmentWriter::WriteStatus::OK);
                            ASSERT_TRUE(m_mockHttp2Connection->isPauseOnSendReceived(ONE_HUNDRED_MILLISECOND_DELAY));
                        }
                        writer->close();
                    });
                    auto postMessage = m_mockHttp2Connection->waitForPostRequest(LONG_RESPONSE_TIMEOUT);
                    ASSERT_NE(postMessage, nullptr);
                    ASSERT_EQ(postMessage->getMimeResponseSink()->getCountOfMimeParts(), 2u);
                    auto mimeMessage = postMessage->getMimeResponseSink()->getMimePart(0);
                    std::string mimeMessageString(mimeMessage.begin(), mimeMessage.end());
                    ASSERT_EQ(TEST_MESSAGE, mimeMessageString);
                    auto mimeAttachment = postMessage->getMimeResponseSink()->getMimePart(1);
                    std::string mimeAttachmentString(mimeAttachment.begin(), mimeAttachment.end());
                    ASSERT_EQ(TEST_ATTACHMENT_MESSAGE, mimeAttachmentString);
                    writerThread.join();
                }
                TEST_F(HTTP2TransportTest, testSlow_messageRequestsQueuing) {
                    authorizeAndConnect();
                    std::vector<std::shared_ptr<TestMessageRequestObserver>> messageObservers;
                    const unsigned messagesCount = 5;
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                        auto messageObserver = std::make_shared<TestMessageRequestObserver>();
                        messageObservers.push_back(messageObserver);
                        messageReq->addObserver(messageObserver);
                        m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                        m_http2Transport->onRequestEnqueued();
                    }
                    std::this_thread::sleep_for(SHORT_DELAY);
                    ASSERT_EQ(m_mockHttp2Connection->getPostRequestsNum(), 1u);
                    unsigned int postsRequestsCount = 0;
                    while (postsRequestsCount < messagesCount) {
                        auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                        if (request) {
                            postsRequestsCount++;
                            std::this_thread::sleep_for(SHORT_DELAY);
                            request->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_OK);
                        } else break;
                    }
                    ASSERT_EQ(postsRequestsCount, messagesCount);
                    EXPECT_CALL(*m_mockHttp2Connection, disconnect()).WillOnce(Invoke([this]() {
                        while (true) {
                            auto request = m_mockHttp2Connection->dequePostRequest();
                            if (!request) break;
                            request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::CANCELLED);
                        };
                    }));
                    m_http2Transport->shutdown();
                    unsigned messagesCanceled = 0;
                    unsigned messagesRemaining = 0;
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        if (messageObservers[messageNum]->m_status.waitFor(RESPONSE_TIMEOUT)) {
                            switch(messageObservers[messageNum]->m_status.getValue()) {
                                case MessageRequestObserverInterface::Status::CANCELED: case MessageRequestObserverInterface::Status::NOT_CONNECTED: messagesCanceled++;
                                default: break;
                            }
                        }
                    }
                    while(m_synchronizedMessageRequestQueue->dequeueOldestRequest()) ++messagesRemaining;
                    ASSERT_EQ(messagesCanceled + messagesRemaining, messagesCount);
                }
                TEST_F(HTTP2TransportTest, messageRequests_SequentialSend) {
                    authorizeAndConnect();
                    std::vector<std::shared_ptr<TestMessageRequestObserver>> messageObservers;
                    unsigned int messagesCount = 5;
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                        auto messageObserver = std::make_shared<TestMessageRequestObserver>();
                        messageObservers.push_back(messageObserver);
                        messageReq->addObserver(messageObserver);
                        m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                        m_http2Transport->onRequestEnqueued();
                    }
                    unsigned int postsRequestsCount = 0;
                    while(postsRequestsCount < messagesCount) {
                        std::this_thread::sleep_for(SHORT_DELAY);
                        postsRequestsCount++;
                        ASSERT_EQ((int)m_mockHttp2Connection->getPostRequestsNum(), (int)postsRequestsCount);
                        auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                        if (request) request->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_OK);
                    }
                    ASSERT_EQ(postsRequestsCount, messagesCount);
                    EXPECT_CALL(*m_mockHttp2Connection, disconnect()).WillOnce(Invoke([this]() {
                        while(true) {
                            auto request = m_mockHttp2Connection->dequePostRequest();
                            if (!request) break;
                            request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::CANCELLED);
                        };
                    }));
                    m_http2Transport->shutdown();
                    unsigned messagesCanceled = 0;
                    unsigned messagesRemaining = 0;
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        if (messageObservers[messageNum]->m_status.waitFor(RESPONSE_TIMEOUT)) {
                            switch(messageObservers[messageNum]->m_status.getValue()) {
                                case MessageRequestObserverInterface::Status::CANCELED: case MessageRequestObserverInterface::Status::NOT_CONNECTED:
                                    messagesCanceled++;
                                default: break;
                            }
                        }
                    }
                    while(m_synchronizedMessageRequestQueue->dequeueOldestRequest()) ++messagesRemaining;
                    ASSERT_EQ(messagesCanceled + messagesRemaining, messagesCount);
                }
                TEST_F(HTTP2TransportTest, messageRequests_concurrentSend) {
                    authorizeAndConnect();
                    std::vector<std::shared_ptr<TestMessageRequestObserver>> messageObservers;
                    unsigned int messagesCount = 5;
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, false, "");
                        auto messageObserver = std::make_shared<TestMessageRequestObserver>();
                        messageObservers.push_back(messageObserver);
                        messageReq->addObserver(messageObserver);
                        m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                        m_http2Transport->onRequestEnqueued();
                    }
                    unsigned int postsRequestsCount = 0;
                    while(postsRequestsCount < messagesCount) {
                        auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                        postsRequestsCount++;
                    }
                    ASSERT_EQ(postsRequestsCount, messagesCount);
                    EXPECT_CALL(*m_mockHttp2Connection, disconnect()).WillOnce(Invoke([this]() {
                        while(true) {
                            auto request = m_mockHttp2Connection->dequePostRequest();
                            if (!request) break;
                            request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::CANCELLED);
                        };
                    }));
                    m_http2Transport->shutdown();
                    unsigned messagesCanceled = 0;
                    unsigned messagesRemaining = 0;
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        if (messageObservers[messageNum]->m_status.waitFor(RESPONSE_TIMEOUT)) {
                            switch(messageObservers[messageNum]->m_status.getValue()) {
                                case MessageRequestObserverInterface::Status::CANCELED: case MessageRequestObserverInterface::Status::NOT_CONNECTED: messagesCanceled++;
                                default: break;
                            }
                        }
                    }
                    while(m_synchronizedMessageRequestQueue->dequeueOldestRequest()) ++messagesRemaining;
                    ASSERT_EQ(messagesCanceled + messagesRemaining, messagesCount);
                }
                TEST_F(HTTP2TransportTest, test_onSendCompletedNotification) {
                    const std::vector<std::tuple<long, HTTP2ResponseFinishedStatus, MessageRequestObserverInterface::Status>>
                        messageResponseMap = {
                            { std::make_tuple(NO_CALL, HTTP2ResponseFinishedStatus::INTERNAL_ERROR, MessageRequestObserverInterface::Status::INTERNAL_ERROR) },
                            { std::make_tuple(NO_CALL, HTTP2ResponseFinishedStatus::CANCELLED, MessageRequestObserverInterface::Status::CANCELED) },
                            { std::make_tuple(NO_CALL, HTTP2ResponseFinishedStatus::TIMEOUT, MessageRequestObserverInterface::Status::TIMEDOUT) },
                            { std::make_tuple(NO_CALL, static_cast<HTTP2ResponseFinishedStatus>(-1), MessageRequestObserverInterface::Status::INTERNAL_ERROR) },
                            { std::make_tuple(HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED, HTTP2ResponseFinishedStatus::INTERNAL_ERROR,
                                              MessageRequestObserverInterface::Status::INTERNAL_ERROR) },
                            { std::make_tuple(HTTPResponseCode::SUCCESS_OK, HTTP2ResponseFinishedStatus::CANCELLED, MessageRequestObserverInterface::Status::CANCELED) },
                            { std::make_tuple(HTTPResponseCode::REDIRECTION_START_CODE, HTTP2ResponseFinishedStatus::TIMEOUT,
                                              MessageRequestObserverInterface::Status::TIMEDOUT) },
                            {std::make_tuple(HTTPResponseCode::CLIENT_ERROR_BAD_REQUEST, static_cast<HTTP2ResponseFinishedStatus>(-1),
                                                MessageRequestObserverInterface::Status::INTERNAL_ERROR) },
                            { std::make_tuple(HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED, HTTP2ResponseFinishedStatus::COMPLETE,
                                              MessageRequestObserverInterface::Status::INTERNAL_ERROR) },
                            { std::make_tuple(HTTPResponseCode::SUCCESS_OK, HTTP2ResponseFinishedStatus::COMPLETE, MessageRequestObserverInterface::Status::SUCCESS) },
                            { std::make_tuple(HTTPResponseCode::SUCCESS_NO_CONTENT, HTTP2ResponseFinishedStatus::COMPLETE,
                                              MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT) },
                            { std::make_tuple(HTTPResponseCode::REDIRECTION_START_CODE, HTTP2ResponseFinishedStatus::COMPLETE,
                                              MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR) },
                            { std::make_tuple(HTTPResponseCode::REDIRECTION_END_CODE, HTTP2ResponseFinishedStatus::COMPLETE,
                                              MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR) },
                            { std::make_tuple(HTTPResponseCode::CLIENT_ERROR_BAD_REQUEST, HTTP2ResponseFinishedStatus::COMPLETE,
                                              MessageRequestObserverInterface::Status::BAD_REQUEST) },
                            { std::make_tuple(HTTPResponseCode::CLIENT_ERROR_FORBIDDEN, HTTP2ResponseFinishedStatus::COMPLETE,
                                              MessageRequestObserverInterface::Status::INVALID_AUTH) },
                            { std::make_tuple(HTTPResponseCode::SERVER_ERROR_INTERNAL, HTTP2ResponseFinishedStatus::COMPLETE,
                                              MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2) },
                            {std::make_tuple(-1, HTTP2ResponseFinishedStatus::COMPLETE, MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR) },
                        };
                    authorizeAndConnect();
                    std::vector<std::shared_ptr<TestMessageRequestObserver>> messageObservers;
                    unsigned messagesCount = messageResponseMap.size();
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                        auto messageObserver = std::make_shared<TestMessageRequestObserver>();
                        messageObservers.push_back(messageObserver);
                        messageReq->addObserver(messageObserver);
                        m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                        m_http2Transport->onRequestEnqueued();
                    }
                    unsigned int postsRequestsCount = 0;
                    for (unsigned int i = 0; i < messagesCount; i++) {
                        auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                        if (request) {
                            m_mockHttp2Connection->dequePostRequest();
                            postsRequestsCount++;
                            auto responseCode = std::get<0>(messageResponseMap[i]);
                            if (responseCode != NO_CALL) request->getSink()->onReceiveResponseCode(responseCode);
                            auto responseFinished = std::get<1>(messageResponseMap[i]);
                            request->getSink()->onResponseFinished(static_cast<HTTP2ResponseFinishedStatus>(responseFinished));
                        } else break;
                    }
                    ASSERT_EQ(postsRequestsCount, messagesCount);
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        if (messageObservers[messageNum]->m_status.waitFor(RESPONSE_TIMEOUT)) {
                            auto expectedMessageObserverStatus = std::get<2>(messageResponseMap[messageNum]);
                            ASSERT_EQ(messageObservers[messageNum]->m_status.getValue(), expectedMessageObserverStatus);
                        }
                    }
                }
                TEST_F(HTTP2TransportTest, test_onExceptionReceivedNon200Content) {
                    authorizeAndConnect();
                    shared_ptr<MessageRequest> messageReq = make_shared<MessageRequest>(TEST_MESSAGE, "");
                    auto messageObserver = std::make_shared<TestMessageRequestObserver>();
                    messageReq->addObserver(messageObserver);
                    m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                    m_http2Transport->onRequestEnqueued();
                    auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                    ASSERT_NE(request, nullptr);
                    request->getSink()->onReceiveResponseCode(HTTPResponseCode::SERVER_ERROR_INTERNAL);
                    request->getSink()->onReceiveData(NON_MIME_PAYLOAD.c_str(), NON_MIME_PAYLOAD.size());
                    request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                    ASSERT_TRUE(messageObserver->m_exception.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_EQ(messageObserver->m_exception.getValue(), NON_MIME_PAYLOAD);
                    ASSERT_TRUE(messageObserver->m_status.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_EQ(messageObserver->m_status.getValue(), MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                }
                TEST_F(HTTP2TransportTest, test_messageConsumerReceiveDirective) {
                    PromiseFuturePair<void> messagesAreConsumed;
                    unsigned int consumedMessageCount = 0;
                    vector<string> messages;
                    authorizeAndConnect();
                    /*EXPECT_CALL(*m_mockMessageConsumer, consumeMessage(_, _))
                        .WillRepeatedly(Invoke([&messagesAreConsumed, &consumedMessageCount, &messages](const string& contextId, const string& message) {
                            consumedMessageCount++;
                            messages.push_back(message);
                            if (consumedMessageCount == 2u) messagesAreConsumed.setValue();
                        }));*/
                    shared_ptr<MessageRequest> messageReq = make_shared<MessageRequest>(TEST_MESSAGE, "");
                    auto messageObserver = make_shared<TestMessageRequestObserver>();
                    messageReq->addObserver(messageObserver);
                    m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                    m_http2Transport->onRequestEnqueued();
                    auto eventStream = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                    ASSERT_NE(eventStream, nullptr);
                    eventStream->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_OK);
                    eventStream->getSink()->onReceiveHeaderLine(HTTP_BOUNDARY_HEADER);
                    eventStream->getSink()->onReceiveData(MIME_BODY_DIRECTIVE1.c_str(), MIME_BODY_DIRECTIVE1.size());
                    eventStream->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                    auto downchannelStream = m_mockHttp2Connection->getDownchannelRequest();
                    ASSERT_NE(downchannelStream, nullptr);
                    downchannelStream->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_OK);
                    downchannelStream->getSink()->onReceiveHeaderLine(HTTP_BOUNDARY_HEADER);
                    downchannelStream->getSink()->onReceiveData(MIME_BODY_DIRECTIVE2.c_str(), MIME_BODY_DIRECTIVE2.size());
                    ASSERT_TRUE(messagesAreConsumed.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_EQ(messages[0], DIRECTIVE1);
                    ASSERT_EQ(messages[1], DIRECTIVE2);
                }
                TEST_F(HTTP2TransportTest, test_onServerSideDisconnectOnDownchannelClosure) {
                    authorizeAndConnect();
                    std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                    m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                    m_http2Transport->onRequestEnqueued();
                    PromiseFuturePair<void> gotOnServerSideDisconnect;
                    auto setGotOnServerSideDisconnect = [&gotOnServerSideDisconnect] { gotOnServerSideDisconnect.setValue(); };
                    PromiseFuturePair<void> gotOnDisconnected;
                    auto setGotOnDisconnected = [&gotOnDisconnected] { gotOnDisconnected.setValue(); };
                    {
                        InSequence dummy;
                        EXPECT_CALL(*m_mockTransportObserver, onServerSideDisconnect(_))
                            .Times(1).WillOnce(InvokeWithoutArgs(setGotOnServerSideDisconnect));
                        /*EXPECT_CALL(*m_mockTransportObserver, onDisconnected(_, _))
                            .Times(1).WillOnce(InvokeWithoutArgs(setGotOnDisconnected));*/
                    }
                    auto eventStream = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                    ASSERT_NE(eventStream, nullptr);
                    auto downchannelStream = m_mockHttp2Connection->getDownchannelRequest();
                    downchannelStream->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                    ASSERT_TRUE(gotOnServerSideDisconnect.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_TRUE(gotOnDisconnected.waitFor(RESPONSE_TIMEOUT));
                }
                TEST_F(HTTP2TransportTest, test_messageRequestTimeoutPingRequest) {
                    authorizeAndConnect();
                    std::shared_ptr<MessageRequest> messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE, "");
                    m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                    m_http2Transport->onRequestEnqueued();
                    auto eventStream = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                    ASSERT_NE(eventStream, nullptr);
                    eventStream->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::TIMEOUT);
                    ASSERT_NE(m_mockHttp2Connection->waitForPingRequest(RESPONSE_TIMEOUT), nullptr);
                }
                TEST_F(HTTP2TransportTest, testTimer_networkInactivityPingRequest) {
                    static const std::chrono::seconds testInactivityTimeout = SHORT_DELAY;
                    static const unsigned expectedInactivityPingCount{3u};
                    static const std::chrono::seconds testInactivityTime = {testInactivityTimeout * expectedInactivityPingCount + SHORT_DELAY};
                    HTTP2Transport::Configuration cfg;
                    cfg.inactivityTimeout = testInactivityTimeout;
                    m_http2Transport = HTTP2Transport::create(m_mockAuthDelegate, TEST_AVS_GATEWAY_STRING, m_mockHttp2Connection, m_mockMessageConsumer,
                                                              m_attachmentManager, m_mockTransportObserver, m_mockPostConnectFactory,
                                                              m_synchronizedMessageRequestQueue, cfg, m_mockMetricRecorder, m_mockEventTracer);
                    authorizeAndConnect();
                    PromiseFuturePair<void> gotPings;
                    std::thread pingResponseThread([this, &gotPings]() {
                        unsigned pingCount{0};
                        while(pingCount < expectedInactivityPingCount) {
                            auto pingRequest = m_mockHttp2Connection->waitForPingRequest(RESPONSE_TIMEOUT);
                            if (!pingRequest) continue;
                            m_mockHttp2Connection->dequePingRequest();
                            pingRequest->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_NO_CONTENT);
                            pingRequest->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                            pingCount++;
                        }
                        gotPings.setValue();
                    });
                    ASSERT_TRUE(gotPings.waitFor(testInactivityTime));
                    pingResponseThread.join();
                }
                TEST_F(HTTP2TransportTest, testSlow_tearDownPingTimeout) {
                    const std::chrono::seconds testInactivityTimeout = SHORT_DELAY;
                    HTTP2Transport::Configuration cfg;
                    cfg.inactivityTimeout = testInactivityTimeout;
                    m_http2Transport = HTTP2Transport::create(m_mockAuthDelegate, TEST_AVS_GATEWAY_STRING, m_mockHttp2Connection, m_mockMessageConsumer,
                                                              m_attachmentManager, m_mockTransportObserver, m_mockPostConnectFactory, m_synchronizedMessageRequestQueue,
                                                              cfg, m_mockMetricRecorder, m_mockEventTracer);
                    authorizeAndConnect();
                    PromiseFuturePair<void> gotOnDisconnected;
                    auto setGotOnDisconnected = [&gotOnDisconnected] { gotOnDisconnected.setValue(); };
                    //EXPECT_CALL(*m_mockTransportObserver, onDisconnected(_, _)).Times(1).WillOnce(InvokeWithoutArgs(setGotOnDisconnected));
                    thread pingThread([this]() {
                        auto pingRequest = m_mockHttp2Connection->waitForPingRequest(RESPONSE_TIMEOUT);
                        ASSERT_NE(pingRequest, nullptr);
                        m_mockHttp2Connection->dequePingRequest();
                        pingRequest->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::TIMEOUT);
                    });
                    ASSERT_TRUE(gotOnDisconnected.waitFor(RESPONSE_TIMEOUT));
                    pingThread.join();
                }
                TEST_F(HTTP2TransportTest, testSlow_tearDownPingFailure) {
                    const seconds testInactivityTimeout = SHORT_DELAY;
                    HTTP2Transport::Configuration cfg;
                    cfg.inactivityTimeout = testInactivityTimeout;
                    m_http2Transport = HTTP2Transport::create(m_mockAuthDelegate, TEST_AVS_GATEWAY_STRING, m_mockHttp2Connection, m_mockMessageConsumer,
                                                              m_attachmentManager, m_mockTransportObserver, m_mockPostConnectFactory, m_synchronizedMessageRequestQueue,
                                                              cfg, m_mockMetricRecorder, m_mockEventTracer);
                    authorizeAndConnect();
                    PromiseFuturePair<void> gotOnDisconnected;
                    auto setGotOnDisconnected = [&gotOnDisconnected] { gotOnDisconnected.setValue(); };
                    //EXPECT_CALL(*m_mockTransportObserver, onDisconnected(_, _)).Times(1).WillOnce(InvokeWithoutArgs(setGotOnDisconnected));
                    std::thread pingThread([this]() {
                        auto pingRequest = m_mockHttp2Connection->waitForPingRequest(RESPONSE_TIMEOUT);
                        ASSERT_NE(pingRequest, nullptr);
                        m_mockHttp2Connection->dequePingRequest();
                        pingRequest->getSink()->onReceiveResponseCode(HTTPResponseCode::CLIENT_ERROR_BAD_REQUEST);
                        pingRequest->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                    });
                    ASSERT_TRUE(gotOnDisconnected.waitFor(RESPONSE_TIMEOUT));
                    pingThread.join();
                }
                TEST_F(HTTP2TransportTest, testSlow_avsStreamsLimit) {
                    const unsigned messagesCount = MAX_POST_STREAMS * 2;
                    authorizeAndConnect();
                    m_mockHttp2Connection->setResponseToPOSTRequests(HTTPResponseCode::SUCCESS_OK);
                    vector<shared_ptr<TestMessageRequestObserver>> messageObservers;
                    for (unsigned messageNum = 0; messageNum < messagesCount; messageNum++) {
                        shared_ptr<MessageRequest> messageReq = make_shared<MessageRequest>(TEST_MESSAGE + to_string(messageNum), "");
                        auto messageObserver = std::make_shared<TestMessageRequestObserver>();
                        messageObservers.push_back(messageObserver);
                        messageReq->addObserver(messageObserver);
                        m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                        m_http2Transport->onRequestEnqueued();
                    }
                    ASSERT_NE(m_mockHttp2Connection->getDownchannelRequest(RESPONSE_TIMEOUT), nullptr);
                    ASSERT_EQ(m_mockHttp2Connection->getPostRequestsNum(), MAX_POST_STREAMS);
                    unsigned int completed = 0;
                    shared_ptr<MockHTTP2Request> request;
                    while((request = m_mockHttp2Connection->dequePostRequest(RESPONSE_TIMEOUT)) != nullptr && completed < messagesCount) {
                        request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                        completed++;
                        this_thread::sleep_for(TEN_MILLISECOND_DELAY);
                    }
                    ASSERT_EQ(completed, messagesCount);
                    ASSERT_EQ(m_mockHttp2Connection->getMaxPostRequestsEnqueud(), MAX_POST_STREAMS);
                }
                TEST_F(HTTP2TransportTest, test_onPostConnectFailureInitiatesShutdownAndNotifiesObservers) {
                    InSequence dummy;
                    EXPECT_CALL(*m_mockPostConnectFactory, createPostConnect()).WillOnce(InvokeWithoutArgs([this] {
                        m_createPostConnectCalled.setValue();
                        return m_mockPostConnect;
                    }));
                    /*EXPECT_CALL(*m_mockPostConnect, doPostConnect(_, _))
                        .WillOnce(Invoke([this](shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver) {
                            m_doPostConnected.setValue(make_pair(postConnectSender, postConnectObserver));
                            postConnectObserver->onUnRecoverablePostConnectFailure();
                            return true;
                        }));*/
                    PromiseFuturePair<void> gotOnDisconnected;
                    /*EXPECT_CALL(*m_mockTransportObserver, onDisconnected(_, _))
                        .WillOnce(Invoke([this, &gotOnDisconnected](shared_ptr<TransportInterface> transport, ConnectionStatusObserverInterface::ChangedReason reason) {
                            gotOnDisconnected.setValue();
                            ASSERT_EQ(transport, m_http2Transport);
                            ASSERT_EQ(reason, ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR);
                        }));*/
                    m_http2Transport->connect();
                    ASSERT_TRUE(m_doPostConnected.waitFor(RESPONSE_TIMEOUT));
                    ASSERT_TRUE(gotOnDisconnected.waitFor(RESPONSE_TIMEOUT));
                }
                TEST_F(HTTP2TransportTest, test_eventTracerIsNotifiedForMessagesSent) {
                    EXPECT_CALL(*m_mockEventTracer, traceEvent(_)).WillOnce(Invoke([](const std::string& content) {
                        ASSERT_EQ(content, TEST_MESSAGE);
                    }));
                    authorizeAndConnect();
                    m_mockHttp2Connection->setResponseToPOSTRequests(HTTPResponseCode::SUCCESS_OK);
                    auto messageReq = make_shared<MessageRequest>(TEST_MESSAGE);
                    m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                    m_http2Transport->onRequestEnqueued();
                    auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                    if (request) {
                        m_mockHttp2Connection->dequePostRequest();
                        request->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_OK);
                        request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                    }
                }
                TEST_F(HTTP2TransportTest, test_eventTracerIsNotNotifiedForFailedMessages) {
                    EXPECT_CALL(*m_mockEventTracer, traceEvent(_)).Times(0);
                    ON_CALL(*m_mockHttp2Connection, createAndSendRequest(Property(&HTTP2RequestConfig::getId, StartsWith("AVSEvent-"))))
                        .WillByDefault(InvokeWithoutArgs(([]() { return nullptr; })));
                    authorizeAndConnect();
                    auto messageReq = std::make_shared<MessageRequest>(TEST_MESSAGE);
                    shared_ptr<TestMessageRequestObserver> observer = std::make_shared<TestMessageRequestObserver>();
                    messageReq->addObserver(observer);
                    m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                    m_http2Transport->onRequestEnqueued();
                    observer->m_status.waitFor(RESPONSE_TIMEOUT);
                }
                TEST_F(HTTP2TransportTest, test_eventTracerNotifiedForEmptyMessageContent) {
                    EXPECT_CALL(*m_mockEventTracer, traceEvent(_)).WillOnce(Invoke([](const std::string& content) {
                        ASSERT_EQ(content, "");
                    }));
                    authorizeAndConnect();
                    m_mockHttp2Connection->setResponseToPOSTRequests(HTTPResponseCode::SUCCESS_OK);
                    auto messageReq = make_shared<MessageRequest>("");
                    m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                    m_http2Transport->onRequestEnqueued();
                    auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                    if (request) {
                        m_mockHttp2Connection->dequePostRequest();
                        request->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_OK);
                        request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                    }
                }
                TEST_F(HTTP2TransportTest, test_eventTracerIsNotifiedForMultipleMessages) {
                    EXPECT_CALL(*m_mockEventTracer, traceEvent(_)).WillRepeatedly(Invoke([](const string& content) {
                        static int i = 0;
                        ASSERT_EQ(content, TEST_MESSAGE + to_string(i++));
                    }));
                    authorizeAndConnect();
                    m_mockHttp2Connection->setResponseToPOSTRequests(HTTPResponseCode::SUCCESS_OK);
                    int messagesCount = 10;
                    for (int messageNum = 0; messageNum < messagesCount; messageNum++) {
                        auto messageReq = make_shared<MessageRequest>(TEST_MESSAGE + std::to_string(messageNum));
                        m_synchronizedMessageRequestQueue->enqueueRequest(messageReq);
                        m_http2Transport->onRequestEnqueued();
                    }
                    int postsRequestsCount = 0;
                    for (int i = 0; i < messagesCount; i++) {
                        auto request = m_mockHttp2Connection->waitForPostRequest(RESPONSE_TIMEOUT);
                        if (request) {
                            m_mockHttp2Connection->dequePostRequest();
                            postsRequestsCount++;
                            request->getSink()->onReceiveResponseCode(HTTPResponseCode::SUCCESS_OK);
                            request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                        } else break;
                    }
                    ASSERT_EQ(postsRequestsCount, messagesCount);
                }
            }
        }
    }
}