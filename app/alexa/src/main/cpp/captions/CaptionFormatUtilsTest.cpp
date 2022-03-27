#include <gtest/gtest.h>
#include "CaptionFormat.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace std;
            TEST(CaptionFormatUtilsTest, test_WebvttCaptionTypePutToString) {
                std::stringstream out;
                out << CaptionFormat::WEBVTT;
                ASSERT_EQ(out.str(), "WEBVTT");
            }
            TEST(CaptionFormatUtilsTest, test_UnkownCaptionTypePutToString) {
                std::stringstream out;
                out << CaptionFormat::UNKNOWN;
                ASSERT_EQ(out.str(), "UNKNOWN");
            }
        }
    }
}