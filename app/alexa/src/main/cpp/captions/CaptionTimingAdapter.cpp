#include <algorithm>
#include <iterator>
#include <avs/FocusState.h>
#include <logger/ThreadMoniker.h>
#include <logger/LogEntry.h>
#include <logger/Logger.h>
#include "DelayInterface.h"
#include "CaptionTimingAdapter.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace utils;
        using namespace logger;
        using namespace mediaPlayer;
        using namespace threading;
        static const string TAG = "CaptionTimingAdapter";
        #define LX(event) LogEntry(TAG, event)
        CaptionTimingAdapter::CaptionTimingAdapter(shared_ptr<CaptionPresenterInterface> presenter, shared_ptr<DelayInterface> delayInterface) :
                                                  m_presenter{presenter}, m_currentCaptionFrameIndex{0}, m_isCurrentlyPresenting{false},
                                                  m_mediaHasBeenPaused{false}, m_threadMoniker{ThreadMoniker::generateMoniker()},
                                                  m_delayInterface{delayInterface} {}
        CaptionTimingAdapter::~CaptionTimingAdapter() {
            ACSDK_DEBUG3(LX(__func__));
            unique_lock<mutex> lock(m_mutex);
            m_mediaHasBeenPaused = true;
            lock.unlock();
            if (m_thread.joinable()) m_thread.join();
            lock.lock();
            m_captionFrames.clear();
        }
        void CaptionTimingAdapter::reset() {
            ACSDK_DEBUG3(LX(__func__));
            unique_lock<mutex> lock(m_mutex);
            m_mediaHasBeenPaused = true;
            lock.unlock();
            if (m_thread.joinable()) m_thread.join();
            lock.lock();
            m_captionFrames.clear();
            m_currentCaptionFrameIndex = 0;
            m_mediaHasBeenPaused = false;
        }
        void CaptionTimingAdapter::queueForDisplay(const CaptionFrame& captionFrame, bool autostart) {
            ACSDK_DEBUG3(LX(__func__).d("captionId", captionFrame.getSourceId()));
            unique_lock<mutex> lock(m_mutex);
            m_captionFrames.emplace_back(captionFrame);
            ACSDK_DEBUG3(LX("captionFrameQueued").d("currentIndex", m_currentCaptionFrameIndex).d("numberOfCaptionFrames", m_captionFrames.size()));
            lock.unlock();
            if (autostart) startCaptionFramesJob();
            else { ACSDK_DEBUG3(LX("presentCaptionFramesJobNotStarted").d("reason", "mediaHasBeenPaused")); }
        }
        void CaptionTimingAdapter::startCaptionFramesJob() {
            unique_lock<mutex> lock(m_mutex);
            if (!m_isCurrentlyPresenting && !m_mediaHasBeenPaused) {
                ACSDK_DEBUG3(LX("startingJobToPresentCaptionFrames"));
                if (m_thread.joinable()) m_thread.join();
                m_isCurrentlyPresenting = true;
                lock.unlock();
                m_thread = thread{bind(&CaptionTimingAdapter::presentCaptionFramesJob, this)};
            } else { ACSDK_DEBUG3(LX("presentCaptionFramesJobNotStarted").d("reason", "jobAlreadyRunning")); }
        }
        void CaptionTimingAdapter::presentCaptionFramesJob() {
            ACSDK_DEBUG3(LX(__func__));
            ThreadMoniker::setThisThreadMoniker(m_threadMoniker);
            CaptionFrame frame;
            while(true) {
                unique_lock<mutex> lock(m_mutex);
                if (m_mediaHasBeenPaused) {
                    lock.unlock();
                    break;
                }
                if (m_currentCaptionFrameIndex >= m_captionFrames.size()) {
                    lock.unlock();
                    break;
                }
                frame = m_captionFrames[m_currentCaptionFrameIndex];
                ACSDK_DEBUG3(LX("sendingCaptionFrameToPresenter").d("sourceId", frame.getSourceId()).d("currentIndex", m_currentCaptionFrameIndex)
                    .d("numberOfCaptionFrames", m_captionFrames.size()).d("delay", std::to_string(frame.getDelay().count()))
                    .d("duration", std::to_string(frame.getDuration().count())));
                m_currentCaptionFrameIndex++;
                lock.unlock();
                m_delayInterface->delay(frame.getDelay());
                lock.lock();
                if (m_mediaHasBeenPaused) {
                    m_presenter->onCaptionActivity(frame,FocusState::BACKGROUND);
                    lock.unlock();
                    break;
                }
                m_presenter->onCaptionActivity(frame,FocusState::FOREGROUND);
                lock.unlock();
                m_delayInterface->delay(frame.getDuration());
            }
            unique_lock<mutex> lock(m_mutex);
            if (m_mediaHasBeenPaused) {
                m_presenter->onCaptionActivity(frame,FocusState::BACKGROUND);
                ACSDK_DEBUG3(LX("endingCaptionDisplay").d("reason", "mediaNotPlaying"));
            } else {
                m_presenter->onCaptionActivity(frame,FocusState::NONE);
                ACSDK_DEBUG3(LX("endingCaptionDisplay").d("reason", "reachedEndOfCaptions"));
            }
            m_isCurrentlyPresenting = false;
        }
        void CaptionTimingAdapter::start() {
            ACSDK_DEBUG3(LX(__func__));
            unique_lock<mutex> lock(m_mutex);
            m_mediaHasBeenPaused = false;
            lock.unlock();
            startCaptionFramesJob();
        }
        void CaptionTimingAdapter::stop() {
            ACSDK_DEBUG3(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_mediaHasBeenPaused = true;
            m_currentCaptionFrameIndex = 0;
        }
        void CaptionTimingAdapter::pause() {
            ACSDK_DEBUG3(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_mediaHasBeenPaused = true;
        }
    }
}