#include <gtest/gtest.h>
#include "Timer.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace utils;
            using namespace testing;
            static const string TIMER_DEFAULT_DATA = "timer default data";
            static const string TIMER_SHORT_DATA = "timer short data";
            class TimerAlertTest : public Test {
            public:
                TimerAlertTest();
                static pair<unique_ptr<istream>, const MediaType> timerDefaultFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(TIMER_DEFAULT_DATA)),MediaType::MPEG);
                }
                static pair<unique_ptr<istream>, const MediaType> timerShortFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(TIMER_SHORT_DATA)),MediaType::MPEG);
                }
                shared_ptr<Timer> m_timer;
            };
            TimerAlertTest::TimerAlertTest() : m_timer{make_shared<Timer>(timerDefaultFactory, timerShortFactory, nullptr)} {}
            TEST_F(TimerAlertTest, test_defaultAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_timer->getDefaultAudioFactory()());
                oss << audioStream->rdbuf();
                ASSERT_EQ(TIMER_DEFAULT_DATA, oss.str());
            }
            TEST_F(TimerAlertTest, test_shortAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_timer->getShortAudioFactory()());
                oss << audioStream->rdbuf();
                ASSERT_EQ(TIMER_SHORT_DATA, oss.str());
            }
            TEST_F(TimerAlertTest, test_getTypeName) {
                ASSERT_EQ(m_timer->getTypeName(), Timer::getTypeNameStatic());
            }
        }
    }
}