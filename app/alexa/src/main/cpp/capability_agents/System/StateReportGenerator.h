#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_STATEREPORTGENERATOR_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_STATEREPORTGENERATOR_H_

#include <array>
#include <memory>
#include <string>
#include <util/PlatformDefinitions.h>
#include <util/Optional.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingStringConversion.h>
#include <settings/SettingsManagerBuilderBase.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace utils;
            using namespace logger;
            using namespace settings;
            class StateReportGenerator {
            public:
                template <class SettingsManagerT> using SettingConfigurations = typename SettingsManagerBuilderBase<SettingsManagerT>::SettingConfigurations;
                template <class SettingsManagerT> static Optional<StateReportGenerator> create(shared_ptr<SettingsManagerT> settingManager,
                                                                                const SettingConfigurations<SettingsManagerT>& configurations);
                vector<string> generateReport();
                StateReportGenerator() = default;
            protected:
                explicit StateReportGenerator(const vector<function<string()>>& reportFunctions);
            private:
                template <class SettingsManagerT, ssize_t index = SettingsManagerT::NUMBER_OF_SETTINGS - 1> struct StringFunctionWrapper {
                    void operator()(
                        const shared_ptr<SettingsManagerT>& manager,
                        const SettingConfigurations<SettingsManagerT>& configurations,
                        vector<function<string()>>& reportFunctions);
                };
                template <class SettingsManagerT> struct StringFunctionWrapper<SettingsManagerT, -1> {
                    void operator()(const shared_ptr<SettingsManagerT>& manager, const SettingConfigurations<SettingsManagerT>& configurations,
                                    vector<function<std::string()>>& reportFunctions);
                };
                static string generateSettingStateReport(const SettingEventMetadata& metadata, const string& value);
                vector<function<string()>> m_reportFunctions;
            };
            template <class SettingsManagerT> Optional<StateReportGenerator> StateReportGenerator::create(shared_ptr<SettingsManagerT> manager,
                                                                                           const SettingConfigurations<SettingsManagerT>& configurations) {
                if (!manager) {
                    acsdkError(LogEntry("StateReportGenerator", "createFailed").d("reason", "nullManager"));
                    return Optional<StateReportGenerator>();
                }
                vector<function<string()>> reportFunctions;
                StringFunctionWrapper<SettingsManagerT> wrapper;
                wrapper(manager, configurations, reportFunctions);
                return Optional<StateReportGenerator>(StateReportGenerator{reportFunctions});
            }
            template <class SettingsManagerT, ssize_t index>
            void StateReportGenerator::StringFunctionWrapper<SettingsManagerT, index>::operator()(const shared_ptr<SettingsManagerT>& settingsManager,
                                                                                                  const SettingConfigurations<SettingsManagerT>& configurations,
                                                                                                  vector<function<string()>>& reportFunctions) {
                if (get<index>(configurations).metadata.hasValue()) {
                    auto metadata = get<index>(configurations).metadata.value();
                    reportFunctions.push_back([settingsManager, metadata] {
                        return generateSettingStateReport(metadata, settingsManager->template getJsonValue<index>());
                    });
                }
                StringFunctionWrapper<SettingsManagerT, index - 1> wrapper;
                wrapper(settingsManager, configurations, reportFunctions);
            }
            template <class SettingsManagerT>
            void StateReportGenerator::StringFunctionWrapper<SettingsManagerT, -1>::operator()(const shared_ptr<SettingsManagerT>& settingsManager,
                                                                                               const SettingConfigurations<SettingsManagerT>& configurations,
                                                                                               vector<function<string()>>& reportFunctions) {}
        }
    }
}
#endif