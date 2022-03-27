#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_TIMINGADAPTERFACTORY_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_TIMINGADAPTERFACTORY_H_

#include "CaptionPresenterInterface.h"
#include "CaptionTimingAdapterInterface.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        class TimingAdapterFactory {
        public:
            TimingAdapterFactory(shared_ptr<DelayInterface> delayInterface = nullptr);
            virtual ~TimingAdapterFactory() = default;
            virtual shared_ptr<CaptionTimingAdapterInterface> getTimingAdapter(shared_ptr<CaptionPresenterInterface> presenter) const;
        private:
            shared_ptr<DelayInterface> m_delayInterface;
        };
    }
}
#endif