#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_TESTTIMINGADAPTERFACTORY_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_TEST_TESTTIMINGADAPTERFACTORY_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockCaptionTimingAdapter.h"
#include "MockSystemClockDelay.h"

namespace alexaClientSDK {
    namespace captions {
        namespace test {
            using namespace ::testing;
            class TestTimingAdapterFactory : public TimingAdapterFactory {
            public:
                TestTimingAdapterFactory() : TimingAdapterFactory{std::shared_ptr<NiceMock<MockSystemClockDelay>>()} {}
                ~TestTimingAdapterFactory();
                shared_ptr<CaptionTimingAdapterInterface> getTimingAdapter(shared_ptr<CaptionPresenterInterface> presenter) const;
                shared_ptr<MockCaptionTimingAdapter> getMockTimingAdapter();
            private:
                shared_ptr<NiceMock<MockCaptionTimingAdapter>> m_timingAdapter;
            };
        }
    }
}
#endif