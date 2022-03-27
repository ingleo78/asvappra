#include <string>
#include "../gtest/gtest.h"
#include "../gtest/gtest-spi.h"
#include "gmock.h"
#include "gmock-generated-nice-strict.h"

class Mock {
public:
    Mock() {}
    MOCK_METHOD0(DoThis, void());
private:
    GTEST_DISALLOW_COPY_AND_ASSIGN_(Mock);
};
namespace testing {
    namespace gmock_nice_strict_test {
        using testing::internal::string;
        using testing::GMOCK_FLAG(verbose);
        using testing::HasSubstr;
        using testing::NaggyMock;
        using testing::NiceMock;
        using testing::StrictMock;
    #if GTEST_HAS_STREAM_REDIRECTION
        using testing::internal::CaptureStdout;
        using testing::internal::GetCapturedStdout;
    #endif
        class Foo {
        public:
            virtual ~Foo() {}
            virtual void DoThis();
            virtual int DoThat(bool flag);
        };
        class MockFoo : public Foo {
        public:
            MockFoo() {}
            void Delete() { delete this; }
            MOCK_METHOD0(DoThis, void());
            MOCK_METHOD1(DoThat, int(bool flag));
        private:
            GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFoo);
        };
        class MockBar {
        public:
            explicit MockBar(const string& s) : str_(s) {}
            MockBar(char a1, char a2, string a3, string a4, int a5, int a6, const string& a7, const string& a8, bool a9, bool a10) {
                str_ = string() + a1 + a2 + a3 + a4 + static_cast<char>(a5) + static_cast<char>(a6) + a7 + a8 + (a9 ? 'T' : 'F') +
                       (a10 ? 'T' : 'F');
            }
            virtual ~MockBar() {}
            const string& str() const { return str_; }
            MOCK_METHOD0(This, int());
            MOCK_METHOD2(That, string(int, bool));
        private:
            string str_;
            GTEST_DISALLOW_COPY_AND_ASSIGN_(MockBar);
        };
    #if GTEST_HAS_STREAM_REDIRECTION
        TEST(RawMockTest, WarningForUninterestingCall) {
            const string saved_flag = GMOCK_FLAG(verbose);
            GMOCK_FLAG(verbose) = "warning";
            MockFoo raw_foo;
            CaptureStdout();
            raw_foo.DoThis();
            raw_foo.DoThat(true);
            EXPECT_THAT(GetCapturedStdout(),HasSubstr("Uninteresting mock function call"));
            GMOCK_FLAG(verbose) = saved_flag;
        }
        TEST(RawMockTest, WarningForUninterestingCallAfterDeath) {
            const string saved_flag = GMOCK_FLAG(verbose);
            GMOCK_FLAG(verbose) = "warning";
            MockFoo* const raw_foo = new MockFoo;
            ON_CALL(*raw_foo, DoThis()).WillByDefault(Invoke(raw_foo, &MockFoo::Delete));
            CaptureStdout();
            raw_foo->DoThis();
            EXPECT_THAT(GetCapturedStdout(),HasSubstr("Uninteresting mock function call"));
            GMOCK_FLAG(verbose) = saved_flag;
        }
        TEST(RawMockTest, InfoForUninterestingCall) {
            MockFoo raw_foo;
            const string saved_flag = GMOCK_FLAG(verbose);
            GMOCK_FLAG(verbose) = "info";
            CaptureStdout();
            raw_foo.DoThis();
            EXPECT_THAT(GetCapturedStdout(),HasSubstr("Uninteresting mock function call"));
            GMOCK_FLAG(verbose) = saved_flag;
        }
        TEST(NiceMockTest, NoWarningForUninterestingCall) {
            NiceMock<MockFoo> nice_foo;
            CaptureStdout();
            nice_foo.DoThis();
            nice_foo.DoThat(true);
            EXPECT_EQ("", GetCapturedStdout());
        }
        TEST(NiceMockTest, NoWarningForUninterestingCallAfterDeath) {
            NiceMock<MockFoo>* const nice_foo = new NiceMock<MockFoo>;
            ON_CALL(*nice_foo, DoThis()).WillByDefault(Invoke(nice_foo, &MockFoo::Delete));
            CaptureStdout();
            nice_foo->DoThis();
            EXPECT_EQ("", GetCapturedStdout());
        }
        TEST(NiceMockTest, InfoForUninterestingCall) {
            NiceMock<MockFoo> nice_foo;
            const string saved_flag = GMOCK_FLAG(verbose);
            GMOCK_FLAG(verbose) = "info";
            CaptureStdout();
            nice_foo.DoThis();
            EXPECT_THAT(GetCapturedStdout(),HasSubstr("Uninteresting mock function call"));
            GMOCK_FLAG(verbose) = saved_flag;
        }
    #endif
        TEST(NiceMockTest, AllowsExpectedCall) {
            NiceMock<MockFoo> nice_foo;
            EXPECT_CALL(nice_foo, DoThis());
            nice_foo.DoThis();
        }
        TEST(NiceMockTest, UnexpectedCallFails) {
            NiceMock<MockFoo> nice_foo;
            EXPECT_CALL(nice_foo, DoThis()).Times(0);
            EXPECT_NONFATAL_FAILURE(nice_foo.DoThis(), "called more times than expected");
        }
        TEST(NiceMockTest, NonDefaultConstructor) {
            NiceMock<MockBar> nice_bar("hi");
            EXPECT_EQ("hi", nice_bar.str());
            nice_bar.This();
            nice_bar.That(5, true);
        }
        TEST(NiceMockTest, NonDefaultConstructor10) {
            NiceMock<MockBar> nice_bar('a', 'b', "c", "d", 'e', 'f',"g", "h", true, false);
            EXPECT_EQ("abcdefghTF", nice_bar.str());
            nice_bar.This();
            nice_bar.That(5, true);
        }
    #if !GTEST_OS_SYMBIAN && !GTEST_OS_WINDOWS_MOBILE
        TEST(NiceMockTest, AcceptsClassNamedMock) {
            NiceMock< ::Mock> nice;
            EXPECT_CALL(nice, DoThis());
            nice.DoThis();
        }
    #endif
    #if GTEST_HAS_STREAM_REDIRECTION
        TEST(NaggyMockTest, WarningForUninterestingCall) {
            const string saved_flag = GMOCK_FLAG(verbose);
            GMOCK_FLAG(verbose) = "warning";
            NaggyMock<MockFoo> naggy_foo;
            CaptureStdout();
            naggy_foo.DoThis();
            naggy_foo.DoThat(true);
            EXPECT_THAT(GetCapturedStdout(),HasSubstr("Uninteresting mock function call"));
            GMOCK_FLAG(verbose) = saved_flag;
        }
        TEST(NaggyMockTest, WarningForUninterestingCallAfterDeath) {
            const string saved_flag = GMOCK_FLAG(verbose);
            GMOCK_FLAG(verbose) = "warning";
            NaggyMock<MockFoo>* const naggy_foo = new NaggyMock<MockFoo>;
            ON_CALL(*naggy_foo, DoThis()).WillByDefault(Invoke(naggy_foo, &MockFoo::Delete));
            CaptureStdout();
            naggy_foo->DoThis();
            EXPECT_THAT(GetCapturedStdout(),HasSubstr("Uninteresting mock function call"));
            GMOCK_FLAG(verbose) = saved_flag;
        }
    #endif
        TEST(NaggyMockTest, AllowsExpectedCall) {
            NaggyMock<MockFoo> naggy_foo;
            EXPECT_CALL(naggy_foo, DoThis());
            naggy_foo.DoThis();
        }
        TEST(NaggyMockTest, UnexpectedCallFails) {
            NaggyMock<MockFoo> naggy_foo;
            EXPECT_CALL(naggy_foo, DoThis()).Times(0);
            EXPECT_NONFATAL_FAILURE(naggy_foo.DoThis(), "called more times than expected");
        }
        TEST(NaggyMockTest, NonDefaultConstructor) {
            NaggyMock<MockBar> naggy_bar("hi");
            EXPECT_EQ("hi", naggy_bar.str());
            naggy_bar.This();
            naggy_bar.That(5, true);
        }
        TEST(NaggyMockTest, NonDefaultConstructor10) {
            NaggyMock<MockBar> naggy_bar('0', '1', "2", "3", '4', '5',"6", "7", true, false);
            EXPECT_EQ("01234567TF", naggy_bar.str());
            naggy_bar.This();
            naggy_bar.That(5, true);
        }
    #if !GTEST_OS_SYMBIAN && !GTEST_OS_WINDOWS_MOBILE
        TEST(NaggyMockTest, AcceptsClassNamedMock) {
            NaggyMock< ::Mock> naggy;
            EXPECT_CALL(naggy, DoThis());
            naggy.DoThis();
        }
    #endif
        TEST(StrictMockTest, AllowsExpectedCall) {
            StrictMock<MockFoo> strict_foo;
            EXPECT_CALL(strict_foo, DoThis());
            strict_foo.DoThis();
        }
        TEST(StrictMockTest, UnexpectedCallFails) {
            StrictMock<MockFoo> strict_foo;
            EXPECT_CALL(strict_foo, DoThis()).Times(0);
            EXPECT_NONFATAL_FAILURE(strict_foo.DoThis(),"called more times than expected");
        }
        TEST(StrictMockTest, UninterestingCallFails) {
            StrictMock<MockFoo> strict_foo;
            EXPECT_NONFATAL_FAILURE(strict_foo.DoThis(),"Uninteresting mock function call");
        }
        TEST(StrictMockTest, UninterestingCallFailsAfterDeath) {
            StrictMock<MockFoo>* const strict_foo = new StrictMock<MockFoo>;
            ON_CALL(*strict_foo, DoThis()).WillByDefault(Invoke(strict_foo, &MockFoo::Delete));
            EXPECT_NONFATAL_FAILURE(strict_foo->DoThis(),"Uninteresting mock function call");
        }
        TEST(StrictMockTest, NonDefaultConstructor) {
            StrictMock<MockBar> strict_bar("hi");
            EXPECT_EQ("hi", strict_bar.str());
            EXPECT_NONFATAL_FAILURE(strict_bar.That(5, true),"Uninteresting mock function call");
        }
        TEST(StrictMockTest, NonDefaultConstructor10) {
            StrictMock<MockBar> strict_bar('a', 'b', "c", "d", 'e', 'f',"g", "h", true, false);
            EXPECT_EQ("abcdefghTF", strict_bar.str());
            EXPECT_NONFATAL_FAILURE(strict_bar.That(5, true),"Uninteresting mock function call");
        }
    #if !GTEST_OS_SYMBIAN && !GTEST_OS_WINDOWS_MOBILE
        TEST(StrictMockTest, AcceptsClassNamedMock) {
            StrictMock< ::Mock> strict;
            EXPECT_CALL(strict, DoThis());
            strict.DoThis();
        }
    #endif
    }
}