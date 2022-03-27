#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_H_

#include "gmock-actions.h"
#include "gmock-cardinalities.h"
#include "gmock-generated-actions.h"
#include "gmock-generated-function-mockers.h"
#include "gmock-generated-nice-strict.h"
#include "gmock-generated-matchers.h"
#include "gmock-matchers.h"
#include "gmock-more-actions.h"
#include "gmock-more-matchers.h"
#include "internal/gmock-internal-utils.h"

namespace testing {
    GMOCK_DECLARE_bool_(catch_leaked_mocks);
    GMOCK_DECLARE_string_(verbose);
    GTEST_API_ void InitGoogleMock(int* argc, char** argv);
    GTEST_API_ void InitGoogleMock(int* argc, wchar_t** argv);
}
#endif