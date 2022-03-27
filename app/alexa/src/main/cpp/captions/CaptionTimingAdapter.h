#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONTIMINGADAPTER_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_CAPTIONTIMINGADAPTER_H_

#include <atomic>
#include <thread>
#include <vector>
#include <media_player/MediaPlayerObserverInterface.h>
#include <threading/TaskThread.h>
#include "CaptionPresenterInterface.h"
#include "CaptionTimingAdapterInterface.h"
#include "SystemClockDelay.h"
#include "TimingAdapterFactory.h"

namespace alexaClientSDK {
    namespace captions {
        class CaptionTimingAdapter : public CaptionTimingAdapterInterface {
        public:
            CaptionTimingAdapter(shared_ptr<CaptionPresenterInterface> presenter, shared_ptr<DelayInterface> delayInterface);
            virtual ~CaptionTimingAdapter() override;
            virtual void queueForDisplay(const CaptionFrame& captionFrame, bool autostart) override;
            virtual void reset() override;
            virtual void start() override;
            virtual void stop() override;
            virtual void pause() override;
        private:
            void presentCaptionFramesJob();
            void startCaptionFramesJob();
            shared_ptr<CaptionPresenterInterface> m_presenter;
            mutex m_mutex;
            size_t m_currentCaptionFrameIndex;
            bool m_isCurrentlyPresenting;
            bool m_mediaHasBeenPaused;
            vector<CaptionFrame> m_captionFrames;
            thread m_thread;
            string m_threadMoniker;
            shared_ptr<DelayInterface> m_delayInterface;
        };
    }
}
#endif