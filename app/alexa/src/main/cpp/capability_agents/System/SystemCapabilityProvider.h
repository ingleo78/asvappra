#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_SYSTEMCAPABILITYPROVIDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_SYSTEMCAPABILITYPROVIDER_H_

#include <memory>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/LocaleAssetsManagerInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace json;
            class SystemCapabilityProvider : public CapabilityConfigurationInterface {
            public:
                static shared_ptr<SystemCapabilityProvider> create(const shared_ptr<LocaleAssetsManagerInterface>& localeAssetsManager);
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
            private:
                SystemCapabilityProvider(const shared_ptr<LocaleAssetsManagerInterface>& localeAssetsManager);
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
            };
        }
    }
}
#endif