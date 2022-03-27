#include <chrono>
#include <memory>
#include <string>
#include <gtest/gtest.h>
#include <registration_manager/CustomerDataManager.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockAVSConnectionManager.h>
#include <sdkinterfaces/Storage/StubMiscStorage.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <avs/AVSMessageHeader.h>
#include <avs/AVSDirective.h>
#include <avs/MessageRequest.h>
#include <avs/attachment/AttachmentManager.h>
#include "ReportStateHandler.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            namespace test {
                using namespace attachment;
                using namespace storage;
                using namespace testing;
                using namespace sdkInterfaces::test;
                using namespace storage::test;
                static const string TEST_MESSAGE_ID("c88f970a-3519-4ecb-bdcc-0488aca22b87");
                static const string TEST_REQUEST_ID("4b73575e-2e7d-425b-bfa4-c6615e0fbd43");
                static const string TEST_CONTEXT_ID("71c967d8-ad58-47b0-924d-b752deb75e4e");
                class MockStateReportGenerator : public StateReportGenerator {
                public:
                    MockStateReportGenerator(function<string()> reportFunction) : StateReportGenerator({reportFunction}) {}
                };
                class ReportStateHandlerTest : public Test {
                protected:
                    void SetUp() override {
                        m_customerDataManager = make_shared<CustomerDataManager>();
                        m_mockExceptionEncounteredSender = make_shared<MockExceptionEncounteredSender>();
                        m_mockAVSConnectionManager = make_shared<NiceMock<MockAVSConnectionManager>>();
                        m_mockMessageSender = make_shared<MockMessageSender>();
                        m_stubMiscStorage = StubMiscStorage::create();
                        m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                        m_mockDirectiveHandlerResult = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
                    }
                    void waitUntilEventSent() {
                        unique_lock<mutex> lock(m_mutex);
                        ASSERT_TRUE(m_condition.wait_for(lock, seconds(2), [this] { return m_directiveCompleted && m_eventSent; }));
                    }
                    shared_ptr<ReportStateHandler> m_unit;
                    shared_ptr<CustomerDataManager> m_customerDataManager;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
                    shared_ptr<MockMessageSender> m_mockMessageSender;
                    shared_ptr<::testing::NiceMock<MockAVSConnectionManager>> m_mockAVSConnectionManager;
                    shared_ptr<StubMiscStorage> m_stubMiscStorage;
                    vector<StateReportGenerator> generators;
                    shared_ptr<AttachmentManager> m_attachmentManager;
                    unique_ptr<MockDirectiveHandlerResult> m_mockDirectiveHandlerResult;
                    mutex m_mutex;
                    condition_variable m_condition;
                    bool m_directiveCompleted = false;
                    bool m_eventSent = false;
                    string m_eventJson;
                    shared_ptr<AVSDirective> createDirective() {
                        auto avsMessageHeader = make_shared<AVSMessageHeader>("System", "ReportState", TEST_MESSAGE_ID, TEST_REQUEST_ID);
                        auto directive = AVSDirective::create("", avsMessageHeader, "", m_attachmentManager, TEST_CONTEXT_ID);
                        return move(directive);
                    }
                };
                TEST_F(ReportStateHandlerTest, testReportState) {
                    void* leak = new int[23];
                    leak = nullptr;
                    ASSERT_EQ(nullptr, leak);
                    MockStateReportGenerator mockGenerator([] { return R"({"unitTest":"ON","complaints":"OFF"})"; });
                    generators.push_back(mockGenerator);
                    m_unit = ReportStateHandler::create(m_customerDataManager,m_mockExceptionEncounteredSender,
                                                        m_mockAVSConnectionManager,m_mockMessageSender,
                                                        m_stubMiscStorage,generators);
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([this] {
                        lock_guard<mutex> lock(m_mutex);
                        m_directiveCompleted = true;
                        m_condition.notify_all();
                    }));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(testing::_))
                        .WillOnce(WithArg<0>(Invoke([this](shared_ptr<MessageRequest> request) {
                            lock_guard<mutex> lock(m_mutex);
                            m_eventSent = true;
                            m_eventJson = request->getJsonContent();
                            request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS);
                            m_condition.notify_all();
                        })));
                    m_unit->CapabilityAgent::preHandleDirective(createDirective(), std::move(m_mockDirectiveHandlerResult));
                    m_unit->CapabilityAgent::handleDirective(TEST_MESSAGE_ID);
                    waitUntilEventSent();
                    ASSERT_TRUE(m_eventJson.find(R"("unitTest":"ON")") != m_eventJson.npos);
                }
            }
        }
    }
}