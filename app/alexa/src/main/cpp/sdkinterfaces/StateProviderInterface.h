#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_STATEPROVIDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_STATEPROVIDERINTERFACE_H_

#include <avs/CapabilityTag.h>
#include <avs/NamespaceAndName.h>
#include "ContextRequestToken.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace avs;
            using namespace utils;
            using namespace logger;
            class StateProviderInterface {
            public:
                virtual ~StateProviderInterface() = default;
                virtual void provideState(const NamespaceAndName& stateProviderName, const ContextRequestToken stateRequestToken);
                virtual void provideState(const CapabilityTag& stateProviderName, const ContextRequestToken stateRequestToken);
                virtual bool canStateBeRetrieved();
                virtual bool hasReportableStateProperties();
            };
            inline void StateProviderInterface::provideState(const NamespaceAndName& stateProviderName, const ContextRequestToken stateRequestToken) {
                acsdkError(LogEntry("ContextRequesterInterface", __func__).d("reason", "methodDeprecated"));
            }
            inline void StateProviderInterface::provideState(const CapabilityTag& stateProviderName, const ContextRequestToken stateRequestToken) {
                provideState(NamespaceAndName(stateProviderName), stateRequestToken);
            }
            inline bool StateProviderInterface::canStateBeRetrieved() {
                return true;
            }
            inline bool StateProviderInterface::hasReportableStateProperties() {
                return false;
            }
        }
    }
}
#endif