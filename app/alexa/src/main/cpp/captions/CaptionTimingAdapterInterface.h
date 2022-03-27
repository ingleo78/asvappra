#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONTIMINGADAPTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONTIMINGADAPTERINTERFACE_H_

#include "DelayInterface.h"
#include "CaptionFrame.h"

namespace alexaClientSDK {
    namespace captions {
        class CaptionTimingAdapterInterface {
        public:
            virtual ~CaptionTimingAdapterInterface() = default;
            virtual void queueForDisplay(const CaptionFrame& captionFrame, bool autostart = true);
            virtual void reset();
            virtual void start();
            virtual void stop();
            virtual void pause();
        };
    }
}
#endif