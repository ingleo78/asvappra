#include <gtest/gtest.h>
#include "CaptionLine.h"
#include "TextStyle.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace std;
            TEST(CaptionLineTest, test_noStylesSplitIndexZero) {
                CaptionLine c1 = CaptionLine("The time is 2:17 PM.", {});
                auto splitC1 = c1.splitAtTextIndex(0);
                ASSERT_EQ(splitC1[0], CaptionLine("", {}));
                ASSERT_EQ(splitC1[1], CaptionLine("The time is 2:17 PM.", {}));
            }
            TEST(CaptionLineTest, test_noStylesSplitIndexOutOfBounds) {
                CaptionLine c1 = CaptionLine("The time is 2:17 PM.", {});
                auto splitC1 = c1.splitAtTextIndex(100);
                ASSERT_EQ(splitC1.size(), (size_t)1);
                ASSERT_EQ(splitC1[0], CaptionLine("The time is 2:17 PM.", {}));
            }
            TEST(CaptionLineTest, test_putToOnEmptyCaptionLine) {
                CaptionLine c1 = CaptionLine();
                std::stringstream out;
                out << c1;
                ASSERT_EQ(out.str(), "CaptionLine(text:\"\", styles:[])");
            }
            TEST(CaptionLineTest, test_singleStyleSplit) {
                vector<TextStyle> styles;
                Style s0 = Style();
                styles.emplace_back(TextStyle(0, s0));
                CaptionLine line1 = CaptionLine("Currently, in Ashburn it's 73 degrees Fahrenheit with clear skies.", styles);
                auto splitLine1 = line1.splitAtTextIndex(66);
                vector<TextStyle> expectedFirstLineStyles;
                Style firstLineStyle1 = Style();
                expectedFirstLineStyles.emplace_back(TextStyle(0, firstLineStyle1));
                vector<TextStyle> expectedSecondLineStyles;
                Style secondLineStyle1 = Style();
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle1));
                ASSERT_EQ(splitLine1[0],CaptionLine("Currently, in Ashburn it's 73 degrees Fahrenheit with clear skies.", expectedFirstLineStyles));
                ASSERT_EQ(splitLine1[1], CaptionLine("", expectedSecondLineStyles));
            }
            TEST(CaptionLineTest, test_singleStyleSplitBeforeWhitespace) {
                vector<TextStyle> styles;
                Style s0 = Style();
                styles.emplace_back(TextStyle(0, s0));
                Style s1 = Style();
                s1.m_bold = true;
                styles.emplace_back(TextStyle(4, s1));
                Style s2 = Style();
                s2.m_bold = false;
                styles.emplace_back(TextStyle(8, s2));
                CaptionLine line1 = CaptionLine("The time is 2:17 PM.", styles);
                auto splitLine1 = line1.splitAtTextIndex(3);
                vector<TextStyle> expectedFirstLineStyles;
                Style firstLineStyle1 = Style();
                expectedFirstLineStyles.emplace_back(TextStyle(0, firstLineStyle1));
                vector<TextStyle> expectedSecondLineStyles;
                Style secondLineStyle1 = Style();
                secondLineStyle1.m_bold = true;
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle1));
                Style secondLineStyle2 = Style();
                secondLineStyle2.m_bold = false;
                expectedSecondLineStyles.emplace_back(TextStyle(4, secondLineStyle2));
                ASSERT_EQ(splitLine1[0], CaptionLine("The", expectedFirstLineStyles));
                ASSERT_EQ(splitLine1[1], CaptionLine("time is 2:17 PM.", expectedSecondLineStyles));
            }
            TEST(CaptionLineTest, test_indexAdjustmentWithWhitespaceContent) {
                vector<TextStyle> styles;
                Style s0 = Style();
                styles.emplace_back(TextStyle(0, s0));
                Style s1 = Style();
                s1.m_bold = true;
                styles.emplace_back(TextStyle(1, s1));
                Style s2 = Style();
                s2.m_bold = false;
                styles.emplace_back(TextStyle(3, s2));
                CaptionLine line1 = CaptionLine("                    ", styles);
                auto splitLine1 = line1.splitAtTextIndex(1);
                vector<TextStyle> expectedFirstLineStyles;
                Style firstLineStyle1 = Style();
                expectedFirstLineStyles.emplace_back(TextStyle(0, firstLineStyle1));
                std::vector<TextStyle> expectedSecondLineStyles;
                Style secondLineStyle1 = Style();
                secondLineStyle1.m_bold = true;
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle1));
                Style secondLineStyle2 = Style();
                secondLineStyle2.m_bold = false;
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle2));
                ASSERT_EQ(splitLine1[0], CaptionLine(" ", expectedFirstLineStyles));
                ASSERT_EQ(splitLine1[1], CaptionLine("", expectedSecondLineStyles));
            }
            TEST(CaptionLineTest, test_indexAdjustmentWithSeveralWhitespacesBeforeContent) {
                vector<TextStyle> styles;
                Style s0 = Style();
                styles.emplace_back(TextStyle(0, s0));
                Style s1 = Style();
                s1.m_bold = true;
                styles.emplace_back(TextStyle(34, s1));
                Style s2 = Style();
                s2.m_bold = false;
                styles.emplace_back(TextStyle(45, s2));
                CaptionLine line1 = CaptionLine("No styles here                    bolded here", styles);
                auto splitLine1 = line1.splitAtTextIndex(16);
                vector<TextStyle> expectedFirstLineStyles;
                Style firstLineStyle1 = Style();
                expectedFirstLineStyles.emplace_back(TextStyle(0, firstLineStyle1));
                vector<TextStyle> expectedSecondLineStyles;
                Style secondLineStyle1 = Style();
                secondLineStyle1.m_bold = true;
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle1));
                Style secondLineStyle2 = Style();
                secondLineStyle2.m_bold = false;
                expectedSecondLineStyles.emplace_back(TextStyle(11, secondLineStyle2));
                ASSERT_EQ(splitLine1[0], CaptionLine("No styles here  ", expectedFirstLineStyles));
                ASSERT_EQ(splitLine1[1], CaptionLine("bolded here", expectedSecondLineStyles));
            }
            TEST(CaptionLineTest, test_singleStyleSplitAfter) {
                vector<TextStyle> styles;
                Style s0 = Style();
                styles.emplace_back(TextStyle(0, s0));
                Style s1 = Style();
                s1.m_bold = true;
                styles.emplace_back(TextStyle(4, s1));
                Style s2 = Style();
                s2.m_bold = false;
                styles.emplace_back(TextStyle(8, s2));
                CaptionLine line1 = CaptionLine("The time is 2:17 PM.", styles);
                auto splitLine1 = line1.splitAtTextIndex(9);
                vector<TextStyle> expectedFirstLineStyles;
                Style firstLineStyle1 = Style();
                expectedFirstLineStyles.emplace_back(TextStyle(0, firstLineStyle1));
                Style firstLineStyle2 = Style();
                firstLineStyle2.m_bold = true;
                expectedFirstLineStyles.emplace_back(TextStyle(4, firstLineStyle2));
                Style firstLineStyle3 = Style();
                firstLineStyle3.m_bold = false;
                expectedFirstLineStyles.emplace_back(TextStyle(8, firstLineStyle3));
                vector<TextStyle> expectedSecondLineStyles;
                Style secondLineStyle1 = Style();
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle1));
                ASSERT_EQ(splitLine1[0], CaptionLine("The time ", expectedFirstLineStyles));
                ASSERT_EQ(splitLine1[1], CaptionLine("is 2:17 PM.", expectedSecondLineStyles));
            }
            TEST(CaptionLineTest, test_singleStyleSplitMiddle) {
                vector<TextStyle> styles;
                Style s0 = Style();
                styles.emplace_back(TextStyle(0, s0));
                Style s1 = Style();
                s1.m_bold = true;
                styles.emplace_back(TextStyle(4, s1));
                Style s2 = Style();
                s2.m_bold = false;
                styles.emplace_back(TextStyle(8, s2));
                CaptionLine line1 = CaptionLine("The time is 2:17 PM.", styles);
                auto splitLine1 = line1.splitAtTextIndex(6);
                vector<TextStyle> expectedFirstLineStyles;
                Style firstLineStyle1 = Style();
                expectedFirstLineStyles.emplace_back(TextStyle(0, firstLineStyle1));
                Style firstLineStyle2 = Style();
                firstLineStyle2.m_bold = true;
                expectedFirstLineStyles.emplace_back(TextStyle(4, firstLineStyle2));
                vector<TextStyle> expectedSecondLineStyles;
                Style secondLineStyle1 = Style();
                secondLineStyle1.m_bold = true;
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle1));
                Style secondLineStyle2 = Style();
                secondLineStyle2.m_bold = false;
                expectedSecondLineStyles.emplace_back(TextStyle(2, secondLineStyle2));
                ASSERT_EQ(splitLine1[0], CaptionLine("The ti", expectedFirstLineStyles));
                ASSERT_EQ(splitLine1[1], CaptionLine("me is 2:17 PM.", expectedSecondLineStyles));
            }
            TEST(CaptionLineTest, test_multipleStyleSplitBefore) {
                vector<TextStyle> styles;
                Style s0 = Style();
                styles.emplace_back(TextStyle(0, s0));
                Style s1_open = Style();
                s1_open.m_bold = true;
                styles.emplace_back(TextStyle(4, s1_open));
                Style s1_close = Style();
                s1_close.m_bold = false;
                styles.emplace_back(TextStyle(8, s1_close));
                Style s2_open = Style();
                s2_open.m_italic = true;
                styles.emplace_back(TextStyle(12, s2_open));
                Style s2_close = Style();
                s2_close.m_italic = false;
                styles.emplace_back(TextStyle(19, s2_close));
                CaptionLine line1 = CaptionLine("The time is 2:17 PM.", styles);
                auto splitLine1 = line1.splitAtTextIndex(3);
                vector<TextStyle> expectedFirstLineStyles;
                Style firstLineStyle0 = Style();
                expectedFirstLineStyles.emplace_back(TextStyle(0, firstLineStyle0));
                vector<TextStyle> expectedSecondLineStyles;
                Style secondLineStyle1_open = Style();
                secondLineStyle1_open.m_bold = true;
                expectedSecondLineStyles.emplace_back(TextStyle(0, secondLineStyle1_open));
                Style secondLineStyle1_close = Style();
                secondLineStyle1_close.m_bold = false;
                expectedSecondLineStyles.emplace_back(TextStyle(4, secondLineStyle1_close));
                Style secondLineStyle2_open = Style();
                secondLineStyle2_open.m_italic = true;
                expectedSecondLineStyles.emplace_back(TextStyle(8, secondLineStyle2_open));
                Style secondLineStyle2_close = Style();
                secondLineStyle2_close.m_italic = false;
                expectedSecondLineStyles.emplace_back(TextStyle(15, secondLineStyle2_close));
                ASSERT_EQ(splitLine1[0], CaptionLine("The", expectedFirstLineStyles));
                ASSERT_EQ(splitLine1[1], CaptionLine("time is 2:17 PM.", expectedSecondLineStyles));
            }
            TEST(CaptionLineTest, test_emptySplit) {
                auto merged = CaptionLine::merge({});
                ASSERT_EQ(merged, CaptionLine());
            }
            TEST(CaptionLineTest, test_singleStyleMerge) {
                vector<TextStyle> firstLineStyles;
                Style firstLineStyle0 = Style();
                firstLineStyles.emplace_back(TextStyle(0, firstLineStyle0));
                auto inputLine = CaptionLine("The time is 2:17 PM.", firstLineStyles);
                auto mergedCaptionLines = CaptionLine::merge({inputLine});
                ASSERT_EQ(mergedCaptionLines, inputLine);
            }
            TEST(CaptionLineTest, test_missingStylesMerge) {
                auto inputLine = CaptionLine("The time is 2:17 PM.", {TextStyle()});
                auto mergedCaptionLines = CaptionLine::merge({inputLine});
                ASSERT_EQ(mergedCaptionLines, inputLine);
            }
            TEST(CaptionLineTest, test_firstLineMissingStylesMerge) {
                vector<TextStyle> firstLineStyles;
                auto inputLine1 = CaptionLine("The time ", firstLineStyles);
                vector<TextStyle> secondLineStyles;
                Style secondLineStyle0 = Style();
                secondLineStyle0.m_bold = true;
                secondLineStyles.emplace_back(TextStyle(0, secondLineStyle0));
                auto inputLine2 = CaptionLine("is 2:17 PM.", secondLineStyles);
                auto mergedCaptionLines = CaptionLine::merge({inputLine1, inputLine2});
                vector<TextStyle> expectedStyles;
                Style s0 = Style();
                expectedStyles.emplace_back(TextStyle(0, s0));
                Style s1 = Style();
                s1.m_bold = true;
                expectedStyles.emplace_back(TextStyle(9, s1));
                CaptionLine expectedLine1 = CaptionLine("The time is 2:17 PM.", expectedStyles);
                ASSERT_EQ(mergedCaptionLines, expectedLine1);
            }
            TEST(CaptionLineTest, test_multipleStyleMerge) {
                vector<TextStyle> firstLineStyles;
                Style firstLineStyle0 = Style();
                firstLineStyles.emplace_back(TextStyle(0, firstLineStyle0));
                vector<TextStyle> secondLineStyles;
                Style secondLineStyle1_open = Style();
                secondLineStyle1_open.m_bold = true;
                secondLineStyles.emplace_back(TextStyle(0, secondLineStyle1_open));
                Style secondLineStyle1_close = Style();
                secondLineStyle1_close.m_bold = false;
                secondLineStyles.emplace_back(TextStyle(4, secondLineStyle1_close));
                Style secondLineStyle2_open = Style();
                secondLineStyle2_open.m_italic = true;
                secondLineStyles.emplace_back(TextStyle(8, secondLineStyle2_open));
                Style secondLineStyle2_close = Style();
                secondLineStyle2_close.m_italic = false;
                secondLineStyles.emplace_back(TextStyle(15, secondLineStyle2_close));
                auto mergedCaptionLines = CaptionLine::merge({CaptionLine("The ", firstLineStyles), CaptionLine("time is 2:17 PM.", secondLineStyles)});
                vector<TextStyle> expectedStyles;
                Style s0 = Style();
                expectedStyles.emplace_back(TextStyle(0, s0));
                Style s1_open = Style();
                s1_open.m_bold = true;
                expectedStyles.emplace_back(TextStyle(4, s1_open));
                Style s1_close = Style();
                s1_close.m_bold = false;
                expectedStyles.emplace_back(TextStyle(8, s1_close));
                Style s2_open = Style();
                s2_open.m_italic = true;
                expectedStyles.emplace_back(TextStyle(12, s2_open));
                Style s2_close = Style();
                s2_close.m_italic = false;
                expectedStyles.emplace_back(TextStyle(19, s2_close));
                CaptionLine expectedLine1 = CaptionLine("The time is 2:17 PM.", expectedStyles);
                ASSERT_EQ(mergedCaptionLines, expectedLine1);
            }
        }
    }
}