#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUTHDELEGATEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUTHDELEGATEINTERFACE_H_

#include <memory>
#include <string>
#include "Bluetooth/BluetoothDeviceConnectionRuleInterface.h"
#include "Bluetooth/BluetoothDeviceManagerInterface.h"
#include "AuthObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AuthDelegateInterface {
            public:
                virtual ~AuthDelegateInterface() = default;
                virtual void addAuthObserver(std::shared_ptr<avsCommon::sdkInterfaces::AuthObserverInterface> observer) = 0;
                virtual void removeAuthObserver(std::shared_ptr<avsCommon::sdkInterfaces::AuthObserverInterface> observer) = 0;
                virtual std::string getAuthToken() = 0;
                virtual void onAuthFailure(const std::string& token) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUTHDELEGATEINTERFACE_H_
