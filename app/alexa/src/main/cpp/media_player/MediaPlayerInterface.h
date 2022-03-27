#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERINTERFACE_H_

#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <avs/attachment/AttachmentReader.h>
#include <util/AudioFormat.h>
#include <util/Optional.h>
#include <util/MediaType.h>
#include <util/Optional.h>
#include "MediaPlayerFactoryInterface.h"
#include "MediaPlayerState.h"
#include "PlaybackAttributes.h"
#include "PlaybackContext.h"
#include "PlaybackReport.h"
#include "SourceConfig.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                using namespace std;
                using namespace chrono;
                using namespace avs;
                using namespace attachment;
                using namespace utils;
                using namespace mediaPlayer;
                static const milliseconds MEDIA_PLAYER_INVALID_OFFSET{-1};
                class MediaPlayerObserverInterface;
                class MediaPlayerInterface {
                public:
                    typedef uint64_t SourceId;
                    static const SourceId ERROR = 0;
                    virtual ~MediaPlayerInterface() = default;
                    virtual SourceId setSource(shared_ptr<AttachmentReader> attachmentReader, const AudioFormat* format = nullptr,
                                               const SourceConfig& config = emptySourceConfig()) = 0;
                    virtual SourceId setSource(const string& url, milliseconds offset = milliseconds::zero(), const SourceConfig& config = emptySourceConfig(),
                                               bool repeat = false, const PlaybackContext& playbackContext = PlaybackContext());
                    virtual SourceId setSource(shared_ptr<istream> stream, bool repeat = false, const SourceConfig& config = emptySourceConfig(),
                                               MediaType format = MediaType::UNKNOWN);
                    virtual bool play(SourceId id);
                    virtual bool stop(SourceId id);
                    virtual bool stop(SourceId id, seconds timeToPipelineShutdown) {
                        return false;
                    }
                    virtual bool pause(SourceId id);
                    virtual bool resume(SourceId id);
                    virtual bool seekTo(SourceId id, milliseconds location, bool fromStart) {
                        return false;
                    }
                    virtual milliseconds getOffset(SourceId id);
                    virtual uint64_t getNumBytesBuffered();
                    virtual Optional<MediaPlayerState> getMediaPlayerState(SourceId id);
                    virtual void addObserver(shared_ptr<MediaPlayerObserverInterface> playerObserver);
                    virtual void removeObserver(shared_ptr<MediaPlayerObserverInterface> playerObserver);
                    virtual Optional<PlaybackAttributes> getPlaybackAttributes();
                    virtual vector<PlaybackReport> getPlaybackReports();
                    virtual Optional<Fingerprint> getFingerprint();
                };
                inline Optional<PlaybackAttributes> MediaPlayerInterface::getPlaybackAttributes() {
                    return Optional<PlaybackAttributes>();
                }
                inline vector<PlaybackReport> MediaPlayerInterface::getPlaybackReports() {
                    return {};
                }
                inline Optional<Fingerprint> MediaPlayerInterface::getFingerprint() {
                    return Optional<Fingerprint>();
                }
            }
        }
    }
}
#endif