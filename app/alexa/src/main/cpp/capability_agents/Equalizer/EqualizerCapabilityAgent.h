#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_EQUALIZER_INCLUDE_EQUALIZER_EQUALIZERCAPABILITYAGENT_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_EQUALIZER_INCLUDE_EQUALIZER_EQUALIZERCAPABILITYAGENT_H_

#include <memory>
#include <mutex>
#include <json/document.h>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/ExceptionErrorType.h>
#include <sdkinterfaces/Audio/EqualizerModeControllerInterface.h>
#include <sdkinterfaces/CapabilitiesDelegateInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ContextRequesterInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <registration_manager/CustomerDataHandler.h>
#include <registration_manager/CustomerDataManager.h>
#include <sdkinterfaces/Audio/EqualizerControllerListenerInterface.h>
#include <sdkinterfaces/Audio/EqualizerStorageInterface.h>
#include <equalizer_implementations/EqualizerController.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace equalizer {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace rapidjson;
            using namespace threading;
            using namespace registrationManager;
            using namespace alexaClientSDK::equalizer;
            using namespace sdkInterfaces::audio;
            class EqualizerCapabilityAgent : public enable_shared_from_this<EqualizerCapabilityAgent>, public EqualizerControllerListenerInterface,
                                             public CapabilityAgent, public CapabilityConfigurationInterface, public RequiresShutdown,
                                             public CustomerDataHandler {
            public:
                ~EqualizerCapabilityAgent() override = default;
                static shared_ptr<EqualizerCapabilityAgent> create(shared_ptr<EqualizerController> equalizerController,
                                                                   shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                                                   shared_ptr<EqualizerStorageInterface> equalizerStorage,
                                                                   shared_ptr<CustomerDataManager> customerDataManager,
                                                                   shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                   shared_ptr<ContextManagerInterface> contextManager,
                                                                   shared_ptr<MessageSenderInterface> messageSender);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
                void doShutdown() override;
                void clearData() override;
                void onEqualizerStateChanged(const EqualizerState& state) override;
                void onEqualizerSameStateChanged(const EqualizerState& state) override;
            private:
                EqualizerCapabilityAgent(shared_ptr<EqualizerController> equalizerController, shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                         shared_ptr<EqualizerStorageInterface> equalizerStorage, shared_ptr<CustomerDataManager> customerDataManager,
                                         shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                         shared_ptr<ContextManagerInterface> contextManager, shared_ptr<MessageSenderInterface> messageSender);
                void generateCapabilityConfiguration();
                void fixConfigurationDesynchronization();
                bool handleSetBandsDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& document);
                bool handleAdjustBandsDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& document);
                bool handleResetBandsDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& document);
                bool handleSetModeDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& document);
                shared_ptr<EqualizerController> m_equalizerController;
                shared_ptr<CapabilitiesDelegateInterface> m_capabilitiesDelegate;
                shared_ptr<EqualizerStorageInterface> m_equalizerStorage;
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<ContextManagerInterface> m_contextManager;
                Executor m_executor;
                mutex m_storageMutex;
            };
        }
    }
}
#endif