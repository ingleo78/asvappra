#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONPARSER_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_MOCKCAPTIONPARSER_H_

#include <gtest/gtest.h>
#include "CaptionParserInterface.h"
#include "CaptionFrameParseListenerInterface.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            class MockCaptionParser : public CaptionParserInterface {
            public:
                //MOCK_METHOD2(parse, void(const CaptionFrame::MediaPlayerSourceId, const CaptionData& captionData));
                MOCK_METHOD1(addListener, void(shared_ptr<CaptionFrameParseListenerInterface> parseListener));
                MOCK_METHOD1(releaseResourcesFor, void(CaptionFrame::MediaPlayerSourceId));
            };
        }
    }
}
#endif