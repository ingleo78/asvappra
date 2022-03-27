#include <json/stringbuffer.h>
#include <json/writer.h>
#include <json/en.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <logger/Logger.h>
#include "VisualActivityTracker.h"

namespace alexaClientSDK {
    namespace afml {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace avs;
        using namespace logger;
        using namespace rapidjson;
        static const std::string VISUALACTIVITYTRACKER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
        static const std::string VISUALACTIVITYTRACKER_CAPABILITY_INTERFACE_NAME = "VisualActivityTracker";
        static const std::string VISUALACTIVITYTRACKER_CAPABILITY_INTERFACE_VERSION = "1.0";
        static const std::string TAG("VisualActivityTracker");
        #define LX(event) LogEntry(TAG, event)
        static const NamespaceAndName CONTEXT_MANAGER_STATE{"VisualActivityTracker", "ActivityState"};
        static const char FOCUSED_KEY[] = "focused";
        static const char INTERFACE_KEY[] = "interface";
        static shared_ptr<CapabilityConfiguration> getVisualActivityTrackerCapabilityConfiguration();
        shared_ptr<VisualActivityTracker> VisualActivityTracker::create(shared_ptr<ContextManagerInterface> contextManager) {
            if (!contextManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                return nullptr;
            }
            auto visualActivityTracker = shared_ptr<VisualActivityTracker>(new VisualActivityTracker(contextManager));
            contextManager->setStateProvider(CONTEXT_MANAGER_STATE, visualActivityTracker);
            return visualActivityTracker;
        }
        void VisualActivityTracker::provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) {
            ACSDK_DEBUG5(LX("provideState"));
            m_executor.submit([this, stateRequestToken]() { executeProvideState(stateRequestToken); });
        }
        void VisualActivityTracker::notifyOfActivityUpdates(const vector<Channel::State>& channels) {
            ACSDK_DEBUG5(LX("notifyOfActivityUpdates"));
            if (channels.empty()) {
                ACSDK_WARN(LX("notifyOfActivityUpdates").d("reason", "emptyVector"));
                return;
            }
            for (auto& channel : channels) {
                if (FocusManagerInterface::VISUAL_CHANNEL_NAME != channel.name) {
                    ACSDK_ERROR(LX("notifyOfActivityUpdates").d("reason", "InvalidChannelName").d("name", channel.name));
                    return;
                }
            }
            m_executor.submit([this, channels]() { m_channelState = channels.back(); });
        }
        VisualActivityTracker::VisualActivityTracker(shared_ptr<ContextManagerInterface> contextManager) : RequiresShutdown{"VisualActivityTracker"},
                                                                                                           m_contextManager{contextManager} {
            m_capabilityConfigurations.insert(getVisualActivityTrackerCapabilityConfiguration());
        }
        shared_ptr<CapabilityConfiguration> getVisualActivityTrackerCapabilityConfiguration() {
            std::unordered_map<string, string> configMap;
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, VISUALACTIVITYTRACKER_CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, VISUALACTIVITYTRACKER_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, VISUALACTIVITYTRACKER_CAPABILITY_INTERFACE_VERSION});
            return make_shared<CapabilityConfiguration>(configMap);
        }
        void VisualActivityTracker::doShutdown() {
            m_contextManager->removeStateProvider(CONTEXT_MANAGER_STATE);
            m_executor.shutdown();
            m_contextManager.reset();
        }
        void VisualActivityTracker::executeProvideState(unsigned int stateRequestToken) {
            ACSDK_DEBUG5(LX("executeProvideState"));
            Document payload(rapidjson::kObjectType);
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            auto sendEmptyContext = true;
            if (FocusState::NONE != m_channelState.focusState) {
                Value contextJson(kObjectType);
                Value interfaceKey(INTERFACE_KEY, 10);
                Value interfaceNameData(m_channelState.interfaceName.data(), m_channelState.interfaceName.length());
                contextJson.AddMember(interfaceKey, interfaceNameData,reinterpret_cast<basic_string<char, char_traits<char>, allocator<char>> &>(payload.GetAllocator()));
                payload.AddMember(FOCUSED_KEY, contextJson, payload.GetAllocator());
                if (!payload.Accept(writer)) { ACSDK_ERROR(LX("executeProvideState").d("reason", "writerRefusedJsonObject")); }
                else sendEmptyContext = false;
            }
            if (sendEmptyContext) m_contextManager->setState(CONTEXT_MANAGER_STATE,"",avsCommon::avs::StateRefreshPolicy::SOMETIMES, stateRequestToken);
            else m_contextManager->setState(CONTEXT_MANAGER_STATE, buffer.GetString(),StateRefreshPolicy::SOMETIMES, stateRequestToken);
        }
        unordered_set<shared_ptr<CapabilityConfiguration>> VisualActivityTracker::
            getCapabilityConfigurations() {
            return m_capabilityConfigurations;
        }
    }
}