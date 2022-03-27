#include <memory>
#include "TimingAdapterFactory.h"
#include "CaptionTimingAdapter.h"

namespace alexaClientSDK {
    namespace captions {
        TimingAdapterFactory::TimingAdapterFactory(shared_ptr<DelayInterface> delayInterface) : m_delayInterface{delayInterface} {
            if (!m_delayInterface) m_delayInterface = make_shared<SystemClockDelay>();
        }
        shared_ptr<CaptionTimingAdapterInterface> TimingAdapterFactory::getTimingAdapter(shared_ptr<CaptionPresenterInterface> presenter) const {
            return shared_ptr<CaptionTimingAdapterInterface>(new CaptionTimingAdapter(presenter, m_delayInterface));
        }
    }
}