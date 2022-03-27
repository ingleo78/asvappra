#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include <logger/Logger.h>
#include "BluetoothMediaInputTransformer.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace avs;
        using namespace logger;
        using namespace sdkInterfaces::bluetooth::services;
        static const string TAG{"BluetoothMediaInputTransformer"};
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<BluetoothMediaInputTransformer> BluetoothMediaInputTransformer::create(shared_ptr<BluetoothEventBus> eventBus, shared_ptr<PlaybackRouterInterface> playbackRouter) {
            ACSDK_DEBUG5(LX(__func__));
            if (!eventBus) { ACSDK_ERROR(LX(__func__).d("reason", "nullEventBus")); }
            else if (!playbackRouter) { ACSDK_ERROR(LX(__func__).d("reason", "nullPlaybackRouter")); }
            else {
                auto mediaInputTransformer = shared_ptr<BluetoothMediaInputTransformer>(new BluetoothMediaInputTransformer(eventBus, playbackRouter));
                if (mediaInputTransformer->init()) return mediaInputTransformer;
                else { ACSDK_ERROR(LX(__func__).d("reason", "initFailed")); }
            }
            return nullptr;
        }
        BluetoothMediaInputTransformer::BluetoothMediaInputTransformer(shared_ptr<BluetoothEventBus> eventBus, shared_ptr<PlaybackRouterInterface> playbackRouter) :
                                                                       m_eventBus{eventBus}, m_playbackRouter{playbackRouter} {}
        bool BluetoothMediaInputTransformer::init() {
            ACSDK_DEBUG5(LX(__func__));
            m_eventBus->addListener({BluetoothEventType::MEDIA_COMMAND_RECEIVED}, shared_from_this());
            return true;
        }
        void BluetoothMediaInputTransformer::onEventFired(const BluetoothEvent& event) {
            ACSDK_DEBUG5(LX(__func__));
            if (BluetoothEventType::MEDIA_COMMAND_RECEIVED != event.getType()) {
                ACSDK_ERROR(LX(__func__).d("reason", "unexpectedEventReceived"));
                return;
            }
            shared_ptr<MediaCommand> mediaCommand = event.getMediaCommand();
            if (!mediaCommand) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullMediaCommand"));
                return;
            }
            switch(*mediaCommand) {
                case MediaCommand::PLAY: m_playbackRouter->buttonPressed(PlaybackButton::PLAY); break;
                case MediaCommand::PAUSE: m_playbackRouter->buttonPressed(PlaybackButton::PAUSE); break;
                case MediaCommand::NEXT: m_playbackRouter->buttonPressed(PlaybackButton::NEXT); break;
                case MediaCommand::PREVIOUS: m_playbackRouter->buttonPressed(PlaybackButton::PREVIOUS); break;
                case MediaCommand::PLAY_PAUSE: m_playbackRouter->buttonPressed(PlaybackButton::PLAY); break;
                default:
                    ACSDK_ERROR(LX(__func__).d("reason", "commandNotSupported").d("command", *mediaCommand));
                    return;
            }
        }
    }
}