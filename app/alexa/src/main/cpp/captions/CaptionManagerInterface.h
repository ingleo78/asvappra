#ifndef ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONMANAGERINTERFACE_H_

#include <memory>
#include "CaptionData.h"
#include "CaptionFrame.h"
#include "CaptionPresenterInterface.h"

namespace alexaClientSDK {
    namespace captions {
        class CaptionManagerInterface {
        public:
            using MediaPlayerSourceId = CaptionFrame::MediaPlayerSourceId;
            virtual ~CaptionManagerInterface() = default;
            virtual void onCaption(MediaPlayerSourceId sourceId, const CaptionData& captionData);
            virtual void setCaptionPresenter(const shared_ptr<CaptionPresenterInterface>& presenter);
            virtual void setMediaPlayers(const vector<shared_ptr<MediaPlayerInterface>>& mediaPlayers);
        };
    }
}
#endif