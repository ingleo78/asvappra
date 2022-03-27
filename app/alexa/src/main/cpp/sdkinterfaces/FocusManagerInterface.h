#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_FOCUSMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_FOCUSMANAGERINTERFACE_H_

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <avs/ContentType.h>
#include "ChannelObserverInterface.h"
#include "FocusManagerObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace avs;
            using namespace chrono;
            class FocusManagerInterface {
            public:
                class Activity {
                public:
                    static shared_ptr<Activity> create(const std::string& interfaceName, const shared_ptr<ChannelObserverInterface>& channelObserver,
                                                       const milliseconds& patienceDuration = milliseconds::zero(),
                                                       const ContentType contentType = ContentType::NONMIXABLE);
                    bool operator==(const Activity& rhs) { return this->m_interface == rhs.m_interface; }
                    const std::string getInterface() const;
                    milliseconds getPatienceDuration() const;
                    ContentType getContentType() const;
                    void setContentType(ContentType contentType);
                    MixingBehavior getMixingBehavior() const;
                    shared_ptr<ChannelObserverInterface> getChannelObserver() const;
                    bool notifyObserver(FocusState focus, MixingBehavior behavior);
                private:
                    Activity(const std::string& interfaceName, const milliseconds& patienceDuration, const shared_ptr<ChannelObserverInterface>& channelObserver,
                             const ContentType contentType) : m_interface{interfaceName}, m_patienceDuration{patienceDuration}, m_channelObserver{channelObserver},
                             m_contentType{contentType}, m_mixingBehavior{MixingBehavior::UNDEFINED} {}
                    void setMixingBehavior(MixingBehavior behavior);
                    mutable mutex m_mutex;
                    const std::string m_interface;
                    const milliseconds m_patienceDuration;
                    const shared_ptr<ChannelObserverInterface> m_channelObserver;
                    ContentType m_contentType;
                    MixingBehavior m_mixingBehavior;
                };
                static constexpr const char* DIALOG_CHANNEL_NAME = "Dialog";
                static constexpr unsigned int DIALOG_CHANNEL_PRIORITY = 100;
                static constexpr const char* COMMUNICATIONS_CHANNEL_NAME = "Communications";
                static constexpr unsigned int COMMUNICATIONS_CHANNEL_PRIORITY = 150;
                static constexpr const char* ALERT_CHANNEL_NAME = "Alert";
                static constexpr unsigned int ALERT_CHANNEL_PRIORITY = 200;
                static constexpr const char* CONTENT_CHANNEL_NAME = "Content";
                static constexpr unsigned int CONTENT_CHANNEL_PRIORITY = 300;
                static constexpr const char* VISUAL_CHANNEL_NAME = "Visual";
                static constexpr unsigned int VISUAL_CHANNEL_PRIORITY = 100;
                virtual ~FocusManagerInterface() = default;
                virtual bool acquireChannel(const std::string& channelName, shared_ptr<ChannelObserverInterface> channelObserver, const std::string& interfaceName) = 0;
                virtual bool acquireChannel(const std::string& channelName, shared_ptr<FocusManagerInterface::Activity> channelActivity) = 0;
                virtual future<bool> releaseChannel(const std::string& channelName, shared_ptr<ChannelObserverInterface> channelObserver) = 0;
                virtual void stopForegroundActivity() = 0;
                virtual void stopAllActivities() = 0;
                virtual void addObserver(const shared_ptr<FocusManagerObserverInterface>& observer) = 0;
                virtual void removeObserver(const shared_ptr<FocusManagerObserverInterface>& observer) = 0;
                virtual void modifyContentType(const std::string& channelName, const std::string& interfaceName, ContentType contentType) = 0;
            };
            inline shared_ptr<FocusManagerInterface::Activity> FocusManagerInterface::Activity::create(const std::string& interfaceName,
                                                                                                       const shared_ptr<ChannelObserverInterface>& channelObserver,
                                                                                                       const milliseconds& patienceDuration,
                                                                                                       const ContentType contentType) {
                if (interfaceName.empty() || patienceDuration.count() < 0 || channelObserver == nullptr) return nullptr;
                auto activity = shared_ptr<FocusManagerInterface::Activity>(new FocusManagerInterface::Activity(interfaceName, patienceDuration,
                                                                            channelObserver, contentType));
                return activity;
            }
            inline const std::string FocusManagerInterface::Activity::getInterface() const {
                return m_interface;
            }
            inline milliseconds FocusManagerInterface::Activity::getPatienceDuration() const {
                return m_patienceDuration;
            }
            inline ContentType FocusManagerInterface::Activity::getContentType() const {
                unique_lock<mutex> lock(m_mutex);
                return m_contentType;
            }
            inline void FocusManagerInterface::Activity::setContentType(ContentType contentType) {
                unique_lock<mutex> lock(m_mutex);
                m_contentType = contentType;
            }
            inline MixingBehavior FocusManagerInterface::Activity::getMixingBehavior() const {
                lock_guard<std::mutex> lock(m_mutex);
                return m_mixingBehavior;
            }
            inline shared_ptr<ChannelObserverInterface> FocusManagerInterface::Activity::
                getChannelObserver() const {
                return m_channelObserver;
            }
            inline bool FocusManagerInterface::Activity::notifyObserver(FocusState focus, MixingBehavior behavior) {
                if (m_channelObserver) {
                    MixingBehavior overrideBehavior{behavior};
                    if ((MixingBehavior::MUST_PAUSE == getMixingBehavior()) && (MixingBehavior::MAY_DUCK == behavior)) {
                        overrideBehavior = MixingBehavior::MUST_PAUSE;
                    }
                    m_channelObserver->onFocusChanged(focus, overrideBehavior);
                    setMixingBehavior(overrideBehavior);
                    return true;
                }
                return false;
            }
            inline void FocusManagerInterface::Activity::setMixingBehavior(MixingBehavior behavior) {
                unique_lock<mutex> lock(m_mutex);
                m_mixingBehavior = behavior;
            }
        }
    }
}
#endif