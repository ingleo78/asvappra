#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONTEXTMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONTEXTMANAGERINTERFACE_H_

#include <chrono>
#include <memory>
#include <string>
#include <avs/CapabilityTag.h>
#include <avs/CapabilityState.h>
#include <avs/StateRefreshPolicy.h>
#include "AlexaStateChangeCauseType.h"
#include "ContextManagerObserverInterface.h"
#include "ContextRequesterInterface.h"
#include "StateProviderInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            enum class SetStateResult {
                SUCCESS,
                STATE_PROVIDER_NOT_REGISTERED,
                STATE_TOKEN_OUTDATED
            };
            using namespace std;
            using namespace avs;
            using namespace chrono;
            class ContextManagerInterface {
            public:
                virtual ~ContextManagerInterface() = default;
                virtual void setStateProvider(const CapabilityTag& capabilityIdentifier, shared_ptr<StateProviderInterface> stateProvider);
                virtual void addStateProvider(const CapabilityTag& capabilityIdentifier, shared_ptr<StateProviderInterface> stateProvider);
                virtual void removeStateProvider(const CapabilityTag& capabilityIdentifier);
                virtual SetStateResult setState(const CapabilityTag& capabilityIdentifier, const std::string& jsonState, const StateRefreshPolicy& refreshPolicy,
                                                const ContextRequestToken stateRequestToken = 0);
                virtual void reportStateChange(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState, AlexaStateChangeCauseType cause);
                virtual void provideStateResponse(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState,
                                                  ContextRequestToken stateRequestToken);
                virtual void provideStateUnavailableResponse(const CapabilityTag& capabilityIdentifier, ContextRequestToken stateRequestToken,
                                                             bool isEndpointUnreachable);
                virtual ContextRequestToken getContext(shared_ptr<ContextRequesterInterface> contextRequester, const std::string& endpointId = "",
                                                       const milliseconds& timeout = seconds(2));
                virtual ContextRequestToken getContextWithoutReportableStateProperties(shared_ptr<ContextRequesterInterface> contextRequester,
                                                                                       const string& endpointId = "", const milliseconds& timeout = seconds(2));
                virtual void addContextManagerObserver(shared_ptr<ContextManagerObserverInterface> observer);
                virtual void removeContextManagerObserver(const shared_ptr<ContextManagerObserverInterface>& observer);
            };
        }
    }
}
#endif