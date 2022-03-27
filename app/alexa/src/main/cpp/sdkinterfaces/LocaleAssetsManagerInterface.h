#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_LOCALEASSETSMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_LOCALEASSETSMANAGERINTERFACE_H_

#include <map>
#include <set>
#include <string>
#include <vector>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class LocaleAssetsManagerInterface {
            public:
                using Locale = std::string;
                using LanguageTag = std::string;
                using WakeWords = std::set<std::string>;
                using WakeWordsSets = std::set<WakeWords>;
                using Locales = std::vector<Locale>;
                using LocaleCombinations = std::set<Locales>;
                virtual bool changeAssets(const Locales& locales, const WakeWords& wakeWords);
                virtual void cancelOngoingChange();
                virtual std::set<Locale> getSupportedLocales() const;
                virtual LocaleCombinations getSupportedLocaleCombinations() const;
                virtual Locale getDefaultLocale() const;
                virtual WakeWordsSets getDefaultSupportedWakeWords() const;
                virtual std::map<LanguageTag, WakeWordsSets> getLanguageSpecificWakeWords() const;
                virtual std::map<Locale, WakeWordsSets> getLocaleSpecificWakeWords() const;
                virtual WakeWordsSets getSupportedWakeWords(const Locale& locale) const;
                virtual ~LocaleAssetsManagerInterface() = default;
            };
        }
    }
}
#endif