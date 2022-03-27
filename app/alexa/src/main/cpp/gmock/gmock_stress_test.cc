#include "../gtest/gtest.h"
#include "gmock.h"

namespace testing {
    namespace {
        using ::testing::internal::ThreadWithParam;
        const int kMaxTestThreads = 50;
        const int kRepeat = 50;
        class MockFoo {
        public:
            MOCK_METHOD1(Bar, int(int n));
            MOCK_METHOD2(Baz, char(const char* s1, const internal::string& s2));
        };
        template <typename T> void JoinAndDelete(ThreadWithParam<T>* t) {
            t->Join();
            delete t;
        }
        using internal::linked_ptr;
        class Base {
        public:
            explicit Base(int a_x) : x_(a_x) {}
            virtual ~Base() {}
            int x() const { return x_; }
        private:
            int x_;
        };
        class Derived1 : public Base {
        public:
            Derived1(int a_x, int a_y) : Base(a_x), y_(a_y) {}
            int y() const { return y_; }
        private:
            int y_;
        };
        class Derived2 : public Base {
        public:
            Derived2(int a_x, int a_z) : Base(a_x), z_(a_z) {}
            int z() const { return z_; }
        private:
            int z_;
        };
        linked_ptr<Derived1> pointer1(new Derived1(1, 2));
        linked_ptr<Derived2> pointer2(new Derived2(3, 4));
        struct Dummy {};
        void TestConcurrentCopyAndReadLinkedPtr(Dummy /* dummy */) {
            EXPECT_EQ(1, pointer1->x());
            EXPECT_EQ(2, pointer1->y());
            EXPECT_EQ(3, pointer2->x());
            EXPECT_EQ(4, pointer2->z());
            linked_ptr<Derived1> p1(pointer1);
            EXPECT_EQ(1, p1->x());
            EXPECT_EQ(2, p1->y());
            linked_ptr<Base> p2;
            p2 = pointer1;
            EXPECT_EQ(1, p2->x());
            p2 = pointer2;
            EXPECT_EQ(3, p2->x());
        }
        const linked_ptr<Derived1> p0(new Derived1(1, 2));
        void TestConcurrentWriteToEqualLinkedPtr(Dummy /* dummy */) {
            linked_ptr<Derived1> p1(p0);
            linked_ptr<Derived1> p2(p0);
            EXPECT_EQ(1, p1->x());
            EXPECT_EQ(2, p1->y());
            EXPECT_EQ(1, p2->x());
            EXPECT_EQ(2, p2->y());
            p1.reset();
            p2 = p0;
            EXPECT_EQ(1, p2->x());
            EXPECT_EQ(2, p2->y());
        }
        void TestConcurrentMockObjects(Dummy /* dummy */) {
            MockFoo foo;
            ON_CALL(foo, Bar(_)).WillByDefault(Return(1));
            ON_CALL(foo, Baz(_, _)).WillByDefault(Return('b'));
            ON_CALL(foo, Baz(_, "you")).WillByDefault(Return('a'));
            EXPECT_CALL(foo, Bar(0)).Times(AtMost(3));
            EXPECT_CALL(foo, Baz(_, _));
            EXPECT_CALL(foo, Baz("hi", "you")).WillOnce(Return('z')).WillRepeatedly(DoDefault());
            EXPECT_EQ(1, foo.Bar(0));
            EXPECT_EQ(1, foo.Bar(0));
            EXPECT_EQ('z', foo.Baz("hi", "you"));
            EXPECT_EQ('a', foo.Baz("hi", "you"));
            EXPECT_EQ('b', foo.Baz("hi", "me"));
        }
        struct Helper1Param {
            MockFoo* mock_foo;
            int* count;
        };
        void Helper1(Helper1Param param) {
            for (int i = 0; i < kRepeat; i++) {
                const char ch = param.mock_foo->Baz("a", "b");
                if (ch == 'a') (*param.count)++;
                else { EXPECT_EQ('\0', ch); }
                EXPECT_EQ('\0', param.mock_foo->Baz("x", "y")) << "Expected failure.";
                EXPECT_EQ(1, param.mock_foo->Bar(5));
            }
        }
        void TestConcurrentCallsOnSameObject(Dummy /* dummy */) {
            MockFoo foo;
            ON_CALL(foo, Bar(_)).WillByDefault(Return(1));
            EXPECT_CALL(foo, Baz(_, "b")).Times(kRepeat).WillRepeatedly(Return('a'));
            EXPECT_CALL(foo, Baz(_, "c"));
            int count1 = 0;
            const Helper1Param param = { &foo, &count1 };
            ThreadWithParam<Helper1Param>* const t = new ThreadWithParam<Helper1Param>(Helper1, param, NULL);
            int count2 = 0;
            const Helper1Param param2 = { &foo, &count2 };
            Helper1(param2);
            JoinAndDelete(t);
            EXPECT_EQ(kRepeat, count1 + count2);
        }
        void Helper2(MockFoo* foo) {
            for (int i = 0; i < kRepeat; i++) {
                foo->Bar(2);
                foo->Bar(3);
            }
        }
        void TestPartiallyOrderedExpectationsWithThreads(Dummy /* dummy */) {
            MockFoo foo;
            Sequence s1, s2;
            {
                InSequence dummy;
                EXPECT_CALL(foo, Bar(0));
                EXPECT_CALL(foo, Bar(1)).InSequence(s1, s2);
            }
            EXPECT_CALL(foo, Bar(2)).Times(2*kRepeat).InSequence(s1).RetiresOnSaturation();
            EXPECT_CALL(foo, Bar(3)).Times(2*kRepeat).InSequence(s2);
            {
                InSequence dummy;
                EXPECT_CALL(foo, Bar(2)).InSequence(s1, s2);
                EXPECT_CALL(foo, Bar(4));
            }
            foo.Bar(0);
            foo.Bar(1);
            ThreadWithParam<MockFoo*>* const t = new ThreadWithParam<MockFoo*>(Helper2, &foo, NULL);
            Helper2(&foo);
            JoinAndDelete(t);
            foo.Bar(2);
            foo.Bar(4);
        }
        TEST(StressTest, CanUseGMockWithThreads) {
            void (*test_routines[])(Dummy dummy) = {
                &TestConcurrentCopyAndReadLinkedPtr,
                &TestConcurrentWriteToEqualLinkedPtr,
                &TestConcurrentMockObjects,
                &TestConcurrentCallsOnSameObject,
                &TestPartiallyOrderedExpectationsWithThreads,
            };
            const int kRoutines = sizeof(test_routines)/sizeof(test_routines[0]);
            const int kCopiesOfEachRoutine = kMaxTestThreads / kRoutines;
            const int kTestThreads = kCopiesOfEachRoutine * kRoutines;
            ThreadWithParam<Dummy>* threads[kTestThreads] = {};
            for (int i = 0; i < kTestThreads; i++) {
                threads[i] = new ThreadWithParam<Dummy>(test_routines[i % kRoutines], Dummy(), NULL);
                GTEST_LOG_(INFO) << "Thread #" << i << " running . . .";
            }
            for (int i = 0; i < kTestThreads; i++) {
                JoinAndDelete(threads[i]);
            }
            const TestInfo* const info = UnitTest::GetInstance()->current_test_info();
            const TestResult& result = *info->result();
            const int kExpectedFailures = (3*kRepeat + 1)*kCopiesOfEachRoutine;
            GTEST_CHECK_(kExpectedFailures == result.total_part_count()) << "Expected " << kExpectedFailures << " failures, but got "
                << result.total_part_count();
        }
    }
}
int main(int argc, char **argv) {
    testing::InitGoogleMock(&argc, argv);
    const int exit_code = RUN_ALL_TESTS();
    GTEST_CHECK_(exit_code != 0) << "RUN_ALL_TESTS() did not fail as expected";
    printf("\nPASS\n");
    return 0;
}