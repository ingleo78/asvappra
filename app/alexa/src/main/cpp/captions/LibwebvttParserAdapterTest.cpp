#include <gtest/gtest.h>
#include <chrono>
#include <logger/Logger.h>
#include <media_player/MockMediaPlayer.h>
#include "CaptionData.h"
#include "CaptionFormat.h"
#include "CaptionFrame.h"
#include "LibwebvttParserAdapter.h"
#include "TextStyle.h"
#include "MockCaptionManager.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace testing;
            using namespace logger;
            using namespace mediaPlayer::test;
        #ifndef ENABLE_CAPTIONS
            class LibwebvttParserAdapterTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                shared_ptr<CaptionParserInterface> m_libwebvttParser;
                shared_ptr<MockCaptionManager> m_mockCaptionManager;
            };
            void LibwebvttParserAdapterTest::SetUp() {
                getConsoleLogger()->setLevel(logger::Level::DEBUG9);
                m_libwebvttParser = LibwebvttParserAdapter::getInstance();
                m_mockCaptionManager = make_shared<NiceMock<MockCaptionManager>>();
            }
            void LibwebvttParserAdapterTest::TearDown() {
                m_libwebvttParser->addListener(nullptr);
            }
            TEST_F(LibwebvttParserAdapterTest, test_createWithNullArgs) {
                m_libwebvttParser->addListener(m_mockCaptionManager);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(_)).Times(0);
                const CaptionData inputData = CaptionData(CaptionFormat::WEBVTT, "");
                m_libwebvttParser->parse(0, inputData);
                m_libwebvttParser->releaseResourcesFor(0);
            }
            TEST_F(LibwebvttParserAdapterTest, test_parseSingleCaptionFrame) {
                m_libwebvttParser->addListener(m_mockCaptionManager);
                vector<TextStyle> expectedStyles;
                Style s0 = Style();
                expectedStyles.emplace_back(TextStyle{0, s0});
                vector<CaptionLine> expectedCaptionLines;
                expectedCaptionLines.emplace_back(CaptionLine{"The time is 2:17 PM.", expectedStyles});
                CaptionFrame expectedCaptionFrame = CaptionFrame(123, milliseconds(1260), milliseconds(0),
                                                                 expectedCaptionLines);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(expectedCaptionFrame)).Times(1);
                const string webvttContent = "WEBVTT\n\n1\n00:00.000 --> 00:01.260\nThe time is 2:17 PM.";
                const CaptionData inputData = CaptionData(CaptionFormat::WEBVTT, webvttContent);
                m_libwebvttParser->parse(123, inputData);
                m_libwebvttParser->releaseResourcesFor(123);
            }
            TEST_F(LibwebvttParserAdapterTest, test_parseMultipleCaptionFrames) {
                m_libwebvttParser->addListener(m_mockCaptionManager);
                vector<TextStyle> frame1_expectedStyles;
                frame1_expectedStyles.emplace_back(TextStyle{0, Style()});
                vector<CaptionLine> frame1_expectedCaptionLines;
                frame1_expectedCaptionLines.emplace_back(CaptionLine{"The time is 2:17 PM.", frame1_expectedStyles});
                CaptionFrame frame1_expectedCaptionFrame = CaptionFrame(101, milliseconds(1260), milliseconds(0),
                                                                        frame1_expectedCaptionLines);
                vector<TextStyle> frame2_expectedStyles;
                frame2_expectedStyles.emplace_back(TextStyle{0, Style()});
                vector<CaptionLine> frame2_expectedCaptionLines;
                frame2_expectedCaptionLines.emplace_back(CaptionLine{"Never drink liquid nitrogen.", frame2_expectedStyles});
                CaptionFrame frame2_expectedCaptionFrame = CaptionFrame(102, milliseconds(3000), milliseconds(1000),
                                                                        frame2_expectedCaptionLines);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(frame1_expectedCaptionFrame)).Times(1);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(frame2_expectedCaptionFrame)).Times(1);
                const string frame1_webvttContent = "WEBVTT\n\n1\n00:00.000 --> 00:01.260\nThe time is 2:17 PM.";
                const CaptionData frame1_inputData = CaptionData(CaptionFormat::WEBVTT, frame1_webvttContent);
                const string frame2_webvttContent = "WEBVTT\n\n00:01.000 --> 00:04.000\nNever drink liquid nitrogen.";
                const CaptionData frame2_inputData = CaptionData(CaptionFormat::WEBVTT, frame2_webvttContent);
                m_libwebvttParser->parse(101, frame1_inputData);
                m_libwebvttParser->parse(102, frame2_inputData);
                m_libwebvttParser->releaseResourcesFor(101);
                m_libwebvttParser->releaseResourcesFor(102);
            }
            TEST_F(LibwebvttParserAdapterTest, test_parseSingleCaptionDataToCaptionFrames) {
                m_libwebvttParser->addListener(m_mockCaptionManager);
                std::vector<CaptionLine> frame1_expectedCaptionLines;
                frame1_expectedCaptionLines.emplace_back(CaptionLine{"Never drink liquid nitrogen.", {TextStyle{0, Style()}}});
                CaptionFrame frame1_expectedCaptionFrame = CaptionFrame(101, milliseconds(3000), milliseconds(0),
                                                                        frame1_expectedCaptionLines);
                std::vector<CaptionLine> frame2_expectedCaptionLines;
                frame2_expectedCaptionLines.emplace_back(CaptionLine{"- It will perforate your stomach.", {TextStyle{0, Style()}}});
                frame2_expectedCaptionLines.emplace_back(CaptionLine{"- You could die.", {TextStyle{0, Style()}}});
                CaptionFrame frame2_expectedCaptionFrame = CaptionFrame(101, milliseconds(4000), milliseconds(0),
                                                                        frame2_expectedCaptionLines);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(frame1_expectedCaptionFrame)).Times(1);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(frame2_expectedCaptionFrame)).Times(1);
                const string webvttContent = "WEBVTT\n\n00:00.000 --> 00:03.000\nNever drink liquid nitrogen.\n\n"
                                             "00:03.000 --> 00:07.000\n- It will perforate your stomach.\n- You could die.";
                const CaptionData inputData = CaptionData(CaptionFormat::WEBVTT, webvttContent);
                m_libwebvttParser->parse(101, inputData);
                m_libwebvttParser->releaseResourcesFor(101);
            }
            TEST_F(LibwebvttParserAdapterTest, test_parseDelayBetweenCaptionFrames) {
                m_libwebvttParser->addListener(m_mockCaptionManager);
                vector<CaptionLine> frame1_expectedCaptionLines;
                frame1_expectedCaptionLines.emplace_back(CaptionLine{"Never drink liquid nitrogen.", {TextStyle{0, Style()}}});
                CaptionFrame frame1_expectedCaptionFrame = CaptionFrame(101, milliseconds(3000), milliseconds(1000),
                                                                        frame1_expectedCaptionLines);
                vector<CaptionLine> frame2_expectedCaptionLines;
                frame2_expectedCaptionLines.emplace_back(CaptionLine{"- It will perforate your stomach.", {TextStyle{0, Style()}}});
                frame2_expectedCaptionLines.emplace_back(CaptionLine{"- You could die.", {TextStyle{0, Style()}}});
                CaptionFrame frame2_expectedCaptionFrame = CaptionFrame(101, milliseconds(4000), milliseconds(1000),
                                                                        frame2_expectedCaptionLines);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(frame1_expectedCaptionFrame)).Times(1);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(frame2_expectedCaptionFrame)).Times(1);
                const string webvttContent = "WEBVTT\n\n00:01.000 --> 00:04.000\nNever drink liquid nitrogen.\n\n"
                                             "00:05.000 --> 00:09.000\n- It will perforate your stomach.\n- You could die.";
                const CaptionData inputData = CaptionData(CaptionFormat::WEBVTT, webvttContent);
                m_libwebvttParser->parse(101, inputData);
                m_libwebvttParser->releaseResourcesFor(101);
            }
            TEST_F(LibwebvttParserAdapterTest, test_parseBoldTagToStyle) {
                m_libwebvttParser->addListener(m_mockCaptionManager);
                vector<TextStyle> expectedStyles;
                Style s0 = Style();
                expectedStyles.emplace_back(TextStyle{0, s0});
                Style s1 = Style();
                s1.m_bold = true;
                expectedStyles.emplace_back(TextStyle{4, s1});
                Style s2 = Style();
                s2.m_bold = false;
                expectedStyles.emplace_back(TextStyle{8, s2});
                vector<CaptionLine> expectedCaptionLines;
                expectedCaptionLines.emplace_back(CaptionLine{"The time is 2:17 PM.", expectedStyles});
                CaptionFrame expectedCaptionFrame = CaptionFrame(123, milliseconds(1260), milliseconds(0),
                                                                 expectedCaptionLines);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(expectedCaptionFrame)).Times(1);
                const string webvttContent = "WEBVTT\n\n1\n00:00.000 --> 00:01.260\nThe <b>time</b> is 2:17 PM.";
                const CaptionData inputData = CaptionData(CaptionFormat::WEBVTT, webvttContent);
                m_libwebvttParser->parse(123, inputData);
                m_libwebvttParser->releaseResourcesFor(123);
            }
            TEST_F(LibwebvttParserAdapterTest, test_parseItalicTagToStyle) {
                m_libwebvttParser->addListener(m_mockCaptionManager);
                vector<TextStyle> expectedStyles;
                Style s0 = Style();
                expectedStyles.emplace_back(TextStyle{0, s0});
                Style s1 = Style();
                s1.m_italic = true;
                expectedStyles.emplace_back(TextStyle{4, s1});
                Style s2 = Style();
                s2.m_italic = false;
                expectedStyles.emplace_back(TextStyle{8, s2});
                vector<CaptionLine> expectedCaptionLines;
                expectedCaptionLines.emplace_back(CaptionLine{"The time is 2:17 PM.", expectedStyles});
                CaptionFrame expectedCaptionFrame = CaptionFrame(123, milliseconds(1260), milliseconds(0),
                                                                 expectedCaptionLines);
                EXPECT_CALL(*(m_mockCaptionManager.get()), onParsed(expectedCaptionFrame)).Times(1);
                const string webvttContent = "WEBVTT\n\n1\n00:00.000 --> 00:01.260\nThe <i>time</i> is 2:17 PM.";
                const CaptionData inputData = CaptionData(CaptionFormat::WEBVTT, webvttContent);
                m_libwebvttParser->parse(123, inputData);
                m_libwebvttParser->releaseResourcesFor(123);
            }
        #endif
        }
    }
}