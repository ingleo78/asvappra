#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <logger/Logger.h>
#include <util/WaitEvent.h>
#include "CaptionTimingAdapter.h"
#include "MockSystemClockDelay.h"
#include "MockCaptionPresenter.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace logger;
            using namespace testing;
            class CaptionTimingAdapterTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                shared_ptr<CaptionTimingAdapter> m_timingAdapter;
                shared_ptr<MockCaptionPresenter> m_mockCaptionPresenter;
                shared_ptr<MockSystemClockDelay> m_mockSystemClockDelay;
            };
            void CaptionTimingAdapterTest::SetUp() {
                getConsoleLogger()->setLevel(Level::DEBUG9);
                m_mockSystemClockDelay = make_shared<NiceMock<MockSystemClockDelay>>();
                m_mockCaptionPresenter = make_shared<NiceMock<MockCaptionPresenter>>();
                m_timingAdapter = shared_ptr<CaptionTimingAdapter>(new CaptionTimingAdapter(m_mockCaptionPresenter, m_mockSystemClockDelay));
            }
            void CaptionTimingAdapterTest::TearDown() {}
            TEST_F(CaptionTimingAdapterTest, test_queueForDisplayNotifiesPresenter) {
                WaitEvent finishedEvent;
                auto TIMEOUT = milliseconds(50);
                vector<CaptionLine> lines;
                CaptionLine line = CaptionLine("The time is 2:17 PM.", {});
                lines.emplace_back(line);
                auto captionFrame = CaptionFrame(1, milliseconds(1), milliseconds(0), lines);
                /*EXPECT_CALL(*(m_mockCaptionPresenter.get()), onCaptionActivity(captionFrame, FocusState::FOREGROUND)).Times(1);
                EXPECT_CALL(*(m_mockCaptionPresenter.get()), onCaptionActivity(_, FocusState::NONE)).Times(1)
                    .WillOnce(InvokeWithoutArgs([&finishedEvent]() { finishedEvent.wakeUp(); }));*/
                EXPECT_CALL(*(m_mockSystemClockDelay.get()), delay(milliseconds(0))).Times(1);
                EXPECT_CALL(*(m_mockSystemClockDelay.get()), delay(milliseconds(1))).Times(1);
                m_timingAdapter->queueForDisplay(captionFrame, true);
                EXPECT_TRUE(finishedEvent.wait(TIMEOUT));
            }
            TEST_F(CaptionTimingAdapterTest, test_queueForDisplayWithDelay) {
                WaitEvent finishedEvent;
                auto TIMEOUT = milliseconds(50);
                vector<CaptionLine> lines;
                CaptionLine line = CaptionLine("The time is 2:17 PM.", {});
                lines.emplace_back(line);
                auto captionFrame = CaptionFrame(1, milliseconds(10), milliseconds(5), lines);
                /*EXPECT_CALL(*(m_mockCaptionPresenter.get()), onCaptionActivity(captionFrame, FocusState::FOREGROUND)).Times(1);
                EXPECT_CALL(*(m_mockCaptionPresenter.get()), onCaptionActivity(_, FocusState::NONE)).Times(1)
                    .WillOnce(InvokeWithoutArgs([&finishedEvent]() { finishedEvent.wakeUp(); }));*/
                EXPECT_CALL(*(m_mockSystemClockDelay.get()), delay(milliseconds(5))).Times(1);
                EXPECT_CALL(*(m_mockSystemClockDelay.get()), delay(milliseconds(10))).Times(1);
                m_timingAdapter->queueForDisplay(captionFrame, true);
                EXPECT_TRUE(finishedEvent.wait(TIMEOUT));
            }
        }
    }
}