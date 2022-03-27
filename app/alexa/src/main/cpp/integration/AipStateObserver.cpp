#include "AipStateObserver.h"

namespace alexaClientSDK {
    namespace integration {
        using sdkInterfaces::AudioInputProcessorObserverInterface;
        AipStateObserver::AipStateObserver() : m_state(AudioInputProcessorObserverInterface::State::IDLE) {}
        void AipStateObserver::onStateChanged(AudioInputProcessorObserverInterface::State newState) {
            unique_lock<mutex> lock(m_mutex);
            m_queue.push_back(newState);
            m_state = newState;
            m_wakeTrigger.notify_all();
        }
        bool AipStateObserver::checkState(const State expectedState, const seconds duration) {
            State hold = waitForNext(duration);
            return hold == expectedState;
        }
        AudioInputProcessorObserverInterface::State AipStateObserver::waitForNext(const seconds duration) {
            State ret;
            unique_lock<mutex> lock(m_mutex);
            if (!m_wakeTrigger.wait_for(lock, duration, [this]() { return !m_queue.empty(); })) {
                ret = State::IDLE;
                return ret;
            }
            ret = m_queue.front();
            m_queue.pop_front();
            return ret;
        }
    }
}