#include "MockDirectiveHandler.h"

using namespace ::testing;

namespace alexaClientSDK {
    namespace adsl {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            const milliseconds MockDirectiveHandler::DEFAULT_HANDLING_TIME_MS(0);
            const milliseconds MockDirectiveHandler::DEFAULT_DONE_TIMEOUT_MS(15000);
            void DirectiveHandlerMockAdapter::preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result) {
                shared_ptr<DirectiveHandlerResultInterface> temp(std::move(result));
                preHandleDirective(directive, temp);
            }
            shared_ptr<NiceMock<MockDirectiveHandler>> MockDirectiveHandler::create(DirectiveHandlerConfiguration config, milliseconds handlingTimeMs) {
                /*auto result = std::make_shared<NiceMock<MockDirectiveHandler>>(config, handlingTimeMs);
                ON_CALL(*result->get(), handleDirectiveImmediately(_)).WillByDefault(Invoke(result.get(), &MockDirectiveHandler::mockHandleDirectiveImmediately));
                //ON_CALL(*result->get(), preHandleDirective(_, _)).WillByDefault(Invoke(result.get(), &MockDirectiveHandler::mockPreHandleDirective));
                ON_CALL(*result->get(), handleDirective(_)).WillByDefault(Invoke(result.get(), &MockDirectiveHandler::mockHandleDirective));
                ON_CALL(*result->get(), cancelDirective(_)).WillByDefault(Invoke(result.get(), &MockDirectiveHandler::mockCancelDirective));
                ON_CALL(*result->get(), onDeregistered()).WillByDefault(Invoke(result.get(), &MockDirectiveHandler::mockOnDeregistered));
                ON_CALL(*result->get(), getConfiguration()).WillByDefault(Return(config));*/
                return nullptr;
            }
            MockDirectiveHandler::MockDirectiveHandler(DirectiveHandlerConfiguration config, milliseconds handlingTimeMs) : m_handlingTimeMs{handlingTimeMs},
                                                       m_isCompleted{false}, m_isShuttingDown{false}, m_preHandlingPromise{},
                                                       m_preHandlingFuture{m_preHandlingPromise.get_future()}, m_handlingPromise{},
                                                       m_handlingFuture{m_handlingPromise.get_future()}, m_cancelingPromise{},
                                                       m_cancelingFuture{m_cancelingPromise.get_future()}, m_completedPromise{},
                                                       m_completedFuture{m_completedPromise.get_future()} {}
            MockDirectiveHandler::~MockDirectiveHandler() {
                shutdown();
            }
            void MockDirectiveHandler::mockHandleDirectiveImmediately(std::shared_ptr<AVSDirective> directive) {
                m_handlingPromise.set_value();
            }
            void MockDirectiveHandler::mockPreHandleDirective(shared_ptr<AVSDirective> directive, shared_ptr<DirectiveHandlerResultInterface> result) {
                m_directive = directive;
                m_result = result;
                m_preHandlingPromise.set_value();
            }
            bool MockDirectiveHandler::mockHandleDirective(const string& messageId) {
                if (!m_directive || m_directive->getMessageId() != messageId) return false;
                m_doHandleDirectiveThread = thread(&MockDirectiveHandler::doHandleDirective, this, messageId);
                return true;
            }
            void MockDirectiveHandler::mockCancelDirective(const string& messageId) {
                if (!m_directive || m_directive->getMessageId() != messageId) return;
                m_cancelingPromise.set_value();
                shutdown();
            }
            void MockDirectiveHandler::doHandleDirective(const string& messageId) {
                auto wake = [this]() { return m_isCompleted || m_isShuttingDown; };
                m_handlingPromise.set_value();
                unique_lock<mutex> lock(m_mutex);
                m_wakeNotifier.wait_for(lock, m_handlingTimeMs, wake);
                if (!m_isShuttingDown) m_isCompleted = true;
                if (m_isCompleted) {
                    m_result->setCompleted();
                    m_completedPromise.set_value();
                }
            }
            void MockDirectiveHandler::mockOnDeregistered() {}
            void MockDirectiveHandler::doHandlingCompleted() {
                lock_guard<mutex> lock(m_mutex);
                m_isCompleted = true;
                m_wakeNotifier.notify_all();
            }
            void MockDirectiveHandler::doPreHandlingFailed(shared_ptr<AVSDirective> directive, shared_ptr<DirectiveHandlerResultInterface> result) {
                m_directive = directive;
                m_result = result;
                m_result->setFailed("doPreHandlingFailed()");
                m_preHandlingPromise.set_value();
            }
            bool MockDirectiveHandler::doHandlingFailed(const string& messageId) {
                if (!m_directive || m_directive->getMessageId() != messageId) return false;
                shutdown();
                m_result->setFailed("doHandlingFailed()");
                m_handlingPromise.set_value();
                return true;
            }
            void MockDirectiveHandler::shutdown() {
                {
                    lock_guard<mutex> lock(m_mutex);
                    m_isShuttingDown = true;
                    m_wakeNotifier.notify_all();
                }
                if (m_doHandleDirectiveThread.joinable()) m_doHandleDirectiveThread.join();
            }
            bool MockDirectiveHandler::waitUntilPreHandling(milliseconds timeout) {
                return m_preHandlingFuture.wait_for(timeout) == future_status::ready;
            }
            bool MockDirectiveHandler::waitUntilHandling(milliseconds timeout) {
                return m_handlingFuture.wait_for(timeout) == future_status::ready;
            }
            bool MockDirectiveHandler::waitUntilCanceling(milliseconds timeout) {
                return m_cancelingFuture.wait_for(timeout) == future_status::ready;
            }
            bool MockDirectiveHandler::waitUntilCompleted(milliseconds timeout) {
                return m_completedFuture.wait_for(timeout) == future_status::ready;
            }
        }
    }
}