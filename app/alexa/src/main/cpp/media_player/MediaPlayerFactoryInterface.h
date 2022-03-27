#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERFACTORYINTERFACE_H_

#include <memory>
#include <string>
#include "MediaPlayerFactoryObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                class MediaPlayerInterface;
                struct Fingerprint {
                    std::string package;
                    std::string buildType;
                    std::string versionNumber;
                };
                class MediaPlayerFactoryInterface {
                public:
                    virtual ~MediaPlayerFactoryInterface() = default;
                    virtual Fingerprint getFingerprint();
                    virtual std::shared_ptr<MediaPlayerInterface> acquireMediaPlayer();
                    virtual bool releaseMediaPlayer(std::shared_ptr<MediaPlayerInterface> mediaPlayer);
                    virtual bool isMediaPlayerAvailable();
                    virtual void addObserver(std::shared_ptr<MediaPlayerFactoryObserverInterface> observer);
                    virtual void removeObserver(std::shared_ptr<MediaPlayerFactoryObserverInterface> observer);
                };
            }
        }
    }
}
#endif