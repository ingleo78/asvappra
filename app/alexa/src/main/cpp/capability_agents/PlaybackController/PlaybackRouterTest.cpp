#include <gtest/gtest.h>
#include <sdkinterfaces/MockPlaybackHandler.h>
#include "PlaybackRouter.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            namespace test {
                using namespace testing;
                using namespace sdkInterfaces::test;
                class PlaybackRouterTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    shared_ptr<PlaybackRouter> m_playbackRouter;
                    shared_ptr<StrictMock<MockPlaybackHandler>> m_defaultPlaybackHandler;
                    shared_ptr<StrictMock<MockPlaybackHandler>> m_secondPlaybackHandler;
                };
                void PlaybackRouterTest::SetUp() {
                    m_defaultPlaybackHandler = make_shared<StrictMock<MockPlaybackHandler>>();
                    m_playbackRouter = PlaybackRouter::create(m_defaultPlaybackHandler);
                    m_secondPlaybackHandler = make_shared<StrictMock<MockPlaybackHandler>>();
                }
                void PlaybackRouterTest::TearDown() {
                    m_playbackRouter->shutdown();
                }
                TEST_F(PlaybackRouterTest, test_defaultHandler) {
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PLAY));
                    m_playbackRouter->buttonPressed(PlaybackButton::PLAY);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PAUSE));
                    m_playbackRouter->buttonPressed(PlaybackButton::PAUSE);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::NEXT));
                    m_playbackRouter->buttonPressed(PlaybackButton::NEXT);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PREVIOUS));
                    m_playbackRouter->buttonPressed(PlaybackButton::PREVIOUS);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::SKIP_FORWARD));
                    m_playbackRouter->buttonPressed(PlaybackButton::SKIP_FORWARD);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::SKIP_BACKWARD));
                    m_playbackRouter->buttonPressed(PlaybackButton::SKIP_BACKWARD);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::SHUFFLE, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::SHUFFLE, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::LOOP, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::LOOP, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::REPEAT, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::REPEAT, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::THUMBS_UP, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::THUMBS_UP, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::THUMBS_DOWN, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::THUMBS_DOWN, true);
                }
                TEST_F(PlaybackRouterTest, test_secondHandler) {
                    m_playbackRouter->setHandler(m_defaultPlaybackHandler);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PLAY));
                    m_playbackRouter->buttonPressed(PlaybackButton::PLAY);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::SHUFFLE, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::SHUFFLE, true);
                    m_playbackRouter->setHandler(m_secondPlaybackHandler);
                    EXPECT_CALL(*m_secondPlaybackHandler, onButtonPressed(PlaybackButton::PLAY));
                    m_playbackRouter->buttonPressed(PlaybackButton::PLAY);
                    EXPECT_CALL(*m_secondPlaybackHandler, onButtonPressed(PlaybackButton::PAUSE));
                    m_playbackRouter->buttonPressed(PlaybackButton::PAUSE);
                    EXPECT_CALL(*m_secondPlaybackHandler, onButtonPressed(PlaybackButton::NEXT));
                    m_playbackRouter->buttonPressed(PlaybackButton::NEXT);
                    EXPECT_CALL(*m_secondPlaybackHandler, onButtonPressed(PlaybackButton::PREVIOUS));
                    m_playbackRouter->buttonPressed(PlaybackButton::PREVIOUS);
                    EXPECT_CALL(*m_secondPlaybackHandler, onButtonPressed(PlaybackButton::SKIP_FORWARD));
                    m_playbackRouter->buttonPressed(PlaybackButton::SKIP_FORWARD);
                    EXPECT_CALL(*m_secondPlaybackHandler, onButtonPressed(PlaybackButton::SKIP_BACKWARD));
                    m_playbackRouter->buttonPressed(PlaybackButton::SKIP_BACKWARD);
                    EXPECT_CALL(*m_secondPlaybackHandler, onTogglePressed(PlaybackToggle::SHUFFLE, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::SHUFFLE, true);
                    EXPECT_CALL(*m_secondPlaybackHandler, onTogglePressed(PlaybackToggle::LOOP, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::LOOP, true);
                    EXPECT_CALL(*m_secondPlaybackHandler, onTogglePressed(PlaybackToggle::REPEAT, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::REPEAT, true);
                    EXPECT_CALL(*m_secondPlaybackHandler, onTogglePressed(PlaybackToggle::THUMBS_UP, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::THUMBS_UP, true);
                    EXPECT_CALL(*m_secondPlaybackHandler, onTogglePressed(PlaybackToggle::THUMBS_DOWN, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::THUMBS_DOWN, true);
                }
                TEST_F(PlaybackRouterTest, test_switchToDefaultHandler) {
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PLAY));
                    m_playbackRouter->buttonPressed(PlaybackButton::PLAY);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::SHUFFLE, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::SHUFFLE, true);
                    m_playbackRouter->setHandler(m_secondPlaybackHandler);
                    EXPECT_CALL(*m_secondPlaybackHandler, onButtonPressed(PlaybackButton::PLAY));
                    m_playbackRouter->buttonPressed(PlaybackButton::PLAY);
                    EXPECT_CALL(*m_secondPlaybackHandler, onTogglePressed(PlaybackToggle::SHUFFLE, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::SHUFFLE, true);
                    m_playbackRouter->switchToDefaultHandler();
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PLAY));
                    m_playbackRouter->buttonPressed(PlaybackButton::PLAY);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PAUSE));
                    m_playbackRouter->buttonPressed(PlaybackButton::PAUSE);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::NEXT));
                    m_playbackRouter->buttonPressed(PlaybackButton::NEXT);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::PREVIOUS));
                    m_playbackRouter->buttonPressed(PlaybackButton::PREVIOUS);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::SKIP_FORWARD));
                    m_playbackRouter->buttonPressed(PlaybackButton::SKIP_FORWARD);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onButtonPressed(PlaybackButton::SKIP_BACKWARD));
                    m_playbackRouter->buttonPressed(PlaybackButton::SKIP_BACKWARD);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::SHUFFLE, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::SHUFFLE, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::LOOP, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::LOOP, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::REPEAT, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::REPEAT, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::THUMBS_UP, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::THUMBS_UP, true);
                    EXPECT_CALL(*m_defaultPlaybackHandler, onTogglePressed(PlaybackToggle::THUMBS_DOWN, true));
                    m_playbackRouter->togglePressed(PlaybackToggle::THUMBS_DOWN, true);
                }
            }
        }
    }
}