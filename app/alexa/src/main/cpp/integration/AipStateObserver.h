#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_AIPSTATEOBSERVER_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_AIPSTATEOBSERVER_H_

#include <chrono>
#include <deque>
#include <condition_variable>
#include <mutex>
#include <sdkinterfaces/AudioInputProcessorObserverInterface.h>

namespace alexaClientSDK {
    namespace integration {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        class AipStateObserver : public AudioInputProcessorObserverInterface {
        public:
            AipStateObserver();
            void onStateChanged(AudioInputProcessorObserverInterface::State newState) override;
            bool checkState(const AudioInputProcessorObserverInterface::State expectedState, const seconds duration = seconds(10));
            AudioInputProcessorObserverInterface::State waitForNext(const seconds duration);
        private:
            AudioInputProcessorObserverInterface::State m_state;
            mutex m_mutex;
            condition_variable m_wakeTrigger;
            deque<AudioInputProcessorObserverInterface::State> m_queue;
        };
    }
}
#endif