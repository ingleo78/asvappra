#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCONTEXTMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCONTEXTMANAGER_H_

#include <gmock/gmock.h>
#include "ContextManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace chrono;
                using namespace avs;
                class MockContextManager : public ContextManagerInterface {
                public:
                    MOCK_METHOD0(doShutdown, void());
                    //MOCK_METHOD2(setStateProvider, void(const CapabilityTag& namespaceAndName, shared_ptr<StateProviderInterface> stateProvider));
                    MOCK_METHOD4(setState, SetStateResult(const CapabilityTag& namespaceAndName, const string& jsonState, const StateRefreshPolicy& refreshPolicy,
                                 const unsigned int stateRequestToken));
                    MOCK_METHOD3(getContext, ContextRequestToken(shared_ptr<ContextRequesterInterface>, const string&, const milliseconds&));
                    MOCK_METHOD3(getContextWithoutReportableStateProperties, ContextRequestToken(shared_ptr<ContextRequesterInterface>, const string&,
                                 const milliseconds&));
                    MOCK_METHOD3(reportStateChange, void(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState,
                                 AlexaStateChangeCauseType cause));
                    MOCK_METHOD3(provideStateResponse, void(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState,
                                 const unsigned int stateRequestToken));
                    MOCK_METHOD3(provideStateUnavailableResponse, void(const CapabilityTag& capabilityIdentifier, const unsigned int stateRequestToken,
                                 bool isEndpointUnreachable));
                    MOCK_METHOD1(addContextManagerObserver, void(std::shared_ptr<ContextManagerObserverInterface> observer));
                    MOCK_METHOD1(removeContextManagerObserver, void(const std::shared_ptr<ContextManagerObserverInterface>& observer));
                    //MOCK_METHOD2(addStateProvider, void(const CapabilityTag& capabilityIdentifier, shared_ptr<StateProviderInterface> stateProvider));
                    MOCK_METHOD1(removeStateProvider, void(const avs::CapabilityTag& capabilityIdentifier));
                };
            }
        }
    }
}
#endif