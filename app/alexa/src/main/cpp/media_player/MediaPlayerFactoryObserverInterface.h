#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERFACTORYOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_MEDIAPLAYERFACTORYOBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                class MediaPlayerFactoryObserverInterface {
                public:
                    virtual ~MediaPlayerFactoryObserverInterface() = default;
                    virtual void onReadyToProvideNextPlayer();
                };
            }
        }
    }
}
#endif