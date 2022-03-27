#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <logger/Logger.h>
#include <media_player/MockMediaPlayer.h>
#include "CaptionManager.h"
#include "CaptionLine.h"
#include "MockCaptionParser.h"
#include "MockCaptionPresenter.h"
#include "TestTimingAdapterFactory.cpp"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace testing;
            using namespace avs;
            using namespace mediaPlayer::test;
            class CaptionManagerTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                shared_ptr<CaptionManager> caption_manager;
                shared_ptr<MockMediaPlayer> m_player;
                shared_ptr<MockCaptionParser> m_parser;
                shared_ptr<NiceMock<MockCaptionPresenter>> m_presenter;
                shared_ptr<TestTimingAdapterFactory> m_timingFactory;
            };
            void CaptionManagerTest::SetUp() {
                getConsoleLogger()->setLevel(Level::DEBUG9);
                m_player = MockMediaPlayer::create();
                m_parser = make_shared<NiceMock<MockCaptionParser>>();
                m_presenter = make_shared<NiceMock<MockCaptionPresenter>>();
                m_timingFactory = make_shared<TestTimingAdapterFactory>();
                caption_manager = CaptionManager::create(m_parser, m_timingFactory);
                caption_manager->setMediaPlayers({m_player});
                caption_manager->setCaptionPresenter(m_presenter);
            }
            void CaptionManagerTest::TearDown() {
                if (m_player) m_player->shutdown();
                if (caption_manager) caption_manager->shutdown();
            }
            TEST_F(CaptionManagerTest, test_testTestTimingAdapterFactory) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                ASSERT_EQ(mockTimingAdapter, m_timingFactory->getTimingAdapter(nullptr));
            }
            TEST_F(CaptionManagerTest, test_testSetMediaPlayerBindsMediaPlayer) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                auto sourceID1 = m_player->setSource("http://fake.url", std::chrono::milliseconds(0));
                caption_manager->onParsed(CaptionFrame(sourceID1));
                EXPECT_CALL(*mockTimingAdapter, pause()).Times(1);
                m_player->mockPause(sourceID1);
            }
            TEST_F(CaptionManagerTest, test_createWithNullArgs) {
                auto manager = CaptionManager::create(nullptr, nullptr);
                ASSERT_FALSE(manager);
            }
            TEST_F(CaptionManagerTest, test_createWithNullTimingAdapterFactory) {
                auto manager = CaptionManager::create(m_parser, nullptr);
                ASSERT_NE(nullptr, manager);
            }
            TEST_F(CaptionManagerTest, test_createWithNullParser) {
                auto manager = CaptionManager::create(nullptr, m_timingFactory);
                ASSERT_FALSE(manager);
            }
            TEST_F(CaptionManagerTest, test_sourceIdDoesNotChange) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                int sourceID1 = 1;
                auto expectedCaptionFrame = CaptionFrame(sourceID1);
                EXPECT_CALL(*mockTimingAdapter.get(), queueForDisplay(expectedCaptionFrame, _)).Times(1);
                caption_manager->onParsed(CaptionFrame(sourceID1));
            }
            TEST_F(CaptionManagerTest, test_singleMediaPlayerPause) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                vector<CaptionLine> expectedLines;
                CaptionLine expectedLine1 = CaptionLine("The time is 2:17 PM.", {TextStyle()});
                expectedLines.emplace_back(expectedLine1);
                auto expectedCaptionFrame = CaptionFrame(1,milliseconds(1), milliseconds(0), expectedLines);
                EXPECT_CALL(*mockTimingAdapter.get(), queueForDisplay(expectedCaptionFrame, _)).Times(1);
                EXPECT_CALL(*m_presenter, getWrapIndex(_)).Times(1).WillOnce(Return(pair<bool, uint32_t>(false, 0)));
                vector<CaptionLine> lines;
                CaptionLine line = CaptionLine("The time is 2:17 PM.", {});
                lines.emplace_back(line);
                auto captionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), lines);
                caption_manager->onParsed(captionFrame);
            }
            TEST_F(CaptionManagerTest, test_splitCaptionFrameWhitespaceOnly) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                vector<CaptionLine> expectedLines;
                CaptionLine expectedLine1 = CaptionLine("     ", {TextStyle()});
                expectedLines.emplace_back(expectedLine1);
                auto expectedCaptionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), expectedLines);
                EXPECT_CALL(*mockTimingAdapter, queueForDisplay(expectedCaptionFrame, _)).Times(1);
                EXPECT_CALL(*m_presenter, getWrapIndex(_)).Times(1).WillOnce(Return(pair<bool, uint32_t>(false, 0)));
                vector<CaptionLine> lines;
                CaptionLine line = CaptionLine("     ", {});
                lines.emplace_back(line);
                auto captionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), lines);
                caption_manager->onParsed(captionFrame);
            }
            TEST_F(CaptionManagerTest, test_splitCaptionFrameWhitespaceAfterLineWrap) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                std::vector<CaptionLine> expectedLines;
                CaptionLine expectedLine1 = CaptionLine("The time is 2:17 PM.", {TextStyle()});
                expectedLines.emplace_back(expectedLine1);
                auto expectedCaptionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), expectedLines);
                EXPECT_CALL(*mockTimingAdapter, queueForDisplay(expectedCaptionFrame, _)).Times(1);
                EXPECT_CALL(*m_presenter, getWrapIndex(_)).Times(2).WillOnce(Return(pair<bool, uint32_t>(true, 20)))
                    .WillOnce(Return(pair<bool, uint32_t>(false, 0)));
                vector<CaptionLine> lines;
                CaptionLine line = CaptionLine("The time is 2:17 PM.     ", {});
                lines.emplace_back(line);
                auto captionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), lines);
                caption_manager->onParsed(captionFrame);
            }
            TEST_F(CaptionManagerTest, test_splitCaptionFrameNoWhitespaceBeforeWrapIndex) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                std::vector<CaptionLine> expectedLines;
                CaptionLine expectedLine1 = CaptionLine("Thiscapti", {TextStyle()});
                CaptionLine expectedLine2 = CaptionLine("onhasnosp", {TextStyle()});
                CaptionLine expectedLine3 = CaptionLine("aces", {TextStyle()});
                expectedLines.emplace_back(expectedLine1);
                expectedLines.emplace_back(expectedLine2);
                expectedLines.emplace_back(expectedLine3);
                auto expectedCaptionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), expectedLines);
                EXPECT_CALL(*mockTimingAdapter, queueForDisplay(expectedCaptionFrame, _)).Times(1);
                EXPECT_CALL(*m_presenter, getWrapIndex(_)).Times(3).WillOnce(Return(pair<bool, uint32_t>(true, 9)))
                    .WillOnce(Return(pair<bool, uint32_t>(true, 9))).WillOnce(Return(pair<bool, uint32_t>(false, 0)));
                vector<CaptionLine> lines;
                CaptionLine line = CaptionLine("Thiscaptionhasnospaces", {});
                lines.emplace_back(line);
                auto captionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), lines);
                caption_manager->onParsed(captionFrame);
            }
            TEST_F(CaptionManagerTest, test_splitCaptionFrameFalseWillNotSplitLine) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                vector<CaptionLine> expectedLines;
                CaptionLine expectedLine1 = CaptionLine("The time is 2:17 PM.", {TextStyle()});
                expectedLines.emplace_back(expectedLine1);
                auto expectedCaptionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), expectedLines);
                EXPECT_CALL(*mockTimingAdapter, queueForDisplay(expectedCaptionFrame, _)).Times(1);
                EXPECT_CALL(*m_presenter, getWrapIndex(_)).Times(1).WillOnce(Return(pair<bool, uint32_t>(false, 0)));
                vector<CaptionLine> lines;
                CaptionLine line = CaptionLine("The time is 2:17 PM.", {});
                lines.emplace_back(line);
                auto captionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), lines);
                caption_manager->onParsed(captionFrame);
            }
            TEST_F(CaptionManagerTest, test_splitCaptionFrameAtSpaceIndex) {
                auto mockTimingAdapter = m_timingFactory->getMockTimingAdapter();
                vector<CaptionLine> expectedLines;
                CaptionLine expectedLine1 = CaptionLine("The time is", {TextStyle()});
                CaptionLine expectedLine2 = CaptionLine("2:17 PM.", {TextStyle()});
                expectedLines.emplace_back(expectedLine1);
                expectedLines.emplace_back(expectedLine2);
                auto expectedCaptionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), expectedLines);
                EXPECT_CALL(*m_presenter, getWrapIndex(_)).Times(2).WillOnce(Return(pair<bool, uint32_t>(true, 12)))
                    .WillOnce(Return(pair<bool, uint32_t>(false, 0)));
                EXPECT_CALL(*mockTimingAdapter, queueForDisplay(expectedCaptionFrame, _)).Times(1);
                vector<CaptionLine> lines;
                CaptionLine line1 = CaptionLine("The time is 2:17 PM.", {});
                lines.emplace_back(line1);
                auto captionFrame = CaptionFrame(1,milliseconds(1),milliseconds(0), lines);
                caption_manager->onParsed(captionFrame);
            }
        }
    }
}