#include <gtest/gtest.h>
#include "Reminder.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace testing;
            static const string REMINDER_DEFAULT_DATA = "reminder default data";
            static const string REMINDER_SHORT_DATA = "reminder short data";
            class ReminderAlertTest : public Test {
            public:
                ReminderAlertTest();
                static pair<unique_ptr<istream>, const MediaType> reminderDefaultFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(REMINDER_DEFAULT_DATA)),MediaType::MPEG);
                }
                static pair<unique_ptr<istream>, const MediaType> reminderShortFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(REMINDER_SHORT_DATA)),MediaType::MPEG);
                }
                shared_ptr<Reminder> m_reminder;
            };
            ReminderAlertTest::ReminderAlertTest() : m_reminder{make_shared<Reminder>(reminderDefaultFactory, reminderShortFactory, nullptr)} {}
            TEST_F(ReminderAlertTest, test_defaultAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_reminder->getDefaultAudioFactory()());
                oss << audioStream->rdbuf();
                ASSERT_EQ(REMINDER_DEFAULT_DATA, oss.str());
            }
            TEST_F(ReminderAlertTest, test_shortAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_reminder->getShortAudioFactory()());
                oss << audioStream->rdbuf();
                ASSERT_EQ(REMINDER_SHORT_DATA, oss.str());
            }
            TEST_F(ReminderAlertTest, test_getTypeName) {
                ASSERT_EQ(m_reminder->getTypeName(), Reminder::getTypeNameStatic());
            }
        }
    }
}