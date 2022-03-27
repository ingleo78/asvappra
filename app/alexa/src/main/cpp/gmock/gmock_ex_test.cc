#include "../gtest/gtest.h"
#include "gmock.h"

namespace {
  using testing::HasSubstr;
  using testing::internal::GoogleTestFailureException;
  class NonDefaultConstructible {
  public:
      explicit NonDefaultConstructible(int /* dummy */) {}
  };
  class MockFoo {
  public:
      MOCK_METHOD0(GetNonDefaultConstructible, NonDefaultConstructible());
  };
#if GTEST_HAS_EXCEPTIONS
  TEST(DefaultValueTest, ThrowsRuntimeErrorWhenNoDefaultValue) {
      MockFoo mock;
      try {
          mock.GetNonDefaultConstructible();
          FAIL() << "GetNonDefaultConstructible()'s return type has no default " << "value, so Google Mock should have thrown.";
      } catch (const GoogleTestFailureException& /* unused */) {
          FAIL() << "Google Test does not try to catch an exception of type " << "GoogleTestFailureException, which is used for reporting "
                 << "a failure to other testing frameworks.  Google Mock should " << "not throw a GoogleTestFailureException as it will kill the "
                 << "entire test program instead of just the current TEST.";
      } catch (const std::exception& ex) {
          EXPECT_THAT(ex.what(), HasSubstr("has no default value"));
      }
  }
#endif
}
