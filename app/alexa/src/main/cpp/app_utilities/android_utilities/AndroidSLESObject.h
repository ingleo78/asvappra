#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESOBJECT_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESOBJECT_H_

#include <functional>
#include <memory>
#include <SLES/OpenSLES.h>
#include <logger/Logger.h>
#include <logger/LoggerUtils.h>
#include <memory/Memory.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            class AndroidSLESObject {
            public:
                static std::unique_ptr<AndroidSLESObject> create(SLObjectItf slObject);
                ~AndroidSLESObject();
                bool getInterface(SLInterfaceID interfaceID, void* retObject);
                SLObjectItf get() const;
            private:
                AndroidSLESObject(SLObjectItf slObject);
                SLObjectItf m_object;
            };
        }
    }
}
#endif