#ifndef ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONPRESENTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONPRESENTERINTERFACE_H_

#include <utility>
#include <avs/FocusState.h>
#include "CaptionFrame.h"
#include "CaptionLine.h"
#include "TextStyle.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        class CaptionPresenterInterface {
        public:
            virtual ~CaptionPresenterInterface() = default;
            virtual void onCaptionActivity(const CaptionFrame& captionFrame, FocusState activityType);
            virtual pair<bool, int> getWrapIndex(const CaptionLine& captionLine);
        };
    }
}
#endif