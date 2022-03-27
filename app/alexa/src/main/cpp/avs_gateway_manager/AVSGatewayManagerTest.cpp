#include <fstream>
#include <string>
#include <gmock/gmock.h>
#include <sdkinterfaces/MockAVSGatewayAssigner.h>
#include <sdkinterfaces/MockAVSGatewayObserver.h>
#include <configuration/ConfigurationNode.h>
#include <registration_manager/CustomerDataManager.h>
#include "AVSGatewayManager.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            using namespace utils;
            using namespace configuration;
            using namespace registrationManager;
            using namespace testing;
            static const string TEST_AVS_GATEWAY = "www.test-avs-gateway.com";
            static const string DEFAULT_AVS_GATEWAY = "https://alexa.na.gateway.devices.a2z.com";
            static const string STORED_AVS_GATEWAY = "www.avs-gatewa-from-storage.com";
            static const string AVS_GATEWAY_MANAGER_JSON = R"({"avsGatewayManager" : {"avsGateway":")" + TEST_AVS_GATEWAY + R"("}})";
            static const std::string AVS_GATEWAY_MANAGER_JSON_NO_GATEWAY = R"({"avsGatewayManager" : {}})";
            static const std::string AVS_GATEWAY_MANAGER_JSON_EMPTY_GATEWAY = R"({"avsGatewayManager" : {"avsGateway":""}})";
            class MockAVSGatewayManagerStorage : public storage::AVSGatewayManagerStorageInterface {
            public:
                MOCK_METHOD0(init, bool());
                MOCK_METHOD1(loadState, bool(GatewayVerifyState*));
                MOCK_METHOD1(saveState, bool(const GatewayVerifyState&));
                MOCK_METHOD0(clear, void());
            };
            class AVSGatewayManagerTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
            protected:
                void initializeConfigRoot(const string& configJson);
                void createAVSGatewayManager();
                shared_ptr<CustomerDataManager> m_customerDataManager;
                shared_ptr<MockAVSGatewayAssigner> m_mockAVSGatewayAssigner;
                shared_ptr<MockAVSGatewayObserver> m_mockAVSGatewayObserver;
                shared_ptr<MockAVSGatewayManagerStorage> m_mockAVSGatewayManagerStorage;
                shared_ptr<AVSGatewayManager> m_avsGatewayManager;
            };
            void AVSGatewayManagerTest::SetUp() {
                m_mockAVSGatewayManagerStorage = make_shared<NiceMock<MockAVSGatewayManagerStorage>>();
                m_mockAVSGatewayAssigner = make_shared<NiceMock<MockAVSGatewayAssigner>>();
                m_mockAVSGatewayObserver = make_shared<NiceMock<MockAVSGatewayObserver>>();
                m_customerDataManager = make_shared<CustomerDataManager>();
            }
            void AVSGatewayManagerTest::TearDown() {
                ConfigurationNode::uninitialize();
            }
            void AVSGatewayManagerTest::initializeConfigRoot(const string& configJson) {
                auto json = shared_ptr<stringstream>(new stringstream());
                *json << configJson;
                vector<shared_ptr<istream>> jsonStream;
                jsonStream.push_back(json);
                ASSERT_TRUE(ConfigurationNode::initialize(jsonStream));
            }
            void AVSGatewayManagerTest::createAVSGatewayManager() {
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, init()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, loadState(_)).Times(1).WillOnce(Return(true));
                m_avsGatewayManager = AVSGatewayManager::create(m_mockAVSGatewayManagerStorage,m_customerDataManager, ConfigurationNode::getRoot());
                ASSERT_NE(m_avsGatewayManager, nullptr);
            }
            TEST_F(AVSGatewayManagerTest, test_createAVSGatewayManagerWithInvalidParameters) {
                auto instance = AVSGatewayManager::create(nullptr, m_customerDataManager, ConfigurationNode::getRoot());
                ASSERT_EQ(instance, nullptr);
                instance = AVSGatewayManager::create(m_mockAVSGatewayManagerStorage,nullptr, ConfigurationNode::getRoot());
                ASSERT_EQ(instance, nullptr);
            }
            TEST_F(AVSGatewayManagerTest, test_defaultAVSGatewayFromConfigFile) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                ASSERT_EQ(TEST_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
                EXPECT_CALL(*m_mockAVSGatewayAssigner, setAVSGateway(_)).WillOnce(Invoke([](const string& gatewayURL) {
                    ASSERT_EQ(gatewayURL, TEST_AVS_GATEWAY);
                }));
                m_avsGatewayManager->setAVSGatewayAssigner(m_mockAVSGatewayAssigner);
            }
            TEST_F(AVSGatewayManagerTest, test_defaultAVSGatewayFromConfigFileWithNoGateway) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON_NO_GATEWAY);
                createAVSGatewayManager();
                ASSERT_EQ(DEFAULT_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
                EXPECT_CALL(*m_mockAVSGatewayAssigner, setAVSGateway(_)).WillOnce(Invoke([](const string& gatewayURL) {
                    ASSERT_EQ(gatewayURL, DEFAULT_AVS_GATEWAY);
                }));
                m_avsGatewayManager->setAVSGatewayAssigner(m_mockAVSGatewayAssigner);
            }
            TEST_F(AVSGatewayManagerTest, test_defaultAVSGatewayFromConfigFileWithEmptyGateway) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON_EMPTY_GATEWAY);
                createAVSGatewayManager();
                ASSERT_EQ(DEFAULT_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
                EXPECT_CALL(*m_mockAVSGatewayAssigner, setAVSGateway(_)).WillOnce(Invoke([](const string& gatewayURL) {
                    ASSERT_EQ(gatewayURL, DEFAULT_AVS_GATEWAY);
                }));
                m_avsGatewayManager->setAVSGatewayAssigner(m_mockAVSGatewayAssigner);
            }
            TEST_F(AVSGatewayManagerTest, test_avsGatewayFromStorage) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, init()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, loadState(_)).WillOnce(Invoke([](GatewayVerifyState* state) {
                    state->avsGatewayURL = STORED_AVS_GATEWAY;
                    state->isVerified = true;
                    return true;
                }));
                m_avsGatewayManager = AVSGatewayManager::create(m_mockAVSGatewayManagerStorage, m_customerDataManager, ConfigurationNode::getRoot());
                ASSERT_NE(m_avsGatewayManager, nullptr);
                ASSERT_EQ(STORED_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
                EXPECT_CALL(*m_mockAVSGatewayAssigner, setAVSGateway(_)).WillOnce(Invoke([](const string& gatewayURL) {
                    ASSERT_EQ(gatewayURL, STORED_AVS_GATEWAY);
                }));
                m_avsGatewayManager->setAVSGatewayAssigner(m_mockAVSGatewayAssigner);
                auto postConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_EQ(postConnectOperation, nullptr);
            }
            TEST_F(AVSGatewayManagerTest, test_setAVSGatewayURLWithNewURL) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                EXPECT_CALL(*m_mockAVSGatewayAssigner, setAVSGateway(_)).Times(2);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, saveState(_)).Times(1);
                EXPECT_CALL(*m_mockAVSGatewayObserver, onAVSGatewayChanged(_)).Times(1);
                m_avsGatewayManager->addObserver(m_mockAVSGatewayObserver);
                m_avsGatewayManager->setAVSGatewayAssigner(m_mockAVSGatewayAssigner);
                ASSERT_TRUE(m_avsGatewayManager->setGatewayURL(DEFAULT_AVS_GATEWAY));
                ASSERT_EQ(DEFAULT_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
            }
            TEST_F(AVSGatewayManagerTest, test_setAVSGatewayURLWithSameURL) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                EXPECT_CALL(*m_mockAVSGatewayAssigner, setAVSGateway(_)).Times(1);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, saveState(_)).Times(0);
                EXPECT_CALL(*m_mockAVSGatewayObserver, onAVSGatewayChanged(_)).Times(0);
                m_avsGatewayManager->addObserver(m_mockAVSGatewayObserver);
                m_avsGatewayManager->setAVSGatewayAssigner(m_mockAVSGatewayAssigner);
                ASSERT_FALSE(m_avsGatewayManager->setGatewayURL(TEST_AVS_GATEWAY));
                ASSERT_EQ(TEST_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
            }
            TEST_F(AVSGatewayManagerTest, test_setAVSGatewayURLWithoutAssigner) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, saveState(_)).Times(1);
                EXPECT_CALL(*m_mockAVSGatewayObserver, onAVSGatewayChanged(_)).Times(1);
                m_avsGatewayManager->addObserver(m_mockAVSGatewayObserver);
                ASSERT_TRUE(m_avsGatewayManager->setGatewayURL(DEFAULT_AVS_GATEWAY));
                ASSERT_EQ(DEFAULT_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
            }
            TEST_F(AVSGatewayManagerTest, test_addNullObserver) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                m_avsGatewayManager->addObserver(nullptr);
            }
            TEST_F(AVSGatewayManagerTest, test_removeNullObserver) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                m_avsGatewayManager->removeObserver(nullptr);
            }
            TEST_F(AVSGatewayManagerTest, test_removeAddedObserver) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                EXPECT_CALL(*m_mockAVSGatewayObserver, onAVSGatewayChanged(_)).Times(1);
                m_avsGatewayManager->addObserver(m_mockAVSGatewayObserver);
                m_avsGatewayManager->setGatewayURL(DEFAULT_AVS_GATEWAY);
                EXPECT_CALL(*m_mockAVSGatewayObserver, onAVSGatewayChanged(_)).Times(0);
                m_avsGatewayManager->removeObserver(m_mockAVSGatewayObserver);
                m_avsGatewayManager->setGatewayURL(TEST_AVS_GATEWAY);
                ASSERT_EQ(TEST_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
            }
            TEST_F(AVSGatewayManagerTest, test_removeObserverNotAddedPreviously) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                EXPECT_CALL(*m_mockAVSGatewayObserver, onAVSGatewayChanged(_)).Times(0);
                m_avsGatewayManager->setGatewayURL(DEFAULT_AVS_GATEWAY);
                m_avsGatewayManager->removeObserver(m_mockAVSGatewayObserver);
            }
            TEST_F(AVSGatewayManagerTest, test_clearData) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                createAVSGatewayManager();
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, clear()).Times(1);
                m_avsGatewayManager->clearData();
                ASSERT_EQ(TEST_AVS_GATEWAY, m_avsGatewayManager->getGatewayURL());
            }
            TEST_F(AVSGatewayManagerTest, test_createPostConnectOperationMultipleTimesWhenDBIsEmpty) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, init()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, loadState(_)).WillOnce(Invoke([](GatewayVerifyState* state) {
                    EXPECT_EQ(state->avsGatewayURL, TEST_AVS_GATEWAY);
                    EXPECT_EQ(state->isVerified, false);
                    return true;
                }));
                m_avsGatewayManager = AVSGatewayManager::create(m_mockAVSGatewayManagerStorage, m_customerDataManager, ConfigurationNode::getRoot());
                ASSERT_NE(m_avsGatewayManager, nullptr);
                auto firstPostConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_NE(firstPostConnectOperation, nullptr);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, saveState(_)).WillOnce(Invoke([](const GatewayVerifyState& state) {
                    EXPECT_EQ(state.isVerified, true);
                    EXPECT_EQ(state.avsGatewayURL, TEST_AVS_GATEWAY);
                    return true;
                }));
                m_avsGatewayManager->onGatewayVerified(firstPostConnectOperation);
                auto secondPostConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_EQ(secondPostConnectOperation, nullptr);
            }
            TEST_F(AVSGatewayManagerTest, test_createPostConnectOperationRetunrsNullIfDBContainsVerifiedGateway) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, init()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, loadState(_)).WillOnce(Invoke([](GatewayVerifyState* state) {
                    state->avsGatewayURL = STORED_AVS_GATEWAY;
                    state->isVerified = false;
                    return true;
                }));
                m_avsGatewayManager = AVSGatewayManager::create(m_mockAVSGatewayManagerStorage,m_customerDataManager, ConfigurationNode::getRoot());
                ASSERT_NE(m_avsGatewayManager, nullptr);
                auto firstPostConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_NE(firstPostConnectOperation, nullptr);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, saveState(_)).WillOnce(Invoke([](const GatewayVerifyState& state) {
                    EXPECT_EQ(state.isVerified, true);
                    EXPECT_EQ(state.avsGatewayURL, STORED_AVS_GATEWAY);
                    return true;
                }));
                m_avsGatewayManager->onGatewayVerified(firstPostConnectOperation);
                auto secondPostConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_EQ(secondPostConnectOperation, nullptr);
            }
            TEST_F(AVSGatewayManagerTest, test_createPostConnectOperationSequenceAfterSetGatewayURL) {
                initializeConfigRoot(AVS_GATEWAY_MANAGER_JSON);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, init()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, loadState(_)).WillOnce(Invoke([](GatewayVerifyState* state) {
                    state->avsGatewayURL = STORED_AVS_GATEWAY;
                    state->isVerified = true;
                    return true;
                }));
                m_avsGatewayManager = AVSGatewayManager::create(m_mockAVSGatewayManagerStorage, m_customerDataManager, ConfigurationNode::getRoot());
                ASSERT_NE(m_avsGatewayManager, nullptr);
                m_avsGatewayManager->setAVSGatewayAssigner(m_mockAVSGatewayAssigner);
                auto firstPostConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_EQ(firstPostConnectOperation, nullptr);
                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, saveState(_)).WillOnce(Invoke([](const GatewayVerifyState& state) {
                    EXPECT_EQ(state.isVerified, false);
                    EXPECT_EQ(state.avsGatewayURL, TEST_AVS_GATEWAY);
                    return true;
                }));
                m_avsGatewayManager->setGatewayURL(TEST_AVS_GATEWAY);
                auto secondPostConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_NE(secondPostConnectOperation, nullptr);

                EXPECT_CALL(*m_mockAVSGatewayManagerStorage, saveState(_)).WillOnce(Invoke([](const GatewayVerifyState& state) {
                    EXPECT_EQ(state.isVerified, true);
                    EXPECT_EQ(state.avsGatewayURL, TEST_AVS_GATEWAY);
                    return true;
                }));
                m_avsGatewayManager->onGatewayVerified(secondPostConnectOperation);
                auto thirdPostConnectOperation = m_avsGatewayManager->createPostConnectOperation();
                ASSERT_EQ(thirdPostConnectOperation, nullptr);
            }
        }
    }
}