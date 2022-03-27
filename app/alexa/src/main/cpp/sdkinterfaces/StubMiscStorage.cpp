#include <gtest/gtest.h>
#include "Storage/StubMiscStorage.h"
#include "MockAVSConnectionManager.h"
#include "MockAlexaInterfaceMessageSender.h"
#include "MockAVSGatewayAssigner.h"
#include "MockAVSGatewayManager.h"
#include "MockAVSGatewayObserver.h"
#include "MockCapabilitiesDelegate.h"
#include "MockChannelVolumeInterface.h"
#include "MockComponentReporterInterface.h"
#include "MockContextManager.h"
#include "MockDirectiveHandler.h"
#include "MockDirectiveHandlerResult.h"
#include "MockDirectiveSequencer.h"
#include "MockExceptionEncounteredSender.h"
#include "MockFocusManager.h"
#include "MockFocusManagerObserver.h"
#include "MockLocaleAssetsManager.h"
#include "MockPlaybackHandler.h"
#include "MockPlaybackRouter.h"
#include "MockPowerResourceManager.h"
#include "MockRenderPlayerInfoCardsObserverInterface.h"
#include "MockRevokeAuthorizationObserver.h"
#include "MockSpeakerInterface.h"
#include "MockSpeakerManager.h"
#include "MockStateSynchronizerObserver.h"
#include "MockSystemSoundPlayer.h"
#include "MockUserInactivityMonitor.h"
#include "MockUserInactivityMonitorObserver.h"
#include "RenderPlayerInfoCardsProviderInterface.h"
#include "SingleSettingObserverInterface.h"
#include "SpeechInteractionHandlerInterface.h"
#include "SpeechSynthesizerObserverInterface.h"
#include "StateProviderInterface.h"
#include "SystemTimeZoneInterface.h"
#include "TemplateRuntimeObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace storage {
                namespace test {
                    using namespace std;
                    bool StubMiscStorage::createDatabase() { return true; }
                    bool StubMiscStorage::open() {
                        m_isOpened = true;
                        return true;
                    }
                    void StubMiscStorage::close() {
                        m_isOpened = false;
                    }
                    bool StubMiscStorage::createTable(const string& componentName, const string& tableName, MiscStorageInterface::KeyType keyType,
                                                      MiscStorageInterface::ValueType valueType) {
                        string key = componentName + ":" + tableName;
                        m_tables.insert(key);
                        return true;
                    }
                    bool StubMiscStorage::clearTable(const string& componentName, const string& tableName) {
                        string keyPrefix = componentName + ":" + tableName + ":";
                        auto it = m_storage.begin();
                        while (it != m_storage.end()) {
                            const string& key = it->first;
                            size_t pos = key.find(keyPrefix);
                            if (string::npos != pos) it = m_storage.erase(it);
                            else ++it;
                        }
                        return true;
                    }
                    bool StubMiscStorage::deleteTable(const string& componentName, const string& tableName) {
                        string key = componentName + ":" + tableName;
                        m_tables.erase(key);
                        return clearTable(componentName, tableName);
                    }
                    bool StubMiscStorage::get(const string& componentName, const string& tableName, const string& key, string* value) {
                        string keyStr = componentName + ":" + tableName + ":" + key;
                        auto it = m_storage.find(keyStr);
                        if (m_storage.end() == it) return false;
                        *value = it->second;
                        return true;
                    }
                    bool StubMiscStorage::add(const string& componentName, const string& tableName, const string& key, const string& value) {
                        return put(componentName, tableName, key, value);
                    }
                    bool StubMiscStorage::update(const string& componentName, const string& tableName, const string& key, const string& value) {
                        return put(componentName, tableName, key, value);
                    }
                    bool StubMiscStorage::put(const string& componentName, const string& tableName, const string& key, const string& value) {
                        string keyStr = componentName + ":" + tableName + ":" + key;
                        m_storage[keyStr] = value;
                        return true;
                    }
                    bool StubMiscStorage::remove(const string& componentName, const string& tableName, const string& key) {
                        string keyStr = componentName + ":" + tableName + ":" + key;
                        m_storage.erase(keyStr);
                        return true;
                    }
                    bool StubMiscStorage::tableEntryExists(const string& componentName, const string& tableName, const string& key, bool* tableEntryExistsValue) {
                        string keyStr = componentName + ":" + tableName + ":" + key;
                        auto it = m_storage.find(keyStr);
                        *tableEntryExistsValue = m_storage.end() != it;
                        return true;
                    }
                    bool StubMiscStorage::tableExists(const string& componentName, const string& tableName, bool* tableExistsValue) {
                        string key = componentName + ":" + tableName;
                        bool exists = m_tables.end() != m_tables.find(key);
                        *tableExistsValue = exists;
                        return true;
                    }
                    bool StubMiscStorage::load(const string& componentName, const string& tableName, unordered_map<string, string>* valueContainer) {
                        string keyStr = componentName + ":" + tableName + ":";
                        size_t keyLen = keyStr.length();
                        for (const auto& it : m_storage) {
                            const string& key = it.first;
                            if (key.substr(0, keyLen) == keyStr) {
                                string targetKey = key.substr(keyLen);
                                valueContainer->insert(pair<string, string>(targetKey, it.second));
                            }
                        }
                        *valueContainer = m_storage;
                        return true;
                    }
                    shared_ptr<StubMiscStorage> StubMiscStorage::create() {
                        return shared_ptr<StubMiscStorage>(new StubMiscStorage());
                    }
                    StubMiscStorage::StubMiscStorage() : m_isOpened{false} {}
                    bool StubMiscStorage::isOpened() {
                        return m_isOpened;
                    }
                }
            }
        }
    }
}
