#ifndef ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_DIAGNOSTICSUTILS_H_
#define ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_DIAGNOSTICSUTILS_H_

#include <string>
#include <vector>

namespace alexaClientSDK {
    namespace diagnostics {
        namespace utils {
            using namespace std;
            struct WavFileHeader {
                uint8_t riffHeader[4];
                uint32_t chunkSize;
                uint8_t waveHeader[4];
                uint8_t fmtHeader[4];
                uint32_t subChunk1Size;
                uint16_t audioFormat;
                uint16_t numberOfChannels;
                uint32_t samplesPerSec;
                uint32_t bytesPerSec;
                uint16_t blockAlign;
                uint16_t bitsPerSample;
                uint8_t subchunk2ID[4];
                uint32_t subchunk2Size;
            };
            bool validateAudioFormat(const WavFileHeader& wavFileHeader);
            bool readWavFileToBuffer(const string& absoluteFilePath, vector<uint16_t>* audioBuffer);
        }
    }
}
#endif