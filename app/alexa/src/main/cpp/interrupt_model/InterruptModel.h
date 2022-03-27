#ifndef ALEXA_CLIENT_SDK_INTERRUPTMODEL_INCLUDE_INTERRUPTMODEL_INTERRUPTMODEL_H_
#define ALEXA_CLIENT_SDK_INTERRUPTMODEL_INCLUDE_INTERRUPTMODEL_INTERRUPTMODEL_H_

#include <map>
#include <memory>
#include <avs/ContentType.h>
#include <avs/MixingBehavior.h>
#include <configuration/ConfigurationNode.h>

namespace alexaClientSDK {
    namespace afml {
        namespace interruptModel {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace utils;
            using namespace configuration;
            class InterruptModel {
            public:
                static shared_ptr<InterruptModel> create(ConfigurationNode interactionConfiguration);
                MixingBehavior getMixingBehavior(const string& lowPriorityChannel, ContentType lowPriorityContentType, const string& highPriorityChannel,
                                                 ContentType highPriorityContentType) const;
            private:
                InterruptModel(ConfigurationNode interactionConfiguration);
                ConfigurationNode m_interactionConfiguration;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_INTERRUPTMODEL_INCLUDE_INTERRUPTMODEL_INTERRUPTMODEL_H_
