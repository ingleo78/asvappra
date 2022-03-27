#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYRESOURCES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYRESOURCES_H_

#include <vector>
#include <string>
#include <tuple>
#include <sdkinterfaces/LocaleAssetsManagerInterface.h>
#include <util/Optional.h>
#include "AlexaAssetId.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace resources;
            using Locale = LocaleAssetsManagerInterface::Locale;
            class CapabilityResources {
            public:
                CapabilityResources();
                bool addFriendlyNameWithAssetId(const AlexaAssetId& assetId);
                bool addFriendlyNameWithText(const string& text, const Locale& locale);
                bool isValid() const;
                string toJson() const;
            private:
                struct FriendlyName {
                    string value;
                    Optional<Locale> locale;
                    bool operator==(const FriendlyName& rhs) const;
                };
                bool m_isValid;
                std::vector<FriendlyName> m_items;
            };
        }
    }
}
#endif