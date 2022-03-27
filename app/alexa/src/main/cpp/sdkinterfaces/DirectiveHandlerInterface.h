#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIRECTIVEHANDLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIRECTIVEHANDLERINTERFACE_H_

#include <memory>
#include <avs/AVSDirective.h>
#include <avs/DirectiveHandlerConfiguration.h>
#include "DirectiveHandlerResultInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace avs;
            class DirectiveHandlerInterface {
            public:
                virtual ~DirectiveHandlerInterface() = default;
                virtual void handleDirectiveImmediately(shared_ptr<AVSDirective> directive);
                virtual void preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result);
                virtual bool handleDirective(const string& messageId);
                virtual void cancelDirective(const string& messageId);
                virtual void onDeregistered();
                virtual DirectiveHandlerConfiguration getConfiguration() const;
            };
        }
    }
}
#endif