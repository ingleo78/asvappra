#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "AVSGatewayManagerStorage.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        namespace storage {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::storage;
            using namespace utils;
            using namespace json;
            using namespace jsonUtils;
            using namespace logger;
            static const string TAG("AVSGatewayManagerStorage");
            #define LX(event) LogEntry(TAG, event)
            static const string COMPONENT_NAME = "avsGatewayManager";
            static const string VERIFICATION_STATE_TABLE = "verificationState";
            static const string VERIFICATION_STATE_KEY = "state";
            static const string GATEWAY_URL_KEY = "gatewayURL";
            static const string IS_VERIFIED_KEY = "isVerified";
            unique_ptr<AVSGatewayManagerStorage> AVSGatewayManagerStorage::create(
                shared_ptr<MiscStorageInterface> miscStorage) {
                if (!miscStorage) { ACSDK_ERROR(LX("createFailed").d("reason", "nullMiscStorage")); }
                else return unique_ptr<AVSGatewayManagerStorage>(new AVSGatewayManagerStorage(miscStorage));
                return nullptr;
            }
            AVSGatewayManagerStorage::AVSGatewayManagerStorage(shared_ptr<MiscStorageInterface> miscStorage) : m_miscStorage{miscStorage} {}
            bool AVSGatewayManagerStorage::init() {
                if (!m_miscStorage->isOpened() && !m_miscStorage->open()) {
                    ACSDK_DEBUG3(LX(__func__).m("Couldn't open misc database. Creating."));
                    if (!m_miscStorage->createDatabase()) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "Could not create misc database."));
                        return false;
                    }
                }
                bool tableExists = false;
                if (!m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &tableExists)) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "Could not check state table information in misc database."));
                    return false;
                }
                if (!tableExists) {
                    ACSDK_DEBUG3(LX(__func__).m("Table doesn't exist in misc database. Creating new."));
                    if (!m_miscStorage->createTable(COMPONENT_NAME, VERIFICATION_STATE_TABLE,MiscStorageInterface::KeyType::STRING_KEY,
                        MiscStorageInterface::ValueType::STRING_VALUE)) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "Cannot create table").d("table", VERIFICATION_STATE_TABLE)
                            .d("component", COMPONENT_NAME));
                        return false;
                    }
                }
                return true;
            }
            string convertToStateString(const GatewayVerifyState& state) {
                ACSDK_DEBUG5(LX(__func__));
                JsonGenerator generator;
                generator.addMember(GATEWAY_URL_KEY, state.avsGatewayURL);
                generator.addMember(IS_VERIFIED_KEY, state.isVerified);
                return generator.toString();
            }
            bool convertFromStateString(const string& stateString, GatewayVerifyState* state) {
                if (!state) return false;
                if (!retrieveValue(stateString, GATEWAY_URL_KEY, &state->avsGatewayURL)) return false;
                if (!retrieveValue(stateString, IS_VERIFIED_KEY, &state->isVerified)) return false;
                return true;
            }
            bool AVSGatewayManagerStorage::loadState(GatewayVerifyState* state) {
                if (!state) return false;
                string stateString;
                if (!m_miscStorage->get(COMPONENT_NAME, VERIFICATION_STATE_TABLE, VERIFICATION_STATE_KEY, &stateString)) return false;
                if (!stateString.empty()) return convertFromStateString(stateString, state);
                return true;
            }
            bool AVSGatewayManagerStorage::saveState(const GatewayVerifyState& state) {
                string stateString = convertToStateString(state);
                if (!m_miscStorage->put(COMPONENT_NAME, VERIFICATION_STATE_TABLE, VERIFICATION_STATE_KEY, stateString)) {
                    ACSDK_ERROR(LX("saveStateFailed").d("reason", "Unable to update the table").d("table", VERIFICATION_STATE_TABLE)
                        .d("component", COMPONENT_NAME));
                    return false;
                }
                return true;
            }
            void AVSGatewayManagerStorage::clear() {
                ACSDK_DEBUG5(LX(__func__));
                bool verificationStateTableExists = false;
                if (m_miscStorage->tableExists(COMPONENT_NAME, VERIFICATION_STATE_TABLE, &verificationStateTableExists)) {
                    if (verificationStateTableExists) {
                        if (!m_miscStorage->clearTable(COMPONENT_NAME, VERIFICATION_STATE_TABLE)) {
                            ACSDK_ERROR(LX("clearFailed").d("reason", "Unable to clear the table").d("table", VERIFICATION_STATE_TABLE)
                                .d("component", COMPONENT_NAME).m("Please clear the table for proper future functioning."));
                        } else if (!m_miscStorage->deleteTable(COMPONENT_NAME, VERIFICATION_STATE_TABLE)) {
                            ACSDK_ERROR(LX("clearFailed").d("reason", "Unable to delete the table").d("table", VERIFICATION_STATE_TABLE)
                                .d("component", COMPONENT_NAME).m("Please delete the table for proper future functioning."));
                        }
                    }
                } else {
                    ACSDK_ERROR(LX("clearFailed").d("reason", "Unable to check if table exists").d("table", VERIFICATION_STATE_TABLE)
                        .d("component", COMPONENT_NAME).m("Please delete the table for proper future functioning."));
                }
            }
        }
    }
}