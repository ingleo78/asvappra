#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEAKERMANAGEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEAKERMANAGEROBSERVERINTERFACE_H_

#include "ChannelVolumeInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class SpeakerManagerObserverInterface {
            public:
                enum class Source {
                    DIRECTIVE,
                    LOCAL_API
                };
                virtual void onSpeakerSettingsChanged(const Source& source, const ChannelVolumeInterface::Type& type,
                                                      const SpeakerInterface::SpeakerSettings& settings);
                virtual ~SpeakerManagerObserverInterface() = default;
            };
            inline std::ostream& operator<<(std::ostream& stream, SpeakerManagerObserverInterface::Source source) {
                switch(source) {
                    case SpeakerManagerObserverInterface::Source::DIRECTIVE:
                        stream << "DIRECTIVE";
                        return stream;
                    case SpeakerManagerObserverInterface::Source::LOCAL_API:
                        stream << "LOCAL_API";
                        return stream;
                }
                stream << "UNKNOWN";
                return stream;
            }
        }
    }
}
#endif