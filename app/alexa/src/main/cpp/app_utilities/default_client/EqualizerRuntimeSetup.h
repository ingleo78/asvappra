#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_EQUALIZERRUNTIMESETUP_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_EQUALIZERRUNTIMESETUP_H_

#include <list>
#include <memory>
#include <sdkinterfaces/Audio/EqualizerConfigurationInterface.h>
#include <sdkinterfaces/Audio/EqualizerModeControllerInterface.h>
#include <sdkinterfaces/Audio/EqualizerStorageInterface.h>
#include <sdkinterfaces/Audio/EqualizerInterface.h>
#include <sdkinterfaces/Audio/EqualizerControllerListenerInterface.h>

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace audio;
        class EqualizerRuntimeSetup {
        public:
            EqualizerRuntimeSetup();
            void setConfiguration(shared_ptr<EqualizerConfigurationInterface> configuration);
            shared_ptr<EqualizerConfigurationInterface> getConfiguration();
            void setStorage(shared_ptr<EqualizerStorageInterface> storage);
            shared_ptr<EqualizerStorageInterface> getStorage();
            void setModeController(shared_ptr<EqualizerModeControllerInterface> modeController);
            shared_ptr<EqualizerModeControllerInterface> getModeController();
            void addEqualizer(std::shared_ptr<avsCommon::sdkInterfaces::audio::EqualizerInterface> equalizer);
            void addEqualizerControllerListener(shared_ptr<EqualizerControllerListenerInterface> listener);
            list<shared_ptr<EqualizerInterface>> getAllEqualizers();
            list<shared_ptr<EqualizerControllerListenerInterface>>
            getAllEqualizerControllerListeners();
        private:
            shared_ptr<EqualizerConfigurationInterface> m_configuration;
            shared_ptr<EqualizerModeControllerInterface> m_modeController;
            shared_ptr<EqualizerStorageInterface> m_storage;
            list<shared_ptr<EqualizerInterface>> m_equalizers;
            list<shared_ptr<EqualizerControllerListenerInterface>> m_equalizerControllerListeners;
        };
    }
}
#endif