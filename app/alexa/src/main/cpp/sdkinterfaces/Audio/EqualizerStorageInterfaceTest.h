#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERSTORAGEINTERFACETEST_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERSTORAGEINTERFACETEST_H_

#include <gmock/gmock.h>
#include <memory>
#include "EqualizerStorageInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    using EqualizerStorageFactory = std::function<std::shared_ptr<EqualizerStorageInterface>()>;
                    class EqualizerStorageInterfaceTest : public ::testing::TestWithParam<EqualizerStorageFactory> {
                    public:
                        void SetUp() override;
                    protected:
                        std::shared_ptr<avsCommon::sdkInterfaces::audio::EqualizerStorageInterface> m_storage = nullptr;
                    };
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERSTORAGEINTERFACETEST_H_
