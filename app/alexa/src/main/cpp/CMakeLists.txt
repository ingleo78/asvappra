cmake_minimum_required(VERSION 3.18.1)
project("alexa")
add_library(alexa SHARED alexa.cpp
        #[[acl/Transport/Common/MockHTTP2Connection.cpp
        acl/Transport/Common/MockHTTP2Request.cpp
        acl/Transport/Common/MockMimeResponseSink.cpp
        acl/Transport/AVSConnectionManagerTest.cpp
        acl/Transport/DownchannelHandler.cpp
        acl/Transport/ExchangeHandler.cpp
        acl/Transport/HTTP2Transport.cpp
        acl/Transport/HTTP2TransportFactory.cpp
        acl/Transport/HTTP2TransportTest.cpp
        acl/Transport/MessageRequestHandler.cpp
        acl/Transport/MessageRequestHandlerTest.cpp
        acl/Transport/MessageRequestQueue.cpp
        acl/Transport/MessageRouter.cpp
        acl/Transport/MessageRouterFactory.cpp
        acl/Transport/MessageRouterTest.cpp
        acl/Transport/MimeResponseSink.cpp
        acl/Transport/PingHandler.cpp
        acl/Transport/PostConnectSequencer.cpp
        acl/Transport/PostConnectSequencerFactory.cpp
        acl/Transport/PostConnectSequencerFactoryTest.cpp
        acl/Transport/PostConnectSequencerTest.cpp
        acl/Transport/SynchronizedMessageRequestQueue.cpp
        acl/Transport/TransportDefines.cpp
        acl/AVSConnectionManager.cpp
        acsdk_alerts/Renderer/Renderer.cpp
        acsdk_alerts/Storage/SQLiteAlertStorage.cpp
        acsdk_alerts/Alarm.cpp
        acsdk_alerts/AlarmAlertTest.cpp
        acsdk_alerts/Alert.cpp
        acsdk_alerts/AlertsCapabilityAgent.cpp
        acsdk_alerts/AlertsCapabilityAgentTest.cpp
        acsdk_alerts/AlertScheduler.cpp
        acsdk_alerts/AlertSchedulerTest.cpp
        acsdk_alerts/AlertTest.cpp
        acsdk_alerts/Reminder.cpp
        acsdk_alerts/ReminderAlertTest.cpp
        acsdk_alerts/RendererTest.cpp
        acsdk_alerts/Timer.cpp
        acsdk_alerts/TimerAlertTest.cpp
        acsdk_alerts_interfaces/AlertObserverInterface.cpp
        adsl/common/MockDirectiveHandler.cpp
        adsl/DirectiveProcessor.cpp
        adsl/DirectiveProcessorTest.cpp
        adsl/DirectiveRouter.cpp
        adsl/DirectiveRouterTest.cpp
        adsl/DirectiveSequencer.cpp
        adsl/DirectiveSequencerTest.cpp
        adsl/MessageInterpreter.cpp
        adsl/MessageInterpreterTest.cpp
        afml/AudioActivityTracker.cpp
        afml/AudioActivityTrackerTest.cpp
        afml/Channel.cpp
        afml/FocusManager.cpp
        afml/FocusManagerTest.cpp
        afml/VisualActivityTracker.cpp
        afml/VisualActivityTrackerTest.cpp
        app_utilities/android_utilities/AndroidLogger.cpp
        app_utilities/android_utilities/AndroidSLESBufferQueue.cpp
        app_utilities/android_utilities/AndroidSLESEngine.cpp
        app_utilities/android_utilities/AndroidSLESMicrophone.cpp
        app_utilities/android_utilities/AndroidSLESObject.cpp
        app_utilities/default_client/ConnectionRetryTrigger.cpp
        app_utilities/default_client/DefaultClient.cpp
        app_utilities/default_client/DeviceSettingsManagerBuilder.cpp
        app_utilities/default_client/EqualizerRuntimeSetup.cpp
        app_utilities/sdkcomponent/SDKComponent.cpp
        app_utilities/sdkcomponent/SDKComponentTest.cpp
        app_utilities/system_sound_player/SystemSoundPlayer.cpp
        app_utilities/system_sound_player/SystemSoundPlayerTest.cpp
        autorization/CBLAuthDelegate/CBLAuthDelegate.cpp
        autorization/CBLAuthDelegate/CBLAuthDelegateConfiguration.cpp
        autorization/CBLAuthDelegate/SQLiteCBLAuthDelegateStorage.cpp
        autorization/DefaultEndpoint/DefaultEndpointModeControllerHandler.cpp
        autorization/DefaultEndpoint/DefaultEndpointRangeControllerHandler.cpp
        autorization/DefaultEndpoint/DefaultEndpointToggleControllerHandler.cpp
        autorization/PeripheralEndpoint/PeripheralEndpointModeControllerHandler.cpp
        autorization/PeripheralEndpoint/PeripheralEndpointPowerControllerHandler.cpp
        autorization/PeripheralEndpoint/PeripheralEndpointRangeControllerHandler.cpp
        autorization/PeripheralEndpoint/PeripheralEndpointToggleControllerHandler.cpp
        autorization/CaptionPresenter.cpp
        autorization/ConnectionObserver.cpp
        autorization/ConsolePrinter.cpp
        autorization/ConsoleReader.cpp
        autorization/ExternalCapabilitiesBuilder.cpp
        autorization/GuiRenderer.cpp
        autorization/InteractionManager.cpp
        autorization/KeywordObserver.cpp
        autorization/LocaleAssetsManager.cpp
        autorization/main.cpp
        autorization/PortAudioMicrophoneWrapper.cpp
        autorization/RevokeAuthorizationObserver.cpp
        autorization/SampleApplication.cpp
        autorization/SampleApplicationComponent.cpp
        autorization/SampleEqualizerModeController.cpp
        autorization/SDKDiagnostics.cpp
        autorization/UIManager.cpp
        autorization/UserInputManager.cpp
        avs/attachment/Attachment.cpp
        avs/attachment/AttachmentManager.cpp
        avs/attachment/AttachmentUtils.cpp
        avs/attachment/InProcessAttachment.cpp
        avs/attachment/InProcessAttachmentReader.cpp
        avs/attachment/InProcessAttachmentWriter.cpp
        avs/AbstractAVSConnectionManager.cpp
        avs/AlexaClientSDKInit.cpp
        avs/AVSContext.cpp
        avs/AVSDirective.cpp
        avs/AVSMessage.cpp
        avs/AVSMessageHeader.cpp
        avs/BlockingPolicy.cpp
        avs/CapabilityAgent.cpp
        avs/CapabilityConfiguration.cpp
        avs/CapabilityResources.cpp
        avs/CapabilityTag.cpp
        avs/ComponentConfiguration.cpp
        avs/DialogUXStateAggregator.cpp
        avs/DirectiveRoutingRule.cpp
        avs/EventBuilder.cpp
        avs/ExceptionEncounteredSender.cpp
        avs/HandlerAndPolicy.cpp
        avs/MessageRequest.cpp
        avs/NamespaceAndName.cpp
        avs/WaitableMessageRequest.cpp
        avs_gateway_manager/Storage/AVSGatewayManagerStorage.cpp
        avs_gateway_manager/Storage/AVSGatewayManagerStorageTest.cpp
        avs_gateway_manager/AVSGatewayManager.cpp
        avs_gateway_manager/AVSGatewayManagerTest.cpp
        avs_gateway_manager/PostConnectVerifyGatewaySender.cpp
        avs_gateway_manager/PostConnectVerifyGatewaySenderTest.cpp
        bluetooth_implementations/BlueZA2DPSink.cpp
        bluetooth_implementations/BlueZA2DPSource.cpp
        bluetooth_implementations/BlueZAVRCPController.cpp
        bluetooth_implementations/BlueZAVRCPTarget.cpp
        bluetooth_implementations/BlueZBluetoothDevice.cpp
        bluetooth_implementations/BlueZBluetoothDeviceManager.cpp
        bluetooth_implementations/BlueZDeviceManager.cpp
        bluetooth_implementations/BlueZHFP.cpp
        bluetooth_implementations/BlueZHID.cpp
        bluetooth_implementations/BlueZHostController.cpp
        bluetooth_implementations/BlueZSPP.cpp
        bluetooth_implementations/DBusConnection.cpp
        bluetooth_implementations/DBusObjectBase.cpp
        bluetooth_implementations/DBusPropertiesProxy.cpp
        bluetooth_implementations/DBusProxy.cpp
        bluetooth_implementations/GVariantMapReader.cpp
        bluetooth_implementations/GVariantTupleReader.cpp
        bluetooth_implementations/MediaContext.cpp
        bluetooth_implementations/MediaEndpoint.cpp
        bluetooth_implementations/MPRISPlayer.cpp
        bluetooth_implementations/PairingAgent.cpp
        bluetooth_implementations/PulseAudioBluetoothInitializer.cpp
        capabilities_delegate/Storage/SQLiteCapabilitiesDelegateStorage.cpp
        capabilities_delegate/Storage/SQLiteCapabilitiesDelegateStorageTest.cpp
        capabilities_delegate/Utils/DiscoveryUtils.cpp
        capabilities_delegate/Utils/DiscoveryUtilsTest.cpp
        capabilities_delegate/CapabilitiesDelegate.cpp
        capabilities_delegate/CapabilitiesDelegateTest.cpp
        capabilities_delegate/DiscoveryEventSender.cpp
        capabilities_delegate/DiscoveryEventSenderTest.cpp
        capabilities_delegate/PostConnectCapabilitiesPublisher.cpp
        capabilities_delegate/PostConnectCapabilitiesPublisherTest.cpp
        capability/audio_player/AudioPlayer.cpp
        capability/audio_player/AudioPlayerTest.cpp
        capability/audio_player/ProgressTimer.cpp
        capability/audio_player/ProgressTimerTest.cpp
        capability/bluetooth/BasicDeviceConnectionRule.cpp
        capability/bluetooth/Bluetooth.cpp
        capability/bluetooth/BluetoothEventState.cpp
        capability/bluetooth/BluetoothMediaInputTransformer.cpp
        capability/bluetooth/BluetoothMediaInputTransformerTest.cpp
        capability/bluetooth/BluetoothTest.cpp
        capability/bluetooth/SQLiteBluetoothStorage.cpp
        capability/bluetooth/SQLiteBluetoothStorageTest.cpp
        capability/do_not_disturb/DNDMessageRequest.cpp
        capability/do_not_disturb/DNDSettingProtocol.cpp
        capability/do_not_disturb/DoNotDisturbCapabilityAgent.cpp
        capability/do_not_disturb/DoNotDisturbCapabilityAgentTest.cpp
        capability/multi_room_music/MRMCapabilityAgent.cpp
        capability/multi_room_music/MRMCapabilityAgentTest.cpp
        capability/notifications/NotificationIndicator.cpp
        capability/notifications/NotificationRenderer.cpp
        capability/notifications/NotificationRendererTest.cpp
        capability/notifications/NotificationsCapabilityAgent.cpp
        capability/notifications/NotificationsCapabilityAgentTest.cpp
        capability/notifications/NotificationsStorageTest.cpp
        capability/notifications/SQLiteNotificationsStorage.cpp
        capability_agents/AIP/AudioInputProcessor.cpp
        capability_agents/AIP/AudioInputProcessorTest.cpp
        capability_agents/Alexa/AlexaInterfaceCapabilityAgent.cpp
        capability_agents/Alexa/AlexaInterfaceCapabilityAgentTest.cpp
        capability_agents/Alexa/AlexaInterfaceMessageSender.cpp
        capability_agents/Alexa/AlexaInterfaceMessageSenderTest.cpp
        capability_agents/ApiGateway/ApiGatewayCapabilityAgent.cpp
        capability_agents/ApiGateway/ApiGatewayCapabilityAgentTest.cpp
        capability_agents/Equalizer/EqualizerCapabilityAgent.cpp
        capability_agents/ExternalMediaPlayer/AuthorizedSender.cpp
        capability_agents/ExternalMediaPlayer/ExternalMediaPlayer.cpp
        capability_agents/ExternalMediaPlayer/ExternalMediaPlayerTest.cpp
        capability_agents/InteractionModel/InteractionModelCapabilityAgent.cpp
        capability_agents/InteractionModel/InteractionModelCapabilityAgentTest.cpp
        capability_agents/ModeController/ModeControllerAttributeBuilder.cpp
        capability_agents/ModeController/ModeControllerCapabilityAgent.cpp
        capability_agents/ModeController/ModeControllerCapabilityAgentTest.cpp
        capability_agents/Notifications/NotificationIndicator.cpp
        capability_agents/Notifications/NotificationRenderer.cpp
        capability_agents/Notifications/NotificationRendererTest.cpp
        capability_agents/Notifications/NotificationsCapabilityAgent.cpp
        capability_agents/Notifications/NotificationsCapabilityAgentTest.cpp
        capability_agents/Notifications/NotificationsStorageTest.cpp
        capability_agents/Notifications/SQLiteNotificationsStorage.cpp
        capability_agents/PlaybackController/PlaybackCommand.cpp
        capability_agents/PlaybackController/PlaybackController.cpp
        capability_agents/PlaybackController/PlaybackControllerTest.cpp
        capability_agents/PlaybackController/PlaybackMessageRequest.cpp
        capability_agents/PlaybackController/PlaybackRouter.cpp
        capability_agents/PlaybackController/PlaybackRouterTest.cpp
        capability_agents/PowerController/PowerControllerCapabilityAgent.cpp
        capability_agents/PowerController/PowerControllerCapabilityAgentTest.cpp
        capability_agents/RangeController/RangeControllerAttributeBuilder.cpp
        capability_agents/RangeController/RangeControllerCapabilityAgent.cpp
        capability_agents/RangeController/RangeControllerCapabilityAgentTest.cpp
        capability_agents/SoftwareComponentReporter/SoftwareComponentReporterCapabilityAgent.cpp
        capability_agents/SoftwareComponentReporter/SoftwareComponentReporterCapabilityAgentTest.cpp
        capability_agents/SpeakerManager/ChannelVolumeManager.cpp
        capability_agents/SpeakerManager/ChannelVolumeManagerTest.cpp
        capability_agents/SpeakerManager/DefaultChannelVolumeFactory.cpp
        capability_agents/SpeakerManager/SpeakerManager.cpp
        capability_agents/SpeakerManager/SpeakerManagerTest.cpp
        capability_agents/SpeechSynthesizer/SpeechSynthesizer.cpp
        capability_agents/SpeechSynthesizer/SpeechSynthesizerTest.cpp
        capability_agents/System/LocaleHandler.cpp
        capability_agents/System/LocaleHandlerTest.cpp
        capability_agents/System/ReportStateHandler.cpp
        capability_agents/System/ReportStateHandlerTest.cpp
        capability_agents/System/RevokeAuthorizationHandler.cpp
        capability_agents/System/RevokeAuthorizationHandlerTest.cpp
        capability_agents/System/SoftwareInfoSender.cpp
        capability_agents/System/SoftwareInfoSendRequest.cpp
        capability_agents/System/SoftwareInfoTest.cpp
        capability_agents/System/StateReportGenerator.cpp
        capability_agents/System/StateReportGeneratorTest.cpp
        capability_agents/System/SystemCapabilityProvider.cpp
        capability_agents/System/TimeZoneHandler.cpp
        capability_agents/System/TimeZoneHandlerTest.cpp
        capability_agents/System/UserInactivityMonitor.cpp
        capability_agents/System/UserInactivityMonitorTest.cpp
        capability_agents/TemplateRuntime/TemplateRuntime.cpp
        capability_agents/TemplateRuntime/TemplateRuntimeTest.cpp
        capability_agents/ToggleController/ToggleControllerAttributeBuilder.cpp
        capability_agents/ToggleController/ToggleControllerCapabilityAgent.cpp
        capability_agents/ToggleController/ToggleControllerCapabilityAgentTest.cpp
        captions/CaptionData.cpp
        captions/CaptionDataTest.cpp
        captions/CaptionFormatAvsTest.cpp
        captions/CaptionFormatUtilsTest.cpp
        captions/CaptionFrame.cpp
        captions/CaptionFrameTest.cpp
        captions/CaptionLine.cpp
        captions/CaptionLineTest.cpp
        captions/CaptionManager.cpp
        captions/CaptionManagerTest.cpp
        captions/CaptionTimingAdapter.cpp
        captions/CaptionTimingAdapterTest.cpp
        captions/LibwebvttParserAdapter.cpp
        captions/LibwebvttParserAdapterTest.cpp
        captions/SystemClockDelay.cpp
        captions/TestTimingAdapterFactory.cpp
        captions/TextStyle.cpp
        captions/TextStyleTest.cpp
        captions/TimingAdapterFactory.cpp
        certified_sender/Common/MockCertifiedSender.cpp
        certified_sender/CertifiedSender.cpp
        certified_sender/CertifiedSenderTest.cpp
        certified_sender/MessageStorageTest.cpp
        certified_sender/SQLiteMessageStorage.cpp
        check/test/check_check_exit.c
        check/test/check_check_export_main.c
        check/test/check_check_fixture.c
        check/test/check_check_fork.c
        check/test/check_check_limit.c
        check/test/check_check_log.c
        check/test/check_check_log_internal.c
        check/test/check_check_main.c
        check/test/check_check_master.c
        check/test/check_check_msg.c
        check/test/check_check_pack.c
        check/test/check_check_selective.c
        check/test/check_check_sub.c
        check/test/check_check_tags.c
        check/test/check_list.c
        check/test/check_mem_leaks.c
        check/test/check_nofork.c
        check/test/check_nofork_teardown.c
        check/test/check_set_max_msg_size.c
        check/test/check_stress.c
        check/test/check_thread_stress.c
        check/test/ex_output.c
        check/test/test_suite.c
        check/test/tests.c
        check/alarm.c
        check/check.c
        check/check_error.c
        check/check_list.c
        check/check_log.c
        check/check_msg.c
        check/check_pack.c
        check/check_print.c
        check/check_run.c
        check/check_str.c
        check/clock_gettime.c
        check/fpclassify.c
        check/getline.c
        check/gettimeofday.c
        check/libcompat.c
        check/localtime_r.c
        check/malloc.c
        check/pthread_mutex.c
        check/realloc.c
        check/snprintf.c
        check/strdup.c
        check/strsignal.c
        check/timer_create.c
        check/timer_delete.c
        check/timer_settime.c
        configuration/ConfigurationNode.cpp
        context_manager/ContextManager.cpp
        context_manager/ContextManagerTest.cpp
        core/CoreComponent.cpp
        curl/vauth/cleartext.c
        curl/vauth/cram.c
        curl/vauth/digest.c
        curl/vauth/digest_sspi.c
        curl/vauth/krb5_gssapi.c
        curl/vauth/krb5_sspi.c
        curl/vauth/ntlm.c
        curl/vauth/ntlm_sspi.c
        curl/vauth/oauth2.c
        curl/vauth/spnego_gssapi.c
        curl/vauth/spnego_sspi.c
        curl/vauth/vauth.c
        curl/vquic/ngtcp2.c
        curl/vquic/quiche.c
        curl/vquic/vquic.c
        curl/vssh/libssh.c
        curl/vssh/libssh2.c
        curl/vssh/wolfssh.c
        curl/vtls/bearssl.c
        curl/vtls/gskit.c
        curl/vtls/gtls.c
        curl/vtls/keylog.c
        curl/vtls/mbedtls.c
        curl/vtls/mbedtls_threadlock.c
        curl/vtls/mesalink.c
        curl/vtls/nss.c
        curl/vtls/openssl.c
        curl/vtls/schannel.c
        curl/vtls/schannel_verify.c
        curl/vtls/sectransp.c
        curl/vtls/vtls.c
        curl/vtls/wolfssl.c
        curl/altsvc.c
        curl/amigaos.c
        curl/asyn-ares.c
        curl/asyn-thread.c
        curl/base64.c
        curl/conncache.c
        curl/connect.c
        curl/content_encoding.c
        curl/cookie.c
        curl/curl_addrinfo.c
        curl/curl_ctype.c
        curl/curl_endian.c
        curl/curl_fnmatch.c
        curl/curl_get_line.c
        curl/curl_gethostname.c
        curl/curl_gssapi.c
        curl/curl_memrchr.c
        curl/curl_multibyte.c
        curl/curl_ntlm_core.c
        curl/curl_ntlm_wb.c
        curl/curl_path.c
        curl/curl_range.c
        curl/curl_rtmp.c
        curl/curl_sasl.c
        curl/curl_sspi.c
        curl/curl_threads.c
        curl/dict.c
        curl/doh.c
        curl/dotdot.c
        curl/dynbuf.c
        curl/escape.c
        curl/file.c
        curl/fileinfo.c
        curl/formdata.c
        curl/ftp.c
        curl/ftplistparser.c
        curl/getenv.c
        curl/getinfo.c
        curl/gopher.c
        curl/hash.c
        curl/hmac.c
        curl/hostasyn.c
        curl/hostcheck.c
        curl/hostip.c
        curl/hostip4.c
        curl/hostip6.c
        curl/hostsyn.c
        curl/http.c
        curl/http2.c
        curl/http_chunks.c
        curl/http_digest.c
        curl/http_negotiate.c
        curl/http_ntlm.c
        curl/http_proxy.c
        curl/idn_win32.c
        curl/if2ip.c
        curl/imap.c
        curl/inet_ntop.c
        curl/inet_pton.c
        curl/krb5.c
        curl/ldap.c
        curl/llist.c
        curl/md4.c
        curl/md5.c
        curl/memdebug.c
        curl/mime.c
        curl/mqtt.c
        curl/netrc.c
        curl/non-ascii.c
        curl/nonblock.c
        curl/nwlib.c
        curl/nwos.c
        curl/openldap.c
        curl/parsedate.c
        curl/pingpong.c
        curl/pop3.c
        curl/progress.c
        curl/psl.c
        curl/rand.c
        curl/rename.c
        curl/rtsp.c
        curl/security.c
        curl/select.c
        curl/sendf.c
        curl/setopt.c
        curl/sha256.c
        curl/share.c
        curl/slist.c
        curl/slist_wc.c
        curl/smb.c
        curl/smtp.c
        curl/socketpair.c
        curl/socks.c
        curl/socks_gssapi.c
        curl/socks_sspi.c
        curl/speedcheck.c
        curl/splay.c
        curl/strcase.c
        curl/strdup.c
        curl/strerror.c
        curl/strtok.c
        curl/strtoofft.c
        curl/system_win32.c
        curl/telnet.c
        curl/tftp.c
        curl/timeval.c
        curl/tool_binmode.c
        curl/tool_bname.c
        curl/tool_cb_dbg.c
        curl/tool_cb_hdr.c
        curl/tool_cb_prg.c
        curl/tool_cb_rea.c
        curl/tool_cb_see.c
        curl/tool_cb_wrt.c
        curl/tool_cfgable.c
        curl/tool_convert.c
        curl/tool_dirhie.c
        curl/tool_doswin.c
        curl/tool_easysrc.c
        curl/tool_filetime.c
        curl/tool_formparse.c
        curl/tool_getparam.c
        curl/tool_getpass.c
        curl/tool_help.c
        curl/tool_helpers.c
        curl/tool_homedir.c
        curl/tool_hugehelp.c
        curl/tool_libinfo.c
        curl/tool_main.c
        curl/tool_metalink.c
        curl/tool_msgs.c
        curl/tool_operate.c
        curl/tool_operhlp.c
        curl/tool_panykey.c
        curl/tool_paramhlp.c
        curl/tool_parsecfg.c
        curl/tool_progress.c
        curl/tool_setopt.c
        curl/tool_sleep.c
        curl/tool_strdup.c
        curl/tool_urlglob.c
        curl/tool_util.c
        curl/tool_vms.c
        curl/tool_writeout.c
        curl/tool_writeout_json.c
        curl/tool_xattr.c
        curl/transfer.c
        curl/url.c
        curl/urlapi.c
        curl/version.c
        curl/warnless.c
        curl/wildcard.c
        curl/x509asn1.c
        dbus/bus/activation.c
        dbus/bus/activation-helper.c
        dbus/bus/activation-helper-bin.c
        dbus/bus/apparmor.c
        dbus/bus/audit.c
        dbus/bus/bus.c
        dbus/bus/config-loader-expat.c
        dbus/bus/config-parser.c
        dbus/bus/config-parser-common.c
        dbus/bus/config-parser-trivial.c
        dbus/bus/connection.c
        dbus/bus/containers.c
        dbus/bus/desktop-file.c
        dbus/bus/dir-watch-default.c
        dbus/bus/dir-watch-inotify.c
        dbus/bus/dir-watch-kqueue.c
        dbus/bus/dispatch.c
        dbus/bus/driver.c
        dbus/bus/expirelist.c
        dbus/bus/main.c
        dbus/bus/policy.c
        dbus/bus/selinux.c
        dbus/bus/services.c
        dbus/bus/signals.c
        dbus/bus/stats.c
        dbus/bus/test.c
        dbus/bus/test-launch-helper.c
        dbus/bus/test-main.c
        dbus/bus/test-system.c
        dbus/bus/utils.c
        dbus/test/internals/printf.c
        dbus/test/internals/refs.c
        dbus/test/internals/server-oom.c
        dbus/test/internals/syslog.c
        dbus/test/internals/variant.c
        dbus/test/name-test/test-autolaunch.c
        dbus/test/name-test/test-ids.c
        dbus/test/name-test/test-pending-call-disconnected.c
        dbus/test/name-test/test-pending-call-dispatch.c
        dbus/test/name-test/test-pending-call-timeout.c
        dbus/test/name-test/test-privserver.c
        dbus/test/name-test/test-privserver-client.c
        dbus/test/name-test/test-shutdown.c
        dbus/test/name-test/test-threads-init.c
        dbus/test/containers.c
        dbus/test/corrupt.c
        dbus/test/dbus-daemon.c
        dbus/test/dbus-daemon-eavesdrop.c
        dbus/test/fdpass.c
        dbus/test/header-fields.c
        dbus/test/loopback.c
        dbus/test/manual-authz.c
        dbus/test/manual-backtrace.c
        dbus/test/manual-dir-iter.c
        dbus/test/manual-paths.c
        dbus/test/manual-tcp.c
        dbus/test/marshal.c
        dbus/test/message.c
        dbus/test/monitor.c
        dbus/test/relay.c
        dbus/test/sd-activation.c
        dbus/test/shell-test.c
        dbus/test/spawn-test.c
        dbus/test/syntax.c
        dbus/test/test-exit.c
        dbus/test/test-names.c
        dbus/test/test-segfault.c
        dbus/test/test-service.c
        dbus/test/test-shell-service.c
        dbus/test/test-sleep-forever.c
        dbus/test/test-utils.c
        dbus/test/test-utils-glib.c
        dbus/test/uid-permissions.c
        dbus/dbus-address.c
        dbus/dbus-asv-util.c
        dbus/dbus-auth.c
        dbus/dbus-auth-script.c
        dbus/dbus-auth-util.c
        dbus/dbus-bus.c
        dbus/dbus-connection.c
        dbus/dbus-credentials.c
        dbus/dbus-credentials-util.c
        dbus/dbus-dataslot.c
        dbus/dbus-errors.c
        dbus/dbus-file.c
        dbus/dbus-file-unix.c
        dbus/dbus-file-win.c
        dbus/dbus-hash.c
        dbus/dbus-init-win.cpp
        dbus/dbus-internals.c
        dbus/dbus-keyring.c
        dbus/dbus-list.c
        dbus/dbus-mainloop.c
        dbus/dbus-marshal-basic.c
        dbus/dbus-marshal-byteswap.c
        dbus/dbus-marshal-byteswap-util.c
        dbus/dbus-marshal-header.c
        dbus/dbus-marshal-recursive.c
        dbus/dbus-marshal-recursive-util.c
        dbus/dbus-marshal-validate.c
        dbus/dbus-marshal-validate-util.c
        dbus/dbus-memory.c
        dbus/dbus-mempool.c
        dbus/dbus-message.c
        dbus/dbus-message-factory.c
        dbus/dbus-message-util.c
        dbus/dbus-misc.c
        dbus/dbus-nonce.c
        dbus/dbus-object-tree.c
        dbus/dbus-pending-call.c
        dbus/dbus-pipe.c
        dbus/dbus-pipe-unix.c
        dbus/dbus-pipe-win.c
        dbus/dbus-resources.c
        dbus/dbus-server.c
        dbus/dbus-server-debug-pipe.c
        dbus/dbus-server-launchd.c
        dbus/dbus-server-socket.c
        dbus/dbus-server-unix.c
        dbus/dbus-server-win.c
        dbus/dbus-sha.c
        dbus/dbus-shell.c
        dbus/dbus-signature.c
        dbus/dbus-socket-set.c
        dbus/dbus-socket-set-epoll.c
        dbus/dbus-socket-set-poll.c
        dbus/dbus-spawn-test.c
        dbus/dbus-spawn-unix.c
        dbus/dbus-spawn-win.c
        dbus/dbus-string.c
        dbus/dbus-string-util.c
        dbus/dbus-syntax.c
        dbus/dbus-sysdeps.c
        dbus/dbus-sysdeps-pthread.c
        dbus/dbus-sysdeps-thread-win.c
        dbus/dbus-sysdeps-unix.c
        dbus/dbus-sysdeps-util.c
        dbus/dbus-sysdeps-util-unix.c
        dbus/dbus-sysdeps-util-win.c
        dbus/dbus-sysdeps-win.c
        dbus/dbus-sysdeps-wince-glue.c
        dbus/dbus-test.c
        dbus/dbus-test-main.c
        dbus/dbus-test-tap.c
        dbus/dbus-threads.c
        dbus/dbus-timeout.c
        dbus/dbus-transport.c
        dbus/dbus-transport-socket.c
        dbus/dbus-transport-unix.c
        dbus/dbus-transport-win.c
        dbus/dbus-userdb.c
        dbus/dbus-userdb-util.c
        dbus/dbus-uuidgen.c
        dbus/dbus-watch.c
        diagnostics/AudioInjectorMicrophone.cpp
        diagnostics/DevicePropertyAggregator.cpp
        diagnostics/DevicePropertyAggregatorTest.cpp
        diagnostics/DeviceProtocolTracer.cpp
        diagnostics/DeviceProtocolTracerTest.cpp
        diagnostics/DiagnosticsUtils.cpp
        diagnostics/FileBasedAudioInjector.cpp
        endpoints/Endpoint.cpp
        endpoints/EndpointAttributeValidation.cpp
        endpoints/EndpointBuilder.cpp
        endpoints/EndpointRegistrationManager.cpp
        equalizer_implementations/EqualizerController.cpp
        equalizer_implementations/EqualizerControllerTest.cpp
        equalizer_implementations/EqualizerLinearBandMapper.cpp
        equalizer_implementations/EqualizerLinearBandMapperTest.cpp
        equalizer_implementations/EqualizerUtils.cpp
        equalizer_implementations/InMemoryEqualizerConfiguration.cpp
        equalizer_implementations/InMemoryEqualizerConfigurationTest.cpp
        equalizer_implementations/MiscDBEqualizerStorage.cpp
        equalizer_implementations/MiscDBEqualizerStorageTest.cpp
        equalizer_implementations/SDKConfigEqualizerConfiguration.cpp
        equalizer_implementations/SDKConfigEqualizerConfigurationTest.cpp
        expat/benchmark/benchmark.c
        expat/fuzz/xml_parse_fuzzer.c
        expat/fuzz/xml_parsebuffer_fuzzer.c
        expat/gennmtab/gennmtab.c
        expat/xmlwf/codepage.c
        expat/xmlwf/ct.c
        expat/xmlwf/readfilemap.c
        expat/xmlwf/unixfilemap.c
        expat/xmlwf/win32filemap.c
        expat/xmlwf/xmlfile.c
        expat/xmlwf/xmlwf.c
        expat/chardata.c
        expat/memcheck.c
        expat/minicheck.c
        expat/runtests.c
        expat/runtestspp.cpp
        expat/structdata.c
        expat/xmlparse.c
        expat/xmlrole.c
        expat/xmltok.c
        expat/xmltok_impl.c
        expat/xmltok_ns.c
        gio/fam/fam-helper.c
        gio/fam/fam-module.c
        gio/fam/gfamdirectorymonitor.c
        gio/fam/gfamfilemonitor.c
        gio/fen/fen-dump.c
        gio/fen/fen-helper.c
        gio/fen/fen-kernel.c
        gio/fen/fen-node.c
        gio/fen/gfendirectorymonitor.c
        gio/fen/gfenfilemonitor.c
        gio/gvdb/gvdb-builder.c
        gio/gvdb/gvdb-reader.c
        gio/inotify/ginotifydirectorymonitor.c
        gio/inotify/ginotifyfilemonitor.c
        gio/inotify/inotify-diag.c
        gio/inotify/inotify-helper.c
        gio/inotify/inotify-kernel.c
        gio/inotify/inotify-missing.c
        gio/inotify/inotify-path.c
        gio/inotify/inotify-sub.c
        gio/libasyncns/asyncns.c
        gio/tests/actions.c
        gio/tests/appinfo.c
        gio/tests/appinfo-test.c
        gio/tests/async-close-output-stream.c
        gio/tests/buffered-input-stream.c
        gio/tests/buffered-output-stream.c
        gio/tests/contenttype.c
        gio/tests/contexts.c
        gio/tests/converter-stream.c
        gio/tests/data-input-stream.c
        gio/tests/data-output-stream.c
        gio/tests/desktop-app-info.c
        gio/tests/echo-server.c
        gio/tests/file.c
        gio/tests/filter-cat.c
        gio/tests/filter-streams.c
        gio/tests/g-file.c
        gio/tests/g-file-info.c
        gio/tests/g-icon.c
        gio/tests/gapplication.c
        gio/tests/gapplication-example-actions.c
        gio/tests/gapplication-example-cmdline.c
        gio/tests/gapplication-example-cmdline2.c
        gio/tests/gapplication-example-open.c
        gio/tests/gdbus-addresses.c
        gio/tests/gdbus-bz627724.c
        gio/tests/gdbus-connection.c
        gio/tests/gdbus-connection-flush-helper.c
        gio/tests/gdbus-connection-slow.c
        gio/tests/gdbus-error.c
        gio/tests/gdbus-example-export.c
        gio/tests/gdbus-example-own-name.c
        gio/tests/gdbus-example-peer.c
        gio/tests/gdbus-example-proxy-subclass.c
        gio/tests/gdbus-example-server.c
        gio/tests/gdbus-example-subtree.c
        gio/tests/gdbus-example-unix-fd-client.c
        gio/tests/gdbus-example-watch-name.c
        gio/tests/gdbus-example-watch-proxy.c
        gio/tests/gdbus-exit-on-close.c
        gio/tests/gdbus-export.c
        gio/tests/gdbus-introspection.c
        gio/tests/gdbus-message.c
        gio/tests/gdbus-names.c
        gio/tests/gdbus-non-socket.c
        gio/tests/gdbus-peer.c
        gio/tests/gdbus-proxy.c
        gio/tests/gdbus-proxy-well-known-name.c
        gio/tests/gdbus-serialization.c
        gio/tests/gdbus-sessionbus.c
        gio/tests/gdbus-tests.c
        gio/tests/gdbus-threading.c
        gio/tests/gschema-compile.c
        gio/tests/gsettings.c
        gio/tests/httpd.c
        gio/tests/io-stream.c
        gio/tests/live-g-file.c
        gio/tests/memory-input-stream.c
        gio/tests/memory-output-stream.c
        gio/tests/network-address.c
        gio/tests/pollable.c
        gio/tests/proxy.c
        gio/tests/readwrite.c
        gio/tests/resolver.c
        gio/tests/send-data.c
        gio/tests/simple-async-result.c
        gio/tests/sleepy-stream.c
        gio/tests/socket.c
        gio/tests/socket-client.c
        gio/tests/socket-common.c
        gio/tests/socket-server.c
        gio/tests/srvtarget.c
        gio/tests/unix-fd.c
        gio/tests/unix-streams.c
        gio/tests/volumemonitor.c
        gio/tests/win32-streams.c
        gio/win32/gwin32directorymonitor.c
        gio/win32/gwinhttpfile.c
        gio/win32/gwinhttpfileinputstream.c
        gio/win32/gwinhttpfileoutputstream.c
        gio/win32/gwinhttpfile.c
        gio/win32/gwinhttpvfs.c
        gio/xdgmime/xdgmime.c
        gio/xdgmime/xdgmimealias.c
        gio/xdgmime/xdgmimecache.c
        gio/xdgmime/xdgmimeglob.c
        gio/xdgmime/xdgmimeicon.c
        gio/xdgmime/xdgmimeint.c
        gio/xdgmime/xdgmimemagic.c
        gio/xdgmime/xdgmimeparent.c
        gio/gaction.c
        gio/gactiongroup.c
        gio/gappinfo.c
        gio/gapplication.c
        gio/gapplicationcommandline.c
        gio/gapplicationimpl-dbus.c
        gio/gasynchelper.c
        gio/gasyncinitable.c
        gio/gasyncresult.c
        gio/gbufferedinputstream.c
        gio/gbufferedoutputstream.c
        gio/gcancellable.c
        gio/gcharsetconverter.c
        gio/gcontenttype.c
        gio/gconverter.c
        gio/gconverterinputstream.c
        gio/gconverteroutputstream.c
        gio/gcredentials.c
        gio/gdatainputstream.c
        gio/gdataoutputstream.c
        gio/gdbus-tool.c
        gio/gdbusaddress.c
        gio/gdbusauth.c
        gio/gdbusauthmechanism.c
        gio/gdbusauthmechanismanon.c
        gio/gdbusauthmechanismexternal.c
        gio/gdbusauthmechanismsha1.c
        gio/gdbusauthobserver.c
        gio/gdbusconnection.c
        gio/gdbuserror.c
        gio/gdbusintrospection.c
        gio/gdbusmessage.c
        gio/gdbusmethodinvocation.c
        gio/gdbusnameowning.c
        gio/gdbusnamewatching.c
        gio/gdbusprivate.c
        gio/gdbusproxy.c
        gio/gdbusserver.c
        gio/gdbusutils.c
        gio/gdelayedsettingsbackend.c
        gio/gdesktopappinfo.c
        gio/gdrive.c
        gio/gdummyfile.c
        gio/gdummyproxyresolver.c
        gio/gdummytlsbackend.c
        gio/gemblem.c
        gio/gemblemedicon.c
        gio/gfile.c
        gio/gfileattribute.c
        gio/gfiledescriptorbased.c
        gio/gfileenumerator.c
        gio/gfileicon.c
        gio/gfileinfo.c
        gio/gfileinputstream.c
        gio/gfileiostream.c
        gio/gfilemonitor.c
        gio/gfilenamecompleter.c
        gio/gfileoutputstream.c
        gio/gfilterinputstream.c
        gio/gfilteroutputstream.c
        gio/gicon.c
        gio/ginetaddress.c
        gio/ginetsocketaddress.c
        gio/ginitable.c
        gio/ginputstream.c
        gio/gio-querymodules.c
        gio/gioenumtypes.c
        gio/gioerror.c
        gio/giomodule.c
        gio/gioscheduler.c
        gio/giostream.c
        gio/gkeyfilesettingsbackend.c
        gio/glib-compile-schemas.c
        gio/gloadableicon.c
        gio/glocaldirectorymonitor.c
        gio/glocalfile.c
        gio/glocalfileenumerator.c
        gio/glocalfileinfo.c
        gio/glocalfileinputstream.c
        gio/glocalfileiostream.c
        gio/glocalfilemonitor.c
        gio/glocalfileoutputstream.c
        gio/glocalvfs.c
        gio/gmemoryinputstream.c
        gio/gmemoryoutputstream.c
        gio/gmemorysettingsbackend.c
        gio/gmount.c
        gio/gmountoperation.c
        gio/gnativevolumemonitor.c
        gio/gnetworkaddress.c
        gio/gnetworkservice.c
        gio/gnullsettingsbackend.c
        gio/goutputstream.c
        gio/gpermission.c
        gio/gpollableinputstream.c
        gio/gpollableoutputstream.c
        gio/gpollfilemonitor.c
        gio/gproxy.c
        gio/gproxyaddress.c
        gio/gproxyaddressenumerator.c
        gio/gproxyresolver.c
        gio/gregistrysettingsbackend.c
        gio/gresolver.c
        gio/gseekable.c
        gio/gsettings.c
        gio/gsettings-mapping.c
        gio/gsettings-tool.c
        gio/gsettingsbackend.c
        gio/gsettingsschema.c
        gio/gsimpleaction.c
        gio/gsimpleactiongroup.c
        gio/gsimpleasyncresult.c
        gio/gsimplepermission.c
        gio/gsocket.c
        gio/gsocketaddress.c
        gio/gsocketaddressenumerator.c
        gio/gsocketclient.c
        gio/gsocketconnectable.c
        gio/gsocketconnection.c
        gio/gsocketcontrolmessage.c
        gio/gsocketinputstream.c
        gio/gsocketlistener.c
        gio/gsocketoutputstream.c
        gio/gsocketservice.c
        gio/gsocks4aproxy.c
        gio/gsocks4proxy.c
        gio/gsocks5proxy.c
        gio/gsrvtarget.c
        gio/gtask.c
        gio/gtcpconnection.c
        gio/gtcpwrapperconnection.c
        gio/gthemedicon.c
        gio/gthreadedresolver.c
        gio/gthreadedsocketservice.c
        gio/gtlsbackend.c
        gio/gtlscertificate.c
        gio/gtlsclientconnection.c
        gio/gtlsconnection.c
        gio/gtlsserverconnection.c
        gio/gunionvolumemonitor.c
        gio/gunixconnection.c
        gio/gunixcredentialsmessage.c
        gio/gunixfdlist.c
        gio/gunixfdmessage.c
        gio/gunixinputstream.c
        gio/gunixmount.c
        gio/gunixmounts.c
        gio/gunixoutputstream.c
        gio/gunixresolver.c
        gio/gunixsocketaddress.c
        gio/gunixvolume.c
        gio/gunixvolumemonitor.c
        gio/gvfs.c
        gio/gvolume.c
        gio/gvolumemonitor.c
        gio/gwin32appinfo.c
        gio/gwin32inputstream.c
        gio/gwin32mount.c
        gio/gwin32outputstream.c
        gio/gwin32resolver.c
        gio/gwin32volumemonitor.c
        gio/gzlibcompressor.c
        gio/gzlibdecompressor.c
        gio/strinfo.c
        glib/glib-mirroring-tab/gen-mirroring-tab.c
        glib/glib-mirroring-tab/packtab.c
        glib/gnulib/asnprintf.c
        glib/gnulib/printf.c
        glib/gnulib/printf-args.c
        glib/gnulib/printf-parse.c
        glib/gnulib/vasnprintf.c
        glib/libcharset/localcharset.c
        glib/pcre/pcre_chartables.c
        glib/pcre/pcre_compile.c
        glib/pcre/pcre_config.c
        glib/pcre/pcre_dfa_exec.c
        glib/pcre/pcre_exec.c
        glib/pcre/pcre_fullinfo.c
        glib/pcre/pcre_get.c
        glib/pcre/pcre_globals.c
        glib/pcre/pcre_newline.c
        glib/pcre/pcre_ord2utf8.c
        glib/pcre/pcre_study.c
        glib/pcre/pcre_tables.c
        glib/pcre/pcre_try_flipped.c
        glib/pcre/pcre_ucp_searchfuncs.c
        glib/pcre/pcre_xclass.c
        glib/tests/array-test.c
        glib/tests/base64.c
        glib/tests/bookmarkfile.c
        glib/tests/checksum.c
        glib/tests/collate.c
        glib/tests/convert.c
        glib/tests/dataset.c
        glib/tests/date.c
        glib/tests/dir.c
        glib/tests/environment.c
        glib/tests/error.c
        glib/tests/fileutils.c
        glib/tests/gdatetime.c
        glib/tests/gvariant.c
        glib/tests/hash.c
        glib/tests/hostutils.c
        glib/tests/keyfile.c
        glib/tests/list.c
        glib/tests/logging.c
        glib/tests/mappedfile.c
        glib/tests/markup-collect.c
        glib/tests/markup-escape.c
        glib/tests/markup-parse.c
        glib/tests/markup-subparser.c
        glib/tests/mem-overflow.c
        glib/tests/node.c
        glib/tests/option-context.c
        glib/tests/pattern.c
        glib/tests/printf.c
        glib/tests/protocol.c
        glib/tests/queue.c
        glib/tests/rand.c
        glib/tests/regex.c
        glib/tests/scannerapi.c
        glib/tests/sequence.c
        glib/tests/shell.c
        glib/tests/slist.c
        glib/tests/sort.c
        glib/tests/strfuncs.c
        glib/tests/string.c
        glib/tests/testing.c
        glib/tests/timeout.c
        glib/tests/tree.c
        glib/tests/uri.c
        glib/tests/utf8-misc.c
        glib/tests/utf8-pointer.c
        glib/tests/utf8-validate.c
        glib/tests/utils.c
        glib/update-pcre/pcre_ucp_searchfuncs.c
        glib/update-pcre/pcre_valid_utf8.c
        glib/garray.c
        glib/gasyncqueue.c
        glib/gatomic.c
        glib/gatomic-gcc.c
        glib/gbacktrace.c
        glib/gbase64.c
        glib/gbitlock.c
        glib/gbookmarkfile.c
        glib/gbuffer.c
        glib/gcache.c
        glib/gchecksum.c
        glib/gcompletion.c
        glib/gconvert.c
        glib/gdataset.c
        glib/gdate.c
        glib/gdatetime.c
        glib/gdir.c
        glib/gerror.c
        glib/gfileutils.c
        glib/ghash.c
        glib/ghook.c
        glib/ghostutils.c
        glib/giochannel.c
        glib/giounix.c
        glib/giowin32.c
        glib/gkeyfile.c
        glib/glib-init.c
        glib/glib-private.c
        glib/glib-unix.c
        glib/glist.c
        glib/gmain.c
        glib/gmappedfile.c
        glib/gmarkup.c
        glib/gmem.c
        glib/gmessages.c
        glib/gnode.c
        glib/goption.c
        glib/gpattern.c
        glib/gpoll.c
        glib/gprimes.c
        glib/gprintf.c
        glib/gqsort.c
        glib/gqueue.c
        glib/grand.c
        glib/gregex.c
        glib/grel.c
        glib/gscanner.c
        glib/gsequence.c
        glib/gshell.c
        glib/gslice.c
        glib/gslist.c
        glib/gspawn.c
        glib/gspawn-win32.c
        glib/gspawn-win32-helper.c
        glib/gstdio.c
        glib/gstrfuncs.c
        glib/gstring.c
        glib/gtester.c
        glib/gtestutils.c
        glib/gthread.c
        glib/gthreadpool.c
        glib/gtimer.c
        glib/gtimezone.c
        glib/gtree.c
        glib/gunibreak.c
        glib/gunicollate.c
        glib/gunidecomp.c
        glib/guniprop.c
        glib/gurifuncs.c
        glib/gutf8.c
        glib/gutils.c
        glib/gvariant.c
        glib/gvariant-core.c
        glib/gvariant-parser.c
        glib/gvariant-serialiser.c
        glib/gvarianttype.c
        glib/gvarianttypeinfo.c
        glib/gwakeup.c
        glib/gwin32.c
        glib/win_iconv.c
        gmock/gmock.cc
        gmock/gmock-actions_test.cc
        gmock/gmock-all.cc
        gmock/gmock-cardinalities.cc
        gmock/gmock-cardinalities_test.cc
        gmock/gmock-generated-actions_test.cc
        gmock/gmock-generated-function-mockers_test.cc
        gmock/gmock-generated-internal-utils_test.cc
        gmock/gmock-generated-matchers_test.cc
        gmock/gmock-internal-utils.cc
        gmock/gmock-internal-utils_test.cc
        gmock/gmock-matchers.cc
        gmock/gmock-matchers_test.cc
        gmock/gmock-more-actions_test.cc
        gmock/gmock-nice-strict_test.cc
        gmock/gmock-port_test.cc
        gmock/gmock-spec-builders.cc
        gmock/gmock-spec-builders_test.cc
        gmock/gmock_all_test.cc
        gmock/gmock_ex_test.cc
        gmock/gmock_leak_test_.cc
        gmock/gmock_link2_test.cc
        gmock/gmock_link_test.cc
        gmock/gmock_main.cc
        gmock/gmock_output_test_.cc
        gmock/gmock_stress_test.cc
        gmock/gmock_test.cc
        gmodule/gmodule.c
        gmodule/gmodule-ar.c
        gmodule/gmodule-beos.c
        gmodule/gmodule-dl.c
        gmodule/gmodule-dld.c
        gmodule/gmodule-dyld.c
        gmodule/gmodule-os2.c
        gmodule/gmodule-win32.c
        gobject/tests/binding.c
        gobject/tests/boxed.c
        gobject/tests/dynamictests.c
        gobject/tests/enums.c
        gobject/tests/ifaceproperties.c
        gobject/tests/param.c
        gobject/tests/properties.c
        gobject/tests/reference.c
        gobject/tests/threadtests.c
        gobject/gatomicarray.c
        gobject/gbinding.c
        gobject/gboxed.c
        gobject/gclosure.c
        gobject/genums.c
        gobject/glib-genmarshal.c
        gobject/gmarshal.c
        gobject/gobject.c
        gobject/gobject-query.c
        gobject/gobjectnotifyqueue.c
        gobject/gparam.c
        gobject/gparamspecs.c
        gobject/gsignal.c
        gobject/gsourceclosure.c
        gobject/gtype.c
        gobject/gtypemodule.c
        gobject/gtypeplugin.c
        gobject/gvalue.c
        gobject/gvaluearray.c
        gobject/gvaluetransform.c
        gobject/gvaluetypes.c
        gobject/testgobject.c
        gstreamer/ext/alsa/gstalsa.c
        gstreamer/ext/alsa/gstalsadeviceprovider.c
        gstreamer/ext/alsa/gstalsaelement.c
        gstreamer/ext/alsa/gstalsamidisrc.c
        gstreamer/ext/alsa/gstalsaplugin.c
        gstreamer/ext/alsa/gstalsasink.c
        gstreamer/ext/alsa/gstalsasrc.c
        gstreamer/ext/cdparanoia/gstcdparanoiasrc.c
        gstreamer/ext/gl/effects/gstgleffectblur.c
        gstreamer/ext/gl/effects/gstgleffectbulge.c
        gstreamer/ext/gl/effects/gstgleffectfisheye.c
        gstreamer/ext/gl/effects/gstgleffectglow.c
        gstreamer/ext/gl/effects/gstgleffectidentity.c
        gstreamer/ext/gl/effects/gstgleffectlaplacian.c
        gstreamer/ext/gl/effects/gstgleffectlumatocurve.c
        gstreamer/ext/gl/effects/gstgleffectmirror.c
        gstreamer/ext/gl/effects/gstgleffectrgbtocurve.c
        gstreamer/ext/gl/effects/gstgleffectscurves.c
        gstreamer/ext/gl/effects/gstgleffectsin.c
        gstreamer/ext/gl/effects/gstgleffectsobel.c
        gstreamer/ext/gl/effects/gstgleffectsquare.c
        gstreamer/ext/gl/effects/gstgleffectsqueeze.c
        gstreamer/ext/gl/effects/gstgleffectssources.c
        gstreamer/ext/gl/effects/gstgleffectstretch.c
        gstreamer/ext/gl/effects/gstgleffecttunnel.c
        gstreamer/ext/gl/effects/gstgleffecttwirl.c
        gstreamer/ext/gl/effects/gstgleffectxray.c
        gstreamer/ext/gl/caopengllayersink.m
        gstreamer/ext/gl/gltestsrc.c
        gstreamer/ext/gl/gstglalpha.c
        gstreamer/ext/gl/gstglbasemixer.c
        gstreamer/ext/gl/gstglbumper.c
        gstreamer/ext/gl/gstglcolorbalance.c
        gstreamer/ext/gl/gstglcolorconvertelement.c
        gstreamer/ext/gl/gstglcolorscale.c
        gstreamer/ext/gl/gstgldeinterlace.c
        gstreamer/ext/gl/gstgldifferencematte.c
        gstreamer/ext/gl/gstgldownloadelement.c
        gstreamer/ext/gl/gstgleffects.c
        gstreamer/ext/gl/gstglelement.c
        gstreamer/ext/gl/gstglfilterapp.c
        gstreamer/ext/gl/gstglfilterbin.c
        gstreamer/ext/gl/gstglfiltercube.c
        gstreamer/ext/gl/gstglfilterglass.c
        gstreamer/ext/gl/gstglfilterreflectedscreen.c
        gstreamer/ext/gl/gstglfiltershader.c
        gstreamer/ext/gl/gstglimagesink.c
        gstreamer/ext/gl/gstglmixer.c
        gstreamer/ext/gl/gstglmixerbin.c
        gstreamer/ext/gl/gstglmosaic.c
        gstreamer/ext/gl/gstgloverlay.c
        gstreamer/ext/gl/gstgloverlaycompositorelement.c
        gstreamer/ext/gl/gstglsinkbin.c
        gstreamer/ext/gl/gstglsrcbin.c
        gstreamer/ext/gl/gstglstereomix.c
        gstreamer/ext/gl/gstglstereosplit.c
        gstreamer/ext/gl/gstgltestsrc.c
        gstreamer/ext/gl/gstgltransformation.c
        gstreamer/ext/gl/gstgluploadelement.c
        gstreamer/ext/gl/gstglutils.c
        gstreamer/ext/gl/gstglvideoflip.c
        gstreamer/ext/gl/gstglvideomixer.c
        gstreamer/ext/gl/gstglviewconvert.c
        gstreamer/ext/gl/gstopengl.c
        gstreamer/ext/libvisual/plugin.c
        gstreamer/ext/libvisual/visual.c
        gstreamer/ext/ogg/dirac_parse.c
        gstreamer/ext/ogg/gstoggaviparse.c
        gstreamer/ext/ogg/gstoggdemux.c
        gstreamer/ext/ogg/gstoggmux.c
        gstreamer/ext/ogg/gstoggparse.c
        gstreamer/ext/ogg/gstoggplugin.c
        gstreamer/ext/ogg/gstoggstream.c
        gstreamer/ext/ogg/gstogmparse.c
        gstreamer/ext/ogg/vorbis_parse.c
        gstreamer/ext/opus/gstopus.c
        gstreamer/ext/opus/gstopuscommon.c
        gstreamer/ext/opus/gstopusdec.c
        gstreamer/ext/opus/gstopuselement.c
        gstreamer/ext/opus/gstopusenc.c
        gstreamer/ext/opus/gstopusheader.c
        gstreamer/ext/pango/gstbasetextoverlay.c
        gstreamer/ext/pango/gstclockoverlay.c
        gstreamer/ext/pango/gstpangoelement.c
        gstreamer/ext/pango/gstpangoplugin.c
        gstreamer/ext/pango/gsttextoverlay.c
        gstreamer/ext/pango/gsttextrender.c
        gstreamer/ext/pango/gsttimeoverlay.c
        gstreamer/ext/theora/gsttheora.c
        gstreamer/ext/theora/gsttheoradec.c
        gstreamer/ext/theora/gsttheoraenc.c
        gstreamer/ext/theora/gsttheoraparse.c
        gstreamer/ext/vorbis/gstvorbisdec.c
        gstreamer/ext/vorbis/gstvorbis.c
        gstreamer/ext/vorbis/gstvorbiscommon.c
        gstreamer/ext/vorbis/gstvorbisdec.c
        gstreamer/ext/vorbis/gstvorbisdeclib.c
        gstreamer/ext/vorbis/gstvorbiselement.c
        gstreamer/ext/vorbis/gstvorbisenc.c
        gstreamer/ext/vorbis/gstvorbisparse.c
        gstreamer/ext/vorbis/gstvorbistag.c
        gstreamer/gst/adder/gstadder.c
        gstreamer/gst/adder/gstadderorc-dist.c
        gstreamer/gst/app/gstapp.c
        gstreamer/gst/app/gstappsink.c
        gstreamer/gst/app/gstappsrc.c
        gstreamer/gst/audioconvert/gstaudioconvert.c
        gstreamer/gst/audioconvert/plugin.c
        gstreamer/gst/audiomixer/gstaudiointerleave.c
        gstreamer/gst/audiomixer/gstaudiomixer.c
        gstreamer/gst/audiomixer/gstaudiomixerelement.c
        gstreamer/gst/audiomixer/gstaudiomixerorc-dist.c
        gstreamer/gst/audiomixer/gstaudiomixerplugin.c
        gstreamer/gst/audiomixer/gstliveadder.c
        gstreamer/gst/audiorate/gstaudiorate.c
        gstreamer/gst/audioresample/gstaudioresample.c
        gstreamer/gst/audiotestsrc/gstaudiotestsrc.c
        gstreamer/gst/compositor/blend.c
        gstreamer/gst/compositor/compositor.c
        gstreamer/gst/compositor/compositororc-dist.c
        gstreamer/gst/encoding/gstencodebasebin.c
        gstreamer/gst/encoding/gstencodebin.c
        gstreamer/gst/encoding/gstencodebin2.c
        gstreamer/gst/encoding/gstencodingelements.c
        gstreamer/gst/encoding/gstsmartencoder.c
        gstreamer/gst/encoding/gststreamcombiner.c
        gstreamer/gst/encoding/gststreamsplitter.c
        gstreamer/gst/encoding/plugin.c
        gstreamer/gst/gio/gstgio.c
        gstreamer/gst/gio/gstgiobasesink.c
        gstreamer/gst/gio/gstgiobasesrc.c
        gstreamer/gst/gio/gstgioelement.c
        gstreamer/gst/gio/gstgiosink.c
        gstreamer/gst/gio/gstgiosrc.c
        gstreamer/gst/gio/gstgiostreamsink.c
        gstreamer/gst/gio/gstgiostreamsrc.c
        gstreamer/gst/overlaycomposition/gstoverlaycomposition.c
        gstreamer/gst/pbtypes/gstpbtypes.c
        gstreamer/gst/playback/gstdecodebin2.c
        gstreamer/gst/playback/gstdecodebin3.c
        gstreamer/gst/playback/gstdecodebin3-parse.c
        gstreamer/gst/playback/gstparsebin.c
        gstreamer/gst/playback/gstplay-enum.c
        gstreamer/gst/playback/gstplaybackelement.c
        gstreamer/gst/playback/gstplaybackplugin.c
        gstreamer/gst/playback/gstplaybackutils.c
        gstreamer/gst/playback/gstplaybin2.c
        gstreamer/gst/playback/gstplaybin3.c
        gstreamer/gst/playback/gstplaysink.c
        gstreamer/gst/playback/gstplaysinkaudioconvert.c
        gstreamer/gst/playback/gstplaysinkconvertbin.c
        gstreamer/gst/playback/gstplaysinkvideoconvert.c
        gstreamer/gst/playback/gststreamsynchronizer.c
        gstreamer/gst/playback/gstsubtitleoverlay.c
        gstreamer/gst/playback/gsturidecodebin.c
        gstreamer/gst/playback/gsturidecodebin3.c
        gstreamer/gst/playback/gsturisourcebin.c
        gstreamer/gst/printf/asnprintf.c
        gstreamer/gst/printf/printf.c
        gstreamer/gst/printf/printf-args.c
        gstreamer/gst/printf/printf-extension.c
        gstreamer/gst/printf/printf-parse.c
        gstreamer/gst/printf/vasnprintf.c
        gstreamer/gst/rawparse/gstrawaudioparse.c
        gstreamer/gst/rawparse/gstrawbaseparse.c
        gstreamer/gst/rawparse/gstrawvideoparse.c
        gstreamer/gst/rawparse/gstunalignedaudioparse.c
        gstreamer/gst/rawparse/gstunalignedvideoparse.c
        gstreamer/gst/rawparse/plugin.c
        gstreamer/gst/subparse/gstssaparse.c
        gstreamer/gst/subparse/gstsubparse.c
        gstreamer/gst/subparse/gstsubparseelement.c
        gstreamer/gst/subparse/gstsubparseplugin.c
        gstreamer/gst/subparse/mpl2parse.c
        gstreamer/gst/subparse/qttextparse.c
        gstreamer/gst/subparse/samiparse.c
        gstreamer/gst/subparse/tmplayerparse.c
        gstreamer/gst/tcp/gstmultifdsink.c
        gstreamer/gst/tcp/gstmultihandlesink.c
        gstreamer/gst/tcp/gstmultisocketsink.c
        gstreamer/gst/tcp/gstsocketsrc.c
        gstreamer/gst/tcp/gsttcpclientsink.c
        gstreamer/gst/tcp/gsttcpclientsrc.c
        gstreamer/gst/tcp/gsttcpelements.c
        gstreamer/gst/tcp/gsttcpplugin.c
        gstreamer/gst/tcp/gsttcpserversink.c
        gstreamer/gst/tcp/gsttcpserversrc.c
        gstreamer/gst/tcp/gsttcpsrcstats.c
        gstreamer/gst/typefind/gsttypefindfunctions.c
        gstreamer/gst/typefind/gsttypefindfunctionsdata.c
        gstreamer/gst/typefind/gsttypefindfunctionsplugin.c
        gstreamer/gst/typefind/gsttypefindfunctionsriff.c
        gstreamer/gst/typefind/gsttypefindfunctionsstartwith.c
        gstreamer/gst/videoconvert/gstvideoconvert.c
        gstreamer/gst/videorate/gstvideorate.c
        gstreamer/gst/videoscale/gstvideoscale.c
        gstreamer/gst/videotestsrc/generate_sine_table.c
        gstreamer/gst/videotestsrc/gstvideotestsrc.c
        gstreamer/gst/videotestsrc/gstvideotestsrcorc-dist.c
        gstreamer/gst/videotestsrc/videotestsrc.c
        gstreamer/gst/volume/gstvolume.c
        gstreamer/gst/volume/gstvolumeorc-dist.c
        gstreamer/gst/glib-compat.c
        gstreamer/gst/gst.c
        gstreamer/gst/gstallocator.c
        gstreamer/gst/gstatomicqueue.c
        gstreamer/gst/gstbin.c
        gstreamer/gst/gstbuffer.c
        gstreamer/gst/gstbufferlist.c
        gstreamer/gst/gstbufferpool.c
        gstreamer/gst/gstbus.c
        gstreamer/gst/gstcaps.c
        gstreamer/gst/gstcapsfeatures.c
        gstreamer/gst/gstchildproxy.c
        gstreamer/gst/gstclock.c
        gstreamer/gst/gstclock-linreg.c
        gstreamer/gst/gstcontext.c
        gstreamer/gst/gstcontrolbinding.c
        gstreamer/gst/gstcontrolsource.c
        gstreamer/gst/gstdatetime.c
        gstreamer/gst/gstdebugutils.c
        gstreamer/gst/gstdevice.c
        gstreamer/gst/gstdevicemonitor.c
        gstreamer/gst/gstdeviceprovider.c
        gstreamer/gst/gstdeviceproviderfactory.c
        gstreamer/gst/gstelement.c
        gstreamer/gst/gstelementfactory.c
        gstreamer/gst/gsterror.c
        gstreamer/gst/gstevent.c
        gstreamer/gst/gstformat.c
        gstreamer/gst/gstghostpad.c
        gstreamer/gst/gstinfo.c
        gstreamer/gst/gstiterator.c
        gstreamer/gst/gstmemory.c
        gstreamer/gst/gstmessage.c
        gstreamer/gst/gstmeta.c
        gstreamer/gst/gstminiobject.c
        gstreamer/gst/gstobject.c
        gstreamer/gst/gstpad.c
        gstreamer/gst/gstpadtemplate.c
        gstreamer/gst/gstparamspecs.c
        gstreamer/gst/gstparse.c
        gstreamer/gst/gstpipeline.c
        gstreamer/gst/gstplugin.c
        gstreamer/gst/gstpluginfeature.c
        gstreamer/gst/gstpluginloader.c
        gstreamer/gst/gstpoll.c
        gstreamer/gst/gstpreset.c
        gstreamer/gst/gstprotection.c
        gstreamer/gst/gstquark.c
        gstreamer/gst/gstquery.c
        gstreamer/gst/gstregistry.c
        gstreamer/gst/gstregistrybinary.c
        gstreamer/gst/gstregistrychunks.c
        gstreamer/gst/gstsample.c
        gstreamer/gst/gstsegment.c
        gstreamer/gst/gststructure.c
        gstreamer/gst/gstsystemclock.c
        gstreamer/gst/gsttaglist.c
        gstreamer/gst/gsttagsetter.c
        gstreamer/gst/gsttask.c
        gstreamer/gst/gsttaskpool.c
        gstreamer/gst/gsttoc.c
        gstreamer/gst/gsttocsetter.c
        gstreamer/gst/gsttrace.c
        gstreamer/gst/gsttracer.c
        gstreamer/gst/gsttracerfactory.c
        gstreamer/gst/gsttracerrecord.c
        gstreamer/gst/gsttracerutils.c
        gstreamer/gst/gsttypefind.c
        gstreamer/gst/gsttypefindfactory.c
        gstreamer/gst/gsturi.c
        gstreamer/gst/gstutils.c
        gstreamer/gst/gstvalue.c
        gstreamer/libs/allocators/gstdmabuf.c
        gstreamer/libs/allocators/gstfdmemory.c
        gstreamer/libs/allocators/gstphysmemory.c
        gstreamer/libs/app/gstappsink.c
        gstreamer/libs/app/gstappsrc.c
        gstreamer/libs/audio/audio.c
        gstreamer/libs/audio/audio-buffer.c
        gstreamer/libs/audio/audio-channel-mixer.c
        gstreamer/libs/audio/audio-channels.c
        gstreamer/libs/audio/audio-converter.c
        gstreamer/libs/audio/audio-format.c
        gstreamer/libs/audio/audio-info.c
        gstreamer/libs/audio/audio-quantize.c
        gstreamer/libs/audio/audio-resampler.c
        gstreamer/libs/audio/audio-resampler-x86-sse.c
        gstreamer/libs/audio/audio-resampler-x86-sse2.c
        gstreamer/libs/audio/audio-resampler-x86-sse41.c
        gstreamer/libs/audio/dbesi0.c
        gstreamer/libs/audio/gstaudioaggregator.c
        gstreamer/libs/audio/gstaudiobasesink.c
        gstreamer/libs/audio/gstaudiobasesrc.c
        gstreamer/libs/audio/gstaudiocdsrc.c
        gstreamer/libs/audio/gstaudioclock.c
        gstreamer/libs/audio/gstaudiodecoder.c
        gstreamer/libs/audio/gstaudioencoder.c
        gstreamer/libs/audio/gstaudiofilter.c
        gstreamer/libs/audio/gstaudioiec61937.c
        gstreamer/libs/audio/gstaudiometa.c
        gstreamer/libs/audio/gstaudiopack-dist.c
        gstreamer/libs/audio/gstaudioringbuffer.c
        gstreamer/libs/audio/gstaudiosink.c
        gstreamer/libs/audio/gstaudiosrc.c
        gstreamer/libs/audio/gstaudiostreamalign.c
        gstreamer/libs/audio/gstaudioutilsprivate.c
        gstreamer/libs/audio/streamvolume.c
        gstreamer/libs/base/gstadapter.c
        gstreamer/libs/base/gstbaseparse.c
        gstreamer/libs/base/gstbasesink.c
        gstreamer/libs/base/gstbasesrc.c
        gstreamer/libs/base/gstbasetransform.c
        gstreamer/libs/base/gstbitreader.c
        gstreamer/libs/base/gstbytereader.c
        gstreamer/libs/base/gstbytewriter.c
        gstreamer/libs/base/gstcollectpads.c
        gstreamer/libs/base/gstdataqueue.c
        gstreamer/libs/base/gstflowcombiner.c
        gstreamer/libs/base/gstindex.c
        gstreamer/libs/base/gstmemindex.c
        gstreamer/libs/base/gstpushsrc.c
        gstreamer/libs/base/gstqueuearray.c
        gstreamer/libs/base/gsttypefindhelper.c
        gstreamer/libs/check/libcheck/alarm.c
        gstreamer/libs/check/libcheck/check.c
        gstreamer/libs/check/libcheck/check_error.c
        gstreamer/libs/check/libcheck/check_list.c
        gstreamer/libs/check/libcheck/check_log.c
        gstreamer/libs/check/libcheck/check_msg.c
        gstreamer/libs/check/libcheck/check_pack.c
        gstreamer/libs/check/libcheck/check_print.c
        gstreamer/libs/check/libcheck/check_run.c
        gstreamer/libs/check/libcheck/check_str.c
        gstreamer/libs/check/libcheck/clock_gettime.c
        gstreamer/libs/check/libcheck/libcompat.c
        gstreamer/libs/check/libcheck/localtime_r.c
        gstreamer/libs/check/libcheck/strsignal.c
        gstreamer/libs/check/libcheck/timer_create.c
        gstreamer/libs/check/libcheck/timer_delete.c
        gstreamer/libs/check/libcheck/timer_settime.c
        gstreamer/libs/check/gstbufferstraw.c
        gstreamer/libs/check/gstcheck.c
        gstreamer/libs/check/gstconsistencychecker.c
        gstreamer/libs/check/gstharness.c
        gstreamer/libs/check/gsttestclock.c
        gstreamer/libs/controller/gstargbcontrolbinding.c
        gstreamer/libs/controller/gstdirectcontrolbinding.c
        gstreamer/libs/controller/gstinterpolationcontrolsource.c
        gstreamer/libs/controller/gstlfocontrolsource.c
        gstreamer/libs/controller/gsttimedvaluecontrolsource.c
        gstreamer/libs/controller/gsttriggercontrolsource.c
        gstreamer/libs/fft/gstfft.c
        gstreamer/libs/fft/gstfftf32.c
        gstreamer/libs/fft/gstfftf64.c
        gstreamer/libs/fft/gstffts16.c
        gstreamer/libs/fft/gstffts32.c
        gstreamer/libs/fft/kiss_fft_f32.c
        gstreamer/libs/fft/kiss_fft_f64.c
        gstreamer/libs/fft/kiss_fft_s16.c
        gstreamer/libs/fft/kiss_fft_s32.c
        gstreamer/libs/fft/kiss_fftr_f32.c
        gstreamer/libs/fft/kiss_fftr_f64.c
        gstreamer/libs/fft/kiss_fftr_s16.c
        gstreamer/libs/fft/kiss_fftr_s32.c
        gstreamer/libs/gl/android/gstglwindow_android_egl.c
        gstreamer/libs/gl/cocoa/gstglcaopengllayer.m
        gstreamer/libs/gl/cocoa/gstglcontext_cocoa.m
        gstreamer/libs/gl/cocoa/gstgldisplay_cocoa.m
        gstreamer/libs/gl/cocoa/gstglwindow_cocoa.m
        gstreamer/libs/gl/dispmanx/gstglwindow_dispmanx_egl.c
        gstreamer/libs/gl/eagl/gstglcontext_eagl.m
        gstreamer/libs/gl/eagl/gstglwindow_eagl.m
        gstreamer/libs/gl/egl/gstegl.c
        gstreamer/libs/gl/egl/gsteglimage.c
        gstreamer/libs/gl/egl/gstglcontext_egl.c
        gstreamer/libs/gl/egl/gstgldisplay_egl.c
        gstreamer/libs/gl/egl/gstgldisplay_egl_device.c
        gstreamer/libs/gl/egl/gstglmemoryegl.c
        gstreamer/libs/gl/gbm/gstgl_gbm_utils.c
        gstreamer/libs/gl/gbm/gstgldisplay_gbm.c
        gstreamer/libs/gl/gbm/gstglwindow_gbm_egl.c
        gstreamer/libs/gl/viv-fb/gstgldisplay_viv_fb.c
        gstreamer/libs/gl/viv-fb/gstglwindow_viv_fb_egl.c
        gstreamer/libs/gl/wayland/gstgldisplay_wayland.c
        gstreamer/libs/gl/wayland/gstglwindow_wayland_egl.c
        gstreamer/libs/gl/wayland/wayland_event_source.c
        gstreamer/libs/gl/wgl/gstglcontext_wgl.c
        gstreamer/libs/gl/win32/gstglwindow_win32.c
        gstreamer/libs/gl/winrt/gstglwindow_winrt_egl.cpp
        gstreamer/libs/gl/x11/gstglcontext_glx.c
        gstreamer/libs/gl/x11/gstgldisplay_x11.c
        gstreamer/libs/gl/x11/gstglwindow_x11.c
        gstreamer/libs/gl/x11/xcb_event_source.c
        gstreamer/libs/gl/gstglapi.c
        gstreamer/libs/gl/gstglbasefilter.c
        gstreamer/libs/gl/gstglbasememory.c
        gstreamer/libs/gl/gstglbasesrc.c
        gstreamer/libs/gl/gstglbuffer.c
        gstreamer/libs/gl/gstglbufferpool.c
        gstreamer/libs/gl/gstglcolorconvert.c
        gstreamer/libs/gl/gstglcontext.c
        gstreamer/libs/gl/gstglcontextconfig.c
        gstreamer/libs/gl/gstgldebug.c
        gstreamer/libs/gl/gstgldisplay.c
        gstreamer/libs/gl/gstglfeature.c
        gstreamer/libs/gl/gstglfilter.c
        gstreamer/libs/gl/gstglformat.c
        gstreamer/libs/gl/gstglframebuffer.c
        gstreamer/libs/gl/gstglmemory.c
        gstreamer/libs/gl/gstglmemorypbo.c
        gstreamer/libs/gl/gstgloverlaycompositor.c
        gstreamer/libs/gl/gstglpixelformat.c
        gstreamer/libs/gl/gstglquery.c
        gstreamer/libs/gl/gstglrenderbuffer.c
        gstreamer/libs/gl/gstglshader.c
        gstreamer/libs/gl/gstglshaderstrings.c
        gstreamer/libs/gl/gstglsl.c
        gstreamer/libs/gl/gstglslstage.c
        gstreamer/libs/gl/gstglsyncmeta.c
        gstreamer/libs/gl/gstglupload.c
        gstreamer/libs/gl/gstglutils.c
        gstreamer/libs/gl/gstglviewconvert.c
        gstreamer/libs/gl/gstglwindow.c
        gstreamer/libs/helpers/gst-completion-helper.c
        gstreamer/libs/helpers/gst-plugin-scanner.c
        gstreamer/libs/helpers/gst-ptp-helper.c
        gstreamer/libs/net/gstnetaddressmeta.c
        gstreamer/libs/net/gstnetclientclock.c
        gstreamer/libs/net/gstnetcontrolmessagemeta.c
        gstreamer/libs/net/gstnettimepacket.c
        gstreamer/libs/net/gstnettimeprovider.c
        gstreamer/libs/net/gstntppacket.c
        gstreamer/libs/net/gstptpclock.c
        gstreamer/libs/pbutils/codec-utils.c
        gstreamer/libs/pbutils/descriptions.c
        gstreamer/libs/pbutils/encoding-profile.c
        gstreamer/libs/pbutils/encoding-target.c
        gstreamer/libs/pbutils/gstaudiovisualizer.c
        gstreamer/libs/pbutils/gstdiscoverer.c
        gstreamer/libs/pbutils/gstdiscoverer-types.c
        gstreamer/libs/pbutils/gstpluginsbaseversion.c
        gstreamer/libs/pbutils/install-plugins.c
        gstreamer/libs/pbutils/missing-plugins.c
        gstreamer/libs/pbutils/pbutils.c
        gstreamer/libs/riff/riff.c
        gstreamer/libs/riff/riff-media.c
        gstreamer/libs/riff/riff-read.c
        gstreamer/libs/rtp/gstrtcpbuffer.c
        gstreamer/libs/rtp/gstrtpbaseaudiopayload.c
        gstreamer/libs/rtp/gstrtpbasedepayload.c
        gstreamer/libs/rtp/gstrtpbasepayload.c
        gstreamer/libs/rtp/gstrtpbuffer.c
        gstreamer/libs/rtp/gstrtphdrext.c
        gstreamer/libs/rtp/gstrtpmeta.c
        gstreamer/libs/rtp/gstrtppayloads.c
        gstreamer/libs/rtsp/gstrtspconnection.c
        gstreamer/libs/rtsp/gstrtspdefs.c
        gstreamer/libs/rtsp/gstrtspextension.c
        gstreamer/libs/rtsp/gstrtspmessage.c
        gstreamer/libs/rtsp/gstrtsprange.c
        gstreamer/libs/rtsp/gstrtsptransport.c
        gstreamer/libs/rtsp/gstrtspurl.c
        gstreamer/libs/sdp/gstmikey.c
        gstreamer/libs/sdp/gstsdpmessage.c
        gstreamer/libs/tag/gstexiftag.c
        gstreamer/libs/tag/gstid3tag.c
        gstreamer/libs/tag/gsttagdemux.c
        gstreamer/libs/tag/gsttageditingprivate.c
        gstreamer/libs/tag/gsttagmux.c
        gstreamer/libs/tag/gstvorbistag.c
        gstreamer/libs/tag/gstxmptag.c
        gstreamer/libs/tag/id3v2.c
        gstreamer/libs/tag/id3v2frames.c
        gstreamer/libs/tag/lang.c
        gstreamer/libs/tag/lang-tables.dat
        gstreamer/libs/tag/licenses.c
        gstreamer/libs/tag/licenses-tables.dat
        gstreamer/libs/tag/mklangtables.c
        gstreamer/libs/tag/mklicensestables.c
        gstreamer/libs/tag/tags.c
        gstreamer/libs/tag/xmpwriter.c
        gstreamer/libs/video/colorbalance.c
        gstreamer/libs/video/colorbalancechannel.c
        gstreamer/libs/video/convertframe.c
        gstreamer/libs/video/gstvideoaffinetransformationmeta.c
        gstreamer/libs/video/gstvideoaggregator.c
        gstreamer/libs/video/gstvideocodecalphameta.c
        gstreamer/libs/video/gstvideodecoder.c
        gstreamer/libs/video/gstvideoencoder.c
        gstreamer/libs/video/gstvideofilter.c
        gstreamer/libs/video/gstvideometa.c
        gstreamer/libs/video/gstvideopool.c
        gstreamer/libs/video/gstvideosink.c
        gstreamer/libs/video/gstvideotimecode.c
        gstreamer/libs/video/gstvideoutils.c
        gstreamer/libs/video/gstvideoutilsprivate.c
        gstreamer/libs/video/navigation.c
        gstreamer/libs/video/video.c
        gstreamer/libs/video/video-anc.c
        gstreamer/libs/video/video-blend.c
        gstreamer/libs/video/video-chroma.c
        gstreamer/libs/video/video-color.c
        gstreamer/libs/video/video-converter.c
        gstreamer/libs/video/video-dither.c
        gstreamer/libs/video/video-event.c
        gstreamer/libs/video/video-format.c
        gstreamer/libs/video/video-frame.c
        gstreamer/libs/video/video-hdr.c
        gstreamer/libs/video/video-info.c
        gstreamer/libs/video/video-multiview.c
        gstreamer/libs/video/video-orc-dist.c
        gstreamer/libs/video/video-overlay-composition.c
        gstreamer/libs/video/video-resampler.c
        gstreamer/libs/video/video-scaler.c
        gstreamer/libs/video/video-tile.c
        gstreamer/libs/video/videodirection.c
        gstreamer/libs/video/videoorientation.c
        gstreamer/libs/video/videooverlay.c
        gstreamer/plugins/elements/gstcapsfilter.c
        gstreamer/plugins/elements/gstconcat.c
        gstreamer/plugins/elements/gstdownloadbuffer.c
        gstreamer/plugins/elements/gstelements.c
        gstreamer/plugins/elements/gstelements_private.c
        gstreamer/plugins/elements/gstfakesink.c
        gstreamer/plugins/elements/gstfakesrc.c
        gstreamer/plugins/elements/gstfdsink.c
        gstreamer/plugins/elements/gstfdsrc.c
        gstreamer/plugins/elements/gstfilesink.c
        gstreamer/plugins/elements/gstfilesrc.c
        gstreamer/plugins/elements/gstfunnel.c
        gstreamer/plugins/elements/gstidentity.c
        gstreamer/plugins/elements/gstinputselector.c
        gstreamer/plugins/elements/gstmultiqueue.c
        gstreamer/plugins/elements/gstoutputselector.c
        gstreamer/plugins/elements/gstqueue.c
        gstreamer/plugins/elements/gstqueue2.c
        gstreamer/plugins/elements/gstsparsefile.c
        gstreamer/plugins/elements/gststreamiddemux.c
        gstreamer/plugins/elements/gsttee.c
        gstreamer/plugins/elements/gsttypefindelement.c
        gstreamer/plugins/elements/gstvalve.c
        gstreamer/plugins/tracers/gstlatency.c
        gstreamer/plugins/tracers/gstlog.c
        gstreamer/plugins/tracers/gstrusage.c
        gstreamer/plugins/tracers/gststats.c
        gstreamer/plugins/tracers/gsttracers.c
        gstreamer/sys/ximage/ximage.c
        gstreamer/sys/ximage/ximagepool.c
        gstreamer/sys/ximage/ximagesink.c
        gstreamer/sys/xvimage/xvcontext.c
        gstreamer/sys/xvimage/xvimage.c
        gstreamer/sys/xvimage/xvimageallocator.c
        gstreamer/sys/xvimage/xvimagepool.c
        gstreamer/sys/xvimage/xvimagesink.c
        gstreamer/tests/benchmarks/caps.c
        gstreamer/tests/benchmarks/capsnego.c
        gstreamer/tests/benchmarks/complexity.c
        gstreamer/tests/benchmarks/controller.c
        gstreamer/tests/benchmarks/gstbufferstress.c
        gstreamer/tests/benchmarks/gstclockstress.c
        gstreamer/tests/benchmarks/gstpollstress.c
        gstreamer/tests/benchmarks/gstpoolstress.c
        gstreamer/tests/benchmarks/init.c
        gstreamer/tests/benchmarks/mass-elements.c
        gstreamer/tests/benchmarks/tracerserialize.c
        gstreamer/tests/check/elements/adder.c
        gstreamer/tests/check/elements/appsink.c
        gstreamer/tests/check/elements/appsrc.c
        gstreamer/tests/check/elements/audioconvert.c
        gstreamer/tests/check/elements/audiointerleave.c
        gstreamer/tests/check/elements/audiomixer.c
        gstreamer/tests/check/elements/audiorate.c
        gstreamer/tests/check/elements/audioresample.c
        gstreamer/tests/check/elements/audiotestsrc.c
        gstreamer/tests/check/elements/capsfilter.c
        gstreamer/tests/check/elements/compositor.c
        gstreamer/tests/check/elements/concat.c
        gstreamer/tests/check/elements/decodebin.c
        gstreamer/tests/check/elements/encodebin.c
        gstreamer/tests/check/elements/fakesink.c
        gstreamer/tests/check/elements/fakesrc.c
        gstreamer/tests/check/elements/fdsrc.c
        gstreamer/tests/check/elements/filesink.c
        gstreamer/tests/check/elements/filesrc.c
        gstreamer/tests/check/elements/funnel.c
        gstreamer/tests/check/elements/glbin.c
        gstreamer/tests/check/elements/glfilter.c
        gstreamer/tests/check/elements/glimagesink.c
        gstreamer/tests/check/elements/glmixer.c
        gstreamer/tests/check/elements/glstereo.c
        gstreamer/tests/check/elements/identity.c
        gstreamer/tests/check/elements/libvisual.c
        gstreamer/tests/check/elements/multifdsink.c
        gstreamer/tests/check/elements/multiqueue.c
        gstreamer/tests/check/elements/multisocketsink.c
        gstreamer/tests/check/elements/opus.c
        gstreamer/tests/check/elements/overlaycomposition.c
        gstreamer/tests/check/elements/playbin.c
        gstreamer/tests/check/elements/playbin-complex.c
        gstreamer/tests/check/elements/playsink.c
        gstreamer/tests/check/elements/queue.c
        gstreamer/tests/check/elements/queue2.c
        gstreamer/tests/check/elements/rawaudioparse.c
        gstreamer/tests/check/elements/rawvideoparse.c
        gstreamer/tests/check/elements/selector.c
        gstreamer/tests/check/elements/streamiddemux.c
        gstreamer/tests/check/elements/streamsynchronizer.c
        gstreamer/tests/check/elements/subparse.c
        gstreamer/tests/check/elements/tee.c
        gstreamer/tests/check/elements/textoverlay.c
        gstreamer/tests/check/elements/urisourcebin.c
        gstreamer/tests/check/elements/valve.c
        gstreamer/tests/check/elements/videoconvert.c
        gstreamer/tests/check/elements/videorate.c
        gstreamer/tests/check/elements/videoscale.c
        gstreamer/tests/check/elements/videotestsrc.c
        gstreamer/tests/check/elements/volume.c
        gstreamer/tests/check/elements/vorbisdec.c
        gstreamer/tests/check/elements/vorbistag.c
        gstreamer/tests/check/generic/clock-selection.c
        gstreamer/tests/check/generic/sinks.c
        gstreamer/tests/check/generic/states.c
        gstreamer/tests/check/gst/gst.c
        gstreamer/tests/check/gst/gstabi.c
        gstreamer/tests/check/gst/gstatomicqueue.c
        gstreamer/tests/check/gst/gstbin.c
        gstreamer/tests/check/gst/gstbuffer.c
        gstreamer/tests/check/gst/gstbufferlist.c
        gstreamer/tests/check/gst/gstbufferpool.c
        gstreamer/tests/check/gst/gstbus.c
        gstreamer/tests/check/gst/gstcaps.c
        gstreamer/tests/check/gst/gstcapsfeatures.c
        gstreamer/tests/check/gst/gstchildproxy.c
        gstreamer/tests/check/gst/gstclock.c
        gstreamer/tests/check/gst/gstcontext.c
        gstreamer/tests/check/gst/gstcontroller.c
        gstreamer/tests/check/gst/gstcpp.cc
        gstreamer/tests/check/gst/gstdatetime.c
        gstreamer/tests/check/gst/gstdevice.c
        gstreamer/tests/check/gst/gstelement.c
        gstreamer/tests/check/gst/gstelementfactory.c
        gstreamer/tests/check/gst/gstevent.c
        gstreamer/tests/check/gst/gstghostpad.c
        gstreamer/tests/check/gst/gstindex.c
        gstreamer/tests/check/gst/gstinfo.c
        gstreamer/tests/check/gst/gstiterator.c
        gstreamer/tests/check/gst/gstmemory.c
        gstreamer/tests/check/gst/gstmessage.c
        gstreamer/tests/check/gst/gstmeta.c
        gstreamer/tests/check/gst/gstminiobject.c
        gstreamer/tests/check/gst/gstobject.c
        gstreamer/tests/check/gst/gstpad.c
        gstreamer/tests/check/gst/gstparamspecs.c
        gstreamer/tests/check/gst/gstpipeline.c
        gstreamer/tests/check/gst/gstplugin.c
        gstreamer/tests/check/gst/gstpoll.c
        gstreamer/tests/check/gst/gstpreset.c
        gstreamer/tests/check/gst/gstprintf.c
        gstreamer/tests/check/gst/gstprotection.c
        gstreamer/tests/check/gst/gstquery.c
        gstreamer/tests/check/gst/gstregistry.c
        gstreamer/tests/check/gst/gstsegment.c
        gstreamer/tests/check/gst/gststructure.c
        gstreamer/tests/check/gst/gstsystemclock.c
        gstreamer/tests/check/gst/gsttag.c
        gstreamer/tests/check/gst/gsttagsetter.c
        gstreamer/tests/check/gst/gsttask.c
        gstreamer/tests/check/gst/gsttoc.c
        gstreamer/tests/check/gst/gsttocsetter.c
        gstreamer/tests/check/gst/gsttracerrecord.c
        gstreamer/tests/check/gst/gsturi.c
        gstreamer/tests/check/gst/gstutils.c
        gstreamer/tests/check/gst/gstvalue.c
        gstreamer/tests/check/gst/typefindfunctions.c
        gstreamer/tests/check/libs/adapter.c
        gstreamer/tests/check/libs/allocators.c
        gstreamer/tests/check/libs/audio.c
        gstreamer/tests/check/libs/audiocdsrc.c
        gstreamer/tests/check/libs/audiodecoder.c
        gstreamer/tests/check/libs/audioencoder.c
        gstreamer/tests/check/libs/audiosink.c
        gstreamer/tests/check/libs/baseaudiovisualizer.c
        gstreamer/tests/check/libs/baseparse.c
        gstreamer/tests/check/libs/basesink.c
        gstreamer/tests/check/libs/basesrc.c
        gstreamer/tests/check/libs/bitreader.c
        gstreamer/tests/check/libs/bitreader-noinline.c
        gstreamer/tests/check/libs/bytereader.c
        gstreamer/tests/check/libs/bytereader-noinline.c
        gstreamer/tests/check/libs/bytewriter.c
        gstreamer/tests/check/libs/bytewriter-noinline.c
        gstreamer/tests/check/libs/collectpads.c
        gstreamer/tests/check/libs/controller.c
        gstreamer/tests/check/libs/discoverer.c
        gstreamer/tests/check/libs/fft.c
        gstreamer/tests/check/libs/flowcombiner.c
        gstreamer/tests/check/libs/gstglapi.c
        gstreamer/tests/check/libs/gstglcolorconvert.c
        gstreamer/tests/check/libs/gstglcontext.c
        gstreamer/tests/check/libs/gstglfeature.c
        gstreamer/tests/check/libs/gstglformat.c
        gstreamer/tests/check/libs/gstglheaders.c
        gstreamer/tests/check/libs/gstglmatrix.c
        gstreamer/tests/check/libs/gstglmemory.c
        gstreamer/tests/check/libs/gstglquery.c
        gstreamer/tests/check/libs/gstglshader.c
        gstreamer/tests/check/libs/gstglsl.c
        gstreamer/tests/check/libs/gstglslstage.c
        gstreamer/tests/check/libs/gstglupload.c
        gstreamer/tests/check/libs/gstharness.c
        gstreamer/tests/check/libs/gstlibscpp.cc
        gstreamer/tests/check/libs/gstnetclientclock.c
        gstreamer/tests/check/libs/gstnettimeprovider.c
        gstreamer/tests/check/libs/gsttestclock.c
        gstreamer/tests/check/libs/libsabi.c
        gstreamer/tests/check/libs/mikey.c
        gstreamer/tests/check/libs/navigation.c
        gstreamer/tests/check/libs/pbutils.c
        gstreamer/tests/check/libs/profile.c
        gstreamer/tests/check/libs/queuearray.c
        gstreamer/tests/check/libs/rtp.c
        gstreamer/tests/check/libs/rtpbasedepayload.c
        gstreamer/tests/check/libs/rtpbasepayload.c
        gstreamer/tests/check/libs/rtpdummyhdrextimpl.c
        gstreamer/tests/check/libs/rtphdrext.c
        gstreamer/tests/check/libs/rtpmeta.c
        gstreamer/tests/check/libs/rtsp.c
        gstreamer/tests/check/libs/rtspconnection.c
        gstreamer/tests/check/libs/sdp.c
        gstreamer/tests/check/libs/sparsefile.c
        gstreamer/tests/check/libs/tag.c
        gstreamer/tests/check/libs/test_transform.c
        gstreamer/tests/check/libs/transform1.c
        gstreamer/tests/check/libs/transform2.c
        gstreamer/tests/check/libs/typefindhelper.c
        gstreamer/tests/check/libs/video.c
        gstreamer/tests/check/libs/videoanc.c
        gstreamer/tests/check/libs/videodecoder.c
        gstreamer/tests/check/libs/videoencoder.c
        gstreamer/tests/check/libs/videotimecode.c
        gstreamer/tests/check/libs/xmpwriter.c
        gstreamer/tests/check/pipelines/basetime.c
        gstreamer/tests/check/pipelines/capsfilter-renegotiation.c
        gstreamer/tests/check/pipelines/cleanup.c
        gstreamer/tests/check/pipelines/gio.c
        gstreamer/tests/check/pipelines/gl-launch-lines.c
        gstreamer/tests/check/pipelines/oggmux.c
        gstreamer/tests/check/pipelines/parse-disabled.c
        gstreamer/tests/check/pipelines/parse-launch.c
        gstreamer/tests/check/pipelines/queue-error.c
        gstreamer/tests/check/pipelines/seek.c
        gstreamer/tests/check/pipelines/simple-launch-lines.c
        gstreamer/tests/check/pipelines/streamsynchronizer.c
        gstreamer/tests/check/pipelines/stress.c
        gstreamer/tests/check/pipelines/tcp.c
        gstreamer/tests/check/pipelines/theoraenc.c
        gstreamer/tests/check/pipelines/vorbisdec.c
        gstreamer/tests/check/pipelines/vorbisenc.c
        gstreamer/tests/check/tools/gstinspect.c
        gstreamer/tests/examples/adapter/adapter_test.c
        gstreamer/tests/examples/app/appsink-src.c
        gstreamer/tests/examples/app/appsink-src2.c
        gstreamer/tests/examples/app/appsrc-ra.c
        gstreamer/tests/examples/app/appsrc-seekable.c
        gstreamer/tests/examples/app/appsrc-stream.c
        gstreamer/tests/examples/app/appsrc-stream2.c
        gstreamer/tests/examples/app/appsrc_ex.c
        gstreamer/tests/examples/audio/audiomix.c
        gstreamer/tests/examples/audio/volume.c
        gstreamer/tests/examples/compositor/crossfade.c
        gstreamer/tests/examples/compositor/mosaic.c
        gstreamer/tests/examples/compositor/signals.c
        gstreamer/tests/examples/controller/absolute-example.c
        gstreamer/tests/examples/controller/audio-example.c
        gstreamer/tests/examples/controller/control-sources.c
        gstreamer/tests/examples/controller/controller-graph.c
        gstreamer/tests/examples/controller/text-color-example.c
        gstreamer/tests/examples/decodebin_next/decodebin3.c
        gstreamer/tests/examples/decodebin_next/playbin-test.c
        gstreamer/tests/examples/decodebin_next/uridecodebin3-select-all.c
        gstreamer/tests/examples/device-provider/example-device-provider.c
        gstreamer/tests/examples/dynamic/addstream.c
        gstreamer/tests/examples/dynamic/codec-select.c
        gstreamer/tests/examples/dynamic/sprinkle.c
        gstreamer/tests/examples/dynamic/sprinkle2.c
        gstreamer/tests/examples/dynamic/sprinkle3.c
        gstreamer/tests/examples/encoding/encoding.c
        gstreamer/tests/examples/encoding/gstcapslist.c
        gstreamer/tests/examples/fft/fftrange.c
        gstreamer/tests/examples/gio/giosrc-mounting.c
        gstreamer/tests/examples/gl/cocoa/cocoa-videooverlay.m
        gstreamer/tests/examples/gl/generic/cube/main.cpp
        gstreamer/tests/examples/gl/generic/cubeyuv/main.cpp
        gstreamer/tests/examples/gl/generic/doublecube/main.cpp
        gstreamer/tests/examples/gl/generic/recordgraphic/main.cpp
        gstreamer/tests/examples/gl/gtk/3dvideo/main.cpp
        gstreamer/tests/examples/gl/gtk/3dvideo/mviewwidget.c
        gstreamer/tests/examples/gl/gtk/filternovideooverlay/main.cpp
        gstreamer/tests/examples/gl/gtk/filtervideooverlay/main.cpp
        gstreamer/tests/examples/gl/gtk/fxtest/fxtest.c
        gstreamer/tests/examples/gl/gtk/fxtest/pixbufdrop.c
        gstreamer/tests/examples/gl/gtk/switchvideooverlay/main.cpp
        gstreamer/tests/examples/gl/gtk/gstgtk.c
        gstreamer/tests/examples/gl/qt/mousevideooverlay/gstthread.cpp
        gstreamer/tests/examples/gl/qt/mousevideooverlay/main.cpp
        gstreamer/tests/examples/gl/qt/mousevideooverlay/pipeline.cpp
        gstreamer/tests/examples/gl/qt/mousevideooverlay/qrenderer.cpp
        gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/gstthread.cpp
        gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/main.cpp
        gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/pipeline.cpp
        gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/qglrenderer.cpp
        gstreamer/tests/examples/gl/qt/qglwtextureshare/cocoa_utils.mm
        gstreamer/tests/examples/gl/qt/qglwtextureshare/gstthread.cpp
        gstreamer/tests/examples/gl/qt/qglwtextureshare/main.cpp
        gstreamer/tests/examples/gl/qt/qglwtextureshare/pipeline.cpp
        gstreamer/tests/examples/gl/qt/qglwtextureshare/qglrenderer.cpp
        gstreamer/tests/examples/gl/qt/videooverlay/videooverlay.cpp
        gstreamer/tests/examples/gl/sdl/sdlshare.c
        gstreamer/tests/examples/gl/sdl/sdlshare2.c
        gstreamer/tests/examples/helloworld/helloworld.c
        gstreamer/tests/examples/memory/memory_test.c
        gstreamer/tests/examples/memory/my-memory.c
        gstreamer/tests/examples/memory/my-vidmem.c
        gstreamer/tests/examples/netclock/netclock-client.c
        gstreamer/tests/examples/netclock/netclock-server.c
        gstreamer/tests/examples/overlay/gtk-videooverlay.c
        gstreamer/tests/examples/overlay/qt-videooverlay.cpp
        gstreamer/tests/examples/overlay/qtgv-videooverlay.cpp
        gstreamer/tests/examples/overlay/win32-videooverlay.c
        gstreamer/tests/examples/overlay/win32-videooverlay-playbin.c
        gstreamer/tests/examples/overlaycomposition/overlaycomposition.c
        gstreamer/tests/examples/playback/playback-test.c
        gstreamer/tests/examples/playrec/playrec.c
        gstreamer/tests/examples/ptp/ptp-print-times.c
        gstreamer/tests/examples/seek/instant-rate-change.c
        gstreamer/tests/examples/seek/jsseek.c
        gstreamer/tests/examples/seek/scrubby.c
        gstreamer/tests/examples/seek/stepping.c
        gstreamer/tests/examples/seek/stepping2.c
        gstreamer/tests/examples/snapshot/snapshot.c
        gstreamer/tests/examples/stepping/framestep1.c
        gstreamer/tests/examples/streamiddemux/streamiddemux-stream.c
        gstreamer/tests/examples/streams/rtpool-test.c
        gstreamer/tests/examples/streams/schedule-tasks.c
        gstreamer/tests/examples/streams/stream-status.c
        gstreamer/tests/examples/streams/testrtpool.c
        gstreamer/tests/icles/playback/decodetest.c
        gstreamer/tests/icles/playback/test.c
        gstreamer/tests/icles/playback/test2.c
        gstreamer/tests/icles/playback/test3.c
        gstreamer/tests/icles/playback/test4.c
        gstreamer/tests/icles/playback/test5.c
        gstreamer/tests/icles/playback/test6.c
        gstreamer/tests/icles/playback/test7.c
        gstreamer/tests/icles/audio-trickplay.c
        gstreamer/tests/icles/benchmark-appsink.c
        gstreamer/tests/icles/benchmark-appsrc.c
        gstreamer/tests/icles/benchmark-video-conversion.c
        gstreamer/tests/icles/input-selector-test.c
        gstreamer/tests/icles/output-selector-test.c
        gstreamer/tests/icles/playbin-text.c
        gstreamer/tests/icles/position-formats.c
        gstreamer/tests/icles/stress-playbin.c
        gstreamer/tests/icles/stress-videooverlay.c
        gstreamer/tests/icles/test-box.c
        gstreamer/tests/icles/test-colorkey.c
        gstreamer/tests/icles/test-effect-switch.c
        gstreamer/tests/icles/test-overlay-blending.c
        gstreamer/tests/icles/test-resample.c
        gstreamer/tests/icles/test-reverseplay.c
        gstreamer/tests/icles/test-scale.c
        gstreamer/tests/icles/test-textoverlay.c
        gstreamer/tests/icles/test-videooverlay.c
        gstreamer/tools/gst-device-monitor.c
        gstreamer/tools/gst-discoverer.c
        gstreamer/tools/gst-inspect.c
        gstreamer/tools/gst-launch.c
        gstreamer/tools/gst-play.c
        gstreamer/tools/gst-play-kb.c
        gstreamer/tools/gst-stats.c
        gstreamer/tools/gst-typefind.c
        gtest/codegear/gtest_all.cc
        gtest/codegear/gtest_link.cc
        gtest/samples/sample1.cc
        gtest/samples/sample1_unittest.cc
        gtest/samples/sample2.cc
        gtest/samples/sample2_unittest.cc
        gtest/samples/sample3_unittest.cc
        gtest/samples/sample4.cc
        gtest/samples/sample4_unittest.cc
        gtest/samples/sample5_unittest.cc
        gtest/samples/sample6_unittest.cc
        gtest/samples/sample7_unittest.cc
        gtest/samples/sample8_unittest.cc
        gtest/samples/sample9_unittest.cc
        gtest/samples/sample10_unittest.cc
        gtest/gtest.cc
        gtest/gtest-all.cc
        gtest/gtest-death-test.cc
        gtest/gtest-death-test_ex_test.cc
        gtest/gtest-death-test_test.cc
        gtest/gtest-filepath.cc
        gtest/gtest-filepath_test.cc
        gtest/gtest-linked_ptr_test.cc
        gtest/gtest-listener_test.cc
        gtest/gtest-message_test.cc
        gtest/gtest-options_test.cc
        gtest/gtest-param-test2_test.cc
        gtest/gtest-param-test_test.cc
        gtest/gtest-port.cc
        gtest/gtest-port_test.cc
        gtest/gtest-printers.cc
        gtest/gtest-printers_test.cc
        gtest/gtest-test-part.cc
        gtest/gtest-test-part_test.cc
        gtest/gtest-tuple_test.cc
        gtest/gtest-typed-test.cc
        gtest/gtest-typed-test2_test.cc
        gtest/gtest-typed-test_test.cc
        gtest/gtest-unittest-api_test.cc
        gtest/gtest_all_test.cc
        gtest/gtest_break_on_failure_unittest_.cc
        gtest/gtest_catch_exceptions_test_.cc
        gtest/gtest_color_test_.cc
        gtest/gtest_env_var_test_.cc
        gtest/gtest_environment_test.cc
        gtest/gtest_filter_unittest_.cc
        gtest/gtest_help_test_.cc
        gtest/gtest_list_tests_unittest_.cc
        gtest/gtest_main.cc
        gtest/gtest_main_unittest.cc
        gtest/gtest_no_test_unittest.cc
        gtest/gtest_output_test_.cc
        gtest/gtest_pred_impl_unittest.cc
        gtest/gtest_premature_exit_test.cc
        gtest/gtest_prod_test.cc
        gtest/gtest_repeat_test.cc
        gtest/gtest_shuffle_test_.cc
        gtest/gtest_sole_header_test.cc
        gtest/gtest_stress_test.cc
        gtest/gtest_throw_on_failure_ex_test.cc
        gtest/gtest_throw_on_failure_test_.cc
        gtest/gtest_uninitialized_test_.cc
        gtest/gtest_unittest.cc
        gtest/gtest_xml_outfile1_test_.cc
        gtest/gtest_xml_outfile2_test_.cc
        gtest/gtest_xml_output_unittest_.cc
        gtest/production.cc
        gthread/tests/1bit-mutex.c
        gthread/gthread-impl.c
        gthread/gthread-none.c
        gthread/gthread-posix.c
        gthread/gthread-win32.c
        integration/ACLTestContext.cpp
        integration/AipStateObserver.cpp
        integration/AlertsIntegrationTest.cpp
        integration/AlexaAuthorizationDelegateTest.cpp
        integration/AlexaCommunicationsLibraryTest.cpp
        integration/AlexaDirectiveSequencerLibraryTest.cpp
        integration/AudioInputProcessorIntegrationTest.cpp
        integration/AudioPlayerIntegrationTest.cpp
        integration/AuthDelegateTestContext.cpp
        integration/AuthObserver.cpp
        integration/ClientMessageHandler.cpp
        integration/ConnectionStatusObserver.cpp
        integration/NetworkIntegrationTests.cpp
        integration/ObservableMessageRequest.cpp
        integration/SDKTestContext.cpp
        integration/ServerDisconnectIntegrationTest.cpp
        integration/SpeechSynthesizerIntegrationTest.cpp
        integration/TestAlertObserver.cpp
        integration/TestDirectiveHandler.cpp
        integration/TestExceptionEncounteredSender.cpp
        integration/TestMediaPlayer.cpp
        integration/TestMessageSender.cpp
        integration/TestSpeechSynthesizerObserver.cpp
        interrupt_model/InterruptModel.cpp
        interrupt_model/InterruptModelConfiguration.cpp
        interrupt_model/InterruptModelTest.cpp
        json/allocators.cpp
        json/JSONGenerator.cpp
        json/JSONUtils.cpp
        json/reader.cpp
        kwd/kittai/KittAiKeyWordDetector.cpp
        kwd/kittai/KittAiKeyWordDetectorTest.cpp
        kwd/kittai/SnowboyWrapper.cpp
        kwd/kwd_provider/KeywordDetectorProvider.cpp
        kwd/sensory/SensoryKeywordDetector.cpp
        kwd/sensory/SensoryKeywordDetectorTest.cpp
        kwd/AbstractKeywordDetector.cpp
        kwd/AbstractKeywordDetectorTest.cpp
        logger/ConsoleLogger.cpp
        logger/Level.cpp
        logger/LogEntry.cpp
        logger/LogEntryBuffer.cpp
        logger/LogEntryStream.cpp
        logger/Logger.cpp
        logger/LoggerSinkManager.cpp
        logger/LoggerUtils.cpp
        logger/LogStringFormatter.cpp
        logger/ModuleLogger.cpp
        logger/ThreadMoniker.cpp
        media_player/AndroidSLESMediaPlayer.cpp
        media_player/AndroidSLESMediaPlayerTest.cpp
        media_player/AndroidSLESMediaQueue.cpp
        media_player/AndroidSLESMediaQueueTest.cpp
        media_player/AndroidSLESSpeaker.cpp
        media_player/AndroidSLESSpeakerTest.cpp
        media_player/AttachmentReaderSource.cpp
        media_player/BaseStreamSource.cpp
        media_player/ErrorTypeConversion.cpp
        media_player/ErrorTypeConversionTest.cpp
        media_player/FFmpegAttachmentInputController.cpp
        media_player/FFmpegAttachmentInputControllerTest.cpp
        media_player/FFmpegDecoder.cpp
        media_player/FFmpegDecoderTest.cpp
        media_player/FFmpegDeleter.cpp
        media_player/FFmpegStreamInputController.cpp
        media_player/FFmpegStreamInputControllerTest.cpp
        media_player/FFmpegUrlInputController.cpp
        media_player/FFmpegUrlInputControllerTest.cpp
        media_player/IStreamSource.cpp
        media_player/MediaPlayer.cpp
        media_player/MediaPlayerTest.cpp
        media_player/Normalizer.cpp
        media_player/NormalizerTest.cpp
        media_player/OffsetManager.cpp
        media_player/PlaybackConfiguration.cpp
        media_player/PlaybackContext.cpp
        media_player/PlaybackContextTest.cpp
        media_player/PooledMediaPlayerFactory.cpp
        metrics/DataPoint.cpp
        metrics/DataPointCounterBuilder.cpp
        metrics/DataPointDurationBuilder.cpp
        metrics/DataPointStringBuilder.cpp
        metrics/MetricEvent.cpp
        metrics/MetricEventBuilder.cpp
        multiparser/multipart.cpp
        network/HTTP2GetMimeHeadersResult.cpp
        network/HTTP2MimeRequestEncoder.cpp
        network/HTTP2MimeResponseDecoder.cpp
        network/HTTP2RequestConfig.cpp
        network/HTTP2SendDataResult.cpp
        playlist_parser/ContentDecrypter.cpp
        playlist_parser/ContentDecrypterTest.cpp
        playlist_parser/FFMpegInputBuffer.cpp
        playlist_parser/IterativePlaylistParser.cpp
        playlist_parser/IterativePlaylistParserTest.cpp
        playlist_parser/M3UParser.cpp
        playlist_parser/M3UParserTest.cpp
        playlist_parser/PlaylistParser.cpp
        playlist_parser/PlaylistParserTest.cpp
        playlist_parser/PlaylistUtils.cpp
        playlist_parser/PlaylistUtilsTest.cpp
        playlist_parser/UrlContentToAttachmentConverter.cpp
        pulse_audio/daemon/caps.c
        pulse_audio/daemon/cmdline.c
        pulse_audio/daemon/cpulimit.c
        pulse_audio/daemon/daemon-conf.c
        pulse_audio/daemon/dumpmodules.c
        pulse_audio/daemon/ltdl-bind-now.c
        pulse_audio/daemon/main.c
        pulse_audio/daemon/server-lookup.c
        pulse_audio/modules/alsa/alsa-mixer.c
        pulse_audio/modules/alsa/alsa-sink.c
        pulse_audio/modules/alsa/alsa-source.c
        pulse_audio/modules/alsa/alsa-ucm.c
        pulse_audio/modules/alsa/alsa-util.c
        pulse_audio/modules/alsa/module-alsa-card.c
        pulse_audio/modules/alsa/module-alsa-sink.c
        pulse_audio/modules/alsa/module-alsa-source.c
        pulse_audio/modules/bluetooth/a2dp-codec-sbc.c
        pulse_audio/modules/bluetooth/a2dp-codec-util.c
        pulse_audio/modules/bluetooth/backend-native.c
        pulse_audio/modules/bluetooth/backend-ofono.c
        pulse_audio/modules/bluetooth/bluez5-util.c
        pulse_audio/modules/bluetooth/module-bluetooth-discover.c
        pulse_audio/modules/bluetooth/module-bluetooth-policy.c
        pulse_audio/modules/bluetooth/module-bluez5-device.c
        pulse_audio/modules/bluetooth/module-bluez5-discover.c
        pulse_audio/modules/dbus/iface-card.c
        pulse_audio/modules/dbus/iface-card-profile.c
        pulse_audio/modules/dbus/iface-client.c
        pulse_audio/modules/dbus/iface-core.c
        pulse_audio/modules/dbus/iface-device.c
        pulse_audio/modules/dbus/iface-device-port.c
        pulse_audio/modules/dbus/iface-memstats.c
        pulse_audio/modules/dbus/iface-module.c
        pulse_audio/modules/dbus/iface-sample.c
        pulse_audio/modules/dbus/iface-stream.c
        pulse_audio/modules/dbus/module-dbus-protocol.c
        pulse_audio/modules/echo-cancel/adrian.c
        pulse_audio/modules/echo-cancel/adrian-aec.c
        pulse_audio/modules/echo-cancel/module-echo-cancel.c
        pulse_audio/modules/echo-cancel/null.c
        pulse_audio/modules/echo-cancel/speex.c
        pulse_audio/modules/echo-cancel/webrtc.cc
        pulse_audio/modules/gconf/gconf-helper.c
        pulse_audio/modules/gconf/module-gconf.c
        pulse_audio/modules/gsettings/gsettings-helper.c
        pulse_audio/modules/gsettings/module-gsettings.c
        pulse_audio/modules/jack/module-jack-sink.c
        pulse_audio/modules/jack/module-jack-source.c
        pulse_audio/modules/jack/module-jackdbus-detect.c
        pulse_audio/modules/macosx/module-bonjour-publish.c
        pulse_audio/modules/macosx/module-coreaudio-detect.c
        pulse_audio/modules/macosx/module-coreaudio-device.c
        pulse_audio/modules/oss/module-oss.c
        pulse_audio/modules/oss/oss-util.c
        pulse_audio/modules/raop/module-raop-discover.c
        pulse_audio/modules/raop/module-raop-sink.c
        pulse_audio/modules/raop/raop-client.c
        pulse_audio/modules/raop/raop-crypto.c
        pulse_audio/modules/raop/raop-packet-buffer.c
        pulse_audio/modules/raop/raop-sink.c
        pulse_audio/modules/raop/raop-util.c
        pulse_audio/modules/rtp/headerlist.c
        pulse_audio/modules/rtp/module-rtp-recv.c
        pulse_audio/modules/rtp/module-rtp-send.c
        pulse_audio/modules/rtp/rtp-common.c
        pulse_audio/modules/rtp/rtp-gstreamer.c
        pulse_audio/modules/rtp/rtp-native.c
        pulse_audio/modules/rtp/rtsp_client.c
        pulse_audio/modules/rtp/sap.c
        pulse_audio/modules/rtp/sdp.c
        pulse_audio/modules/x11/module-x11-bell.c
        pulse_audio/modules/x11/module-x11-cork-request.c
        pulse_audio/modules/x11/module-x11-publish.c
        pulse_audio/modules/x11/module-x11-xsmp.c
        pulse_audio/modules/module-allow-passthrough.c
        pulse_audio/modules/module-always-sink.c
        pulse_audio/modules/module-always-source.c
        pulse_audio/modules/module-augment-properties.c
        pulse_audio/modules/module-card-restore.c
        pulse_audio/modules/module-cli.c
        pulse_audio/modules/module-combine.c
        pulse_audio/modules/module-combine-sink.c
        pulse_audio/modules/module-console-kit.c
        pulse_audio/modules/module-default-device-restore.c
        pulse_audio/modules/module-detect.c
        pulse_audio/modules/module-device-manager.c
        pulse_audio/modules/module-device-restore.c
        pulse_audio/modules/module-equalizer-sink.c
        pulse_audio/modules/module-esound-compat-spawnfd.c
        pulse_audio/modules/module-esound-compat-spawnpid.c
        pulse_audio/modules/module-esound-sink.c
        pulse_audio/modules/module-filter-apply.c
        pulse_audio/modules/module-filter-heuristics.c
        pulse_audio/modules/module-hal-detect-compat.c
        pulse_audio/modules/module-intended-roles.c
        pulse_audio/modules/module-ladspa-sink.c
        pulse_audio/modules/module-lirc.c
        pulse_audio/modules/module-loopback.c
        pulse_audio/modules/module-match.c
        pulse_audio/modules/module-mmkbd-evdev.c
        pulse_audio/modules/module-native-protocol-fd.c
        pulse_audio/modules/module-null-sink.c
        pulse_audio/modules/module-null-source.c
        pulse_audio/modules/module-pipe-sink.c
        pulse_audio/modules/module-pipe-source.c
        pulse_audio/modules/module-position-event-sounds.c
        pulse_audio/modules/module-protocol-stub.c
        pulse_audio/modules/module-remap-sink.c
        pulse_audio/modules/module-remap-source.c
        pulse_audio/modules/module-rescue-streams.c
        pulse_audio/modules/module-role-cork.c
        pulse_audio/modules/module-role-ducking.c
        pulse_audio/modules/module-rygel-media-server.c
        pulse_audio/modules/module-sine.c
        pulse_audio/modules/module-sine-source.c
        pulse_audio/modules/module-solaris.c
        pulse_audio/modules/module-stream-restore.c
        pulse_audio/modules/module-suspend-on-idle.c
        pulse_audio/modules/module-switch-on-connect.c
        pulse_audio/modules/module-switch-on-port-available.c
        pulse_audio/modules/module-systemd-login.c
        pulse_audio/modules/module-tunnel.c
        pulse_audio/modules/module-tunnel-sink-new.c
        pulse_audio/modules/module-tunnel-source-new.c
        pulse_audio/modules/module-udev-detect.c
        pulse_audio/modules/module-virtual-sink.c
        pulse_audio/modules/module-virtual-source.c
        pulse_audio/modules/module-virtual-surround-sink.c
        pulse_audio/modules/module-volume-restore.c
        pulse_audio/modules/module-waveout.c
        pulse_audio/modules/module-zeroconf-discover.c
        pulse_audio/modules/module-zeroconf-publish.c
        pulse_audio/modules/reserve.c
        pulse_audio/modules/reserve-monitor.c
        pulse_audio/modules/reserve-wrap.c
        pulse_audio/modules/stdin-util.c
        pulse_audio/modules/stream-interaction.c
        pulse_audio/modules/udev-util.c
        pulse_audio/pulse/channelmap.c
        pulse_audio/pulse/client-conf.c
        pulse_audio/pulse/client-conf-x11.c
        pulse_audio/pulse/context.c
        pulse_audio/pulse/direction.c
        pulse_audio/pulse/error.c
        pulse_audio/pulse/ext-device-manager.c
        pulse_audio/pulse/ext-device-restore.c
        pulse_audio/pulse/ext-stream-restore.c
        pulse_audio/pulse/fork-detect.c
        pulse_audio/pulse/format.c
        pulse_audio/pulse/glib-mainloop.c
        pulse_audio/pulse/introspect.c
        pulse_audio/pulse/json.c
        pulse_audio/pulse/mainloop.c
        pulse_audio/pulse/mainloop-api.c
        pulse_audio/pulse/mainloop-signal.c
        pulse_audio/pulse/operation.c
        pulse_audio/pulse/proplist.c
        pulse_audio/pulse/rtclock.c
        pulse_audio/pulse/sample.c
        pulse_audio/pulse/scache.c
        pulse_audio/pulse/simple.c
        pulse_audio/pulse/stream.c
        pulse_audio/pulse/subscribe.c
        pulse_audio/pulse/thread-mainloop.c
        pulse_audio/pulse/timeval.c
        pulse_audio/pulse/utf8.c
        pulse_audio/pulse/util.c
        pulse_audio/pulse/volume.c
        pulse_audio/pulse/xmalloc.c
        pulse_audio/pulsecore/ffmpeg/resample2.c
        pulse_audio/pulsecore/filter/biquad.c
        pulse_audio/pulsecore/filter/crossover.c
        pulse_audio/pulsecore/filter/lfe-filter.c
        pulse_audio/pulsecore/resampler/ffmpeg.c
        pulse_audio/pulsecore/resampler/libsamplerate.c
        pulse_audio/pulsecore/resampler/peaks.c
        pulse_audio/pulsecore/resampler/soxr.c
        pulse_audio/pulsecore/resampler/speex.c
        pulse_audio/pulsecore/resampler/trivial.c
        pulse_audio/pulsecore/arpa-inet.c
        pulse_audio/pulsecore/asyncmsgq.c
        pulse_audio/pulsecore/asyncq.c
        pulse_audio/pulsecore/aupdate.c
        pulse_audio/pulsecore/auth-cookie.c
        pulse_audio/pulsecore/authkey.c
        pulse_audio/pulsecore/avahi-wrap.c
        pulse_audio/pulsecore/bitset.c
        pulse_audio/pulsecore/card.c
        pulse_audio/pulsecore/cli.c
        pulse_audio/pulsecore/cli-command.c
        pulse_audio/pulsecore/cli-text.c
        pulse_audio/pulsecore/client.c
        pulse_audio/pulsecore/conf-parser.c
        pulse_audio/pulsecore/core.c
        pulse_audio/pulsecore/core-error.c
        pulse_audio/pulsecore/core-format.c
        pulse_audio/pulsecore/core-rtclock.c
        pulse_audio/pulsecore/core-scache.c
        pulse_audio/pulsecore/core-subscribe.c
        pulse_audio/pulsecore/core-util.c
        pulse_audio/pulsecore/cpu.c
        pulse_audio/pulsecore/cpu-arm.c
        pulse_audio/pulsecore/cpu-orc.c
        pulse_audio/pulsecore/cpu-x86.c
        pulse_audio/pulsecore/database.c
        pulse_audio/pulsecore/database-gdbm.c
        pulse_audio/pulsecore/database-simple.c
        pulse_audio/pulsecore/database-tdb.c
        pulse_audio/pulsecore/dbus-shared.c
        pulse_audio/pulsecore/dbus-util.c
        pulse_audio/pulsecore/device-port.c
        pulse_audio/pulsecore/dllmain.c
        pulse_audio/pulsecore/dynarray.c
        pulse_audio/pulsecore/fdsem.c
        pulse_audio/pulsecore/flist.c
        pulse_audio/pulsecore/g711.c
        pulse_audio/pulsecore/hashmap.c
        pulse_audio/pulsecore/hook-list.c
        pulse_audio/pulsecore/i18n.c
        pulse_audio/pulsecore/idxset.c
        pulse_audio/pulsecore/iochannel.c
        pulse_audio/pulsecore/ioline.c
        pulse_audio/pulsecore/ipacl.c
        pulse_audio/pulsecore/lock-autospawn.c
        pulse_audio/pulsecore/log.c
        pulse_audio/pulsecore/ltdl-helper.c
        pulse_audio/pulsecore/mcalign.c
        pulse_audio/pulsecore/memblock.c
        pulse_audio/pulsecore/memblockq.c
        pulse_audio/pulsecore/memchunk.c
        pulse_audio/pulsecore/memtrap.c
        pulse_audio/pulsecore/message-handler.c
        pulse_audio/pulsecore/mime-type.c
        pulse_audio/pulsecore/mix.c
        pulse_audio/pulsecore/mix_neon.c
        pulse_audio/pulsecore/modargs.c
        pulse_audio/pulsecore/modinfo.c
        pulse_audio/pulsecore/module.c
        pulse_audio/pulsecore/msgobject.c
        pulse_audio/pulsecore/mutex-posix.c
        pulse_audio/pulsecore/mutex-win32.c
        pulse_audio/pulsecore/namereg.c
        pulse_audio/pulsecore/native-common.c
        pulse_audio/pulsecore/object.c
        pulse_audio/pulsecore/once.c
        pulse_audio/pulsecore/packet.c
        pulse_audio/pulsecore/parseaddr.c
        pulse_audio/pulsecore/pdispatch.c
        pulse_audio/pulsecore/pid.c
        pulse_audio/pulsecore/pipe.c
        pulse_audio/pulsecore/play-memblockq.c
        pulse_audio/pulsecore/play-memchunk.c
        pulse_audio/pulsecore/poll-posix.c
        pulse_audio/pulsecore/poll-win32.c
        pulse_audio/pulsecore/proplist-util.c
        pulse_audio/pulsecore/protocol-cli.c
        pulse_audio/pulsecore/protocol-dbus.c
        pulse_audio/pulsecore/protocol-esound.c
        pulse_audio/pulsecore/protocol-http.c
        pulse_audio/pulsecore/protocol-native.c
        pulse_audio/pulsecore/protocol-simple.c
        pulse_audio/pulsecore/pstream.c
        pulse_audio/pulsecore/pstream-util.c
        pulse_audio/pulsecore/queue.c
        pulse_audio/pulsecore/random.c
        pulse_audio/pulsecore/ratelimit.c
        pulse_audio/pulsecore/remap.c
        pulse_audio/pulsecore/remap_mmx.c
        pulse_audio/pulsecore/remap_neon.c
        pulse_audio/pulsecore/remap_sse.c
        pulse_audio/pulsecore/resampler
        pulse_audio/pulsecore/rtkit.c
        pulse_audio/pulsecore/rtpoll.c
        pulse_audio/pulsecore/sample-util.c
        pulse_audio/pulsecore/sconv.c
        pulse_audio/pulsecore/sconv-s16be.c
        pulse_audio/pulsecore/sconv-s16le.c
        pulse_audio/pulsecore/sconv_neon.c
        pulse_audio/pulsecore/sconv_sse.c
        pulse_audio/pulsecore/semaphore-osx.c
        pulse_audio/pulsecore/semaphore-posix.c
        pulse_audio/pulsecore/semaphore-win32.c
        pulse_audio/pulsecore/shared.c
        pulse_audio/pulsecore/shm.c
        pulse_audio/pulsecore/sink.c
        pulse_audio/pulsecore/sink-input.c
        pulse_audio/pulsecore/sioman.c
        pulse_audio/pulsecore/sndfile-util.c
        pulse_audio/pulsecore/socket-client.c
        pulse_audio/pulsecore/socket-server.c
        pulse_audio/pulsecore/socket-util.c
        pulse_audio/pulsecore/sound-file.c
        pulse_audio/pulsecore/sound-file-stream.c
        pulse_audio/pulsecore/source.c
        pulse_audio/pulsecore/source-output.c
        pulse_audio/pulsecore/srbchannel.c
        pulse_audio/pulsecore/start-child.c
        pulse_audio/pulsecore/strbuf.c
        pulse_audio/pulsecore/stream-util.c
        pulse_audio/pulsecore/strlist.c
        pulse_audio/pulsecore/svolume_arm.c
        pulse_audio/pulsecore/svolume_c.c
        pulse_audio/pulsecore/svolume_mmx.c
        pulse_audio/pulsecore/svolume_orc.c
        pulse_audio/pulsecore/svolume_sse.c
        pulse_audio/pulsecore/tagstruct.c
        pulse_audio/pulsecore/thread-mq.c
        pulse_audio/pulsecore/thread-posix.c
        pulse_audio/pulsecore/thread-win32.c
        pulse_audio/pulsecore/time-smoother.c
        pulse_audio/pulsecore/tokenizer.c
        pulse_audio/pulsecore/usergroup.c
        pulse_audio/pulsecore/x11prop.c
        pulse_audio/pulsecore/x11wrap.c
        pulse_audio/tests/alsa-mixer-path-test.c
        pulse_audio/tests/alsa-time-test.c
        pulse_audio/tests/asyncmsgq-test.c
        pulse_audio/tests/asyncq-test.c
        pulse_audio/tests/atomic-test.c
        pulse_audio/tests/channelmap-test.c
        pulse_audio/tests/close-test.c
        pulse_audio/tests/connect-stress.c
        pulse_audio/tests/core-util-test.c
        pulse_audio/tests/cpu-mix-test.c
        pulse_audio/tests/cpu-remap-test.c
        pulse_audio/tests/cpu-sconv-test.c
        pulse_audio/tests/cpu-volume-test.c
        pulse_audio/tests/cpulimit-test.c
        pulse_audio/tests/extended-test.c
        pulse_audio/tests/flist-test.c
        pulse_audio/tests/format-test.c
        pulse_audio/tests/get-binary-name-test.c
        pulse_audio/tests/gtk-test.c
        pulse_audio/tests/hashmap-test.c
        pulse_audio/tests/hook-list-test.c
        pulse_audio/tests/interpol-test.c
        pulse_audio/tests/ipacl-test.c
        pulse_audio/tests/json-test.c
        pulse_audio/tests/lfe-filter-test.c
        pulse_audio/tests/lo-latency-test.c
        pulse_audio/tests/lo-test-util.c
        pulse_audio/tests/lock-autospawn-test.c
        pulse_audio/tests/mainloop-test.c
        pulse_audio/tests/mcalign-test.c
        pulse_audio/tests/memblock-test.c
        pulse_audio/tests/memblockq-test.c
        pulse_audio/tests/mix-test.c
        pulse_audio/tests/mult-s16-test.c
        pulse_audio/tests/once-test.c
        pulse_audio/tests/pacat-simple.c
        pulse_audio/tests/parec-simple.c
        pulse_audio/tests/passthrough-test.c
        pulse_audio/tests/proplist-test.c
        pulse_audio/tests/queue-test.c
        pulse_audio/tests/remix-test.c
        pulse_audio/tests/resampler-test.c
        pulse_audio/tests/rtpoll-test.c
        pulse_audio/tests/rtstutter.c
        pulse_audio/tests/sig2str-test.c
        pulse_audio/tests/sigbus-test.c
        pulse_audio/tests/smoother-test.c
        pulse_audio/tests/srbchannel-test.c
        pulse_audio/tests/stripnul.c
        pulse_audio/tests/strlist-test.c
        pulse_audio/tests/sync-playback.c
        pulse_audio/tests/thread-mainloop-test.c
        pulse_audio/tests/thread-test.c
        pulse_audio/tests/usergroup-test.c
        pulse_audio/tests/utf8-test.c
        pulse_audio/tests/volume-test.c
        pulse_audio/utils/pacat.c
        pulse_audio/utils/pacmd.c
        pulse_audio/utils/pactl.c
        pulse_audio/utils/padsp.c
        pulse_audio/utils/pasuspender.c
        pulse_audio/utils/pax11publish.c
        pulse_audio/model.c
        registration_manager/CustomerDataHandler.cpp
        registration_manager/CustomerDataManager.cpp
        registration_manager/CustomerDataManagerTest.cpp
        registration_manager/RegistrationManager.cpp
        registration_manager/RegistrationManagerTest.cpp
        sbc/sbc.c
        sbc/sbc_primitives.c
        sbc/sbc_primitives_armv6.c
        sbc/sbc_primitives_iwmmxt.c
        sbc/sbc_primitives_mmx.c
        sbc/sbc_primitives_neon.c
        sbc/sbcdec.c
        sbc/sbcenc.c
        sbc/sbcinfo.c
        sbc/sbctester.c
        sdkinterfaces/EqualizerStorageInterfaceTest.cpp
        sdkinterfaces/StubMiscStorage.cpp
        settings/Storage/SQLiteDeviceSettingStorage.cpp
        settings/Types/LocaleWakeWordsSetting.cpp
        settings/Types/NetworkInfo.cpp
        settings/CloudControlledSettingProtocol.cpp
        settings/DeviceControlledSettingProtocol.cpp
        settings/LocaleWakeWordsSettingTest.cpp
        settings/NetworkInfoTest.cpp
        settings/SettingCallbackAdapterTest.cpp
        settings/SettingCallbacksTest.cpp
        settings/SettingConnectionObserver.cpp
        settings/SettingEventRequestObserver.cpp
        settings/SettingEventSender.cpp
        settings/SettingEventSenderTest.cpp
        settings/SettingsManagerTest.cpp
        settings/SettingStringConversionTest.cpp
        settings/SettingTest.cpp
        settings/SharedAVSSettingProtocol.cpp
        settings/SharedAVSSettingProtocolTest.cpp
        settings/SQLiteDeviceSettingStorageTest.cpp
        shared/CookBook.cpp
        shared/ManufactoryTest.cpp
        shared/ManufactoryUtilsTest.cpp
        shared/SharedComponent.cpp
        speech_enconder/OpusEncoderContext.cpp
        speech_enconder/SpeechEncoder.cpp
        speech_enconder/SpeechEncoderTest.cpp
        storage/sqlite3.c
        storage/SQLiteDatabase.cpp
        storage/SQLiteDatabaseTest.cpp
        storage/SQLiteMiscStorage.cpp
        storage/SQLiteMiscStorageTest.cpp
        storage/SQLiteStatement.cpp
        storage/SQLiteUtils.cpp
        synchronize_state_sender/PostConnectSynchronizeStateSender.cpp
        synchronize_state_sender/PostConnectSynchronizeStateSenderTest.cpp
        synchronize_state_sender/SynchronizeStateSenderFactory.cpp
        timing/SafeCTimeAccess.cpp
        util/bluetooth/SDPRecords.cpp
        util/HTTP2/HTTP2GetMimeHeadersResult.cpp
        util/HTTP2/HTTP2MimeRequestEncoder.cpp
        util/HTTP2/HTTP2MimeResponseDecoder.cpp
        util/HTTP2/HTTP2SendDataResult.cpp
        util/sds/BufferLayout.cpp
        util/sds/Reader.cpp
        util/sds/Writer.cpp
        util/stream/Streambuf.cpp
        util/stream/StreamFunctions.cpp
        util/BluetoothEventBus.cpp
        util/BufferLayout.cpp
        util/DeviceInfo.cpp
        util/Executor.cpp
        util/FileUtils.cpp
        util/FormattedAudioStreamAdapter.cpp
        util/MacAddressString.cpp
        util/Metrics.cpp
        util/MultiTimer.cpp
        util/Reader.cpp
        util/RequiresShutdown.cpp
        util/RetryTimer.cpp
        util/SafeCTimeAccess.cpp
        util/SharedDataStream.cpp
        util/Stopwatch.cpp
        util/StringUtils.cpp
        util/SystemClockMonitor.cpp
        util/TaskThread.cpp
        util/ThreadPool.cpp
        util/TimePoint.cpp
        util/Timer.cpp
        util/TimeUtils.cpp
        util/UUIDGeneration.cpp
        util/WaitEvent.cpp
        util/WorkerThread.cpp
        util/Writer.cpp
        webvtt/webvtt/alloc.c
        webvtt/webvtt/cue.c
        webvtt/webvtt/cuetext.c
        webvtt/webvtt/error.c
        webvtt/webvtt/lexer.c
        webvtt/webvtt/node.c
        webvtt/webvtt/parser.c
        webvtt/webvtt/string.c
        webvtt/webvttxx/abstract_parser.cpp
        webvtt/webvttxx/file_parser.cpp
        webvtt/parsevtt_main.c
        xcb/check_all.c
        xcb/check_public.c
        xcb/xcb_auth.c
        xcb/xcb_conn.c
        xcb/xcb_ext.c
        xcb/xcb_in.c
        xcb/xcb_list.c
        xcb/xcb_out.c
        xcb/xcb_util.c
        xcb/xcb_xid.c]]
        )
