#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_REPORTSTATEHANDLER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_REPORTSTATEHANDLER_H_

#include <memory>
#include <string>
#include <thread>
#include <avs/CapabilityAgent.h>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/Storage/MiscStorageInterface.h>
#include <threading/Executor.h>
#include <registration_manager/CustomerDataHandler.h>
#include <settings/SettingEventSenderInterface.h>
#include <settings/SettingConnectionObserver.h>
#include <capability_agents/System/StateReportGenerator.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace json;
            using namespace registrationManager;
            class ReportStateHandler : public CapabilityAgent, public registrationManager::CustomerDataHandler {
            public:
                static unique_ptr<ReportStateHandler> create(shared_ptr<registrationManager::CustomerDataManager> dataManager,
                                                             shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                             shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                                             shared_ptr<MessageSenderInterface> messageSender,
                                                             shared_ptr<storage::MiscStorageInterface> settingStorage,
                                                             const vector<StateReportGenerator>& generators);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void clearData() override;
                ~ReportStateHandler();
                void addStateReportGenerator(const StateReportGenerator& generator);
            private:
                ReportStateHandler(shared_ptr<registrationManager::CustomerDataManager> dataManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                   shared_ptr<AVSConnectionManagerInterface> connectionManager, shared_ptr<storage::MiscStorageInterface> storage,
                                   unique_ptr<SettingEventSenderInterface> eventSender, const vector<StateReportGenerator>& generators, bool pendingReport);
                bool handleReportState(const AVSDirective& directive);
                void sendReportState();
                void initialize();
            private:
                mutex m_stateMutex;
                threading::Executor m_executor;
                shared_ptr<AVSConnectionManagerInterface> m_connectionManager;
                shared_ptr<storage::MiscStorageInterface> m_storage;
                vector<StateReportGenerator> m_generators;
                unique_ptr<settings::SettingEventSenderInterface> m_eventSender;
                shared_ptr<settings::SettingConnectionObserver> m_connectionObserver;
                bool m_pendingReport;
            };
        }
    }
}
#endif