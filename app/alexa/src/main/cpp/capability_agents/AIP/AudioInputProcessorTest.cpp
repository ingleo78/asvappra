#include <cstring>
#include <climits>
#include <numeric>
#include <sstream>
#include <vector>
#include <gtest/gtest.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/AudioInputProcessorObserverInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/ContextRequestToken.h>
#include <sdkinterfaces/MockAVSConnectionManager.h>
#include <sdkinterfaces/MockDirectiveSequencer.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockLocaleAssetsManager.h>
#include <sdkinterfaces/MockSystemSoundPlayer.h>
#include <sdkinterfaces/MockUserInactivityMonitor.h>
#include <sdkinterfaces/MockPowerResourceManager.h>
#include <json/JSONUtils.h>
#include <memory/Memory.h>
#include <metrics/MockMetricRecorder.h>
#include <uuid_generation/UUIDGeneration.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Settings/MockDeviceSettingStorage.h>
#include <settings/Settings/MockSettingEventSender.h>
#include <settings/SettingEventSender.h>
#include <settings/Settings/MockSetting.h>
#include "AudioInputProcessor.h"
#include "MockObserver.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace aip {
            namespace test {
                using namespace json;
                using namespace storage;
                using namespace uuidGeneration;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                using namespace settings::test;
                using namespace storage::test;
                using namespace metrics::test;
                using namespace aip::test;
                using namespace rapidjson;
                using namespace testing;
                using aip::test::AudioInputProcessorObserverInterface;
                using PowerResourceLevel = PowerResourceManagerInterface::PowerResourceLevel;
                static const string CHANNEL_NAME = FocusManagerInterface::DIALOG_CHANNEL_NAME;
                static const string NAMESPACE = "SpeechRecognizer";
                static const NamespaceAndName STOP_CAPTURE{NAMESPACE, "StopCapture"};
                static const NamespaceAndName EXPECT_SPEECH{NAMESPACE, "ExpectSpeech"};
                static const NamespaceAndName SET_END_OF_SPEECH_OFFSET{NAMESPACE, "SetEndOfSpeechOffset"};
                static const NamespaceAndName SET_WAKE_WORD_CONFIRMATION{NAMESPACE, "SetWakeWordConfirmation"};
                static const NamespaceAndName SET_SPEECH_CONFIRMATION{NAMESPACE, "SetSpeechConfirmation"};
                static const NamespaceAndName SET_WAKE_WORDS{NAMESPACE, "SetWakeWords"};
                static const NamespaceAndName DIRECTIVES[] = {STOP_CAPTURE, EXPECT_SPEECH, SET_END_OF_SPEECH_OFFSET, SET_WAKE_WORD_CONFIRMATION,
                                                              SET_SPEECH_CONFIRMATION, SET_WAKE_WORDS};
                static const NamespaceAndName RECOGNIZER_STATE{NAMESPACE, "RecognizerState"};
                static const unsigned int SAMPLE_RATE_HZ = 16000;
                using Sample = uint16_t;
                static const size_t SDS_WORDSIZE = sizeof(Sample);
                static const unsigned int SAMPLE_SIZE_IN_BITS = SDS_WORDSIZE * CHAR_BIT;
                static const unsigned int NUM_CHANNELS = 1;
                static const milliseconds PREROLL_MS = milliseconds(500);
                static const size_t PREROLL_WORDS = (SAMPLE_RATE_HZ / 1000) * PREROLL_MS.count();
                static const size_t WAKEWORD_WORDS = PREROLL_WORDS;
                static const size_t SDS_WORDS = PREROLL_WORDS + WAKEWORD_WORDS + 1000;
                static const size_t PATTERN_WORDS = SDS_WORDS / 2;
                static const size_t SDS_MAXREADERS = 3;
                static const bool ALWAYS_READABLE = true;
                static const bool CAN_OVERRIDE = true;
                static const bool CAN_BE_OVERRIDDEN = true;
                static const string MESSAGE_CONTEXT_KEY = "context";
                static const string MESSAGE_EVENT_KEY = "event";
                static const string MESSAGE_DIRECTIVE_KEY = "directive";
                static const string MESSAGE_HEADER_KEY = "header";
                static const string MESSAGE_PAYLOAD_KEY = "payload";
                static const string MESSAGE_NAMESPACE_KEY = "namespace";
                static const string MESSAGE_NAME_KEY = "name";
                static const string MESSAGE_MESSAGE_ID_KEY = "messageId";
                static const string MESSAGE_DIALOG_REQUEST_ID_KEY = "dialogRequestId";
                static const string RECOGNIZE_EVENT_NAME = "Recognize";
                static const string ASR_PROFILE_KEY = "profile";
                static const string AUDIO_FORMAT_KEY = "format";
                static const unordered_set<string> AUDIO_FORMAT_VALUES = {"AUDIO_L16_RATE_16000_CHANNELS_1", "OPUS"};
                static const string RECOGNIZE_INITIATOR_KEY = "initiator";
                static const string INITIATOR_TYPE_KEY = "type";
                static const string INITIATOR_PAYLOAD_KEY = "payload";
                static const string WAKE_WORD_INDICES_KEY = "wakeWordIndices";
                static const string START_INDEX_KEY = "startIndexInSamples";
                static const string END_INDEX_KEY = "endIndexInSamples";
                static const string EXPECT_SPEECH_INITIATOR = R"({"opaque":"expectSpeechInitiator"})";
                static const string EXPECT_SPEECH_TIMEOUT_KEY = "timeoutInMilliseconds";
                static const int64_t EXPECT_SPEECH_TIMEOUT_IN_MILLISECONDS = 100;
                static const string EXPECT_SPEECH_INITIATOR_KEY = "initiator";
                static const string EXPECT_SPEECH_TIMED_OUT_EVENT_NAME = "ExpectSpeechTimedOut";
                static const string KEYWORD_TEXT = "ALEXA";
                static const bool WITH_DIALOG_REQUEST_ID = true;
                static const bool VERIFY_TIMEOUT = true;
                static const seconds TEST_TIMEOUT(10);
                static const string ESP_EVENT_NAME = "ReportEchoSpatialPerceptionData";
                static const string ESP_VOICE_ENERGY_KEY = "voiceEnergy";
                static const string ESP_AMBIENT_ENERGY_KEY = "ambientEnergy";
                static const string AUDIO_ATTACHMENT_FIELD_NAME = "audio";
                static const string KWD_METADATA_FIELD_NAME = "wakewordEngineMetadata";
                static const string WAKEWORD_FIELD_NAME = "wakeWord";
                static const string END_OF_SPEECH_OFFSET_FIELD_NAME = "endOfSpeechOffsetInMilliseconds";
                static const int64_t END_OF_SPEECH_OFFSET_IN_MILLISECONDS = 1526;
                static const string START_OF_SPEECH_TIMESTAMP_FIELD_NAME = "startOfSpeechTimestamp";
                static const auto START_OF_SPEECH_TIMESTAMP = steady_clock::now();
                static const auto START_OF_SPEECH_TIMESTAMP_STR = to_string(START_OF_SPEECH_TIMESTAMP.time_since_epoch().count());
                static const size_t MESSAGE_ATTACHMENT_KWD_METADATA_INDEX = 0;
                static const string KWD_METADATA_EXAMPLE = "Wakeword engine metadata example";
                static const string WAKE_WORD_CONFIRMATION_PAYLOAD_KEY = "wakeWordConfirmation";
                static const string SPEECH_CONFIRMATION_PAYLOAD_KEY = "speechConfirmation";
                static const string WAKEWORDS_PAYLOAD_KEY = "wakeWords";
                static const set<string> SUPPORTED_WAKE_WORDS = {"ALEXA", "ECHO"};
                static const set<string> SUPPORTED_LOCALES = {"en-CA", "en-US"};
                static const string DEFAULT_LOCALE = "en-CA";
                static const string CAPABILITY_INTERFACE_CONFIGURATIONS_KEY = "configurations";
                static const ContextRequestToken CONTEXT_REQUEST_TOKEN{1};
                static const string COMPONENT_NAME("AudioInputProcessor");
                static Document parseJson(const string& json) {
                    Document document;
                    document.Parse(json.data());
                    EXPECT_FALSE(document.HasParseError()) << "rapidjson detected a parsing error at offset:" +
                        to_string(document.GetErrorOffset()) + ", error message: " + GetParseError_En(document.GetParseError());
                    return document;
                }
                static string getJsonString(const rapidjson::Value& container, const string& key) {
                    auto member = container.FindMember(key.data());
                    EXPECT_TRUE(member->value.IsString());
                    if (!member->value.IsString()) return "";
                    return member->value.GetString();
                }
                static int64_t getJsonInt64(const rapidjson::Value& container, const string& key) {
                    auto member = container.FindMember(key.data());
                    EXPECT_TRUE(member->value.IsInt64());
                    if (!member->value.IsInt64()) return 0;
                    return member->value.GetInt64();
                }
                class RecognizeEvent {
                public:
                    RecognizeEvent(AudioProvider audioProvider, Initiator initiator, AudioInputStream::Index begin = AudioInputProcessor::INVALID_INDEX,
                                   AudioInputStream::Index keywordEnd = AudioInputProcessor::INVALID_INDEX, string keyword = "",
                                   shared_ptr<string> avsInitiator = nullptr, const shared_ptr<vector<char>> KWDMetadata = nullptr);
                    future<bool> send(shared_ptr<AudioInputProcessor> audioInputProcessor);
                    void verifyJsonState(const NamespaceAndName&, const string& jsonState, const StateRefreshPolicy&,
                                         const unsigned int);
                    void verifyEspMessage(shared_ptr<MessageRequest> request, const string& dialogRequestId);
                    void verifyMetadata(const shared_ptr<MessageRequest> request, const shared_ptr<vector<char>> KWDMetadata);
                    void verifyMessage(shared_ptr<MessageRequest> request, const vector<Sample>& pattern, const string& dialogRequestId);
                    shared_ptr<AttachmentReader> getReader();
                private:
                    AudioProvider m_audioProvider;
                    Initiator m_initiator;
                    AudioInputStream::Index m_begin;
                    AudioInputStream::Index m_keywordEnd;
                    string m_keyword;
                    shared_ptr<string> m_avsInitiator;
                    shared_ptr<MessageRequest::NamedReader> m_reader;
                    shared_ptr<vector<char>> m_KWDMetadata;
                };
                RecognizeEvent::RecognizeEvent(AudioProvider audioProvider, Initiator initiator, AudioInputStream::Index begin,
                                               AudioInputStream::Index keywordEnd, string keyword, shared_ptr<string> avsInitiator,
                                               const shared_ptr<vector<char>> KWDMetadata) : m_audioProvider{audioProvider},
                                               m_initiator{initiator}, m_begin{begin}, m_keywordEnd{keywordEnd}, m_keyword{keyword},
                                               m_avsInitiator{avsInitiator}, m_KWDMetadata{KWDMetadata} {}
                future<bool> RecognizeEvent::send(shared_ptr<AudioInputProcessor> audioInputProcessor) {
                    auto result = audioInputProcessor->recognize(m_audioProvider, m_initiator, START_OF_SPEECH_TIMESTAMP,
                                                                 m_begin, m_keywordEnd, m_keyword, m_KWDMetadata);
                    EXPECT_TRUE(result.valid());
                    return result;
                }
                void RecognizeEvent::verifyEspMessage(shared_ptr<MessageRequest> request, const string& dialogRequestId) {
                    Document document;
                    document.Parse(request->getJsonContent().c_str());
                    EXPECT_FALSE(document.HasParseError()) << "rapidjson detected a parsing error at offset:" +
                        to_string(document.GetErrorOffset()) + ", error message: " + GetParseError_En(document.GetParseError());
                    auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                    EXPECT_NE(event, document.MemberEnd());
                    auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                    EXPECT_NE(header, event->value.MemberEnd());
                    auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY.data());
                    EXPECT_NE(payload, event->value.MemberEnd());
                    rapidjson::Value value{header->value.GetString(), strlen(header->value.GetString())};
                    EXPECT_EQ(getJsonString(value, MESSAGE_NAMESPACE_KEY.data()), NAMESPACE);
                    EXPECT_EQ(getJsonString(value, MESSAGE_NAME_KEY.data()), ESP_EVENT_NAME);
                    EXPECT_NE(getJsonString(value, MESSAGE_MESSAGE_ID_KEY.data()), "");
                    EXPECT_EQ(getJsonString(value, MESSAGE_DIALOG_REQUEST_ID_KEY.data()), dialogRequestId);
                }
                void RecognizeEvent::verifyMetadata(const shared_ptr<MessageRequest> request, const shared_ptr<vector<char>> KWDMetadata) {
                    if (!KWDMetadata) { EXPECT_EQ(request->attachmentReadersCount(), 1); }
                    else {
                        char buffer[50];
                        auto readStatus = AttachmentReader::ReadStatus::OK;
                        EXPECT_EQ(request->attachmentReadersCount(), 2);
                        EXPECT_NE(request->getAttachmentReader(MESSAGE_ATTACHMENT_KWD_METADATA_INDEX), nullptr);
                        auto bytesRead = request->getAttachmentReader(MESSAGE_ATTACHMENT_KWD_METADATA_INDEX)->reader->read(buffer,
                                                                      KWD_METADATA_EXAMPLE.length(), &readStatus);
                        EXPECT_EQ(bytesRead, KWD_METADATA_EXAMPLE.length());
                        EXPECT_EQ(memcmp(buffer, KWD_METADATA_EXAMPLE.data(), KWD_METADATA_EXAMPLE.length()), 0);
                    }
                }
                void RecognizeEvent::verifyMessage(shared_ptr<MessageRequest> request, const vector<Sample>& pattern, const string& dialogRequestId) {
                    rapidjson::Document document;
                    document.Parse(request->getJsonContent().c_str());
                    EXPECT_FALSE(document.HasParseError())
                        << "rapidjson detected a parsing error at offset:" + std::to_string(document.GetErrorOffset()) +
                               ", error message: " + GetParseError_En(document.GetParseError());
                    auto context = document.FindMember(MESSAGE_CONTEXT_KEY.data());
                    EXPECT_NE(context, document.MemberEnd());
                    auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                    EXPECT_NE(event, document.MemberEnd());
                    auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                    EXPECT_NE(header, event->value.MemberEnd());
                    auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY.data());
                    rapidjson::Value value{header->value.GetString(), strlen(value.GetString())};
                    EXPECT_NE(payload, event->value.MemberEnd());
                    EXPECT_EQ(getJsonString(value, MESSAGE_NAMESPACE_KEY.data()), NAMESPACE);
                    EXPECT_EQ(getJsonString(value, MESSAGE_NAME_KEY.data()), RECOGNIZE_EVENT_NAME);
                    EXPECT_NE(getJsonString(value, MESSAGE_MESSAGE_ID_KEY.data()), "");
                    EXPECT_EQ(getJsonString(value, MESSAGE_DIALOG_REQUEST_ID_KEY.data()), dialogRequestId);
                    std::ostringstream profile;
                    profile << m_audioProvider.profile;
                    std::ostringstream encodingFormat;
                    encodingFormat << m_audioProvider.format.encoding;
                    rapidjson::Value _value{payload->value.GetString(), strlen(payload->value.GetString())};
                    EXPECT_EQ(getJsonString(_value, ASR_PROFILE_KEY.data()), profile.str());
                    EXPECT_EQ(getJsonString(_value, START_OF_SPEECH_TIMESTAMP_FIELD_NAME.data()), START_OF_SPEECH_TIMESTAMP_STR);
                    EXPECT_FALSE(AUDIO_FORMAT_VALUES.find(getJsonString(_value, AUDIO_FORMAT_KEY.data())) == AUDIO_FORMAT_VALUES.end());
                    auto initiator = payload->value.FindMember(RECOGNIZE_INITIATOR_KEY.data());
                    EXPECT_NE(initiator, payload->value.MemberEnd());
                    if (m_avsInitiator) {
                        std::string initiatorString;
                        EXPECT_TRUE(jsonUtils::convertToValue(initiator->value, &initiatorString));
                        EXPECT_EQ(initiatorString, *m_avsInitiator);
                    } else {
                        rapidjson::Value __value{initiator->value.GetString(), strlen(initiator->value.GetString())};
                        //EXPECT_EQ(getJsonString(__value, INITIATOR_TYPE_KEY.data()), initiatorToString(m_initiator));
                        auto initiatorPayload = initiator->value.FindMember(INITIATOR_PAYLOAD_KEY.data());
                        EXPECT_NE(initiatorPayload, initiator->value.MemberEnd());
                        if (m_initiator == Initiator::WAKEWORD) {
                            if (m_begin != AudioInputProcessor::INVALID_INDEX && m_keywordEnd != AudioInputProcessor::INVALID_INDEX) {
                                auto wakeWordIndices = initiatorPayload->value.FindMember(WAKE_WORD_INDICES_KEY.data());
                                EXPECT_NE(wakeWordIndices, initiatorPayload->value.MemberEnd());
                                if (wakeWordIndices != initiatorPayload->value.MemberEnd()) {
                                    rapidjson::Value indicesValue{wakeWordIndices->value.GetString(), strlen(wakeWordIndices->value.GetString())};
                                    EXPECT_EQ(getJsonInt64(indicesValue, START_INDEX_KEY.data()), static_cast<int64_t>(m_begin));
                                    EXPECT_EQ(getJsonInt64(indicesValue, END_INDEX_KEY.data()), static_cast<int64_t>(m_keywordEnd));
                                }
                            }
                            rapidjson::Value payloadValue{initiatorPayload->value.GetString(), strlen(initiatorPayload->value.GetString())};
                            EXPECT_EQ(getJsonString(payloadValue, WAKEWORD_FIELD_NAME.data()), KEYWORD_TEXT);
                        }
                    }
                    m_reader = request->getAttachmentReader(request->attachmentReadersCount() - 1);
                    EXPECT_NE(m_reader, nullptr);
                    EXPECT_EQ(m_reader->name, AUDIO_ATTACHMENT_FIELD_NAME);
                    vector<Sample> samples(PATTERN_WORDS);
                    size_t samplesRead = 0;
                    auto t0 = steady_clock::now();
                    do {
                        AttachmentReader::ReadStatus status;
                        auto bytesRead = m_reader->reader->read(samples.data() + samplesRead, (samples.size() - samplesRead) * SDS_WORDSIZE, &status);
                        if (AttachmentReader::ReadStatus::OK_WOULDBLOCK == status) {
                            this_thread::yield();
                            continue;
                        }
                        EXPECT_EQ(status, AttachmentReader::ReadStatus::OK);
                        EXPECT_GT(bytesRead, 0u);
                        EXPECT_EQ(bytesRead % 2, 0u);
                        samplesRead += bytesRead / 2;
                    } while (samplesRead < samples.size() && t0 - steady_clock::now() < TEST_TIMEOUT);
                    EXPECT_EQ(samplesRead, samples.size());
                    EXPECT_EQ(samples, pattern);
                }
                shared_ptr<AttachmentReader> RecognizeEvent::getReader() {
                    return m_reader->reader;
                }
                class TestDialogUXStateObserver : public DialogUXStateObserverInterface {
                public:
                    TestDialogUXStateObserver(shared_ptr<DialogUXStateAggregator> aggregator);
                    void onDialogUXStateChanged(DialogUXState newState) override;
                private:
                    shared_ptr<DialogUXStateAggregator> m_aggregator;
                };
                TestDialogUXStateObserver::TestDialogUXStateObserver(shared_ptr<DialogUXStateAggregator> aggregator) : m_aggregator(aggregator) {}
                void TestDialogUXStateObserver::onDialogUXStateChanged(DialogUXState newState) {
                    if (DialogUXState::THINKING == newState) m_aggregator->receive("", "");
                }
                class AudioInputProcessorTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    enum class RecognizeStopPoint {
                        AFTER_RECOGNIZE,
                        AFTER_CONTEXT,
                        AFTER_FOCUS,
                        AFTER_SEND,
                        NONE
                    };
                    enum class StopCaptureDirectiveSchedule {
                        BEFORE_EVENT_STREAM_CLOSE,
                        AFTER_EVENT_STREAM_CLOSE,
                        NONE
                    };
                    bool testRecognizeFails(AudioProvider audioProvider, Initiator initiator, AudioInputStream::Index begin = AudioInputProcessor::INVALID_INDEX,
                                            AudioInputStream::Index keywordEnd = AudioInputProcessor::INVALID_INDEX, string keyword = "");
                    bool testRecognizeSucceeds(AudioProvider audioProvider, Initiator initiator, AudioInputStream::Index begin = AudioInputProcessor::INVALID_INDEX,
                                               AudioInputStream::Index keywordEnd = AudioInputProcessor::INVALID_INDEX, string keyword = "",
                                               RecognizeStopPoint stopPoint = RecognizeStopPoint::NONE, shared_ptr<string> avsInitiator = nullptr,
                                               const shared_ptr<vector<char>> KWDMetadata = nullptr);
                    bool testStopCaptureSucceeds();
                    bool testContextFailure(ContextRequestError error);
                    bool testStopCaptureDirectiveSucceeds(bool withDialogRequestId);
                    bool testStopCaptureDirectiveFails(bool withDialogRequestId);
                    bool testExpectSpeechSucceeds(bool withDialogRequestId);
                    bool testExpectSpeechWaits(bool withDialogRequestId, bool verifyTimeout);
                    bool testExpectSpeechFails(bool withDialogRequestId);
                    bool testRecognizeWithExpectSpeechInitiator(bool withInitiator);
                    static shared_ptr<AVSDirective> createAVSDirective(const NamespaceAndName& directive, bool withDialogRequestId,
                                                                       bool withInitiator = true);
                    static shared_ptr<AVSDirective> createAVSDirective(const NamespaceAndName& directive, bool withDialogRequestId, bool withInitiator,
                                                                       Document& document, rapidjson::Value& payloadJson);
                    static void verifyExpectSpeechTimedOut(shared_ptr<MessageRequest> request);
                    void removeDefaultAudioProvider();
                    void makeDefaultAudioProviderNotAlwaysReadable();
                    bool testFocusChange(FocusState state, MixingBehavior behavior);
                    void testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status eventStreamFinishedStatus,
                                                             StopCaptureDirectiveSchedule stopCaptureSchedule,
                                                             AudioInputProcessorObserverInterface::State expectedAIPFinalState,
                                                             bool expectFocusReleased);
                    shared_ptr<MetricRecorderInterface> m_metricRecorder;
                    shared_ptr<MockDirectiveSequencer> m_mockDirectiveSequencer;
                    shared_ptr<MockMessageSender> m_mockMessageSender;
                    shared_ptr<MockSetting<SpeechConfirmationSettingType>> m_mockSpeechConfirmation;
                    shared_ptr<MockSetting<WakeWordConfirmationSettingType>> m_mockWakeWordConfirmation;
                    shared_ptr<MockSetting<WakeWords>> m_mockWakeWordSetting;
                    shared_ptr<MockContextManager> m_mockContextManager;
                    shared_ptr<MockFocusManager> m_mockFocusManager;
                    shared_ptr<DialogUXStateAggregator> m_dialogUXStateAggregator;
                    shared_ptr<TestDialogUXStateObserver> m_dialogUXStateObserver;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
                    shared_ptr<MockUserInactivityMonitor> m_mockUserInactivityMonitor;
                    unique_ptr<AudioInputStream::Writer> m_writer;
                    unique_ptr<AudioProvider> m_audioProvider;
                    shared_ptr<AudioInputProcessor> m_audioInputProcessor;
                    shared_ptr<MockObserver> m_mockObserver;
                    shared_ptr<MockSystemSoundPlayer> m_mockSystemSoundPlayer;
                    shared_ptr<MockLocaleAssetsManager> m_mockAssetsManager;
                    shared_ptr<RecognizeEvent> m_recognizeEvent;
                    shared_ptr<MockPowerResourceManager> m_mockPowerResourceManager;
                    vector<Sample> m_pattern;
                    string m_dialogRequestId;
                };
                void AudioInputProcessorTest::SetUp() {
                    m_metricRecorder = make_shared<NiceMock<MockMetricRecorder>>();
                    m_mockDirectiveSequencer = make_shared<MockDirectiveSequencer>();
                    m_mockMessageSender = make_shared<MockMessageSender>();
                    //m_mockContextManager = make_shared<MockContextManager>();
                    //m_mockFocusManager = make_shared<MockFocusManager>();
                    m_dialogUXStateAggregator = make_shared<DialogUXStateAggregator>(m_metricRecorder);
                    m_dialogUXStateObserver = make_shared<TestDialogUXStateObserver>(m_dialogUXStateAggregator);
                    m_mockSystemSoundPlayer = make_shared<NiceMock<MockSystemSoundPlayer>>();
                    m_dialogUXStateAggregator->addObserver(m_dialogUXStateObserver);
                    //m_mockAssetsManager = make_shared<NiceMock<MockLocaleAssetsManager>>();
                    m_mockSpeechConfirmation = make_shared<MockSetting<SpeechConfirmationSettingType>>(getSpeechConfirmationDefault());
                    m_mockWakeWordConfirmation = make_shared<MockSetting<WakeWordConfirmationSettingType>>(getWakeWordConfirmationDefault());
                    m_mockWakeWordSetting = make_shared<MockSetting<WakeWords>>(SUPPORTED_WAKE_WORDS);
                    m_mockExceptionEncounteredSender = make_shared<MockExceptionEncounteredSender>();
                    m_mockUserInactivityMonitor = make_shared<MockUserInactivityMonitor>();
                    m_mockPowerResourceManager = make_shared<MockPowerResourceManager>();
                    size_t bufferSize = AudioInputStream::calculateBufferSize(SDS_WORDS, SDS_WORDSIZE, SDS_MAXREADERS);
                    auto buffer = make_shared<AudioInputStream::Buffer>(bufferSize);
                    auto stream = AudioInputStream::create(buffer, SDS_WORDSIZE, SDS_MAXREADERS);
                    ASSERT_NE(stream, nullptr);
                    m_writer = stream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
                    ASSERT_NE(m_writer, nullptr);
                    AudioFormat format = {AudioFormat::Encoding::LPCM,AudioFormat::Endianness::LITTLE, SAMPLE_RATE_HZ,
                                          SAMPLE_SIZE_IN_BITS, NUM_CHANNELS,false,AudioFormat::Layout::NON_INTERLEAVED};
                    m_audioProvider = make_unique<AudioProvider>(move(stream), format, ASRProfile::NEAR_FIELD, ALWAYS_READABLE,
                                                                 CAN_OVERRIDE, CAN_BE_OVERRIDDEN);
                    ON_CALL(*m_mockAssetsManager, getSupportedWakeWords(_))
                        .WillByDefault(InvokeWithoutArgs([]()->LocaleAssetsManagerInterface::WakeWordsSets {
                            return {SUPPORTED_WAKE_WORDS};
                        }));
                    ON_CALL(*m_mockAssetsManager, getDefaultSupportedWakeWords())
                        .WillByDefault(InvokeWithoutArgs([]()->LocaleAssetsManagerInterface::WakeWordsSets {
                            return {SUPPORTED_WAKE_WORDS};
                        }));
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,m_mockExceptionEncounteredSender,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation,m_mockWakeWordSetting,
                                                            nullptr,*m_audioProvider,
                                                      m_mockPowerResourceManager,m_metricRecorder);
                    ASSERT_NE(m_audioInputProcessor, nullptr);
                    m_audioInputProcessor->addObserver(m_dialogUXStateAggregator);
                    m_mockObserver = make_shared<StrictMock<MockObserver>>();
                    ASSERT_NE(m_mockObserver, nullptr);
                    m_audioInputProcessor->addObserver(m_mockObserver);
                    m_pattern.resize(PATTERN_WORDS);
                    iota(m_pattern.begin(), m_pattern.end(), 0);
                }
                void AudioInputProcessorTest::TearDown() {
                    if (m_audioInputProcessor) {
                        m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                        //EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _)).Times(AtLeast(0));
                        EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE)).Times(AtLeast(0));
                        m_audioInputProcessor->resetState().wait();
                    }
                    m_dialogUXStateAggregator->removeObserver(m_dialogUXStateObserver);
                }
                bool AudioInputProcessorTest::testRecognizeFails(AudioProvider audioProvider, Initiator initiator, AudioInputStream::Index begin,
                                                                 AudioInputStream::Index keywordEnd, string keyword) {
                    RecognizeEvent recognize(audioProvider, initiator, begin, keywordEnd, keyword);
                    return !recognize.send(m_audioInputProcessor).get();
                }
                bool AudioInputProcessorTest::testRecognizeSucceeds(AudioProvider audioProvider, Initiator initiator, AudioInputStream::Index begin,
                                                                    AudioInputStream::Index keywordEnd, string keyword, RecognizeStopPoint stopPoint,
                                                                    shared_ptr<string> avsInitiator, const shared_ptr<vector<char>> KWDMetadata) {
                    mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    bool bargeIn = m_recognizeEvent != nullptr;
                    if (begin != AudioInputProcessor::INVALID_INDEX) {
                        EXPECT_EQ(m_writer->write(m_pattern.data(), m_pattern.size()), static_cast<ssize_t>(m_pattern.size()));
                    }
                    Document contextDocument(kObjectType);
                    rapidjson::Value contextArray(kArrayType);
                    rapidjson::Value message_context_key{MESSAGE_CONTEXT_KEY.data(), MESSAGE_CONTEXT_KEY.length()};
                    contextDocument.AddMember(message_context_key, contextArray, contextDocument.GetAllocator());
                    StringBuffer contextBuffer;
                    rapidjson::Writer<StringBuffer> contextWriter(contextBuffer);
                    contextDocument.Accept(contextWriter);
                    string contextJson = contextBuffer.GetString();
                    m_recognizeEvent = make_shared<RecognizeEvent>(audioProvider, initiator, begin, keywordEnd, keyword, avsInitiator, KWDMetadata);
                    if (keyword.empty()) {
                        EXPECT_CALL(*m_mockContextManager, getContextWithoutReportableStateProperties(_, _, _))
                            .WillOnce(InvokeWithoutArgs([this, contextJson, stopPoint] {
                                m_audioInputProcessor->onContextAvailable(contextJson);
                                if (RecognizeStopPoint::AFTER_CONTEXT == stopPoint) {
                                    EXPECT_TRUE(m_audioInputProcessor->stopCapture().valid());
                                    m_dialogUXStateAggregator->onRequestProcessingStarted();
                                }
                                return CONTEXT_REQUEST_TOKEN;
                            }));
                    } else {
                        InSequence dummy;
                        EXPECT_CALL(*m_mockContextManager, getContextWithoutReportableStateProperties(_, _, _))
                            .WillOnce(InvokeWithoutArgs([this, contextJson, stopPoint] {
                                m_audioInputProcessor->onContextAvailable(contextJson);
                                if (RecognizeStopPoint::AFTER_CONTEXT == stopPoint) {
                                    EXPECT_TRUE(m_audioInputProcessor->stopCapture().valid());
                                    m_dialogUXStateAggregator->onRequestProcessingStarted();
                                }
                                return CONTEXT_REQUEST_TOKEN;
                            }));
                    }
                    if (!bargeIn) {
                        EXPECT_CALL(*m_mockUserInactivityMonitor, onUserActive()).Times(2);
                        EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::RECOGNIZING));
                        /*EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _)).WillOnce(InvokeWithoutArgs([this, stopPoint] {
                            m_audioInputProcessor->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                            if (RecognizeStopPoint::AFTER_FOCUS == stopPoint) {
                                EXPECT_TRUE(m_audioInputProcessor->stopCapture().valid());
                                m_dialogUXStateAggregator->onRequestProcessingStarted();
                            }
                            return true;
                        }));*/
                        EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                            .Times(AtLeast(1));
                    }
                    m_mockDirectiveSequencer->setDialogRequestId("dialogRequestId");
                    {
                        InSequence dummy;
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                            .WillOnce(DoAll(Invoke([this, KWDMetadata](shared_ptr<MessageRequest> request) {
                                    m_recognizeEvent->verifyMetadata(request, KWDMetadata);
                                    m_recognizeEvent->verifyMessage(request, m_pattern, m_mockDirectiveSequencer->getDialogRequestId());
                                }),
                                InvokeWithoutArgs([&] {
                                    if (RecognizeStopPoint::AFTER_SEND == stopPoint) {
                                        EXPECT_TRUE(m_audioInputProcessor->stopCapture().valid());
                                        m_dialogUXStateAggregator->onRequestProcessingStarted();
                                    } else if (RecognizeStopPoint::NONE == stopPoint) {
                                        lock_guard<std::mutex> lock(mutex);
                                        done = true;
                                        conditionVariable.notify_one();
                                    }
                                })
                            ));
                    }
                    if (stopPoint != RecognizeStopPoint::NONE) {
                        EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::BUSY));
                        //EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _));
                        EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE))
                            .WillOnce(InvokeWithoutArgs([&] {
                                lock_guard<std::mutex> lock(mutex);
                                done = true;
                                conditionVariable.notify_one();
                            }));
                        EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    }
                    auto sentFuture = m_recognizeEvent->send(m_audioInputProcessor);
                    if (AudioInputProcessor::INVALID_INDEX == begin) {
                        EXPECT_EQ(m_writer->write(m_pattern.data(), m_pattern.size()), static_cast<ssize_t>(m_pattern.size()));
                    }
                    auto sent = sentFuture.get();
                    EXPECT_TRUE(sent);
                    if (!sent) return false;
                    if (RecognizeStopPoint::AFTER_RECOGNIZE == stopPoint) {
                        EXPECT_TRUE(m_audioInputProcessor->stopCapture().valid());
                        m_dialogUXStateAggregator->onRequestProcessingStarted();
                    }
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                bool AudioInputProcessorTest::testStopCaptureSucceeds() {
                    mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::BUSY));
                    //EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE))
                        .WillOnce(InvokeWithoutArgs([&] {
                            lock_guard<std::mutex> lock(mutex);
                            done = true;
                            conditionVariable.notify_one();
                        }));
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    auto stopCaptureResult = m_audioInputProcessor->stopCapture();
                    m_dialogUXStateAggregator->onRequestProcessingStarted();
                    EXPECT_TRUE(stopCaptureResult.valid());
                    if (!stopCaptureResult.valid() && stopCaptureResult.get()) return false;
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                bool AudioInputProcessorTest::testContextFailure(avsCommon::sdkInterfaces::ContextRequestError error) {
                    std::mutex mutex;
                    std::condition_variable conditionVariable;
                    bool done = false;
                    RecognizeEvent recognize(*m_audioProvider, Initiator::TAP);
                    EXPECT_CALL(*m_mockContextManager, getContextWithoutReportableStateProperties(_, _, _))
                        .WillOnce(InvokeWithoutArgs([this, error] {
                            m_audioInputProcessor->onContextFailure(error);
                            return CONTEXT_REQUEST_TOKEN;
                        }));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::RECOGNIZING));
                    EXPECT_CALL(*m_mockUserInactivityMonitor, onUserActive()).Times(2);
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE))
                        .WillOnce(InvokeWithoutArgs([&] {
                            std::lock_guard<std::mutex> lock(mutex);
                            done = true;
                            conditionVariable.notify_one();
                        }));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    if (recognize.send(m_audioInputProcessor).get()) {
                        std::unique_lock<std::mutex> lock(mutex);
                        return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                    }
                    return false;
                }
                bool AudioInputProcessorTest::testStopCaptureDirectiveSucceeds(bool withDialogRequestId) {
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    auto avsDirective = createAVSDirective(STOP_CAPTURE, withDialogRequestId);
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::BUSY));
                    //EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _));
                    if (withDialogRequestId) {
                        EXPECT_CALL(*result, setCompleted());
                    }
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE))
                        .WillOnce(InvokeWithoutArgs([&] {
                            lock_guard<std::mutex> lock(mutex);
                            done = true;
                            conditionVariable.notify_one();
                        }));
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    if (!withDialogRequestId) directiveHandler->handleDirectiveImmediately(avsDirective);
                    else {
                        directiveHandler->preHandleDirective(avsDirective, std::move(result));
                        EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                    }
                    m_dialogUXStateAggregator->onRequestProcessingStarted();
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                bool AudioInputProcessorTest::testStopCaptureDirectiveFails(bool withDialogRequestId) {
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    auto avsDirective = createAVSDirective(STOP_CAPTURE, withDialogRequestId);
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    EXPECT_CALL(*result, setFailed(_)).WillOnce(InvokeWithoutArgs([&] {
                        lock_guard<std::mutex> lock(mutex);
                        done = true;
                        conditionVariable.notify_one();
                    }));
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    if (!withDialogRequestId) directiveHandler->handleDirectiveImmediately(avsDirective);
                    else {
                        directiveHandler->preHandleDirective(avsDirective, move(result));
                        EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                    }
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                bool AudioInputProcessorTest::testExpectSpeechSucceeds(bool withDialogRequestId) {
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    auto avsDirective = createAVSDirective(EXPECT_SPEECH, withDialogRequestId);
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::RECOGNIZING));
                    EXPECT_CALL(*m_mockUserInactivityMonitor, onUserActive()).Times(2);
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    if (withDialogRequestId) { EXPECT_CALL(*result, setCompleted()); }
                    EXPECT_CALL(*m_mockContextManager, getContextWithoutReportableStateProperties(_, _, _))
                        .WillOnce(InvokeWithoutArgs([&] {
                            lock_guard<std::mutex> lock(mutex);
                            done = true;
                            conditionVariable.notify_one();
                            return CONTEXT_REQUEST_TOKEN;
                        }));
                    if (!withDialogRequestId) directiveHandler->handleDirectiveImmediately(avsDirective);
                    else {
                        directiveHandler->preHandleDirective(avsDirective, move(result));
                        EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                    }
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                bool AudioInputProcessorTest::testExpectSpeechWaits(bool withDialogRequestId, bool verifyTimeout) {
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    auto avsDirective = createAVSDirective(EXPECT_SPEECH, withDialogRequestId);
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    if (withDialogRequestId) { EXPECT_CALL(*result, setCompleted()); }
                    if (verifyTimeout) {
                        EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).WillOnce(Invoke(&verifyExpectSpeechTimedOut));
                        EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE))
                            .WillOnce(InvokeWithoutArgs([&] {
                                lock_guard<std::mutex> lock(mutex);
                                done = true;
                                conditionVariable.notify_one();
                            }));
                        EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                            .Times(AtLeast(1));
                        EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    } else {
                        EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH))
                            .WillOnce(InvokeWithoutArgs([&] {
                                lock_guard<std::mutex> lock(mutex);
                                done = true;
                                conditionVariable.notify_one();
                            }));
                        EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                            .Times(AtLeast(1));
                    }
                    if (!withDialogRequestId) directiveHandler->handleDirectiveImmediately(avsDirective);
                    else {
                        directiveHandler->preHandleDirective(avsDirective, std::move(result));
                        EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                    }
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                bool AudioInputProcessorTest::testExpectSpeechFails(bool withDialogRequestId) {
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    auto avsDirective = createAVSDirective(EXPECT_SPEECH, withDialogRequestId);
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    if (withDialogRequestId) {
                        EXPECT_CALL(*result, setFailed(_)).WillOnce(InvokeWithoutArgs([&] {
                            lock_guard<std::mutex> lock(mutex);
                            done = true;
                            conditionVariable.notify_one();
                        }));
                    }
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    if (!withDialogRequestId) {
                        directiveHandler->handleDirectiveImmediately(avsDirective);
                        return true;
                    } else {
                        directiveHandler->preHandleDirective(avsDirective, move(result));
                        EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                        unique_lock<std::mutex> lock(mutex);
                        return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                    }
                }
                static bool getInitiatorFromDirective(const string directive, string* initiator) {
                    string event, payload;
                    if (!jsonUtils::retrieveValue(directive, MESSAGE_EVENT_KEY, &event)) return false;
                    if (!jsonUtils::retrieveValue(event, MESSAGE_PAYLOAD_KEY, &payload)) return false;
                    return jsonUtils::retrieveValue(payload, EXPECT_SPEECH_INITIATOR_KEY, initiator);
                }
                bool AudioInputProcessorTest::testRecognizeWithExpectSpeechInitiator(bool withInitiator) {
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    auto avsDirective = createAVSDirective(EXPECT_SPEECH, true, withInitiator);
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                        .WillOnce(Invoke([&](shared_ptr<MessageRequest> request) {
                            string actualInitiatorString;
                            if (withInitiator) {
                                ASSERT_TRUE(getInitiatorFromDirective(request->getJsonContent(), &actualInitiatorString));
                                ASSERT_EQ(actualInitiatorString, EXPECT_SPEECH_INITIATOR);
                            } else { ASSERT_FALSE(getInitiatorFromDirective(request->getJsonContent(), &actualInitiatorString)); }
                            lock_guard<std::mutex> lock(mutex);
                            done = true;
                            conditionVariable.notify_one();
                        }));
                    Document contextDocument(kObjectType);
                    rapidjson::Value contextArray(kArrayType);
                    rapidjson::Value message_context_key{MESSAGE_CONTEXT_KEY.data(), MESSAGE_CONTEXT_KEY.length()};
                    contextDocument.AddMember(message_context_key, contextArray, contextDocument.GetAllocator());
                    rapidjson::StringBuffer contextBuffer;
                    rapidjson::Writer<StringBuffer> contextWriter(contextBuffer);
                    contextDocument.Accept(contextWriter);
                    string contextJson = contextBuffer.GetString();
                    EXPECT_CALL(*result, setCompleted());
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::RECOGNIZING));
                    EXPECT_CALL(*m_mockUserInactivityMonitor, onUserActive()).Times(2);
                    EXPECT_CALL(*m_mockContextManager, getContextWithoutReportableStateProperties(_, _, _))
                        .WillOnce(Return(CONTEXT_REQUEST_TOKEN));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    directiveHandler->preHandleDirective(avsDirective, std::move(result));
                    EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                    EXPECT_EQ(std::string(""), m_mockDirectiveSequencer->getDialogRequestId());
                    m_audioInputProcessor->onFocusChanged(avsCommon::avs::FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    m_audioInputProcessor->onContextAvailable(contextJson);
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                shared_ptr<AVSDirective> AudioInputProcessorTest::createAVSDirective(const NamespaceAndName& directive, bool withDialogRequestId,
                                                                                     bool withInitiator) {
                    rapidjson::Document document(rapidjson::kObjectType);
                    rapidjson::Value payloadJson(rapidjson::kObjectType);
                    if (EXPECT_SPEECH == directive) {
                        rapidjson::Value timeoutInMillisecondsJson(EXPECT_SPEECH_TIMEOUT_IN_MILLISECONDS);
                        rapidjson::Value expect_speech_timeout_key{EXPECT_SPEECH_TIMEOUT_KEY.data(), EXPECT_SPEECH_TIMEOUT_KEY.length()};
                        payloadJson.AddMember(expect_speech_timeout_key, timeoutInMillisecondsJson);
                        if (withInitiator) {
                            rapidjson::Value initiatorJson(EXPECT_SPEECH_INITIATOR.data(), EXPECT_SPEECH_INITIATOR.length());
                            rapidjson::Value expect_speech_initiator_key{EXPECT_SPEECH_INITIATOR_KEY.data(), EXPECT_SPEECH_INITIATOR_KEY.length()};
                            payloadJson.AddMember(expect_speech_initiator_key, initiatorJson);
                        }
                    }
                    return createAVSDirective(directive, withDialogRequestId, withInitiator, document, payloadJson);
                }
                shared_ptr<AVSDirective> AudioInputProcessorTest::createAVSDirective(const NamespaceAndName& directive, bool withDialogRequestId,
                                                                                     bool withInitiator, Document& document,
                                                                                     rapidjson::Value& payloadJson) {
                    auto header = make_shared<AVSMessageHeader>(directive.nameSpace, directive.name, generateUUID(), generateUUID());
                    rapidjson::Value directiveJson(kObjectType);
                    rapidjson::Value headerJson(kObjectType);
                    rapidjson::Value namespaceJson(header->getNamespace().data(), header->getNamespace().length());
                    rapidjson::Value nameJson(header->getName().data(), header->getName().length());
                    rapidjson::Value messageIdJson(header->getMessageId().data(), header->getMessageId().length());
                    rapidjson::Value dialogRequestIdJson(header->getDialogRequestId().data(), header->getDialogRequestId().length());
                    rapidjson::Value message_namespace_key{MESSAGE_NAMESPACE_KEY.data(), MESSAGE_NAMESPACE_KEY.length()};
                    rapidjson::Value message_name_key{MESSAGE_NAME_KEY.data(), MESSAGE_NAME_KEY.length()};
                    rapidjson::Value message_message_id_key{MESSAGE_MESSAGE_ID_KEY.data(), MESSAGE_MESSAGE_ID_KEY.length()};
                    rapidjson::Value message_dialog_request_id_key{MESSAGE_DIALOG_REQUEST_ID_KEY.data(), MESSAGE_DIALOG_REQUEST_ID_KEY.length()};
                    rapidjson::Value message_header_key{MESSAGE_HEADER_KEY.data(), MESSAGE_HEADER_KEY.length()};
                    headerJson.AddMember(message_namespace_key, namespaceJson);
                    headerJson.AddMember(message_name_key, nameJson);
                    headerJson.AddMember(message_message_id_key, messageIdJson);
                    headerJson.AddMember(message_dialog_request_id_key, dialogRequestIdJson);
                    directiveJson.AddMember(message_header_key, headerJson);
                    rapidjson::StringBuffer payloadBuffer;
                    rapidjson::Writer<rapidjson::StringBuffer> payloadWriter(payloadBuffer);
                    payloadJson.Accept(payloadWriter);
                    rapidjson::Value message_payload_key{MESSAGE_PAYLOAD_KEY.data(), MESSAGE_PAYLOAD_KEY.length()};
                    rapidjson::Value message_directive_key{MESSAGE_DIRECTIVE_KEY.data(), MESSAGE_DIRECTIVE_KEY.length()};
                    directiveJson.AddMember(message_payload_key, payloadJson);
                    document.AddMember(message_directive_key, directiveJson);
                    rapidjson::StringBuffer documentBuffer;
                    rapidjson::Writer<rapidjson::StringBuffer> documentWriter(documentBuffer);
                    document.Accept(documentWriter);
                    //auto mockAttachmentManager = make_shared<MockAttachmentManager>();
                    //return AVSDirective::create(documentBuffer.GetString(), header, payloadBuffer.GetString(), mockAttachmentManager, "");
                    return nullptr;
                }
                void AudioInputProcessorTest::verifyExpectSpeechTimedOut(shared_ptr<MessageRequest> request) {
                    Document document = parseJson(request->getJsonContent());
                    auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                    EXPECT_NE(event, document.MemberEnd());
                    auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                    EXPECT_NE(header, event->value.MemberEnd());
                    auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY.data());
                    EXPECT_NE(payload, event->value.MemberEnd());
                    rapidjson::Value value{header->value.GetString(), strlen(header->value.GetString())};
                    EXPECT_EQ(getJsonString(value, MESSAGE_NAMESPACE_KEY), NAMESPACE);
                    EXPECT_EQ(getJsonString(value, MESSAGE_NAME_KEY), EXPECT_SPEECH_TIMED_OUT_EVENT_NAME);
                    EXPECT_NE(getJsonString(value, MESSAGE_MESSAGE_ID_KEY), "");
                    EXPECT_EQ(request->attachmentReadersCount(), 0);
                }
                void AudioInputProcessorTest::removeDefaultAudioProvider() {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,m_mockExceptionEncounteredSender,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation,m_mockWakeWordSetting,
                                                            nullptr, AudioProvider::null(),m_mockPowerResourceManager,
                                                                         m_metricRecorder);
                    EXPECT_NE(m_audioInputProcessor, nullptr);
                    m_audioInputProcessor->addObserver(m_mockObserver);
                    m_audioInputProcessor->addObserver(m_dialogUXStateAggregator);
                }
                void AudioInputProcessorTest::makeDefaultAudioProviderNotAlwaysReadable() {
                    m_audioProvider->alwaysReadable = false;
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,m_mockExceptionEncounteredSender,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation,m_mockWakeWordSetting,
                                                            nullptr,*m_audioProvider,m_mockPowerResourceManager,
                                                                         m_metricRecorder);
                    EXPECT_NE(m_audioInputProcessor, nullptr);
                    m_audioInputProcessor->addObserver(m_mockObserver);
                    m_audioInputProcessor->addObserver(m_dialogUXStateAggregator);
                }
                bool AudioInputProcessorTest::testFocusChange(FocusState state, MixingBehavior behavior) {
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    bool recognizeSucceeded = testRecognizeSucceeds(*m_audioProvider, Initiator::TAP);
                    EXPECT_TRUE(recognizeSucceeded);
                    if (!recognizeSucceeded) return false;
                    /*if (state != FocusState::NONE) {
                        EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _));
                    }*/
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE))
                        .WillOnce(InvokeWithoutArgs([&] {
                            lock_guard<std::mutex> lock(mutex);
                            done = true;
                            conditionVariable.notify_one();
                        }));
                    m_audioInputProcessor->onFocusChanged(state, behavior);
                    unique_lock<std::mutex> lock(mutex);
                    return conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                }
                void AudioInputProcessorTest::testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status eventStreamFinishedStatus,
                                                                                  StopCaptureDirectiveSchedule stopCaptureSchedule,
                                                                                  AudioInputProcessorObserverInterface::State expectedAIPFinalState,
                                                                                  bool expectFocusReleased) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(_)).Times(AtLeast(0));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(expectedAIPFinalState)).Times(1);
                    //if (expectFocusReleased) { EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _)); }
                    auto avsDirective = createAVSDirective(STOP_CAPTURE, true);
                    if (StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE == stopCaptureSchedule) {
                        m_audioInputProcessor->handleDirectiveImmediately(avsDirective);
                    }
                    m_audioInputProcessor->onSendCompleted(eventStreamFinishedStatus);
                    if (StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE == stopCaptureSchedule) {
                        m_audioInputProcessor->handleDirectiveImmediately(avsDirective);
                    }
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutDirectiveSequencer) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(nullptr,m_mockMessageSender,m_mockContextManager,
                                                                        m_mockFocusManager,m_dialogUXStateAggregator,
                                                                        m_mockExceptionEncounteredSender,m_mockUserInactivityMonitor,
                                                                        m_mockSystemSoundPlayer,m_mockAssetsManager,
                                                                        m_mockWakeWordConfirmation,m_mockSpeechConfirmation,
                                                                        m_mockWakeWordSetting, nullptr,*m_audioProvider,
                                                     m_mockPowerResourceManager,m_metricRecorder);
                    EXPECT_EQ(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutMessageSender) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,nullptr,m_mockContextManager,
                                                            m_mockFocusManager,m_dialogUXStateAggregator,
                                                 m_mockExceptionEncounteredSender,m_mockUserInactivityMonitor,
                                                         m_mockSystemSoundPlayer,m_mockAssetsManager,
                                                      m_mockWakeWordConfirmation,m_mockSpeechConfirmation,
                                                         m_mockWakeWordSetting,nullptr,*m_audioProvider,
                                                                        m_mockPowerResourceManager,m_metricRecorder);
                    EXPECT_EQ(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutContextManager) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           nullptr,m_mockFocusManager,m_dialogUXStateAggregator,
                                                  m_mockExceptionEncounteredSender,m_mockUserInactivityMonitor,
                                                         m_mockSystemSoundPlayer,m_mockAssetsManager,
                                                      m_mockWakeWordConfirmation,m_mockSpeechConfirmation,
                                                          m_mockWakeWordSetting,nullptr,*m_audioProvider,
                                                      m_mockPowerResourceManager,m_metricRecorder);
                    EXPECT_EQ(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutFocusManager) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,nullptr,m_dialogUXStateAggregator,
                                                  m_mockExceptionEncounteredSender,m_mockUserInactivityMonitor,
                                                         m_mockSystemSoundPlayer,m_mockAssetsManager,
                                                      m_mockWakeWordConfirmation,m_mockSpeechConfirmation,
                                                          m_mockWakeWordSetting,nullptr,*m_audioProvider,
                                                      m_mockPowerResourceManager,m_metricRecorder);
                    EXPECT_EQ(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutStateAggregator) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                                         nullptr,m_mockExceptionEncounteredSender,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation,m_mockWakeWordSetting,
                                                            nullptr,*m_audioProvider,m_mockPowerResourceManager,
                                                            m_metricRecorder);
                    EXPECT_EQ(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutExceptionSender) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,nullptr,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation,m_mockWakeWordSetting,
                                                            nullptr,*m_audioProvider,
                                                      m_mockPowerResourceManager,m_metricRecorder);
                    EXPECT_EQ(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutUserInactivityMonitor) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,m_mockExceptionEncounteredSender,
                                                        nullptr,m_mockSystemSoundPlayer,m_mockAssetsManager,
                                                      m_mockWakeWordConfirmation,m_mockSpeechConfirmation,
                                                          m_mockWakeWordSetting,nullptr,*m_audioProvider,
                                                      m_mockPowerResourceManager,m_metricRecorder);
                    EXPECT_EQ(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutAudioProvider) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,m_mockExceptionEncounteredSender,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation,m_mockWakeWordSetting,
                                                            nullptr, AudioProvider::null(),m_mockPowerResourceManager,
                                                            m_metricRecorder);
                    EXPECT_NE(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutPowerResourceManager) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,m_mockExceptionEncounteredSender,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation,m_mockWakeWordSetting,
                                                            nullptr, AudioProvider::null(),nullptr,
                                                            m_metricRecorder);
                    EXPECT_NE(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_createWithoutMetricRecorder) {
                    m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
                    m_audioInputProcessor = AudioInputProcessor::create(m_mockDirectiveSequencer,m_mockMessageSender,
                                                           m_mockContextManager,m_mockFocusManager,
                                                     m_dialogUXStateAggregator,m_mockExceptionEncounteredSender,
                                                        m_mockUserInactivityMonitor,m_mockSystemSoundPlayer,
                                                             m_mockAssetsManager,m_mockWakeWordConfirmation,
                                                         m_mockSpeechConfirmation, m_mockWakeWordSetting,nullptr,
                                                                         AudioProvider::null(),m_mockPowerResourceManager,
                                                            nullptr);
                    EXPECT_NE(m_audioInputProcessor, nullptr);
                }
                TEST_F(AudioInputProcessorTest, test_getConfiguration) {
                    DirectiveHandlerConfiguration expectedConfiguration{
                        {STOP_CAPTURE, BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)},
                        {EXPECT_SPEECH, BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true)},
                        {SET_END_OF_SPEECH_OFFSET, BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)},
                        {SET_WAKE_WORD_CONFIRMATION, BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)},
                        {SET_SPEECH_CONFIRMATION, BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)},
                        {SET_WAKE_WORDS, BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)}};
                    auto configuration = m_audioInputProcessor->getConfiguration();
                    EXPECT_EQ(configuration, expectedConfiguration);
                }
                TEST_F(AudioInputProcessorTest, test_addRemoveObserver) {
                    m_audioInputProcessor->addObserver(nullptr);
                    m_audioInputProcessor->removeObserver(nullptr);
                    auto observer = std::make_shared<MockObserver>();
                    m_audioInputProcessor->addObserver(observer);
                    m_audioInputProcessor->removeObserver(observer);
                    auto observer2 = std::make_shared<MockObserver>();
                    m_audioInputProcessor->addObserver(observer);
                    m_audioInputProcessor->addObserver(observer2);
                    m_audioInputProcessor->removeObserver(observer);
                    m_audioInputProcessor->removeObserver(observer2);
                    m_audioInputProcessor->removeObserver(observer);
                }
                TEST_F(AudioInputProcessorTest, test_recognizeNullStream) {
                    auto result = m_audioInputProcessor->recognize(AudioProvider::null(), Initiator::PRESS_AND_HOLD);
                    ASSERT_TRUE(result.valid());
                    ASSERT_FALSE(result.get());
                }
                TEST_F(AudioInputProcessorTest, test_recognizeInvalidAudioFormat) {
                    AudioProvider audioProvider = *m_audioProvider;
                    audioProvider.format.endianness = avsCommon::utils::AudioFormat::Endianness::BIG;
                    EXPECT_FALSE(m_audioInputProcessor->recognize(audioProvider, Initiator::PRESS_AND_HOLD).get());
                    audioProvider = *m_audioProvider;
                    audioProvider.format.sampleRateHz = 0;
                    EXPECT_FALSE(m_audioInputProcessor->recognize(audioProvider, Initiator::PRESS_AND_HOLD).get());
                    audioProvider = *m_audioProvider;
                    audioProvider.format.sampleSizeInBits = 0;
                    EXPECT_FALSE(m_audioInputProcessor->recognize(audioProvider, Initiator::PRESS_AND_HOLD).get());
                    audioProvider = *m_audioProvider;
                    audioProvider.format.numChannels = 0;
                    EXPECT_FALSE(m_audioInputProcessor->recognize(audioProvider, Initiator::PRESS_AND_HOLD).get());
                }
                TEST_F(AudioInputProcessorTest, test_recognizePressAndHold) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::PRESS_AND_HOLD));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeTap) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithoutKeyword) {
                    EXPECT_TRUE(testRecognizeFails(*m_audioProvider, Initiator::WAKEWORD));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithBadBegin) {
                    for (size_t written = 0; written <= SDS_WORDS; written += PATTERN_WORDS) {
                        EXPECT_EQ(m_writer->write(m_pattern.data(), m_pattern.size()), static_cast<ssize_t>(m_pattern.size()));
                    }
                    avsCommon::avs::AudioInputStream::Index begin = 0;
                    auto end = AudioInputProcessor::INVALID_INDEX;
                    EXPECT_TRUE(testRecognizeFails(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithKeyword) {
                    auto begin = AudioInputProcessor::INVALID_INDEX;
                    auto end = AudioInputProcessor::INVALID_INDEX;
                    EXPECT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithGoodBegin) {
                    avsCommon::avs::AudioInputStream::Index begin = 0;
                    auto end = AudioInputProcessor::INVALID_INDEX;
                    EXPECT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithGoodBeginAndEnd) {
                    avsCommon::avs::AudioInputStream::Index begin = PREROLL_WORDS;
                    avsCommon::avs::AudioInputStream::Index end = PREROLL_WORDS + WAKEWORD_WORDS;
                    EXPECT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeCloseTalk) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::CLOSE_TALK;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::PRESS_AND_HOLD));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeNearField) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::NEAR_FIELD;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeFarField) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::FAR_FIELD;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWhileExpectingSpeech) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, !VERIFY_TIMEOUT));
                    ASSERT_TRUE(testRecognizeSucceeds(
                        *m_audioProvider,
                        Initiator::PRESS_AND_HOLD,
                        AudioInputProcessor::INVALID_INDEX,
                        AudioInputProcessor::INVALID_INDEX,
                        "",
                        RecognizeStopPoint::NONE,
                        std::make_shared<std::string>(EXPECT_SPEECH_INITIATOR)));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeStopAfterRecognize) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::CLOSE_TALK;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::PRESS_AND_HOLD, AudioInputProcessor::INVALID_INDEX,
                                AudioInputProcessor::INVALID_INDEX, "", RecognizeStopPoint::AFTER_RECOGNIZE));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeStopAfterContext) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::CLOSE_TALK;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::PRESS_AND_HOLD, AudioInputProcessor::INVALID_INDEX,
                                AudioInputProcessor::INVALID_INDEX, "", RecognizeStopPoint::AFTER_CONTEXT));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeStopAfterFocus) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::CLOSE_TALK;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::PRESS_AND_HOLD, AudioInputProcessor::INVALID_INDEX,
                                AudioInputProcessor::INVALID_INDEX, "", RecognizeStopPoint::AFTER_FOCUS));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeStopAfterSend) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::CLOSE_TALK;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::PRESS_AND_HOLD, AudioInputProcessor::INVALID_INDEX,
                                AudioInputProcessor::INVALID_INDEX, "", RecognizeStopPoint::AFTER_SEND));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeBargeInWhileRecognizingCloseTalk) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::CLOSE_TALK;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::TAP));
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeBargeInWhileRecognizingNearField) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::NEAR_FIELD;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::TAP));
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeBargeInWhileRecognizingFarField) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.profile = ASRProfile::FAR_FIELD;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::TAP));
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeBargeInWhileRecognizingCantOverride) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP));
                    auto audioProvider = *m_audioProvider;
                    audioProvider.canOverride = false;
                    ASSERT_TRUE(testRecognizeFails(audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeBargeInWhileRecognizingCantBeOverridden) {
                    auto audioProvider = *m_audioProvider;
                    audioProvider.canBeOverridden = false;
                    ASSERT_TRUE(testRecognizeSucceeds(audioProvider, Initiator::TAP));
                    ASSERT_TRUE(testRecognizeFails(*m_audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureWhenIdle) {
                    ASSERT_FALSE(m_audioInputProcessor->stopCapture().get());
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureWhenExpectingSpeech) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, !VERIFY_TIMEOUT));
                    ASSERT_FALSE(m_audioInputProcessor->stopCapture().get());
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureWhenRecognizing) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    ASSERT_TRUE(testStopCaptureSucceeds());
                    auto readStatus = avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK;
                    std::vector<uint8_t> buf(SDS_WORDS * SDS_WORDSIZE);
                    EXPECT_EQ(m_recognizeEvent->getReader()->read(buf.data(), buf.size(), &readStatus), 0U);
                    ASSERT_EQ(readStatus, avsCommon::avs::attachment::AttachmentReader::ReadStatus::CLOSED);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureWhenRecognizingFollowByStopCaptureDirective) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    ASSERT_TRUE(testStopCaptureSucceeds());
                    auto readStatus = avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK;
                    vector<uint8_t> buf(SDS_WORDS * SDS_WORDSIZE);
                    EXPECT_EQ(m_recognizeEvent->getReader()->read(buf.data(), buf.size(), &readStatus), 0U);
                    ASSERT_EQ(readStatus,AttachmentReader::ReadStatus::CLOSED);
                    std::mutex mutex;
                    condition_variable conditionVariable;
                    bool done = false;
                    auto avsDirective = createAVSDirective(STOP_CAPTURE, true);
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    EXPECT_CALL(*result, setCompleted()).WillOnce(InvokeWithoutArgs([&] {
                        lock_guard<std::mutex> lock(mutex);
                        done = true;
                        conditionVariable.notify_one();
                    }));
                    directiveHandler->preHandleDirective(avsDirective, move(result));
                    EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                    unique_lock<std::mutex> lock(mutex);
                    auto setCompletedResult = conditionVariable.wait_for(lock, TEST_TIMEOUT, [&done] { return done; });
                    EXPECT_TRUE(setCompletedResult);
                }
                TEST_F(AudioInputProcessorTest, test_resetStateWhenIdle) {
                    m_audioInputProcessor->resetState().get();
                }
                TEST_F(AudioInputProcessorTest, test_resetStateWhenExpectingSpeech) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, !VERIFY_TIMEOUT));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE));
                    m_audioInputProcessor->resetState().get();
                }
                TEST_F(AudioInputProcessorTest, test_resetStateWhenRecognizing) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    //EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE));
                    m_audioInputProcessor->resetState().get();
                }
                TEST_F(AudioInputProcessorTest, test_contextFailureStateProviderTimedout) {
                    ASSERT_TRUE(testContextFailure(avsCommon::sdkInterfaces::ContextRequestError::STATE_PROVIDER_TIMEDOUT));
                }
                TEST_F(AudioInputProcessorTest, test_contextFailureBuildContextError) {
                    ASSERT_TRUE(testContextFailure(avsCommon::sdkInterfaces::ContextRequestError::BUILD_CONTEXT_ERROR));
                }
                TEST_F(AudioInputProcessorTest, test_preHandleAndHandleDirectiveStopCaptureWhenIdle) {
                    ASSERT_TRUE(testStopCaptureDirectiveFails(WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_preHandleAndHandleDirectiveStopCaptureWhenRecognizing) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    ASSERT_TRUE(testStopCaptureDirectiveSucceeds(WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_preHandleAndHandleDirectiveStopCaptureWhenExpectingSpeech) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, !VERIFY_TIMEOUT));
                    ASSERT_TRUE(testStopCaptureDirectiveFails(WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_handleDirectiveImmediatelyStopCaptureWhenRecognizing) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    ASSERT_TRUE(testStopCaptureDirectiveSucceeds(!WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_preHandleAndHandleDirectiveExpectSpeechWhenIdle) {
                    ASSERT_TRUE(testExpectSpeechSucceeds(WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_handleDirectiveImmediatelyExpectSpeechWhenIdle) {
                    ASSERT_TRUE(testExpectSpeechSucceeds(!WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_preHandleAndHandleDirectiveExpectSpeechWhenRecognizing) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    ASSERT_TRUE(testExpectSpeechFails(WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_preHandleAndHandleDirectiveExpectSpeechWhenExpectingSpeech) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, !VERIFY_TIMEOUT));
                    ASSERT_TRUE(testExpectSpeechFails(WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_expectSpeechNoDefaultNoPrevious) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, VERIFY_TIMEOUT));
                }
                TEST_F(AudioInputProcessorTest, test_expectSpeechUnreadableDefaultNoPrevious) {
                    makeDefaultAudioProviderNotAlwaysReadable();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, VERIFY_TIMEOUT));
                }
                TEST_F(AudioInputProcessorTest, test_expectSpeechUnreadableDefaultUnreadablePrevious) {
                    makeDefaultAudioProviderNotAlwaysReadable();
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::PRESS_AND_HOLD, 0));
                    ASSERT_TRUE(testStopCaptureSucceeds());
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, VERIFY_TIMEOUT));
                }
                TEST_F(AudioInputProcessorTest, test_expectSpeechNoDefaultReadablePrevious) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::PRESS_AND_HOLD, 0));
                    ASSERT_TRUE(testStopCaptureSucceeds());
                    ASSERT_TRUE(testExpectSpeechSucceeds(WITH_DIALOG_REQUEST_ID));
                }
                TEST_F(AudioInputProcessorTest, test_expectSpeechWithInitiator) {
                    ASSERT_TRUE(testRecognizeWithExpectSpeechInitiator(true));
                }
                TEST_F(AudioInputProcessorTest, test_expectSpeechWithNoInitiator) {
                    ASSERT_TRUE(testRecognizeWithExpectSpeechInitiator(false));
                }
                TEST_F(AudioInputProcessorTest, test_expectSpeechWithInitiatorTimedOut) {
                    removeDefaultAudioProvider();
                    ASSERT_TRUE(testExpectSpeechWaits(WITH_DIALOG_REQUEST_ID, VERIFY_TIMEOUT));
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_focusChangedBackground) {
                    ASSERT_TRUE(testFocusChange(avsCommon::avs::FocusState::BACKGROUND, avsCommon::avs::MixingBehavior::MUST_PAUSE));
                }
                TEST_F(AudioInputProcessorTest, test_focusChangedNone) {
                    ASSERT_TRUE(testFocusChange(avsCommon::avs::FocusState::NONE, avsCommon::avs::MixingBehavior::MUST_STOP));
                }
                TEST_F(AudioInputProcessorTest, test_resetStateOnTimeOut) {
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP, 0));
                    //EXPECT_CALL(*m_mockFocusManager, releaseChannel(CHANNEL_NAME, _));
                    EXPECT_CALL(*m_mockObserver, onStateChanged(AudioInputProcessorObserverInterface::State::IDLE));
                    m_audioInputProcessor->onSendCompleted(avsCommon::sdkInterfaces::MessageRequestObserverInterface::Status::TIMEDOUT);
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithESPWithKeyword) {
                    auto begin = AudioInputProcessor::INVALID_INDEX;
                    auto end = AudioInputProcessor::INVALID_INDEX;
                    EXPECT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT, RecognizeStopPoint::NONE, nullptr));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithInvalidESPWithKeyword) {
                    auto begin = AudioInputProcessor::INVALID_INDEX;
                    auto end = AudioInputProcessor::INVALID_INDEX;
                    EXPECT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT, RecognizeStopPoint::NONE, nullptr));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeOPUSWithTap) {
                    m_audioProvider->format.encoding = avsCommon::utils::AudioFormat::Encoding::OPUS;
                    m_audioProvider->format.sampleRateHz = 32000;
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::TAP));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeOPUSWithPressAndHold) {
                    m_audioProvider->format.encoding = avsCommon::utils::AudioFormat::Encoding::OPUS;
                    m_audioProvider->format.sampleRateHz = 32000;
                    ASSERT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::PRESS_AND_HOLD));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeOPUSWithWakeWord) {
                    avsCommon::avs::AudioInputStream::Index begin = 0;
                    avsCommon::avs::AudioInputStream::Index end = AudioInputProcessor::INVALID_INDEX;
                    m_audioProvider->format.encoding = avsCommon::utils::AudioFormat::Encoding::OPUS;
                    m_audioProvider->format.sampleRateHz = 32000;
                    EXPECT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeWakewordWithKWDMetadata) {
                    auto begin = AudioInputProcessor::INVALID_INDEX;
                    auto end = AudioInputProcessor::INVALID_INDEX;
                    auto metadata = std::make_shared<std::vector<char>>();
                    metadata->assign(KWD_METADATA_EXAMPLE.data(), KWD_METADATA_EXAMPLE.data() + KWD_METADATA_EXAMPLE.length());
                    EXPECT_TRUE(testRecognizeSucceeds(*m_audioProvider, Initiator::WAKEWORD, begin, end, KEYWORD_TEXT, RecognizeStopPoint::NONE, nullptr, metadata));
                }
                TEST_F(AudioInputProcessorTest, test_recognizeInvalidWakeWord) {
                    AudioInputStream::Index begin = PREROLL_WORDS;
                    AudioInputStream::Index end = PREROLL_WORDS + WAKEWORD_WORDS;
                    EXPECT_TRUE(testRecognizeFails(*m_audioProvider, Initiator::WAKEWORD, begin, end, AudioInputProcessor::KEYWORD_TEXT_STOP));
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamSuccess) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SUCCESS,StopCaptureDirectiveSchedule::NONE,
                                      AudioInputProcessorObserverInterface::State::BUSY,false);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamSuccessNoContent) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT,StopCaptureDirectiveSchedule::NONE,
                                       AudioInputProcessorObserverInterface::State::IDLE,false);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamSuccessNotConnected) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::NOT_CONNECTED,StopCaptureDirectiveSchedule::NONE,
                                       AudioInputProcessorObserverInterface::State::IDLE,true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamNotSynchronized) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::NOT_SYNCHRONIZED,StopCaptureDirectiveSchedule::NONE,
                                       AudioInputProcessorObserverInterface::State::IDLE,true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamInternalrror) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::INTERNAL_ERROR,StopCaptureDirectiveSchedule::NONE,
                                       AudioInputProcessorObserverInterface::State::IDLE,true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamProtocolError) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::PROTOCOL_ERROR,StopCaptureDirectiveSchedule::NONE,
                                      AudioInputProcessorObserverInterface::State::IDLE,true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamServerInternalError) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2,
                                       StopCaptureDirectiveSchedule::NONE, AudioInputProcessorObserverInterface::State::IDLE,
                                       true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamRefused) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::REFUSED,StopCaptureDirectiveSchedule::NONE,
                                      AudioInputProcessorObserverInterface::State::IDLE,true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamCanceled) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::CANCELED,StopCaptureDirectiveSchedule::NONE,
                                      AudioInputProcessorObserverInterface::State::IDLE,true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamThrottled) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::THROTTLED,StopCaptureDirectiveSchedule::NONE,
                                      AudioInputProcessorObserverInterface::State::IDLE,true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamInvalidAuth) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::INVALID_AUTH,
                                       StopCaptureDirectiveSchedule::NONE,AudioInputProcessorObserverInterface::State::IDLE,
                                       true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamBadRequest) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::BAD_REQUEST,
                                       StopCaptureDirectiveSchedule::NONE,AudioInputProcessorObserverInterface::State::IDLE,
                                       true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamUnknownServerError) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR,
                                       StopCaptureDirectiveSchedule::NONE,AudioInputProcessorObserverInterface::State::IDLE,
                                       true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamSuccess) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SUCCESS,StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::BUSY, false);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamSuccessNoContent) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                       AudioInputProcessorObserverInterface::State::BUSY, false);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamSuccessNotConnected) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::NOT_CONNECTED,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamNotSynchronized) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::NOT_SYNCHRONIZED,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamInternalrror) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::INTERNAL_ERROR,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamProtocolError) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::PROTOCOL_ERROR,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamServerInternalError) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2,
                        StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                        AudioInputProcessorObserverInterface::State::IDLE,
                        true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamRefused) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::REFUSED,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamCanceled) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::CANCELED,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamThrottled) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::THROTTLED,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamInvalidAuth) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::INVALID_AUTH,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamBadRequest) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::BAD_REQUEST,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnDirectiveAndStreamUnknownServerError) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR,
                                       StopCaptureDirectiveSchedule::BEFORE_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamSuccessAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SUCCESS,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::BUSY, false);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamSuccessNoContentAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, false);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamSuccessNotConnectedAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::NOT_CONNECTED,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamNotSynchronizedAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::NOT_SYNCHRONIZED,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamInternalrrorAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::INTERNAL_ERROR,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamProtocolErrorAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::PROTOCOL_ERROR,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamServerInternalErrorAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamRefusedAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::REFUSED,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamCanceledAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::CANCELED,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamThrottledAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::THROTTLED,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamInvalidAuthAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::INVALID_AUTH,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamBadRequestAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::BAD_REQUEST,
                                       AudioInputProcessorTest::StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_stopCaptureOnStreamUnknownServerErrorAndDirective) {
                    testAIPStateTransitionOnEventFinish(MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR,
                                       StopCaptureDirectiveSchedule::AFTER_EVENT_STREAM_CLOSE,
                                      AudioInputProcessorObserverInterface::State::IDLE, true);
                }
                TEST_F(AudioInputProcessorTest, test_handleSetEndOfSpeechOffsetSuccess) {
                    Document document(kObjectType);
                    rapidjson::Value payloadJson(kObjectType);
                    rapidjson::Value endOfSpeechOffsetMilliseconds(END_OF_SPEECH_OFFSET_IN_MILLISECONDS);
                    rapidjson::Value startOfSpeechTimestamp(START_OF_SPEECH_TIMESTAMP_STR.data(), START_OF_SPEECH_TIMESTAMP_STR.length());
                    rapidjson::Value start_of_speech_timestamp_field_name{START_OF_SPEECH_TIMESTAMP_FIELD_NAME.data(), START_OF_SPEECH_TIMESTAMP_FIELD_NAME.length()};
                    rapidjson::Value end_of_speech_offset_field_name(END_OF_SPEECH_OFFSET_FIELD_NAME.data(), END_OF_SPEECH_OFFSET_FIELD_NAME.length());
                    payloadJson.AddMember(start_of_speech_timestamp_field_name, startOfSpeechTimestamp);
                    payloadJson.AddMember(end_of_speech_offset_field_name, endOfSpeechOffsetMilliseconds);
                    auto avsDirective = createAVSDirective(SET_END_OF_SPEECH_OFFSET, true, true, document, payloadJson);
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    EXPECT_CALL(*result, setCompleted());
                    directiveHandler->preHandleDirective(avsDirective, std::move(result));
                    EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                }
                TEST_F(AudioInputProcessorTest, test_handleSetEndOfSpeechOffsetFailureInvalid) {
                    Document document(rapidjson::kObjectType);
                    rapidjson::Value payloadJson(rapidjson::kObjectType);
                    rapidjson::Value badValue("foobar");
                    rapidjson::Value end_of_speech_offset_field_name{END_OF_SPEECH_OFFSET_FIELD_NAME.data(), END_OF_SPEECH_OFFSET_FIELD_NAME.length()};
                    payloadJson.AddMember(end_of_speech_offset_field_name, badValue);
                    auto avsDirective = createAVSDirective(SET_END_OF_SPEECH_OFFSET, true, true, document, payloadJson);
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    EXPECT_CALL(*result, setFailed(_));
                    directiveHandler->preHandleDirective(avsDirective, move(result));
                    EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                }
                TEST_F(AudioInputProcessorTest, test_handleSetEndOfSpeechOffsetFailureMissing) {
                    Document document(kObjectType);
                    rapidjson::Value payloadJson(kObjectType);
                    auto avsDirective = createAVSDirective(SET_END_OF_SPEECH_OFFSET, true, true, document, payloadJson);
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    auto result = make_unique<MockDirectiveHandlerResult>();
                    EXPECT_CALL(*result, setFailed(_));
                    directiveHandler->preHandleDirective(avsDirective, move(result));
                    EXPECT_TRUE(directiveHandler->handleDirective(avsDirective->getMessageId()));
                }
                TEST_F(AudioInputProcessorTest, test_handleSetWakeWordConfirmation) {
                    Document document(kObjectType);
                    rapidjson::Value payloadJson(kObjectType);
                    const string WAKE_WORD_CONFIRMATION_ENABLED_VALUE = "TONE";
                    rapidjson::Value wake_word_confirmation_playload_key{WAKE_WORD_CONFIRMATION_PAYLOAD_KEY.data(), WAKE_WORD_CONFIRMATION_PAYLOAD_KEY.length()};
                    rapidjson::Value wake_word_confirmation_enabled_value{WAKE_WORD_CONFIRMATION_ENABLED_VALUE.data(), WAKE_WORD_CONFIRMATION_ENABLED_VALUE.length()};
                    payloadJson.AddMember(wake_word_confirmation_playload_key, wake_word_confirmation_enabled_value);
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockWakeWordConfirmation, setAvsChange(WakeWordConfirmationSettingType::TONE))
                        .WillOnce(InvokeWithoutArgs([&waitEvent] {
                            waitEvent.wakeUp();
                            return true;
                        }));
                    auto avsDirective = createAVSDirective(SET_WAKE_WORD_CONFIRMATION, true, true, document, payloadJson);
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    directiveHandler->handleDirectiveImmediately(avsDirective);
                    ASSERT_TRUE(waitEvent.wait(TEST_TIMEOUT));
                }
                TEST_F(AudioInputProcessorTest, test_setSpeechConfirmation) {
                    Document document(kObjectType);
                    rapidjson::Value payloadJson(kObjectType);
                    const string SPEECH_WORD_CONFIRMATION_ENABLED_VALUE = "TONE";
                    rapidjson::Value speech_confirmation_payload_key{SPEECH_CONFIRMATION_PAYLOAD_KEY.data(), SPEECH_CONFIRMATION_PAYLOAD_KEY.length()};
                    rapidjson::Value speech_word_confirmation_enabled_value{SPEECH_WORD_CONFIRMATION_ENABLED_VALUE.data(), SPEECH_WORD_CONFIRMATION_ENABLED_VALUE.length()};
                    payloadJson.AddMember(speech_confirmation_payload_key, speech_word_confirmation_enabled_value);
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockSpeechConfirmation, setAvsChange(SpeechConfirmationSettingType::TONE))
                        .WillOnce(InvokeWithoutArgs([&waitEvent] {
                            waitEvent.wakeUp();
                            return true;
                        }));
                    auto avsDirective = createAVSDirective(SET_SPEECH_CONFIRMATION, true, true, document, payloadJson);
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    directiveHandler->handleDirectiveImmediately(avsDirective);
                    ASSERT_TRUE(waitEvent.wait(TEST_TIMEOUT));
                }
                TEST_F(AudioInputProcessorTest, test_setWakeWordsDirectiveSuccess) {
                    JsonGenerator generator;
                    generator.addStringArray(WAKEWORDS_PAYLOAD_KEY, SUPPORTED_WAKE_WORDS);
                    Document document(kObjectType);
                    rapidjson::Value payloadJson(kObjectType);
                    rapidjson::Value wakeWordsPayload(kArrayType);
                    auto newWakeWord = *SUPPORTED_WAKE_WORDS.begin();
                    rapidjson::Value value{newWakeWord.data(), newWakeWord.length()};
                    wakeWordsPayload.PushBack(value);
                    rapidjson::Value wakewords_payload_key{WAKEWORDS_PAYLOAD_KEY.data(), WAKEWORDS_PAYLOAD_KEY.length()};
                    payloadJson.AddMember(wakewords_payload_key, wakeWordsPayload);
                    promise<bool> messagePromise;
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockWakeWordSetting, setAvsChange(std::set<std::string>{newWakeWord}))
                        .WillOnce(InvokeWithoutArgs([&waitEvent] {
                            waitEvent.wakeUp();
                            return true;
                        }));
                    auto avsDirective = createAVSDirective(SET_WAKE_WORDS, true, true, document, payloadJson);
                    shared_ptr<DirectiveHandlerInterface> directiveHandler = m_audioInputProcessor;
                    directiveHandler->handleDirectiveImmediately(avsDirective);
                    ASSERT_TRUE(waitEvent.wait(TEST_TIMEOUT));
                }
                TEST_F(AudioInputProcessorTest, test_publishedCapabiltiesContainsSupportedWakeWords) {
                    unordered_set<shared_ptr<CapabilityConfiguration>> caps = m_audioInputProcessor->getCapabilityConfigurations();
                    auto cap = *caps.begin();
                    auto configuration = cap->additionalConfigurations.find(CAPABILITY_INTERFACE_CONFIGURATIONS_KEY);
                    ASSERT_NE(configuration, cap->additionalConfigurations.end());
                    string wakeWordString;
                    for (auto wakeWord : SUPPORTED_WAKE_WORDS) {
                        if (!wakeWordString.empty()) wakeWordString += ",";
                        wakeWordString += "\"" + wakeWord + "\"";
                    }
                    EXPECT_THAT(configuration->second, MatchesRegex(R"(.*\[)" + wakeWordString + R"(\].*)"));
                }
            }
        }
    }
}