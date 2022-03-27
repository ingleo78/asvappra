#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_MESSAGEREQUEST_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_MESSAGEREQUEST_H_

#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include "attachment/AttachmentReader.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace attachment;
            using namespace sdkInterfaces;
            class MessageRequest {
            public:
                struct NamedReader {
                    NamedReader(const string& name, shared_ptr<AttachmentReader> reader) : name{name}, reader{reader} {}
                    string name;
                    shared_ptr<AttachmentReader> reader;
                };
                MessageRequest(const string& jsonContent, const string& uriPathExtension = "");
                MessageRequest(const string& jsonContent, bool isSerialized, const string& uriPathExtension = "", vector<pair<string, string>> headers = {});
                virtual ~MessageRequest();
                void addAttachmentReader(const string& name, shared_ptr<AttachmentReader> attachmentReader);
                string getJsonContent() const;
                bool getIsSerialized() const;
                string getUriPathExtension() const;
                int attachmentReadersCount() const;
                shared_ptr<NamedReader> getAttachmentReader(size_t index);
                virtual void sendCompleted(MessageRequestObserverInterface::Status status);
                virtual void exceptionReceived(const string& exceptionMessage);
                void addObserver(shared_ptr<MessageRequestObserverInterface> observer);
                void removeObserver(shared_ptr<MessageRequestObserverInterface> observer);
                const vector<pair<string, string>>& getHeaders() const;
            protected:
                mutex m_observerMutex;
                unordered_set<shared_ptr<MessageRequestObserverInterface>> m_observers;
                string m_jsonContent;
                bool m_isSerialized;
                string m_uriPathExtension;
                vector<shared_ptr<NamedReader>> m_readers;
                vector<pair<string, string>> m_headers;
            };
        }
    }
}
#endif