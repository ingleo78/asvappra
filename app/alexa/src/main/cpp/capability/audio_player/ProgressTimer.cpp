#include <algorithm>
#include <logger/Logger.h>
#include <timing/Timer.h>
#include "ProgressTimer.h"
#include "ErrorType.h"

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        using namespace avsCommon;
        using namespace utils;
        using namespace logger;
        using namespace timing;
        static const string TAG("ProgressTimer");
        #define LX(event) LogEntry(TAG, event)
        ProgressTimer::ProgressTimer() : m_state{State::IDLE}, m_delay{ProgressTimer::getNoDelay()}, m_interval{ProgressTimer::getNoInterval()},
                                         m_target{milliseconds::zero()}, m_gotProgress{false}, m_progress{milliseconds::zero()} {}
        std::ostream& operator<<(ostream& stream, ProgressTimer::State state) {
            switch(state) {
                case ProgressTimer::State::IDLE: return stream << "IDLE";
                case ProgressTimer::State::INITIALIZED: return stream << "INITIALIZED";
                case ProgressTimer::State::RUNNING: return stream << "RUNNING";
                case ProgressTimer::State::PAUSED: return stream << "PAUSED";
                case ProgressTimer::State::STOPPING: return stream << "STOPPING";
            }
            return stream << "Unknown state: " << static_cast<int>(state);
        }
        ProgressTimer::~ProgressTimer() {
            stop();
        }
        void ProgressTimer::init(const shared_ptr<ProgressTimer::ContextInterface>& context, milliseconds delay, milliseconds interval, milliseconds offset) {
            ACSDK_DEBUG5(LX(__func__).d("delay", delay.count()).d("interval", interval.count()).d("offset", offset.count()));
            if (!context) {
                ACSDK_ERROR(LX("initFailed").d("reason", "nullContext"));
                return;
            }
            if (milliseconds::zero() == interval) {
                ACSDK_ERROR(LX("initFailed").d("reason", "intervalWasZero"));
                return;
            }
            lock_guard<mutex> callLock(m_callMutex);
            if (!setState(State::INITIALIZED)) {
                ACSDK_ERROR(LX("initFailed").d("reason", "setStateFailed"));
                return;
            }
            m_context = context;
            m_interval = interval;
            m_delay = delay >= offset ? delay : ProgressTimer::getNoDelay();
            m_offset = offset;
            m_progress = offset;
        }
        void ProgressTimer::start() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> callLock(m_callMutex);
            if (!setState(State::RUNNING)) {
                ACSDK_ERROR(LX("startFailed").d("reason", "setStateFailed"));
                return;
            }
            if (m_delay != ProgressTimer::getNoDelay()) {
                if (m_interval != ProgressTimer::getNoInterval()) m_target = std::min(m_delay, m_interval * ((m_offset / m_interval) + 1));
                else m_target = m_delay;
            } else {
                if (m_interval != ProgressTimer::getNoInterval()) m_target = m_interval * ((m_offset / m_interval) + 1);
                else {
                    ACSDK_DEBUG5(LX("startNotStartingThread").d("reason", "noTarget"));
                    setState(State::IDLE);
                    m_context.reset();
                    return;
                }
            }
            startThread();
        }
        void ProgressTimer::pause() {
            lock_guard<mutex> callLock(m_callMutex);
            pauseLocked();
        }
        void ProgressTimer::pauseLocked() {
            ACSDK_DEBUG5(LX(__func__));
            if (!setState(State::PAUSED)) {
                ACSDK_ERROR(LX("pauseFailed").d("reason", "setStateFailed"));
                return;
            }
            if (m_thread.joinable()) m_thread.join();
        }
        void ProgressTimer::resume() {
            lock_guard<mutex> callLock(m_callMutex);
            resumeLocked();
        }
        void ProgressTimer::resumeLocked() {
            ACSDK_DEBUG5(LX(__func__));
            if (!setState(State::RUNNING)) {
                ACSDK_ERROR(LX("resumeFailed").d("reason", "setStateFailed"));
                return;
            }
            startThread();
        }
        void ProgressTimer::stop() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> callLock(m_callMutex);
            {
                lock_guard<mutex> stateLock(m_stateMutex);
                if (State::IDLE == m_state) return;
            }
            if (!setState(State::STOPPING)) {
                ACSDK_ERROR(LX("stopFailed").d("reason", "setStateFailed"));
                return;
            }
            if (m_thread.joinable()) m_thread.join();
            if (!setState(State::IDLE)) {
                ACSDK_ERROR(LX("stopFailed").d("reason", "setStateFailed"));
                return;
            }
            m_context.reset();
            m_delay = ProgressTimer::getNoDelay();
            m_interval = ProgressTimer::getNoInterval();
            m_target = milliseconds::zero();
        }
        void ProgressTimer::updateInterval(const std::chrono::milliseconds& newInterval) {
            ACSDK_DEBUG5(LX(__func__));
            shared_ptr<ContextInterface> context;
            lock_guard<mutex> callLock(m_callMutex);
            {
                {
                    lock_guard<mutex> stateLock(m_stateMutex);
                    if (m_state == State::IDLE) {
                        ACSDK_ERROR(LX("updateIntervalFailed").d("reason", "invalidState"));
                        return;
                    }
                }
                if (m_context == nullptr) {
                    ACSDK_ERROR(LX("updateIntervalFailed").d("reason", "nullContext"));
                    return;
                }
                context = m_context;
                if (milliseconds::zero() == newInterval || ProgressTimer::getNoInterval() == newInterval) {
                    ACSDK_ERROR(LX("updateIntervalFailed").d("reason", "invalidNewInterval"));
                    return;
                }
                pauseLocked();
                m_interval = newInterval;
                if (ProgressTimer::getNoDelay() != m_delay && m_progress < m_delay) m_target = std::min(m_delay, m_interval * ((m_progress / m_interval) + 1));
                else m_target = m_interval * ((m_progress / m_interval) + 1);
                resumeLocked();
            }
            context->onProgressReportIntervalUpdated();
        }
        void ProgressTimer::onProgress(milliseconds progress) {
            ACSDK_DEBUG5(LX(__func__).d("progress", progress.count()));
            lock_guard<mutex> stateLock(m_stateMutex);
            m_progress = progress;
            m_gotProgress = true;
            m_wake.notify_all();
        }
        bool ProgressTimer::setState(State newState) {
            lock_guard<mutex> stateLock(m_stateMutex);
            bool allowed = false;
            switch(newState) {
                case State::IDLE: allowed = State::RUNNING == m_state || State::STOPPING == m_state; break;
                case State::INITIALIZED: allowed = State::IDLE == m_state; break;
                case State::RUNNING: allowed = State::INITIALIZED == m_state || State::PAUSED == m_state; break;
                case State::PAUSED: allowed = State::RUNNING == m_state; break;
                case State::STOPPING: allowed = true; break;
            }
            if (allowed) {
                ACSDK_DEBUG9(LX(__func__).d("state", m_state).d("newState", newState));
                m_state = newState;
                m_wake.notify_one();
            } else { ACSDK_ERROR(LX("setStateFailed").d("reason", "notAllowed").d("state", m_state).d("newState", newState)); }
            return allowed;
        }
        void ProgressTimer::startThread() {
            if (m_thread.joinable()) m_thread.join();
            m_thread = thread(&ProgressTimer::mainLoop, this);
        }
        void ProgressTimer::mainLoop() {
            ACSDK_DEBUG5(LX(__func__));
            unique_lock<mutex> stateLock(m_stateMutex);
            if (ProgressTimer::getNoDelay() == m_delay && ProgressTimer::getNoInterval() == m_interval) {
                ACSDK_DEBUG5(LX("mainLoopExiting").d("reason", "noDelayOrInterval"));
                return;
            }
            while(State::RUNNING == m_state) {
                m_gotProgress = false;
                stateLock.unlock();
                m_context->requestProgress();
                stateLock.lock();
                m_wake.wait(stateLock, [this] { return m_state != State::RUNNING || m_gotProgress; });
                if (m_state != State::RUNNING) break;
                if (m_progress >= m_target) {
                    if (m_target == m_delay) {
                        m_context->onProgressReportDelayElapsed();
                        if (m_interval != ProgressTimer::getNoInterval() && (m_target.count() % m_interval.count()) == 0) m_context->onProgressReportIntervalElapsed();
                    } else m_context->onProgressReportIntervalElapsed();
                    if (!updateTargetLocked()) {
                        ACSDK_DEBUG5(LX("mainLoopExiting").d("reason", "noTarget"));
                        return;
                    }
                } else {
                    milliseconds timeout = m_target - m_progress;
                    m_wake.wait_for(stateLock, timeout, [this] { return m_state != State::RUNNING; });
                }
            }
            ACSDK_DEBUG5(LX("mainLoopExiting").d("reason", "isStopping"));
        }
        bool ProgressTimer::updateTargetLocked() {
            ACSDK_DEBUG9(LX(__func__).d("target", m_target.count()).d("progress", m_progress.count()));
            if (m_progress < m_target) return true;
            if (ProgressTimer::getNoDelay() == m_delay) {
                if (ProgressTimer::getNoInterval() == m_interval) {
                    ACSDK_DEBUG9(LX("noTarget"));
                    return false;
                }
                m_target += m_interval;
                ACSDK_DEBUG9(LX("newTarget").d("target", m_target.count()));
                return true;
            }
            if (ProgressTimer::getNoInterval() == m_interval) {
                if (m_target == m_delay) {
                    m_delay = ProgressTimer::getNoDelay();
                    return false;
                }
                m_target = m_delay;
                ACSDK_DEBUG9(LX("newTarget").d("target", m_target.count()));
                return true;
            }
            if (m_target < m_delay) {
                m_target += m_interval;
                if (m_target > m_delay) m_target = m_delay;
            } else if (m_target == m_delay) {
                auto count = (m_delay.count() / m_interval.count()) + 1;
                m_target = m_interval * count;
            } else  m_target += m_interval;
            ACSDK_DEBUG9(LX("newTarget").d("target", m_target.count()));
            return true;
        }
    }
}