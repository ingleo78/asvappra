#include <fstream>
#include <sstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <sdkinterfaces/Bluetooth/MockBluetoothDevice.h>
#include <sdkinterfaces/Bluetooth/MockBluetoothDeviceConnectionRule.h>
#include <sdkinterfaces/Bluetooth/MockBluetoothDeviceManager.h>
#include <sdkinterfaces/Bluetooth/MockBluetoothHostController.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSinkInterface.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPControllerInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include <sdkinterfaces/Bluetooth/Services/HIDInterface.h>
#include <sdkinterfaces/Bluetooth/Services/MockBluetoothService.h>
#include <sdkinterfaces/Bluetooth/Services/SPPInterface.h>
#include <sdkinterfaces/MockChannelVolumeInterface.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include <util/bluetooth/DeviceCategory.h>
#include <util/bluetooth/SDPRecords.h>
#include <configuration/ConfigurationNode.h>
#include <json/JSONUtils.h>
#include <media_player/MockMediaPlayer.h>
#include <memory/Memory.h>
#include <util/Optional.h>
#include <registration_manager/CustomerDataManager.h>
#include "BasicDeviceConnectionRule.h"
#include "Bluetooth.h"
#include "SQLiteBluetoothStorage.h"
#include "MockBluetoothDeviceObserver.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        namespace test {
            using namespace logger;
            using namespace json;
            using namespace configuration;
            using namespace registrationManager;
            using namespace acsdkBluetoothInterfaces::test;
            using namespace avs::attachment::test;
            using namespace sdkInterfaces::bluetooth::services;
            using namespace sdkInterfaces::bluetooth::test;
            using namespace sdkInterfaces::bluetooth::services::test;
            using namespace sdkInterfaces::test;
            using namespace utils::bluetooth;
            using namespace utils::mediaPlayer;
            using namespace utils::memory;
            using namespace utils::mediaPlayer::test;
            using namespace testing;
            static const string TAG{"BluetoothTest"};
            #define LX(event) LogEntry(TAG, event)
            static const string TEST_BLUETOOTH_DEVICE_MAC = "01:23:45:67:89:ab";
            static const string TEST_BLUETOOTH_FRIENDLY_NAME = "test_friendly_name_1";
            static const string TEST_BLUETOOTH_UUID = "650f973b-c2ab-4c6e-bff4-3788cd521340";
            static const string TEST_BLUETOOTH_DEVICE_MAC_2 = "11:23:45:67:89:ab";
            static const string TEST_BLUETOOTH_FRIENDLY_NAME_2 = "test_friendly_name_2";
            static const string TEST_BLUETOOTH_UUID_2 = "650f973b-c2ab-4c6e-bff4-3788cd521341";
            static const string TEST_BLUETOOTH_DEVICE_MAC_3 = "21:23:45:67:89:ab";
            static const string TEST_BLUETOOTH_FRIENDLY_NAME_3 = "test_friendly_name_3";
            static const string TEST_BLUETOOTH_UUID_3 = "650f973b-c2ab-4c6e-bff4-3788cd521342";
            static const string TEST_DATABASE = "BluetoothCATest.db";
            static const string BLUETOOTH_JSON = R"({"bluetooth" : {"databaseFilePath":")" + TEST_DATABASE + R"("}})";
            static const string FILE_EXISTS_ERROR = "Database File " + TEST_DATABASE + " already exists.";
            static const string NAMESPACE_BLUETOOTH = "Bluetooth";
            static const NamespaceAndName BLUETOOTH_STATE{NAMESPACE_BLUETOOTH, "BluetoothState"};
            static const string MESSAGE_EVENT_KEY = "event";
            static const string MESSAGE_HEADER_KEY = "header";
            static const string MESSAGE_NAME_KEY = "name";
            static const string PAYLOAD_KEY = "payload";
            static const string REQUESTER_KEY = "requester";
            static const string CLOUD_REQUESTER_VALUE = "CLOUD";
            static const string DEVICE_REQUESTER_VALUE = "DEVICE";
            static const string CONNECT_BY_DEVICE_IDS_DIRECTIVE = "ConnectByDeviceIds";
            static const string CONNECT_BY_PROFILE_DIRECTIVE = "ConnectByProfile";
            static const string PAIR_DEVICES_DIRECTIVE = "PairDevices";
            static const string UNPAIR_DEVICES_DIRECTIVE = "UnpairDevices";
            static const string DISCONNECT_DEVICES_DIRECTIVE = "DisconnectDevices";
            static const string SET_DEVICE_CATEGORIES = "SetDeviceCategories";
            static const string TEST_MESSAGE_ID = "MessageId_Test";
            static const string TEST_MESSAGE_ID_2 = "MessageId_Test_2";
            static const string TEST_CONNECT_BY_DEVICE_IDS_PAYLOAD = R"({"devices" : [{"uniqueDeviceId":")" + TEST_BLUETOOTH_UUID + R"(","friendlyName" :")" +
                                                                     TEST_BLUETOOTH_FRIENDLY_NAME + R"("}, {"uniqueDeviceId":")" + TEST_BLUETOOTH_UUID_2 + R"(",
                                                                     "friendlyName" :")" + TEST_BLUETOOTH_FRIENDLY_NAME_2 + R"("}]})";
            static const string CONNECT_BY_DEVICE_IDS_SUCCEEDED = "ConnectByDeviceIdsSucceeded";
            static const string CONNECT_BY_PROFILE_SUCCEEDED = "ConnectByProfileSucceeded";
            static const string CONNECT_BY_PROFILE_FAILED = "ConnectByProfileFailed";
            static const string PAIR_DEVICES_SUCCEEDED = "PairDevicesSucceeded";
            static const string UNPAIR_DEVICES_SUCCEEDED = "UnpairDevicesSucceeded";
            static const string SET_DEVICE_CATEGORIES_SUCCEEDED = "SetDeviceCategoriesSucceeded";
            static const string DISCONNECT_DEVICES_SUCCEEDED = "DisconnectDevicesSucceeded";
            static const string SCAN_DEVICES_REPORT = "ScanDevicesReport";
            static const string STREAMING_STARTED = "StreamingStarted";
            static const string STREAMING_ENDED = "StreamingEnded";
            static const string TEST_UNMATCHED_PROFILE_NAME = "HFP";
            static const string TEST_MATCHED_PROFILE_NAME = "AVRCP";
            static const string TEST_PROFILE_VERSION = "1";
            static const string TEST_CONNECT_BY_PROFILE_PAYLOAD_1 = R"({"profile" : {"name":")" + TEST_UNMATCHED_PROFILE_NAME + R"(","version" :")" +
                                                                    TEST_PROFILE_VERSION + R"("}})";
            static const string TEST_CONNECT_BY_PROFILE_PAYLOAD_2 = R"({"profile" : {"name":")" + TEST_MATCHED_PROFILE_NAME + R"(","version" :")" +
                                                                    TEST_PROFILE_VERSION + R"("}})";
            static const string TEST_PAIR_DEVICES_PAYLOAD = R"({"devices" : [{"uniqueDeviceId":")" + TEST_BLUETOOTH_UUID + R"("}, {"uniqueDeviceId":")" +
                                                            TEST_BLUETOOTH_UUID_2 + R"("}]})";
            static const string TEST_UNPAIR_DEVICES_PAYLOAD = R"({"devices" : [{"uniqueDeviceId":")" + TEST_BLUETOOTH_UUID + R"("}, {"uniqueDeviceId":")" +
                                                              TEST_BLUETOOTH_UUID_2 + R"("}]})";
            static const string TEST_DISCONNECT_DEVICES_PAYLOAD = R"({"devices" : [{"uniqueDeviceId":")" + TEST_BLUETOOTH_UUID + R"("}, {"uniqueDeviceId":")" +
                                                                  TEST_BLUETOOTH_UUID_2 + R"("}]})";
            static const string TEST_SET_DEVICE_CATEGORIES_PAYLOAD = R"({"devices" : [{"uniqueDeviceId":")" + TEST_BLUETOOTH_UUID + R"(","deviceCategory":
                                                                     "PHONE"}, {"uniqueDeviceId":")" + TEST_BLUETOOTH_UUID_2 + R"(","deviceCategory": "GADGET"}]})";
            static const string MOCK_CONTEXT = R"({"context": [{"header": {"namespace": "Bluetooth","name": "BluetoothState"},"payload": {"alexaDevice": {
                                               "friendlyName": "{{STRING}}"},"pairedDevices": [{"uniqueDeviceId": "{{STRING}}","friendlyName": "{{STRING}}",
                                               "supportedProfiles": [{"name": "{{STRING}}","version": "{{STRING}}"}]}],"activeDevice": {"uniqueDeviceId":
                                               "{{STRING}}","friendlyName": "{{STRING}}","supportedProfiles": [{"name": "{{STRING}}","version": "{{STRING}}"}],
                                               "streaming": "{{STRING}}"}}}]})";
            static milliseconds WAIT_TIMEOUT_MS(1000);
            static milliseconds EVENT_PROCESS_DELAY_MS(500);
            static bool fileExists(const string& file) {
                ifstream dbFile(file);
                return dbFile.good();
            }
            class BluetoothTest: public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                shared_ptr<Bluetooth> m_Bluetooth;
                shared_ptr<MockContextManager> m_mockContextManager;
                shared_ptr<MockFocusManager> m_mockFocusManager;
                shared_ptr<MockMessageSender> m_mockMessageSender;
                shared_ptr<MockExceptionEncounteredSender> m_mockExceptionSender;
                shared_ptr<SQLiteBluetoothStorage> m_bluetoothStorage;
                shared_ptr<MockMediaPlayer> m_mockBluetoothMediaPlayer;
                unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> m_mockEnabledConnectionRules;
                shared_ptr<BluetoothEventBus> m_eventBus;
                shared_ptr<registrationManager::CustomerDataManager> m_customerDataManager;
                shared_ptr<MockBluetoothHostController> m_mockBluetoothHostController;
                list<shared_ptr<BluetoothDeviceInterface>> m_mockDiscoveredBluetoothDevices;
                shared_ptr<MockBluetoothDevice> m_mockBluetoothDevice1;
                shared_ptr<MockBluetoothDevice> m_mockBluetoothDevice2;
                shared_ptr<MockBluetoothDevice> m_mockBluetoothDevice3;
                shared_ptr<MockBluetoothDeviceConnectionRule> m_remoteControlConnectionRule;
                shared_ptr<MockBluetoothDeviceConnectionRule> m_gadgetConnectionRule;
                unique_ptr<MockBluetoothDeviceManager> m_mockDeviceManager;
                unique_ptr<MockDirectiveHandlerResult> m_mockDirectiveHandlerResult;
                shared_ptr<MockBluetoothDeviceObserver> m_mockBluetoothDeviceObserver;
                shared_ptr<MockChannelVolumeInterface> m_mockChannelVolumeInterface;
                condition_variable m_messageSentTrigger;
                map<string, int> m_messages;
                void wakeOnSetCompleted();
                string getRequestName(shared_ptr<MessageRequest> request);
                bool verifyMessage(shared_ptr<MessageRequest> request, string expectedName);
                bool verifyMessagesSentInOrder(vector<string> orderedEvents, function<void()> trigger);
                void verifyMessagesCount(shared_ptr<MessageRequest> request, map<string, int>* messages);
                BluetoothTest() : m_wakeSetCompletedPromise{}, m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {}
            protected:
                promise<void> m_wakeSetCompletedPromise;
                future<void> m_wakeSetCompletedFuture;
            };
            void BluetoothTest::SetUp() {
                //m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                m_mockFocusManager = make_shared<NiceMock<MockFocusManager>>();
                m_mockMessageSender = make_shared<NiceMock<MockMessageSender>>();
                m_mockExceptionSender = make_shared<NiceMock<MockExceptionEncounteredSender>>();
                m_eventBus = make_shared<BluetoothEventBus>();
                m_mockBluetoothHostController = make_shared<NiceMock<MockBluetoothHostController>>();
                m_mockDirectiveHandlerResult = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
                m_mockBluetoothDeviceObserver = make_shared<NiceMock<MockBluetoothDeviceObserver>>();
                m_mockBluetoothMediaPlayer = MockMediaPlayer::create();
                m_customerDataManager = std::make_shared<CustomerDataManager>();
                auto metaData =MockBluetoothDevice::MetaData(Optional<int>(),Optional<int>(), MockBluetoothDevice::MetaData::UNDEFINED_CLASS_VALUE,
                                             Optional<int>(), Optional<string>());
                auto a2dpSink = make_shared<NiceMock<MockBluetoothService>>(make_shared<A2DPSinkRecord>(""));
                auto avrcpTarget = make_shared<NiceMock<MockBluetoothService>>(make_shared<AVRCPTargetRecord>(""));
                vector<shared_ptr<BluetoothServiceInterface>> services = {a2dpSink, avrcpTarget};
                m_mockBluetoothDevice1 = make_shared<NiceMock<MockBluetoothDevice>>(TEST_BLUETOOTH_DEVICE_MAC, TEST_BLUETOOTH_FRIENDLY_NAME, metaData, services);
                m_mockDiscoveredBluetoothDevices.push_back(m_mockBluetoothDevice1);
                auto metaData2 =MockBluetoothDevice::MetaData(Optional<int>(), Optional<int>(), MockBluetoothDevice::MetaData::UNDEFINED_CLASS_VALUE,
                                              Optional<int>(), Optional<string>());
                auto hid = make_shared<NiceMock<MockBluetoothService>>(make_shared<HIDRecord>(""));
                auto spp = make_shared<NiceMock<MockBluetoothService>>(make_shared<SPPRecord>(""));
                auto a2dpSource = make_shared<NiceMock<MockBluetoothService>>(make_shared<A2DPSourceRecord>(""));
                vector<shared_ptr<BluetoothServiceInterface>> services2 = {spp, hid, a2dpSource};
                m_mockBluetoothDevice2 = std::make_shared<NiceMock<MockBluetoothDevice>>(TEST_BLUETOOTH_DEVICE_MAC_2, TEST_BLUETOOTH_FRIENDLY_NAME_2, metaData2, services2);
                m_mockDiscoveredBluetoothDevices.push_back(m_mockBluetoothDevice2);
                auto metaData3 =MockBluetoothDevice::MetaData(Optional<int>(), Optional<int>(),
                        MockBluetoothDevice::MetaData::UNDEFINED_CLASS_VALUE, Optional<int>(), Optional<std::string>());
                vector<shared_ptr<BluetoothServiceInterface>> services3 = {a2dpSink};
                m_mockBluetoothDevice3 = make_shared<NiceMock<MockBluetoothDevice>>(TEST_BLUETOOTH_DEVICE_MAC_3, TEST_BLUETOOTH_FRIENDLY_NAME_3, metaData3, services3);
                m_mockDiscoveredBluetoothDevices.push_back(m_mockBluetoothDevice3);
                set<DeviceCategory> remoteCategory{DeviceCategory::REMOTE_CONTROL};
                set<string> remoteDependentProfiles{HIDInterface::UUID, SPPInterface::UUID};
                m_remoteControlConnectionRule = make_shared<NiceMock<MockBluetoothDeviceConnectionRule>>(remoteCategory, remoteDependentProfiles);
                set<DeviceCategory> gadgetCategory{DeviceCategory::GADGET};
                set<string> gadgetDependentProfiles{HIDInterface::UUID, SPPInterface::UUID};
                m_gadgetConnectionRule = make_shared<NiceMock<MockBluetoothDeviceConnectionRule>>(gadgetCategory, gadgetDependentProfiles);
                m_gadgetConnectionRule->setExplicitlyConnect(false);
                m_gadgetConnectionRule->setExplicitlyDisconnect(true);
                m_remoteControlConnectionRule->setExplicitlyConnect(false);
                m_remoteControlConnectionRule->setExplicitlyDisconnect(false);
                m_mockEnabledConnectionRules = {m_remoteControlConnectionRule, m_gadgetConnectionRule, BasicDeviceConnectionRule::create()};
                m_mockChannelVolumeInterface = make_shared<MockChannelVolumeInterface>();
                m_mockChannelVolumeInterface->DelegateToReal();
                if (fileExists(TEST_DATABASE)) {
                    ADD_FAILURE() << FILE_EXISTS_ERROR;
                    exit(1);
                }
                auto json = shared_ptr<std::stringstream>(new stringstream());
                *json << BLUETOOTH_JSON;
                vector<shared_ptr<std::istream>> jsonStream;
                jsonStream.push_back(json);
                ConfigurationNode::initialize(jsonStream);
                m_bluetoothStorage = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_TRUE(m_bluetoothStorage->createDatabase());
                m_bluetoothStorage->insertByMac(TEST_BLUETOOTH_DEVICE_MAC, TEST_BLUETOOTH_UUID, true);
                m_bluetoothStorage->insertByMac(TEST_BLUETOOTH_DEVICE_MAC_2, TEST_BLUETOOTH_UUID_2, true);
                m_bluetoothStorage->insertByMac(TEST_BLUETOOTH_DEVICE_MAC_3, TEST_BLUETOOTH_UUID_3, true);
                m_bluetoothStorage->close();
                m_Bluetooth = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,m_mockExceptionSender,m_bluetoothStorage,
                                                memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(m_mockBluetoothHostController,
                                                m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,m_mockBluetoothMediaPlayer,m_customerDataManager,
                                                m_mockEnabledConnectionRules,m_mockChannelVolumeInterface,nullptr);
                ASSERT_THAT(m_Bluetooth, NotNull());
                m_Bluetooth->addObserver(m_mockBluetoothDeviceObserver);
            }
            void BluetoothTest::TearDown() {
                if (m_Bluetooth) m_Bluetooth->shutdown();
                m_mockBluetoothMediaPlayer->shutdown();
                if (fileExists(TEST_DATABASE)) remove(TEST_DATABASE.c_str());
            }
            void BluetoothTest::wakeOnSetCompleted() {
                m_wakeSetCompletedPromise.set_value();
            }
            string BluetoothTest::getRequestName(shared_ptr<MessageRequest> request) {
                Document document;
                document.Parse(request->getJsonContent().c_str());
                EXPECT_FALSE(document.HasParseError());
                auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                EXPECT_NE(event, document.MemberEnd());
                auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                EXPECT_NE(header, event->value.MemberEnd());
                auto payload = event->value.FindMember(PAYLOAD_KEY.data());
                EXPECT_NE(payload, event->value.MemberEnd());
                string requestName;
                rapidjson::Value value{header->value.GetString(), strlen(header->value.GetString())};
                jsonUtils::retrieveValue1(value, MESSAGE_NAME_KEY.data(), &requestName);
                return requestName;
            }
            bool BluetoothTest::verifyMessage(shared_ptr<MessageRequest> request, string expectedName) {
                return getRequestName(request) == expectedName;
            }
            bool BluetoothTest::verifyMessagesSentInOrder(vector<string> orderedEvents, function<void()> trigger) {
                size_t curIndex = 0;
                mutex waitMutex;
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this, orderedEvents, &curIndex](shared_ptr<MessageRequest> request) {
                        if (curIndex < orderedEvents.size()) {
                            if (verifyMessage(request, orderedEvents.at(curIndex))) {
                                if (curIndex < orderedEvents.size()) curIndex++;
                            }
                        }
                        m_messageSentTrigger.notify_one();
                    }));
                trigger();
                bool result;
                {
                    unique_lock<mutex> lock(waitMutex);
                    result = m_messageSentTrigger.wait_for(lock, WAIT_TIMEOUT_MS, [orderedEvents, &curIndex] {
                        if (curIndex == orderedEvents.size()) return true;
                        return false;
                    });
                }
                return result;
            }
            void BluetoothTest::verifyMessagesCount(shared_ptr<MessageRequest> request, map<string, int>* messages) {
                string requestName = getRequestName(request);
                if (messages->find(requestName) != messages->end()) messages->at(requestName) += 1;
            }
            TEST_F(BluetoothTest, test_createBTWithNullParams) {
                auto bluetooth1 = Bluetooth::create(nullptr,m_mockFocusManager,m_mockMessageSender, m_mockExceptionSender,m_bluetoothStorage,
                                                    memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(m_mockBluetoothHostController,
                                                    m_mockDiscoveredBluetoothDevices, m_eventBus), m_eventBus, m_mockBluetoothMediaPlayer,
                                                    m_customerDataManager, m_mockEnabledConnectionRules, m_mockChannelVolumeInterface, nullptr);
                EXPECT_THAT(bluetooth1, IsNull());
                auto bluetooth2 = Bluetooth::create(m_mockContextManager, nullptr, m_mockMessageSender, m_mockExceptionSender, m_bluetoothStorage,
                                                    memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(m_mockBluetoothHostController,
                                                    m_mockDiscoveredBluetoothDevices, m_eventBus), m_eventBus, m_mockBluetoothMediaPlayer, m_customerDataManager,
                                                    m_mockEnabledConnectionRules, m_mockChannelVolumeInterface, nullptr);
                EXPECT_THAT(bluetooth2, IsNull());
                auto bluetooth3 = Bluetooth::create(m_mockContextManager,m_mockFocusManager,nullptr,m_mockExceptionSender,
                                     m_bluetoothStorage,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(m_mockBluetoothHostController,
                                                    m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,m_mockBluetoothMediaPlayer,
                                 m_customerDataManager,m_mockEnabledConnectionRules,m_mockChannelVolumeInterface,
                                 nullptr);
                EXPECT_THAT(bluetooth3, IsNull());
                auto bluetooth4 = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                             nullptr,m_bluetoothStorage,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(m_mockBluetoothHostController,
                                                    m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,m_mockBluetoothMediaPlayer,
                                 m_customerDataManager,m_mockEnabledConnectionRules,m_mockChannelVolumeInterface,
                                 nullptr);
                EXPECT_THAT(bluetooth4, IsNull());
                auto bluetooth5 = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                             m_mockExceptionSender,nullptr,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(
                                                    m_mockBluetoothHostController, m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,
                                         m_mockBluetoothMediaPlayer,m_customerDataManager,m_mockEnabledConnectionRules,
                         m_mockChannelVolumeInterface,nullptr);
                EXPECT_THAT(bluetooth5, IsNull());
                auto bluetooth6 = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                             m_mockExceptionSender,m_bluetoothStorage,nullptr,m_eventBus,
                                         m_mockBluetoothMediaPlayer,m_customerDataManager,m_mockEnabledConnectionRules,
                         m_mockChannelVolumeInterface,nullptr);
                EXPECT_THAT(bluetooth6, IsNull());
                auto bluetooth7 = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                             m_mockExceptionSender,m_bluetoothStorage,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(
                                                    m_mockBluetoothHostController, m_mockDiscoveredBluetoothDevices, m_eventBus),nullptr,
                                         m_mockBluetoothMediaPlayer,m_customerDataManager,m_mockEnabledConnectionRules,
                         m_mockChannelVolumeInterface,nullptr);
                EXPECT_THAT(bluetooth7, IsNull());
                auto bluetooth8 = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                             m_mockExceptionSender,m_bluetoothStorage,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(
                                                    m_mockBluetoothHostController, m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,nullptr,
                                 m_customerDataManager,m_mockEnabledConnectionRules,m_mockChannelVolumeInterface,
                                 nullptr);
                EXPECT_THAT(bluetooth8, IsNull());
                auto bluetooth9 = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                             m_mockExceptionSender,m_bluetoothStorage,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(
                                                    m_mockBluetoothHostController, m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,
                                         m_mockBluetoothMediaPlayer,nullptr,m_mockEnabledConnectionRules,
                         m_mockChannelVolumeInterface,nullptr);
                EXPECT_THAT(bluetooth9, IsNull());
            }
            TEST_F(BluetoothTest, test_createBTWithDuplicateDeviceCategoriesInConnectionRules) {
                set<DeviceCategory> categories1{DeviceCategory::REMOTE_CONTROL};
                set<DeviceCategory> categories2{DeviceCategory::REMOTE_CONTROL, DeviceCategory::GADGET};
                set<string> dependentProfiles{HIDInterface::UUID, SPPInterface::UUID};
                auto mockDeviceConnectionRule1 = make_shared<MockBluetoothDeviceConnectionRule>(categories1, dependentProfiles);
                auto mockDeviceConnectionRule2 = make_shared<MockBluetoothDeviceConnectionRule>(categories2, dependentProfiles);
                unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledRules = {mockDeviceConnectionRule1, mockDeviceConnectionRule2};
                auto bluetooth = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                            m_mockExceptionSender,m_bluetoothStorage,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(
                                                   m_mockBluetoothHostController, m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,
                                        m_mockBluetoothMediaPlayer,m_customerDataManager, enabledRules,m_mockChannelVolumeInterface);
                ASSERT_THAT(bluetooth, IsNull());
            }
            TEST_F(BluetoothTest, test_createBTWithLackOfProfilesInConnectionRules) {
                set<DeviceCategory> categories{DeviceCategory::REMOTE_CONTROL};
                set<string> dependentProfiles{HIDInterface::UUID};
                auto mockDeviceConnectionRule = make_shared<MockBluetoothDeviceConnectionRule>(categories, dependentProfiles);
                unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledRules = {mockDeviceConnectionRule};
                auto bluetooth = Bluetooth::create(m_mockContextManager,m_mockFocusManager,m_mockMessageSender,
                            m_mockExceptionSender,m_bluetoothStorage,memory::make_unique<NiceMock<MockBluetoothDeviceManager>>(
                                                   m_mockBluetoothHostController, m_mockDiscoveredBluetoothDevices, m_eventBus),m_eventBus,
                                        m_mockBluetoothMediaPlayer,m_customerDataManager, enabledRules,m_mockChannelVolumeInterface);
                ASSERT_THAT(bluetooth, IsNull());
            }
            TEST_F(BluetoothTest, test_handleConnectByDeviceIdsDirectiveWithTwoA2DPDevices) {
                EXPECT_CALL(*m_mockBluetoothDeviceObserver, onActiveDeviceConnected(_)).Times(Exactly(2));
                EXPECT_CALL(*m_mockBluetoothDeviceObserver, onActiveDeviceDisconnected(_)).Times(Exactly(1));
                EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1).WillOnce(InvokeWithoutArgs(this, &BluetoothTest::wakeOnSetCompleted));
                EXPECT_CALL(*m_mockContextManager, setState(BLUETOOTH_STATE, _, StateRefreshPolicy::NEVER, _)).Times(Exactly(3));
                vector<string> events = {CONNECT_BY_DEVICE_IDS_SUCCEEDED, CONNECT_BY_DEVICE_IDS_SUCCEEDED, DISCONNECT_DEVICES_SUCCEEDED};
                ASSERT_TRUE(verifyMessagesSentInOrder(events, [this]() {
                    //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, CONNECT_BY_DEVICE_IDS_DIRECTIVE, TEST_MESSAGE_ID);
                    //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_CONNECT_BY_DEVICE_IDS_PAYLOAD, attachmentManager, "");
                    shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                    //agentAsDirectiveHandler->preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                    this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY_MS));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::CONNECTED));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::CONNECTED));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::DISCONNECTED));
                }));
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                ASSERT_FALSE(m_mockBluetoothDevice2->isConnected());
            }
            TEST_F(BluetoothTest, test_handleConnectByDeviceIdsDirectiveWithOnePhoneOneGadget) {
                m_mockBluetoothDevice1->disconnect();
                m_mockBluetoothDevice2->disconnect();
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::PHONE).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::GADGET).data());
                EXPECT_CALL(*m_mockBluetoothDeviceObserver, onActiveDeviceConnected(_)).Times(Exactly(2));
                EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1)
                    .WillOnce(InvokeWithoutArgs(this, &BluetoothTest::wakeOnSetCompleted));
                EXPECT_CALL(*m_mockContextManager, setState(BLUETOOTH_STATE, _, StateRefreshPolicy::NEVER, _)).Times(Exactly(2));
                vector<string> events = {CONNECT_BY_DEVICE_IDS_SUCCEEDED, CONNECT_BY_DEVICE_IDS_SUCCEEDED};
                ASSERT_TRUE(verifyMessagesSentInOrder(events, [this]() {
                    //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, CONNECT_BY_DEVICE_IDS_DIRECTIVE, TEST_MESSAGE_ID);
                    //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_CONNECT_BY_DEVICE_IDS_PAYLOAD, attachmentManager, "");
                    shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                    //agentAsDirectiveHandler->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                    this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY_MS));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::CONNECTED));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::CONNECTED));
                }));
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                ASSERT_TRUE(m_mockBluetoothDevice2->isConnected());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
            }
            TEST_F(BluetoothTest, test_handleConnectByProfileWithUnmatchedProfileName) {
                m_mockBluetoothDevice1->pair();
                m_mockBluetoothDevice2->pair();
                EXPECT_CALL(*m_mockBluetoothDeviceObserver, onActiveDeviceConnected(_)).Times(Exactly(0));
                EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1).WillOnce(InvokeWithoutArgs(this, &BluetoothTest::wakeOnSetCompleted));
                EXPECT_CALL(*m_mockContextManager, setState(BLUETOOTH_STATE, _, StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                vector<string> events = {CONNECT_BY_PROFILE_FAILED};
                ASSERT_TRUE(verifyMessagesSentInOrder(events, [this]() {
                    //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, CONNECT_BY_PROFILE_DIRECTIVE, TEST_MESSAGE_ID);
                    //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_CONNECT_BY_PROFILE_PAYLOAD_1, attachmentManager, "");
                    shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                    //agentAsDirectiveHandler->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                }));
                ASSERT_FALSE(m_mockBluetoothDevice1->isConnected());
                ASSERT_FALSE(m_mockBluetoothDevice2->isConnected());
            }
            TEST_F(BluetoothTest, test_handleConnectByProfileWithMatchedProfileName) {
                mutex waitMutex;
                unique_lock<mutex> waitLock(waitMutex);
                m_mockBluetoothDevice1->pair();
                m_mockBluetoothDevice2->pair();
                EXPECT_CALL(*m_mockBluetoothDeviceObserver, onActiveDeviceConnected(_)).Times(Exactly(1));
                EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1).WillOnce(InvokeWithoutArgs(this, &BluetoothTest::wakeOnSetCompleted));
                vector<string> events = {CONNECT_BY_PROFILE_SUCCEEDED};
                ASSERT_TRUE(verifyMessagesSentInOrder(events, [this]() {
                    //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, CONNECT_BY_PROFILE_DIRECTIVE, TEST_MESSAGE_ID);
                    //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_CONNECT_BY_PROFILE_PAYLOAD_2, attachmentManager, "");
                    shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                    //agentAsDirectiveHandler->preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                    std::this_thread::sleep_for(std::chrono::milliseconds(EVENT_PROCESS_DELAY_MS));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::CONNECTED));
                }));
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                ASSERT_FALSE(m_mockBluetoothDevice2->isConnected());
            }
            TEST_F(BluetoothTest, DISABLED_test_handlePairDevices) {
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::PHONE).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::GADGET).data());
                m_messages.insert({PAIR_DEVICES_SUCCEEDED, 0});
                m_messages.insert({CONNECT_BY_DEVICE_IDS_SUCCEEDED, 0});
                std::mutex waitMutex;
                EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1).WillOnce(InvokeWithoutArgs(this, &BluetoothTest::wakeOnSetCompleted));
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](std::shared_ptr<MessageRequest> request) {
                        verifyMessagesCount(request, &m_messages);
                        m_messageSentTrigger.notify_one();
                    }));
                //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, PAIR_DEVICES_DIRECTIVE, TEST_MESSAGE_ID);
                //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_PAIR_DEVICES_PAYLOAD, attachmentManager, "");
                shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                //agentAsDirectiveHandler->preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY_MS));
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::PAIRED));
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::CONNECTED));
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::PAIRED));
                unique_lock<mutex> lock(waitMutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, WAIT_TIMEOUT_MS, [this] {
                   for(auto message : m_messages) {
                       if (message.first == PAIR_DEVICES_SUCCEEDED) {
                           if (message.second != 2) return false;
                       } else if (message.first == CONNECT_BY_DEVICE_IDS_SUCCEEDED) {
                           if (message.second != 1) return false;
                       }
                   }
                   return true;
                });
                ASSERT_TRUE(result);
                ASSERT_TRUE(m_mockBluetoothDevice1->isPaired());
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                ASSERT_TRUE(m_mockBluetoothDevice2->isPaired());
                ASSERT_FALSE(m_mockBluetoothDevice2->isConnected());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
            }
            TEST_F(BluetoothTest, test_handleUnpairDevices) {
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::PHONE).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::GADGET).data());
                m_mockBluetoothDevice1->pair();
                m_mockBluetoothDevice1->connect();
                m_mockBluetoothDevice2->pair();
                m_mockBluetoothDevice2->connect();
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                ASSERT_TRUE(m_mockBluetoothDevice2->isConnected());
                EXPECT_CALL(*m_mockBluetoothDeviceObserver, onActiveDeviceDisconnected(_)).Times(Exactly(2));
                EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1).WillOnce(InvokeWithoutArgs(this, &BluetoothTest::wakeOnSetCompleted));
                vector<string> events = {DISCONNECT_DEVICES_SUCCEEDED, UNPAIR_DEVICES_SUCCEEDED, DISCONNECT_DEVICES_SUCCEEDED, UNPAIR_DEVICES_SUCCEEDED};
                ASSERT_TRUE(verifyMessagesSentInOrder(events, [this]() {
                    //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, UNPAIR_DEVICES_DIRECTIVE, TEST_MESSAGE_ID);
                    //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_UNPAIR_DEVICES_PAYLOAD, attachmentManager, "");
                    shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                    //agentAsDirectiveHandler->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                    this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY_MS));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::DISCONNECTED));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::UNPAIRED));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::DISCONNECTED));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::UNPAIRED));
                }));
                ASSERT_FALSE(m_mockBluetoothDevice1->isPaired());
                ASSERT_FALSE(m_mockBluetoothDevice1->isConnected());
                ASSERT_FALSE(m_mockBluetoothDevice2->isPaired());
                ASSERT_FALSE(m_mockBluetoothDevice2->isConnected());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
            }
            TEST_F(BluetoothTest, test_handleDisconnectDevices) {
                m_mockBluetoothDevice1->pair();
                m_mockBluetoothDevice1->connect();
                m_mockBluetoothDevice2->pair();
                m_mockBluetoothDevice2->connect();
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                ASSERT_TRUE(m_mockBluetoothDevice2->isConnected());
                EXPECT_CALL(*m_mockBluetoothDeviceObserver, onActiveDeviceDisconnected(_)).Times(Exactly(2));
                vector<string> events = {DISCONNECT_DEVICES_SUCCEEDED, DISCONNECT_DEVICES_SUCCEEDED};
                ASSERT_TRUE(verifyMessagesSentInOrder(events, [this]() {
                    //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader= make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, DISCONNECT_DEVICES_DIRECTIVE, TEST_MESSAGE_ID);
                    //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_DISCONNECT_DEVICES_PAYLOAD, attachmentManager, "");
                    shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                    //agentAsDirectiveHandler->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                    m_wakeSetCompletedPromise = promise<void>();
                    m_wakeSetCompletedFuture = m_wakeSetCompletedPromise.get_future();
                    m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                    this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY_MS));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::DISCONNECTED));
                    m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::DISCONNECTED));
                }));
                ASSERT_FALSE(m_mockBluetoothDevice1->isConnected());
                ASSERT_FALSE(m_mockBluetoothDevice2->isConnected());
            }
            TEST_F(BluetoothTest, test_handleSetDeviceCategories) {
                vector<string> events = {SET_DEVICE_CATEGORIES_SUCCEEDED};
                ASSERT_TRUE(verifyMessagesSentInOrder(events, [this]() {
                    //auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader= make_shared<AVSMessageHeader>(NAMESPACE_BLUETOOTH, SET_DEVICE_CATEGORIES, TEST_MESSAGE_ID);
                    //shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEST_SET_DEVICE_CATEGORIES_PAYLOAD, attachmentManager, "");
                    shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_Bluetooth;
                    //agentAsDirectiveHandler->preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    agentAsDirectiveHandler->handleDirective(TEST_MESSAGE_ID);
                    m_wakeSetCompletedPromise = promise<void>();
                    m_wakeSetCompletedFuture = m_wakeSetCompletedPromise.get_future();
                    m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                    m_Bluetooth->onContextAvailable(MOCK_CONTEXT);
                }));
                string category1;
                string category2;
                m_bluetoothStorage->getCategory(TEST_BLUETOOTH_UUID, &category1);
                m_bluetoothStorage->getCategory(TEST_BLUETOOTH_UUID_2, &category2);
                ASSERT_EQ(deviceCategoryToString(DeviceCategory::PHONE), category1);
                ASSERT_EQ(deviceCategoryToString(DeviceCategory::GADGET), category2);
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
            }
            TEST_F(BluetoothTest, test_contentDucksUponReceivingBackgroundFocus) {
                m_mockBluetoothDevice1->pair();
                m_mockBluetoothDevice1->connect();
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::CONNECTED));
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                m_eventBus->sendEvent(MediaStreamingStateChangedEvent(MediaStreamingState::ACTIVE, A2DPRole::SOURCE, m_mockBluetoothDevice1));
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                EXPECT_CALL(*m_mockBluetoothMediaPlayer, stop(_)).Times(0);
                EXPECT_CALL(*m_mockChannelVolumeInterface, startDucking()).Times(1);
                m_Bluetooth->onFocusChanged(FocusState::BACKGROUND,MixingBehavior::MAY_DUCK);
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
            }
            TEST_F(BluetoothTest, test_contentUnducksUponReceivingForegroundOrNoneFocus) {
                m_mockBluetoothDevice1->pair();
                m_mockBluetoothDevice1->connect();
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::CONNECTED));
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                ASSERT_TRUE(m_mockBluetoothDevice1->isConnected());
                m_eventBus->sendEvent(MediaStreamingStateChangedEvent(MediaStreamingState::ACTIVE,A2DPRole::SOURCE, m_mockBluetoothDevice1));
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                EXPECT_CALL(*m_mockBluetoothMediaPlayer, stop(_)).Times(0);
                EXPECT_CALL(*m_mockChannelVolumeInterface, startDucking()).Times(1);
                m_Bluetooth->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                EXPECT_CALL(*m_mockChannelVolumeInterface, stopDucking()).Times(1);
                m_Bluetooth->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                EXPECT_CALL(*m_mockChannelVolumeInterface, stopDucking()).Times(1);
                m_Bluetooth->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
            }
            TEST_F(BluetoothTest, test_streamingStateChange) {
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::AUDIO_VIDEO).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::PHONE).data());
                EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(AtLeast(1)).WillOnce(Invoke([this](shared_ptr<MessageRequest> request) {
                    verifyMessage(request, STREAMING_STARTED);
                })).WillOnce(Invoke([this](shared_ptr<MessageRequest> request) {
                    verifyMessage(request, STREAMING_ENDED);
                }));
                m_mockBluetoothDevice1->connect();
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice1, DeviceState::CONNECTED));
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY_MS));
                m_eventBus->sendEvent(MediaStreamingStateChangedEvent(MediaStreamingState::ACTIVE, A2DPRole::SOURCE, m_mockBluetoothDevice1));
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY_MS));
                m_mockBluetoothDevice2->connect();
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice2, DeviceState::CONNECTED));
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_2, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
            }
            TEST_F(BluetoothTest, test_focusStateChange) {
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_3, deviceCategoryToString(DeviceCategory::PHONE).data());
                //EXPECT_CALL(*m_mockFocusManager, acquireChannel(_, _)).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockBluetoothMediaPlayer, play(_)).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(0);
                EXPECT_CALL(*m_mockBluetoothMediaPlayer, stop(_)).Times(1).WillOnce(Return(true));
                m_mockBluetoothDevice3->connect();
                m_eventBus->sendEvent(DeviceStateChangedEvent(m_mockBluetoothDevice3, DeviceState::CONNECTED));
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                m_eventBus->sendEvent(MediaStreamingStateChangedEvent(MediaStreamingState::ACTIVE, A2DPRole::SINK, m_mockBluetoothDevice3));
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                m_Bluetooth->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                m_Bluetooth->onPlaybackStarted(m_mockBluetoothMediaPlayer->getCurrentSourceId(), {std::chrono::milliseconds(0)});
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                m_Bluetooth->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_STOP);
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                m_eventBus->sendEvent(MediaStreamingStateChangedEvent(MediaStreamingState::IDLE, A2DPRole::SINK, m_mockBluetoothDevice3));
                m_wakeSetCompletedFuture.wait_for(WAIT_TIMEOUT_MS);
                m_bluetoothStorage->updateByCategory(TEST_BLUETOOTH_UUID_3, deviceCategoryToString(DeviceCategory::UNKNOWN).data());
            }
        }
    }
}