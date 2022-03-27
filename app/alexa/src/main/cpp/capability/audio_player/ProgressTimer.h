#ifndef ACSDKAUDIOPLAYER_PROGRESSTIMER_H_
#define ACSDKAUDIOPLAYER_PROGRESSTIMER_H_

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        using namespace std;
        using namespace chrono;
        class ProgressTimer {
        public:
            class ContextInterface {
            public:
                virtual void requestProgress() = 0;
                virtual void onProgressReportDelayElapsed() = 0;
                virtual void onProgressReportIntervalElapsed() = 0;
                virtual void onProgressReportIntervalUpdated() = 0;
                virtual ~ContextInterface() = default;
            };
            static milliseconds getNoDelay() {
                return milliseconds::max();
            }
            static milliseconds getNoInterval() {
                return milliseconds::max();
            }
            ProgressTimer();
            virtual ~ProgressTimer();
            void init(const shared_ptr<ContextInterface>& context, milliseconds delay, milliseconds interval, milliseconds offset = milliseconds::zero());
            void start();
            void pause();
            void resume();
            void stop();
            void updateInterval(const milliseconds& newInterval);
            void onProgress(milliseconds progress);
        private:
            enum class State {
                IDLE,
                INITIALIZED,
                RUNNING,
                PAUSED,
                STOPPING,
            };
            friend ostream& operator<<(ostream& stream, ProgressTimer::State state);
            bool setState(State newState);
            void startThread();
            void mainLoop();
            bool updateTargetLocked();
            void pauseLocked();
            void resumeLocked();
            mutex m_callMutex;
            mutex m_stateMutex;
            State m_state;
            shared_ptr<ContextInterface> m_context;
            milliseconds m_delay;
            milliseconds m_interval;
            milliseconds m_offset;
            milliseconds m_target;
            bool m_gotProgress;
            milliseconds m_progress;
            condition_variable m_wake;
            thread m_thread;
        };
    }
}
#endif