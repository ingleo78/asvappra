#include <gtest/gtest.h>
#include "Alarm.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace testing;
            using namespace utils;
            static const string ALARM_DEFAULT_DATA = "alarm default data";
            static const string ALARM_SHORT_DATA = "alarm short data";
            class AlarmAlertTest : public Test {
            public:
                AlarmAlertTest();
                static pair<unique_ptr<istream>, const MediaType> alarmDefaultFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(ALARM_DEFAULT_DATA)),MediaType::MPEG);
                }
                static pair<unique_ptr<istream>, const MediaType> alarmShortFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(ALARM_SHORT_DATA)),MediaType::MPEG);
                }
                shared_ptr<Alarm> m_alarm;
            };
            AlarmAlertTest::AlarmAlertTest() : m_alarm{ make_shared<Alarm>(alarmDefaultFactory, alarmShortFactory, nullptr) } {
            }
            TEST_F(AlarmAlertTest, test_defaultAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_alarm->getDefaultAudioFactory()());
                oss << audioStream->rdbuf();
                ASSERT_EQ(ALARM_DEFAULT_DATA, oss.str());
            }
            TEST_F(AlarmAlertTest, test_shortAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_alarm->getShortAudioFactory()());
                oss << audioStream->rdbuf();

                ASSERT_EQ(ALARM_SHORT_DATA, oss.str());
            }
            TEST_F(AlarmAlertTest, test_getTypeName) {
                ASSERT_EQ(m_alarm->getTypeName(), Alarm::getTypeNameStatic());
            }
        }
    }
}