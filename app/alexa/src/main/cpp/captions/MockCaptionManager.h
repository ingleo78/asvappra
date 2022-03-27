#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONMANAGER_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONMANAGER_H_

#include <vector>
#include <gmock/gmock.h>
#include "CaptionManager.h"
#include "CaptionManagerInterface.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace avsCommon;
            using namespace utils;
            using namespace utils::mediaPlayer;
            class MockCaptionManager : public CaptionFrameParseListenerInterface, public MediaPlayerObserverInterface,
                                       public CaptionManagerInterface {
            public:
                MOCK_METHOD1(onParsed, void(const CaptionFrame&));
                MOCK_METHOD1(setCaptionPresenter, void(const shared_ptr<CaptionPresenterInterface>&));
                //MOCK_METHOD2(onCaption, void(uint64_t sourceId, const captions::CaptionData&));
                MOCK_METHOD1(setMediaPlayers, void(const vector<shared_ptr<MediaPlayerInterface>>&));
                //MOCK_METHOD2(onPlaybackStarted, void(SourceId, const avsCommon::utils::mediaPlayer::MediaPlayerState&));
                //MOCK_METHOD2(onPlaybackFinished, void(SourceId, const avsCommon::utils::mediaPlayer::MediaPlayerState&));
                MOCK_METHOD4(onPlaybackError, void(SourceId, const ErrorType&, string, const MediaPlayerState&));
                //MOCK_METHOD2(onFirstByteRead, void(SourceId, const avsCommon::utils::mediaPlayer::MediaPlayerState&));
            };
        }
    }
}
#endif