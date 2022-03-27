#ifndef ALEXA_CLIENT_SDK_CERTIFIEDSENDER_INCLUDE_CERTIFIEDSENDER_MESSAGESTORAGEINTERFACE_H_
#define ALEXA_CLIENT_SDK_CERTIFIEDSENDER_INCLUDE_CERTIFIEDSENDER_MESSAGESTORAGEINTERFACE_H_

#include <memory>
#include <string>
#include <queue>

namespace alexaClientSDK {
    namespace certifiedSender {
        using namespace std;
        class MessageStorageInterface {
        public:
            struct StoredMessage {
                StoredMessage() : id{0} {}
                StoredMessage(int id, const string& message, const string& uriPathExtension = "") : id{id}, message{message},
                                                                                                    uriPathExtension{uriPathExtension} {}
                int id;
                string message;
                string uriPathExtension;
            };
            virtual ~MessageStorageInterface() = default;
            virtual bool createDatabase();
            virtual bool open();
            virtual void close();
            virtual bool store(const string& message, int* id);
            virtual bool store(const string& message, const string& uriPathExtension, int* id);
            virtual bool load(queue<StoredMessage>* messageContainer);
            virtual bool erase(int messageId);
            virtual bool clearDatabase();
        };
    }
}
#endif