#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONFRAMEPARSELISTENERINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONFRAMEPARSELISTENERINTERFACE_H_

#include "CaptionFrame.h"

namespace alexaClientSDK {
    namespace captions {
        class CaptionFrameParseListenerInterface {
        public:
            virtual ~CaptionFrameParseListenerInterface() = default;
            virtual void onParsed(const CaptionFrame& captionFrame);
        };
    }
}
#endif