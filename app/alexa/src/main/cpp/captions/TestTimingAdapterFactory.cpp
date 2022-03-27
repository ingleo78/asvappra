#include "TestTimingAdapterFactory.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            TestTimingAdapterFactory::~TestTimingAdapterFactory() {
                m_timingAdapter.reset();
            }
            shared_ptr<CaptionTimingAdapterInterface> TestTimingAdapterFactory::getTimingAdapter(shared_ptr<CaptionPresenterInterface> presenter) const {
                return m_timingAdapter;
            }
            shared_ptr<MockCaptionTimingAdapter> TestTimingAdapterFactory::getMockTimingAdapter() {
                if (!m_timingAdapter) m_timingAdapter = make_shared<NiceMock<MockCaptionTimingAdapter>>();
                return m_timingAdapter;
            }
        }
    }
}