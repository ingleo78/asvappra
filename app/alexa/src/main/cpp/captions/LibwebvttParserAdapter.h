#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_LIBWEBVTTPARSERADAPTER_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_LIBWEBVTTPARSERADAPTER_H_

#include <chrono>
#include <map>
#include <memory>
#include "CaptionParserInterface.h"
#include "CaptionData.h"
#include "CaptionFrame.h"
#include "CaptionLine.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        using MediaPlayerSourceId = CaptionFrame::MediaPlayerSourceId ;
        class LibwebvttParserAdapter : public CaptionParserInterface {
        public:
            static shared_ptr<LibwebvttParserAdapter> getInstance();
            void parse(MediaPlayerSourceId captionId, const CaptionData& captionData) override;
            void addListener(shared_ptr<CaptionFrameParseListenerInterface> parseListener) override;
            void releaseResourcesFor(MediaPlayerSourceId captionId) override;
        private:
            LibwebvttParserAdapter(){};
            LibwebvttParserAdapter(LibwebvttParserAdapter const&) = delete;
        };
    }
}
#endif