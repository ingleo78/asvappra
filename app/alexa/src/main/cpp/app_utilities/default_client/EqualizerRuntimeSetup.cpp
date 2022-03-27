#include <logger/Logger.h>
#include "EqualizerRuntimeSetup.h"
#include "EqualizerRuntimeSetup.h"

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace alexaClientSDK::avsCommon::utils::logger;
        using namespace alexaClientSDK::avsCommon::sdkInterfaces;
        using namespace avsCommon::sdkInterfaces::audio;
        static const std::string TAG("EqualizerRuntimeSetup");
        #define LX(event) LogEntry(TAG, event)
        void EqualizerRuntimeSetup::setConfiguration(shared_ptr<EqualizerConfigurationInterface> configuration) {
            m_configuration = configuration;
        }
        shared_ptr<EqualizerConfigurationInterface> EqualizerRuntimeSetup::getConfiguration() {
            return m_configuration;
        }
        shared_ptr<EqualizerStorageInterface> EqualizerRuntimeSetup::getStorage() {
            return m_storage;
        }
        void EqualizerRuntimeSetup::setStorage(shared_ptr<EqualizerStorageInterface> storage) {
            m_storage = storage;
        }
        void EqualizerRuntimeSetup::setModeController(shared_ptr<EqualizerModeControllerInterface> modeController) {
            m_modeController = modeController;
        }
        shared_ptr<EqualizerModeControllerInterface> EqualizerRuntimeSetup::getModeController() {
            return m_modeController;
        }
        void EqualizerRuntimeSetup::addEqualizer(shared_ptr<EqualizerInterface> equalizer) {
            m_equalizers.push_back(equalizer);
        }
        list<shared_ptr<EqualizerInterface>> EqualizerRuntimeSetup::getAllEqualizers() {
            return m_equalizers;
        }
        void EqualizerRuntimeSetup::addEqualizerControllerListener(
            shared_ptr<EqualizerControllerListenerInterface> listener) {
            m_equalizerControllerListeners.push_back(listener);
        }
        list<std::shared_ptr<EqualizerControllerListenerInterface>> EqualizerRuntimeSetup::
            getAllEqualizerControllerListeners() {
            return m_equalizerControllerListeners;
        }
        EqualizerRuntimeSetup::EqualizerRuntimeSetup() {}
    }
}