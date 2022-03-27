#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <util/PromiseFuturePair.h>
#include "MockPostConnectObserver.h"
#include "MockPostConnectOperation.h"
#include "PostConnectSequencer.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                using namespace avsCommon::sdkInterfaces;
                using namespace avsCommon::sdkInterfaces::test;
                using namespace avsCommon::utils;
                using namespace ::testing;
                static const auto SHORT_DELAY = std::chrono::seconds(1);
                class PostConnectSequencerTest : public Test {
                public:
                    void SetUp() override;
                protected:
                    std::shared_ptr<MockPostConnectObserver> m_mockPostConnectObserver;
                    std::shared_ptr<MockMessageSender> m_mockMessageSender;
                };
                void PostConnectSequencerTest::SetUp() {
                    m_mockPostConnectObserver = std::make_shared<NiceMock<MockPostConnectObserver>>();
                    m_mockMessageSender = std::make_shared<NiceMock<MockMessageSender>>();
                }
                TEST_F(PostConnectSequencerTest, test_postConnectOperationsSet) {
                    auto operation1 = std::make_shared<NiceMock<MockPostConnectOperation>>();
                    auto operation2 = std::make_shared<NiceMock<MockPostConnectOperation>>();
                    auto operation3 = std::make_shared<NiceMock<MockPostConnectOperation>>();
                    EXPECT_CALL(*operation1, getOperationPriority()).WillRepeatedly(Return(3));
                    EXPECT_CALL(*operation2, getOperationPriority()).WillRepeatedly(Return(2));
                    EXPECT_CALL(*operation3, getOperationPriority()).WillRepeatedly(Return(1));
                    PostConnectSequencer::PostConnectOperationsSet operationsSet;
                    operationsSet.insert(operation1);
                    operationsSet.insert(operation2);
                    operationsSet.insert(operation3);
                    auto it = operationsSet.begin();
                    ASSERT_EQ(*it, operation3);
                    it++;
                    ASSERT_EQ(*it, operation2);
                    it++;
                    ASSERT_EQ(*it, operation1);
                }
                TEST_F(PostConnectSequencerTest, test_postConnectOperationsSetRemovesDuplicates) {
                    auto operation1 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    auto operation2 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    EXPECT_CALL(*operation1, getOperationPriority()).WillRepeatedly(Return(3));
                    EXPECT_CALL(*operation2, getOperationPriority()).WillRepeatedly(Return(3));
                    PostConnectSequencer::PostConnectOperationsSet operationsSet;
                    operationsSet.insert(operation1);
                    operationsSet.insert(operation2);
                    ASSERT_EQ(operationsSet.size(), 1U);
                }
                TEST_F(PostConnectSequencerTest, test_happyPathAndPostConnectObserverGetsNotified) {
                    auto operation1 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    auto operation2 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    auto operation3 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    EXPECT_CALL(*operation1, getOperationPriority()).WillRepeatedly(Return(3));
                    EXPECT_CALL(*operation2, getOperationPriority()).WillRepeatedly(Return(2));
                    EXPECT_CALL(*operation3, getOperationPriority()).WillRepeatedly(Return(1));
                    PostConnectSequencer::PostConnectOperationsSet operationsSet;
                    operationsSet.insert(operation1);
                    operationsSet.insert(operation2);
                    operationsSet.insert(operation3);
                    auto postConnectSequencer = PostConnectSequencer::create(operationsSet);
                    ASSERT_NE(postConnectSequencer, nullptr);
                    {
                        InSequence s;
                        EXPECT_CALL(*operation3, performOperation(_)).WillOnce(Return(true));
                        EXPECT_CALL(*operation2, performOperation(_)).WillOnce(Return(true));
                        EXPECT_CALL(*operation1, performOperation(_)).WillOnce(Return(true));
                    }
                    PromiseFuturePair<bool> promiseFuturePair;
                    EXPECT_CALL(*m_mockPostConnectObserver, onPostConnected()).WillOnce(Invoke([&promiseFuturePair] {
                        promiseFuturePair.setValue(true);
                    }));
                    postConnectSequencer->doPostConnect(m_mockMessageSender, m_mockPostConnectObserver);
                    promiseFuturePair.waitFor(SHORT_DELAY);
                }
                TEST_F(PostConnectSequencerTest, test_doPostConnectReturnFalseOnSecondCall) {
                    auto operation1 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    EXPECT_CALL(*operation1, getOperationPriority()).WillRepeatedly(Return(1));
                    PostConnectSequencer::PostConnectOperationsSet operationsSet;
                    operationsSet.insert(operation1);
                    auto postConnectSequencer = PostConnectSequencer::create(operationsSet);
                    ASSERT_NE(postConnectSequencer, nullptr);
                    EXPECT_CALL(*operation1, performOperation(_)).WillOnce(Return(true));
                    PromiseFuturePair<bool> promiseFuturePair;
                    EXPECT_CALL(*m_mockPostConnectObserver, onPostConnected()).WillOnce(Invoke([&promiseFuturePair] {
                        promiseFuturePair.setValue(true);
                    }));
                    ASSERT_TRUE(postConnectSequencer->doPostConnect(m_mockMessageSender, m_mockPostConnectObserver));
                    ASSERT_FALSE(postConnectSequencer->doPostConnect(m_mockMessageSender, m_mockPostConnectObserver));
                    promiseFuturePair.waitFor(SHORT_DELAY);
                }
                TEST_F(PostConnectSequencerTest, test_subsequentOperationsDontExecute) {
                    auto operation1 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    auto operation2 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    auto operation3 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    EXPECT_CALL(*operation1, getOperationPriority()).WillRepeatedly(Return(3));
                    EXPECT_CALL(*operation2, getOperationPriority()).WillRepeatedly(Return(2));
                    EXPECT_CALL(*operation3, getOperationPriority()).WillRepeatedly(Return(1));
                    PostConnectSequencer::PostConnectOperationsSet operationsSet;
                    operationsSet.insert(operation1);
                    operationsSet.insert(operation2);
                    operationsSet.insert(operation3);
                    auto postConnectSequencer = PostConnectSequencer::create(operationsSet);
                    ASSERT_NE(postConnectSequencer, nullptr);
                    EXPECT_CALL(*operation3, performOperation(_)).WillOnce(Return(false));
                    PromiseFuturePair<bool> promiseFuturePair;
                    EXPECT_CALL(*m_mockPostConnectObserver, onUnRecoverablePostConnectFailure()).WillOnce(Invoke([&promiseFuturePair] {
                        promiseFuturePair.setValue(true);
                    }));
                    postConnectSequencer->doPostConnect(m_mockMessageSender, m_mockPostConnectObserver);
                    EXPECT_CALL(*operation3, abortOperation()).Times(AtMost(1));
                    promiseFuturePair.waitFor(SHORT_DELAY);
                }
                TEST_F(PostConnectSequencerTest, test_onDisconnectStopsExecution) {
                    auto operation1 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    auto operation2 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    auto operation3 = std::make_shared<StrictMock<MockPostConnectOperation>>();
                    EXPECT_CALL(*operation1, getOperationPriority()).WillRepeatedly(Return(3));
                    EXPECT_CALL(*operation2, getOperationPriority()).WillRepeatedly(Return(2));
                    EXPECT_CALL(*operation3, getOperationPriority()).WillRepeatedly(Return(1));
                    PostConnectSequencer::PostConnectOperationsSet operationsSet;
                    operationsSet.insert(operation1);
                    operationsSet.insert(operation2);
                    operationsSet.insert(operation3);
                    auto postConnectSequencer = PostConnectSequencer::create(operationsSet);
                    ASSERT_NE(postConnectSequencer, nullptr);
                    PromiseFuturePair<bool> notifyOnPerfromOperation, notifyOnAbortOperation;
                    EXPECT_CALL(*operation3, performOperation(_))
                        .WillOnce(Invoke([&notifyOnAbortOperation, &notifyOnPerfromOperation](const std::shared_ptr<MessageSenderInterface>& postConnectSender) {
                            notifyOnPerfromOperation.setValue(true);
                            notifyOnAbortOperation.waitFor(SHORT_DELAY);
                            return true;
                        }));
                    EXPECT_CALL(*operation3, abortOperation()).WillOnce(Invoke([&notifyOnAbortOperation] {
                        notifyOnAbortOperation.setValue(true);
                    }));
                    postConnectSequencer->doPostConnect(m_mockMessageSender, m_mockPostConnectObserver);
                    notifyOnPerfromOperation.waitFor(SHORT_DELAY);
                    postConnectSequencer->onDisconnect();
                }
            }
        }
    }
}