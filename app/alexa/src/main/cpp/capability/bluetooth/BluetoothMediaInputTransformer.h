#ifndef ACSDKBLUETOOTH_BLUETOOTHMEDIAINPUTTRANSFORMER_H_
#define ACSDKBLUETOOTH_BLUETOOTHMEDIAINPUTTRANSFORMER_H_

#include <memory>
#include <mutex>
#include <sdkinterfaces/PlaybackRouterInterface.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include <util/bluetooth/BluetoothEvents.h>

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace sdkInterfaces;
        using namespace utils::bluetooth;
        class BluetoothMediaInputTransformer : public enable_shared_from_this<BluetoothMediaInputTransformer>, public BluetoothEventListenerInterface {
        public:
            static shared_ptr<BluetoothMediaInputTransformer> create(shared_ptr<BluetoothEventBus> eventBus, shared_ptr<PlaybackRouterInterface> playbackRouter);
        protected:
            void onEventFired(const BluetoothEvent& event) override;
        private:
            BluetoothMediaInputTransformer(shared_ptr<BluetoothEventBus> eventBus, shared_ptr<PlaybackRouterInterface> playbackRouter);
            bool init();
            shared_ptr<BluetoothEventBus> m_eventBus;
            shared_ptr<PlaybackRouterInterface> m_playbackRouter;
        };
    }
}
#endif