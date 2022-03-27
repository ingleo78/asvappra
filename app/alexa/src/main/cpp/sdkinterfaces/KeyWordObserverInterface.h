#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_KEYWORDOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_KEYWORDOBSERVERINTERFACE_H_

#include <limits>
#include <memory>
#include <vector>
#include <stdint.h>
#include <avs/AudioInputStream.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace avs;
            using namespace std;
            class KeyWordObserverInterface {
            public:
                static constexpr uint64_t UNSPECIFIED_INDEX = std::numeric_limits<uint64_t>::max();
                virtual ~KeyWordObserverInterface() = default;
                virtual void onKeyWordDetected(
                    shared_ptr<AudioInputStream> stream,
                    string keyword,
                    uint64_t beginIndex = UNSPECIFIED_INDEX,
                    uint64_t endIndex = UNSPECIFIED_INDEX,
                    shared_ptr<const vector<char>> KWDMetadata = nullptr) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_KEYWORDOBSERVERINTERFACE_H_
