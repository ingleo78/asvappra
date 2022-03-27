#include <gtest/gtest.h>

#include "CaptionFrame.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace std;
            TEST(CaptionFrameTest, test_putToOperatorOnEmptyCaptionFrame) {
                auto captionFrame = CaptionFrame();
                stringstream out;
                out << captionFrame;
                ASSERT_EQ(out.str(), "CaptionFrame(id:0, duration:0, delay:0, lines:[])");
            }
        }
    }
}