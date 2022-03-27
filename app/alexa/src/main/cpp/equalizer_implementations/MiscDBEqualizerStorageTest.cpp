#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/Audio/EqualizerStorageInterfaceTest.h>
#include <sdkinterfaces/Storage/StubMiscStorage.h>
#include "MiscDBEqualizerStorage.h"

namespace alexaClientSDK {
    namespace equalizer {
        namespace test {
            using namespace testing;
            using namespace audio::test;
            using namespace storage::test;
            using ESIT = EqualizerStorageInterfaceTest;
            INSTANTIATE_TEST_CASE_P(MiscDBStorageTests, ESIT, Values([]() { return MiscDBEqualizerStorage::create(StubMiscStorage::create()); }));
        }
    }
}