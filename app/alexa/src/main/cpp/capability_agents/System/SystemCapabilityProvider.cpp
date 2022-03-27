#include <string>
#include <unordered_set>
#include <avs/CapabilityConfiguration.h>
#include <json/JSONGenerator.h>
#include <logger/Logger.h>
#include "SystemCapabilityProvider.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            static const string TAG{"SystemCapabilityProvider"};
            #define LX(event) LogEntry(TAG, event)
            static const string SYSTEM_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string SYSTEM_CAPABILITY_INTERFACE_NAME = "System";
            static const string SYSTEM_CAPABILITY_INTERFACE_VERSION = "2.0";
            static const string LOCALES_CONFIGURATION_KEY = "locales";
            static const string LOCALE_COMBINATION_CONFIGURATION_KEY = "localeCombinations";
            static shared_ptr<CapabilityConfiguration> getSystemCapabilityConfiguration(
                const shared_ptr<LocaleAssetsManagerInterface>& localeAssetsManager);
            shared_ptr<SystemCapabilityProvider> SystemCapabilityProvider::create(
                const shared_ptr<LocaleAssetsManagerInterface>& localeAssetsManager) {
                if (!localeAssetsManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullLocaleAssetsManager"));
                    return nullptr;
                }
                return shared_ptr<SystemCapabilityProvider>(new SystemCapabilityProvider(localeAssetsManager));
            }
            SystemCapabilityProvider::SystemCapabilityProvider(const shared_ptr<LocaleAssetsManagerInterface>& localeAssetsManager) {
                m_capabilityConfigurations.insert(getSystemCapabilityConfiguration(localeAssetsManager));
            }
            shared_ptr<CapabilityConfiguration> getSystemCapabilityConfiguration(const shared_ptr<LocaleAssetsManagerInterface>& localeAssetsManager) {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, SYSTEM_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, SYSTEM_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, SYSTEM_CAPABILITY_INTERFACE_VERSION});
                JsonGenerator generator;
                generator.addStringArray(LOCALES_CONFIGURATION_KEY, localeAssetsManager->getSupportedLocales());
                generator.addCollectionOfStringArray(LOCALE_COMBINATION_CONFIGURATION_KEY, localeAssetsManager->getSupportedLocaleCombinations());
                configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, generator.toString()});
                return make_shared<CapabilityConfiguration>(configMap);
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> SystemCapabilityProvider::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
        }
    }
}