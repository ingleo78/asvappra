#include <json/document.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "MRMCapabilityAgent.h"

static const std::string TAG = "MRMCapabilityAgent";
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace mrm {
            static const string CAPABILITY_AGENT_NAMESPACE_STR = "MRM";
            static const string DIRECTIVE_NAMESPACE_STR = "WholeHomeAudio";
            static const string SKEW_DIRECTIVE_NAMESPACE_STR = "WholeHomeAudio.Skew";
            static const NamespaceAndName WHA_NAMESPACE_WILDCARD{DIRECTIVE_NAMESPACE_STR, "*"};
            static const NamespaceAndName WHA_SKEW_NAMESPACE_WILDCARD{SKEW_DIRECTIVE_NAMESPACE_STR, "*"};
            static const string MRM_CONFIGURATION_ROOT_KEY = "mrm";
            static const string MRM_CAPABILITIES_KEY = "capabilities";
            static unordered_set<shared_ptr<CapabilityConfiguration>> readCapabilities() {
                unordered_set<shared_ptr<CapabilityConfiguration>> capabilitiesSet;
                auto configRoot = ConfigurationNode::getRoot();
                if (!configRoot) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "configurationRootNotFound"));
                    return capabilitiesSet;
                }
                auto mrmConfig = configRoot[MRM_CONFIGURATION_ROOT_KEY];
                if (!mrmConfig) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "configurationKeyNotFound").d("configurationKey", MRM_CONFIGURATION_ROOT_KEY));
                    return capabilitiesSet;
                }
                auto capabilitiesConfig = mrmConfig[MRM_CAPABILITIES_KEY];
                if (!capabilitiesConfig) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "capabilitiesKeyNotFound").d("key", MRM_CAPABILITIES_KEY));
                    return capabilitiesSet;
                }
                string capabilitiesString = capabilitiesConfig.serialize();
                Document capabilities;
                if (!jsonUtils::parseJSON(capabilitiesString, &capabilities)) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "failedToParseCapabilitiesString").d("capabilitiesString", capabilitiesString));
                    return capabilitiesSet;
                }
                for (auto itr = capabilities.MemberBegin(); itr != capabilities.MemberEnd(); ++itr) {
                    string interfaceType;
                    Value value{itr->value.GetString(), strlen(itr->value.GetString())};
                    if (!jsonUtils::retrieveValue1(value, CAPABILITY_INTERFACE_TYPE_KEY.data(), &interfaceType)) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "failedToFindCapabilityInterfaceTypeKey").d("key", CAPABILITY_INTERFACE_TYPE_KEY));
                        return capabilitiesSet;
                    }
                    string interfaceName;
                    if (!jsonUtils::retrieveValue1(value, CAPABILITY_INTERFACE_NAME_KEY.data(), &interfaceName)) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "failedToFindCapabilityInterfaceNameKey").d("key", CAPABILITY_INTERFACE_NAME_KEY));
                        return capabilitiesSet;
                    }
                    string interfaceVersion;
                    if (!jsonUtils::retrieveValue1(value, CAPABILITY_INTERFACE_VERSION_KEY.data(), &interfaceVersion)) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "failedToFindCapabilityInterfaceVersionKey").d("key", CAPABILITY_INTERFACE_VERSION_KEY));
                        return capabilitiesSet;
                    }
                    string configurationsString;
                    Value::ConstMemberIterator configurations;
                    if (jsonUtils::findNode(value, CAPABILITY_INTERFACE_CONFIGURATIONS_KEY.data(), &configurations)) {
                        if (!jsonUtils::convertToValue(configurations->value, &configurationsString)) {
                            ACSDK_ERROR(LX("initializeFailed").d("reason", "failedToConvertConfigurations"));
                            return capabilitiesSet;
                        }
                    }
                    unordered_map<string, string> capabilityMap = {{CAPABILITY_INTERFACE_TYPE_KEY, interfaceType}, {CAPABILITY_INTERFACE_NAME_KEY, interfaceName},
                                                                   {CAPABILITY_INTERFACE_VERSION_KEY, interfaceVersion}};
                    if (!configurationsString.empty()) capabilityMap[CAPABILITY_INTERFACE_CONFIGURATIONS_KEY] = configurationsString;
                    capabilitiesSet.insert(std::make_shared<CapabilityConfiguration>(capabilityMap));
                }
                if (capabilitiesSet.empty()) { ACSDK_ERROR(LX("initializeFailed").d("reason", "missingCapabilityConfigurations")); }
                return capabilitiesSet;
            }
            shared_ptr<MRMCapabilityAgent> MRMCapabilityAgent::create(shared_ptr<MRMHandlerInterface> mrmHandler, shared_ptr<SpeakerManagerInterface> speakerManager,
                                                                      shared_ptr<UserInactivityMonitorInterface> userInactivityMonitor,
                                                                      shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) {
                ACSDK_DEBUG5(LX(__func__));
                if (!mrmHandler) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "mrmHandler was nullptr."));
                    return nullptr;
                }
                if (!speakerManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "speakerManager was nullptr."));
                    return nullptr;
                }
                if (!userInactivityMonitor) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "userInactivityMonitor was nullptr."));
                    return nullptr;
                }
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "exceptionEncounteredSender was nullptr."));
                    return nullptr;
                }
                auto agent = shared_ptr<MRMCapabilityAgent>(new MRMCapabilityAgent(mrmHandler, speakerManager, userInactivityMonitor, exceptionEncounteredSender));
                userInactivityMonitor->addObserver(agent);
                speakerManager->addSpeakerManagerObserver(agent);
                return agent;
            }
            MRMCapabilityAgent::MRMCapabilityAgent(shared_ptr<MRMHandlerInterface> handler, shared_ptr<SpeakerManagerInterface> speakerManager,
                                                   shared_ptr<UserInactivityMonitorInterface> userInactivityMonitor,
                                                   shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                                   CapabilityAgent(CAPABILITY_AGENT_NAMESPACE_STR, exceptionEncounteredSender), RequiresShutdown("MRMCapabilityAgent"),
                                                   m_mrmHandler{handler}, m_speakerManager{speakerManager}, m_userInactivityMonitor{userInactivityMonitor},
                                                   m_wasPreviouslyActive{false} {
                ACSDK_DEBUG5(LX(__func__));
            };
            MRMCapabilityAgent::~MRMCapabilityAgent() {
                ACSDK_DEBUG5(LX(__func__));
            }
            void MRMCapabilityAgent::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
            }
            void MRMCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "info is nullptr."));
                    return;
                }
                m_executor.submit([this, info]() { executeHandleDirectiveImmediately(info); });
            }
            void MRMCapabilityAgent::cancelDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
            }
            void MRMCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "directive is nullptr."));
                    return;
                }
                auto info = make_shared<DirectiveInfo>(directive, nullptr);
                m_executor.submit([this, info]() { executeHandleDirectiveImmediately(info); });
            }
            DirectiveHandlerConfiguration MRMCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                configuration[WHA_NAMESPACE_WILDCARD] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                configuration[WHA_SKEW_NAMESPACE_WILDCARD] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                return configuration;
            }
            void MRMCapabilityAgent::onUserInactivityReportSent() {
                ACSDK_DEBUG5(LX(__func__));
                m_executor.submit([this]() { executeOnUserInactivityReportSent(); });
            }
            void MRMCapabilityAgent::onSpeakerSettingsChanged(const SpeakerManagerObserverInterface::Source& source, const ChannelVolumeInterface::Type& type,
                                                              const SpeakerInterface::SpeakerSettings& settings) {
                ACSDK_DEBUG5(LX(__func__).d("type", type));
                m_executor.submit([this, type]() { executeOnSpeakerSettingsChanged(type); });
            }
            void MRMCapabilityAgent::onCallStateChange(CallStateObserverInterface::CallState callState) {
                ACSDK_DEBUG5(LX(__func__).d("callState", callState));
                m_executor.submit([this, callState]() { executeOnCallStateChange(callState); });
            }
            string MRMCapabilityAgent::getVersionString() const {
                ACSDK_DEBUG5(LX(__func__));
                return m_mrmHandler->getVersionString();
            }
            void MRMCapabilityAgent::setObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("setObserverFailed").m("Observer is null."));
                    return;
                }
                ACSDK_DEBUG5(LX(__func__));
                m_executor.submit([this, observer]() { executeSetObserver(observer); });
            }
            void MRMCapabilityAgent::executeHandleDirectiveImmediately(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (m_mrmHandler->handleDirective(info->directive->getNamespace(), info->directive->getName(), info->directive->getMessageId(),
                    info->directive->getPayload())) {
                    if (info->result) info->result->setCompleted();
                } else {
                    string errorMessage = "MultiRoomMusic Handler was unable to handle Directive - " + info->directive->getNamespace() + ":" + info->directive->getName();
                    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), ExceptionErrorType::INTERNAL_ERROR, errorMessage);
                    ACSDK_ERROR(LX("executeHandleDirectiveImmediatelyFailed").d("reason", errorMessage));
                    if (info->result) info->result->setFailed(errorMessage);
                }
                removeDirective(info->directive->getMessageId());
            }
            void MRMCapabilityAgent::executeOnSpeakerSettingsChanged(const ChannelVolumeInterface::Type& type) {
                ACSDK_DEBUG5(LX(__func__));
                m_mrmHandler->onSpeakerSettingsChanged(type);
            }
            void MRMCapabilityAgent::executeOnUserInactivityReportSent() {
                ACSDK_DEBUG5(LX(__func__));
                m_mrmHandler->onUserInactivityReportSent();
            }
            void MRMCapabilityAgent::executeOnCallStateChange(const CallStateObserverInterface::CallState callState) {
                ACSDK_DEBUG5(LX(__func__));
                bool isCurrentlyActive = CallStateObserverInterface::isStateActive(callState);
                if (m_wasPreviouslyActive != isCurrentlyActive) {
                    m_mrmHandler->onCallStateChange(isCurrentlyActive);
                    m_wasPreviouslyActive = isCurrentlyActive;
                } else { ACSDK_WARN(LX(__func__).m("call active state didn't actually change")); }
            }
            void MRMCapabilityAgent::executeSetObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer) {
                ACSDK_DEBUG5(LX(__func__));
                m_mrmHandler->setObserver(observer);
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> MRMCapabilityAgent::getCapabilityConfigurations() {
                return readCapabilities();
            }
            void MRMCapabilityAgent::doShutdown() {
                ACSDK_DEBUG5(LX(__func__));
                m_speakerManager->removeSpeakerManagerObserver(shared_from_this());
                m_speakerManager.reset();
                m_userInactivityMonitor->removeObserver(shared_from_this());
                m_userInactivityMonitor.reset();
                m_mrmHandler->shutdown();
            }
        }
    }
}