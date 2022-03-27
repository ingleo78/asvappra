#include <memory>
#include <sstream>
#include <logger/Logger.h>
#include <util/string/StringUtils.h>
#include "MiscDBEqualizerStorage.h"
#include "EqualizerUtils.h"

namespace alexaClientSDK {
    namespace equalizer {
        using namespace storage;
        using namespace string;
        using namespace logger;
        static const std::string TAG{"MiscDBEqualizerStorage"};
        #define LX(event) LogEntry(TAG, event)
        static const std::string COMPONENT_NAME = "equalizerController";
        static const std::string EQUALIZER_STATE_TABLE = "equalizerState";
        static const std::string EQUALIZER_STATE_KEY = "state";
        void MiscDBEqualizerStorage::saveState(const EqualizerState& state) {
            std::string stateStr = (char*)EqualizerUtils::serializeEqualizerState(state).data();
            if (!m_miscStorage->put(COMPONENT_NAME, EQUALIZER_STATE_TABLE, EQUALIZER_STATE_KEY, stateStr)) {
                ACSDK_ERROR(LX("saveStateFailed").d("reason", "Unable to update the table").d("table", EQUALIZER_STATE_TABLE)
                    .d("component", COMPONENT_NAME));
                ACSDK_DEBUG3(LX(__func__).m("Clearing table"));
                if (!m_miscStorage->clearTable(COMPONENT_NAME, EQUALIZER_STATE_TABLE)) {
                    ACSDK_ERROR(LX("saveStateFailed").d("reason", "Unable to clear the table").d("table", EQUALIZER_STATE_TABLE)
                        .d("component", COMPONENT_NAME).m("Please clear the table for proper future functioning."));
                }
            }
        }
        SuccessResult<EqualizerState> MiscDBEqualizerStorage::loadState() {
            std::string stateString;
            m_miscStorage->get(COMPONENT_NAME, EQUALIZER_STATE_TABLE, EQUALIZER_STATE_KEY, &stateString);
            if (stateString.empty()) return SuccessResult<EqualizerState>::failure();
            return EqualizerUtils::deserializeEqualizerState(stateString.data());
        }
        void MiscDBEqualizerStorage::clear() {
            ACSDK_DEBUG5(LX(__func__));
            bool capabilitiesTableExists = false;
            if (m_miscStorage->tableExists(COMPONENT_NAME, EQUALIZER_STATE_TABLE, &capabilitiesTableExists)) {
                if (capabilitiesTableExists) {
                    if (!m_miscStorage->clearTable(COMPONENT_NAME, EQUALIZER_STATE_TABLE)) {
                        ACSDK_ERROR(LX("clearFailed").d("reason", "Unable to clear the table").d("table", EQUALIZER_STATE_TABLE)
                            .d("component", COMPONENT_NAME).m("Please clear the table for proper future functioning."));
                    } else if (!m_miscStorage->deleteTable(COMPONENT_NAME, EQUALIZER_STATE_TABLE)) {
                        ACSDK_ERROR(LX("clearFailed").d("reason", "Unable to delete the table").d("table", EQUALIZER_STATE_TABLE)
                            .d("component", COMPONENT_NAME).m("Please delete the table for proper future functioning."));
                    }
                }
            } else {
                ACSDK_ERROR(LX("clearFailed").d("reason", "Unable to check if table exists").d("table", EQUALIZER_STATE_TABLE)
                    .d("component", COMPONENT_NAME).m("Please delete the table for proper future functioning."));
            }
        }
        shared_ptr<MiscDBEqualizerStorage> MiscDBEqualizerStorage::create(shared_ptr<MiscStorageInterface> storage) {
            if (nullptr == storage) {
                ACSDK_ERROR(LX("createFailed").d("reason", "storageNull"));
                return nullptr;
            }
            auto equalizerStorage = shared_ptr<MiscDBEqualizerStorage>(new MiscDBEqualizerStorage(storage));
            if (!equalizerStorage->initialize()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "Failed to initialize."));
                return nullptr;
            }
            return equalizerStorage;
        }
        MiscDBEqualizerStorage::MiscDBEqualizerStorage(shared_ptr<MiscStorageInterface> storage) : m_miscStorage{storage} {}
        bool MiscDBEqualizerStorage::initialize() {
            if (!m_miscStorage->isOpened() && !m_miscStorage->open()) {
                ACSDK_DEBUG3(LX(__func__).m("Couldn't open misc database. Creating."));
                if (!m_miscStorage->createDatabase()) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "Could not create misc database."));
                    return false;
                }
            }
            bool tableExists = false;
            if (!m_miscStorage->tableExists(COMPONENT_NAME, EQUALIZER_STATE_TABLE, &tableExists)) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "Could not get Capabilities table information misc database."));
                return false;
            }
            if (!tableExists) {
                ACSDK_DEBUG3(LX(__func__).m("Table doesn't exist in misc database. Creating new."));
                if (!m_miscStorage->createTable(COMPONENT_NAME, EQUALIZER_STATE_TABLE,MiscStorageInterface::KeyType::STRING_KEY,
                    MiscStorageInterface::ValueType::STRING_VALUE)) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "Cannot create table").d("table", EQUALIZER_STATE_TABLE)
                        .d("component", COMPONENT_NAME));
                    return false;
                }
            }
            return true;
        }
    }
}