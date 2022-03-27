#include <utility>
#include <string>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <json/en.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <logger/Logger.h>
#include <util/string/StringUtils.h>
#include "AudioActivityTracker.h"
#include "ActivityTrackerInterface.h"

namespace alexaClientSDK {
    namespace afml {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace rapidjson;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace avs;
        using namespace logger;
        static const std::string AUDIOACTIVITYTRACKER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
        static const std::string AUDIOACTIVITYTRACKER_CAPABILITY_INTERFACE_NAME = "AudioActivityTracker";
        static const std::string AUDIOACTIVITYTRACKER_CAPABILITY_INTERFACE_VERSION = "1.0";
        static const std::string TAG("AudioActivityTracker");
        #define LX(event) LogEntry(TAG, event)
        static const NamespaceAndName CONTEXT_MANAGER_STATE{"AudioActivityTracker", "ActivityState"};
        static const char IDLE_TIME_KEY[] = "idleTimeInMilliseconds";
        static const char INTERFACE_KEY[] = "interface";
        static shared_ptr<CapabilityConfiguration> getAudioActivityTrackerCapabilityConfiguration();
        shared_ptr<AudioActivityTracker> AudioActivityTracker::create(shared_ptr<ContextManagerInterface> contextManager) {
            if (!contextManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                return nullptr;
            }
            auto audioActivityTracker = shared_ptr<AudioActivityTracker>(new AudioActivityTracker(contextManager));
            contextManager->setStateProvider(CONTEXT_MANAGER_STATE, audioActivityTracker);
            return audioActivityTracker;
        }
        void AudioActivityTracker::provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) {
            ACSDK_DEBUG5(LX("provideState"));
            m_executor.submit([this, stateRequestToken]() { executeProvideState(stateRequestToken); });
        }
        void AudioActivityTracker::notifyOfActivityUpdates(const vector<Channel::State>& channelStates) {
            ACSDK_DEBUG5(LX("notifyOfActivityUpdates"));
            m_executor.submit([this, channelStates]() { executeNotifyOfActivityUpdates(channelStates); });
        }
        AudioActivityTracker::AudioActivityTracker(shared_ptr<ContextManagerInterface> contextManager) : RequiresShutdown{"AudioActivityTracker"},
                                                                                                         m_contextManager{contextManager} {
            m_capabilityConfigurations.insert(getAudioActivityTrackerCapabilityConfiguration());
        }
        shared_ptr<CapabilityConfiguration> getAudioActivityTrackerCapabilityConfiguration() {
            unordered_map<std::string, std::string> configMap;
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, AUDIOACTIVITYTRACKER_CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, AUDIOACTIVITYTRACKER_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, AUDIOACTIVITYTRACKER_CAPABILITY_INTERFACE_VERSION});
            return make_shared<CapabilityConfiguration>(configMap);
        }
        void AudioActivityTracker::doShutdown() {
            m_contextManager->removeStateProvider(CONTEXT_MANAGER_STATE);
            m_executor.shutdown();
            m_contextManager.reset();
        }
        void AudioActivityTracker::executeNotifyOfActivityUpdates(const vector<Channel::State>& channelStates) {
            ACSDK_DEBUG5(LX("executeNotifyOfActivityUpdates"));
            for (const auto& state : channelStates) {
                if ("SpeechRecognizer" == state.interfaceName && FocusManagerInterface::DIALOG_CHANNEL_NAME == state.name) continue;
                m_channelStates[state.name] = state;
            }
        }
        void AudioActivityTracker::executeProvideState(unsigned int stateRequestToken) {
            ACSDK_DEBUG5(LX("executeProvideState"));
            Document payload(rapidjson::kObjectType);
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            auto currentTime = steady_clock::now();
            auto sendEmptyContext = true;
            if (!m_channelStates.empty()) {
                for (auto it = m_channelStates.begin(); it != m_channelStates.end(); ++it) {
                    Value contextJson(kObjectType);
                    auto idleTime = milliseconds::zero();
                    auto& channelContext = it->second;
                    if (FocusState::NONE == channelContext.focusState) idleTime = duration_cast<milliseconds>(currentTime - channelContext.timeAtIdle);
                    std::string interfaceKey = INTERFACE_KEY;
                    std::string idleTimeKey = IDLE_TIME_KEY;
                    std::string interfaceNameData = channelContext.interfaceName.data();
                    std::string data = executeChannelNameInLowerCase(it->first).data();
                    contextJson.AddMember(interfaceKey, interfaceNameData,reinterpret_cast<basic_string<char, char_traits<char>, allocator<char>> &>(payload.GetAllocator()));
                    contextJson.AddMember(idleTimeKey, static_cast<uint64_t>(idleTime.count()),reinterpret_cast<basic_string<char, char_traits<char>, allocator<char>> &>(payload.GetAllocator()));
                    payload.AddMember(data.data(), contextJson, payload.GetAllocator());
                }
                if (!payload.Accept(writer)) { ACSDK_ERROR(LX("executeProvideState").d("reason", "writerRefusedJsonObject")); }
                else sendEmptyContext = false;
            }
            if (sendEmptyContext) m_contextManager->setState(CONTEXT_MANAGER_STATE, "", avsCommon::avs::StateRefreshPolicy::SOMETIMES, stateRequestToken);
            else m_contextManager->setState(CONTEXT_MANAGER_STATE, buffer.GetString(),avsCommon::avs::StateRefreshPolicy::SOMETIMES, stateRequestToken);
        }
        const std::string& AudioActivityTracker::executeChannelNameInLowerCase(const std::string& channelName) {
            auto it = m_channelNamesInLowerCase.find(channelName);
            if (it == m_channelNamesInLowerCase.end()) {
                auto channelNameInLowerCase = avsCommon::utils::string::stringToLowerCase(channelName);
                it = m_channelNamesInLowerCase.insert(it, std::make_pair(channelName, channelNameInLowerCase));
            }
            return it->second;
        }
        std::unordered_set<std::shared_ptr<avsCommon::avs::CapabilityConfiguration>> AudioActivityTracker::
            getCapabilityConfigurations() {
            return m_capabilityConfigurations;
        }
    }
}