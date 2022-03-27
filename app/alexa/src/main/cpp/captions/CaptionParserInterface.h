#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONPARSERINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONPARSERINTERFACE_H_

#include <memory>
#include "CaptionData.h"
#include "CaptionFrame.h"
#include "CaptionFrameParseListenerInterface.h"

namespace alexaClientSDK {
    namespace captions {
        class CaptionParserInterface {
        public:
            virtual ~CaptionParserInterface() = default;
            virtual void parse(CaptionFrame::MediaPlayerSourceId captionId, const CaptionData& captionData);
            virtual void releaseResourcesFor(CaptionFrame::MediaPlayerSourceId captionId) ;
            virtual void addListener(std::shared_ptr<CaptionFrameParseListenerInterface> parseListener);
        };
    }
}
#endif