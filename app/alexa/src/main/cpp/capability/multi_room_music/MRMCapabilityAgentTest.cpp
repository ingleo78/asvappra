#include <iterator>
#include <memory>
#include <sstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockSpeakerManager.h>
#include <sdkinterfaces/MockUserInactivityMonitor.h>
#include <sdkinterfaces/SpeakerInterface.h>
#include "MRMCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace mrm {
            namespace test {
                using namespace std;
                using namespace chrono;
                using namespace avsCommon;
                using namespace avs;
                using namespace sdkInterfaces;
                using namespace sdkInterfaces::test;
                using namespace testing;
                static const string TEST_MRM_HANDLER_VERSION_STRING = "test_version_string";
                static const auto WAIT_FOR_INVOCATION_LONG_TIMEOUT = milliseconds{100};
                static const auto WAIT_FOR_INVOCATION_SHORT_TIMEOUT = milliseconds{5};
                static const string TEST_DIRECTIVE_JSON_STRING = R"delim({"directive": {"header": {"namespace": "MRM","name": "TestDirective",
                                                                 "messageId": "12345"},"payload": {}}})delim";
                class SynchronizedInvocation {
                public:
                    SynchronizedInvocation() : m_hasBeenInvoked{false} {}
                    void invoke() {
                        unique_lock<std::mutex> lock(m_mutex);
                        m_hasBeenInvoked = true;
                        m_cv.notify_all();
                    }
                    bool wait(const milliseconds timeout) {
                        unique_lock<std::mutex> lock(m_mutex);
                        if (m_cv.wait_for(lock, timeout, [this]() { return m_hasBeenInvoked; })) {
                            m_hasBeenInvoked = false;
                            return true;
                        }
                        return false;
                    }
                private:
                    mutex m_mutex;
                    condition_variable m_cv;
                    bool m_hasBeenInvoked;
                };
                class WaitableExceptionEncounteredSender : public ExceptionEncounteredSenderInterface {
                public:
                    void sendExceptionEncountered(
                        const string& unparsedDirective,
                        ExceptionErrorType error,
                        const string& message) override {
                        m_invocation.invoke();
                    }
                    bool wait(const milliseconds timeout) {
                        return m_invocation.wait(timeout);
                    }
                private:
                    SynchronizedInvocation m_invocation;
                };
                class MockMRMHandler : public MRMHandlerInterface {
                public:
                    MockMRMHandler() : MRMHandlerInterface{"MockMRMHandler"} {}
                    MOCK_CONST_METHOD0(getVersionString, string());
                    MOCK_METHOD4(handleDirective, bool(const string&, const string&, const string&, const string&));
                    MOCK_METHOD0(doShutdown, void());
                    MOCK_METHOD1(onCallStateChange, void(bool));
                    MOCK_METHOD1(setObserver, void(shared_ptr<RenderPlayerInfoCardsObserverInterface>));
                #ifdef ENABLE_AUXCONTROLLER
                    MOCK_METHOD1(onPlaybackStateChanged, void(const interfaces::auxController::AuxPlaybackState));
                    MOCK_METHOD1(onAuxPlaybackStateChanged, void(interfaces::auxController::AuxPlaybackState));
                    MOCK_METHOD1(setObserver, void(std::shared_ptr<interfaces::auxController::AuxPlaybackDelegateObserverInterface>));
                    MOCK_METHOD1(clearObserver, void(std::shared_ptr<interfaces::auxController::AuxPlaybackDelegateObserverInterface>));
                #endif
                    void onSpeakerSettingsChanged(const ChannelVolumeInterface::Type& type) {
                        m_lastSpeakerType = type;
                        m_speakerSettingUpdatedInvocation.invoke();
                    }
                    void onUserInactivityReportSent() {
                        m_userInactivityInvocation.invoke();
                    }
                    bool waitForSpeakerSettingChanged(
                        const ChannelVolumeInterface::Type& expectedType,
                        const milliseconds timeout) {
                        if (m_speakerSettingUpdatedInvocation.wait(timeout)) return expectedType == m_lastSpeakerType;
                        return false;
                    }
                    bool waitForUserInactivityReport(const std::chrono::milliseconds timeout) {
                        return m_userInactivityInvocation.wait(timeout);
                    }
                private:
                    SynchronizedInvocation m_userInactivityInvocation;
                    SynchronizedInvocation m_speakerSettingUpdatedInvocation;
                    ChannelVolumeInterface::Type m_lastSpeakerType;
                };
                class MRMCapabilityAgentTest : public ::testing::Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    shared_ptr<MRMCapabilityAgent> m_mrmCA;
                    MockMRMHandler* m_mockMRMHandlerPtr;
                    shared_ptr<MockSpeakerManager> m_mockSpeakerManager;
                    shared_ptr<MockUserInactivityMonitor> m_mockUserInactivityMonitor;
                    shared_ptr<WaitableExceptionEncounteredSender> m_exceptionSender;
                };
                void MRMCapabilityAgentTest::SetUp() {
                    auto mrmHandler = unique_ptr<MRMHandlerInterface>(new MockMRMHandler());
                    m_mockMRMHandlerPtr = static_cast<MockMRMHandler*>(mrmHandler.get());
                    m_mockSpeakerManager = make_shared<MockSpeakerManager>();
                    m_mockUserInactivityMonitor = make_shared<MockUserInactivityMonitor>();
                    m_exceptionSender = make_shared<WaitableExceptionEncounteredSender>();
                    EXPECT_CALL(*m_mockSpeakerManager, addSpeakerManagerObserver(_)).Times(1);
                    EXPECT_CALL(*m_mockUserInactivityMonitor, addObserver(_)).Times(1);
                    m_mrmCA = MRMCapabilityAgent::create(move(mrmHandler),m_mockSpeakerManager,m_mockUserInactivityMonitor,
                                  m_exceptionSender);
                    ASSERT_NE(m_mrmCA, nullptr);
                }
                void MRMCapabilityAgentTest::TearDown() {
                    EXPECT_CALL(*m_mockMRMHandlerPtr, doShutdown()).Times(1);
                    EXPECT_CALL(*m_mockSpeakerManager, removeSpeakerManagerObserver(_)).Times(1);
                    EXPECT_CALL(*m_mockUserInactivityMonitor, removeObserver(_)).Times(1);
                    m_mrmCA->shutdown();
                }
                TEST_F(MRMCapabilityAgentTest, test_create) {
                    auto mrmHandler = unique_ptr<MRMHandlerInterface>(new MockMRMHandler());
                    ASSERT_EQ(nullptr,MRMCapabilityAgent::create(nullptr, m_mockSpeakerManager, m_mockUserInactivityMonitor, m_exceptionSender));
                    ASSERT_EQ(nullptr,MRMCapabilityAgent::create(move(mrmHandler), nullptr, m_mockUserInactivityMonitor, m_exceptionSender));
                    ASSERT_EQ(nullptr, MRMCapabilityAgent::create(move(mrmHandler), m_mockSpeakerManager, nullptr, m_exceptionSender));
                    ASSERT_EQ(nullptr,MRMCapabilityAgent::create(move(mrmHandler), m_mockSpeakerManager, m_mockUserInactivityMonitor, nullptr));
                }
                TEST_F(MRMCapabilityAgentTest, test_getConfiguration) {
                    auto config = m_mrmCA->getConfiguration();
                    ASSERT_NE(true, config.empty());
                }
                TEST_F(MRMCapabilityAgentTest, test_getVersionString) {
                    EXPECT_CALL(*m_mockMRMHandlerPtr, getVersionString()).WillOnce(Return(TEST_MRM_HANDLER_VERSION_STRING));
                    string versionString = m_mrmCA->getVersionString();
                    ASSERT_NE(true, versionString.empty());
                }
                TEST_F(MRMCapabilityAgentTest, test_handleMRMDirective) {
                    auto directivePair = AVSDirective::create(TEST_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    EXPECT_CALL(*m_mockMRMHandlerPtr, handleDirective(_, _, _, _)).WillOnce(Return(false));
                    m_mrmCA->handleDirectiveImmediately(directive);
                    ASSERT_TRUE(m_exceptionSender->wait(WAIT_FOR_INVOCATION_LONG_TIMEOUT));
                    EXPECT_CALL(*m_mockMRMHandlerPtr, handleDirective(_, _, _, _)).WillOnce(Return(true));
                    m_mrmCA->handleDirectiveImmediately(directive);
                    ASSERT_FALSE(m_exceptionSender->wait(WAIT_FOR_INVOCATION_SHORT_TIMEOUT));
                }
                TEST_F(MRMCapabilityAgentTest, test_onSpeakerSettingsChanged) {
                    SpeakerInterface::SpeakerSettings dummySpeakerSettings;
                    m_mrmCA->onSpeakerSettingsChanged(SpeakerManagerObserverInterface::Source::DIRECTIVE,ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME,
                                                      dummySpeakerSettings);
                    ASSERT_TRUE(m_mockMRMHandlerPtr->waitForSpeakerSettingChanged(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, WAIT_FOR_INVOCATION_LONG_TIMEOUT));
                    m_mrmCA->onSpeakerSettingsChanged(SpeakerManagerObserverInterface::Source::DIRECTIVE,ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME,
                                                      dummySpeakerSettings);
                    ASSERT_TRUE(m_mockMRMHandlerPtr->waitForSpeakerSettingChanged(ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME, WAIT_FOR_INVOCATION_LONG_TIMEOUT));
                }
                TEST_F(MRMCapabilityAgentTest, test_onUserInactivityReport) {
                    m_mrmCA->onUserInactivityReportSent();
                    ASSERT_TRUE(m_mockMRMHandlerPtr->waitForUserInactivityReport(WAIT_FOR_INVOCATION_LONG_TIMEOUT));
                    ASSERT_FALSE(m_mockMRMHandlerPtr->waitForUserInactivityReport(WAIT_FOR_INVOCATION_SHORT_TIMEOUT));
                }
            }
        }
    }
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc < 1) {
        std::cerr << "USAGE: " << std::string(argv[0]) << std::endl;
        return 1;
    }
    return RUN_ALL_TESTS();
}