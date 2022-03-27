#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKLOCALEASSETSMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKLOCALEASSETSMANAGER_H_

#include <gmock/gmock.h>
#include "LocaleAssetsManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockLocaleAssetsManager : public LocaleAssetsManagerInterface {
                public:
                    MOCK_METHOD2(changeAssets, bool(const Locales& locale, const WakeWords& wakeWords));
                    MOCK_METHOD0(cancelOngoingChange, void());
                    MOCK_CONST_METHOD0(getDefaultSupportedWakeWords, WakeWordsSets());
                    MOCK_CONST_METHOD0(getLanguageSpecificWakeWords, std::map<LanguageTag, WakeWordsSets>());
                    MOCK_CONST_METHOD0(getLocaleSpecificWakeWords, std::map<Locale, WakeWordsSets>());
                    MOCK_CONST_METHOD1(getSupportedWakeWords, WakeWordsSets(const Locale&));
                    MOCK_CONST_METHOD0(getSupportedLocales, std::set<Locale>());
                    MOCK_CONST_METHOD0(getSupportedLocaleCombinations, LocaleCombinations());
                    MOCK_CONST_METHOD0(getDefaultLocale, Locale());
                };
            }
        }
    }
}
#endif