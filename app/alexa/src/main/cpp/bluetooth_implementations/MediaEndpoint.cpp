#include <poll.h>
#include <logger/Logger.h>
#include "BlueZConstants.h"
#include "MediaEndpoint.h"
#include "a2dp-rtp.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            using namespace utils::bluetooth;
            constexpr const char* DBUS_ERROR_FAILED = "org.bluez.Error.Rejected";
            static const milliseconds POLL_TIMEOUT_MS(100);
            constexpr const char* MEDIAENDPOINT1_SETCONFIGURATION_METHOD_NAME = "SetConfiguration";
            constexpr const char* MEDIAENDPOINT1_SELECTCONFIGURATION_METHOD_NAME = "SelectConfiguration";
            constexpr const char* MEDIAENDPOINT1_CLEARCONFIGURATION_METHOD_NAME = "ClearConfiguration";
            constexpr const char* MEDIAENDPOINT1_RELEASE_METHOD_NAME = "Release";
            constexpr int CHECK_4_OPTIONS = 4;
            constexpr int CHECK_2_OPTIONS = 2;
            constexpr uint8_t FIRST_FREQUENCY = 0x10;
            constexpr uint8_t FIRST_CHANNEL_MODE = 0x01;
            constexpr uint8_t FIRST_BLOCK_LENGTH = 0x10;
            constexpr uint8_t FIRST_SUBBANDS = 0x04;
            constexpr uint8_t FIRST_ALLOCATION_METHOD = 0x01;
            constexpr int SAMPLING_RATE_16000 = 16000;
            constexpr int SAMPLING_RATE_32000 = 32000;
            constexpr int SAMPLING_RATE_44100 = 44100;
            constexpr int SAMPLING_RATE_48000 = 48000;
            constexpr size_t MAX_SANE_FRAME_LENGTH = 200;
            constexpr size_t MIN_SANE_FRAME_LENGTH = 1;
            constexpr size_t MAX_SANE_CODE_SIZE = MAX_SANE_FRAME_LENGTH * 32;
            constexpr size_t MIN_SANE_CODE_SIZE = 1;
            static const string TAG{"MediaEndpoint"};
            #define LX(event) LogEntry(TAG, event)
            static const gchar mediaEndpointIntrospectionXml[] = "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
                                                                 "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\";>\n<node>"
                                                                " <interface name=\"org.bluez.MediaEndpoint1\">  <method name=\"SetConfiguration\">"
                                                                "   <arg name=\"transport\" direction=\"in\" type=\"o\"/>   <arg name=\"properties\" direction=\""
                                                                "in\" type=\"a{sv}\"/>  </method>  <method name=\"SelectConfiguration\">   <arg name=\""
                                                                "capabilities\" direction=\"in\" type=\"ay\"/>   <arg name=\"configuration\" direction=\"out\""
                                                                " type=\"ay\"/>  </method>  <method name=\"ClearConfiguration\">  <arg name=\"transport\" "
                                                                "direction=\"in\" type=\"o\"/>  </method>  <method name=\"Release\">  </method> </interface>"
                                                                "</node>";
            MediaEndpoint::MediaEndpoint(shared_ptr<DBusConnection> connection, const string& endpointPath) :
                                         DBusObject(connection, mediaEndpointIntrospectionXml, endpointPath,
                              {{MEDIAENDPOINT1_SETCONFIGURATION_METHOD_NAME, &MediaEndpoint::onSetConfiguration},
                                         {MEDIAENDPOINT1_SELECTCONFIGURATION_METHOD_NAME, &MediaEndpoint::onSelectConfiguration},
                                         {MEDIAENDPOINT1_CLEARCONFIGURATION_METHOD_NAME, &MediaEndpoint::onClearConfiguration},
                                         {MEDIAENDPOINT1_RELEASE_METHOD_NAME, &MediaEndpoint::onRelease}}), m_endpointPath{endpointPath},
                                         m_operatingModeChanged{false}, m_operatingMode{OperatingMode::INACTIVE} {
                m_thread = std::thread(&MediaEndpoint::mediaThread, this);
            }
            MediaEndpoint::~MediaEndpoint() {
                ACSDK_DEBUG5(LX(__func__));
                setOperatingMode(OperatingMode::RELEASED);
                if (m_thread.joinable()) m_thread.join();
                ACSDK_DEBUG5(LX(__func__).m("MediaEndpoit finalized."));
            }
            void MediaEndpoint::abortStreaming() {
                setOperatingMode(OperatingMode::INACTIVE);
                shared_ptr<DBusProxy> deviceProxy = DBusProxy::create(BlueZConstants::BLUEZ_DEVICE_INTERFACE, m_streamingDevicePath);
                if (deviceProxy) deviceProxy->callMethod("Disconnect");
            }
            void MediaEndpoint::mediaThread() {
                pollfd pollStruct = {0, POLLIN, 0};
                shared_ptr<MediaContext> mediaContext;
                while(m_operatingMode != OperatingMode::RELEASED) {
                    {
                        unique_lock<mutex> modeLock(m_mutex);
                        m_modeChangeSignal.wait(modeLock, [this]() { return m_operatingModeChanged; });
                        m_operatingModeChanged = false;
                        if (OperatingMode::SINK != m_operatingMode) continue;
                        if (!m_currentMediaContext || !m_currentMediaContext->isSBCInitialized()) {
                            ACSDK_ERROR(LX("mediaThreadFailed").d("reason", "no valid media context, no media streaming started"));
                            continue;
                        }
                        mediaContext = m_currentMediaContext;
                    }
                    ACSDK_DEBUG5(LX("Starting media streaming..."));
                    pollStruct.fd = mediaContext->getStreamFD();
                    m_ioBuffer.resize(static_cast<unsigned long>(mediaContext->getReadMTU()));
                    const size_t sbcCodeSize = sbc_get_codesize(mediaContext->getSBCContextPtr());
                    const size_t sbcFrameLength = sbc_get_frame_length(mediaContext->getSBCContextPtr());
                    ACSDK_DEBUG9(LX(__func__).d("code size", sbcCodeSize).d("frame length", sbcFrameLength));
                    if (sbcFrameLength < MIN_SANE_FRAME_LENGTH || sbcFrameLength > MAX_SANE_FRAME_LENGTH) {
                        ACSDK_ERROR(LX("mediaThreadFailed").d("reason", "Invalid sbcFrameLength"));
                        abortStreaming();
                        continue;
                    }
                    if (sbcCodeSize < MIN_SANE_CODE_SIZE || sbcCodeSize > MAX_SANE_CODE_SIZE) {
                        ACSDK_ERROR(LX("mediaThreadFailed").d("reason", "Invalid sbcCodeSize"));
                        abortStreaming();
                        continue;
                    }
                    const size_t outBufferSize = sbcCodeSize * (m_ioBuffer.size() / sbcFrameLength + 1);
                    m_sbcBuffer.resize(outBufferSize);
                    ACSDK_DEBUG7(LX(__func__).d("codesize", sbcCodeSize).d("frame len", sbcFrameLength).d("output buf size", outBufferSize));
                    int positionInReadBuf = 0;
                    while (OperatingMode::SINK == m_operatingMode) {
                        int timeout = poll(&pollStruct, 1, static_cast<int>(POLL_TIMEOUT_MS.count()));
                        if (0 == timeout) continue;
                        if (timeout < 0) {
                            ACSDK_ERROR(LX("mediaThreadFailed").d("reason", "Failed to poll bluetooth media stream"));
                            abortStreaming();
                            break;
                        }
                        if (OperatingMode::SINK != m_operatingMode) break;
                        ssize_t bytesReadSigned = read(pollStruct.fd, m_ioBuffer.data() + positionInReadBuf, m_ioBuffer.size() - positionInReadBuf);
                        if (bytesReadSigned < 0) {
                            ACSDK_ERROR(LX("mediaThreadFailed").d("reason", "Failed to read bluetooth media stream").d("errno", errno));
                            abortStreaming();
                            break;
                        }
                        if (0 == bytesReadSigned) {
                            setOperatingMode(OperatingMode::INACTIVE);
                            break;
                        }
                        size_t bytesRead = static_cast<size_t>(bytesReadSigned);
                        if (bytesRead < sizeof(rtp_header)) {
                            ACSDK_DEBUG9(LX(__func__).d("reason", "Invalid RPT frame, skipping..."));
                            continue;
                        }
                        rtp_header_t* rtpHeader = reinterpret_cast<rtp_header_t*>(m_ioBuffer.data());
                        const rtp_payload_sbc_t* rtpPayload = reinterpret_cast<const rtp_payload_sbc_t*>(&rtpHeader->csrc[rtpHeader->cc]);
                        const uint8_t* payloadData = reinterpret_cast<const uint8_t*>(rtpPayload + 1);
                        uint8_t* output = m_sbcBuffer.data();
                        size_t headersSize = reinterpret_cast<size_t>(payloadData) - reinterpret_cast<size_t>(m_ioBuffer.data());
                        size_t inputLength = bytesRead - headersSize;
                        if (inputLength > m_ioBuffer.size()) {
                            ACSDK_DEBUG9(LX(__func__).d("reason", "Invalid RPT packet, skipping"));
                            continue;
                        }
                        size_t outputLength = outBufferSize;
                        size_t frameCount = rtpPayload->frame_count;
                        while(frameCount-- && inputLength >= sbcFrameLength) {
                            ssize_t bytesProcessed = 0;
                            size_t bytesDecoded = 0;
                            if ((bytesProcessed = sbc_decode(mediaContext->getSBCContextPtr(), payloadData, inputLength, output, outputLength, &bytesDecoded)) < 0) {
                                ACSDK_ERROR(LX("mediaThreadFailed").d("reason", "SBC decoding error").d("error", strerror(-bytesProcessed)));
                                break;
                            }
                            payloadData += bytesProcessed;
                            inputLength -= bytesProcessed;
                            output += bytesDecoded;
                            outputLength -= bytesDecoded;
                        }
                        size_t writeSize = output - m_sbcBuffer.data();
                        if (OperatingMode::SINK != m_operatingMode) break;
                        m_ioStream->send(m_sbcBuffer.data(), writeSize);
                    }
                }
                mediaContext.reset();
                ACSDK_DEBUG5(LX("Exiting media thread."));
            }
            shared_ptr<FormattedAudioStreamAdapter> MediaEndpoint::getAudioStream() {
                lock_guard<mutex> guard(m_streamMutex);
                if (m_ioStream) return m_ioStream;
                m_ioStream = make_shared<FormattedAudioStreamAdapter>(m_audioFormat);
                return m_ioStream;
            }
            string MediaEndpoint::getEndpointPath() const {
                return m_endpointPath;
            }
            string MediaEndpoint::getStreamingDevicePath() const {
                return m_streamingDevicePath;
            }
            void MediaEndpoint::setOperatingMode(OperatingMode mode) {
                lock_guard<mutex> modeLock(m_mutex);
                m_operatingMode = mode;
                m_operatingModeChanged = true;
                ACSDK_DEBUG5(LX(__func__).d("newMode", operatingModeToString(mode)));
                m_modeChangeSignal.notify_all();
            }
            void MediaEndpoint::onMediaTransportStateChanged(MediaStreamingState newState, const string& devicePath) {
                ACSDK_DEBUG5(LX(__func__).d("devicePath", devicePath));
                if (OperatingMode::RELEASED == m_operatingMode) return;
                if (devicePath != m_streamingDevicePath) return;
                if (MediaStreamingState::PENDING == newState) {
                    shared_ptr<DBusProxy> transportProxy = DBusProxy::create(BlueZConstants::BLUEZ_MEDIATRANSPORT_INTERFACE, m_streamingDevicePath);
                    if (!transportProxy) {
                        ACSDK_ERROR(LX("onMediaTransportStateChangedFailed").d("reason", "Failed to get MediaTransport1 proxy").d("path", devicePath));
                        return;
                    }
                    ManagedGError error;
                    GUnixFDList* fdList = nullptr;
                    ManagedGVariant transportDetails = transportProxy->callMethodWithFDList("Acquire", nullptr, &fdList, error.toOutputParameter());
                    if (error.hasError()) {
                        ACSDK_ERROR(LX("onMediaTransportStateChangedFailed").d("reason", "Failed to acquire media stream").d("error", error.getMessage()));
                        return;
                    } else if (!fdList) {
                        ACSDK_ERROR(LX("onMediaTransportStateChangedFailed").d("reason", "nullFdList").d("error", error.getMessage()));
                        return;
                    }
                    gint32 streamFDIndex = 0;
                    guint16 readMTU = 0;
                    guint16 writeMTU = 0;
                    g_variant_get(transportDetails.get(), "(hqq)", &streamFDIndex, &readMTU, &writeMTU);
                    if (streamFDIndex > g_unix_fd_list_get_length(fdList)) {
                        ACSDK_ERROR(LX("onMediaTransportStateChangedFailed").d("reason", "indexOutOfBounds").d("fdIndex", streamFDIndex)
                            .d("fdListLength", g_unix_fd_list_get_length(fdList)));
                        return;
                    }
                    gint streamFD = g_unix_fd_list_get(fdList, streamFDIndex, error.toOutputParameter());
                    if (streamFD < 0) {
                        ACSDK_ERROR(LX("onMediaTransportStateChangedFailed").d("reason", "Invalid media stream file descriptor"));
                        return;
                    }
                    if (error.hasError()) {
                        ACSDK_ERROR(LX("onMediaTransportStateChangedFailed").d("reason", "failedToGetFD").d("error", error.getMessage()));
                        close(streamFD);
                        return;
                    }
                    ACSDK_DEBUG9(LX("Transport details").d("File Descriptor Index", streamFDIndex).d("File Descriptor", streamFD).d("read MTU", readMTU)
                        .d("write MTU", writeMTU));
                    {
                        lock_guard<std::mutex> modeLock(m_mutex);
                        if (!m_currentMediaContext) m_currentMediaContext = make_shared<MediaContext>();
                        m_currentMediaContext->setStreamFD(streamFD);
                        m_currentMediaContext->setReadMTU(readMTU);
                        m_currentMediaContext->setWriteMTU(writeMTU);
                    }
                    getAudioStream();
                    setOperatingMode(OperatingMode::SINK);
                } else if (MediaStreamingState::IDLE == newState) setOperatingMode(OperatingMode::INACTIVE);
            }
            void MediaEndpoint::onSetConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                {
                    lock_guard<std::mutex> modeLock(m_mutex);
                    if (!m_currentMediaContext) m_currentMediaContext = make_shared<MediaContext>();
                    m_currentMediaContext->setSBCInitialized(false);
                    const char* path = nullptr;
                    GVariant* configuration = nullptr;
                    GVariant* variant = g_dbus_method_invocation_get_parameters(invocation);
                    g_variant_get(variant, "(o*)", &path, &configuration);
                    GVariantMapReader mapReader(configuration);
                    ManagedGVariant sbcConfigVar = mapReader.getVariant("Configuration");
                    if (sbcConfigVar.hasValue()) {
                        gsize configSize = 0;
                        gconstpointer ptr = g_variant_get_fixed_array(sbcConfigVar.get(), &configSize, 1);
                        if (ptr) {
                            sbc_t* sbcContext = m_currentMediaContext->getSBCContextPtr();
                            int sbcError = -sbc_init_a2dp(sbcContext, 0, ptr, configSize);
                            if (sbcError != 0) {
                                ACSDK_ERROR(LX("onSetConfigurationFailed").d("reason", "Failed to init SBC decoder") .d("error", strerror(sbcError)));
                                g_dbus_method_invocation_return_dbus_error(invocation, DBUS_ERROR_FAILED, "Failed to init SBC decoder");
                                return;
                            } else {
                                m_audioFormat.encoding = AudioFormat::Encoding::LPCM;
                                m_audioFormat.endianness = SBC_LE == sbcContext->endian ? AudioFormat::Endianness::LITTLE : AudioFormat::Endianness::BIG;
                                m_audioFormat.sampleSizeInBits = 16;
                                m_audioFormat.numChannels = SBC_MODE_MONO == sbcContext->mode ? 1 : 2;
                                switch(sbcContext->frequency) {
                                    case SBC_FREQ_16000: m_audioFormat.sampleRateHz = SAMPLING_RATE_16000; break;
                                    case SBC_FREQ_32000: m_audioFormat.sampleRateHz = SAMPLING_RATE_32000; break;
                                    case SBC_FREQ_44100: m_audioFormat.sampleRateHz = SAMPLING_RATE_44100; break;
                                    default: m_audioFormat.sampleRateHz = SAMPLING_RATE_48000; break;
                                }
                                m_audioFormat.layout = avsCommon::utils::AudioFormat::Layout::INTERLEAVED;
                                m_audioFormat.dataSigned = true;
                                ACSDK_DEBUG5(LX("Bluetooth stream parameters").d("numChannels", m_audioFormat.numChannels).d("rate", m_audioFormat.sampleRateHz));
                                m_currentMediaContext->setSBCInitialized(true);
                            }
                        } else {
                            ACSDK_ERROR(LX("onSetConfigurationFailed").d("reason", "Failed to convert sbc configuration to bytestream"));
                            g_dbus_method_invocation_return_dbus_error(invocation, DBUS_ERROR_FAILED, "Failed to convert sbc configuration to bytestream");
                            return;
                        }
                    } else {
                        ACSDK_ERROR(LX("onSetConfigurationFailed").d("reason", "Failed to read SBC configuration"));
                        g_dbus_method_invocation_return_dbus_error(invocation, DBUS_ERROR_FAILED, "Failed to read SBC configuration");
                        return;
                    }
                    ACSDK_DEBUG3(LX(__func__).d("mediaTransport", path));
                    m_streamingDevicePath = path;
                }
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            static uint8_t selectCompatibleConfig(uint8_t caps, uint8_t mask, int width) {
                for (; !(caps & mask) && width > 0; mask <<= 1, width--);
                return mask;
            }
            void MediaEndpoint::onSelectConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                GVariantTupleReader capsTuple(g_dbus_method_invocation_get_parameters(invocation));
                ManagedGVariant streamSupportedCapabilitiesBytes = capsTuple.getVariant(0);
                gsize configSize = 0;
                gconstpointer ptr = g_variant_get_fixed_array(streamSupportedCapabilitiesBytes.get(), &configSize, sizeof(uint8_t));
                if (ptr && configSize >= 4) {
                    auto capabilitiesBytes = static_cast<const uint8_t*>(ptr);
                    uint8_t byte1 = capabilitiesBytes[0];
                    uint8_t byte2 = capabilitiesBytes[1];
                    uint8_t selectedConfig1 = selectCompatibleConfig(byte1, FIRST_FREQUENCY, CHECK_4_OPTIONS);
                    selectedConfig1 |= selectCompatibleConfig(byte1, FIRST_CHANNEL_MODE, CHECK_4_OPTIONS);
                    uint8_t selectedConfig2 = selectCompatibleConfig(byte2, FIRST_BLOCK_LENGTH, CHECK_4_OPTIONS);
                    selectedConfig2 |= selectCompatibleConfig(byte2, FIRST_SUBBANDS, CHECK_2_OPTIONS);
                    selectedConfig2 |= selectCompatibleConfig(byte2, FIRST_ALLOCATION_METHOD, CHECK_2_OPTIONS);
                    GVariantBuilder* capBuilder = g_variant_builder_new(G_VARIANT_TYPE("ay"));
                    g_variant_builder_add(capBuilder, "y", selectedConfig1);
                    g_variant_builder_add(capBuilder, "y", selectedConfig2);
                    g_variant_builder_add(capBuilder, "y", capabilitiesBytes[2]);
                    g_variant_builder_add(capBuilder, "y", capabilitiesBytes[3]);
                    g_dbus_method_invocation_return_value(invocation, g_variant_new("(ay)", capBuilder));
                } else { ACSDK_ERROR(LX("onSelectConfigurationFailed").d("reason", "Invalid SBC config")); }
            }
            void MediaEndpoint::onClearConfiguration(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
            string MediaEndpoint::operatingModeToString(OperatingMode mode) {
                switch (mode) {
                    case OperatingMode::INACTIVE: return "INACTIVE";
                    case OperatingMode::SINK: return "SINK";
                    case OperatingMode::SOURCE: return "SOURCE";
                    case OperatingMode::RELEASED: return "RELEASED";
                }
                return "UNKNOWN";
            }
            void MediaEndpoint::onRelease(GVariant* arguments, GDBusMethodInvocation* invocation) {
                ACSDK_DEBUG5(LX(__func__));
                g_dbus_method_invocation_return_value(invocation, nullptr);
            }
        }
    }
}