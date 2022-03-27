#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONMANAGER_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONMANAGER_H_

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <avs/FocusState.h>
#include <util/RequiresShutdown.h>
#include "CaptionFrame.h"
#include "CaptionFrameParseListenerInterface.h"
#include "CaptionLine.h"
#include "CaptionManagerInterface.h"
#include "CaptionParserInterface.h"
#include "CaptionTimingAdapter.h"
#include "TimingAdapterFactory.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace logger;
        using namespace mediaPlayer;
        using MediaPlayerSourceId = CaptionFrame::MediaPlayerSourceId;
        class CaptionManager : public CaptionManagerInterface, public CaptionFrameParseListenerInterface, public MediaPlayerObserverInterface,
                               public RequiresShutdown, public enable_shared_from_this<CaptionManager> {
        public:
            static shared_ptr<CaptionManager> create(shared_ptr<CaptionParserInterface> parser, shared_ptr<TimingAdapterFactory> timingAdapterFactory = nullptr);
            void doShutdown() override;
            void onCaption(MediaPlayerSourceId sourceId, const CaptionData& captionData) override;
            void setCaptionPresenter(const shared_ptr<CaptionPresenterInterface>& presenter) override;
            void setMediaPlayers(const vector<shared_ptr<MediaPlayerInterface>>& mediaPlayers) override;
            void onParsed(const CaptionFrame& captionFrame) override;
            void onPlaybackStarted(MediaPlayerSourceId id, const MediaPlayerState& state) override;
            void onPlaybackFinished(MediaPlayerSourceId id, const MediaPlayerState& state) override;
            void onPlaybackError(MediaPlayerSourceId id, const ErrorType& type, std::string error, const MediaPlayerState& state) override;
            void onPlaybackPaused(MediaPlayerSourceId id, const MediaPlayerState& state) override;
            void onPlaybackResumed(MediaPlayerSourceId id, const MediaPlayerState& state) override;
            void onPlaybackStopped(MediaPlayerSourceId id, const MediaPlayerState& state) override;
            void onFirstByteRead(MediaPlayerSourceId id,const MediaPlayerState& state) override{};
        private:
            CaptionManager(shared_ptr<CaptionParserInterface> parser, shared_ptr<TimingAdapterFactory> timingAdapterFactory);
            void logMediaStateNotHandled(const std::string& event, const std::string& reason, MediaPlayerSourceId id);
            unordered_map<MediaPlayerSourceId, shared_ptr<CaptionTimingAdapterInterface>> m_timingAdaptersBySourceIds;
            shared_ptr<CaptionPresenterInterface> m_presenter;
            shared_ptr<CaptionParserInterface> m_parser;
            shared_ptr<TimingAdapterFactory> m_timingFactory;
            vector<shared_ptr<MediaPlayerInterface>> m_mediaPlayers;
            mutex m_mutex;
        };
    }
}
#endif