#include <gmock/gmock.h>
#include <sdkinterfaces/Storage/StubMiscStorage.h>
#include "AVSGatewayManagerStorage.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        namespace storage {
            namespace test {
                using namespace ::testing;
                using namespace avsCommon::sdkInterfaces::storage;
                using namespace avsCommon::sdkInterfaces::storage::test;
                static const std::string COMPONENT_NAME = "avsGatewayManager";
                static const std::string VERIFICATION_STATE_TABLE = "verificationState";
                static const std::string VERIFICATION_STATE_KEY = "state";
                static const std::string TEST_URL = "www.amazon.com";
                static const std::string SECOND_TEST_URL = "www.avs.amazon.com";
                static const std::string STORED_STATE = R"({"gatewayURL":")" + TEST_URL + R"(","isVerified":false})";
                static const std::string SECOND_STORED_STATE = R"({"gatewayURL":")" + SECOND_TEST_URL + R"(","isVerified":true})";
                class AVSGatewayManagerStorageTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    std::shared_ptr<StubMiscStorage> m_miscStorage;
                    std::shared_ptr<AVSGatewayManagerStorage> m_avsGatewayManagerStorage;
                };
                void AVSGatewayManagerStorageTest::SetUp() {
                    m_miscStorage = StubMiscStorage::create();
                    m_avsGatewayManagerStorage = AVSGatewayManagerStorage::create(m_miscStorage);
                }
                void AVSGatewayManagerStorageTest::TearDown() {
                    m_avsGatewayManagerStorage->clear();
                }
                TEST_F(AVSGatewayManagerStorageTest, test_createWithNullMiscStorage) {
                    ASSERT_EQ(AVSGatewayManagerStorage::create(nullptr), nullptr);
                }
                TEST_F(AVSGatewayManagerStorageTest, test_init) {
                    bool tableExists = false;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_avsGatewayManagerStorage->init());
                    tableExists = false;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists));
                    ASSERT_TRUE(tableExists);
                }
                TEST_F(AVSGatewayManagerStorageTest, test_storeGatewayState) {
                    bool tableExists = false;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_avsGatewayManagerStorage->init());
                    GatewayVerifyState gatewayVerifyState = {TEST_URL, false};
                    ASSERT_TRUE(m_avsGatewayManagerStorage->saveState(gatewayVerifyState));
                    std::string stateString;
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, VERIFICATION_STATE_TABLE, VERIFICATION_STATE_KEY, &stateString));
                    ASSERT_EQ(stateString, STORED_STATE);
                }
                TEST_F(AVSGatewayManagerStorageTest, test_storeSameValue) {
                    bool tableExists = false;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_avsGatewayManagerStorage->init());
                    GatewayVerifyState gatewayVerifyState = {TEST_URL, false};
                    ASSERT_TRUE(m_avsGatewayManagerStorage->saveState(gatewayVerifyState));
                    std::string stateString;
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, VERIFICATION_STATE_TABLE, VERIFICATION_STATE_KEY, &stateString));
                    ASSERT_EQ(stateString, STORED_STATE);
                    gatewayVerifyState = {SECOND_TEST_URL, true};
                    ASSERT_TRUE(m_avsGatewayManagerStorage->saveState(gatewayVerifyState));
                    stateString = "";
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, VERIFICATION_STATE_TABLE, VERIFICATION_STATE_KEY, &stateString));
                    ASSERT_EQ(stateString, SECOND_STORED_STATE);
                }
                TEST_F(AVSGatewayManagerStorageTest, test_loadGatewayState) {
                    bool tableExists = false;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_avsGatewayManagerStorage->init());
                    GatewayVerifyState gatewayVerifyState = {TEST_URL, true};
                    ASSERT_TRUE(m_avsGatewayManagerStorage->saveState(gatewayVerifyState));
                    GatewayVerifyState stateFromStorage = {"", false};
                    ASSERT_TRUE(m_avsGatewayManagerStorage->loadState(&stateFromStorage));
                    ASSERT_EQ(stateFromStorage.avsGatewayURL, TEST_URL);
                    ASSERT_EQ(stateFromStorage.isVerified, true);
                }
                TEST_F(AVSGatewayManagerStorageTest, test_loadGatewayStateFromEmptyStorage) {
                    bool tableExists = false;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_avsGatewayManagerStorage->init());
                    GatewayVerifyState stateFromStorage = {"", false};
                    ASSERT_FALSE(m_avsGatewayManagerStorage->loadState(&stateFromStorage));
                    ASSERT_EQ(stateFromStorage.avsGatewayURL, "");
                    ASSERT_EQ(stateFromStorage.isVerified, false);
                }
                TEST_F(AVSGatewayManagerStorageTest, test_clearState) {
                    bool tableExists = false;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_avsGatewayManagerStorage->init());
                    GatewayVerifyState gatewayVerifyState = {TEST_URL, true};
                    ASSERT_TRUE(m_avsGatewayManagerStorage->saveState(gatewayVerifyState));
                    GatewayVerifyState stateFromStorage = {"", false};
                    ASSERT_TRUE(m_avsGatewayManagerStorage->loadState(&stateFromStorage));
                    ASSERT_EQ(stateFromStorage.avsGatewayURL, TEST_URL);
                    ASSERT_EQ(stateFromStorage.isVerified, true);
                    m_avsGatewayManagerStorage->clear();
                    stateFromStorage = {"", false};
                    ASSERT_FALSE(m_avsGatewayManagerStorage->loadState(&stateFromStorage));
                    ASSERT_EQ(stateFromStorage.avsGatewayURL, "");
                    ASSERT_EQ(stateFromStorage.isVerified, false);
                }
            }
        }
    }
}