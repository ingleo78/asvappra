#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYMANAGERINTERFACE_H_

#include <string>
#include <memory>
#include "AVSGatewayAssignerInterface.h"
#include "AVSGatewayObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AVSGatewayManagerInterface {
            public:
                virtual ~AVSGatewayManagerInterface() = default;
                virtual bool setAVSGatewayAssigner(std::shared_ptr<avsCommon::sdkInterfaces::AVSGatewayAssignerInterface> avsGatewayAssigner) = 0;
                virtual std::string getGatewayURL() const = 0;
                virtual bool setGatewayURL(const std::string& avsGatewayURL) = 0;
                virtual void addObserver(std::shared_ptr<avsCommon::sdkInterfaces::AVSGatewayObserverInterface> observer) = 0;
                virtual void removeObserver(std::shared_ptr<avsCommon::sdkInterfaces::AVSGatewayObserverInterface> observer) = 0;
            };
        }
    }
}
#endif