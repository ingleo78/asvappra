#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYEROBSERVERINTERFACE_H_

#include <string>
#include <vector>
#include <memory>
#include "ErrorTypes.h"
#include "MediaPlayerInterface.h"
#include "MediaPlayerState.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                class MediaPlayerObserverInterface {
                public:
                    using SourceId = MediaPlayerInterface::SourceId;
                    enum class TagType { STRING, UINT, INT, DOUBLE, BOOLEAN };
                    struct TagKeyValueType {
                        std::string key;
                        std::string value;
                        MediaPlayerObserverInterface::TagType type;
                    };
                    typedef std::vector<struct TagKeyValueType> VectorOfTags;
                    virtual ~MediaPlayerObserverInterface() = default;
                    virtual void onFirstByteRead(SourceId id, const MediaPlayerState& state);
                    virtual void onPlaybackStarted(SourceId id, const MediaPlayerState& state);
                    virtual void onPlaybackFinished(SourceId id, const MediaPlayerState& state);
                    virtual void onPlaybackError(SourceId id, const ErrorType& type, std::string error, const MediaPlayerState& state);
                    virtual void onPlaybackPaused(SourceId, const MediaPlayerState&){};
                    virtual void onPlaybackResumed(SourceId, const MediaPlayerState&){};
                    virtual void onPlaybackStopped(SourceId, const MediaPlayerState&){};
                    virtual void onBufferUnderrun(SourceId, const MediaPlayerState&) {}
                    virtual void onBufferRefilled(SourceId, const MediaPlayerState&) {}
                    virtual void onBufferingComplete(SourceId, const MediaPlayerState&) {}
                    virtual void onSeeked(SourceId, const MediaPlayerState&, const MediaPlayerState&) {}
                    virtual void onTags(SourceId, std::unique_ptr<const VectorOfTags>, const MediaPlayerState&){};
                };
                inline std::ostream& operator<<(std::ostream& stream, const MediaPlayerState& state) {
                    return stream << "MediaPlayerState: offsetInMilliseconds=" << state.offset.count();
                }
            }
        }
    }
}
#endif