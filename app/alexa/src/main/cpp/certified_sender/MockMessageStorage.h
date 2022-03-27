#ifndef ALEXA_CLIENT_SDK_CERTIFIEDSENDER_TEST_MOCKMESSAGESTORAGE_H_
#define ALEXA_CLIENT_SDK_CERTIFIEDSENDER_TEST_MOCKMESSAGESTORAGE_H_

#include <gmock/gmock.h>
#include "MessageStorageInterface.h"

namespace alexaClientSDK {
    namespace certifiedSender {
        namespace test {
            class MockMessageStorage : public MessageStorageInterface {
            public:
                MOCK_METHOD0(createDatabase, bool());
                MOCK_METHOD0(open, bool());
                MOCK_METHOD0(close, void());
                //MOCK_METHOD2(store, bool(const std::string& message, int* id));
                MOCK_METHOD3(store, bool(const std::string& message, const std::string& uriPathExtension, int* id));
                MOCK_METHOD1(load, bool(std::queue<StoredMessage>* messageContainer));
                MOCK_METHOD1(erase, bool(int messageId));
                MOCK_METHOD0(clearDatabase, bool());
            };
        }
    }
}
#endif