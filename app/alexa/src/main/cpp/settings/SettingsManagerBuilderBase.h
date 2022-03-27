#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSMANAGERBUILDERBASE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSMANAGERBUILDERBASE_H_

#include <tuple>
#include <util/Optional.h>
#include "SettingEventMetadata.h"
#include "SettingsManager.h"

namespace alexaClientSDK {
    namespace settings {
        template <typename SettingsT> struct SettingConfiguration {
            std::shared_ptr<SettingsT> setting;
            avsCommon::utils::Optional<settings::SettingEventMetadata> metadata;
        };
        template <class ManagerT> class SettingsManagerBuilderBase {};
        template <typename... SettingsT> class SettingsManagerBuilderBase<settings::SettingsManager<SettingsT...>> {
        public:
            template <size_t index>
            using SettingType = typename std::tuple_element<index, std::tuple<SettingsT...>>::type;
            template <size_t index>
            using ValueType = typename SettingType<index>::ValueType;
            using SettingConfigurations = std::tuple<SettingConfiguration<SettingsT>...>;
            static constexpr size_t NUMBER_OF_SETTINGS{sizeof...(SettingsT)};
            virtual std::unique_ptr<settings::SettingsManager<SettingsT...>> build();
            const SettingConfigurations getConfigurations() const;
            virtual ~SettingsManagerBuilderBase() = default;
        protected:
            SettingConfigurations m_settingConfigs;
        };
        template <typename... SettingsT> const std::tuple<SettingConfiguration<SettingsT>...> SettingsManagerBuilderBase<SettingsManager<SettingsT...>>::getConfigurations() const {
            return m_settingConfigs;
        }
    }
}
#endif