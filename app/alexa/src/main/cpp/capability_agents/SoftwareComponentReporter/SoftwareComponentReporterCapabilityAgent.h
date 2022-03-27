#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SOFTWARECOMPONENTREPORTER_INCLUDE_SOFTWARECOMPONENTREPORTER_SOFTWARECOMPONENTREPORTERCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SOFTWARECOMPONENTREPORTER_INCLUDE_SOFTWARECOMPONENTREPORTER_SOFTWARECOMPONENTREPORTERCAPABILITYAGENT_H_

#include <memory>
#include <string>
#include <unordered_set>
#include <avs/CapabilityAgent.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ComponentReporterInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace softwareComponentReporter {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace json;
            using namespace logger;
            class SoftwareComponentReporterCapabilityAgent : public ComponentReporterInterface, public CapabilityConfigurationInterface {
            public:
                ~SoftwareComponentReporterCapabilityAgent() = default;
                static shared_ptr<SoftwareComponentReporterCapabilityAgent> create();
                bool addConfiguration(const shared_ptr<ComponentConfiguration> configuration) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
            private:
                SoftwareComponentReporterCapabilityAgent() = default;
                shared_ptr<CapabilityConfiguration> buildCapabilityConfiguration();
                unordered_set<shared_ptr<ComponentConfiguration>> m_configurations;
            };
        }
    }
}
#endif