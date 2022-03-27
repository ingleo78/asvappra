#include <logger/Logger.h>
#include <timing/Stopwatch.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                static const std::string TAG("Stopwatch");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                static std::chrono::milliseconds elapsed(
                    std::chrono::steady_clock::time_point later,
                    std::chrono::steady_clock::time_point earlier) {
                    if (earlier >= later) return std::chrono::milliseconds::zero();
                    return std::chrono::duration_cast<std::chrono::milliseconds>(later - earlier);
                }
                Stopwatch::Stopwatch() {
                    reset();
                }
                bool Stopwatch::start() {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_state != State::RESET) {
                        ACSDK_ERROR(LX("startFailed").d("reason", "stateNotRESET"));
                        return false;
                    }
                    m_startTime = std::chrono::steady_clock::now();
                    m_state = State::RUNNING;
                    return true;
                }
                bool Stopwatch::pause() {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_state != State::RUNNING) {
                        ACSDK_ERROR(LX("pauseFailed").d("reason", "stateNotRUNNING"));
                        return false;
                    }
                    m_pauseTime = std::chrono::steady_clock::now();
                    m_state = State::PAUSED;
                    return true;
                }
                bool Stopwatch::resume() {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_state != State::PAUSED) {
                        ACSDK_ERROR(LX("resumeFailed").d("reason", "stateNotPAUSED"));
                        return false;
                    }
                    m_totalTimePaused += elapsed(std::chrono::steady_clock::now(), m_pauseTime);
                    m_state = State::RUNNING;
                    return true;
                }
                void Stopwatch::stop() {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_state != State::RESET && m_state != State::STOPPED) {
                        m_stopTime = std::chrono::steady_clock::now();
                    }
                    m_state = State::STOPPED;
                }
                void Stopwatch::reset() {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_state = State::RESET;
                    m_startTime = std::chrono::steady_clock::time_point();
                    m_pauseTime = std::chrono::steady_clock::time_point();
                    m_stopTime = std::chrono::steady_clock::time_point();
                    m_totalTimePaused = std::chrono::milliseconds::zero();
                }
                std::chrono::milliseconds Stopwatch::getElapsed() {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    switch (m_state) {
                        case State::RESET: return std::chrono::milliseconds::zero();
                        case State::RUNNING: return elapsed(std::chrono::steady_clock::now(), m_startTime) - m_totalTimePaused;
                        case State::PAUSED: return elapsed(m_pauseTime, m_startTime) - m_totalTimePaused;
                        case State::STOPPED: return elapsed(m_stopTime, m_startTime) - m_totalTimePaused;
                    }
                    return std::chrono::milliseconds::zero();
                }
            }
        }
    }
}
