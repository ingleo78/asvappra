#include <iostream>
#include "../gtest/gtest.h"
#include "gmock.h"

#if GTEST_OS_WINDOWS_MOBILE
# include <tchar.h>
GTEST_API_ int _tmain(int argc, TCHAR** argv) {
#else
GTEST_API_ int main(int argc, char** argv) {
#endif
    std::cout << "Running main() from gmock_main.cc\n";
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}