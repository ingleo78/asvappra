#include <ostream>
#include <avs/CapabilityResources.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "SoftwareComponentReporterCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace softwareComponentReporter {
            using namespace configuration;
            static const string TAG{"SoftwareComponentReporterCapabilityAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string SOFTWARECOMPONENTREPORTER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string SOFTWARECOMPONENTREPORTER_CAPABILITY_INTERFACE_NAME = "Alexa.SoftwareComponentReporter";
            static const string SOFTWARECOMPONENTREPORTER_CAPABILITY_INTERFACE_VERSION = "1.0";
            static const string SOFTWARECOMPONENTS_KEY = "softwareComponents";
            static const string SOFTWARECOMPONENTS_NAME_KEY = "name";
            static const string SOFTWARECOMPONENTS_VERSION_KEY = "version";
            shared_ptr<SoftwareComponentReporterCapabilityAgent> SoftwareComponentReporterCapabilityAgent::create() {
                return shared_ptr<SoftwareComponentReporterCapabilityAgent>(new SoftwareComponentReporterCapabilityAgent());
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> SoftwareComponentReporterCapabilityAgent::getCapabilityConfigurations() {
                unordered_set<shared_ptr<CapabilityConfiguration>> capabilityConfigurations;
                capabilityConfigurations.insert(buildCapabilityConfiguration());
                return capabilityConfigurations;
            }
            bool SoftwareComponentReporterCapabilityAgent::addConfiguration(const shared_ptr<ComponentConfiguration> configuration) {
                if (!configuration) {
                    ACSDK_ERROR(LX(__func__).m("configuration is null"));
                    return false;
                }
                bool success = m_configurations.insert(configuration).second;
                if (!success) {
                    ACSDK_ERROR(LX(__func__).d("name", configuration->name).d("version", configuration->version).m("component already exists"));
                }
                return success;
            }
            shared_ptr<CapabilityConfiguration> SoftwareComponentReporterCapabilityAgent::buildCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, SOFTWARECOMPONENTREPORTER_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, SOFTWARECOMPONENTREPORTER_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, SOFTWARECOMPONENTREPORTER_CAPABILITY_INTERFACE_VERSION});
                if (!m_configurations.empty()) {
                    JsonGenerator configurations;
                    vector<string> softwareComponentsJsons;
                    for (const auto& configuration : m_configurations) {
                        JsonGenerator softwareComponentsJsonGenerator;
                        softwareComponentsJsonGenerator.addMember(SOFTWARECOMPONENTS_NAME_KEY, configuration->name);
                        softwareComponentsJsonGenerator.addMember(SOFTWARECOMPONENTS_VERSION_KEY, configuration->version);
                        softwareComponentsJsons.push_back(softwareComponentsJsonGenerator.toString());
                    }
                    configurations.addMembersArray(SOFTWARECOMPONENTS_KEY, softwareComponentsJsons);
                    configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, configurations.toString()});
                }
                return make_shared<CapabilityConfiguration>(configMap);
            }
        }
    }
}