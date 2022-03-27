#ifndef ACSDKNOTIFICATIONS_NOTIFICATIONSCAPABILITYAGENT_H_
#define ACSDKNOTIFICATIONS_NOTIFICATIONSCAPABILITYAGENT_H_

#include <memory>
#include <unordered_set>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/Audio/NotificationsAudioFactoryInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <registration_manager/CustomerDataHandler.h>
#include <acsdk_alerts_interfaces/NotificationsObserverInterface.h>
#include "NotificationIndicator.h"
#include "NotificationsCapabilityAgentState.h"
#include "NotificationRendererInterface.h"
#include "NotificationRendererObserverInterface.h"
#include "NotificationsStorageInterface.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace audio;
        using namespace utils;
        using namespace configuration;
        using namespace logger;
        using namespace json;
        using namespace metrics;
        using namespace threading;
        using namespace registrationManager;
        using namespace acsdkNotificationsInterfaces;
        using namespace rapidjson;
        class NotificationsCapabilityAgent : public NotificationRendererObserverInterface , public CapabilityAgent, public CapabilityConfigurationInterface,
                                             public RequiresShutdown, public CustomerDataHandler, public enable_shared_from_this<NotificationsCapabilityAgent> {
        public:
            static shared_ptr<NotificationsCapabilityAgent> create(shared_ptr<NotificationsStorageInterface> notificationsStorage,
                                                                   shared_ptr<NotificationRendererInterface> renderer,
                                                                   shared_ptr<ContextManagerInterface> contextManager,
                                                                   shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                   shared_ptr<NotificationsAudioFactoryInterface> notificationsAudioFactory,
                                                                   shared_ptr<CustomerDataManager> dataManager,
                                                                   shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
            void addObserver(shared_ptr<NotificationsObserverInterface> observer);
            void removeObserver(shared_ptr<NotificationsObserverInterface> observer);
            void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
            void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
            void handleDirective(shared_ptr<DirectiveInfo> info) override;
            void cancelDirective(shared_ptr<DirectiveInfo> info) override;
            DirectiveHandlerConfiguration getConfiguration() const override;
            void provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) override;
            void onNotificationRenderingFinished() override;
            void clearData() override;
            unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
        private:
            NotificationsCapabilityAgent(shared_ptr<NotificationsStorageInterface> notificationsStorage, shared_ptr<NotificationRendererInterface> renderer,
                                         shared_ptr<ContextManagerInterface> contextManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                         shared_ptr<NotificationsAudioFactoryInterface> notificationsAudioFactory, shared_ptr<CustomerDataManager> dataManager,
                                         shared_ptr<MetricRecorderInterface> metricRecorder);
            bool init();
            bool parseDirectivePayload(shared_ptr<DirectiveInfo> info, rapidjson::Document* document);
            void handleSetIndicatorDirective(shared_ptr<DirectiveInfo> info);
            void handleClearIndicatorDirective(shared_ptr<DirectiveInfo> info);
            void doShutdown() override;
            void setHandlingCompleted(shared_ptr<DirectiveInfo> info);
            void executeRenderNotification(const NotificationIndicator& notificationIndicator);
            void executePossibleIndicatorStateChange(const IndicatorState& nextIndicatorState);
            void executeSetState(NotificationsCapabilityAgentState newState);
            void executeProvideState(bool sendToken = false, unsigned int stateRequestToken = 0);
            void notifyObserversOfIndicatorState(IndicatorState state);
            void notifyObserversOfNotificationReceived();
            void executeInit();
            void executeOnPlayFinished();
            void executeStartQueueNotEmpty();
            void executeSetIndicator(const NotificationIndicator& nextNotificationIndicator, shared_ptr<DirectiveInfo> info);
            void executeClearIndicator(shared_ptr<DirectiveInfo> info);
            void executePlayFinishedZeroQueued();
            void executePlayFinishedOneQueued();
            void executePlayFinishedMultipleQueued();
            void executeShutdown();
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            shared_ptr<NotificationsStorageInterface> m_notificationsStorage;
            shared_ptr<ContextManagerInterface> m_contextManager;
            shared_ptr<NotificationRendererInterface> m_renderer;
            shared_ptr<NotificationsAudioFactoryInterface> m_notificationsAudioFactory;
            string m_currentAssetId;
            bool m_isEnabled;
            unordered_set<shared_ptr<NotificationsObserverInterface>> m_observers;
            NotificationsCapabilityAgentState m_currentState;
            unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
            Executor m_executor;
            condition_variable m_shutdownTrigger;
            mutex m_shutdownMutex;
        };
    }
}
#endif