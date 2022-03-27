#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKSYSTEMSOUNDAUDIOFACTORY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKSYSTEMSOUNDAUDIOFACTORY_H_

#include <gmock/gmock.h>
#include "SystemSoundAudioFactoryInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    using namespace std;
                    using namespace utils;
                    using namespace testing;
                    class MockSystemSoundAudioFactory : public SystemSoundAudioFactoryInterface {
                    public:
                        static shared_ptr<NiceMock<MockSystemSoundAudioFactory>> create();
                        MOCK_CONST_METHOD0(endSpeechTone, function<pair<unique_ptr<istream>, const MediaType>()>());
                        MOCK_CONST_METHOD0(wakeWordNotificationTone, function<pair<unique_ptr<istream>, const MediaType>()>());
                    private:
                        static pair<unique_ptr<istream>, const MediaType>
                        createWakeWordNotificationTone() {
                            return make_pair(unique_ptr<stringstream>(new stringstream("testWakeTone")),MediaType::MPEG);
                        }
                        static pair<unique_ptr<istream>, const MediaType> createEndSpeechTone() {
                            return make_pair(unique_ptr<stringstream>(new stringstream("testEndSpeech")),MediaType::MPEG);
                        }
                    };
                    shared_ptr<NiceMock<MockSystemSoundAudioFactory>> MockSystemSoundAudioFactory::create() {
                        auto result = make_shared<NiceMock<MockSystemSoundAudioFactory>>();
                        ON_CALL(*result.get(), endSpeechTone()).WillByDefault(Return(createEndSpeechTone));
                        ON_CALL(*result.get(), wakeWordNotificationTone()).WillByDefault(Return(createWakeWordNotificationTone));
                        return result;
                    }
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_AUDIO_MOCKSYSTEMSOUNDAUDIOFACTORY_H_
