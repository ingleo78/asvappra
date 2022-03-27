#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIME_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIME_H_

#include <chrono>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/RenderPlayerInfoCardsObserverInterface.h>
#include <sdkinterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <sdkinterfaces/TemplateRuntimeObserverInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace templateRuntime {
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace threading;
            using namespace timing;
            using DialogUXState = DialogUXStateObserverInterface::DialogUXState;
            using AudioPlayerInfo = TemplateRuntimeObserverInterface::AudioPlayerInfo;
            class TemplateRuntime : public CapabilityAgent, public RequiresShutdown, public RenderPlayerInfoCardsObserverInterface,
                                    public CapabilityConfigurationInterface, public DialogUXStateObserverInterface,
                                    public enable_shared_from_this<TemplateRuntime> {
            public:
                using DirectiveInfo = CapabilityAgent::DirectiveInfo;
                static shared_ptr<TemplateRuntime> create(const unordered_set<shared_ptr<RenderPlayerInfoCardsProviderInterface>>& renderPlayerInfoCardsInterfaces,
                                                          shared_ptr<FocusManagerInterface> focusManager,
                                                          shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender);
                virtual ~TemplateRuntime() = default;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                void onFocusChanged(FocusState newFocus, MixingBehavior behavior) override;
                void onRenderPlayerCardsInfoChanged(PlayerActivity state, const Context& context) override;
                void onDialogUXStateChanged(DialogUXState newState) override;
                void addObserver(shared_ptr<TemplateRuntimeObserverInterface> observer);
                void removeObserver(shared_ptr<TemplateRuntimeObserverInterface> observer);
                void displayCardCleared();
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
                void addRenderPlayerInfoCardsProvider(shared_ptr<RenderPlayerInfoCardsProviderInterface> cardsProvider);
            private:
                enum class State {
                    IDLE,
                    ACQUIRING,
                    DISPLAYING,
                    RELEASING,
                    REACQUIRING
                };
                struct AudioItemPair {
                    AudioItemPair() = default;
                    AudioItemPair(string itemId, shared_ptr<DirectiveInfo> renderPlayerInfoDirective) : audioItemId{itemId},
                                                                                                        directive{renderPlayerInfoDirective} {};
                    string audioItemId;
                    shared_ptr<DirectiveInfo> directive;
                };
                TemplateRuntime(const unordered_set<shared_ptr<RenderPlayerInfoCardsProviderInterface>>& renderPlayerInfoCardsInterfaces,
                                shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender);
                void doShutdown() override;
                bool initialize();
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void setHandlingCompleted(shared_ptr<DirectiveInfo> info);
                void handleRenderTemplateDirective(shared_ptr<DirectiveInfo> info);
                void handleRenderPlayerInfoDirective(shared_ptr<DirectiveInfo> info);
                void handleUnknownDirective(shared_ptr<DirectiveInfo> info);
                void executeAudioPlayerInfoUpdates(PlayerActivity state, const Context& context);
                void executeAudioPlayerStartTimer(PlayerActivity state);
                void executeRenderPlayerInfoCallbacks(bool isClearCard);
                void executeRenderTemplateCallbacks(bool isClearCard);
                void executeDisplayCard();
                void executeClearCard();
                void executeStartTimer(milliseconds timeout);
                void executeStopTimer();
                string stateToString(const State state);
                void executeTimerEvent();
                void executeOnFocusChangedEvent(FocusState newFocus);
                void executeDisplayCardEvent(const shared_ptr<DirectiveInfo> info);
                void executeCardClearedEvent();
                Timer m_clearDisplayTimer;
                unordered_set<shared_ptr<TemplateRuntimeObserverInterface>> m_observers;
                unordered_map<shared_ptr<MediaPropertiesInterface>, AudioItemPair> m_audioItemsInExecution;
                shared_ptr<MediaPropertiesInterface> m_activeRenderPlayerInfoCardsProvider;
                deque<AudioItemPair> m_audioItems;
                unordered_map<shared_ptr<MediaPropertiesInterface>, AudioPlayerInfo> m_audioPlayerInfo;
                shared_ptr<DirectiveInfo> m_lastDisplayedDirective;
                bool m_isRenderTemplateLastReceived;
                FocusState m_focus;
                State m_state;
                unordered_set<shared_ptr<RenderPlayerInfoCardsProviderInterface>> m_renderPlayerInfoCardsInterfaces;
                shared_ptr<FocusManagerInterface> m_focusManager;
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                Executor m_executor;
                milliseconds m_ttsFinishedTimeout;
                milliseconds m_audioPlaybackFinishedTimeout;
                milliseconds m_audioPlaybackStoppedPausedTimeout;
            };
        }
    }
}
#endif