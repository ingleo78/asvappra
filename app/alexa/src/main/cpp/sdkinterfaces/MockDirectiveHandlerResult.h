#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKDIRECTIVEHANDLERRESULT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKDIRECTIVEHANDLERRESULT_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "DirectiveHandlerResultInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockDirectiveHandlerResult : public DirectiveHandlerResultInterface {
                public:
                    MOCK_METHOD0(setCompleted, void());
                    MOCK_METHOD1(setFailed, void(const std::string& description));
                };
            }
        }
    }
}
#endif