#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CALLSTATEOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CALLSTATEOBSERVERINTERFACE_H_

#include <iostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class CallStateObserverInterface {
            public:
                enum class CallState {
                    CONNECTING,
                    INBOUND_RINGING,
                    CALL_CONNECTED,
                    CALL_DISCONNECTED,
                    NONE
                };
                virtual ~CallStateObserverInterface() = default;
                virtual void onCallStateChange(CallState state) = 0;
                static bool isStateActive(const CallStateObserverInterface::CallState& state);
            };
            inline std::ostream& operator<<(std::ostream& stream, const CallStateObserverInterface::CallState& state) {
                switch (state) {
                    case CallStateObserverInterface::CallState::CONNECTING:
                        stream << "CONNECTING";
                        return stream;
                    case CallStateObserverInterface::CallState::INBOUND_RINGING:
                        stream << "INBOUND_RINGING";
                        return stream;
                    case CallStateObserverInterface::CallState::CALL_CONNECTED:
                        stream << "CALL_CONNECTED";
                        return stream;
                    case CallStateObserverInterface::CallState::CALL_DISCONNECTED:
                        stream << "CALL_DISCONNECTED";
                        return stream;
                    case CallStateObserverInterface::CallState::NONE:
                        stream << "NONE";
                        return stream;
                }
                return stream << "UNKNOWN STATE";
            }
            inline bool CallStateObserverInterface::isStateActive(const CallStateObserverInterface::CallState& state) {
                switch (state) {
                    case CallStateObserverInterface::CallState::CONNECTING:
                    case CallStateObserverInterface::CallState::INBOUND_RINGING:
                    case CallStateObserverInterface::CallState::CALL_CONNECTED:
                        return true;
                    case CallStateObserverInterface::CallState::CALL_DISCONNECTED:
                    case CallStateObserverInterface::CallState::NONE:
                        return false;
                }
                return false;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CALLSTATEOBSERVERINTERFACE_H_
