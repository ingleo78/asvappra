#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIRECTIVESEQUENCERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIRECTIVESEQUENCERINTERFACE_H_

#include <memory>
#include <string>
#include <avs/AVSDirective.h>
#include <util/RequiresShutdown.h>
#include "DirectiveHandlerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class DirectiveSequencerInterface : public utils::RequiresShutdown {
            public:
                DirectiveSequencerInterface(const std::string& name);
                virtual ~DirectiveSequencerInterface() = default;
                virtual bool addDirectiveHandler(std::shared_ptr<DirectiveHandlerInterface> handler);
                virtual bool removeDirectiveHandler(std::shared_ptr<DirectiveHandlerInterface> handler);
                virtual void setDialogRequestId(const std::string& dialogRequestId);
                virtual std::string getDialogRequestId();
                virtual bool onDirective(std::shared_ptr<avsCommon::avs::AVSDirective> directive);
                virtual void disable();
                virtual void enable();
            };
            inline DirectiveSequencerInterface::DirectiveSequencerInterface(const std::string& name) : utils::RequiresShutdown{name} {}
        }
    }
}
#endif