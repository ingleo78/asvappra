#ifndef ACSDKMULTIROOMMUSIC_MRMCAPABILITYAGENT_H_
#define ACSDKMULTIROOMMUSIC_MRMCAPABILITYAGENT_H_

#include <memory>
#include <string>
#include <avs/CapabilityAgent.h>
#include <sdkinterfaces/CallStateObserverInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <sdkinterfaces/SpeakerManagerInterface.h>
#include <sdkinterfaces/SpeakerManagerObserverInterface.h>
#include <sdkinterfaces/UserInactivityMonitorInterface.h>
#include <sdkinterfaces/UserInactivityMonitorObserverInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include "MRMHandlerInterface.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace mrm {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace configuration;
            using namespace json;
            using namespace threading;
            using namespace rapidjson;
            class MRMCapabilityAgent : public CapabilityAgent, public SpeakerManagerObserverInterface, public UserInactivityMonitorObserverInterface,
                                       public CallStateObserverInterface, public CapabilityConfigurationInterface, public RenderPlayerInfoCardsProviderInterface,
                                       public RequiresShutdown, public enable_shared_from_this<MRMCapabilityAgent> {
            public:
                static shared_ptr<MRMCapabilityAgent> create(shared_ptr<MRMHandlerInterface> mrmHandler, shared_ptr<SpeakerManagerInterface> speakerManager,
                                                             shared_ptr<UserInactivityMonitorInterface> userInactivityMonitor,
                                                             shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                ~MRMCapabilityAgent() override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                void onSpeakerSettingsChanged(const SpeakerManagerObserverInterface::Source& source, const ChannelVolumeInterface::Type& type,
                                              const SpeakerInterface::SpeakerSettings& settings) override;
                void onUserInactivityReportSent() override;
                void onCallStateChange(CallStateObserverInterface::CallState callState) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
                void setObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer) override;
                void doShutdown() override;
                string getVersionString() const;
            private:
                MRMCapabilityAgent(shared_ptr<MRMHandlerInterface> handler, shared_ptr<SpeakerManagerInterface> speakerManager,
                                   shared_ptr<UserInactivityMonitorInterface> userInactivityMonitor,
                                   shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                void executeHandleDirectiveImmediately(shared_ptr<DirectiveInfo> info);
                void executeOnSpeakerSettingsChanged(const ChannelVolumeInterface::Type& type);
                void executeOnCallStateChange(const CallStateObserverInterface::CallState state);
                void executeOnUserInactivityReportSent();
                void executeSetObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer);
                shared_ptr<MRMHandlerInterface> m_mrmHandler;
                shared_ptr<SpeakerManagerInterface> m_speakerManager;
                shared_ptr<UserInactivityMonitorInterface> m_userInactivityMonitor;
                bool m_wasPreviouslyActive;
                Executor m_executor;
            };
        }
    }
}
#endif