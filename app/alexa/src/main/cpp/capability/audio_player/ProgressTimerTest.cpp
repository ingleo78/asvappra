#include <gmock/gmock.h>
#include <util/PromiseFuturePair.h>
#include <timing/Stopwatch.h>
#include "ProgressTimer.h"

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace utils;
            using namespace timing;
            using namespace testing;
            static const milliseconds MILLIS_10{10};
            static const milliseconds MILLIS_25{25};
            static const milliseconds MILLIS_100{100};
            static const seconds FAIL_TIMEOUT{5};
            static const milliseconds OFFSET_TEST_DELAY{300};
            static const milliseconds OFFSET_TEST_INTERVAL{500};
            static const milliseconds LOWER_ERROR{100};
            static const milliseconds UPPER_ERROR{200};
            static void verifyOffset(milliseconds expected, milliseconds received) {
                ASSERT_GE(received, expected - LOWER_ERROR);
                ASSERT_LE(received, expected + UPPER_ERROR);
            };
            class MockContext : public ProgressTimer::ContextInterface {
            public:
                MOCK_METHOD0(requestProgress, void());
                MOCK_METHOD0(onProgressReportDelayElapsed, void());
                MOCK_METHOD0(onProgressReportIntervalElapsed, void());
                MOCK_METHOD0(onProgressReportIntervalUpdated, void());
            };
            class ProgressTimerTest : public Test {
            public:
                void SetUp() override;
                void play();
                void pause();
                void resume();
                void stop();
                void callOnProgress();
                shared_ptr<MockContext> m_mockContext;
                ProgressTimer m_timer;
                Stopwatch m_stopwatch;
            };
            void ProgressTimerTest::SetUp() {
                m_mockContext = make_shared<NiceMock<MockContext>>();
            }
            void ProgressTimerTest::play() {
                ASSERT_TRUE(m_stopwatch.start());
                m_timer.start();
            }
            void ProgressTimerTest::pause() {
                ASSERT_TRUE(m_stopwatch.pause());
                m_timer.pause();
            }
            void ProgressTimerTest::resume() {
                ASSERT_TRUE(m_stopwatch.resume());
                m_timer.resume();
            }
            void ProgressTimerTest::stop() {
                m_stopwatch.stop();
                m_timer.stop();
            }
            void ProgressTimerTest::callOnProgress() {
                auto progress = m_stopwatch.getElapsed();
                m_timer.onProgress(progress);
            }
            TEST_F(ProgressTimerTest, test_noDelayOrInterval) {
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(0);
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(0);
                m_timer.init(m_mockContext, ProgressTimer::getNoDelay(), ProgressTimer::getNoInterval());
                play();
                this_thread::sleep_for(MILLIS_100);
                stop();
            }
            TEST_F(ProgressTimerTest, test_zeroInterval) {
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(0);
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(0);
                m_timer.init(m_mockContext, ProgressTimer::getNoDelay(), std::chrono::milliseconds{0});
                play();
                this_thread::sleep_for(MILLIS_100);
                stop();
            }
            TEST_F(ProgressTimerTest, test_justDelay) {
                auto requestProgress = [this] { callOnProgress(); };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(1);
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(0);
                m_timer.init(m_mockContext, MILLIS_10, ProgressTimer::getNoInterval());
                play();
                this_thread::sleep_for(MILLIS_100);
                stop();
            }
            TEST_F(ProgressTimerTest, test_justInterval) {
                auto requestProgress = [this] { callOnProgress(); };
                int reportCounter = 0;
                PromiseFuturePair<void> gotTenReports;
                auto notifyOnTenReports = [&reportCounter, &gotTenReports]() {
                    if (10 == ++reportCounter) gotTenReports.setValue();
                };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(0);
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).WillRepeatedly(Invoke(notifyOnTenReports));
                m_timer.init(m_mockContext, ProgressTimer::getNoDelay(), MILLIS_10);
                play();
                ASSERT_TRUE(gotTenReports.waitFor(FAIL_TIMEOUT));
                stop();
            }
            TEST_F(ProgressTimerTest, test_delayAndInterval) {
                auto requestProgress = [this] { callOnProgress(); };
                int reportCounter = 0;
                PromiseFuturePair<void> gotTenReports;
                auto notifyOnTenReports = [&reportCounter, &gotTenReports]() {
                    if (10 == ++reportCounter) gotTenReports.setValue();
                };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                {
                    InSequence sequence;
                    EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(2).WillRepeatedly(Invoke(notifyOnTenReports)).RetiresOnSaturation();
                    EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(1);
                    EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).WillRepeatedly(Invoke(notifyOnTenReports));
                }
                m_timer.init(m_mockContext, MILLIS_25, MILLIS_10);
                play();
                ASSERT_TRUE(gotTenReports.waitFor(FAIL_TIMEOUT));
                stop();
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(0);
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(0);
                std::this_thread::sleep_for(MILLIS_100);
            }
            TEST_F(ProgressTimerTest, test_pause) {
                auto requestProgress = [this] { callOnProgress(); };
                mutex counterMutex;
                condition_variable wake;
                int reportCounter = 0;
                auto notifyOnTenReports = [&counterMutex, &wake, &reportCounter]() {
                    std::lock_guard<std::mutex> lock(counterMutex);
                    if (10 == ++reportCounter) wake.notify_all();
                };
                auto wakeOnTenReports = [&counterMutex, &wake, &reportCounter] {
                    std::unique_lock<std::mutex> lock(counterMutex);
                    ASSERT_TRUE(wake.wait_for(lock, FAIL_TIMEOUT, [&reportCounter] { return reportCounter >= 10; }));
                };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(1).RetiresOnSaturation();
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).WillRepeatedly(Invoke(notifyOnTenReports));
                m_timer.init(m_mockContext, MILLIS_10, MILLIS_10);
                play();
                wakeOnTenReports();
                for (int ix = 0; ix < 2; ix++) {
                    pause();
                    EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(0);
                    EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(0);
                    std::this_thread::sleep_for(MILLIS_100);
                    reportCounter = 0;
                    EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).WillRepeatedly(Invoke(notifyOnTenReports));
                    resume();
                    wakeOnTenReports();
                }
                stop();
            }
            TEST_F(ProgressTimerTest, test_resumeDoesNotRepeat) {
                auto requestProgress = [this] { callOnProgress(); };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(1);
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(0);
                m_timer.init(m_mockContext, MILLIS_10, ProgressTimer::getNoInterval());
                play();
                this_thread::sleep_for(MILLIS_100);
                pause();
                this_thread::sleep_for(MILLIS_100);
                resume();
                this_thread::sleep_for(MILLIS_100);
                stop();
            }
            TEST_F(ProgressTimerTest, testTimer_offsets) {
                auto requestProgress = [this] { callOnProgress(); };
                auto verifyDelayOffset = [this]() { verifyOffset(OFFSET_TEST_DELAY, m_stopwatch.getElapsed()); };
                int reportCounter = 0;
                PromiseFuturePair<void> gotTenReports;
                auto notifyOnTenReports = [this, &reportCounter, &gotTenReports]() {
                    ++reportCounter;
                    verifyOffset(reportCounter * OFFSET_TEST_INTERVAL, m_stopwatch.getElapsed());
                    if (3 == reportCounter) gotTenReports.setValue();
                };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(1).WillOnce(Invoke(verifyDelayOffset));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).WillRepeatedly(Invoke(notifyOnTenReports));
                m_timer.init(m_mockContext, OFFSET_TEST_DELAY, OFFSET_TEST_INTERVAL);
                play();
                ASSERT_TRUE(gotTenReports.waitFor(FAIL_TIMEOUT));
                stop();
            }
            TEST_F(ProgressTimerTest, test_delayAndIntervalCoincide) {
                auto requestProgress = [this] { callOnProgress(); };
                PromiseFuturePair<void> gotReport;
                auto notifyGotReport = [&gotReport]() { gotReport.setValue(); };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportDelayElapsed()).Times(1);
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(1).WillOnce(Invoke(notifyGotReport));
                m_timer.init(m_mockContext, MILLIS_100, MILLIS_100);
                play();
                ASSERT_TRUE(gotReport.waitFor(FAIL_TIMEOUT));
                stop();
            }
            TEST_F(ProgressTimerTest, test_updateInterval) {
                auto requestProgress = [this] { callOnProgress(); };
                PromiseFuturePair<void> oldIntervalReport;
                auto notifyOldReport = [this, &oldIntervalReport]() {
                    verifyOffset(OFFSET_TEST_INTERVAL, m_stopwatch.getElapsed());
                    oldIntervalReport.setValue();
                };
                EXPECT_CALL(*(m_mockContext.get()), requestProgress()).WillRepeatedly(Invoke(requestProgress));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).Times(1).WillOnce(Invoke(notifyOldReport));
                m_timer.init(m_mockContext, ProgressTimer::getNoDelay(), OFFSET_TEST_INTERVAL);
                play();
                ASSERT_TRUE(oldIntervalReport.waitFor(FAIL_TIMEOUT));
                PromiseFuturePair<void> newIntervalReport;
                auto notifyNewReport = [this, &newIntervalReport]() {
                    verifyOffset(OFFSET_TEST_INTERVAL + MILLIS_100, m_stopwatch.getElapsed());
                    newIntervalReport.setValue();
                };
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalElapsed()).WillRepeatedly(Invoke(notifyNewReport));
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalUpdated()).Times(1);
                m_timer.updateInterval(MILLIS_100);
                ASSERT_TRUE(newIntervalReport.waitFor(FAIL_TIMEOUT));
                stop();
            }
            TEST_F(ProgressTimerTest, test_updateIntervalAfterStop) {
                m_timer.init(m_mockContext, ProgressTimer::getNoDelay(), OFFSET_TEST_INTERVAL);
                play();
                stop();
                EXPECT_CALL(*(m_mockContext.get()), onProgressReportIntervalUpdated()).Times(0);
                m_timer.updateInterval(MILLIS_100);
            }
        }
    }
}