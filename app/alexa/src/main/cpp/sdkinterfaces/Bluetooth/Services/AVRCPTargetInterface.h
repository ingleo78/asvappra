#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPTARGETINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPTARGETINTERFACE_H_

#include <ostream>
#include <string>
#include "BluetoothServiceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    enum class MediaCommand {
                        PLAY,
                        PAUSE,
                        NEXT,
                        PREVIOUS,
                        PLAY_PAUSE
                    };
                    inline std::string_view commandToString(MediaCommand cmd) {
                        switch (cmd) {
                            case MediaCommand::PLAY: return "PLAY";
                            case MediaCommand::PAUSE: return "PAUSE";
                            case MediaCommand::NEXT: return "NEXT";
                            case MediaCommand::PREVIOUS: return "PREVIOUS";
                            case MediaCommand::PLAY_PAUSE: return "PLAY_PAUSE";
                        }
                        return "UNKNOWN";
                    }
                    inline std::ostream& operator<<(std::ostream& stream, const MediaCommand cmd) {
                        return stream << commandToString(cmd);
                    }
                    class AVRCPTargetInterface : public BluetoothServiceInterface {
                    public:
                        static constexpr const char* UUID = "0000110c-0000-1000-8000-00805f9b34fb";
                        static constexpr const char* NAME = "A/V_RemoteControlTarget";
                        virtual bool play() = 0;
                        virtual bool pause() = 0;
                        virtual bool next() = 0;
                        virtual bool previous() = 0;
                        virtual ~AVRCPTargetInterface() = default;
                    };
                }
            }
        }
    }
}
#endif