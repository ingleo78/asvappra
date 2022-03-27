#include <iostream>
#include <avs/CapabilityConfiguration.h>
#include <avs/EventBuilder.h>
#include <logger/Logger.h>
#include "PlaybackController.h"
#include "PlaybackMessageRequest.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            static const string PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_NAME = "PlaybackController";
            static const string PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.1";
            static const string TAG("PlaybackController");
            #define LX(event) LogEntry(TAG, event)
            static const string PLAYBACK_CONTROLLER_NAMESPACE = "PlaybackController";
            static shared_ptr<CapabilityConfiguration> getPlaybackControllerCapabilityConfiguration();
            shared_ptr<PlaybackController> PlaybackController::create(shared_ptr<ContextManagerInterface> contextManager,
                                                                      shared_ptr<MessageSenderInterface> messageSender) {
                if (!contextManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                    return nullptr;
                }
                if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                }
                return shared_ptr<PlaybackController>(new PlaybackController(contextManager, messageSender));
            }
            void PlaybackController::doShutdown() {
                m_executor.shutdown();
                m_messageSender.reset();
                m_contextManager.reset();
            }
            void PlaybackController::handleCommand(const PlaybackCommand& command) {
                auto task = [this, &command]() {
                    ACSDK_DEBUG9(LX("buttonPressedExecutor").d("Button", command));
                    if (m_commands.empty()) {
                        ACSDK_DEBUG9(LX("buttonPressedExecutor").m("Queue is empty, call getContext()."));
                        m_contextManager->getContext(shared_from_this());
                    }
                    m_commands.push(&command);
                };
                ACSDK_DEBUG9(LX("buttonPressed").d("Button", command));
                m_executor.submit(task);
            }
            void PlaybackController::onButtonPressed(PlaybackButton button) {
                auto& command = PlaybackCommand::buttonToCommand(button);
                handleCommand(command);
            }
            void PlaybackController::onTogglePressed(PlaybackToggle toggle, bool action) {
                auto& command = PlaybackCommand::toggleToCommand(toggle, action);
                handleCommand(command);
            }
            void PlaybackController::messageSent(const PlaybackCommand& command, MessageRequestObserverInterface::Status messageStatus) {
                if (MessageRequestObserverInterface::Status::SUCCESS == messageStatus) {
                    ACSDK_DEBUG(LX("messageSentSucceeded").d("ButtonPressed", command));
                } else {
                    ACSDK_ERROR(LX("messageSentFailed").d("ButtonPressed", command).d("error", messageStatus));
                }
            }
            void PlaybackController::onContextAvailable(const string& jsonContext) {
                auto task = [this, jsonContext]() {
                    ACSDK_DEBUG9(LX("onContextAvailableExecutor"));
                    if (m_commands.empty()) {
                        ACSDK_WARN(LX("onContextAvailableExecutor").m("Queue is empty, return."));
                        return;
                    }
                    auto& command = *m_commands.front();
                    m_commands.pop();
                    auto msgIdAndJsonEvent = buildJsonEventString(PLAYBACK_CONTROLLER_NAMESPACE, command.getEventName(), "",
                                                                  command.getEventPayload(), jsonContext);
                    m_messageSender->sendMessage(make_shared<PlaybackMessageRequest>(command, msgIdAndJsonEvent.second, shared_from_this()));
                    if (!m_commands.empty()) {
                        ACSDK_DEBUG9(LX("onContextAvailableExecutor").m("Queue is not empty, call getContext()."));
                        m_contextManager->getContext(shared_from_this());
                    }
                };
                ACSDK_DEBUG9(LX("onContextAvailable"));
                m_executor.submit(task);
            }
            void PlaybackController::onContextFailure(const ContextRequestError error) {
                auto task = [this, error]() {
                    if (m_commands.empty()) {
                        ACSDK_WARN(LX("onContextFailureExecutor").m("Queue is empty, return."));
                        return;
                    }
                    auto& command = m_commands.front();
                    m_commands.pop();
                    ACSDK_ERROR(LX("contextRetrievalFailed").d("ButtonPressed", command).d("error", error));
                    if (!m_commands.empty()) m_contextManager->getContext(shared_from_this());
                };
                ACSDK_DEBUG9(LX("onContextFailure"));
                m_executor.submit(task);
            }
            PlaybackController::PlaybackController(shared_ptr<ContextManagerInterface> contextManager,
                                                   shared_ptr<MessageSenderInterface> messageSender) : RequiresShutdown{"PlaybackController"},
                                                   m_messageSender{messageSender}, m_contextManager{contextManager} {
                m_capabilityConfigurations.insert(getPlaybackControllerCapabilityConfiguration());
            }
            shared_ptr<CapabilityConfiguration> getPlaybackControllerCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_VERSION});
                return make_shared<CapabilityConfiguration>(configMap);
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> PlaybackController::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
        }
    }
}