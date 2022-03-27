#include <gtest/gtest.h>
#include "CaptionFormat.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace test {
                using namespace captions;
                TEST(CaptionFormatAvsTest, test_parseWebvttCaptionTypeFromString) {
                    ASSERT_EQ(CaptionFormat::WEBVTT, avsStringToCaptionFormat("WEBVTT"));
                }
                TEST(CaptionFormatAvsTest, test_parseUnkownCaptionTypeFromString) {
                    ASSERT_EQ(CaptionFormat::UNKNOWN, avsStringToCaptionFormat("FOO"));
                }
                TEST(CaptionFormatAvsTest, test_parseUnkownCaptionTypeFromUnknownString) {
                    ASSERT_EQ(CaptionFormat::UNKNOWN, avsStringToCaptionFormat(""));
                }
            }
        }
    }
}