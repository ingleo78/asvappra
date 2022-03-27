#ifndef ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_MISCDBEQUALIZERSTORAGE_H_
#define ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_MISCDBEQUALIZERSTORAGE_H_

#include <memory>
#include <mutex>
#include <sdkinterfaces/Audio/EqualizerStorageInterface.h>
#include <sdkinterfaces/Storage/MiscStorageInterface.h>
#include <util/error/SuccessResult.h>

namespace alexaClientSDK {
    namespace equalizer {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace audio;
        using namespace error;
        using namespace storage;
        class MiscDBEqualizerStorage : public EqualizerStorageInterface {
        public:
            static shared_ptr<MiscDBEqualizerStorage> create(shared_ptr<MiscStorageInterface> storage);
            void saveState(const EqualizerState& state) override;
            SuccessResult<EqualizerState> loadState() override;
            void clear() override;
        private:
            MiscDBEqualizerStorage(shared_ptr<MiscStorageInterface> storage);
            bool initialize();
            shared_ptr<MiscStorageInterface> m_miscStorage;
        };
    }
}
#endif