target_sources(alexa PUBLIC acl/Transport/Common/MockHTTP2Connection.cpp)
target_sources(alexa PUBLIC acl/Transport/Common/MockHTTP2Request.cpp)
target_sources(alexa PUBLIC acl/Transport/Common/MockMimeResponseSink.cpp)
target_sources(alexa PUBLIC acl/Transport/AVSConnectionManagerTest.cpp)
target_sources(alexa PUBLIC acl/Transport/DownchannelHandler.cpp)
target_sources(alexa PUBLIC acl/Transport/ExchangeHandler.cpp)
target_sources(alexa PUBLIC acl/Transport/HTTP2Transport.cpp)
target_sources(alexa PUBLIC acl/Transport/HTTP2TransportFactory.cpp)
target_sources(alexa PUBLIC acl/Transport/HTTP2TransportTest.cpp)
target_sources(alexa PUBLIC acl/Transport/MessageRequestHandler.cpp)
target_sources(alexa PUBLIC acl/Transport/MessageRequestHandlerTest.cpp)
target_sources(alexa PUBLIC acl/Transport/MessageRequestQueue.cpp)
target_sources(alexa PUBLIC acl/Transport/MessageRouter.cpp)
target_sources(alexa PUBLIC acl/Transport/MessageRouterFactory.cpp)
target_sources(alexa PUBLIC acl/Transport/MessageRouterTest.cpp)
target_sources(alexa PUBLIC acl/Transport/MimeResponseSink.cpp)
target_sources(alexa PUBLIC acl/Transport/PingHandler.cpp)
target_sources(alexa PUBLIC acl/Transport/PostConnectSequencer.cpp)
target_sources(alexa PUBLIC acl/Transport/PostConnectSequencerFactory.cpp)
target_sources(alexa PUBLIC acl/Transport/PostConnectSequencerFactoryTest.cpp)
target_sources(alexa PUBLIC acl/Transport/PostConnectSequencerTest.cpp)
target_sources(alexa PUBLIC acl/Transport/SynchronizedMessageRequestQueue.cpp)
target_sources(alexa PUBLIC acl/Transport/TransportDefines.cpp)
target_sources(alexa PUBLIC acl/AVSConnectionManager.cpp)
target_sources(alexa PUBLIC acsdk_alerts/Renderer/Renderer.cpp)
target_sources(alexa PUBLIC acsdk_alerts/Storage/SQLiteAlertStorage.cpp)
target_sources(alexa PUBLIC acsdk_alerts/Alarm.cpp)
target_sources(alexa PUBLIC acsdk_alerts/AlarmAlertTest.cpp)
target_sources(alexa PUBLIC acsdk_alerts/Alert.cpp)
target_sources(alexa PUBLIC acsdk_alerts/AlertsCapabilityAgent.cpp)
target_sources(alexa PUBLIC acsdk_alerts/AlertsCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC acsdk_alerts/AlertScheduler.cpp)
target_sources(alexa PUBLIC acsdk_alerts/AlertSchedulerTest.cpp)
target_sources(alexa PUBLIC acsdk_alerts/AlertTest.cpp)
target_sources(alexa PUBLIC acsdk_alerts/Reminder.cpp)
target_sources(alexa PUBLIC acsdk_alerts/ReminderAlertTest.cpp)
target_sources(alexa PUBLIC acsdk_alerts/RendererTest.cpp)
target_sources(alexa PUBLIC acsdk_alerts/Timer.cpp)
target_sources(alexa PUBLIC acsdk_alerts/TimerAlertTest.cpp)
target_sources(alexa PUBLIC acsdk_alerts_interfaces/AlertObserverInterface.cpp)
target_sources(alexa PUBLIC adsl/common/MockDirectiveHandler.cpp)
target_sources(alexa PUBLIC adsl/DirectiveProcessor.cpp)
target_sources(alexa PUBLIC adsl/DirectiveProcessorTest.cpp)
target_sources(alexa PUBLIC adsl/DirectiveRouter.cpp)
target_sources(alexa PUBLIC adsl/DirectiveRouterTest.cpp)
target_sources(alexa PUBLIC adsl/DirectiveSequencer.cpp)
target_sources(alexa PUBLIC adsl/DirectiveSequencerTest.cpp)
target_sources(alexa PUBLIC adsl/MessageInterpreter.cpp)
target_sources(alexa PUBLIC adsl/MessageInterpreterTest.cpp)
target_sources(alexa PUBLIC afml/AudioActivityTracker.cpp)
target_sources(alexa PUBLIC afml/AudioActivityTrackerTest.cpp)
target_sources(alexa PUBLIC afml/Channel.cpp)
target_sources(alexa PUBLIC afml/FocusManager.cpp)
target_sources(alexa PUBLIC afml/FocusManagerTest.cpp)
target_sources(alexa PUBLIC afml/VisualActivityTracker.cpp)
target_sources(alexa PUBLIC afml/VisualActivityTrackerTest.cpp)
target_sources(alexa PUBLIC app_utilities/android_utilities/AndroidLogger.cpp)
target_sources(alexa PUBLIC app_utilities/android_utilities/AndroidSLESBufferQueue.cpp)
target_sources(alexa PUBLIC app_utilities/android_utilities/AndroidSLESEngine.cpp)
target_sources(alexa PUBLIC app_utilities/android_utilities/AndroidSLESMicrophone.cpp)
target_sources(alexa PUBLIC app_utilities/android_utilities/AndroidSLESObject.cpp)
target_sources(alexa PUBLIC app_utilities/default_client/ConnectionRetryTrigger.cpp)
target_sources(alexa PUBLIC app_utilities/default_client/DefaultClient.cpp)
target_sources(alexa PUBLIC app_utilities/default_client/DeviceSettingsManagerBuilder.cpp)
target_sources(alexa PUBLIC app_utilities/default_client/EqualizerRuntimeSetup.cpp)
target_sources(alexa PUBLIC app_utilities/sdkcomponent/SDKComponent.cpp)
target_sources(alexa PUBLIC app_utilities/sdkcomponent/SDKComponentTest.cpp)
target_sources(alexa PUBLIC app_utilities/system_sound_player/SystemSoundPlayer.cpp)
target_sources(alexa PUBLIC app_utilities/system_sound_player/SystemSoundPlayerTest.cpp)
target_sources(alexa PUBLIC autorization/CBLAuthDelegate/CBLAuthDelegate.cpp)
target_sources(alexa PUBLIC autorization/CBLAuthDelegate/CBLAuthDelegateConfiguration.cpp)
target_sources(alexa PUBLIC autorization/CBLAuthDelegate/SQLiteCBLAuthDelegateStorage.cpp)
target_sources(alexa PUBLIC autorization/DefaultEndpoint/DefaultEndpointModeControllerHandler.cpp)
target_sources(alexa PUBLIC autorization/DefaultEndpoint/DefaultEndpointRangeControllerHandler.cpp)
target_sources(alexa PUBLIC autorization/DefaultEndpoint/DefaultEndpointToggleControllerHandler.cpp)
target_sources(alexa PUBLIC autorization/PeripheralEndpoint/PeripheralEndpointModeControllerHandler.cpp)
target_sources(alexa PUBLIC autorization/PeripheralEndpoint/PeripheralEndpointPowerControllerHandler.cpp)
target_sources(alexa PUBLIC autorization/PeripheralEndpoint/PeripheralEndpointRangeControllerHandler.cpp)
target_sources(alexa PUBLIC autorization/PeripheralEndpoint/PeripheralEndpointToggleControllerHandler.cpp)
target_sources(alexa PUBLIC autorization/CaptionPresenter.cpp)
target_sources(alexa PUBLIC autorization/ConnectionObserver.cpp)
target_sources(alexa PUBLIC autorization/ConsolePrinter.cpp)
target_sources(alexa PUBLIC autorization/ConsoleReader.cpp)
target_sources(alexa PUBLIC autorization/ExternalCapabilitiesBuilder.cpp)
target_sources(alexa PUBLIC autorization/GuiRenderer.cpp)
target_sources(alexa PUBLIC autorization/InteractionManager.cpp)
target_sources(alexa PUBLIC autorization/KeywordObserver.cpp)
target_sources(alexa PUBLIC autorization/LocaleAssetsManager.cpp)
target_sources(alexa PUBLIC autorization/main.cpp)
target_sources(alexa PUBLIC autorization/PortAudioMicrophoneWrapper.cpp)
target_sources(alexa PUBLIC autorization/RevokeAuthorizationObserver.cpp)
target_sources(alexa PUBLIC autorization/SampleApplication.cpp)
target_sources(alexa PUBLIC autorization/SampleApplicationComponent.cpp)
target_sources(alexa PUBLIC autorization/SampleEqualizerModeController.cpp)
target_sources(alexa PUBLIC autorization/SDKDiagnostics.cpp)
target_sources(alexa PUBLIC autorization/UIManager.cpp)
target_sources(alexa PUBLIC autorization/UserInputManager.cpp)
target_sources(alexa PUBLIC avs/attachment/Attachment.cpp)
target_sources(alexa PUBLIC avs/attachment/AttachmentManager.cpp)
target_sources(alexa PUBLIC avs/attachment/AttachmentUtils.cpp)
target_sources(alexa PUBLIC avs/attachment/InProcessAttachment.cpp)
target_sources(alexa PUBLIC avs/attachment/InProcessAttachmentReader.cpp)
target_sources(alexa PUBLIC avs/attachment/InProcessAttachmentWriter.cpp)
target_sources(alexa PUBLIC avs/AbstractAVSConnectionManager.cpp)
target_sources(alexa PUBLIC avs/AlexaClientSDKInit.cpp)
target_sources(alexa PUBLIC avs/AVSContext.cpp)
target_sources(alexa PUBLIC avs/AVSDirective.cpp)
target_sources(alexa PUBLIC avs/AVSMessage.cpp)
target_sources(alexa PUBLIC avs/AVSMessageHeader.cpp)
target_sources(alexa PUBLIC avs/BlockingPolicy.cpp)
target_sources(alexa PUBLIC avs/CapabilityAgent.cpp)
target_sources(alexa PUBLIC avs/CapabilityConfiguration.cpp)
target_sources(alexa PUBLIC avs/CapabilityResources.cpp)
target_sources(alexa PUBLIC avs/CapabilityTag.cpp)
target_sources(alexa PUBLIC avs/ComponentConfiguration.cpp)
target_sources(alexa PUBLIC avs/DialogUXStateAggregator.cpp)
target_sources(alexa PUBLIC avs/DirectiveRoutingRule.cpp)
target_sources(alexa PUBLIC avs/EventBuilder.cpp)
target_sources(alexa PUBLIC avs/ExceptionEncounteredSender.cpp)
target_sources(alexa PUBLIC avs/HandlerAndPolicy.cpp)
target_sources(alexa PUBLIC avs/MessageRequest.cpp)
target_sources(alexa PUBLIC avs/NamespaceAndName.cpp)
target_sources(alexa PUBLIC avs/WaitableMessageRequest.cpp)
target_sources(alexa PUBLIC avs_gateway_manager/Storage/AVSGatewayManagerStorage.cpp)
target_sources(alexa PUBLIC avs_gateway_manager/Storage/AVSGatewayManagerStorageTest.cpp)
target_sources(alexa PUBLIC avs_gateway_manager/AVSGatewayManager.cpp)
target_sources(alexa PUBLIC avs_gateway_manager/AVSGatewayManagerTest.cpp)
target_sources(alexa PUBLIC avs_gateway_manager/PostConnectVerifyGatewaySender.cpp)
target_sources(alexa PUBLIC avs_gateway_manager/PostConnectVerifyGatewaySenderTest.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZA2DPSink.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZA2DPSource.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZAVRCPController.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZAVRCPTarget.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZBluetoothDevice.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZBluetoothDeviceManager.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZDeviceManager.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZHFP.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZHID.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZHostController.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/BlueZSPP.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/DBusConnection.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/DBusObjectBase.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/DBusPropertiesProxy.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/DBusProxy.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/GVariantMapReader.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/GVariantTupleReader.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/MediaContext.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/MediaEndpoint.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/MPRISPlayer.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/PairingAgent.cpp)
target_sources(alexa PUBLIC bluetooth_implementations/PulseAudioBluetoothInitializer.cpp)
target_sources(alexa PUBLIC capabilities_delegate/Storage/SQLiteCapabilitiesDelegateStorage.cpp)
target_sources(alexa PUBLIC capabilities_delegate/Storage/SQLiteCapabilitiesDelegateStorageTest.cpp)
target_sources(alexa PUBLIC capabilities_delegate/Utils/DiscoveryUtils.cpp)
target_sources(alexa PUBLIC capabilities_delegate/Utils/DiscoveryUtilsTest.cpp)
target_sources(alexa PUBLIC capabilities_delegate/CapabilitiesDelegate.cpp)
target_sources(alexa PUBLIC capabilities_delegate/CapabilitiesDelegateTest.cpp)
target_sources(alexa PUBLIC capabilities_delegate/DiscoveryEventSender.cpp)
target_sources(alexa PUBLIC capabilities_delegate/DiscoveryEventSenderTest.cpp)
target_sources(alexa PUBLIC capabilities_delegate/PostConnectCapabilitiesPublisher.cpp)
target_sources(alexa PUBLIC capabilities_delegate/PostConnectCapabilitiesPublisherTest.cpp)
target_sources(alexa PUBLIC capability/audio_player/AudioPlayer.cpp)
target_sources(alexa PUBLIC capability/audio_player/AudioPlayerTest.cpp)
target_sources(alexa PUBLIC capability/audio_player/ProgressTimer.cpp)
target_sources(alexa PUBLIC capability/audio_player/ProgressTimerTest.cpp)
target_sources(alexa PUBLIC capability/bluetooth/BasicDeviceConnectionRule.cpp)
target_sources(alexa PUBLIC capability/bluetooth/Bluetooth.cpp)
target_sources(alexa PUBLIC capability/bluetooth/BluetoothEventState.cpp)
target_sources(alexa PUBLIC capability/bluetooth/BluetoothMediaInputTransformer.cpp)
target_sources(alexa PUBLIC capability/bluetooth/BluetoothMediaInputTransformerTest.cpp)
target_sources(alexa PUBLIC capability/bluetooth/BluetoothTest.cpp)
target_sources(alexa PUBLIC capability/bluetooth/SQLiteBluetoothStorage.cpp)
target_sources(alexa PUBLIC capability/bluetooth/SQLiteBluetoothStorageTest.cpp)
target_sources(alexa PUBLIC capability/do_not_disturb/DNDMessageRequest.cpp)
target_sources(alexa PUBLIC capability/do_not_disturb/DNDSettingProtocol.cpp)
target_sources(alexa PUBLIC capability/do_not_disturb/DoNotDisturbCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability/do_not_disturb/DoNotDisturbCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability/multi_room_music/MRMCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability/multi_room_music/MRMCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability/notifications/NotificationIndicator.cpp)
target_sources(alexa PUBLIC capability/notifications/NotificationRenderer.cpp)
target_sources(alexa PUBLIC capability/notifications/NotificationRendererTest.cpp)
target_sources(alexa PUBLIC capability/notifications/NotificationsCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability/notifications/NotificationsCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability/notifications/NotificationsStorageTest.cpp)
target_sources(alexa PUBLIC capability/notifications/SQLiteNotificationsStorage.cpp)
target_sources(alexa PUBLIC capability_agents/AIP/AudioInputProcessor.cpp)
target_sources(alexa PUBLIC capability_agents/AIP/AudioInputProcessorTest.cpp)
target_sources(alexa PUBLIC capability_agents/Alexa/AlexaInterfaceCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/Alexa/AlexaInterfaceCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/Alexa/AlexaInterfaceMessageSender.cpp)
target_sources(alexa PUBLIC capability_agents/Alexa/AlexaInterfaceMessageSenderTest.cpp)
target_sources(alexa PUBLIC capability_agents/ApiGateway/ApiGatewayCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/ApiGateway/ApiGatewayCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/Equalizer/EqualizerCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/ExternalMediaPlayer/AuthorizedSender.cpp)
target_sources(alexa PUBLIC capability_agents/ExternalMediaPlayer/ExternalMediaPlayer.cpp)
target_sources(alexa PUBLIC capability_agents/ExternalMediaPlayer/ExternalMediaPlayerTest.cpp)
target_sources(alexa PUBLIC capability_agents/InteractionModel/InteractionModelCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/InteractionModel/InteractionModelCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/ModeController/ModeControllerAttributeBuilder.cpp)
target_sources(alexa PUBLIC capability_agents/ModeController/ModeControllerCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/ModeController/ModeControllerCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/Notifications/NotificationIndicator.cpp)
target_sources(alexa PUBLIC capability_agents/Notifications/NotificationRenderer.cpp)
target_sources(alexa PUBLIC capability_agents/Notifications/NotificationRendererTest.cpp)
target_sources(alexa PUBLIC capability_agents/Notifications/NotificationsCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/Notifications/NotificationsCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/Notifications/NotificationsStorageTest.cpp)
target_sources(alexa PUBLIC capability_agents/Notifications/SQLiteNotificationsStorage.cpp)
target_sources(alexa PUBLIC capability_agents/PlaybackController/PlaybackCommand.cpp)
target_sources(alexa PUBLIC capability_agents/PlaybackController/PlaybackController.cpp)
target_sources(alexa PUBLIC capability_agents/PlaybackController/PlaybackControllerTest.cpp)
target_sources(alexa PUBLIC capability_agents/PlaybackController/PlaybackMessageRequest.cpp)
target_sources(alexa PUBLIC capability_agents/PlaybackController/PlaybackRouter.cpp)
target_sources(alexa PUBLIC capability_agents/PlaybackController/PlaybackRouterTest.cpp)
target_sources(alexa PUBLIC capability_agents/PowerController/PowerControllerCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/PowerController/PowerControllerCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/RangeController/RangeControllerAttributeBuilder.cpp)
target_sources(alexa PUBLIC capability_agents/RangeController/RangeControllerCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/RangeController/RangeControllerCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/SoftwareComponentReporter/SoftwareComponentReporterCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/SoftwareComponentReporter/SoftwareComponentReporterCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC capability_agents/SpeakerManager/ChannelVolumeManager.cpp)
target_sources(alexa PUBLIC capability_agents/SpeakerManager/ChannelVolumeManagerTest.cpp)
target_sources(alexa PUBLIC capability_agents/SpeakerManager/DefaultChannelVolumeFactory.cpp)
target_sources(alexa PUBLIC capability_agents/SpeakerManager/SpeakerManager.cpp)
target_sources(alexa PUBLIC capability_agents/SpeakerManager/SpeakerManagerTest.cpp)
target_sources(alexa PUBLIC capability_agents/SpeechSynthesizer/SpeechSynthesizer.cpp)
target_sources(alexa PUBLIC capability_agents/SpeechSynthesizer/SpeechSynthesizerTest.cpp)
target_sources(alexa PUBLIC capability_agents/System/LocaleHandler.cpp)
target_sources(alexa PUBLIC capability_agents/System/LocaleHandlerTest.cpp)
target_sources(alexa PUBLIC capability_agents/System/ReportStateHandler.cpp)
target_sources(alexa PUBLIC capability_agents/System/ReportStateHandlerTest.cpp)
target_sources(alexa PUBLIC capability_agents/System/RevokeAuthorizationHandler.cpp)
target_sources(alexa PUBLIC capability_agents/System/RevokeAuthorizationHandlerTest.cpp)
target_sources(alexa PUBLIC capability_agents/System/SoftwareInfoSender.cpp)
target_sources(alexa PUBLIC capability_agents/System/SoftwareInfoSendRequest.cpp)
target_sources(alexa PUBLIC capability_agents/System/SoftwareInfoTest.cpp)
target_sources(alexa PUBLIC capability_agents/System/StateReportGenerator.cpp)
target_sources(alexa PUBLIC capability_agents/System/StateReportGeneratorTest.cpp)
target_sources(alexa PUBLIC capability_agents/System/SystemCapabilityProvider.cpp)
target_sources(alexa PUBLIC capability_agents/System/TimeZoneHandler.cpp)
target_sources(alexa PUBLIC capability_agents/System/TimeZoneHandlerTest.cpp)
target_sources(alexa PUBLIC capability_agents/System/UserInactivityMonitor.cpp)
target_sources(alexa PUBLIC capability_agents/System/UserInactivityMonitorTest.cpp)
target_sources(alexa PUBLIC capability_agents/TemplateRuntime/TemplateRuntime.cpp)
target_sources(alexa PUBLIC capability_agents/TemplateRuntime/TemplateRuntimeTest.cpp)
target_sources(alexa PUBLIC capability_agents/ToggleController/ToggleControllerAttributeBuilder.cpp)
target_sources(alexa PUBLIC capability_agents/ToggleController/ToggleControllerCapabilityAgent.cpp)
target_sources(alexa PUBLIC capability_agents/ToggleController/ToggleControllerCapabilityAgentTest.cpp)
target_sources(alexa PUBLIC captions/CaptionData.cpp)
target_sources(alexa PUBLIC captions/CaptionDataTest.cpp)
target_sources(alexa PUBLIC captions/CaptionFormatAvsTest.cpp)
target_sources(alexa PUBLIC captions/CaptionFormatUtilsTest.cpp)
target_sources(alexa PUBLIC captions/CaptionFrame.cpp)
target_sources(alexa PUBLIC captions/CaptionFrameTest.cpp)
target_sources(alexa PUBLIC captions/CaptionLine.cpp)
target_sources(alexa PUBLIC captions/CaptionLineTest.cpp)
target_sources(alexa PUBLIC captions/CaptionManager.cpp)
target_sources(alexa PUBLIC captions/CaptionManagerTest.cpp)
target_sources(alexa PUBLIC captions/CaptionTimingAdapter.cpp)
target_sources(alexa PUBLIC captions/CaptionTimingAdapterTest.cpp)
target_sources(alexa PUBLIC captions/LibwebvttParserAdapter.cpp)
target_sources(alexa PUBLIC captions/LibwebvttParserAdapterTest.cpp)
target_sources(alexa PUBLIC captions/SystemClockDelay.cpp)
target_sources(alexa PUBLIC captions/TestTimingAdapterFactory.cpp)
target_sources(alexa PUBLIC captions/TextStyle.cpp)
target_sources(alexa PUBLIC captions/TextStyleTest.cpp)
target_sources(alexa PUBLIC captions/TimingAdapterFactory.cpp)
target_sources(alexa PUBLIC certified_sender/Common/MockCertifiedSender.cpp)
target_sources(alexa PUBLIC certified_sender/CertifiedSender.cpp)
target_sources(alexa PUBLIC certified_sender/CertifiedSenderTest.cpp)
target_sources(alexa PUBLIC certified_sender/MessageStorageTest.cpp)
target_sources(alexa PUBLIC certified_sender/SQLiteMessageStorage.cpp)
target_sources(alexa PUBLIC check/test/check_check_exit.c)
target_sources(alexa PUBLIC check/test/check_check_export_main.c)
target_sources(alexa PUBLIC check/test/check_check_fixture.c)
target_sources(alexa PUBLIC check/test/check_check_fork.c)
target_sources(alexa PUBLIC check/test/check_check_limit.c)
target_sources(alexa PUBLIC check/test/check_check_log.c)
target_sources(alexa PUBLIC check/test/check_check_log_internal.c)
target_sources(alexa PUBLIC check/test/check_check_main.c)
target_sources(alexa PUBLIC check/test/check_check_master.c)
target_sources(alexa PUBLIC check/test/check_check_msg.c)
target_sources(alexa PUBLIC check/test/check_check_pack.c)
target_sources(alexa PUBLIC check/test/check_check_selective.c)
target_sources(alexa PUBLIC check/test/check_check_sub.c)
target_sources(alexa PUBLIC check/test/check_check_tags.c)
target_sources(alexa PUBLIC check/test/check_list.c)
target_sources(alexa PUBLIC check/test/check_mem_leaks.c)
target_sources(alexa PUBLIC check/test/check_nofork.c)
target_sources(alexa PUBLIC check/test/check_nofork_teardown.c)
target_sources(alexa PUBLIC check/test/check_set_max_msg_size.c)
target_sources(alexa PUBLIC check/test/check_stress.c)
target_sources(alexa PUBLIC check/test/check_thread_stress.c)
target_sources(alexa PUBLIC check/test/ex_output.c)
target_sources(alexa PUBLIC check/test/test_suite.c)
target_sources(alexa PUBLIC check/test/tests.c)
target_sources(alexa PUBLIC check/alarm.c)
target_sources(alexa PUBLIC check/check.c)
target_sources(alexa PUBLIC check/check_error.c)
target_sources(alexa PUBLIC check/check_list.c)
target_sources(alexa PUBLIC check/check_log.c)
target_sources(alexa PUBLIC check/check_msg.c)
target_sources(alexa PUBLIC check/check_pack.c)
target_sources(alexa PUBLIC check/check_print.c)
target_sources(alexa PUBLIC check/check_run.c)
target_sources(alexa PUBLIC check/check_str.c)
target_sources(alexa PUBLIC check/clock_gettime.c)
target_sources(alexa PUBLIC check/fpclassify.c)
target_sources(alexa PUBLIC check/getline.c)
target_sources(alexa PUBLIC check/gettimeofday.c)
target_sources(alexa PUBLIC check/libcompat.c)
target_sources(alexa PUBLIC check/localtime_r.c)
target_sources(alexa PUBLIC check/malloc.c)
target_sources(alexa PUBLIC check/pthread_mutex.c)
target_sources(alexa PUBLIC check/realloc.c)
target_sources(alexa PUBLIC check/snprintf.c)
target_sources(alexa PUBLIC check/strdup.c)
target_sources(alexa PUBLIC check/strsignal.c)
target_sources(alexa PUBLIC check/timer_create.c)
target_sources(alexa PUBLIC check/timer_delete.c)
target_sources(alexa PUBLIC check/timer_settime.c)
target_sources(alexa PUBLIC configuration/ConfigurationNode.cpp)
target_sources(alexa PUBLIC context_manager/ContextManager.cpp)
target_sources(alexa PUBLIC context_manager/ContextManagerTest.cpp)
target_sources(alexa PUBLIC core/CoreComponent.cpp)
target_sources(alexa PUBLIC curl/vauth/cleartext.c)
target_sources(alexa PUBLIC curl/vauth/cram.c)
target_sources(alexa PUBLIC curl/vauth/digest.c)
target_sources(alexa PUBLIC curl/vauth/digest_sspi.c)
target_sources(alexa PUBLIC curl/vauth/krb5_gssapi.c)
target_sources(alexa PUBLIC curl/vauth/krb5_sspi.c)
target_sources(alexa PUBLIC curl/vauth/ntlm.c)
target_sources(alexa PUBLIC curl/vauth/ntlm_sspi.c)
target_sources(alexa PUBLIC curl/vauth/oauth2.c)
target_sources(alexa PUBLIC curl/vauth/spnego_gssapi.c)
target_sources(alexa PUBLIC curl/vauth/spnego_sspi.c)
target_sources(alexa PUBLIC curl/vauth/vauth.c)
target_sources(alexa PUBLIC curl/vquic/ngtcp2.c)
target_sources(alexa PUBLIC curl/vquic/quiche.c)
target_sources(alexa PUBLIC curl/vquic/vquic.c)
target_sources(alexa PUBLIC curl/vssh/libssh.c)
target_sources(alexa PUBLIC curl/vssh/libssh2.c)
target_sources(alexa PUBLIC curl/vssh/wolfssh.c)
target_sources(alexa PUBLIC curl/vtls/bearssl.c)
target_sources(alexa PUBLIC curl/vtls/gskit.c)
target_sources(alexa PUBLIC curl/vtls/gtls.c)
target_sources(alexa PUBLIC curl/vtls/keylog.c)
target_sources(alexa PUBLIC curl/vtls/mbedtls.c)
target_sources(alexa PUBLIC curl/vtls/mbedtls_threadlock.c)
target_sources(alexa PUBLIC curl/vtls/mesalink.c)
target_sources(alexa PUBLIC curl/vtls/nss.c)
target_sources(alexa PUBLIC curl/vtls/openssl.c)
target_sources(alexa PUBLIC curl/vtls/schannel.c)
target_sources(alexa PUBLIC curl/vtls/schannel_verify.c)
target_sources(alexa PUBLIC curl/vtls/sectransp.c)
target_sources(alexa PUBLIC curl/vtls/vtls.c)
target_sources(alexa PUBLIC curl/vtls/wolfssl.c)
target_sources(alexa PUBLIC curl/altsvc.c)
target_sources(alexa PUBLIC curl/amigaos.c)
target_sources(alexa PUBLIC curl/asyn-ares.c)
target_sources(alexa PUBLIC curl/asyn-thread.c)
target_sources(alexa PUBLIC curl/base64.c)
target_sources(alexa PUBLIC curl/conncache.c)
target_sources(alexa PUBLIC curl/connect.c)
target_sources(alexa PUBLIC curl/content_encoding.c)
target_sources(alexa PUBLIC curl/cookie.c)
target_sources(alexa PUBLIC curl/curl_addrinfo.c)
target_sources(alexa PUBLIC curl/curl_ctype.c)
target_sources(alexa PUBLIC curl/curl_endian.c)
target_sources(alexa PUBLIC curl/curl_fnmatch.c)
target_sources(alexa PUBLIC curl/curl_get_line.c)
target_sources(alexa PUBLIC curl/curl_gethostname.c)
target_sources(alexa PUBLIC curl/curl_gssapi.c)
target_sources(alexa PUBLIC curl/curl_memrchr.c)
target_sources(alexa PUBLIC curl/curl_multibyte.c)
target_sources(alexa PUBLIC curl/curl_ntlm_core.c)
target_sources(alexa PUBLIC curl/curl_ntlm_wb.c)
target_sources(alexa PUBLIC curl/curl_path.c)
target_sources(alexa PUBLIC curl/curl_range.c)
target_sources(alexa PUBLIC curl/curl_rtmp.c)
target_sources(alexa PUBLIC curl/curl_sasl.c)
target_sources(alexa PUBLIC curl/curl_sspi.c)
target_sources(alexa PUBLIC curl/curl_threads.c)
target_sources(alexa PUBLIC curl/dict.c)
target_sources(alexa PUBLIC curl/doh.c)
target_sources(alexa PUBLIC curl/dotdot.c)
target_sources(alexa PUBLIC curl/dynbuf.c)
target_sources(alexa PUBLIC curl/escape.c)
target_sources(alexa PUBLIC curl/file.c)
target_sources(alexa PUBLIC curl/fileinfo.c)
target_sources(alexa PUBLIC curl/formdata.c)
target_sources(alexa PUBLIC curl/ftp.c)
target_sources(alexa PUBLIC curl/ftplistparser.c)
target_sources(alexa PUBLIC curl/getenv.c)
target_sources(alexa PUBLIC curl/getinfo.c)
target_sources(alexa PUBLIC curl/gopher.c)
target_sources(alexa PUBLIC curl/hash.c)
target_sources(alexa PUBLIC curl/hmac.c)
target_sources(alexa PUBLIC curl/hostasyn.c)
target_sources(alexa PUBLIC curl/hostcheck.c)
target_sources(alexa PUBLIC curl/hostip.c)
target_sources(alexa PUBLIC curl/hostip4.c)
target_sources(alexa PUBLIC curl/hostip6.c)
target_sources(alexa PUBLIC curl/hostsyn.c)
target_sources(alexa PUBLIC curl/http.c)
target_sources(alexa PUBLIC curl/http2.c)
target_sources(alexa PUBLIC curl/http_chunks.c)
target_sources(alexa PUBLIC curl/http_digest.c)
target_sources(alexa PUBLIC curl/http_negotiate.c)
target_sources(alexa PUBLIC curl/http_ntlm.c)
target_sources(alexa PUBLIC curl/http_proxy.c)
target_sources(alexa PUBLIC curl/idn_win32.c)
target_sources(alexa PUBLIC curl/if2ip.c)
target_sources(alexa PUBLIC curl/imap.c)
target_sources(alexa PUBLIC curl/inet_ntop.c)
target_sources(alexa PUBLIC curl/inet_pton.c)
target_sources(alexa PUBLIC curl/krb5.c)
target_sources(alexa PUBLIC curl/ldap.c)
target_sources(alexa PUBLIC curl/llist.c)
target_sources(alexa PUBLIC curl/md4.c)
target_sources(alexa PUBLIC curl/md5.c)
target_sources(alexa PUBLIC curl/memdebug.c)
target_sources(alexa PUBLIC curl/mime.c)
target_sources(alexa PUBLIC curl/mqtt.c)
target_sources(alexa PUBLIC curl/netrc.c)
target_sources(alexa PUBLIC curl/non-ascii.c)
target_sources(alexa PUBLIC curl/nonblock.c)
target_sources(alexa PUBLIC curl/nwlib.c)
target_sources(alexa PUBLIC curl/nwos.c)
target_sources(alexa PUBLIC curl/openldap.c)
target_sources(alexa PUBLIC curl/parsedate.c)
target_sources(alexa PUBLIC curl/pingpong.c)
target_sources(alexa PUBLIC curl/pop3.c)
target_sources(alexa PUBLIC curl/progress.c)
target_sources(alexa PUBLIC curl/psl.c)
target_sources(alexa PUBLIC curl/rand.c)
target_sources(alexa PUBLIC curl/rename.c)
target_sources(alexa PUBLIC curl/rtsp.c)
target_sources(alexa PUBLIC curl/security.c)
target_sources(alexa PUBLIC curl/select.c)
target_sources(alexa PUBLIC curl/sendf.c)
target_sources(alexa PUBLIC curl/setopt.c)
target_sources(alexa PUBLIC curl/sha256.c)
target_sources(alexa PUBLIC curl/share.c)
target_sources(alexa PUBLIC curl/slist.c)
target_sources(alexa PUBLIC curl/slist_wc.c)
target_sources(alexa PUBLIC curl/smb.c)
target_sources(alexa PUBLIC curl/smtp.c)
target_sources(alexa PUBLIC curl/socketpair.c)
target_sources(alexa PUBLIC curl/socks.c)
target_sources(alexa PUBLIC curl/socks_gssapi.c)
target_sources(alexa PUBLIC curl/socks_sspi.c)
target_sources(alexa PUBLIC curl/speedcheck.c)
target_sources(alexa PUBLIC curl/splay.c)
target_sources(alexa PUBLIC curl/strcase.c)
target_sources(alexa PUBLIC curl/strdup.c)
target_sources(alexa PUBLIC curl/strerror.c)
target_sources(alexa PUBLIC curl/strtok.c)
target_sources(alexa PUBLIC curl/strtoofft.c)
target_sources(alexa PUBLIC curl/system_win32.c)
target_sources(alexa PUBLIC curl/telnet.c)
target_sources(alexa PUBLIC curl/tftp.c)
target_sources(alexa PUBLIC curl/timeval.c)
target_sources(alexa PUBLIC curl/tool_binmode.c)
target_sources(alexa PUBLIC curl/tool_bname.c)
target_sources(alexa PUBLIC curl/tool_cb_dbg.c)
target_sources(alexa PUBLIC curl/tool_cb_hdr.c)
target_sources(alexa PUBLIC curl/tool_cb_prg.c)
target_sources(alexa PUBLIC curl/tool_cb_rea.c)
target_sources(alexa PUBLIC curl/tool_cb_see.c)
target_sources(alexa PUBLIC curl/tool_cb_wrt.c)
target_sources(alexa PUBLIC curl/tool_cfgable.c)
target_sources(alexa PUBLIC curl/tool_convert.c)
target_sources(alexa PUBLIC curl/tool_dirhie.c)
target_sources(alexa PUBLIC curl/tool_doswin.c)
target_sources(alexa PUBLIC curl/tool_easysrc.c)
target_sources(alexa PUBLIC curl/tool_filetime.c)
target_sources(alexa PUBLIC curl/tool_formparse.c)
target_sources(alexa PUBLIC curl/tool_getparam.c)
target_sources(alexa PUBLIC curl/tool_getpass.c)
target_sources(alexa PUBLIC curl/tool_help.c)
target_sources(alexa PUBLIC curl/tool_helpers.c)
target_sources(alexa PUBLIC curl/tool_homedir.c)
target_sources(alexa PUBLIC curl/tool_hugehelp.c)
target_sources(alexa PUBLIC curl/tool_libinfo.c)
target_sources(alexa PUBLIC curl/tool_main.c)
target_sources(alexa PUBLIC curl/tool_metalink.c)
target_sources(alexa PUBLIC curl/tool_msgs.c)
target_sources(alexa PUBLIC curl/tool_operate.c)
target_sources(alexa PUBLIC curl/tool_operhlp.c)
target_sources(alexa PUBLIC curl/tool_panykey.c)
target_sources(alexa PUBLIC curl/tool_paramhlp.c)
target_sources(alexa PUBLIC curl/tool_parsecfg.c)
target_sources(alexa PUBLIC curl/tool_progress.c)
target_sources(alexa PUBLIC curl/tool_setopt.c)
target_sources(alexa PUBLIC curl/tool_sleep.c)
target_sources(alexa PUBLIC curl/tool_strdup.c)
target_sources(alexa PUBLIC curl/tool_urlglob.c)
target_sources(alexa PUBLIC curl/tool_util.c)
target_sources(alexa PUBLIC curl/tool_vms.c)
target_sources(alexa PUBLIC curl/tool_writeout.c)
target_sources(alexa PUBLIC curl/tool_writeout_json.c)
target_sources(alexa PUBLIC curl/tool_xattr.c)
target_sources(alexa PUBLIC curl/transfer.c)
target_sources(alexa PUBLIC curl/url.c)
target_sources(alexa PUBLIC curl/urlapi.c)
target_sources(alexa PUBLIC curl/version.c)
target_sources(alexa PUBLIC curl/warnless.c)
target_sources(alexa PUBLIC curl/wildcard.c)
target_sources(alexa PUBLIC curl/x509asn1.c)
target_sources(alexa PUBLIC dbus/bus/activation.c)
target_sources(alexa PUBLIC dbus/bus/activation-helper.c)
target_sources(alexa PUBLIC dbus/bus/activation-helper-bin.c)
target_sources(alexa PUBLIC dbus/bus/apparmor.c)
target_sources(alexa PUBLIC dbus/bus/audit.c)
target_sources(alexa PUBLIC dbus/bus/bus.c)
target_sources(alexa PUBLIC dbus/bus/config-loader-expat.c)
target_sources(alexa PUBLIC dbus/bus/config-parser.c)
target_sources(alexa PUBLIC dbus/bus/config-parser-common.c)
target_sources(alexa PUBLIC dbus/bus/config-parser-trivial.c)
target_sources(alexa PUBLIC dbus/bus/connection.c)
target_sources(alexa PUBLIC dbus/bus/containers.c)
target_sources(alexa PUBLIC dbus/bus/desktop-file.c)
target_sources(alexa PUBLIC dbus/bus/dir-watch-default.c)
target_sources(alexa PUBLIC dbus/bus/dir-watch-inotify.c)
target_sources(alexa PUBLIC dbus/bus/dir-watch-kqueue.c)
target_sources(alexa PUBLIC dbus/bus/dispatch.c)
target_sources(alexa PUBLIC dbus/bus/driver.c)
target_sources(alexa PUBLIC dbus/bus/expirelist.c)
target_sources(alexa PUBLIC dbus/bus/main.c)
target_sources(alexa PUBLIC dbus/bus/policy.c)
target_sources(alexa PUBLIC dbus/bus/selinux.c)
target_sources(alexa PUBLIC dbus/bus/services.c)
target_sources(alexa PUBLIC dbus/bus/signals.c)
target_sources(alexa PUBLIC dbus/bus/stats.c)
target_sources(alexa PUBLIC dbus/bus/test.c)
target_sources(alexa PUBLIC dbus/bus/test-launch-helper.c)
target_sources(alexa PUBLIC dbus/bus/test-main.c)
target_sources(alexa PUBLIC dbus/bus/test-system.c)
target_sources(alexa PUBLIC dbus/bus/utils.c)
target_sources(alexa PUBLIC dbus/test/internals/printf.c)
target_sources(alexa PUBLIC dbus/test/internals/refs.c)
target_sources(alexa PUBLIC dbus/test/internals/server-oom.c)
target_sources(alexa PUBLIC dbus/test/internals/syslog.c)
target_sources(alexa PUBLIC dbus/test/internals/variant.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-autolaunch.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-ids.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-pending-call-disconnected.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-pending-call-dispatch.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-pending-call-timeout.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-privserver.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-privserver-client.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-shutdown.c)
target_sources(alexa PUBLIC dbus/test/name-test/test-threads-init.c)
target_sources(alexa PUBLIC dbus/test/containers.c)
target_sources(alexa PUBLIC dbus/test/corrupt.c)
target_sources(alexa PUBLIC dbus/test/dbus-daemon.c)
target_sources(alexa PUBLIC dbus/test/dbus-daemon-eavesdrop.c)
target_sources(alexa PUBLIC dbus/test/fdpass.c)
target_sources(alexa PUBLIC dbus/test/header-fields.c)
target_sources(alexa PUBLIC dbus/test/loopback.c)
target_sources(alexa PUBLIC dbus/test/manual-authz.c)
target_sources(alexa PUBLIC dbus/test/manual-backtrace.c)
target_sources(alexa PUBLIC dbus/test/manual-dir-iter.c)
target_sources(alexa PUBLIC dbus/test/manual-paths.c)
target_sources(alexa PUBLIC dbus/test/manual-tcp.c)
target_sources(alexa PUBLIC dbus/test/marshal.c)
target_sources(alexa PUBLIC dbus/test/message.c)
target_sources(alexa PUBLIC dbus/test/monitor.c)
target_sources(alexa PUBLIC dbus/test/relay.c)
target_sources(alexa PUBLIC dbus/test/sd-activation.c)
target_sources(alexa PUBLIC dbus/test/shell-test.c)
target_sources(alexa PUBLIC dbus/test/spawn-test.c)
target_sources(alexa PUBLIC dbus/test/syntax.c)
target_sources(alexa PUBLIC dbus/test/test-exit.c)
target_sources(alexa PUBLIC dbus/test/test-names.c)
target_sources(alexa PUBLIC dbus/test/test-segfault.c)
target_sources(alexa PUBLIC dbus/test/test-service.c)
target_sources(alexa PUBLIC dbus/test/test-shell-service.c)
target_sources(alexa PUBLIC dbus/test/test-sleep-forever.c)
target_sources(alexa PUBLIC dbus/test/test-utils.c)
target_sources(alexa PUBLIC dbus/test/test-utils-glib.c)
target_sources(alexa PUBLIC dbus/test/uid-permissions.c)
target_sources(alexa PUBLIC dbus/dbus-address.c)
target_sources(alexa PUBLIC dbus/dbus-asv-util.c)
target_sources(alexa PUBLIC dbus/dbus-auth.c)
target_sources(alexa PUBLIC dbus/dbus-auth-script.c)
target_sources(alexa PUBLIC dbus/dbus-auth-util.c)
target_sources(alexa PUBLIC dbus/dbus-bus.c)
target_sources(alexa PUBLIC dbus/dbus-connection.c)
target_sources(alexa PUBLIC dbus/dbus-credentials.c)
target_sources(alexa PUBLIC dbus/dbus-credentials-util.c)
target_sources(alexa PUBLIC dbus/dbus-dataslot.c)
target_sources(alexa PUBLIC dbus/dbus-errors.c)
target_sources(alexa PUBLIC dbus/dbus-file.c)
target_sources(alexa PUBLIC dbus/dbus-file-unix.c)
target_sources(alexa PUBLIC dbus/dbus-file-win.c)
target_sources(alexa PUBLIC dbus/dbus-hash.c)
target_sources(alexa PUBLIC dbus/dbus-init-win.cpp)
target_sources(alexa PUBLIC dbus/dbus-internals.c)
target_sources(alexa PUBLIC dbus/dbus-keyring.c)
target_sources(alexa PUBLIC dbus/dbus-list.c)
target_sources(alexa PUBLIC dbus/dbus-mainloop.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-basic.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-byteswap.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-byteswap-util.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-header.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-recursive.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-recursive-util.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-validate.c)
target_sources(alexa PUBLIC dbus/dbus-marshal-validate-util.c)
target_sources(alexa PUBLIC dbus/dbus-memory.c)
target_sources(alexa PUBLIC dbus/dbus-mempool.c)
target_sources(alexa PUBLIC dbus/dbus-message.c)
target_sources(alexa PUBLIC dbus/dbus-message-factory.c)
target_sources(alexa PUBLIC dbus/dbus-message-util.c)
target_sources(alexa PUBLIC dbus/dbus-misc.c)
target_sources(alexa PUBLIC dbus/dbus-nonce.c)
target_sources(alexa PUBLIC dbus/dbus-object-tree.c)
target_sources(alexa PUBLIC dbus/dbus-pending-call.c)
target_sources(alexa PUBLIC dbus/dbus-pipe.c)
target_sources(alexa PUBLIC dbus/dbus-pipe-unix.c)
target_sources(alexa PUBLIC dbus/dbus-pipe-win.c)
target_sources(alexa PUBLIC dbus/dbus-resources.c)
target_sources(alexa PUBLIC dbus/dbus-server.c)
target_sources(alexa PUBLIC dbus/dbus-server-debug-pipe.c)
target_sources(alexa PUBLIC dbus/dbus-server-launchd.c)
target_sources(alexa PUBLIC dbus/dbus-server-socket.c)
target_sources(alexa PUBLIC dbus/dbus-server-unix.c)
target_sources(alexa PUBLIC dbus/dbus-server-win.c)
target_sources(alexa PUBLIC dbus/dbus-sha.c)
target_sources(alexa PUBLIC dbus/dbus-shell.c)
target_sources(alexa PUBLIC dbus/dbus-signature.c)
target_sources(alexa PUBLIC dbus/dbus-socket-set.c)
target_sources(alexa PUBLIC dbus/dbus-socket-set-epoll.c)
target_sources(alexa PUBLIC dbus/dbus-socket-set-poll.c)
target_sources(alexa PUBLIC dbus/dbus-spawn-test.c)
target_sources(alexa PUBLIC dbus/dbus-spawn-unix.c)
target_sources(alexa PUBLIC dbus/dbus-spawn-win.c)
target_sources(alexa PUBLIC dbus/dbus-string.c)
target_sources(alexa PUBLIC dbus/dbus-string-util.c)
target_sources(alexa PUBLIC dbus/dbus-syntax.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-pthread.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-thread-win.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-unix.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-util.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-util-unix.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-util-win.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-win.c)
target_sources(alexa PUBLIC dbus/dbus-sysdeps-wince-glue.c)
target_sources(alexa PUBLIC dbus/dbus-test.c)
target_sources(alexa PUBLIC dbus/dbus-test-main.c)
target_sources(alexa PUBLIC dbus/dbus-test-tap.c)
target_sources(alexa PUBLIC dbus/dbus-threads.c)
target_sources(alexa PUBLIC dbus/dbus-timeout.c)
target_sources(alexa PUBLIC dbus/dbus-transport.c)
target_sources(alexa PUBLIC dbus/dbus-transport-socket.c)
target_sources(alexa PUBLIC dbus/dbus-transport-unix.c)
target_sources(alexa PUBLIC dbus/dbus-transport-win.c)
target_sources(alexa PUBLIC dbus/dbus-userdb.c)
target_sources(alexa PUBLIC dbus/dbus-userdb-util.c)
target_sources(alexa PUBLIC dbus/dbus-uuidgen.c)
target_sources(alexa PUBLIC dbus/dbus-watch.c)
target_sources(alexa PUBLIC diagnostics/AudioInjectorMicrophone.cpp)
target_sources(alexa PUBLIC diagnostics/DevicePropertyAggregator.cpp)
target_sources(alexa PUBLIC diagnostics/DevicePropertyAggregatorTest.cpp)
target_sources(alexa PUBLIC diagnostics/DeviceProtocolTracer.cpp)
target_sources(alexa PUBLIC diagnostics/DeviceProtocolTracerTest.cpp)
target_sources(alexa PUBLIC diagnostics/DiagnosticsUtils.cpp)
target_sources(alexa PUBLIC diagnostics/FileBasedAudioInjector.cpp)
target_sources(alexa PUBLIC endpoints/Endpoint.cpp)
target_sources(alexa PUBLIC endpoints/EndpointAttributeValidation.cpp)
target_sources(alexa PUBLIC endpoints/EndpointBuilder.cpp)
target_sources(alexa PUBLIC endpoints/EndpointRegistrationManager.cpp)
target_sources(alexa PUBLIC equalizer_implementations/EqualizerController.cpp)
target_sources(alexa PUBLIC equalizer_implementations/EqualizerControllerTest.cpp)
target_sources(alexa PUBLIC equalizer_implementations/EqualizerLinearBandMapper.cpp)
target_sources(alexa PUBLIC equalizer_implementations/EqualizerLinearBandMapperTest.cpp)
target_sources(alexa PUBLIC equalizer_implementations/EqualizerUtils.cpp)
target_sources(alexa PUBLIC equalizer_implementations/InMemoryEqualizerConfiguration.cpp)
target_sources(alexa PUBLIC equalizer_implementations/InMemoryEqualizerConfigurationTest.cpp)
target_sources(alexa PUBLIC equalizer_implementations/MiscDBEqualizerStorage.cpp)
target_sources(alexa PUBLIC equalizer_implementations/MiscDBEqualizerStorageTest.cpp)
target_sources(alexa PUBLIC equalizer_implementations/SDKConfigEqualizerConfiguration.cpp)
target_sources(alexa PUBLIC equalizer_implementations/SDKConfigEqualizerConfigurationTest.cpp)
target_sources(alexa PUBLIC expat/benchmark/benchmark.c)
target_sources(alexa PUBLIC expat/fuzz/xml_parse_fuzzer.c)
target_sources(alexa PUBLIC expat/fuzz/xml_parsebuffer_fuzzer.c)
target_sources(alexa PUBLIC expat/gennmtab/gennmtab.c)
target_sources(alexa PUBLIC expat/xmlwf/codepage.c)
target_sources(alexa PUBLIC expat/xmlwf/ct.c)
target_sources(alexa PUBLIC expat/xmlwf/readfilemap.c)
target_sources(alexa PUBLIC expat/xmlwf/unixfilemap.c)
target_sources(alexa PUBLIC expat/xmlwf/win32filemap.c)
target_sources(alexa PUBLIC expat/xmlwf/xmlfile.c)
target_sources(alexa PUBLIC expat/xmlwf/xmlwf.c)
target_sources(alexa PUBLIC expat/chardata.c)
target_sources(alexa PUBLIC expat/memcheck.c)
target_sources(alexa PUBLIC expat/minicheck.c)
target_sources(alexa PUBLIC expat/runtests.c)
target_sources(alexa PUBLIC expat/runtestspp.cpp)
target_sources(alexa PUBLIC expat/structdata.c)
target_sources(alexa PUBLIC expat/xmlparse.c)
target_sources(alexa PUBLIC expat/xmlrole.c)
target_sources(alexa PUBLIC expat/xmltok.c)
target_sources(alexa PUBLIC expat/xmltok_impl.c)
target_sources(alexa PUBLIC expat/xmltok_ns.c)
target_sources(alexa PUBLIC gio/fam/fam-helper.c)
target_sources(alexa PUBLIC gio/fam/fam-module.c)
target_sources(alexa PUBLIC gio/fam/gfamdirectorymonitor.c)
target_sources(alexa PUBLIC gio/fam/gfamfilemonitor.c)
target_sources(alexa PUBLIC gio/fen/fen-dump.c)
target_sources(alexa PUBLIC gio/fen/fen-helper.c)
target_sources(alexa PUBLIC gio/fen/fen-kernel.c)
target_sources(alexa PUBLIC gio/fen/fen-node.c)
target_sources(alexa PUBLIC gio/fen/gfendirectorymonitor.c)
target_sources(alexa PUBLIC gio/fen/gfenfilemonitor.c)
target_sources(alexa PUBLIC gio/gvdb/gvdb-builder.c)
target_sources(alexa PUBLIC gio/gvdb/gvdb-reader.c)
target_sources(alexa PUBLIC gio/inotify/ginotifydirectorymonitor.c)
target_sources(alexa PUBLIC gio/inotify/ginotifyfilemonitor.c)
target_sources(alexa PUBLIC gio/inotify/inotify-diag.c)
target_sources(alexa PUBLIC gio/inotify/inotify-helper.c)
target_sources(alexa PUBLIC gio/inotify/inotify-kernel.c)
target_sources(alexa PUBLIC gio/inotify/inotify-missing.c)
target_sources(alexa PUBLIC gio/inotify/inotify-path.c)
target_sources(alexa PUBLIC gio/inotify/inotify-sub.c)
target_sources(alexa PUBLIC gio/libasyncns/asyncns.c)
target_sources(alexa PUBLIC gio/tests/actions.c)
target_sources(alexa PUBLIC gio/tests/appinfo.c)
target_sources(alexa PUBLIC gio/tests/appinfo-test.c)
target_sources(alexa PUBLIC gio/tests/async-close-output-stream.c)
target_sources(alexa PUBLIC gio/tests/buffered-input-stream.c)
target_sources(alexa PUBLIC gio/tests/buffered-output-stream.c)
target_sources(alexa PUBLIC gio/tests/contenttype.c)
target_sources(alexa PUBLIC gio/tests/contexts.c)
target_sources(alexa PUBLIC gio/tests/converter-stream.c)
target_sources(alexa PUBLIC gio/tests/data-input-stream.c)
target_sources(alexa PUBLIC gio/tests/data-output-stream.c)
target_sources(alexa PUBLIC gio/tests/desktop-app-info.c)
target_sources(alexa PUBLIC gio/tests/echo-server.c)
target_sources(alexa PUBLIC gio/tests/file.c)
target_sources(alexa PUBLIC gio/tests/filter-cat.c)
target_sources(alexa PUBLIC gio/tests/filter-streams.c)
target_sources(alexa PUBLIC gio/tests/g-file.c)
target_sources(alexa PUBLIC gio/tests/g-file-info.c)
target_sources(alexa PUBLIC gio/tests/g-icon.c)
target_sources(alexa PUBLIC gio/tests/gapplication.c)
target_sources(alexa PUBLIC gio/tests/gapplication-example-actions.c)
target_sources(alexa PUBLIC gio/tests/gapplication-example-cmdline.c)
target_sources(alexa PUBLIC gio/tests/gapplication-example-cmdline2.c)
target_sources(alexa PUBLIC gio/tests/gapplication-example-open.c)
target_sources(alexa PUBLIC gio/tests/gdbus-addresses.c)
target_sources(alexa PUBLIC gio/tests/gdbus-bz627724.c)
target_sources(alexa PUBLIC gio/tests/gdbus-connection.c)
target_sources(alexa PUBLIC gio/tests/gdbus-connection-flush-helper.c)
target_sources(alexa PUBLIC gio/tests/gdbus-connection-slow.c)
target_sources(alexa PUBLIC gio/tests/gdbus-error.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-export.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-own-name.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-peer.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-proxy-subclass.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-server.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-subtree.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-unix-fd-client.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-watch-name.c)
target_sources(alexa PUBLIC gio/tests/gdbus-example-watch-proxy.c)
target_sources(alexa PUBLIC gio/tests/gdbus-exit-on-close.c)
target_sources(alexa PUBLIC gio/tests/gdbus-export.c)
target_sources(alexa PUBLIC gio/tests/gdbus-introspection.c)
target_sources(alexa PUBLIC gio/tests/gdbus-message.c)
target_sources(alexa PUBLIC gio/tests/gdbus-names.c)
target_sources(alexa PUBLIC gio/tests/gdbus-non-socket.c)
target_sources(alexa PUBLIC gio/tests/gdbus-peer.c)
target_sources(alexa PUBLIC gio/tests/gdbus-proxy.c)
target_sources(alexa PUBLIC gio/tests/gdbus-proxy-well-known-name.c)
target_sources(alexa PUBLIC gio/tests/gdbus-serialization.c)
target_sources(alexa PUBLIC gio/tests/gdbus-sessionbus.c)
target_sources(alexa PUBLIC gio/tests/gdbus-tests.c)
target_sources(alexa PUBLIC gio/tests/gdbus-threading.c)
target_sources(alexa PUBLIC gio/tests/gschema-compile.c)
target_sources(alexa PUBLIC gio/tests/gsettings.c)
target_sources(alexa PUBLIC gio/tests/httpd.c)
target_sources(alexa PUBLIC gio/tests/io-stream.c)
target_sources(alexa PUBLIC gio/tests/live-g-file.c)
target_sources(alexa PUBLIC gio/tests/memory-input-stream.c)
target_sources(alexa PUBLIC gio/tests/memory-output-stream.c)
target_sources(alexa PUBLIC gio/tests/network-address.c)
target_sources(alexa PUBLIC gio/tests/pollable.c)
target_sources(alexa PUBLIC gio/tests/proxy.c)
target_sources(alexa PUBLIC gio/tests/readwrite.c)
target_sources(alexa PUBLIC gio/tests/resolver.c)
target_sources(alexa PUBLIC gio/tests/send-data.c)
target_sources(alexa PUBLIC gio/tests/simple-async-result.c)
target_sources(alexa PUBLIC gio/tests/sleepy-stream.c)
target_sources(alexa PUBLIC gio/tests/socket.c)
target_sources(alexa PUBLIC gio/tests/socket-client.c)
target_sources(alexa PUBLIC gio/tests/socket-common.c)
target_sources(alexa PUBLIC gio/tests/socket-server.c)
target_sources(alexa PUBLIC gio/tests/srvtarget.c)
target_sources(alexa PUBLIC gio/tests/unix-fd.c)
target_sources(alexa PUBLIC gio/tests/unix-streams.c)
target_sources(alexa PUBLIC gio/tests/volumemonitor.c)
target_sources(alexa PUBLIC gio/tests/win32-streams.c)
target_sources(alexa PUBLIC gio/win32/gwin32directorymonitor.c)
target_sources(alexa PUBLIC gio/win32/gwinhttpfile.c)
target_sources(alexa PUBLIC gio/win32/gwinhttpfileinputstream.c)
target_sources(alexa PUBLIC gio/win32/gwinhttpfileoutputstream.c)
target_sources(alexa PUBLIC gio/win32/gwinhttpfile.c)
target_sources(alexa PUBLIC gio/win32/gwinhttpvfs.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmime.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmimealias.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmimecache.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmimeglob.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmimeicon.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmimeint.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmimemagic.c)
target_sources(alexa PUBLIC gio/xdgmime/xdgmimeparent.c)
target_sources(alexa PUBLIC gio/gaction.c)
target_sources(alexa PUBLIC gio/gactiongroup.c)
target_sources(alexa PUBLIC gio/gappinfo.c)
target_sources(alexa PUBLIC gio/gapplication.c)
target_sources(alexa PUBLIC gio/gapplicationcommandline.c)
target_sources(alexa PUBLIC gio/gapplicationimpl-dbus.c)
target_sources(alexa PUBLIC gio/gasynchelper.c)
target_sources(alexa PUBLIC gio/gasyncinitable.c)
target_sources(alexa PUBLIC gio/gasyncresult.c)
target_sources(alexa PUBLIC gio/gbufferedinputstream.c)
target_sources(alexa PUBLIC gio/gbufferedoutputstream.c)
target_sources(alexa PUBLIC gio/gcancellable.c)
target_sources(alexa PUBLIC gio/gcharsetconverter.c)
target_sources(alexa PUBLIC gio/gcontenttype.c)
target_sources(alexa PUBLIC gio/gconverter.c)
target_sources(alexa PUBLIC gio/gconverterinputstream.c)
target_sources(alexa PUBLIC gio/gconverteroutputstream.c)
target_sources(alexa PUBLIC gio/gcredentials.c)
target_sources(alexa PUBLIC gio/gdatainputstream.c)
target_sources(alexa PUBLIC gio/gdataoutputstream.c)
target_sources(alexa PUBLIC gio/gdbus-tool.c)
target_sources(alexa PUBLIC gio/gdbusaddress.c)
target_sources(alexa PUBLIC gio/gdbusauth.c)
target_sources(alexa PUBLIC gio/gdbusauthmechanism.c)
target_sources(alexa PUBLIC gio/gdbusauthmechanismanon.c)
target_sources(alexa PUBLIC gio/gdbusauthmechanismexternal.c)
target_sources(alexa PUBLIC gio/gdbusauthmechanismsha1.c)
target_sources(alexa PUBLIC gio/gdbusauthobserver.c)
target_sources(alexa PUBLIC gio/gdbusconnection.c)
target_sources(alexa PUBLIC gio/gdbuserror.c)
target_sources(alexa PUBLIC gio/gdbusintrospection.c)
target_sources(alexa PUBLIC gio/gdbusmessage.c)
target_sources(alexa PUBLIC gio/gdbusmethodinvocation.c)
target_sources(alexa PUBLIC gio/gdbusnameowning.c)
target_sources(alexa PUBLIC gio/gdbusnamewatching.c)
target_sources(alexa PUBLIC gio/gdbusprivate.c)
target_sources(alexa PUBLIC gio/gdbusproxy.c)
target_sources(alexa PUBLIC gio/gdbusserver.c)
target_sources(alexa PUBLIC gio/gdbusutils.c)
target_sources(alexa PUBLIC gio/gdelayedsettingsbackend.c)
target_sources(alexa PUBLIC gio/gdesktopappinfo.c)
target_sources(alexa PUBLIC gio/gdrive.c)
target_sources(alexa PUBLIC gio/gdummyfile.c)
target_sources(alexa PUBLIC gio/gdummyproxyresolver.c)
target_sources(alexa PUBLIC gio/gdummytlsbackend.c)
target_sources(alexa PUBLIC gio/gemblem.c)
target_sources(alexa PUBLIC gio/gemblemedicon.c)
target_sources(alexa PUBLIC gio/gfile.c)
target_sources(alexa PUBLIC gio/gfileattribute.c)
target_sources(alexa PUBLIC gio/gfiledescriptorbased.c)
target_sources(alexa PUBLIC gio/gfileenumerator.c)
target_sources(alexa PUBLIC gio/gfileicon.c)
target_sources(alexa PUBLIC gio/gfileinfo.c)
target_sources(alexa PUBLIC gio/gfileinputstream.c)
target_sources(alexa PUBLIC gio/gfileiostream.c)
target_sources(alexa PUBLIC gio/gfilemonitor.c)
target_sources(alexa PUBLIC gio/gfilenamecompleter.c)
target_sources(alexa PUBLIC gio/gfileoutputstream.c)
target_sources(alexa PUBLIC gio/gfilterinputstream.c)
target_sources(alexa PUBLIC gio/gfilteroutputstream.c)
target_sources(alexa PUBLIC gio/gicon.c)
target_sources(alexa PUBLIC gio/ginetaddress.c)
target_sources(alexa PUBLIC gio/ginetsocketaddress.c)
target_sources(alexa PUBLIC gio/ginitable.c)
target_sources(alexa PUBLIC gio/ginputstream.c)
target_sources(alexa PUBLIC gio/gio-querymodules.c)
target_sources(alexa PUBLIC gio/gioenumtypes.c)
target_sources(alexa PUBLIC gio/gioerror.c)
target_sources(alexa PUBLIC gio/giomodule.c)
target_sources(alexa PUBLIC gio/gioscheduler.c)
target_sources(alexa PUBLIC gio/giostream.c)
target_sources(alexa PUBLIC gio/gkeyfilesettingsbackend.c)
target_sources(alexa PUBLIC gio/glib-compile-schemas.c)
target_sources(alexa PUBLIC gio/gloadableicon.c)
target_sources(alexa PUBLIC gio/glocaldirectorymonitor.c)
target_sources(alexa PUBLIC gio/glocalfile.c)
target_sources(alexa PUBLIC gio/glocalfileenumerator.c)
target_sources(alexa PUBLIC gio/glocalfileinfo.c)
target_sources(alexa PUBLIC gio/glocalfileinputstream.c)
target_sources(alexa PUBLIC gio/glocalfileiostream.c)
target_sources(alexa PUBLIC gio/glocalfilemonitor.c)
target_sources(alexa PUBLIC gio/glocalfileoutputstream.c)
target_sources(alexa PUBLIC gio/glocalvfs.c)
target_sources(alexa PUBLIC gio/gmemoryinputstream.c)
target_sources(alexa PUBLIC gio/gmemoryoutputstream.c)
target_sources(alexa PUBLIC gio/gmemorysettingsbackend.c)
target_sources(alexa PUBLIC gio/gmount.c)
target_sources(alexa PUBLIC gio/gmountoperation.c)
target_sources(alexa PUBLIC gio/gnativevolumemonitor.c)
target_sources(alexa PUBLIC gio/gnetworkaddress.c)
target_sources(alexa PUBLIC gio/gnetworkservice.c)
target_sources(alexa PUBLIC gio/gnullsettingsbackend.c)
target_sources(alexa PUBLIC gio/goutputstream.c)
target_sources(alexa PUBLIC gio/gpermission.c)
target_sources(alexa PUBLIC gio/gpollableinputstream.c)
target_sources(alexa PUBLIC gio/gpollableoutputstream.c)
target_sources(alexa PUBLIC gio/gpollfilemonitor.c)
target_sources(alexa PUBLIC gio/gproxy.c)
target_sources(alexa PUBLIC gio/gproxyaddress.c)
target_sources(alexa PUBLIC gio/gproxyaddressenumerator.c)
target_sources(alexa PUBLIC gio/gproxyresolver.c)
target_sources(alexa PUBLIC gio/gregistrysettingsbackend.c)
target_sources(alexa PUBLIC gio/gresolver.c)
target_sources(alexa PUBLIC gio/gseekable.c)
target_sources(alexa PUBLIC gio/gsettings.c)
target_sources(alexa PUBLIC gio/gsettings-mapping.c)
target_sources(alexa PUBLIC gio/gsettings-tool.c)
target_sources(alexa PUBLIC gio/gsettingsbackend.c)
target_sources(alexa PUBLIC gio/gsettingsschema.c)
target_sources(alexa PUBLIC gio/gsimpleaction.c)
target_sources(alexa PUBLIC gio/gsimpleactiongroup.c)
target_sources(alexa PUBLIC gio/gsimpleasyncresult.c)
target_sources(alexa PUBLIC gio/gsimplepermission.c)
target_sources(alexa PUBLIC gio/gsocket.c)
target_sources(alexa PUBLIC gio/gsocketaddress.c)
target_sources(alexa PUBLIC gio/gsocketaddressenumerator.c)
target_sources(alexa PUBLIC gio/gsocketclient.c)
target_sources(alexa PUBLIC gio/gsocketconnectable.c)
target_sources(alexa PUBLIC gio/gsocketconnection.c)
target_sources(alexa PUBLIC gio/gsocketcontrolmessage.c)
target_sources(alexa PUBLIC gio/gsocketinputstream.c)
target_sources(alexa PUBLIC gio/gsocketlistener.c)
target_sources(alexa PUBLIC gio/gsocketoutputstream.c)
target_sources(alexa PUBLIC gio/gsocketservice.c)
target_sources(alexa PUBLIC gio/gsocks4aproxy.c)
target_sources(alexa PUBLIC gio/gsocks4proxy.c)
target_sources(alexa PUBLIC gio/gsocks5proxy.c)
target_sources(alexa PUBLIC gio/gsrvtarget.c)
target_sources(alexa PUBLIC gio/gtask.c)
target_sources(alexa PUBLIC gio/gtcpconnection.c)
target_sources(alexa PUBLIC gio/gtcpwrapperconnection.c)
target_sources(alexa PUBLIC gio/gthemedicon.c)
target_sources(alexa PUBLIC gio/gthreadedresolver.c)
target_sources(alexa PUBLIC gio/gthreadedsocketservice.c)
target_sources(alexa PUBLIC gio/gtlsbackend.c)
target_sources(alexa PUBLIC gio/gtlscertificate.c)
target_sources(alexa PUBLIC gio/gtlsclientconnection.c)
target_sources(alexa PUBLIC gio/gtlsconnection.c)
target_sources(alexa PUBLIC gio/gtlsserverconnection.c)
target_sources(alexa PUBLIC gio/gunionvolumemonitor.c)
target_sources(alexa PUBLIC gio/gunixconnection.c)
target_sources(alexa PUBLIC gio/gunixcredentialsmessage.c)
target_sources(alexa PUBLIC gio/gunixfdlist.c)
target_sources(alexa PUBLIC gio/gunixfdmessage.c)
target_sources(alexa PUBLIC gio/gunixinputstream.c)
target_sources(alexa PUBLIC gio/gunixmount.c)
target_sources(alexa PUBLIC gio/gunixmounts.c)
target_sources(alexa PUBLIC gio/gunixoutputstream.c)
target_sources(alexa PUBLIC gio/gunixresolver.c)
target_sources(alexa PUBLIC gio/gunixsocketaddress.c)
target_sources(alexa PUBLIC gio/gunixvolume.c)
target_sources(alexa PUBLIC gio/gunixvolumemonitor.c)
target_sources(alexa PUBLIC gio/gvfs.c)
target_sources(alexa PUBLIC gio/gvolume.c)
target_sources(alexa PUBLIC gio/gvolumemonitor.c)
target_sources(alexa PUBLIC gio/gwin32appinfo.c)
target_sources(alexa PUBLIC gio/gwin32inputstream.c)
target_sources(alexa PUBLIC gio/gwin32mount.c)
target_sources(alexa PUBLIC gio/gwin32outputstream.c)
target_sources(alexa PUBLIC gio/gwin32resolver.c)
target_sources(alexa PUBLIC gio/gwin32volumemonitor.c)
target_sources(alexa PUBLIC gio/gzlibcompressor.c)
target_sources(alexa PUBLIC gio/gzlibdecompressor.c)
target_sources(alexa PUBLIC gio/strinfo.c)
target_sources(alexa PUBLIC glib/glib-mirroring-tab/gen-mirroring-tab.c)
target_sources(alexa PUBLIC glib/glib-mirroring-tab/packtab.c)
target_sources(alexa PUBLIC glib/gnulib/asnprintf.c)
target_sources(alexa PUBLIC glib/gnulib/printf.c)
target_sources(alexa PUBLIC glib/gnulib/printf-args.c)
target_sources(alexa PUBLIC glib/gnulib/printf-parse.c)
target_sources(alexa PUBLIC glib/gnulib/vasnprintf.c)
target_sources(alexa PUBLIC glib/libcharset/localcharset.c)
target_sources(alexa PUBLIC glib/pcre/pcre_chartables.c)
target_sources(alexa PUBLIC glib/pcre/pcre_compile.c)
target_sources(alexa PUBLIC glib/pcre/pcre_config.c)
target_sources(alexa PUBLIC glib/pcre/pcre_dfa_exec.c)
target_sources(alexa PUBLIC glib/pcre/pcre_exec.c)
target_sources(alexa PUBLIC glib/pcre/pcre_fullinfo.c)
target_sources(alexa PUBLIC glib/pcre/pcre_get.c)
target_sources(alexa PUBLIC glib/pcre/pcre_globals.c)
target_sources(alexa PUBLIC glib/pcre/pcre_newline.c)
target_sources(alexa PUBLIC glib/pcre/pcre_ord2utf8.c)
target_sources(alexa PUBLIC glib/pcre/pcre_study.c)
target_sources(alexa PUBLIC glib/pcre/pcre_tables.c)
target_sources(alexa PUBLIC glib/pcre/pcre_try_flipped.c)
target_sources(alexa PUBLIC glib/pcre/pcre_ucp_searchfuncs.c)
target_sources(alexa PUBLIC glib/pcre/pcre_xclass.c)
target_sources(alexa PUBLIC glib/tests/array-test.c)
target_sources(alexa PUBLIC glib/tests/base64.c)
target_sources(alexa PUBLIC glib/tests/bookmarkfile.c)
target_sources(alexa PUBLIC glib/tests/checksum.c)
target_sources(alexa PUBLIC glib/tests/collate.c)
target_sources(alexa PUBLIC glib/tests/convert.c)
target_sources(alexa PUBLIC glib/tests/dataset.c)
target_sources(alexa PUBLIC glib/tests/date.c)
target_sources(alexa PUBLIC glib/tests/dir.c)
target_sources(alexa PUBLIC glib/tests/environment.c)
target_sources(alexa PUBLIC glib/tests/error.c)
target_sources(alexa PUBLIC glib/tests/fileutils.c)
target_sources(alexa PUBLIC glib/tests/gdatetime.c)
target_sources(alexa PUBLIC glib/tests/gvariant.c)
target_sources(alexa PUBLIC glib/tests/hash.c)
target_sources(alexa PUBLIC glib/tests/hostutils.c)
target_sources(alexa PUBLIC glib/tests/keyfile.c)
target_sources(alexa PUBLIC glib/tests/list.c)
target_sources(alexa PUBLIC glib/tests/logging.c)
target_sources(alexa PUBLIC glib/tests/mappedfile.c)
target_sources(alexa PUBLIC glib/tests/markup-collect.c)
target_sources(alexa PUBLIC glib/tests/markup-escape.c)
target_sources(alexa PUBLIC glib/tests/markup-parse.c)
target_sources(alexa PUBLIC glib/tests/markup-subparser.c)
target_sources(alexa PUBLIC glib/tests/mem-overflow.c)
target_sources(alexa PUBLIC glib/tests/node.c)
target_sources(alexa PUBLIC glib/tests/option-context.c)
target_sources(alexa PUBLIC glib/tests/pattern.c)
target_sources(alexa PUBLIC glib/tests/printf.c)
target_sources(alexa PUBLIC glib/tests/protocol.c)
target_sources(alexa PUBLIC glib/tests/queue.c)
target_sources(alexa PUBLIC glib/tests/rand.c)
target_sources(alexa PUBLIC glib/tests/regex.c)
target_sources(alexa PUBLIC glib/tests/scannerapi.c)
target_sources(alexa PUBLIC glib/tests/sequence.c)
target_sources(alexa PUBLIC glib/tests/shell.c)
target_sources(alexa PUBLIC glib/tests/slist.c)
target_sources(alexa PUBLIC glib/tests/sort.c)
target_sources(alexa PUBLIC glib/tests/strfuncs.c)
target_sources(alexa PUBLIC glib/tests/string.c)
target_sources(alexa PUBLIC glib/tests/testing.c)
target_sources(alexa PUBLIC glib/tests/timeout.c)
target_sources(alexa PUBLIC glib/tests/tree.c)
target_sources(alexa PUBLIC glib/tests/uri.c)
target_sources(alexa PUBLIC glib/tests/utf8-misc.c)
target_sources(alexa PUBLIC glib/tests/utf8-pointer.c)
target_sources(alexa PUBLIC glib/tests/utf8-validate.c)
target_sources(alexa PUBLIC glib/tests/utils.c)
target_sources(alexa PUBLIC glib/update-pcre/pcre_ucp_searchfuncs.c)
target_sources(alexa PUBLIC glib/update-pcre/pcre_valid_utf8.c)
target_sources(alexa PUBLIC glib/garray.c)
target_sources(alexa PUBLIC glib/gasyncqueue.c)
target_sources(alexa PUBLIC glib/gatomic.c)
target_sources(alexa PUBLIC glib/gatomic-gcc.c)
target_sources(alexa PUBLIC glib/gbacktrace.c)
target_sources(alexa PUBLIC glib/gbase64.c)
target_sources(alexa PUBLIC glib/gbitlock.c)
target_sources(alexa PUBLIC glib/gbookmarkfile.c)
target_sources(alexa PUBLIC glib/gbuffer.c)
target_sources(alexa PUBLIC glib/gcache.c)
target_sources(alexa PUBLIC glib/gchecksum.c)
target_sources(alexa PUBLIC glib/gcompletion.c)
target_sources(alexa PUBLIC glib/gconvert.c)
target_sources(alexa PUBLIC glib/gdataset.c)
target_sources(alexa PUBLIC glib/gdate.c)
target_sources(alexa PUBLIC glib/gdatetime.c)
target_sources(alexa PUBLIC glib/gdir.c)
target_sources(alexa PUBLIC glib/gerror.c)
target_sources(alexa PUBLIC glib/gfileutils.c)
target_sources(alexa PUBLIC glib/ghash.c)
target_sources(alexa PUBLIC glib/ghook.c)
target_sources(alexa PUBLIC glib/ghostutils.c)
target_sources(alexa PUBLIC glib/giochannel.c)
target_sources(alexa PUBLIC glib/giounix.c)
target_sources(alexa PUBLIC glib/giowin32.c)
target_sources(alexa PUBLIC glib/gkeyfile.c)
target_sources(alexa PUBLIC glib/glib-init.c)
target_sources(alexa PUBLIC glib/glib-private.c)
target_sources(alexa PUBLIC glib/glib-unix.c)
target_sources(alexa PUBLIC glib/glist.c)
target_sources(alexa PUBLIC glib/gmain.c)
target_sources(alexa PUBLIC glib/gmappedfile.c)
target_sources(alexa PUBLIC glib/gmarkup.c)
target_sources(alexa PUBLIC glib/gmem.c)
target_sources(alexa PUBLIC glib/gmessages.c)
target_sources(alexa PUBLIC glib/gnode.c)
target_sources(alexa PUBLIC glib/goption.c)
target_sources(alexa PUBLIC glib/gpattern.c)
target_sources(alexa PUBLIC glib/gpoll.c)
target_sources(alexa PUBLIC glib/gprimes.c)
target_sources(alexa PUBLIC glib/gprintf.c)
target_sources(alexa PUBLIC glib/gqsort.c)
target_sources(alexa PUBLIC glib/gqueue.c)
target_sources(alexa PUBLIC glib/grand.c)
target_sources(alexa PUBLIC glib/gregex.c)
target_sources(alexa PUBLIC glib/grel.c)
target_sources(alexa PUBLIC glib/gscanner.c)
target_sources(alexa PUBLIC glib/gsequence.c)
target_sources(alexa PUBLIC glib/gshell.c)
target_sources(alexa PUBLIC glib/gslice.c)
target_sources(alexa PUBLIC glib/gslist.c)
target_sources(alexa PUBLIC glib/gspawn.c)
target_sources(alexa PUBLIC glib/gspawn-win32.c)
target_sources(alexa PUBLIC glib/gspawn-win32-helper.c)
target_sources(alexa PUBLIC glib/gstdio.c)
target_sources(alexa PUBLIC glib/gstrfuncs.c)
target_sources(alexa PUBLIC glib/gstring.c)
target_sources(alexa PUBLIC glib/gtester.c)
target_sources(alexa PUBLIC glib/gtestutils.c)
target_sources(alexa PUBLIC glib/gthread.c)
target_sources(alexa PUBLIC glib/gthreadpool.c)
target_sources(alexa PUBLIC glib/gtimer.c)
target_sources(alexa PUBLIC glib/gtimezone.c)
target_sources(alexa PUBLIC glib/gtree.c)
target_sources(alexa PUBLIC glib/gunibreak.c)
target_sources(alexa PUBLIC glib/gunicollate.c)
target_sources(alexa PUBLIC glib/gunidecomp.c)
target_sources(alexa PUBLIC glib/guniprop.c)
target_sources(alexa PUBLIC glib/gurifuncs.c)
target_sources(alexa PUBLIC glib/gutf8.c)
target_sources(alexa PUBLIC glib/gutils.c)
target_sources(alexa PUBLIC glib/gvariant.c)
target_sources(alexa PUBLIC glib/gvariant-core.c)
target_sources(alexa PUBLIC glib/gvariant-parser.c)
target_sources(alexa PUBLIC glib/gvariant-serialiser.c)
target_sources(alexa PUBLIC glib/gvarianttype.c)
target_sources(alexa PUBLIC glib/gvarianttypeinfo.c)
target_sources(alexa PUBLIC glib/gwakeup.c)
target_sources(alexa PUBLIC glib/gwin32.c)
target_sources(alexa PUBLIC glib/win_iconv.c)
target_sources(alexa PUBLIC gmock/gmock.cc)
target_sources(alexa PUBLIC gmock/gmock-actions_test.cc)
target_sources(alexa PUBLIC gmock/gmock-all.cc)
target_sources(alexa PUBLIC gmock/gmock-cardinalities.cc)
target_sources(alexa PUBLIC gmock/gmock-cardinalities_test.cc)
target_sources(alexa PUBLIC gmock/gmock-generated-actions_test.cc)
target_sources(alexa PUBLIC gmock/gmock-generated-function-mockers_test.cc)
target_sources(alexa PUBLIC gmock/gmock-generated-internal-utils_test.cc)
target_sources(alexa PUBLIC gmock/gmock-generated-matchers_test.cc)
target_sources(alexa PUBLIC gmock/gmock-internal-utils.cc)
target_sources(alexa PUBLIC gmock/gmock-internal-utils_test.cc)
target_sources(alexa PUBLIC gmock/gmock-matchers.cc)
target_sources(alexa PUBLIC gmock/gmock-matchers_test.cc)
target_sources(alexa PUBLIC gmock/gmock-more-actions_test.cc)
target_sources(alexa PUBLIC gmock/gmock-nice-strict_test.cc)
target_sources(alexa PUBLIC gmock/gmock-port_test.cc)
target_sources(alexa PUBLIC gmock/gmock-spec-builders.cc)
target_sources(alexa PUBLIC gmock/gmock-spec-builders_test.cc)
target_sources(alexa PUBLIC gmock/gmock_all_test.cc)
target_sources(alexa PUBLIC gmock/gmock_ex_test.cc)
target_sources(alexa PUBLIC gmock/gmock_leak_test_.cc)
target_sources(alexa PUBLIC gmock/gmock_link2_test.cc)
target_sources(alexa PUBLIC gmock/gmock_link_test.cc)
target_sources(alexa PUBLIC gmock/gmock_main.cc)
target_sources(alexa PUBLIC gmock/gmock_output_test_.cc)
target_sources(alexa PUBLIC gmock/gmock_stress_test.cc)
target_sources(alexa PUBLIC gmock/gmock_test.cc)
target_sources(alexa PUBLIC gmodule/gmodule.c)
target_sources(alexa PUBLIC gmodule/gmodule-ar.c)
target_sources(alexa PUBLIC gmodule/gmodule-beos.c)
target_sources(alexa PUBLIC gmodule/gmodule-dl.c)
target_sources(alexa PUBLIC gmodule/gmodule-dld.c)
target_sources(alexa PUBLIC gmodule/gmodule-dyld.c)
target_sources(alexa PUBLIC gmodule/gmodule-os2.c)
target_sources(alexa PUBLIC gmodule/gmodule-win32.c)
target_sources(alexa PUBLIC gobject/tests/binding.c)
target_sources(alexa PUBLIC gobject/tests/boxed.c)
target_sources(alexa PUBLIC gobject/tests/dynamictests.c)
target_sources(alexa PUBLIC gobject/tests/enums.c)
target_sources(alexa PUBLIC gobject/tests/ifaceproperties.c)
target_sources(alexa PUBLIC gobject/tests/param.c)
target_sources(alexa PUBLIC gobject/tests/properties.c)
target_sources(alexa PUBLIC gobject/tests/reference.c)
target_sources(alexa PUBLIC gobject/tests/threadtests.c)
target_sources(alexa PUBLIC gobject/gatomicarray.c)
target_sources(alexa PUBLIC gobject/gbinding.c)
target_sources(alexa PUBLIC gobject/gboxed.c)
target_sources(alexa PUBLIC gobject/gclosure.c)
target_sources(alexa PUBLIC gobject/genums.c)
target_sources(alexa PUBLIC gobject/glib-genmarshal.c)
target_sources(alexa PUBLIC gobject/gmarshal.c)
target_sources(alexa PUBLIC gobject/gobject.c)
target_sources(alexa PUBLIC gobject/gobject-query.c)
target_sources(alexa PUBLIC gobject/gobjectnotifyqueue.c)
target_sources(alexa PUBLIC gobject/gparam.c)
target_sources(alexa PUBLIC gobject/gparamspecs.c)
target_sources(alexa PUBLIC gobject/gsignal.c)
target_sources(alexa PUBLIC gobject/gsourceclosure.c)
target_sources(alexa PUBLIC gobject/gtype.c)
target_sources(alexa PUBLIC gobject/gtypemodule.c)
target_sources(alexa PUBLIC gobject/gtypeplugin.c)
target_sources(alexa PUBLIC gobject/gvalue.c)
target_sources(alexa PUBLIC gobject/gvaluearray.c)
target_sources(alexa PUBLIC gobject/gvaluetransform.c)
target_sources(alexa PUBLIC gobject/gvaluetypes.c)
target_sources(alexa PUBLIC gobject/testgobject.c)
target_sources(alexa PUBLIC gstreamer/ext/alsa/gstalsa.c)
target_sources(alexa PUBLIC gstreamer/ext/alsa/gstalsadeviceprovider.c)
target_sources(alexa PUBLIC gstreamer/ext/alsa/gstalsaelement.c)
target_sources(alexa PUBLIC gstreamer/ext/alsa/gstalsamidisrc.c)
target_sources(alexa PUBLIC gstreamer/ext/alsa/gstalsaplugin.c)
target_sources(alexa PUBLIC gstreamer/ext/alsa/gstalsasink.c)
target_sources(alexa PUBLIC gstreamer/ext/alsa/gstalsasrc.c)
target_sources(alexa PUBLIC gstreamer/ext/cdparanoia/gstcdparanoiasrc.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectblur.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectbulge.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectfisheye.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectglow.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectidentity.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectlaplacian.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectlumatocurve.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectmirror.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectrgbtocurve.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectscurves.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectsin.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectsobel.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectsquare.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectsqueeze.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectssources.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectstretch.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffecttunnel.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffecttwirl.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/effects/gstgleffectxray.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/caopengllayersink.m)
target_sources(alexa PUBLIC gstreamer/ext/gl/gltestsrc.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglalpha.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglbasemixer.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglbumper.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglcolorbalance.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglcolorconvertelement.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglcolorscale.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgldeinterlace.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgldifferencematte.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgldownloadelement.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgleffects.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglelement.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglfilterapp.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglfilterbin.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglfiltercube.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglfilterglass.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglfilterreflectedscreen.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglfiltershader.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglimagesink.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglmixer.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglmixerbin.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglmosaic.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgloverlay.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgloverlaycompositorelement.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglsinkbin.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglsrcbin.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglstereomix.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglstereosplit.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgltestsrc.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgltransformation.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstgluploadelement.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglutils.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglvideoflip.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglvideomixer.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstglviewconvert.c)
target_sources(alexa PUBLIC gstreamer/ext/gl/gstopengl.c)
target_sources(alexa PUBLIC gstreamer/ext/libvisual/plugin.c)
target_sources(alexa PUBLIC gstreamer/ext/libvisual/visual.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/dirac_parse.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/gstoggaviparse.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/gstoggdemux.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/gstoggmux.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/gstoggparse.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/gstoggplugin.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/gstoggstream.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/gstogmparse.c)
target_sources(alexa PUBLIC gstreamer/ext/ogg/vorbis_parse.c)
target_sources(alexa PUBLIC gstreamer/ext/opus/gstopus.c)
target_sources(alexa PUBLIC gstreamer/ext/opus/gstopuscommon.c)
target_sources(alexa PUBLIC gstreamer/ext/opus/gstopusdec.c)
target_sources(alexa PUBLIC gstreamer/ext/opus/gstopuselement.c)
target_sources(alexa PUBLIC gstreamer/ext/opus/gstopusenc.c)
target_sources(alexa PUBLIC gstreamer/ext/opus/gstopusheader.c)
target_sources(alexa PUBLIC gstreamer/ext/pango/gstbasetextoverlay.c)
target_sources(alexa PUBLIC gstreamer/ext/pango/gstclockoverlay.c)
target_sources(alexa PUBLIC gstreamer/ext/pango/gstpangoelement.c)
target_sources(alexa PUBLIC gstreamer/ext/pango/gstpangoplugin.c)
target_sources(alexa PUBLIC gstreamer/ext/pango/gsttextoverlay.c)
target_sources(alexa PUBLIC gstreamer/ext/pango/gsttextrender.c)
target_sources(alexa PUBLIC gstreamer/ext/pango/gsttimeoverlay.c)
target_sources(alexa PUBLIC gstreamer/ext/theora/gsttheora.c)
target_sources(alexa PUBLIC gstreamer/ext/theora/gsttheoradec.c)
target_sources(alexa PUBLIC gstreamer/ext/theora/gsttheoraenc.c)
target_sources(alexa PUBLIC gstreamer/ext/theora/gsttheoraparse.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbisdec.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbis.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbiscommon.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbisdec.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbisdeclib.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbiselement.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbisenc.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbisparse.c)
target_sources(alexa PUBLIC gstreamer/ext/vorbis/gstvorbistag.c)
target_sources(alexa PUBLIC gstreamer/gst/adder/gstadder.c)
target_sources(alexa PUBLIC gstreamer/gst/adder/gstadderorc-dist.c)
target_sources(alexa PUBLIC gstreamer/gst/app/gstapp.c)
target_sources(alexa PUBLIC gstreamer/gst/app/gstappsink.c)
target_sources(alexa PUBLIC gstreamer/gst/app/gstappsrc.c)
target_sources(alexa PUBLIC gstreamer/gst/audioconvert/gstaudioconvert.c)
target_sources(alexa PUBLIC gstreamer/gst/audioconvert/plugin.c)
target_sources(alexa PUBLIC gstreamer/gst/audiomixer/gstaudiointerleave.c)
target_sources(alexa PUBLIC gstreamer/gst/audiomixer/gstaudiomixer.c)
target_sources(alexa PUBLIC gstreamer/gst/audiomixer/gstaudiomixerelement.c)
target_sources(alexa PUBLIC gstreamer/gst/audiomixer/gstaudiomixerorc-dist.c)
target_sources(alexa PUBLIC gstreamer/gst/audiomixer/gstaudiomixerplugin.c)
target_sources(alexa PUBLIC gstreamer/gst/audiomixer/gstliveadder.c)
target_sources(alexa PUBLIC gstreamer/gst/audiorate/gstaudiorate.c)
target_sources(alexa PUBLIC gstreamer/gst/audioresample/gstaudioresample.c)
target_sources(alexa PUBLIC gstreamer/gst/audiotestsrc/gstaudiotestsrc.c)
target_sources(alexa PUBLIC gstreamer/gst/compositor/blend.c)
target_sources(alexa PUBLIC gstreamer/gst/compositor/compositor.c)
target_sources(alexa PUBLIC gstreamer/gst/compositor/compositororc-dist.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/gstencodebasebin.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/gstencodebin.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/gstencodebin2.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/gstencodingelements.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/gstsmartencoder.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/gststreamcombiner.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/gststreamsplitter.c)
target_sources(alexa PUBLIC gstreamer/gst/encoding/plugin.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgio.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgiobasesink.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgiobasesrc.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgioelement.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgiosink.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgiosrc.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgiostreamsink.c)
target_sources(alexa PUBLIC gstreamer/gst/gio/gstgiostreamsrc.c)
target_sources(alexa PUBLIC gstreamer/gst/overlaycomposition/gstoverlaycomposition.c)
target_sources(alexa PUBLIC gstreamer/gst/pbtypes/gstpbtypes.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstdecodebin2.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstdecodebin3.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstdecodebin3-parse.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstparsebin.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplay-enum.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaybackelement.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaybackplugin.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaybackutils.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaybin2.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaybin3.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaysink.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaysinkaudioconvert.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaysinkconvertbin.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstplaysinkvideoconvert.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gststreamsynchronizer.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gstsubtitleoverlay.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gsturidecodebin.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gsturidecodebin3.c)
target_sources(alexa PUBLIC gstreamer/gst/playback/gsturisourcebin.c)
target_sources(alexa PUBLIC gstreamer/gst/printf/asnprintf.c)
target_sources(alexa PUBLIC gstreamer/gst/printf/printf.c)
target_sources(alexa PUBLIC gstreamer/gst/printf/printf-args.c)
target_sources(alexa PUBLIC gstreamer/gst/printf/printf-extension.c)
target_sources(alexa PUBLIC gstreamer/gst/printf/printf-parse.c)
target_sources(alexa PUBLIC gstreamer/gst/printf/vasnprintf.c)
target_sources(alexa PUBLIC gstreamer/gst/rawparse/gstrawaudioparse.c)
target_sources(alexa PUBLIC gstreamer/gst/rawparse/gstrawbaseparse.c)
target_sources(alexa PUBLIC gstreamer/gst/rawparse/gstrawvideoparse.c)
target_sources(alexa PUBLIC gstreamer/gst/rawparse/gstunalignedaudioparse.c)
target_sources(alexa PUBLIC gstreamer/gst/rawparse/gstunalignedvideoparse.c)
target_sources(alexa PUBLIC gstreamer/gst/rawparse/plugin.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/gstssaparse.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/gstsubparse.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/gstsubparseelement.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/gstsubparseplugin.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/mpl2parse.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/qttextparse.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/samiparse.c)
target_sources(alexa PUBLIC gstreamer/gst/subparse/tmplayerparse.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gstmultifdsink.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gstmultihandlesink.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gstmultisocketsink.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gstsocketsrc.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gsttcpclientsink.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gsttcpclientsrc.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gsttcpelements.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gsttcpplugin.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gsttcpserversink.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gsttcpserversrc.c)
target_sources(alexa PUBLIC gstreamer/gst/tcp/gsttcpsrcstats.c)
target_sources(alexa PUBLIC gstreamer/gst/typefind/gsttypefindfunctions.c)
target_sources(alexa PUBLIC gstreamer/gst/typefind/gsttypefindfunctionsdata.c)
target_sources(alexa PUBLIC gstreamer/gst/typefind/gsttypefindfunctionsplugin.c)
target_sources(alexa PUBLIC gstreamer/gst/typefind/gsttypefindfunctionsriff.c)
target_sources(alexa PUBLIC gstreamer/gst/typefind/gsttypefindfunctionsstartwith.c)
target_sources(alexa PUBLIC gstreamer/gst/videoconvert/gstvideoconvert.c)
target_sources(alexa PUBLIC gstreamer/gst/videorate/gstvideorate.c)
target_sources(alexa PUBLIC gstreamer/gst/videoscale/gstvideoscale.c)
target_sources(alexa PUBLIC gstreamer/gst/videotestsrc/generate_sine_table.c)
target_sources(alexa PUBLIC gstreamer/gst/videotestsrc/gstvideotestsrc.c)
target_sources(alexa PUBLIC gstreamer/gst/videotestsrc/gstvideotestsrcorc-dist.c)
target_sources(alexa PUBLIC gstreamer/gst/videotestsrc/videotestsrc.c)
target_sources(alexa PUBLIC gstreamer/gst/volume/gstvolume.c)
target_sources(alexa PUBLIC gstreamer/gst/volume/gstvolumeorc-dist.c)
target_sources(alexa PUBLIC gstreamer/gst/glib-compat.c)
target_sources(alexa PUBLIC gstreamer/gst/gst.c)
target_sources(alexa PUBLIC gstreamer/gst/gstallocator.c)
target_sources(alexa PUBLIC gstreamer/gst/gstatomicqueue.c)
target_sources(alexa PUBLIC gstreamer/gst/gstbin.c)
target_sources(alexa PUBLIC gstreamer/gst/gstbuffer.c)
target_sources(alexa PUBLIC gstreamer/gst/gstbufferlist.c)
target_sources(alexa PUBLIC gstreamer/gst/gstbufferpool.c)
target_sources(alexa PUBLIC gstreamer/gst/gstbus.c)
target_sources(alexa PUBLIC gstreamer/gst/gstcaps.c)
target_sources(alexa PUBLIC gstreamer/gst/gstcapsfeatures.c)
target_sources(alexa PUBLIC gstreamer/gst/gstchildproxy.c)
target_sources(alexa PUBLIC gstreamer/gst/gstclock.c)
target_sources(alexa PUBLIC gstreamer/gst/gstclock-linreg.c)
target_sources(alexa PUBLIC gstreamer/gst/gstcontext.c)
target_sources(alexa PUBLIC gstreamer/gst/gstcontrolbinding.c)
target_sources(alexa PUBLIC gstreamer/gst/gstcontrolsource.c)
target_sources(alexa PUBLIC gstreamer/gst/gstdatetime.c)
target_sources(alexa PUBLIC gstreamer/gst/gstdebugutils.c)
target_sources(alexa PUBLIC gstreamer/gst/gstdevice.c)
target_sources(alexa PUBLIC gstreamer/gst/gstdevicemonitor.c)
target_sources(alexa PUBLIC gstreamer/gst/gstdeviceprovider.c)
target_sources(alexa PUBLIC gstreamer/gst/gstdeviceproviderfactory.c)
target_sources(alexa PUBLIC gstreamer/gst/gstelement.c)
target_sources(alexa PUBLIC gstreamer/gst/gstelementfactory.c)
target_sources(alexa PUBLIC gstreamer/gst/gsterror.c)
target_sources(alexa PUBLIC gstreamer/gst/gstevent.c)
target_sources(alexa PUBLIC gstreamer/gst/gstformat.c)
target_sources(alexa PUBLIC gstreamer/gst/gstghostpad.c)
target_sources(alexa PUBLIC gstreamer/gst/gstinfo.c)
target_sources(alexa PUBLIC gstreamer/gst/gstiterator.c)
target_sources(alexa PUBLIC gstreamer/gst/gstmemory.c)
target_sources(alexa PUBLIC gstreamer/gst/gstmessage.c)
target_sources(alexa PUBLIC gstreamer/gst/gstmeta.c)
target_sources(alexa PUBLIC gstreamer/gst/gstminiobject.c)
target_sources(alexa PUBLIC gstreamer/gst/gstobject.c)
target_sources(alexa PUBLIC gstreamer/gst/gstpad.c)
target_sources(alexa PUBLIC gstreamer/gst/gstpadtemplate.c)
target_sources(alexa PUBLIC gstreamer/gst/gstparamspecs.c)
target_sources(alexa PUBLIC gstreamer/gst/gstparse.c)
target_sources(alexa PUBLIC gstreamer/gst/gstpipeline.c)
target_sources(alexa PUBLIC gstreamer/gst/gstplugin.c)
target_sources(alexa PUBLIC gstreamer/gst/gstpluginfeature.c)
target_sources(alexa PUBLIC gstreamer/gst/gstpluginloader.c)
target_sources(alexa PUBLIC gstreamer/gst/gstpoll.c)
target_sources(alexa PUBLIC gstreamer/gst/gstpreset.c)
target_sources(alexa PUBLIC gstreamer/gst/gstprotection.c)
target_sources(alexa PUBLIC gstreamer/gst/gstquark.c)
target_sources(alexa PUBLIC gstreamer/gst/gstquery.c)
target_sources(alexa PUBLIC gstreamer/gst/gstregistry.c)
target_sources(alexa PUBLIC gstreamer/gst/gstregistrybinary.c)
target_sources(alexa PUBLIC gstreamer/gst/gstregistrychunks.c)
target_sources(alexa PUBLIC gstreamer/gst/gstsample.c)
target_sources(alexa PUBLIC gstreamer/gst/gstsegment.c)
target_sources(alexa PUBLIC gstreamer/gst/gststructure.c)
target_sources(alexa PUBLIC gstreamer/gst/gstsystemclock.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttaglist.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttagsetter.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttask.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttaskpool.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttoc.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttocsetter.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttrace.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttracer.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttracerfactory.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttracerrecord.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttracerutils.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttypefind.c)
target_sources(alexa PUBLIC gstreamer/gst/gsttypefindfactory.c)
target_sources(alexa PUBLIC gstreamer/gst/gsturi.c)
target_sources(alexa PUBLIC gstreamer/gst/gstutils.c)
target_sources(alexa PUBLIC gstreamer/gst/gstvalue.c)
target_sources(alexa PUBLIC gstreamer/libs/allocators/gstdmabuf.c)
target_sources(alexa PUBLIC gstreamer/libs/allocators/gstfdmemory.c)
target_sources(alexa PUBLIC gstreamer/libs/allocators/gstphysmemory.c)
target_sources(alexa PUBLIC gstreamer/libs/app/gstappsink.c)
target_sources(alexa PUBLIC gstreamer/libs/app/gstappsrc.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-buffer.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-channel-mixer.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-channels.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-converter.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-format.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-info.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-quantize.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-resampler.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-resampler-x86-sse.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-resampler-x86-sse2.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/audio-resampler-x86-sse41.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/dbesi0.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudioaggregator.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiobasesink.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiobasesrc.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiocdsrc.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudioclock.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiodecoder.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudioencoder.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiofilter.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudioiec61937.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiometa.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiopack-dist.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudioringbuffer.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiosink.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiosrc.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudiostreamalign.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/gstaudioutilsprivate.c)
target_sources(alexa PUBLIC gstreamer/libs/audio/streamvolume.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstadapter.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstbaseparse.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstbasesink.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstbasesrc.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstbasetransform.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstbitreader.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstbytereader.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstbytewriter.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstcollectpads.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstdataqueue.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstflowcombiner.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstindex.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstmemindex.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstpushsrc.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gstqueuearray.c)
target_sources(alexa PUBLIC gstreamer/libs/base/gsttypefindhelper.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/alarm.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_error.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_list.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_log.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_msg.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_pack.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_print.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_run.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/check_str.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/clock_gettime.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/libcompat.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/localtime_r.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/strsignal.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/timer_create.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/timer_delete.c)
target_sources(alexa PUBLIC gstreamer/libs/check/libcheck/timer_settime.c)
target_sources(alexa PUBLIC gstreamer/libs/check/gstbufferstraw.c)
target_sources(alexa PUBLIC gstreamer/libs/check/gstcheck.c)
target_sources(alexa PUBLIC gstreamer/libs/check/gstconsistencychecker.c)
target_sources(alexa PUBLIC gstreamer/libs/check/gstharness.c)
target_sources(alexa PUBLIC gstreamer/libs/check/gsttestclock.c)
target_sources(alexa PUBLIC gstreamer/libs/controller/gstargbcontrolbinding.c)
target_sources(alexa PUBLIC gstreamer/libs/controller/gstdirectcontrolbinding.c)
target_sources(alexa PUBLIC gstreamer/libs/controller/gstinterpolationcontrolsource.c)
target_sources(alexa PUBLIC gstreamer/libs/controller/gstlfocontrolsource.c)
target_sources(alexa PUBLIC gstreamer/libs/controller/gsttimedvaluecontrolsource.c)
target_sources(alexa PUBLIC gstreamer/libs/controller/gsttriggercontrolsource.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/gstfft.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/gstfftf32.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/gstfftf64.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/gstffts16.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/gstffts32.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fft_f32.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fft_f64.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fft_s16.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fft_s32.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fftr_f32.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fftr_f64.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fftr_s16.c)
target_sources(alexa PUBLIC gstreamer/libs/fft/kiss_fftr_s32.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/android/gstglwindow_android_egl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/cocoa/gstglcaopengllayer.m)
target_sources(alexa PUBLIC gstreamer/libs/gl/cocoa/gstglcontext_cocoa.m)
target_sources(alexa PUBLIC gstreamer/libs/gl/cocoa/gstgldisplay_cocoa.m)
target_sources(alexa PUBLIC gstreamer/libs/gl/cocoa/gstglwindow_cocoa.m)
target_sources(alexa PUBLIC gstreamer/libs/gl/dispmanx/gstglwindow_dispmanx_egl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/eagl/gstglcontext_eagl.m)
target_sources(alexa PUBLIC gstreamer/libs/gl/eagl/gstglwindow_eagl.m)
target_sources(alexa PUBLIC gstreamer/libs/gl/egl/gstegl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/egl/gsteglimage.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/egl/gstglcontext_egl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/egl/gstgldisplay_egl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/egl/gstgldisplay_egl_device.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/egl/gstglmemoryegl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gbm/gstgl_gbm_utils.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gbm/gstgldisplay_gbm.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gbm/gstglwindow_gbm_egl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/viv-fb/gstgldisplay_viv_fb.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/viv-fb/gstglwindow_viv_fb_egl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/wayland/gstgldisplay_wayland.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/wayland/gstglwindow_wayland_egl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/wayland/wayland_event_source.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/wgl/gstglcontext_wgl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/win32/gstglwindow_win32.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/winrt/gstglwindow_winrt_egl.cpp)
target_sources(alexa PUBLIC gstreamer/libs/gl/x11/gstglcontext_glx.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/x11/gstgldisplay_x11.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/x11/gstglwindow_x11.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/x11/xcb_event_source.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglapi.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglbasefilter.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglbasememory.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglbasesrc.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglbuffer.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglbufferpool.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglcolorconvert.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglcontext.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglcontextconfig.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstgldebug.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstgldisplay.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglfeature.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglfilter.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglformat.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglframebuffer.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglmemory.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglmemorypbo.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstgloverlaycompositor.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglpixelformat.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglquery.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglrenderbuffer.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglshader.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglshaderstrings.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglsl.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglslstage.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglsyncmeta.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglupload.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglutils.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglviewconvert.c)
target_sources(alexa PUBLIC gstreamer/libs/gl/gstglwindow.c)
target_sources(alexa PUBLIC gstreamer/libs/helpers/gst-completion-helper.c)
target_sources(alexa PUBLIC gstreamer/libs/helpers/gst-plugin-scanner.c)
target_sources(alexa PUBLIC gstreamer/libs/helpers/gst-ptp-helper.c)
target_sources(alexa PUBLIC gstreamer/libs/net/gstnetaddressmeta.c)
target_sources(alexa PUBLIC gstreamer/libs/net/gstnetclientclock.c)
target_sources(alexa PUBLIC gstreamer/libs/net/gstnetcontrolmessagemeta.c)
target_sources(alexa PUBLIC gstreamer/libs/net/gstnettimepacket.c)
target_sources(alexa PUBLIC gstreamer/libs/net/gstnettimeprovider.c)
target_sources(alexa PUBLIC gstreamer/libs/net/gstntppacket.c)
target_sources(alexa PUBLIC gstreamer/libs/net/gstptpclock.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/codec-utils.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/descriptions.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/encoding-profile.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/encoding-target.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/gstaudiovisualizer.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/gstdiscoverer.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/gstdiscoverer-types.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/gstpluginsbaseversion.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/install-plugins.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/missing-plugins.c)
target_sources(alexa PUBLIC gstreamer/libs/pbutils/pbutils.c)
target_sources(alexa PUBLIC gstreamer/libs/riff/riff.c)
target_sources(alexa PUBLIC gstreamer/libs/riff/riff-media.c)
target_sources(alexa PUBLIC gstreamer/libs/riff/riff-read.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtcpbuffer.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtpbaseaudiopayload.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtpbasedepayload.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtpbasepayload.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtpbuffer.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtphdrext.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtpmeta.c)
target_sources(alexa PUBLIC gstreamer/libs/rtp/gstrtppayloads.c)
target_sources(alexa PUBLIC gstreamer/libs/rtsp/gstrtspconnection.c)
target_sources(alexa PUBLIC gstreamer/libs/rtsp/gstrtspdefs.c)
target_sources(alexa PUBLIC gstreamer/libs/rtsp/gstrtspextension.c)
target_sources(alexa PUBLIC gstreamer/libs/rtsp/gstrtspmessage.c)
target_sources(alexa PUBLIC gstreamer/libs/rtsp/gstrtsprange.c)
target_sources(alexa PUBLIC gstreamer/libs/rtsp/gstrtsptransport.c)
target_sources(alexa PUBLIC gstreamer/libs/rtsp/gstrtspurl.c)
target_sources(alexa PUBLIC gstreamer/libs/sdp/gstmikey.c)
target_sources(alexa PUBLIC gstreamer/libs/sdp/gstsdpmessage.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/gstexiftag.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/gstid3tag.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/gsttagdemux.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/gsttageditingprivate.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/gsttagmux.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/gstvorbistag.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/gstxmptag.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/id3v2.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/id3v2frames.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/lang.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/lang-tables.dat)
target_sources(alexa PUBLIC gstreamer/libs/tag/licenses.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/licenses-tables.dat)
target_sources(alexa PUBLIC gstreamer/libs/tag/mklangtables.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/mklicensestables.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/tags.c)
target_sources(alexa PUBLIC gstreamer/libs/tag/xmpwriter.c)
target_sources(alexa PUBLIC gstreamer/libs/video/colorbalance.c)
target_sources(alexa PUBLIC gstreamer/libs/video/colorbalancechannel.c)
target_sources(alexa PUBLIC gstreamer/libs/video/convertframe.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideoaffinetransformationmeta.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideoaggregator.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideocodecalphameta.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideodecoder.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideoencoder.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideofilter.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideometa.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideopool.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideosink.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideotimecode.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideoutils.c)
target_sources(alexa PUBLIC gstreamer/libs/video/gstvideoutilsprivate.c)
target_sources(alexa PUBLIC gstreamer/libs/video/navigation.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-anc.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-blend.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-chroma.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-color.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-converter.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-dither.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-event.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-format.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-frame.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-hdr.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-info.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-multiview.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-orc-dist.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-overlay-composition.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-resampler.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-scaler.c)
target_sources(alexa PUBLIC gstreamer/libs/video/video-tile.c)
target_sources(alexa PUBLIC gstreamer/libs/video/videodirection.c)
target_sources(alexa PUBLIC gstreamer/libs/video/videoorientation.c)
target_sources(alexa PUBLIC gstreamer/libs/video/videooverlay.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstcapsfilter.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstconcat.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstdownloadbuffer.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstelements.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstelements_private.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstfakesink.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstfakesrc.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstfdsink.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstfdsrc.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstfilesink.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstfilesrc.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstfunnel.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstidentity.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstinputselector.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstmultiqueue.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstoutputselector.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstqueue.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstqueue2.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstsparsefile.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gststreamiddemux.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gsttee.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gsttypefindelement.c)
target_sources(alexa PUBLIC gstreamer/plugins/elements/gstvalve.c)
target_sources(alexa PUBLIC gstreamer/plugins/tracers/gstlatency.c)
target_sources(alexa PUBLIC gstreamer/plugins/tracers/gstlog.c)
target_sources(alexa PUBLIC gstreamer/plugins/tracers/gstrusage.c)
target_sources(alexa PUBLIC gstreamer/plugins/tracers/gststats.c)
target_sources(alexa PUBLIC gstreamer/plugins/tracers/gsttracers.c)
target_sources(alexa PUBLIC gstreamer/sys/ximage/ximage.c)
target_sources(alexa PUBLIC gstreamer/sys/ximage/ximagepool.c)
target_sources(alexa PUBLIC gstreamer/sys/ximage/ximagesink.c)
target_sources(alexa PUBLIC gstreamer/sys/xvimage/xvcontext.c)
target_sources(alexa PUBLIC gstreamer/sys/xvimage/xvimage.c)
target_sources(alexa PUBLIC gstreamer/sys/xvimage/xvimageallocator.c)
target_sources(alexa PUBLIC gstreamer/sys/xvimage/xvimagepool.c)
target_sources(alexa PUBLIC gstreamer/sys/xvimage/xvimagesink.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/caps.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/capsnego.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/complexity.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/controller.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/gstbufferstress.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/gstclockstress.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/gstpollstress.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/gstpoolstress.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/init.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/mass-elements.c)
target_sources(alexa PUBLIC gstreamer/tests/benchmarks/tracerserialize.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/adder.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/appsink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/appsrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/audioconvert.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/audiointerleave.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/audiomixer.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/audiorate.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/audioresample.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/audiotestsrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/capsfilter.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/compositor.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/concat.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/decodebin.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/encodebin.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/fakesink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/fakesrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/fdsrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/filesink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/filesrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/funnel.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/glbin.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/glfilter.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/glimagesink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/glmixer.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/glstereo.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/identity.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/libvisual.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/multifdsink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/multiqueue.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/multisocketsink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/opus.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/overlaycomposition.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/playbin.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/playbin-complex.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/playsink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/queue.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/queue2.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/rawaudioparse.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/rawvideoparse.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/selector.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/streamiddemux.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/streamsynchronizer.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/subparse.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/tee.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/textoverlay.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/urisourcebin.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/valve.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/videoconvert.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/videorate.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/videoscale.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/videotestsrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/volume.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/vorbisdec.c)
target_sources(alexa PUBLIC gstreamer/tests/check/elements/vorbistag.c)
target_sources(alexa PUBLIC gstreamer/tests/check/generic/clock-selection.c)
target_sources(alexa PUBLIC gstreamer/tests/check/generic/sinks.c)
target_sources(alexa PUBLIC gstreamer/tests/check/generic/states.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gst.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstabi.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstatomicqueue.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstbin.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstbuffer.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstbufferlist.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstbufferpool.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstbus.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstcaps.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstcapsfeatures.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstchildproxy.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstclock.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstcontext.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstcontroller.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstcpp.cc)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstdatetime.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstdevice.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstelement.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstelementfactory.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstevent.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstghostpad.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstindex.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstinfo.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstiterator.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstmemory.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstmessage.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstmeta.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstminiobject.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstobject.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstpad.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstparamspecs.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstpipeline.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstplugin.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstpoll.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstpreset.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstprintf.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstprotection.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstquery.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstregistry.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstsegment.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gststructure.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstsystemclock.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gsttag.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gsttagsetter.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gsttask.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gsttoc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gsttocsetter.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gsttracerrecord.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gsturi.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstutils.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/gstvalue.c)
target_sources(alexa PUBLIC gstreamer/tests/check/gst/typefindfunctions.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/adapter.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/allocators.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/audio.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/audiocdsrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/audiodecoder.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/audioencoder.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/audiosink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/baseaudiovisualizer.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/baseparse.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/basesink.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/basesrc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/bitreader.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/bitreader-noinline.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/bytereader.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/bytereader-noinline.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/bytewriter.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/bytewriter-noinline.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/collectpads.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/controller.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/discoverer.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/fft.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/flowcombiner.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglapi.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglcolorconvert.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglcontext.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglfeature.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglformat.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglheaders.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglmatrix.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglmemory.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglquery.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglshader.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglsl.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglslstage.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstglupload.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstharness.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstlibscpp.cc)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstnetclientclock.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gstnettimeprovider.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/gsttestclock.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/libsabi.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/mikey.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/navigation.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/pbutils.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/profile.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/queuearray.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtp.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtpbasedepayload.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtpbasepayload.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtpdummyhdrextimpl.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtphdrext.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtpmeta.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtsp.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/rtspconnection.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/sdp.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/sparsefile.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/tag.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/test_transform.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/transform1.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/transform2.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/typefindhelper.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/video.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/videoanc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/videodecoder.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/videoencoder.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/videotimecode.c)
target_sources(alexa PUBLIC gstreamer/tests/check/libs/xmpwriter.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/basetime.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/capsfilter-renegotiation.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/cleanup.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/gio.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/gl-launch-lines.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/oggmux.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/parse-disabled.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/parse-launch.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/queue-error.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/seek.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/simple-launch-lines.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/streamsynchronizer.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/stress.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/tcp.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/theoraenc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/vorbisdec.c)
target_sources(alexa PUBLIC gstreamer/tests/check/pipelines/vorbisenc.c)
target_sources(alexa PUBLIC gstreamer/tests/check/tools/gstinspect.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/adapter/adapter_test.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/app/appsink-src.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/app/appsink-src2.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/app/appsrc-ra.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/app/appsrc-seekable.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/app/appsrc-stream.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/app/appsrc-stream2.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/app/appsrc_ex.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/audio/audiomix.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/audio/volume.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/compositor/crossfade.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/compositor/mosaic.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/compositor/signals.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/controller/absolute-example.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/controller/audio-example.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/controller/control-sources.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/controller/controller-graph.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/controller/text-color-example.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/decodebin_next/decodebin3.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/decodebin_next/playbin-test.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/decodebin_next/uridecodebin3-select-all.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/device-provider/example-device-provider.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/dynamic/addstream.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/dynamic/codec-select.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/dynamic/sprinkle.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/dynamic/sprinkle2.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/dynamic/sprinkle3.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/encoding/encoding.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/encoding/gstcapslist.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/fft/fftrange.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/gio/giosrc-mounting.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/cocoa/cocoa-videooverlay.m)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/generic/cube/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/generic/cubeyuv/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/generic/doublecube/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/generic/recordgraphic/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/3dvideo/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/3dvideo/mviewwidget.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/filternovideooverlay/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/filtervideooverlay/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/fxtest/fxtest.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/fxtest/pixbufdrop.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/switchvideooverlay/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/gtk/gstgtk.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/mousevideooverlay/gstthread.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/mousevideooverlay/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/mousevideooverlay/pipeline.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/mousevideooverlay/qrenderer.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/gstthread.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/pipeline.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwidgetvideooverlay/qglrenderer.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwtextureshare/cocoa_utils.mm)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwtextureshare/gstthread.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwtextureshare/main.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwtextureshare/pipeline.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/qglwtextureshare/qglrenderer.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/qt/videooverlay/videooverlay.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/sdl/sdlshare.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/gl/sdl/sdlshare2.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/helloworld/helloworld.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/memory/memory_test.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/memory/my-memory.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/memory/my-vidmem.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/netclock/netclock-client.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/netclock/netclock-server.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/overlay/gtk-videooverlay.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/overlay/qt-videooverlay.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/overlay/qtgv-videooverlay.cpp)
target_sources(alexa PUBLIC gstreamer/tests/examples/overlay/win32-videooverlay.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/overlay/win32-videooverlay-playbin.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/overlaycomposition/overlaycomposition.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/playback/playback-test.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/playrec/playrec.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/ptp/ptp-print-times.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/seek/instant-rate-change.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/seek/jsseek.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/seek/scrubby.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/seek/stepping.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/seek/stepping2.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/snapshot/snapshot.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/stepping/framestep1.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/streamiddemux/streamiddemux-stream.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/streams/rtpool-test.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/streams/schedule-tasks.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/streams/stream-status.c)
target_sources(alexa PUBLIC gstreamer/tests/examples/streams/testrtpool.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/decodetest.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/test.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/test2.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/test3.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/test4.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/test5.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/test6.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playback/test7.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/audio-trickplay.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/benchmark-appsink.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/benchmark-appsrc.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/benchmark-video-conversion.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/input-selector-test.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/output-selector-test.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/playbin-text.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/position-formats.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/stress-playbin.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/stress-videooverlay.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-box.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-colorkey.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-effect-switch.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-overlay-blending.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-resample.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-reverseplay.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-scale.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-textoverlay.c)
target_sources(alexa PUBLIC gstreamer/tests/icles/test-videooverlay.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-device-monitor.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-discoverer.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-inspect.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-launch.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-play.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-play-kb.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-stats.c)
target_sources(alexa PUBLIC gstreamer/tools/gst-typefind.c)
target_sources(alexa PUBLIC gtest/codegear/gtest_all.cc)
target_sources(alexa PUBLIC gtest/codegear/gtest_link.cc)
target_sources(alexa PUBLIC gtest/samples/sample1.cc)
target_sources(alexa PUBLIC gtest/samples/sample1_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample2.cc)
target_sources(alexa PUBLIC gtest/samples/sample2_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample3_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample4.cc)
target_sources(alexa PUBLIC gtest/samples/sample4_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample5_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample6_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample7_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample8_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample9_unittest.cc)
target_sources(alexa PUBLIC gtest/samples/sample10_unittest.cc)
target_sources(alexa PUBLIC gtest/gtest.cc)
target_sources(alexa PUBLIC gtest/gtest-all.cc)
target_sources(alexa PUBLIC gtest/gtest-death-test.cc)
target_sources(alexa PUBLIC gtest/gtest-death-test_ex_test.cc)
target_sources(alexa PUBLIC gtest/gtest-death-test_test.cc)
target_sources(alexa PUBLIC gtest/gtest-filepath.cc)
target_sources(alexa PUBLIC gtest/gtest-filepath_test.cc)
target_sources(alexa PUBLIC gtest/gtest-linked_ptr_test.cc)
target_sources(alexa PUBLIC gtest/gtest-listener_test.cc)
target_sources(alexa PUBLIC gtest/gtest-message_test.cc)
target_sources(alexa PUBLIC gtest/gtest-options_test.cc)
target_sources(alexa PUBLIC gtest/gtest-param-test2_test.cc)
target_sources(alexa PUBLIC gtest/gtest-param-test_test.cc)
target_sources(alexa PUBLIC gtest/gtest-port.cc)
target_sources(alexa PUBLIC gtest/gtest-port_test.cc)
target_sources(alexa PUBLIC gtest/gtest-printers.cc)
target_sources(alexa PUBLIC gtest/gtest-printers_test.cc)
target_sources(alexa PUBLIC gtest/gtest-test-part.cc)
target_sources(alexa PUBLIC gtest/gtest-test-part_test.cc)
target_sources(alexa PUBLIC gtest/gtest-tuple_test.cc)
target_sources(alexa PUBLIC gtest/gtest-typed-test.cc)
target_sources(alexa PUBLIC gtest/gtest-typed-test2_test.cc)
target_sources(alexa PUBLIC gtest/gtest-typed-test_test.cc)
target_sources(alexa PUBLIC gtest/gtest-unittest-api_test.cc)
target_sources(alexa PUBLIC gtest/gtest_all_test.cc)
target_sources(alexa PUBLIC gtest/gtest_break_on_failure_unittest_.cc)
target_sources(alexa PUBLIC gtest/gtest_catch_exceptions_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_color_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_env_var_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_environment_test.cc)
target_sources(alexa PUBLIC gtest/gtest_filter_unittest_.cc)
target_sources(alexa PUBLIC gtest/gtest_help_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_list_tests_unittest_.cc)
target_sources(alexa PUBLIC gtest/gtest_main.cc)
target_sources(alexa PUBLIC gtest/gtest_main_unittest.cc)
target_sources(alexa PUBLIC gtest/gtest_no_test_unittest.cc)
target_sources(alexa PUBLIC gtest/gtest_output_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_pred_impl_unittest.cc)
target_sources(alexa PUBLIC gtest/gtest_premature_exit_test.cc)
target_sources(alexa PUBLIC gtest/gtest_prod_test.cc)
target_sources(alexa PUBLIC gtest/gtest_repeat_test.cc)
target_sources(alexa PUBLIC gtest/gtest_shuffle_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_sole_header_test.cc)
target_sources(alexa PUBLIC gtest/gtest_stress_test.cc)
target_sources(alexa PUBLIC gtest/gtest_throw_on_failure_ex_test.cc)
target_sources(alexa PUBLIC gtest/gtest_throw_on_failure_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_uninitialized_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_unittest.cc)
target_sources(alexa PUBLIC gtest/gtest_xml_outfile1_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_xml_outfile2_test_.cc)
target_sources(alexa PUBLIC gtest/gtest_xml_output_unittest_.cc)
target_sources(alexa PUBLIC gtest/production.cc)
target_sources(alexa PUBLIC gthread/tests/1bit-mutex.c)
target_sources(alexa PUBLIC gthread/gthread-impl.c)
target_sources(alexa PUBLIC gthread/gthread-none.c)
target_sources(alexa PUBLIC gthread/gthread-posix.c)
target_sources(alexa PUBLIC gthread/gthread-win32.c)
target_sources(alexa PUBLIC integration/ACLTestContext.cpp)
target_sources(alexa PUBLIC integration/AipStateObserver.cpp)
target_sources(alexa PUBLIC integration/AlertsIntegrationTest.cpp)
target_sources(alexa PUBLIC integration/AlexaAuthorizationDelegateTest.cpp)
target_sources(alexa PUBLIC integration/AlexaCommunicationsLibraryTest.cpp)
target_sources(alexa PUBLIC integration/AlexaDirectiveSequencerLibraryTest.cpp)
target_sources(alexa PUBLIC integration/AudioInputProcessorIntegrationTest.cpp)
target_sources(alexa PUBLIC integration/AudioPlayerIntegrationTest.cpp)
target_sources(alexa PUBLIC integration/AuthDelegateTestContext.cpp)
target_sources(alexa PUBLIC integration/AuthObserver.cpp)
target_sources(alexa PUBLIC integration/ClientMessageHandler.cpp)
target_sources(alexa PUBLIC integration/ConnectionStatusObserver.cpp)
target_sources(alexa PUBLIC integration/NetworkIntegrationTests.cpp)
target_sources(alexa PUBLIC integration/ObservableMessageRequest.cpp)
target_sources(alexa PUBLIC integration/SDKTestContext.cpp)
target_sources(alexa PUBLIC integration/ServerDisconnectIntegrationTest.cpp)
target_sources(alexa PUBLIC integration/SpeechSynthesizerIntegrationTest.cpp)
target_sources(alexa PUBLIC integration/TestAlertObserver.cpp)
target_sources(alexa PUBLIC integration/TestDirectiveHandler.cpp)
target_sources(alexa PUBLIC integration/TestExceptionEncounteredSender.cpp)
target_sources(alexa PUBLIC integration/TestMediaPlayer.cpp)
target_sources(alexa PUBLIC integration/TestMessageSender.cpp)
target_sources(alexa PUBLIC integration/TestSpeechSynthesizerObserver.cpp)
target_sources(alexa PUBLIC interrupt_model/InterruptModel.cpp)
target_sources(alexa PUBLIC interrupt_model/InterruptModelConfiguration.cpp)
target_sources(alexa PUBLIC interrupt_model/InterruptModelTest.cpp)
target_sources(alexa PUBLIC json/allocators.cpp)
target_sources(alexa PUBLIC json/JSONGenerator.cpp)
target_sources(alexa PUBLIC json/JSONUtils.cpp)
target_sources(alexa PUBLIC json/reader.cpp)
target_sources(alexa PUBLIC kwd/kittai/KittAiKeyWordDetector.cpp)
target_sources(alexa PUBLIC kwd/kittai/KittAiKeyWordDetectorTest.cpp)
target_sources(alexa PUBLIC kwd/kittai/SnowboyWrapper.cpp)
target_sources(alexa PUBLIC kwd/kwd_provider/KeywordDetectorProvider.cpp)
target_sources(alexa PUBLIC kwd/sensory/SensoryKeywordDetector.cpp)
target_sources(alexa PUBLIC kwd/sensory/SensoryKeywordDetectorTest.cpp)
target_sources(alexa PUBLIC kwd/AbstractKeywordDetector.cpp)
target_sources(alexa PUBLIC kwd/AbstractKeywordDetectorTest.cpp)
target_sources(alexa PUBLIC logger/ConsoleLogger.cpp)
target_sources(alexa PUBLIC logger/Level.cpp)
target_sources(alexa PUBLIC logger/LogEntry.cpp)
target_sources(alexa PUBLIC logger/LogEntryBuffer.cpp)
target_sources(alexa PUBLIC logger/LogEntryStream.cpp)
target_sources(alexa PUBLIC logger/Logger.cpp)
target_sources(alexa PUBLIC logger/LoggerSinkManager.cpp)
target_sources(alexa PUBLIC logger/LoggerUtils.cpp)
target_sources(alexa PUBLIC logger/LogStringFormatter.cpp)
target_sources(alexa PUBLIC logger/ModuleLogger.cpp)
target_sources(alexa PUBLIC logger/ThreadMoniker.cpp)
target_sources(alexa PUBLIC media_player/AndroidSLESMediaPlayer.cpp)
target_sources(alexa PUBLIC media_player/AndroidSLESMediaPlayerTest.cpp)
target_sources(alexa PUBLIC media_player/AndroidSLESMediaQueue.cpp)
target_sources(alexa PUBLIC media_player/AndroidSLESMediaQueueTest.cpp)
target_sources(alexa PUBLIC media_player/AndroidSLESSpeaker.cpp)
target_sources(alexa PUBLIC media_player/AndroidSLESSpeakerTest.cpp)
target_sources(alexa PUBLIC media_player/AttachmentReaderSource.cpp)
target_sources(alexa PUBLIC media_player/BaseStreamSource.cpp)
target_sources(alexa PUBLIC media_player/ErrorTypeConversion.cpp)
target_sources(alexa PUBLIC media_player/ErrorTypeConversionTest.cpp)
target_sources(alexa PUBLIC media_player/FFmpegAttachmentInputController.cpp)
target_sources(alexa PUBLIC media_player/FFmpegAttachmentInputControllerTest.cpp)
target_sources(alexa PUBLIC media_player/FFmpegDecoder.cpp)
target_sources(alexa PUBLIC media_player/FFmpegDecoderTest.cpp)
target_sources(alexa PUBLIC media_player/FFmpegDeleter.cpp)
target_sources(alexa PUBLIC media_player/FFmpegStreamInputController.cpp)
target_sources(alexa PUBLIC media_player/FFmpegStreamInputControllerTest.cpp)
target_sources(alexa PUBLIC media_player/FFmpegUrlInputController.cpp)
target_sources(alexa PUBLIC media_player/FFmpegUrlInputControllerTest.cpp)
target_sources(alexa PUBLIC media_player/IStreamSource.cpp)
target_sources(alexa PUBLIC media_player/MediaPlayer.cpp)
target_sources(alexa PUBLIC media_player/MediaPlayerTest.cpp)
target_sources(alexa PUBLIC media_player/Normalizer.cpp)
target_sources(alexa PUBLIC media_player/NormalizerTest.cpp)
target_sources(alexa PUBLIC media_player/OffsetManager.cpp)
target_sources(alexa PUBLIC media_player/PlaybackConfiguration.cpp)
target_sources(alexa PUBLIC media_player/PlaybackContext.cpp)
target_sources(alexa PUBLIC media_player/PlaybackContextTest.cpp)
target_sources(alexa PUBLIC media_player/PooledMediaPlayerFactory.cpp)
target_sources(alexa PUBLIC metrics/DataPoint.cpp)
target_sources(alexa PUBLIC metrics/DataPointCounterBuilder.cpp)
target_sources(alexa PUBLIC metrics/DataPointDurationBuilder.cpp)
target_sources(alexa PUBLIC metrics/DataPointStringBuilder.cpp)
target_sources(alexa PUBLIC metrics/MetricEvent.cpp)
target_sources(alexa PUBLIC metrics/MetricEventBuilder.cpp)
target_sources(alexa PUBLIC multiparser/multipart.cpp)
target_sources(alexa PUBLIC network/HTTP2GetMimeHeadersResult.cpp)
target_sources(alexa PUBLIC network/HTTP2MimeRequestEncoder.cpp)
target_sources(alexa PUBLIC network/HTTP2MimeResponseDecoder.cpp)
target_sources(alexa PUBLIC network/HTTP2RequestConfig.cpp)
target_sources(alexa PUBLIC network/HTTP2SendDataResult.cpp)
target_sources(alexa PUBLIC playlist_parser/ContentDecrypter.cpp)
target_sources(alexa PUBLIC playlist_parser/ContentDecrypterTest.cpp)
target_sources(alexa PUBLIC playlist_parser/FFMpegInputBuffer.cpp)
target_sources(alexa PUBLIC playlist_parser/IterativePlaylistParser.cpp)
target_sources(alexa PUBLIC playlist_parser/IterativePlaylistParserTest.cpp)
target_sources(alexa PUBLIC playlist_parser/M3UParser.cpp)
target_sources(alexa PUBLIC playlist_parser/M3UParserTest.cpp)
target_sources(alexa PUBLIC playlist_parser/PlaylistParser.cpp)
target_sources(alexa PUBLIC playlist_parser/PlaylistParserTest.cpp)
target_sources(alexa PUBLIC playlist_parser/PlaylistUtils.cpp)
target_sources(alexa PUBLIC playlist_parser/PlaylistUtilsTest.cpp)
target_sources(alexa PUBLIC playlist_parser/UrlContentToAttachmentConverter.cpp)
target_sources(alexa PUBLIC pulse_audio/daemon/caps.c)
target_sources(alexa PUBLIC pulse_audio/daemon/cmdline.c)
target_sources(alexa PUBLIC pulse_audio/daemon/cpulimit.c)
target_sources(alexa PUBLIC pulse_audio/daemon/daemon-conf.c)
target_sources(alexa PUBLIC pulse_audio/daemon/dumpmodules.c)
target_sources(alexa PUBLIC pulse_audio/daemon/ltdl-bind-now.c)
target_sources(alexa PUBLIC pulse_audio/daemon/main.c)
target_sources(alexa PUBLIC pulse_audio/daemon/server-lookup.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/alsa-mixer.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/alsa-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/alsa-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/alsa-ucm.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/alsa-util.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/module-alsa-card.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/module-alsa-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/alsa/module-alsa-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/a2dp-codec-sbc.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/a2dp-codec-util.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/backend-native.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/backend-ofono.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/bluez5-util.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/module-bluetooth-discover.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/module-bluetooth-policy.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/module-bluez5-device.c)
target_sources(alexa PUBLIC pulse_audio/modules/bluetooth/module-bluez5-discover.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-card.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-card-profile.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-client.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-core.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-device.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-device-port.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-memstats.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-module.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-sample.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/iface-stream.c)
target_sources(alexa PUBLIC pulse_audio/modules/dbus/module-dbus-protocol.c)
target_sources(alexa PUBLIC pulse_audio/modules/echo-cancel/adrian.c)
target_sources(alexa PUBLIC pulse_audio/modules/echo-cancel/adrian-aec.c)
target_sources(alexa PUBLIC pulse_audio/modules/echo-cancel/module-echo-cancel.c)
target_sources(alexa PUBLIC pulse_audio/modules/echo-cancel/null.c)
target_sources(alexa PUBLIC pulse_audio/modules/echo-cancel/speex.c)
target_sources(alexa PUBLIC pulse_audio/modules/echo-cancel/webrtc.cc)
target_sources(alexa PUBLIC pulse_audio/modules/gconf/gconf-helper.c)
target_sources(alexa PUBLIC pulse_audio/modules/gconf/module-gconf.c)
target_sources(alexa PUBLIC pulse_audio/modules/gsettings/gsettings-helper.c)
target_sources(alexa PUBLIC pulse_audio/modules/gsettings/module-gsettings.c)
target_sources(alexa PUBLIC pulse_audio/modules/jack/module-jack-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/jack/module-jack-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/jack/module-jackdbus-detect.c)
target_sources(alexa PUBLIC pulse_audio/modules/macosx/module-bonjour-publish.c)
target_sources(alexa PUBLIC pulse_audio/modules/macosx/module-coreaudio-detect.c)
target_sources(alexa PUBLIC pulse_audio/modules/macosx/module-coreaudio-device.c)
target_sources(alexa PUBLIC pulse_audio/modules/oss/module-oss.c)
target_sources(alexa PUBLIC pulse_audio/modules/oss/oss-util.c)
target_sources(alexa PUBLIC pulse_audio/modules/raop/module-raop-discover.c)
target_sources(alexa PUBLIC pulse_audio/modules/raop/module-raop-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/raop/raop-client.c)
target_sources(alexa PUBLIC pulse_audio/modules/raop/raop-crypto.c)
target_sources(alexa PUBLIC pulse_audio/modules/raop/raop-packet-buffer.c)
target_sources(alexa PUBLIC pulse_audio/modules/raop/raop-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/raop/raop-util.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/headerlist.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/module-rtp-recv.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/module-rtp-send.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/rtp-common.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/rtp-gstreamer.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/rtp-native.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/rtsp_client.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/sap.c)
target_sources(alexa PUBLIC pulse_audio/modules/rtp/sdp.c)
target_sources(alexa PUBLIC pulse_audio/modules/x11/module-x11-bell.c)
target_sources(alexa PUBLIC pulse_audio/modules/x11/module-x11-cork-request.c)
target_sources(alexa PUBLIC pulse_audio/modules/x11/module-x11-publish.c)
target_sources(alexa PUBLIC pulse_audio/modules/x11/module-x11-xsmp.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-allow-passthrough.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-always-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-always-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-augment-properties.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-card-restore.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-cli.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-combine.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-combine-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-console-kit.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-default-device-restore.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-detect.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-device-manager.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-device-restore.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-equalizer-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-esound-compat-spawnfd.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-esound-compat-spawnpid.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-esound-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-filter-apply.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-filter-heuristics.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-hal-detect-compat.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-intended-roles.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-ladspa-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-lirc.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-loopback.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-match.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-mmkbd-evdev.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-native-protocol-fd.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-null-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-null-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-pipe-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-pipe-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-position-event-sounds.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-protocol-stub.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-remap-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-remap-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-rescue-streams.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-role-cork.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-role-ducking.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-rygel-media-server.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-sine.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-sine-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-solaris.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-stream-restore.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-suspend-on-idle.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-switch-on-connect.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-switch-on-port-available.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-systemd-login.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-tunnel.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-tunnel-sink-new.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-tunnel-source-new.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-udev-detect.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-virtual-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-virtual-source.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-virtual-surround-sink.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-volume-restore.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-waveout.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-zeroconf-discover.c)
target_sources(alexa PUBLIC pulse_audio/modules/module-zeroconf-publish.c)
target_sources(alexa PUBLIC pulse_audio/modules/reserve.c)
target_sources(alexa PUBLIC pulse_audio/modules/reserve-monitor.c)
target_sources(alexa PUBLIC pulse_audio/modules/reserve-wrap.c)
target_sources(alexa PUBLIC pulse_audio/modules/stdin-util.c)
target_sources(alexa PUBLIC pulse_audio/modules/stream-interaction.c)
target_sources(alexa PUBLIC pulse_audio/modules/udev-util.c)
target_sources(alexa PUBLIC pulse_audio/pulse/channelmap.c)
target_sources(alexa PUBLIC pulse_audio/pulse/client-conf.c)
target_sources(alexa PUBLIC pulse_audio/pulse/client-conf-x11.c)
target_sources(alexa PUBLIC pulse_audio/pulse/context.c)
target_sources(alexa PUBLIC pulse_audio/pulse/direction.c)
target_sources(alexa PUBLIC pulse_audio/pulse/error.c)
target_sources(alexa PUBLIC pulse_audio/pulse/ext-device-manager.c)
target_sources(alexa PUBLIC pulse_audio/pulse/ext-device-restore.c)
target_sources(alexa PUBLIC pulse_audio/pulse/ext-stream-restore.c)
target_sources(alexa PUBLIC pulse_audio/pulse/fork-detect.c)
target_sources(alexa PUBLIC pulse_audio/pulse/format.c)
target_sources(alexa PUBLIC pulse_audio/pulse/glib-mainloop.c)
target_sources(alexa PUBLIC pulse_audio/pulse/introspect.c)
target_sources(alexa PUBLIC pulse_audio/pulse/json.c)
target_sources(alexa PUBLIC pulse_audio/pulse/mainloop.c)
target_sources(alexa PUBLIC pulse_audio/pulse/mainloop-api.c)
target_sources(alexa PUBLIC pulse_audio/pulse/mainloop-signal.c)
target_sources(alexa PUBLIC pulse_audio/pulse/operation.c)
target_sources(alexa PUBLIC pulse_audio/pulse/proplist.c)
target_sources(alexa PUBLIC pulse_audio/pulse/rtclock.c)
target_sources(alexa PUBLIC pulse_audio/pulse/sample.c)
target_sources(alexa PUBLIC pulse_audio/pulse/scache.c)
target_sources(alexa PUBLIC pulse_audio/pulse/simple.c)
target_sources(alexa PUBLIC pulse_audio/pulse/stream.c)
target_sources(alexa PUBLIC pulse_audio/pulse/subscribe.c)
target_sources(alexa PUBLIC pulse_audio/pulse/thread-mainloop.c)
target_sources(alexa PUBLIC pulse_audio/pulse/timeval.c)
target_sources(alexa PUBLIC pulse_audio/pulse/utf8.c)
target_sources(alexa PUBLIC pulse_audio/pulse/util.c)
target_sources(alexa PUBLIC pulse_audio/pulse/volume.c)
target_sources(alexa PUBLIC pulse_audio/pulse/xmalloc.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/ffmpeg/resample2.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/filter/biquad.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/filter/crossover.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/filter/lfe-filter.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/resampler/ffmpeg.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/resampler/libsamplerate.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/resampler/peaks.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/resampler/soxr.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/resampler/speex.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/resampler/trivial.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/arpa-inet.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/asyncmsgq.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/asyncq.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/aupdate.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/auth-cookie.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/authkey.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/avahi-wrap.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/bitset.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/card.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/cli.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/cli-command.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/cli-text.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/client.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/conf-parser.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/core.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/core-error.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/core-format.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/core-rtclock.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/core-scache.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/core-subscribe.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/core-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/cpu.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/cpu-arm.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/cpu-orc.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/cpu-x86.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/database.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/database-gdbm.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/database-simple.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/database-tdb.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/dbus-shared.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/dbus-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/device-port.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/dllmain.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/dynarray.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/fdsem.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/flist.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/g711.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/hashmap.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/hook-list.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/i18n.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/idxset.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/iochannel.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/ioline.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/ipacl.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/lock-autospawn.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/log.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/ltdl-helper.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/mcalign.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/memblock.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/memblockq.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/memchunk.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/memtrap.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/message-handler.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/mime-type.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/mix.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/mix_neon.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/modargs.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/modinfo.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/module.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/msgobject.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/mutex-posix.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/mutex-win32.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/namereg.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/native-common.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/object.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/once.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/packet.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/parseaddr.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/pdispatch.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/pid.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/pipe.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/play-memblockq.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/play-memchunk.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/poll-posix.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/poll-win32.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/proplist-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/protocol-cli.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/protocol-dbus.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/protocol-esound.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/protocol-http.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/protocol-native.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/protocol-simple.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/pstream.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/pstream-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/queue.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/random.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/ratelimit.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/remap.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/remap_mmx.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/remap_neon.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/remap_sse.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/resampler)
target_sources(alexa PUBLIC pulse_audio/pulsecore/rtkit.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/rtpoll.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sample-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sconv.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sconv-s16be.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sconv-s16le.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sconv_neon.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sconv_sse.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/semaphore-osx.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/semaphore-posix.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/semaphore-win32.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/shared.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/shm.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sink.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sink-input.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sioman.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sndfile-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/socket-client.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/socket-server.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/socket-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sound-file.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/sound-file-stream.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/source.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/source-output.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/srbchannel.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/start-child.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/strbuf.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/stream-util.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/strlist.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/svolume_arm.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/svolume_c.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/svolume_mmx.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/svolume_orc.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/svolume_sse.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/tagstruct.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/thread-mq.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/thread-posix.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/thread-win32.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/time-smoother.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/tokenizer.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/usergroup.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/x11prop.c)
target_sources(alexa PUBLIC pulse_audio/pulsecore/x11wrap.c)
target_sources(alexa PUBLIC pulse_audio/tests/alsa-mixer-path-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/alsa-time-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/asyncmsgq-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/asyncq-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/atomic-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/channelmap-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/close-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/connect-stress.c)
target_sources(alexa PUBLIC pulse_audio/tests/core-util-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/cpu-mix-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/cpu-remap-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/cpu-sconv-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/cpu-volume-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/cpulimit-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/extended-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/flist-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/format-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/get-binary-name-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/gtk-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/hashmap-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/hook-list-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/interpol-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/ipacl-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/json-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/lfe-filter-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/lo-latency-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/lo-test-util.c)
target_sources(alexa PUBLIC pulse_audio/tests/lock-autospawn-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/mainloop-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/mcalign-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/memblock-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/memblockq-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/mix-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/mult-s16-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/once-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/pacat-simple.c)
target_sources(alexa PUBLIC pulse_audio/tests/parec-simple.c)
target_sources(alexa PUBLIC pulse_audio/tests/passthrough-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/proplist-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/queue-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/remix-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/resampler-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/rtpoll-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/rtstutter.c)
target_sources(alexa PUBLIC pulse_audio/tests/sig2str-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/sigbus-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/smoother-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/srbchannel-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/stripnul.c)
target_sources(alexa PUBLIC pulse_audio/tests/strlist-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/sync-playback.c)
target_sources(alexa PUBLIC pulse_audio/tests/thread-mainloop-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/thread-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/usergroup-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/utf8-test.c)
target_sources(alexa PUBLIC pulse_audio/tests/volume-test.c)
target_sources(alexa PUBLIC pulse_audio/utils/pacat.c)
target_sources(alexa PUBLIC pulse_audio/utils/pacmd.c)
target_sources(alexa PUBLIC pulse_audio/utils/pactl.c)
target_sources(alexa PUBLIC pulse_audio/utils/padsp.c)
target_sources(alexa PUBLIC pulse_audio/utils/pasuspender.c)
target_sources(alexa PUBLIC pulse_audio/utils/pax11publish.c)
target_sources(alexa PUBLIC pulse_audio/model.c)
target_sources(alexa PUBLIC registration_manager/CustomerDataHandler.cpp)
target_sources(alexa PUBLIC registration_manager/CustomerDataManager.cpp)
target_sources(alexa PUBLIC registration_manager/CustomerDataManagerTest.cpp)
target_sources(alexa PUBLIC registration_manager/RegistrationManager.cpp)
target_sources(alexa PUBLIC registration_manager/RegistrationManagerTest.cpp)
target_sources(alexa PUBLIC sbc/sbc.c)
target_sources(alexa PUBLIC sbc/sbc_primitives.c)
target_sources(alexa PUBLIC sbc/sbc_primitives_armv6.c)
target_sources(alexa PUBLIC sbc/sbc_primitives_iwmmxt.c)
target_sources(alexa PUBLIC sbc/sbc_primitives_mmx.c)
target_sources(alexa PUBLIC sbc/sbc_primitives_neon.c)
target_sources(alexa PUBLIC sbc/sbcdec.c)
target_sources(alexa PUBLIC sbc/sbcenc.c)
target_sources(alexa PUBLIC sbc/sbcinfo.c)
target_sources(alexa PUBLIC sbc/sbctester.c)
target_sources(alexa PUBLIC sdkinterfaces/EqualizerStorageInterfaceTest.cpp)
target_sources(alexa PUBLIC sdkinterfaces/StubMiscStorage.cpp)
target_sources(alexa PUBLIC settings/Storage/SQLiteDeviceSettingStorage.cpp)
target_sources(alexa PUBLIC settings/Types/LocaleWakeWordsSetting.cpp)
target_sources(alexa PUBLIC settings/Types/NetworkInfo.cpp)
target_sources(alexa PUBLIC settings/CloudControlledSettingProtocol.cpp)
target_sources(alexa PUBLIC settings/DeviceControlledSettingProtocol.cpp)
target_sources(alexa PUBLIC settings/LocaleWakeWordsSettingTest.cpp)
target_sources(alexa PUBLIC settings/NetworkInfoTest.cpp)
target_sources(alexa PUBLIC settings/SettingCallbackAdapterTest.cpp)
target_sources(alexa PUBLIC settings/SettingCallbacksTest.cpp)
target_sources(alexa PUBLIC settings/SettingConnectionObserver.cpp)
target_sources(alexa PUBLIC settings/SettingEventRequestObserver.cpp)
target_sources(alexa PUBLIC settings/SettingEventSender.cpp)
target_sources(alexa PUBLIC settings/SettingEventSenderTest.cpp)
target_sources(alexa PUBLIC settings/SettingsManagerTest.cpp)
target_sources(alexa PUBLIC settings/SettingStringConversionTest.cpp)
target_sources(alexa PUBLIC settings/SettingTest.cpp)
target_sources(alexa PUBLIC settings/SharedAVSSettingProtocol.cpp)
target_sources(alexa PUBLIC settings/SharedAVSSettingProtocolTest.cpp)
target_sources(alexa PUBLIC settings/SQLiteDeviceSettingStorageTest.cpp)
target_sources(alexa PUBLIC shared/CookBook.cpp)
target_sources(alexa PUBLIC shared/ManufactoryTest.cpp)
target_sources(alexa PUBLIC shared/ManufactoryUtilsTest.cpp)
target_sources(alexa PUBLIC shared/SharedComponent.cpp)
target_sources(alexa PUBLIC speech_enconder/OpusEncoderContext.cpp)
target_sources(alexa PUBLIC speech_enconder/SpeechEncoder.cpp)
target_sources(alexa PUBLIC speech_enconder/SpeechEncoderTest.cpp)
target_sources(alexa PUBLIC storage/sqlite3.c)
target_sources(alexa PUBLIC storage/SQLiteDatabase.cpp)
target_sources(alexa PUBLIC storage/SQLiteDatabaseTest.cpp)
target_sources(alexa PUBLIC storage/SQLiteMiscStorage.cpp)
target_sources(alexa PUBLIC storage/SQLiteMiscStorageTest.cpp)
target_sources(alexa PUBLIC storage/SQLiteStatement.cpp)
target_sources(alexa PUBLIC storage/SQLiteUtils.cpp)
target_sources(alexa PUBLIC synchronize_state_sender/PostConnectSynchronizeStateSender.cpp)
target_sources(alexa PUBLIC synchronize_state_sender/PostConnectSynchronizeStateSenderTest.cpp)
target_sources(alexa PUBLIC synchronize_state_sender/SynchronizeStateSenderFactory.cpp)
target_sources(alexa PUBLIC timing/SafeCTimeAccess.cpp)
target_sources(alexa PUBLIC util/bluetooth/SDPRecords.cpp)
target_sources(alexa PUBLIC util/HTTP2/HTTP2GetMimeHeadersResult.cpp)
target_sources(alexa PUBLIC util/HTTP2/HTTP2MimeRequestEncoder.cpp)
target_sources(alexa PUBLIC util/HTTP2/HTTP2MimeResponseDecoder.cpp)
target_sources(alexa PUBLIC util/HTTP2/HTTP2SendDataResult.cpp)
target_sources(alexa PUBLIC util/sds/BufferLayout.cpp)
target_sources(alexa PUBLIC util/sds/Reader.cpp)
target_sources(alexa PUBLIC util/sds/Writer.cpp)
target_sources(alexa PUBLIC util/stream/Streambuf.cpp)
target_sources(alexa PUBLIC util/stream/StreamFunctions.cpp)
target_sources(alexa PUBLIC util/BluetoothEventBus.cpp)
target_sources(alexa PUBLIC util/BufferLayout.cpp)
target_sources(alexa PUBLIC util/DeviceInfo.cpp)
target_sources(alexa PUBLIC util/Executor.cpp)
target_sources(alexa PUBLIC util/FileUtils.cpp)
target_sources(alexa PUBLIC util/FormattedAudioStreamAdapter.cpp)
target_sources(alexa PUBLIC util/MacAddressString.cpp)
target_sources(alexa PUBLIC util/Metrics.cpp)
target_sources(alexa PUBLIC util/MultiTimer.cpp)
target_sources(alexa PUBLIC util/Reader.cpp)
target_sources(alexa PUBLIC util/RequiresShutdown.cpp)
target_sources(alexa PUBLIC util/RetryTimer.cpp)
target_sources(alexa PUBLIC util/SafeCTimeAccess.cpp)
target_sources(alexa PUBLIC util/SharedDataStream.cpp)
target_sources(alexa PUBLIC util/Stopwatch.cpp)
target_sources(alexa PUBLIC util/StringUtils.cpp)
target_sources(alexa PUBLIC util/SystemClockMonitor.cpp)
target_sources(alexa PUBLIC util/TaskThread.cpp)
target_sources(alexa PUBLIC util/ThreadPool.cpp)
target_sources(alexa PUBLIC util/TimePoint.cpp)
target_sources(alexa PUBLIC util/Timer.cpp)
target_sources(alexa PUBLIC util/TimeUtils.cpp)
target_sources(alexa PUBLIC util/UUIDGeneration.cpp)
target_sources(alexa PUBLIC util/WaitEvent.cpp)
target_sources(alexa PUBLIC util/WorkerThread.cpp)
target_sources(alexa PUBLIC util/Writer.cpp)
target_sources(alexa PUBLIC webvtt/webvtt/alloc.c)
target_sources(alexa PUBLIC webvtt/webvtt/cue.c)
target_sources(alexa PUBLIC webvtt/webvtt/cuetext.c)
target_sources(alexa PUBLIC webvtt/webvtt/error.c)
target_sources(alexa PUBLIC webvtt/webvtt/lexer.c)
target_sources(alexa PUBLIC webvtt/webvtt/node.c)
target_sources(alexa PUBLIC webvtt/webvtt/parser.c)
target_sources(alexa PUBLIC webvtt/webvtt/string.c)
target_sources(alexa PUBLIC webvtt/webvttxx/abstract_parser.cpp)
target_sources(alexa PUBLIC webvtt/webvttxx/file_parser.cpp)
target_sources(alexa PUBLIC webvtt/parsevtt_main.c)
target_sources(alexa PUBLIC xcb/check_all.c)
target_sources(alexa PUBLIC xcb/check_public.c)
target_sources(alexa PUBLIC xcb/xcb_auth.c)
target_sources(alexa PUBLIC xcb/xcb_conn.c)
target_sources(alexa PUBLIC xcb/xcb_ext.c)
target_sources(alexa PUBLIC xcb/xcb_in.c)
target_sources(alexa PUBLIC xcb/xcb_list.c)
target_sources(alexa PUBLIC xcb/xcb_out.c)
target_sources(alexa PUBLIC xcb/xcb_util.c)
target_sources(alexa PUBLIC xcb/xcb_xid.c)
find_library(log-lib log)
find_library(audio-lib aaudio)
find_library(OpenSLES-lib OpenSLES)
find_library(definitions-stdc++ stdc++)
target_link_libraries(alexa ${log-lib})
target_link_libraries(alexa ${android-lib})
target_link_libraries(alexa ${aaudio-lib})
target_link_libraries(alexa ${definitions-stdc++})
#set(CMAKE_CXX_FLAGS '-O1 -O0)