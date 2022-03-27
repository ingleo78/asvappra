#include <gtest/gtest.h>
#include "CaptionData.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            TEST(CaptionDataTest, test_captionFormatUnknownIsInvalidWithNonBlankText) {
                CaptionData captionData = CaptionData(CaptionFormat::UNKNOWN, "Some unknown caption data");
                ASSERT_FALSE(captionData.isValid());
            }
            TEST(CaptionDataTest, test_captionFormatUnknownIsInvalidWithBlankText) {
                CaptionData captionData = CaptionData(CaptionFormat::UNKNOWN);
                ASSERT_FALSE(captionData.isValid());
            }
            TEST(CaptionDataTest, test_captionFormatWebvttIsValidWithNonBlankText) {
                CaptionData captionData = CaptionData(CaptionFormat::WEBVTT, "WEBVTT\n\n1\n00:00:00.000 --> 00:00:01.500\nTest for WebVTT format.");
                ASSERT_TRUE(captionData.isValid());
            }
            TEST(CaptionDataTest, test_captionFormatWebvttIsValidWithBlankText) {
                CaptionData captionData = CaptionData(CaptionFormat::WEBVTT);
                ASSERT_FALSE(captionData.isValid());
            }
        }
    }
}