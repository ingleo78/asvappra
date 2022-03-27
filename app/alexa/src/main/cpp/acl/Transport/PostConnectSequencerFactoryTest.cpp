#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "MockPostConnectOperation.h"
#include "MockPostConnectOperationProvider.h"
#include "PostConnectSequencerFactory.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                using namespace std;
                using namespace avsCommon;
                using namespace sdkInterfaces;
                using namespace testing;
                class PostConnectSequencerFactoryTest : public Test {};
                TEST_F(PostConnectSequencerFactoryTest, test_createWithNullProviders) {
                    vector<shared_ptr<PostConnectOperationProviderInterface>> providers;
                    auto provider1 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    auto provider2 = nullptr;
                    auto provider3 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    providers.push_back(provider1);
                    providers.push_back(provider2);
                    providers.push_back(provider3);
                    auto instance = PostConnectSequencerFactory::create(providers);
                    ASSERT_EQ(instance, nullptr);
                }
                TEST_F(PostConnectSequencerFactoryTest, test_createPostConnectCallsProviders) {
                    vector<shared_ptr<PostConnectOperationProviderInterface>> providers;
                    auto provider1 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    auto provider2 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    auto provider3 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    providers.push_back(provider1);
                    providers.push_back(provider2);
                    providers.push_back(provider3);
                    EXPECT_CALL(*provider1, createPostConnectOperation()).Times(1);
                    EXPECT_CALL(*provider2, createPostConnectOperation()).Times(1);
                    EXPECT_CALL(*provider3, createPostConnectOperation()).Times(1);
                    auto instance = PostConnectSequencerFactory::create(providers);
                    ASSERT_NE(instance->createPostConnect(), nullptr);
                }
                TEST_F(PostConnectSequencerFactoryTest, test_createPostConnectWhenProviderReturnsNull) {
                    vector<shared_ptr<PostConnectOperationProviderInterface>> providers;
                    auto provider1 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    auto provider2 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    auto provider3 = make_shared<NiceMock<MockPostConnectOperationProvider>>();
                    providers.push_back(provider1);
                    providers.push_back(provider2);
                    providers.push_back(provider3);
                    auto operation1 = make_shared<NiceMock<MockPostConnectOperation>>();
                    auto operation3 = make_shared<NiceMock<MockPostConnectOperation>>();
                    EXPECT_CALL(*provider1, createPostConnectOperation()).Times(1).WillOnce(Return(operation1));
                    EXPECT_CALL(*provider2, createPostConnectOperation()).Times(1).WillOnce(Return(nullptr));
                    EXPECT_CALL(*provider3, createPostConnectOperation()).Times(1).WillOnce(Return(operation3));
                    auto instance = PostConnectSequencerFactory::create(providers);
                    ASSERT_NE(instance->createPostConnect(), nullptr);
                }
            }
        }
    }
}