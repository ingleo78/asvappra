#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKMESSAGESENDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKMESSAGESENDER_H_

#include <gmock/gmock.h>
#include "MessageSenderInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockMessageSender : public MessageSenderInterface {
                public:
                    MOCK_METHOD1(sendMessage, void(std::shared_ptr<avs::MessageRequest> request));
                };
            }
        }
    }
}
#endif