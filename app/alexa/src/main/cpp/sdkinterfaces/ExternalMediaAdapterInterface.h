#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXTERNALMEDIAADAPTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXTERNALMEDIAADAPTERINTERFACE_H_

#include <chrono>
#include <set>
#include <string>
#include <stdint.h>
#include <avs/PlayRequestor.h>
#include <util/RequiresShutdown.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace externalMediaPlayer {
                using namespace std;
                using namespace avs;
                using namespace utils;
                using namespace chrono;
                enum class RequestType {
                    INIT,
                    DEINIT,
                    LOGIN,
                    LOGOUT,
                    PLAY,
                    RESUME,
                    PAUSE,
                    STOP,
                    PAUSE_RESUME_TOGGLE,
                    NEXT,
                    PREVIOUS,
                    START_OVER,
                    FAST_FORWARD,
                    REWIND,
                    ENABLE_REPEAT_ONE,
                    DISABLE_REPEAT_ONE,
                    ENABLE_REPEAT,
                    DISABLE_REPEAT,
                    ENABLE_SHUFFLE,
                    DISABLE_SHUFFLE,
                    FAVORITE,
                    DESELECT_FAVORITE,
                    UNFAVORITE,
                    DESELECT_UNFAVORITE,
                    SEEK,
                    ADJUST_SEEK,
                    SET_VOLUME,
                    ADJUST_VOLUME,
                    SET_MUTE,
                    SET_DISPLAY_NAME,
                    GET_INFO,
                    ADD_USER,
                    RESET_USER,
                    NONE
                };
                enum class SupportedPlaybackOperation {
                    PLAY,
                    RESUME,
                    PAUSE,
                    STOP,
                    NEXT,
                    PREVIOUS,
                    START_OVER,
                    FAST_FORWARD,
                    REWIND,
                    ENABLE_REPEAT,
                    ENABLE_REPEAT_ONE,
                    DISABLE_REPEAT,
                    ENABLE_SHUFFLE,
                    DISABLE_SHUFFLE,
                    FAVORITE,
                    UNFAVORITE,
                    SEEK,
                    ADJUST_SEEK,
                };
                enum class ChangeCauseType {
                    VOICE_INTERACTION,
                    PHYSICAL_INTERACTION,
                    APP_INTERACTION,
                    RULE_TRIGGER,
                    PERIODIC_POLL
                };
                enum class Favorites {
                    FAVORITED,
                    UNFAVORITED,
                    NOT_RATED
                };
                enum class MediaType {
                    TRACK,
                    PODCAST,
                    STATION,
                    AD,
                    SAMPLE,
                    OTHER
                };
                struct AdapterSessionState {
                    AdapterSessionState();
                    string playerId;
                    string localPlayerId;
                    string endpointId;
                    bool loggedIn;
                    string userName;
                    bool isGuest;
                    bool launched;
                    bool active;
                    string spiVersion;
                    string playerCookie;
                    string skillToken;
                    string playbackSessionId;
                    string accessToken;
                    chrono::milliseconds tokenRefreshInterval;
                };
                struct AdapterPlaybackState {
                    AdapterPlaybackState();
                    string playerId;
                    string state;
                    set<SupportedPlaybackOperation> supportedOperations;
                    milliseconds trackOffset;
                    bool shuffleEnabled;
                    bool repeatEnabled;
                    Favorites favorites;
                    string type;
                    string playbackSource;
                    string playbackSourceId;
                    string trackName;
                    string trackId;
                    string trackNumber;
                    string artistName;
                    string artistId;
                    string albumName;
                    string albumId;
                    string tinyURL;
                    string smallURL;
                    string mediumURL;
                    string largeURL;
                    string coverId;
                    string mediaProvider;
                    MediaType mediaType;
                    milliseconds duration;
                    avsCommon::avs::PlayRequestor playRequestor;
                };
                class AdapterState {
                public:
                    AdapterState() = default;
                    AdapterSessionState sessionState;
                    AdapterPlaybackState playbackState;
                };
                class ExternalMediaAdapterInterface : public virtual RequiresShutdown {
                public:
                    explicit ExternalMediaAdapterInterface(const string& adapterName);
                    virtual ~ExternalMediaAdapterInterface() = default;
                    virtual void init();
                    virtual void deInit();
                    virtual void handleLogin(const string& accessToken, const string& userName, bool forceLogin, milliseconds tokenRefreshInterval);
                    virtual void handleLogout();
                    virtual void handlePlay(string& playContextToken, int64_t index, milliseconds offset, const string& skillToken,
                                            const string& playbackSessionId, const string& navigation, bool preload, const PlayRequestor& playRequestor);
                    virtual void handlePlayControl(RequestType requestType);
                    virtual void handleSeek(milliseconds offset);
                    virtual void handleAdjustSeek(milliseconds deltaOffset);
                    virtual void handleAuthorized(bool authorized, const string& playerId, const string& defaultSkillToken);
                    virtual AdapterState getState();
                    virtual milliseconds getOffset();
                };
                inline AdapterSessionState::AdapterSessionState() : loggedIn{false}, isGuest{false}, launched{false}, active{false} {}
                inline AdapterPlaybackState::AdapterPlaybackState() : state{"IDLE"}, trackOffset{0}, shuffleEnabled{false}, repeatEnabled{false},
                                                                      favorites{Favorites::NOT_RATED}, mediaType{MediaType::TRACK}, duration{0} {}
                inline ExternalMediaAdapterInterface::ExternalMediaAdapterInterface(const std::string& adapterName) : RequiresShutdown{adapterName} {}
                inline string SupportedPlaybackOperationToString(SupportedPlaybackOperation operation) {
                    switch(operation) {
                        case SupportedPlaybackOperation::PLAY: return "Play";
                        case SupportedPlaybackOperation::RESUME: return "Resume";
                        case SupportedPlaybackOperation::PAUSE: return "Pause";
                        case SupportedPlaybackOperation::STOP: return "Stop";
                        case SupportedPlaybackOperation::NEXT: return "Next";
                        case SupportedPlaybackOperation::PREVIOUS: return "Previous";
                        case SupportedPlaybackOperation::START_OVER: return "StartOver";
                        case SupportedPlaybackOperation::FAST_FORWARD: return "FastForward";
                        case SupportedPlaybackOperation::REWIND: return "Rewind";
                        case SupportedPlaybackOperation::ENABLE_REPEAT: return "EnableRepeat";
                        case SupportedPlaybackOperation::ENABLE_REPEAT_ONE: return "EnableRepeatOne";
                        case SupportedPlaybackOperation::DISABLE_REPEAT: return "DisableRepeat";
                        case SupportedPlaybackOperation::ENABLE_SHUFFLE: return "EnableShuffle";
                        case SupportedPlaybackOperation::DISABLE_SHUFFLE: return "DisableShuffle";
                        case SupportedPlaybackOperation::FAVORITE: return "Favorite";
                        case SupportedPlaybackOperation::UNFAVORITE: return "Unfavorite";
                        case SupportedPlaybackOperation::SEEK: return "SetSeekPosition";
                        case SupportedPlaybackOperation::ADJUST_SEEK: return "AdjustSeekPosition";
                    }
                    return "unknown operation";
                }
                inline string ChangeTriggerToString(ChangeCauseType changeType) {
                    switch(changeType) {
                        case ChangeCauseType::VOICE_INTERACTION: return "VOICE_INTERACTION";
                        case ChangeCauseType::PHYSICAL_INTERACTION: return "PHYSICAL_INTERACTION";
                        case ChangeCauseType::APP_INTERACTION: return "APP_INTERACTION";
                        case ChangeCauseType::RULE_TRIGGER: return "RULE_TRIGGER";
                        case ChangeCauseType::PERIODIC_POLL: return "PERIODIC_POLL";
                    }
                    return "unknown changeTrigger";
                }
                inline string RatingToString(Favorites rating) {
                    switch(rating) {
                        case Favorites::FAVORITED: return "FAVORITED";
                        case Favorites::UNFAVORITED: return "UNFAVORITED";
                        case Favorites::NOT_RATED: return "NOT_RATED";
                    }
                    return "unknown rating";
                }
                inline string MediaTypeToString(MediaType mediaType) {
                    switch(mediaType) {
                        case MediaType::TRACK: return "TRACK";
                        case MediaType::PODCAST: return "PODCAST";
                        case MediaType::STATION: return "STATION";
                        case MediaType::AD: return "AD";
                        case MediaType::SAMPLE: return "SAMPLE";
                        case MediaType::OTHER: return "OTHER";
                    }
                    return "unknown mediaType";
                }
                inline string SHUFFLE_STATUS_STRING(bool shuffleEnabled) {
                    return (shuffleEnabled == true) ? "SHUFFLED" : "NOT_SHUFFLED";
                }
                inline string REPEAT_STATUS_STRING(bool repeatEnabled) {
                    return (repeatEnabled == true) ? "REPEATED" : "NOT_REPEATED";
                }
            }
        }
    }
}
#endif