#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKDIRECTIVEHANDLER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKDIRECTIVEHANDLER_H_

#include <gmock/gmock.h>
#include "DirectiveHandlerInterface.h"
#include "DirectiveHandlerResultInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                class DirectiveHandlerMockAdapter : public DirectiveHandlerInterface {
                public:
                    void preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result) override;
                    virtual ~DirectiveHandlerMockAdapter() = default;
                    virtual void preHandleDirective(const shared_ptr<AVSDirective>& directive, const shared_ptr<DirectiveHandlerResultInterface>& result) = 0;
                };
                class MockDirectiveHandler : public DirectiveHandlerMockAdapter {
                public:
                    MOCK_METHOD1(handleDirectiveImmediately, void(shared_ptr<AVSDirective>));
                    MOCK_METHOD1(handleDirective, bool(const string&));
                    MOCK_METHOD1(cancelDirective, void(const string&));
                    MOCK_METHOD0(onDeregistered, void());
                    MOCK_CONST_METHOD0(getConfiguration, DirectiveHandlerConfiguration());
                    MOCK_METHOD2(preHandleDirective, void(const shared_ptr<AVSDirective>&, const shared_ptr<DirectiveHandlerResultInterface>&));
                };
                void DirectiveHandlerMockAdapter::preHandleDirective(shared_ptr<AVSDirective> avsDirective, unique_ptr<DirectiveHandlerResultInterface> handler) {
                    if (handler) preHandleDirective(avsDirective, move(handler));
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKDIRECTIVEHANDLER_H_
