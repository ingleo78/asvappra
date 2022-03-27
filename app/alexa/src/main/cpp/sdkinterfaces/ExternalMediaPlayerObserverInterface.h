#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXTERNALMEDIAPLAYEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXTERNALMEDIAPLAYEROBSERVERINTERFACE_H_

#include <avs/PlayRequestor.h>
#include "ExternalMediaAdapterInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace externalMediaPlayer {
                using namespace std;
                using namespace avs;
                struct ObservableSessionProperties {
                    ObservableSessionProperties();
                    ObservableSessionProperties(bool loggedIn, const std::string& userName);
                    bool loggedIn;
                    string userName;
                };
                inline ObservableSessionProperties::ObservableSessionProperties() : loggedIn{false}, userName{""} {}
                inline ObservableSessionProperties::ObservableSessionProperties(bool loggedIn, const string& userName) : loggedIn{loggedIn}, userName{userName} {}
                inline bool operator==(const ObservableSessionProperties& observableSessionPropertiesA, const ObservableSessionProperties& observableSessionPropertiesB) {
                    return observableSessionPropertiesA.loggedIn == observableSessionPropertiesB.loggedIn &&
                           observableSessionPropertiesA.userName == observableSessionPropertiesB.userName;
                }
                struct ObservablePlaybackStateProperties {
                    ObservablePlaybackStateProperties();
                    ObservablePlaybackStateProperties(const string& state, const string& trackName, const PlayRequestor& playRequestor);
                    string state;
                    string trackName;
                    PlayRequestor playRequestor;
                };
                inline ObservablePlaybackStateProperties::ObservablePlaybackStateProperties() : state{"IDLE"}, trackName{""}, playRequestor{} {};
                inline ObservablePlaybackStateProperties::ObservablePlaybackStateProperties(const string& state, const string& trackName,
                                                                                            const PlayRequestor& playRequestor) : state{state},
                                                                                            trackName{trackName}, playRequestor(playRequestor){};
                inline bool operator==(const ObservablePlaybackStateProperties& observableA, const ObservablePlaybackStateProperties& observableB) {
                    return observableA.state == observableB.state && observableA.trackName == observableB.trackName &&
                           observableA.playRequestor == observableB.playRequestor;
                }
                class ExternalMediaPlayerObserverInterface {
                public:
                    virtual ~ExternalMediaPlayerObserverInterface() = default;
                    virtual void onLoginStateProvided(const string& playerId, const ObservableSessionProperties sessionStateProperties);
                    virtual void onPlaybackStateProvided(const string& playerId, const ObservablePlaybackStateProperties playbackStateProperties);
                };
            }
        }
    }
}
#endif