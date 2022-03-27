#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sdkinterfaces/CapabilitiesObserverInterface.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <json/JSONUtils.h>
#include <util/WaitEvent.h>
#include "Storage/CapabilitiesDelegateStorageInterface.h"
#include "Utils/DiscoveryUtils.h"
#include "CapabilitiesDelegate.h""
#include "PostConnectCapabilitiesPublisher.h"
#include "MockAuthDelegate.h"
#include "MockCapabilitiesObserver.h"
#include "MockCapabilitiesStorage.h"
#include "MockDiscoveryEventSender.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace test {
            using namespace chrono;
            using namespace avs;
            using namespace sdkInterfaces::endpoints;
            using namespace sdkInterfaces::test;
            using namespace json::jsonUtils;
            using namespace capabilitiesDelegate::utils;
            using namespace rapidjson;
            using namespace testing;
            static const string TEST_AUTH_TOKEN = "TEST_AUTH_TOKEN";
            static const string EVENT_KEY = "event";
            static const string HEADER_KEY = "header";
            static const string EVENT_CORRELATION_TOKEN_KEY = "eventCorrelationToken";
            static const seconds MY_WAIT_TIMEOUT{5};
            struct EventData {
                string namespaceString;
                string nameString;
                string payloadVersionString;
                string eventCorrelationTokenString;
                vector<std::string> endpointIdsInPayload;
                string authToken;
            };
            AVSDiscoveryEndpointAttributes createEndpointAttributes(const string endpointId = "TEST_ENDPOINT_ID") {
                AVSDiscoveryEndpointAttributes attributes;
                attributes.endpointId = endpointId;
                attributes.description = "TEST_DESCRIPTION";
                attributes.manufacturerName = "TEST_MANUFACTURER_NAME";
                attributes.displayCategories = {"TEST_DISPLAY_CATEGORY"};
                return attributes;
            }
            CapabilityConfiguration createCapabilityConfiguration() {
                return CapabilityConfiguration("TEST_TYPE", "TEST_INTERFACE", "TEST_VERSION");
            }
            class CapabilitiesDelegateTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
            protected:
                void validateAuthDelegate();
                void addEndpoint(AVSDiscoveryEndpointAttributes attributes, CapabilityConfiguration configuration);
                void getEventCorrelationTokenString(shared_ptr<MessageRequest> request, string& eventCorrelationTokenString);
                shared_ptr<MockAuthDelegate> m_mockAuthDelegate;
                shared_ptr<MockCapabilitiesDelegateStorage> m_mockCapabilitiesStorage;
                shared_ptr<MockCapabilitiesObserver> m_mockCapabilitiesObserver;
                shared_ptr<CustomerDataManager> m_dataManager;
                shared_ptr<MockMessageSender> m_mockMessageSender;
                shared_ptr<CapabilitiesDelegate> m_capabilitiesDelegate;
            };
            void CapabilitiesDelegateTest::SetUp() {
                m_mockCapabilitiesStorage = make_shared<StrictMock<MockCapabilitiesDelegateStorage>>();
                m_mockAuthDelegate = make_shared<StrictMock<MockAuthDelegate>>();
                m_mockCapabilitiesObserver = make_shared<StrictMock<MockCapabilitiesObserver>>();
                m_dataManager = make_shared<registrationManager::CustomerDataManager>();
                m_mockMessageSender = make_shared<StrictMock<MockMessageSender>>();
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).WillOnce(Return(true));
                m_capabilitiesDelegate = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                ASSERT_NE(m_capabilitiesDelegate, nullptr);
                m_mockCapabilitiesObserver = make_shared<StrictMock<MockCapabilitiesObserver>>();
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                                     vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, std::vector<std::string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, std::vector<std::string>{});
                    }));
                m_capabilitiesDelegate->addCapabilitiesObserver(m_mockCapabilitiesObserver);
                m_capabilitiesDelegate->setMessageSender(m_mockMessageSender);
            }
            void CapabilitiesDelegateTest::TearDown() {
                m_capabilitiesDelegate->shutdown();
            }
            void CapabilitiesDelegateTest::validateAuthDelegate() {
                EXPECT_CALL(*m_mockAuthDelegate, addAuthObserver(_))
                    .WillRepeatedly(Invoke([](const std::shared_ptr<AuthObserverInterface>& observer) {
                        observer->onAuthStateChange(AuthObserverInterface::State::REFRESHED, AuthObserverInterface::Error::SUCCESS);
                    }));
                EXPECT_CALL(*m_mockAuthDelegate, removeAuthObserver(_)).Times(AtLeast(1));
                EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).WillRepeatedly(Return(TEST_AUTH_TOKEN));
            }
            void CapabilitiesDelegateTest::getEventCorrelationTokenString(shared_ptr<MessageRequest> request, string& eventCorrelationTokenString) {
                auto eventJson = request->getJsonContent();
                string eventString;
                retrieveValue(eventJson, EVENT_KEY, &eventString);
                string headerString;
                retrieveValue(eventString, HEADER_KEY, &headerString);
                retrieveValue(headerString, EVENT_CORRELATION_TOKEN_KEY, &eventCorrelationTokenString);
            }
            void CapabilitiesDelegateTest::addEndpoint(AVSDiscoveryEndpointAttributes attributes, CapabilityConfiguration configuration) {
                WaitEvent e;
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                                                           ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke([this](std::shared_ptr<MessageRequest> request) {
                    string eventCorrelationTokenString;
                    getEventCorrelationTokenString(request, eventCorrelationTokenString);
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    m_capabilitiesDelegate->onAlexaEventProcessedReceived(eventCorrelationTokenString);
                }));
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(_)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, erase((unordered_map<string, string>{}))).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(CapabilitiesObserverInterface::State::SUCCESS, CapabilitiesObserverInterface::Error::SUCCESS, _, _))
                    .Times(1).WillOnce(Invoke([&](CapabilitiesObserverInterface::State state, CapabilitiesObserverInterface::Error error, vector<string> addedEndpoints,
                    vector<string> deletedEndpoints) { e.wakeUp(); }));
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, {configuration}));
                ASSERT_TRUE(e.wait(MY_WAIT_TIMEOUT));
            }
            TEST_F(CapabilitiesDelegateTest, test_createMethodWithInvalidParameters) {
                auto instance = CapabilitiesDelegate::create(nullptr, m_mockCapabilitiesStorage, m_dataManager);
                ASSERT_EQ(instance, nullptr);
                instance = CapabilitiesDelegate::create(m_mockAuthDelegate, nullptr, m_dataManager);
                ASSERT_EQ(instance, nullptr);
                instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, nullptr);
                ASSERT_EQ(instance, nullptr);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).WillOnce(Return(true));
                instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                ASSERT_NE(instance, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_init) {
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).WillOnce(Return(false));
                EXPECT_CALL(*m_mockCapabilitiesStorage, createDatabase()).Times(1).WillOnce(Return(false));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                ASSERT_EQ(instance, nullptr);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).WillOnce(Return(false));
                EXPECT_CALL(*m_mockCapabilitiesStorage, createDatabase()).WillOnce(Return(true));
                instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                ASSERT_NE(instance, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_invalidateCapabilities) {
                EXPECT_CALL(*m_mockCapabilitiesStorage, clearDatabase()).WillOnce(Return(true));
                m_capabilitiesDelegate->invalidateCapabilities();
            }
            TEST_F(CapabilitiesDelegateTest, test_clearData) {
                EXPECT_CALL(*m_mockCapabilitiesStorage, clearDatabase()).WillOnce(Return(true));
                m_capabilitiesDelegate->clearData();
            }
            TEST_F(CapabilitiesDelegateTest, test_shutdown) {
                auto discoveryEventSender = std::make_shared<StrictMock<MockDiscoveryEventSender>>();
                EXPECT_CALL(*discoveryEventSender, addDiscoveryStatusObserver(_))
                    .WillOnce(Invoke([this](const std::shared_ptr<DiscoveryStatusObserverInterface>& observer) {
                        EXPECT_EQ(observer, m_capabilitiesDelegate);
                    }));
                m_capabilitiesDelegate->addDiscoveryEventSender(discoveryEventSender);
                EXPECT_CALL(*discoveryEventSender, removeDiscoveryStatusObserver(_))
                    .WillOnce(Invoke([this](const std::shared_ptr<DiscoveryStatusObserverInterface>& observer) {
                        EXPECT_EQ(observer, m_capabilitiesDelegate);
                    }));
                EXPECT_CALL(*discoveryEventSender, stop()).Times(1);
                m_capabilitiesDelegate->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_addCapabilitiesObserver) {
                m_capabilitiesDelegate->addCapabilitiesObserver(nullptr);
                auto mockObserver = std::make_shared<StrictMock<MockCapabilitiesObserver>>();
                EXPECT_CALL(*mockObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                                     vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                m_capabilitiesDelegate->addCapabilitiesObserver(mockObserver);
                m_capabilitiesDelegate->addCapabilitiesObserver(mockObserver);
            }
            TEST_F(CapabilitiesDelegateTest, test_onDiscoveryCompleted) {
                unordered_map<string, string> addOrUpdateReportEndpoints = {{"add_1", "1"}, {"update_1", "2"}};
                unordered_map<string, string> deleteReportEndpoints = {{"delete_1", "1"}};
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(addOrUpdateReportEndpoints)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, erase(deleteReportEndpoints)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _)).WillOnce(Invoke([](CapabilitiesObserverInterface::State newState,
                    CapabilitiesObserverInterface::Error newError, vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::SUCCESS);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::SUCCESS);
                        sort(addOrUpdateReportEndpointIdentifiers.begin(), addOrUpdateReportEndpointIdentifiers.end());
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, (vector<string>{"add_1", "update_1"}));
                        EXPECT_EQ(deleteReportEndpointIdentifiers, (vector<string>{"delete_1"}));
                    }));
                m_capabilitiesDelegate->onDiscoveryCompleted(addOrUpdateReportEndpoints, deleteReportEndpoints);
                m_capabilitiesDelegate->removeCapabilitiesObserver(m_mockCapabilitiesObserver);
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(addOrUpdateReportEndpoints)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, erase(deleteReportEndpoints)).WillOnce(Return(true));
                m_capabilitiesDelegate->onDiscoveryCompleted(addOrUpdateReportEndpoints, deleteReportEndpoints);
            }
            TEST_F(CapabilitiesDelegateTest, test_onDiscoveryCompletedButStorageFails) {
                unordered_map<string, string> addOrUpdateReportEndpoints = {{"add_1", "1"}, {"update_1", "2"}};
                unordered_map<string, string> deleteReportEndpoints = {{"delete_1", "1"}};
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(addOrUpdateReportEndpoints)).WillOnce(Return(false));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _)).WillOnce(Invoke([](CapabilitiesObserverInterface::State newState,
                    CapabilitiesObserverInterface::Error newError, vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::FATAL_ERROR);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNKNOWN_ERROR);
                        sort(addOrUpdateReportEndpointIdentifiers.begin(), addOrUpdateReportEndpointIdentifiers.end());
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, (vector<string>{"add_1", "update_1"}));
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{"delete_1"});
                    }));
                m_capabilitiesDelegate->onDiscoveryCompleted(addOrUpdateReportEndpoints, deleteReportEndpoints);
            }
            TEST_F(CapabilitiesDelegateTest, test_onDiscoveryFailure) {
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _)).WillOnce(Invoke([](CapabilitiesObserverInterface::State newState,
                    CapabilitiesObserverInterface::Error newError, vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::RETRIABLE_ERROR);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::SERVER_INTERNAL_ERROR);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                m_capabilitiesDelegate->onDiscoveryFailure(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                    vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::FATAL_ERROR);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::FORBIDDEN);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                m_capabilitiesDelegate->onDiscoveryFailure(MessageRequestObserverInterface::Status::INVALID_AUTH);
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _)).WillOnce(Invoke([](CapabilitiesObserverInterface::State newState,
                    CapabilitiesObserverInterface::Error newError, vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::FATAL_ERROR);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::BAD_REQUEST);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                m_capabilitiesDelegate->onDiscoveryFailure(MessageRequestObserverInterface::Status::BAD_REQUEST);
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _)).WillOnce(Invoke([](CapabilitiesObserverInterface::State newState,
                    CapabilitiesObserverInterface::Error newError, vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::RETRIABLE_ERROR);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNKNOWN_ERROR);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                m_capabilitiesDelegate->onDiscoveryFailure(MessageRequestObserverInterface::Status::THROTTLED);
            }
            TEST_F(CapabilitiesDelegateTest, test_addOrUpdateEndpointReturnsFalseWithInvalidInput) {
                auto attributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                ASSERT_FALSE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, vector<CapabilityConfiguration>()));
                capabilityConfig.version = "";
                ASSERT_FALSE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, {capabilityConfig}));
                attributes.endpointId = "";
                ASSERT_FALSE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, {capabilityConfig}));
                attributes = createEndpointAttributes("duplicateId");
                capabilityConfig.version = "1";
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, {capabilityConfig}));
                ASSERT_FALSE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, {capabilityConfig}));
            }
            TEST_F(CapabilitiesDelegateTest, test_dynamicAddOrUpdateEndpoint) {
                WaitEvent e;
                validateAuthDelegate();
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                auto attributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(AtLeast(1)).WillOnce(Invoke([this](shared_ptr<MessageRequest> request) {
                        string eventCorrelationTokenString;
                        getEventCorrelationTokenString(request, eventCorrelationTokenString);
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                        m_capabilitiesDelegate->onAlexaEventProcessedReceived(eventCorrelationTokenString);
                    }));
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(_)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, erase((std::unordered_map<string, string>{}))).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _)).WillOnce(Invoke([&](CapabilitiesObserverInterface::State state,
                    CapabilitiesObserverInterface::Error error, vector<string> addedEndpoints, vector<string> deletedEndpoints) {
                        EXPECT_EQ(state, CapabilitiesObserverInterface::State::SUCCESS);
                        EXPECT_EQ(error, CapabilitiesObserverInterface::Error::SUCCESS);
                        EXPECT_EQ(addedEndpoints, (std::vector<std::string>{"endpointId"}));
                        EXPECT_EQ(deletedEndpoints, (std::vector<std::string>{}));
                        e.wakeUp();
                    }));
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, {capabilityConfig}));
                ASSERT_TRUE(e.wait(MY_WAIT_TIMEOUT));
            }
            TEST_F(CapabilitiesDelegateTest, test_deleteEndpointReturnsFalseWithInvalidInput) {
                auto attributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                ASSERT_FALSE(m_capabilitiesDelegate->deleteEndpoint(attributes, std::vector<CapabilityConfiguration>()));
                capabilityConfig.version = "";
                ASSERT_FALSE(m_capabilitiesDelegate->deleteEndpoint(attributes, {capabilityConfig}));
                attributes.endpointId = "";
                ASSERT_FALSE(m_capabilitiesDelegate->deleteEndpoint(attributes, {capabilityConfig}));
            }
            TEST_F(CapabilitiesDelegateTest, test_dynamicDeleteEndpoint) {
                WaitEvent e;
                validateAuthDelegate();
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                auto attributes = createEndpointAttributes("deleteId");
                auto capabilityConfig = createCapabilityConfiguration();
                auto configJson = getEndpointConfigJson(attributes, vector<CapabilityConfiguration>{capabilityConfig});
                addEndpoint(attributes, capabilityConfig);
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillRepeatedly(Invoke([](shared_ptr<MessageRequest> request) {
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    }));
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(_)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage,erase(unordered_map<string, string>({{attributes.endpointId, configJson}}))).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([&](CapabilitiesObserverInterface::State state, CapabilitiesObserverInterface::Error error,
                    vector<string> addedEndpoints, vector<string> deletedEndpoints) {
                        EXPECT_EQ(state, CapabilitiesObserverInterface::State::SUCCESS);
                        EXPECT_EQ(error, CapabilitiesObserverInterface::Error::SUCCESS);
                        EXPECT_EQ(addedEndpoints, (std::vector<std::string>{}));
                        EXPECT_EQ(deletedEndpoints, (std::vector<std::string>{"deleteId"}));
                        e.wakeUp();
                    }));
                ASSERT_TRUE(m_capabilitiesDelegate->deleteEndpoint(attributes, {capabilityConfig}));
                ASSERT_TRUE(e.wait(MY_WAIT_TIMEOUT));
            }
            TEST_F(CapabilitiesDelegateTest, test_dynamicDeleteEndpointWhenEndpointNotRegisteredShouldFail) {
                auto attributes = createEndpointAttributes("deleteId");
                auto capabilityConfig = createCapabilityConfiguration();
                ASSERT_FALSE(m_capabilitiesDelegate->deleteEndpoint(attributes, {capabilityConfig}));
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationWithDifferentEndpointConfigs) {
                auto endpointAttributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                string endpointConfig = "TEST_CONFIG";
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_))
                    .Times(1).WillOnce(Invoke([endpointAttributes, endpointConfig](unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({endpointAttributes.endpointId, endpointConfig});
                        return true;
                    }));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addOrUpdateEndpoint(endpointAttributes, {capabilityConfig});
                auto publisher = instance->createPostConnectOperation();
                instance->shutdown();
                ASSERT_NE(publisher, nullptr);
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationWithPendingEndpointsWithSameEndpointConfigs) {
                auto endpointAttributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                vector<CapabilityConfiguration> capabilityConfigs = {capabilityConfig};
                string endpointConfig = getEndpointConfigJson(endpointAttributes, capabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_))
                    .Times(1).WillOnce(Invoke([endpointAttributes, endpointConfig](unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({endpointAttributes.endpointId, endpointConfig});
                        return true;
                    }));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                    vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addCapabilitiesObserver(m_mockCapabilitiesObserver);
                instance->addOrUpdateEndpoint(endpointAttributes, capabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(CapabilitiesObserverInterface::State::SUCCESS,
                            CapabilitiesObserverInterface::Error::SUCCESS, vector<string>{endpointAttributes.endpointId}, _));
                auto publisher = instance->createPostConnectOperation();
                ASSERT_EQ(publisher, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationWithoutPendingEndpointsAndSameEndpointConfigs) {
                auto endpointAttributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                vector<CapabilityConfiguration> capabilityConfigs = {capabilityConfig};
                string endpointConfig = getEndpointConfigJson(endpointAttributes, capabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_))
                    .Times(2).WillRepeatedly(Invoke([endpointAttributes, endpointConfig](unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({endpointAttributes.endpointId, endpointConfig});
                        return true;
                    }));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                    vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, std::vector<std::string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, std::vector<std::string>{});
                    }));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addCapabilitiesObserver(m_mockCapabilitiesObserver);
                instance->addOrUpdateEndpoint(endpointAttributes, capabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(CapabilitiesObserverInterface::State::SUCCESS,
                            CapabilitiesObserverInterface::Error::SUCCESS, vector<string>{endpointAttributes.endpointId}, _));
                auto publisher = instance->createPostConnectOperation();
                ASSERT_EQ(publisher, nullptr);
                publisher = instance->createPostConnectOperation();
                ASSERT_EQ(publisher, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationCachesEndpoints) {
                auto endpointAttributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                vector<CapabilityConfiguration> capabilityConfigs = {capabilityConfig};
                string endpointConfig = getEndpointConfigJson(endpointAttributes, capabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_))
                    .Times(2).WillOnce(Invoke([endpointAttributes, endpointConfig](unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({endpointAttributes.endpointId, endpointConfig});
                        return true;
                    })).WillOnce(Invoke([endpointAttributes, endpointConfig](unordered_map<string, string>* storedEndpoints) {
                        return true;
                    }));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                    vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, std::vector<std::string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, std::vector<std::string>{});
                    }));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addCapabilitiesObserver(m_mockCapabilitiesObserver);
                instance->addOrUpdateEndpoint(endpointAttributes, capabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(CapabilitiesObserverInterface::State::SUCCESS,
                            CapabilitiesObserverInterface::Error::SUCCESS, vector<string>{endpointAttributes.endpointId}, _));
                auto publisher = instance->createPostConnectOperation();
                ASSERT_EQ(publisher, nullptr);
                publisher = instance->createPostConnectOperation();
                ASSERT_NE(publisher, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationWithStaleEndpointAndWithoutPendingEndpointsAndSameEndpointConfigs) {
                auto endpointAttributes = createEndpointAttributes("endpointId");
                auto capabilityConfig = createCapabilityConfiguration();
                vector<CapabilityConfiguration> capabilityConfigs = {capabilityConfig};
                auto staleEndpointAttributes = createEndpointAttributes("staleEndpointId");
                auto staleEndpointConfiguration = createCapabilityConfiguration();
                vector<CapabilityConfiguration> staleCapabilityConfigs = {staleEndpointConfiguration};
                string endpointConfig = getEndpointConfigJson(endpointAttributes, capabilityConfigs);
                string staleEndpointConfig = getEndpointConfigJson(staleEndpointAttributes, staleCapabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_)).Times(2)
                    .WillOnce(Invoke([endpointAttributes, endpointConfig, staleEndpointAttributes, staleEndpointConfig](unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({endpointAttributes.endpointId, endpointConfig});
                        return true;
                    })).WillOnce(Invoke([endpointAttributes, endpointConfig, staleEndpointAttributes, staleEndpointConfig](unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({endpointAttributes.endpointId, endpointConfig});
                        storedEndpoints->insert({staleEndpointAttributes.endpointId, staleEndpointConfig});
                        return true;
                    }));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                    vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addCapabilitiesObserver(m_mockCapabilitiesObserver);
                instance->addOrUpdateEndpoint(endpointAttributes, capabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(CapabilitiesObserverInterface::State::SUCCESS,
                            CapabilitiesObserverInterface::Error::SUCCESS, vector<string>{endpointAttributes.endpointId}, _));
                auto publisher = instance->createPostConnectOperation();
                ASSERT_EQ(publisher, nullptr);
                publisher = instance->createPostConnectOperation();
                ASSERT_NE(publisher, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationWithStaleEndpoint) {
                auto staleEndpointAttributes = createEndpointAttributes("staleEndpointId");
                auto staleEndpointConfiguration = createCapabilityConfiguration();
                vector<CapabilityConfiguration> staleCapabilityConfigs = {staleEndpointConfiguration};
                string staleEndpointConfig = getEndpointConfigJson(staleEndpointAttributes, staleCapabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_))
                    .WillOnce(Invoke([staleEndpointAttributes, staleEndpointConfig](unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({staleEndpointAttributes.endpointId, staleEndpointConfig});
                        return true;
                    }));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                    vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addCapabilitiesObserver(m_mockCapabilitiesObserver);
                auto publisher = instance->createPostConnectOperation();
                ASSERT_NE(publisher, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationWithStaleEndpointAndPendingEndpointsWithSameEndpointConfigs) {
                auto unchangedEndpointAttributes = createEndpointAttributes("endpointId");
                auto unchangedEndpointConfiguration = createCapabilityConfiguration();
                vector<CapabilityConfiguration> unchangedCapabilityConfigs = {unchangedEndpointConfiguration};
                auto staleEndpointAttributes = createEndpointAttributes("staleEndpointId");
                auto staleEndpointConfiguration = createCapabilityConfiguration();
                vector<CapabilityConfiguration> staleCapabilityConfigs = {staleEndpointConfiguration};
                string unchangedEndpointConfig = getEndpointConfigJson(unchangedEndpointAttributes, unchangedCapabilityConfigs);
                string staleEndpointConfig = getEndpointConfigJson(staleEndpointAttributes, staleCapabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_)).Times(1)
                    .WillOnce(Invoke([unchangedEndpointAttributes, unchangedEndpointConfig, staleEndpointAttributes, staleEndpointConfig](
                    unordered_map<string, string>* storedEndpoints) {
                        storedEndpoints->insert({unchangedEndpointAttributes.endpointId, unchangedEndpointConfig});
                        storedEndpoints->insert({staleEndpointAttributes.endpointId, staleEndpointConfig});
                        return true;
                        }));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([](CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError,
                    vector<string> addOrUpdateReportEndpointIdentifiers, vector<string> deleteReportEndpointIdentifiers) {
                        EXPECT_EQ(newState, CapabilitiesObserverInterface::State::UNINITIALIZED);
                        EXPECT_EQ(newError, CapabilitiesObserverInterface::Error::UNINITIALIZED);
                        EXPECT_EQ(addOrUpdateReportEndpointIdentifiers, vector<string>{});
                        EXPECT_EQ(deleteReportEndpointIdentifiers, vector<string>{});
                    }));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addCapabilitiesObserver(m_mockCapabilitiesObserver);
                instance->addOrUpdateEndpoint(unchangedEndpointAttributes, unchangedCapabilityConfigs);
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(CapabilitiesObserverInterface::State::SUCCESS,
                            CapabilitiesObserverInterface::Error::SUCCESS, vector<string>{unchangedEndpointAttributes.endpointId}, vector<string>{}));
                auto publisher = instance->createPostConnectOperation();
                ASSERT_NE(publisher, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_createPostConnectOperationWithReconnects) {
                auto endpointAttributes = createEndpointAttributes();
                auto capabilityConfig = createCapabilityConfiguration();
                vector<CapabilityConfiguration> capabilityConfigs = {capabilityConfig};
                string endpointConfig = getEndpointConfigJson(endpointAttributes, capabilityConfigs);
                unordered_map<string, string> addOrUpdateReportEndpoints = {{endpointAttributes.endpointId, endpointConfig}};
                unordered_map<string, string> emptyDeleteReportEndpoints;
                EXPECT_CALL(*m_mockCapabilitiesStorage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_)).Times(1).WillOnce(Return(true));
                auto instance = CapabilitiesDelegate::create(m_mockAuthDelegate, m_mockCapabilitiesStorage, m_dataManager);
                instance->addOrUpdateEndpoint(endpointAttributes, capabilityConfigs);
                auto publisher = instance->createPostConnectOperation();
                ASSERT_NE(publisher, nullptr);
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(_))
                    .WillOnce(Invoke([addOrUpdateReportEndpoints](const unordered_map<string, string>& endpointIdToConfigMap) {
                        EXPECT_EQ(endpointIdToConfigMap, addOrUpdateReportEndpoints);
                        return true;
                    }));
                EXPECT_CALL(*m_mockCapabilitiesStorage, erase(emptyDeleteReportEndpoints))
                    .WillOnce(Invoke([emptyDeleteReportEndpoints](const unordered_map<string, string>& endpointIdToConfigMap) {
                        EXPECT_EQ(endpointIdToConfigMap, emptyDeleteReportEndpoints);
                        return true;
                    }));
                instance->onDiscoveryCompleted(addOrUpdateReportEndpoints, emptyDeleteReportEndpoints);
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_))
                    .WillOnce(Invoke([addOrUpdateReportEndpoints](std::unordered_map<std::string, std::string>* storedEndpoints) {
                        *storedEndpoints = addOrUpdateReportEndpoints;
                        return true;
                    }));
                auto postConnectOperationOnReconnection = instance->createPostConnectOperation();
                ASSERT_EQ(postConnectOperationOnReconnection, nullptr);
                instance->shutdown();
            }
            TEST_F(CapabilitiesDelegateTest, test_onAVSGatewayChangedNotification) {
                EXPECT_CALL(*m_mockCapabilitiesStorage, clearDatabase()).WillOnce(Return(true));
                m_capabilitiesDelegate->onAVSGatewayChanged("TEST_GATEWAY");
            }
            TEST_F(CapabilitiesDelegateTest, test_reconnectWhenStorageIsEmpty) {
                WaitEvent e;
                validateAuthDelegate();
                auto capabilityConfig = createCapabilityConfiguration();
                const std::string firstEndpointId = "add_1";
                auto firstEndpointAttributes = createEndpointAttributes(firstEndpointId);
                addEndpoint(firstEndpointAttributes, capabilityConfig);
                const string secondEndpointId = "add_2";
                auto secondEndpointAttributes = createEndpointAttributes(secondEndpointId);
                addEndpoint(secondEndpointAttributes, capabilityConfig);
                const string thirdEndpointId = "add_3";
                auto thirdEndpointAttributes = createEndpointAttributes(thirdEndpointId);
                addEndpoint(thirdEndpointAttributes, capabilityConfig);
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                    .WillOnce(Invoke([this](std::shared_ptr<MessageRequest> request) {
                        string eventCorrelationTokenString;
                        getEventCorrelationTokenString(request, eventCorrelationTokenString);
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                        m_capabilitiesDelegate->onAlexaEventProcessedReceived(eventCorrelationTokenString);
                    })).WillOnce(Invoke([](shared_ptr<MessageRequest> request) {
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    }));
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(_)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage,erase((unordered_map<string, std::string>{
                            {thirdEndpointId, getEndpointConfigJson(thirdEndpointAttributes, {capabilityConfig})}}))).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([&](CapabilitiesObserverInterface::State state, CapabilitiesObserverInterface::Error error,
                    vector<string> addedEndpoints, vector<string> deletedEndpoints) {
                        EXPECT_EQ(state, CapabilitiesObserverInterface::State::SUCCESS);
                        EXPECT_EQ(error, CapabilitiesObserverInterface::Error::SUCCESS);
                        std::sort(addedEndpoints.begin(), addedEndpoints.end());
                        EXPECT_EQ(addedEndpoints, (vector<string>{firstEndpointId, secondEndpointId}));
                        EXPECT_EQ(deletedEndpoints, (vector<string>{thirdEndpointId}));
                        e.wakeUp();
                    }));
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::DISCONNECTED,
                                                           ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
                ASSERT_TRUE(m_capabilitiesDelegate->deleteEndpoint(thirdEndpointAttributes, {capabilityConfig}));
                EXPECT_CALL(*m_mockCapabilitiesStorage, load(_)).Times(1).WillOnce(Invoke([](unordered_map<string, string>* storedEndpoints) { return true; }));
                auto postconnect = m_capabilitiesDelegate->createPostConnectOperation();
                ASSERT_NE(postconnect, nullptr);
                ASSERT_TRUE(postconnect->performOperation(m_mockMessageSender));
                ASSERT_TRUE(e.wait(MY_WAIT_TIMEOUT));
            }
            TEST_F(CapabilitiesDelegateTest, test_deferSendDiscoveryEventsWhileDiscoveryEventSenderInFlight) {
                validateAuthDelegate();
                auto capabilityConfig = createCapabilityConfiguration();
                const string firstEndpointId = "add_1";
                auto firstEndpointAttributes = createEndpointAttributes(firstEndpointId);
                const string secondEndpointId = "delete_1";
                auto secondEndpointAttributes = createEndpointAttributes(secondEndpointId);
                auto deleteCapabilityConfigJson = getEndpointConfigJson(secondEndpointAttributes, {capabilityConfig});
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                addEndpoint(secondEndpointAttributes, capabilityConfig);
                auto discoveryEventSender = make_shared<StrictMock<MockDiscoveryEventSender>>();
                EXPECT_CALL(*discoveryEventSender, addDiscoveryStatusObserver(_))
                    .WillOnce(Invoke([this](const shared_ptr<DiscoveryStatusObserverInterface>& observer) {
                        EXPECT_EQ(observer, m_capabilitiesDelegate);
                    }));
                EXPECT_CALL(*discoveryEventSender, removeDiscoveryStatusObserver(_))
                    .WillOnce(Invoke([this](const shared_ptr<DiscoveryStatusObserverInterface>& observer) {
                        EXPECT_EQ(observer, m_capabilitiesDelegate);
                    }));
                EXPECT_CALL(*discoveryEventSender, stop());
                m_capabilitiesDelegate->addDiscoveryEventSender(discoveryEventSender);
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(_, _, std::vector<std::string>{firstEndpointId}, vector<string>{secondEndpointId})).Times(0);
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(firstEndpointAttributes, {capabilityConfig}));
                ASSERT_TRUE(m_capabilitiesDelegate->deleteEndpoint(secondEndpointAttributes, {capabilityConfig}));
            }
            TEST_F(CapabilitiesDelegateTest, test_observerCallingIntoCapabilitiesDelegateOnSuccessNotificationSucceeds) {
                WaitEvent e;
                validateAuthDelegate();
                auto capabilityConfig = createCapabilityConfiguration();
                const string endpointId = "delete_1";
                auto endpointAttributes = createEndpointAttributes(endpointId);
                auto capabilityConfigJson = getEndpointConfigJson(endpointAttributes, {capabilityConfig});
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(2)
                    .WillOnce(Invoke([this](shared_ptr<MessageRequest> request) {
                        string eventCorrelationTokenString;
                        getEventCorrelationTokenString(request, eventCorrelationTokenString);
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                        m_capabilitiesDelegate->onAlexaEventProcessedReceived(eventCorrelationTokenString);
                    })).WillOnce(Invoke([&](shared_ptr<MessageRequest> request) {
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                    }));
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(_)).WillRepeatedly(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage, erase(unordered_map<string, string>{})).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage,erase((unordered_map<string, string>{{endpointId, capabilityConfigJson}}))).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesObserver,onCapabilitiesStateChange(CapabilitiesObserverInterface::State::SUCCESS, CapabilitiesObserverInterface::Error::SUCCESS, _, _))
                    .Times(2).WillOnce(Invoke([&, this](CapabilitiesObserverInterface::State state, CapabilitiesObserverInterface::Error error,
                    vector<string> addedEndpoints, vector<string> deletedEndpoints) {
                        ASSERT_TRUE(m_capabilitiesDelegate->deleteEndpoint(endpointAttributes, {capabilityConfig}));
                    })).WillOnce(Invoke([&](CapabilitiesObserverInterface::State state, CapabilitiesObserverInterface::Error error, vector<string> addedEndpoints,
                    vector<string> deletedEndpoints) { e.wakeUp(); }));
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(endpointAttributes, {capabilityConfig}));
                ASSERT_TRUE(e.wait(MY_WAIT_TIMEOUT));
            }
            TEST_F(CapabilitiesDelegateTest, test_doNotSendEndpointsWhileDisconnected) {
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::DISCONNECTED,
                                                           ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
                auto attributes = createEndpointAttributes("add_1");
                auto capabilityConfig = createCapabilityConfiguration();
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(attributes, {capabilityConfig}));
            }
            TEST_F(CapabilitiesDelegateTest, test_reconnectTriggersSendPendingEndpoints) {
                WaitEvent e;
                validateAuthDelegate();
                auto capabilityConfig = createCapabilityConfiguration();
                const string firstEndpointId = "add_1";
                auto firstEndpointAttributes = createEndpointAttributes(firstEndpointId);
                const string secondEndpointId = "add_2";
                auto secondEndpointAttributes = createEndpointAttributes(secondEndpointId);
                const string thirdEndpointId = "delete_1";
                auto thirdEndpointAttributes = createEndpointAttributes(thirdEndpointId);
                auto deleteCapabilityConfigJson = utils::getEndpointConfigJson(thirdEndpointAttributes, {capabilityConfig});
                addEndpoint(thirdEndpointAttributes, capabilityConfig);
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        string eventCorrelationTokenString;
                        getEventCorrelationTokenString(request, eventCorrelationTokenString);
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
                        m_capabilitiesDelegate->onAlexaEventProcessedReceived(eventCorrelationTokenString);
                    }));
                EXPECT_CALL(*m_mockCapabilitiesStorage, store(_)).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesStorage,erase((unordered_map<string, string>{{thirdEndpointId, deleteCapabilityConfigJson}}))).WillOnce(Return(true));
                EXPECT_CALL(*m_mockCapabilitiesObserver, onCapabilitiesStateChange(_, _, _, _))
                    .WillOnce(Invoke([&](CapabilitiesObserverInterface::State state, CapabilitiesObserverInterface::Error error,
                    vector<string> addedEndpoints, vector<string> deletedEndpoints) {
                        EXPECT_EQ(state, CapabilitiesObserverInterface::State::SUCCESS);
                        EXPECT_EQ(error, CapabilitiesObserverInterface::Error::SUCCESS);
                        sort(addedEndpoints.begin(), addedEndpoints.end());
                        EXPECT_EQ(addedEndpoints, (vector<string>{firstEndpointId, secondEndpointId}));
                        EXPECT_EQ(deletedEndpoints, (vector<string>{thirdEndpointId}));
                        e.wakeUp();
                    }));
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::DISCONNECTED,ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(firstEndpointAttributes, {capabilityConfig}));
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(secondEndpointAttributes, {capabilityConfig}));
                ASSERT_TRUE(m_capabilitiesDelegate->deleteEndpoint(thirdEndpointAttributes, {capabilityConfig}));
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                ASSERT_TRUE(e.wait(MY_WAIT_TIMEOUT));
            }
            TEST_F(CapabilitiesDelegateTest, test_duplicateEndpointInPendingAddOrUpdateAndDeleteShouldFail) {
                auto deleteEndpointAttributes = createEndpointAttributes("delete_1");
                auto addEndpointAttributes = createEndpointAttributes("add_1");
                auto capabilityConfig = createCapabilityConfiguration();
                validateAuthDelegate();
                addEndpoint(deleteEndpointAttributes, capabilityConfig);
                m_capabilitiesDelegate->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::DISCONNECTED,ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
                ASSERT_TRUE(m_capabilitiesDelegate->addOrUpdateEndpoint(addEndpointAttributes, {capabilityConfig}));
                ASSERT_FALSE(m_capabilitiesDelegate->deleteEndpoint(addEndpointAttributes, {capabilityConfig}));
                ASSERT_TRUE(m_capabilitiesDelegate->deleteEndpoint(deleteEndpointAttributes, {capabilityConfig}));
                ASSERT_FALSE(m_capabilitiesDelegate->addOrUpdateEndpoint(deleteEndpointAttributes, {capabilityConfig}));
            }
        }
    }
}