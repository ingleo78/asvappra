#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CALLMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CALLMANAGERINTERFACE_H_

#include <memory>
#include <string>
#include "AVSGatewayObserverInterface.h"
#include "CallStateObserverInterface.h"
#include "ConnectionStatusObserverInterface.h"
#include "SoftwareInfoSenderObserverInterface.h"
#include "ExceptionEncounteredSenderInterface.h"
#include "../avs/CapabilityAgent.h"
#include "../util/RequiresShutdown.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace avs;
            using namespace utils;
            class CallManagerInterface : public RequiresShutdown, public CapabilityAgent, public ConnectionStatusObserverInterface,
                                         public SoftwareInfoSenderObserverInterface, public AVSGatewayObserverInterface {
            public:
                CallManagerInterface(const string& objectName, const string& avsNamespace, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                enum class DTMFTone {
                    DTMF_ZERO,
                    DTMF_ONE,
                    DTMF_TWO,
                    DTMF_THREE,
                    DTMF_FOUR,
                    DTMF_FIVE,
                    DTMF_SIX,
                    DTMF_SEVEN,
                    DTMF_EIGHT,
                    DTMF_NINE,
                    DTMF_STAR,
                    DTMF_POUND
                };
                virtual ~CallManagerInterface() = default;
                virtual void addObserver(std::shared_ptr<avsCommon::sdkInterfaces::CallStateObserverInterface> observer) = 0;
                virtual void removeObserver(std::shared_ptr<avsCommon::sdkInterfaces::CallStateObserverInterface> observer) = 0;
                virtual void acceptCall() = 0;
                virtual void sendDtmf(DTMFTone dtmfTone) = 0;
                virtual void stopCall() = 0;
                virtual void muteSelf() = 0;
                virtual void unmuteSelf() = 0;
                virtual bool isSelfMuted() const = 0;
            };
            inline CallManagerInterface::CallManagerInterface(const string& objectName, const string& avsNamespace,
                                                              shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                                              RequiresShutdown{objectName}, CapabilityAgent{avsNamespace, exceptionEncounteredSender} {}
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CALLMANAGERINTERFACE_H_
