#include <gtest/gtest.h>
#include "TextStyle.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace std;
            TEST(TextStyleTest, test_putToOperatorOnEmptyTextStyle) {
                auto textStyle = TextStyle();
                std::stringstream out;
                out << textStyle;
                ASSERT_EQ(out.str(), "TextStyle(charIndex:0, activeStyle:Style(bold=0, underline=0, italic=0))");
            }
        }
    }
}