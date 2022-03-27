#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTDIRECTIVEHANDLER_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTDIRECTIVEHANDLER_H_

#include <condition_variable>
#include <string>
#include <future>
#include <fstream>
#include <chrono>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <avs/attachment/AttachmentManager.h>
#include <json/JSONUtils.h>

using namespace alexaClientSDK::avsCommon;
namespace alexaClientSDK {
    namespace integration {
        namespace test {
            using namespace std;
            using namespace sdkInterfaces;
            using namespace avs;
            class TestDirectiveHandler : public DirectiveHandlerInterface {
            public:
                TestDirectiveHandler(DirectiveHandlerConfiguration config);
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result) override;
                bool handleDirective(const string& messageId) override;
                void cancelDirective(const string& messageId) override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                void onDeregistered() override;
                class DirectiveParams {
                public:
                    DirectiveParams();
                    bool isUnset() const {
                        return Type::UNSET == type;
                    }
                    bool isHandleImmediately() const {
                        return Type::HANDLE_IMMEDIATELY == type;
                    }
                    bool isPreHandle() const {
                        return Type::PREHANDLE == type;
                    }
                    bool isHandle() const {
                        return Type::HANDLE == type;
                    }
                    bool isCancel() const {
                        return Type::CANCEL == type;
                    }
                    bool isTimeout() const {
                        return Type::TIMEOUT == type;
                    }
                    enum class Type {
                        UNSET,
                        HANDLE_IMMEDIATELY,
                        PREHANDLE,
                        HANDLE,
                        CANCEL,
                        TIMEOUT
                    };
                    Type type;
                    shared_ptr<AVSDirective> directive;
                    shared_ptr<DirectiveHandlerResultInterface> result;
                };
                DirectiveParams waitForNext(const seconds duration);
            private:
                mutex m_mutex;
                condition_variable m_wakeTrigger;
                deque<DirectiveParams> m_queue;
                unordered_map<string, shared_ptr<DirectiveHandlerResultInterface>> m_results;
                unordered_map<string, shared_ptr<AVSDirective>> m_directives;
                DirectiveHandlerConfiguration m_configuration;
            };
        }
    }
}
#endif