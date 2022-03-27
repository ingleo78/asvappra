#ifndef ACSDKMULTIROOMMUSIC_MRMHANDLERINTERFACE_H_
#define ACSDKMULTIROOMMUSIC_MRMHANDLERINTERFACE_H_

#include <memory>
#include <mutex>
#include <string>
#include <avs/AVSDirective.h>
#include <sdkinterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <sdkinterfaces/SpeakerInterface.h>
#include <util/RequiresShutdown.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace mrm {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            class MRMHandlerInterface : public RequiresShutdown {
            public:
                explicit MRMHandlerInterface(const string& shutdownName);
                virtual ~MRMHandlerInterface() override = default;
                virtual string getVersionString() const = 0;
                virtual bool handleDirective(const string& nameSpace, const string& name, const string& messageId, const string& payload) = 0;
                virtual void onSpeakerSettingsChanged(const ChannelVolumeInterface::Type& type) = 0;
                virtual void onUserInactivityReportSent() = 0;
                virtual void onCallStateChange(bool active) = 0;
                virtual void setObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer) = 0;
            };
            inline MRMHandlerInterface::MRMHandlerInterface(const string& shutdownName) : RequiresShutdown{shutdownName} {}
        }
    }
}
#endif