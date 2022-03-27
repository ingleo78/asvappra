#ifndef ALEXA_CLIENT_SDK_SPEECHENCODER_OPUSENCODERCONTEXT_INCLUDE_SPEECHENCODER_OPUSENCODERCONTEXT_H_
#define ALEXA_CLIENT_SDK_SPEECHENCODER_OPUSENCODERCONTEXT_INCLUDE_SPEECHENCODER_OPUSENCODERCONTEXT_H_

#include <memory>
#include <speech_enconder/EncoderContext.h>

#ifdef OPUS_H
#define OPUS_ENCODER OpusEncoder
#else
#define OPUS_ENCODER void
#endif

namespace alexaClientSDK {
    namespace speechencoder {
        class OpusEncoderContext : public EncoderContext {
        public:
            OpusEncoderContext() = default;
            ~OpusEncoderContext();
            bool init(alexaClientSDK::avsCommon::utils::AudioFormat inputFormat) override;
            size_t getInputFrameSize() override;
            size_t getOutputFrameSize() override;
            bool requiresFullyRead() override;
            alexaClientSDK::avsCommon::utils::AudioFormat getAudioFormat() override;
            std::string getAVSFormatName() override;
            bool start() override;
            ssize_t processSamples(void* samples, size_t numberOfWords, uint8_t* buffer) override;
            void close() override;
        private:
            bool configureEncoder();
            OPUS_ENCODER* m_encoder = NULL;
            alexaClientSDK::avsCommon::utils::AudioFormat m_outputFormat;
            alexaClientSDK::avsCommon::utils::AudioFormat m_inputFormat;
        };
    }
}

#endif  // ALEXA_CLIENT_SDK_SPEECHENCODER_OPUSENCODERCONTEXT_INCLUDE_SPEECHENCODER_OPUSENCODERCONTEXT_H_