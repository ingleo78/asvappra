#include "AndroidSLESObject.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            static const std::string TAG{"AndroidSLESObject"};
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            bool AndroidSLESObject::getInterface(SLInterfaceID interfaceID, void* retObject) {
                auto result = (*m_object)->GetInterface(m_object, interfaceID, retObject);
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("getInterfaceFailed").d("result", result));
                    return false;
                }
                return true;
            }
            AndroidSLESObject::AndroidSLESObject(SLObjectItf slObject) : m_object{slObject} {}
            AndroidSLESObject::~AndroidSLESObject() {
                if (m_object) (*m_object)->Destroy(m_object);
            }
            std::unique_ptr<AndroidSLESObject> AndroidSLESObject::create(SLObjectItf object) {
                if (object != nullptr) {
                    auto result = (*object)->Realize(object, SL_BOOLEAN_FALSE);
                    if (result == SL_RESULT_SUCCESS) return std::unique_ptr<AndroidSLESObject>(new AndroidSLESObject(object));
                    else {
                        ACSDK_ERROR(LX("createSlOjectFailed").d("reason", "Failed to realize object.").d("result", result));
                        (*object)->Destroy(object);
                    }
                } else { ACSDK_ERROR(LX("createSlOjectFailed").d("reason", "nullObject")); }
                return nullptr;
            }
            SLObjectItf AndroidSLESObject::get() const {
                return m_object;
            }
        }
    }
}