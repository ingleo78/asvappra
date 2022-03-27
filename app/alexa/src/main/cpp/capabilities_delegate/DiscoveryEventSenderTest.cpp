#include <memory>
#include <thread>
#include <gmock/gmock.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/PostConnectOperationInterface.h>
#include <json/JSONUtils.h>
#include <util/PromiseFuturePair.h>
#include "DiscoveryEventSender.h"
#include "MockAuthDelegate.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace test {
            using namespace testing;
            using namespace avs;
            using namespace utils;
            using namespace json;
            using namespace rapidjson;
            using namespace sdkInterfaces::test;
            using namespace json::jsonUtils;
            static const string DISCOVERY_NAMESPACE = "Alexa.Discovery";
            static const string ADD_OR_UPDATE_REPORT_NAME = "AddOrUpdateReport";
            static const string DELETE_REPORT_NAME = "DeleteReport";
            static const string DISCOVERY_PAYLOAD_VERSION = "3";
            static const string SCOPE_KEY = "scope";
            static const string TYPE_KEY = "type";
            static const string BEARER_TOKEN_KEY = "bearerToken";
            static const string TOKEN_KEY = "token";
            static const string ENDPOINTS_KEY = "endpoints";
            static const string ENDPOINTID_KEY = "endpointId";
            static const string EVENT_KEY = "event";
            static const string HEADER_KEY = "header";
            static const string NAMESPACE_KEY = "namespace";
            static const string NAME_KEY = "name";
            static const string PAYLOAD_VERSION_KEY = "payloadVersion";
            static const string EVENT_CORRELATION_TOKEN_KEY = "eventCorrelationToken";
            static const string PAYLOAD_KEY = "payload";
            static const string TEST_AUTH_TOKEN = "TEST_AUTH_TOKEN";
            static const string TEST_ENDPOINT_ID_1 = "1";
            static const string TEST_ENDPOINT_ID_2 = "2";
            static const unordered_map<string, string> TEST_ADD_OR_UPDATE_ENDPOINTS = {{TEST_ENDPOINT_ID_1, R"({"endpointId":")" + TEST_ENDPOINT_ID_1 + R"("})"}};
            static const unordered_map<string, std::string> TEST_DELETE_ENDPOINTS = {{TEST_ENDPOINT_ID_2, R"({"endpointId":")" + TEST_ENDPOINT_ID_2 + R"("})"}};
            static const vector<string> EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS = {TEST_ENDPOINT_ID_1};
            static const vector<string> EXPECTED_DELETE_ENDPOINT_IDS = {TEST_ENDPOINT_ID_2};
            static const string TEST_EVENT_CORRELATION_TOKEN = "TEST_EVENT_CORRELATION_TOKEN";
            static const int TEST_RETRY_COUNT = 2;
            static const int MAX_ENDPOINTS_PER_EVENT = 300;
            struct EventData {
                string namespaceString;
                string nameString;
                string payloadVersionString;
                string eventCorrelationTokenString;
                vector<string> endpointIdsInPayload;
                string authToken;
            };
            bool parseAuthToken(const string& payloadString, EventData* eventData) {
                string scopeString;
                if (!retrieveValue(payloadString, "scope", &scopeString)) return false;
                string bearerToken;
                if (!retrieveValue(scopeString, "type", &bearerToken)) return false;
                if (bearerToken != "BearerToken") return false;
                if (!retrieveValue(scopeString, "token", &eventData->authToken)) return false;
                return true;
            }
            bool parseEndpointsIds(const string& payloadString, EventData* eventData) {
                Document document;
                document.Parse(payloadString.data());
                if (document.HasParseError()) return false;
                rapidjson::Value _document{document.GetString(), strlen(document.GetString())};
                if (!jsonArrayExists(_document, ENDPOINTS_KEY)) return false;
                auto endpointsJsonArray = document.FindMember(ENDPOINTS_KEY.c_str())->value.GetArray();
                for (SizeType i = 0; i < endpointsJsonArray.Size(); ++i) {
                    string endpointId;
                    if (!retrieveValue(endpointsJsonArray[i].GetString(), ENDPOINTID_KEY, &endpointId)) return false;
                    eventData->endpointIdsInPayload.push_back(endpointId);
                }
                return true;
            }
            bool parseEventJson(const string& eventJson, EventData* eventData) {
                string eventString;
                if (!retrieveValue(eventJson, EVENT_KEY, &eventString)) return false;
                string headerString;
                if (!retrieveValue(eventString, HEADER_KEY, &headerString)) return false;
                if (!retrieveValue(headerString, NAMESPACE_KEY, &eventData->namespaceString)) return false;
                if (!retrieveValue(headerString, NAME_KEY, &eventData->nameString)) return false;
                if (!retrieveValue(headerString, PAYLOAD_VERSION_KEY, &eventData->payloadVersionString)) return false;
                if (!retrieveValue(headerString, EVENT_CORRELATION_TOKEN_KEY, &eventData->eventCorrelationTokenString)) return false;
                string payloadString;
                if (!retrieveValue(eventString, PAYLOAD_KEY, &payloadString)) return false;
                if (!parseAuthToken(payloadString, eventData)) return false;
                if (!parseEndpointsIds(payloadString, eventData)) return false;
                return true;
            }
            void validateDiscoveryEvent(const EventData& eventData, const string& eventName, const vector<string>& endpointIds = vector<string>()) {
                ASSERT_EQ(eventData.namespaceString, DISCOVERY_NAMESPACE);
                ASSERT_EQ(eventData.nameString, eventName);
                ASSERT_EQ(eventData.payloadVersionString, DISCOVERY_PAYLOAD_VERSION);
                ASSERT_EQ(eventData.authToken, TEST_AUTH_TOKEN);
                if (!endpointIds.empty()) { ASSERT_EQ(eventData.endpointIdsInPayload, endpointIds); }
            }
            class MockDiscoveryStatusObserver : public DiscoveryStatusObserverInterface {
            public:
                //MOCK_METHOD2(onDiscoveryCompleted, void(const unordered_map<string, string>&, const unordered_map<string, string>&));
                MOCK_METHOD1(onDiscoveryFailure, void(MessageRequestObserverInterface::Status));
            };
            class DiscoveryEventSenderTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
            protected:
                void validateCallsToAuthDelegate(const shared_ptr<DiscoveryEventSenderInterface>& discoveryEventSender, bool expectAuthFailure);
                shared_ptr<MockMessageSender> m_mockMessageSender;
                shared_ptr<MockAuthDelegate> m_mockAuthDelegate;
                shared_ptr<MockDiscoveryStatusObserver> m_mockDiscoveryStatusObserver;
                shared_ptr<DiscoveryEventSenderInterface> m_discoveryEventSender;
            };
            void DiscoveryEventSenderTest::SetUp() {
                m_mockMessageSender = make_shared<StrictMock<MockMessageSender>>();
                m_mockAuthDelegate = make_shared<StrictMock<MockAuthDelegate>>();
                //m_mockDiscoveryStatusObserver = make_shared<StrictMock<MockDiscoveryStatusObserver>>();
                m_discoveryEventSender = DiscoveryEventSender::create(TEST_ADD_OR_UPDATE_ENDPOINTS, TEST_DELETE_ENDPOINTS, m_mockAuthDelegate);
                m_discoveryEventSender->addDiscoveryStatusObserver(m_mockDiscoveryStatusObserver);
            }
            void DiscoveryEventSenderTest::TearDown() {
                m_discoveryEventSender->stop();
                m_discoveryEventSender->removeDiscoveryStatusObserver(m_mockDiscoveryStatusObserver);
                EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_mockAuthDelegate.get()));
            }
            void DiscoveryEventSenderTest::validateCallsToAuthDelegate(
                const std::shared_ptr<DiscoveryEventSenderInterface>& discoveryEventSender,
                bool expectAuthFailure = false) {
                EXPECT_CALL(*m_mockAuthDelegate, addAuthObserver(_))
                    .WillRepeatedly(Invoke([](const std::shared_ptr<AuthObserverInterface>& observer) {
                        observer->onAuthStateChange(AuthObserverInterface::State::REFRESHED, AuthObserverInterface::Error::SUCCESS);
                    }));
                EXPECT_CALL(*m_mockAuthDelegate, removeAuthObserver(_)).Times(AtLeast(1));
                EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).WillRepeatedly(Return(TEST_AUTH_TOKEN));
                if (expectAuthFailure) { EXPECT_CALL(*m_mockAuthDelegate, onAuthFailure(TEST_AUTH_TOKEN)); }
            }
            TEST_F(DiscoveryEventSenderTest, test_createWithInvalidParams) {
                auto instance = DiscoveryEventSender::create(TEST_ADD_OR_UPDATE_ENDPOINTS, TEST_DELETE_ENDPOINTS, nullptr);
                ASSERT_EQ(instance, nullptr);
                unordered_map<string, string> addOrUpdateEndpoints, deleteEndpoints;
                instance = DiscoveryEventSender::create(addOrUpdateEndpoints, deleteEndpoints, m_mockAuthDelegate);
                ASSERT_EQ(instance, nullptr);
            }
            TEST_F(DiscoveryEventSenderTest, test_sendsDiscoveryEvents) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                auto sendAddOrUpdateReport = [this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    m_discoveryEventSender->onAlexaEventProcessedReceived(eventData.eventCorrelationTokenString);
                };
                auto sendDeleteReport = [](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, DELETE_REPORT_NAME, EXPECTED_DELETE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                };
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(sendAddOrUpdateReport)).WillOnce(Invoke(sendDeleteReport));
                //EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryCompleted(TEST_ADD_OR_UPDATE_ENDPOINTS, TEST_DELETE_ENDPOINTS)).WillOnce(Return());
                ASSERT_TRUE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_sendDiscoveryEventsFailsWithNullMessageSender) {
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(nullptr));
            }
            static int getExpectedNumberOfDiscoveryEvents(int numEndpoints) {
                return numEndpoints / MAX_ENDPOINTS_PER_EVENT + ((numEndpoints % MAX_ENDPOINTS_PER_EVENT) != 0);
            }
            TEST_F(DiscoveryEventSenderTest, test_sendDiscoveryEventsSplitsEventsWithMaxEndpoints) {
                unordered_map<string, string> addOrUpdateReportEndpoints, deleteReportEndpoints;
                string endpointIdPrefix = "ENDPOINT_ID_";
                int testNumAddOrUpdateReportEndpoints = 1400;
                int testNumDeleteReportEndpoints = 299;
                for (int i = 1; i <= testNumAddOrUpdateReportEndpoints; ++i) {
                    string endpointId = endpointIdPrefix + to_string(i);
                    string endpointIdConfig = "{\"endpointId\":\"" + to_string(i) + "\"}";
                    addOrUpdateReportEndpoints.insert({endpointId, endpointIdConfig});
                }
                for (int i = 1; i <= testNumDeleteReportEndpoints; ++i) {
                    string endpointId = endpointIdPrefix + to_string(i);
                    string endpointIdConfig = "{\"endpointId\":\"" + to_string(i) + "\"}";
                    deleteReportEndpoints.insert({endpointId, endpointIdConfig});
                }
                auto discoveryEventSender = DiscoveryEventSender::create(addOrUpdateReportEndpoints, deleteReportEndpoints, m_mockAuthDelegate);
                discoveryEventSender->addDiscoveryStatusObserver(m_mockDiscoveryStatusObserver);
                validateCallsToAuthDelegate(discoveryEventSender);
                discoveryEventSender->addDiscoveryStatusObserver(m_mockDiscoveryStatusObserver);
                auto handleAddOrUpdateReport = [discoveryEventSender](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    discoveryEventSender->onAlexaEventProcessedReceived(eventData.eventCorrelationTokenString);
                };
                auto handleDeleteReport = [](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, DELETE_REPORT_NAME);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                };
                int expectedNumOfAddOrUpdateReportEvents = getExpectedNumberOfDiscoveryEvents(testNumAddOrUpdateReportEndpoints);
                int expectedNumOfDeleteReportEvents = getExpectedNumberOfDiscoveryEvents(testNumDeleteReportEndpoints);
                {
                    InSequence s;
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(expectedNumOfAddOrUpdateReportEvents).WillRepeatedly(Invoke(handleAddOrUpdateReport));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(expectedNumOfDeleteReportEvents).WillRepeatedly(Invoke(handleDeleteReport));
                }
                //EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryCompleted(addOrUpdateReportEndpoints, deleteReportEndpoints)).WillOnce(Return());
                EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).WillRepeatedly(Return(TEST_AUTH_TOKEN));
                ASSERT_TRUE(discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
                discoveryEventSender->removeDiscoveryStatusObserver(m_mockDiscoveryStatusObserver);
            }
            TEST_F(DiscoveryEventSenderTest, test_deleteReportEventReceives4xxResponse) {
                validateCallsToAuthDelegate(m_discoveryEventSender, true);
                auto handleAddOrUpdateReport = [this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    m_discoveryEventSender->onAlexaEventProcessedReceived(eventData.eventCorrelationTokenString);
                };
                auto handleDeleteReport = [](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, DELETE_REPORT_NAME, EXPECTED_DELETE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::INVALID_AUTH);
                };
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(handleAddOrUpdateReport)).WillOnce(Invoke(handleDeleteReport));
                EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryFailure(MessageRequestObserverInterface::Status::INVALID_AUTH)).WillOnce(Return());
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_sendDiscoveryEventsWhenAuthTokenIsEmpty) {
                EXPECT_CALL(*m_mockAuthDelegate, addAuthObserver(_)).WillOnce(InvokeWithoutArgs([this]() {
                    m_discoveryEventSender->onAuthStateChange(AuthObserverInterface::State::REFRESHED, AuthObserverInterface::Error::SUCCESS);
                }));
                EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).WillRepeatedly(Return(""));
                EXPECT_CALL(*m_mockAuthDelegate, removeAuthObserver(_)).WillOnce(Return());
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_addOrUpdateReportReceives4xxResponse) {
                validateCallsToAuthDelegate(m_discoveryEventSender, true);
                auto handleAddOrUpdateReport = [this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::INVALID_AUTH);
                    m_discoveryEventSender->onAlexaEventProcessedReceived(eventData.eventCorrelationTokenString);
                };
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(handleAddOrUpdateReport));
                EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryFailure(MessageRequestObserverInterface::Status::INVALID_AUTH)).WillOnce(Return());
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_retriesWhenAddOrUpdateReportReceives5xxResponse) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                int retryCount = 0;
                auto handleAddOrUpdateReport = [this, &retryCount](std::shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                    static int count = 0;
                    count++;
                    if (TEST_RETRY_COUNT == count) {
                        retryCount = TEST_RETRY_COUNT;
                        std::thread localThread([this] { m_discoveryEventSender->stop(); });
                        if (localThread.joinable()) localThread.join();
                    }
                };
                EXPECT_CALL(*m_mockDiscoveryStatusObserver,onDiscoveryFailure(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2)).WillRepeatedly(Return());
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillRepeatedly(Invoke(handleAddOrUpdateReport));
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
                ASSERT_EQ(retryCount, TEST_RETRY_COUNT);
            }
            TEST_F(DiscoveryEventSenderTest, test_addOrUpdateRetriesThenSuccessfulResponse) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                int retryCount = 0;
                auto handleAddOrUpdateReport = [this, &retryCount](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    if (retryCount < 2) {
                        request->sendCompleted(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                        retryCount++;
                    } else {
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                        m_discoveryEventSender->onAlexaEventProcessedReceived(eventData.eventCorrelationTokenString);
                    }
                };
                auto handleDeleteReport = [](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, DELETE_REPORT_NAME, EXPECTED_DELETE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                };
                EXPECT_CALL(*m_mockDiscoveryStatusObserver,onDiscoveryFailure(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2)).WillRepeatedly(Return());
                //EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryCompleted(TEST_ADD_OR_UPDATE_ENDPOINTS, TEST_DELETE_ENDPOINTS)).WillOnce(Return());
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(handleAddOrUpdateReport)).WillOnce(Invoke(handleAddOrUpdateReport))
                    .WillOnce(Invoke(handleAddOrUpdateReport)).WillOnce(Invoke(handleDeleteReport));
                ASSERT_TRUE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_deleteReportRetriesThenSuccessfulResponse) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                auto handleAddOrUpdateReport = [this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    m_discoveryEventSender->onAlexaEventProcessedReceived(eventData.eventCorrelationTokenString);
                };
                int retryCount = 0;
                auto handleDeleteReport = [&retryCount](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, DELETE_REPORT_NAME, EXPECTED_DELETE_ENDPOINT_IDS);
                    if (retryCount < 2) {
                        request->sendCompleted(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                        retryCount++;
                    } else request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                };
                EXPECT_CALL(*m_mockDiscoveryStatusObserver,onDiscoveryFailure(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2)).WillRepeatedly(Return());
                //EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryCompleted(TEST_ADD_OR_UPDATE_ENDPOINTS, TEST_DELETE_ENDPOINTS)).WillOnce(Return());
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(handleAddOrUpdateReport)).WillOnce(Invoke(handleDeleteReport))
                    .WillOnce(Invoke(handleDeleteReport)).WillOnce(Invoke(handleDeleteReport));
                ASSERT_TRUE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_retriesWhenEventProcessedDirectiveIsNotReceived) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                int retryCount = 0;
                auto handleAddOrUpdateReport = [&retryCount, this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    static int count = 0;
                    count++;
                    if (TEST_RETRY_COUNT == count) {
                        retryCount = TEST_RETRY_COUNT;
                        thread localThread([this] { m_discoveryEventSender->stop(); });
                        if (localThread.joinable()) localThread.join();
                    }
                };
                EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryFailure(MessageRequestObserverInterface::Status::TIMEDOUT)).WillRepeatedly(Return());
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillRepeatedly(Invoke(handleAddOrUpdateReport));
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_retriesWhenInvalidEventProcessedDirectiveIsReceived) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                int retryCount = 0;
                auto handleAddOrUpdateReport = [&retryCount, this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    m_discoveryEventSender->onAlexaEventProcessedReceived(TEST_EVENT_CORRELATION_TOKEN);
                    static int count = 0;
                    count++;
                    if (TEST_RETRY_COUNT == count) {
                        retryCount = TEST_RETRY_COUNT;
                        std::thread localThread([this] { m_discoveryEventSender->stop(); });
                        if (localThread.joinable()) localThread.join();
                    }
                };
                EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryFailure(MessageRequestObserverInterface::Status::TIMEDOUT)).WillRepeatedly(Return());
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillRepeatedly(Invoke(handleAddOrUpdateReport));
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
                ASSERT_EQ(retryCount, TEST_RETRY_COUNT);
            }
            TEST_F(DiscoveryEventSenderTest, test_stopWhenWaitingOnEventProcessedDirective) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                auto handleAddOrUpdateReport = [this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    validateDiscoveryEvent(eventData, ADD_OR_UPDATE_REPORT_NAME, EXPECTED_ADD_OR_UPDATE_ENDPOINT_IDS);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    std::thread localThread([this] { m_discoveryEventSender->stop(); });
                    if (localThread.joinable()) localThread.join();
                };
                EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryFailure(MessageRequestObserverInterface::Status::CANCELED)).WillRepeatedly(Return());
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(handleAddOrUpdateReport));
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_stopWhenAuthTokenIsRequested) {
                EXPECT_CALL(*m_mockAuthDelegate, addAuthObserver(_)).WillOnce(InvokeWithoutArgs([this]() {
                    m_discoveryEventSender->onAuthStateChange(AuthObserverInterface::State::UNINITIALIZED, AuthObserverInterface::Error::SUCCESS);
                    EXPECT_CALL(*m_mockAuthDelegate, removeAuthObserver(_));
                    this_thread::sleep_for(milliseconds(200));
                    thread localThread([this] { m_discoveryEventSender->stop(); });
                    if (localThread.joinable()) localThread.join();
                }));
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
            TEST_F(DiscoveryEventSenderTest, test_sendDiscoveryEventsFailsWhenCalledTwice) {
                validateCallsToAuthDelegate(m_discoveryEventSender);
                auto sendAddOrUpdateReport = [this](shared_ptr<MessageRequest> request) {
                    EventData eventData;
                    ASSERT_TRUE(parseEventJson(request->getJsonContent(), &eventData));
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    m_discoveryEventSender->onAlexaEventProcessedReceived(eventData.eventCorrelationTokenString);
                };
                auto sendDeleteReport = [](shared_ptr<MessageRequest> request) {
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                };
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(sendAddOrUpdateReport)).WillOnce(Invoke(sendDeleteReport));
                //EXPECT_CALL(*m_mockDiscoveryStatusObserver, onDiscoveryCompleted(TEST_ADD_OR_UPDATE_ENDPOINTS, TEST_DELETE_ENDPOINTS)).WillOnce(Return());
                ASSERT_TRUE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
                ASSERT_FALSE(m_discoveryEventSender->sendDiscoveryEvents(m_mockMessageSender));
            }
        }
    }
}