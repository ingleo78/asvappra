#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_POOLEDMEDIAPLAYERFACTORY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_POOLEDMEDIAPLAYERFACTORY_H_

#include <vector>
#include <unordered_set>
#include <util/RequiresShutdown.h>
#include "MediaPlayerFactoryInterface.h"
#include "MediaPlayerInterface.h"
#include "MediaPlayerFactoryObserverInterface.h"

namespace alexaClientSDK {
    namespace mediaPlayer {
        class PooledMediaPlayerFactory : public avsCommon::utils::mediaPlayer::MediaPlayerFactoryInterface {
        public:
            static std::unique_ptr<PooledMediaPlayerFactory> create(
                const std::vector<std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface>>& pool,
                const avsCommon::utils::mediaPlayer::Fingerprint& fingerprint = {});
            virtual ~PooledMediaPlayerFactory();
            avsCommon::utils::mediaPlayer::Fingerprint getFingerprint() override;
            std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> acquireMediaPlayer() override;
            bool releaseMediaPlayer(std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> mediaPlayer) override;
            bool isMediaPlayerAvailable() override;
            void addObserver(std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerFactoryObserverInterface> observer);
            void removeObserver(std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerFactoryObserverInterface> observer);
        protected:
            PooledMediaPlayerFactory(
                const std::vector<std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface>>& pool,
                const avsCommon::utils::mediaPlayer::Fingerprint& fingerprint);
            void notifyObservers();
            std::vector<std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface>> m_availablePlayerPool;
            std::vector<std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface>> m_inUsePlayerPool;
            std::unordered_set<std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerFactoryObserverInterface>> m_observers;
            avsCommon::utils::mediaPlayer::Fingerprint m_fingerprint;
        };
    }
}
#endif