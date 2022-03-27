#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MEDIAENDPOINT_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MEDIAENDPOINT_H_

#include <atomic>
#include <condition_variable>
#include <vector>
#include <avs/AudioInputStream.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <util/bluetooth/FormattedAudioStreamAdapter.h>
#include <gio/gio.h>
#include <sbc/sbc.h>
#include "BlueZDeviceManager.h"
#include "BlueZUtils.h"
#include "DBusObject.h"
#include "DBusConnection.h"
#include "MediaContext.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace utils::bluetooth;
            class MediaEndpoint : public DBusObject<MediaEndpoint> {
            public:
                MediaEndpoint(shared_ptr<DBusConnection> connection, const string& endpointPath);
                ~MediaEndpoint() override;
                void onSetConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation);
                void onSelectConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation);
                void onClearConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation);
                void onRelease(GVariant* arguments, GDBusMethodInvocation* invocation);
                void onMediaTransportStateChanged(MediaStreamingState newState, const string& devicePath);
                string getEndpointPath() const;
                string getStreamingDevicePath() const;
                shared_ptr<FormattedAudioStreamAdapter> getAudioStream();
            private:
                enum class OperatingMode {
                    INACTIVE,
                    SINK,
                    SOURCE,
                    RELEASED
                };
                string operatingModeToString(OperatingMode mode);
                void mediaThread();
                void setOperatingMode(OperatingMode mode);
                void abortStreaming();
                string m_endpointPath;
                string m_streamingDevicePath;
                bool m_operatingModeChanged;
                atomic<OperatingMode> m_operatingMode;
                vector<uint8_t> m_sbcBuffer;
                mutex m_mutex;
                mutex m_streamMutex;
                condition_variable m_modeChangeSignal;
                shared_ptr<FormattedAudioStreamAdapter> m_ioStream;
                vector<uint8_t> m_ioBuffer;
                AudioFormat m_audioFormat;
                shared_ptr<MediaContext> m_currentMediaContext;
                thread m_thread;
            };
        }
    }
}
#endif