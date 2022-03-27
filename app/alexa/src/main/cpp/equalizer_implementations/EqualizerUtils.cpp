#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "EqualizerUtils.h"

namespace alexaClientSDK {
    namespace equalizer {
        using namespace rapidjson;
        static const string TAG{"EqualizerUtils"};
        #define LX(event) LogEntry(TAG, event)
        constexpr char JSON_KEY_BANDS[] = "bands";
        constexpr char JSON_KEY_NAME[] = "name";
        constexpr char JSON_KEY_LEVEL[] = "level";
        constexpr char JSON_KEY_MODE[] = "mode";
        string EqualizerUtils::serializeEqualizerState(const EqualizerState& state) {
            Document payload(kObjectType);
            auto& allocator = payload.GetAllocator();
            rapidjson::Value bandsBranch(kArrayType);
            for (const auto& bandIter : state.bandLevels) {
                EqualizerBand band = bandIter.first;
                int bandLevel = bandIter.second;
                Value bandDesc(kObjectType);
                Value _JSON_KEY_NAME{JSON_KEY_NAME, strlen(JSON_KEY_NAME)};
                Value _JSON_KEY_LEVEL{JSON_KEY_LEVEL, strlen(JSON_KEY_LEVEL)};
                string _band = equalizerBandToString(band);
                Value __band{_band.data(), _band.length()};
                Value _bandLevel{bandLevel};
                bandDesc.AddMember(_JSON_KEY_NAME, __band);
                bandDesc.AddMember(_JSON_KEY_LEVEL, _bandLevel);
                bandsBranch.PushBack(bandDesc);
            }
            payload.AddMember(JSON_KEY_BANDS, bandsBranch, allocator);
            if (EqualizerMode::NONE != state.mode) {
                string stateMode = equalizerModeToString(state.mode);
                Value _JSON_KEY_MODE{JSON_KEY_MODE, strlen(JSON_KEY_MODE)};
                Value _stateMode{stateMode.data(), stateMode.length()};
                payload.AddMember(_JSON_KEY_MODE, _stateMode);
            }
            StringBuffer buffer;
            rapidjson::Writer<StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX("serializeEqualizerStateFailed").d("reason", "writerRefusedJsonObject"));
                return "";
            }
            return buffer.GetString();
        }
        SuccessResult<EqualizerState> EqualizerUtils::deserializeEqualizerState(const std::string& serializedState) {
            Document document;
            ParseResult result = document.Parse(serializedState.data());
            if (!result) {
                ACSDK_ERROR(LX("deserializeEqualizerStateFailed").d("reason", "parseFailed").d("error", GetParseError_En(result.Code()))
                    .d("offset", result.Offset()));
                return SuccessResult<EqualizerState>::failure();
            }
            EqualizerState state;
            Value::ConstMemberIterator it;
            Value _document{document.GetString(), strlen(document.GetString())};
            if (!findNode(_document, JSON_KEY_BANDS, &it) || !it->value.IsArray()) {
                ACSDK_ERROR(LX("deserializeEqualizerStateFailed").d("reason", "'bands' value is missing"));
                return SuccessResult<EqualizerState>::failure();
            }
            for (const auto& bandDesc : it->value.GetArray()) {
                string bandName;
                if (!retrieveValue(bandDesc, JSON_KEY_NAME, &bandName)) {
                    ACSDK_ERROR(LX("deserializeEqualizerStateFailed").d("reason", "'name' value is missing"));
                    return SuccessResult<EqualizerState>::failure();
                }
                SuccessResult<EqualizerBand> bandResult = stringToEqualizerBand(bandName);
                if (!bandResult.isSucceeded()) {
                    ACSDK_ERROR(LX("deserializeEqualizerStateFailed").d("reason", "Invalid band").d("band", bandName));
                    return SuccessResult<EqualizerState>::failure();
                }
                EqualizerBand band = bandResult.value();
                int64_t bandLevel = 0;
                if (!retrieveValue(bandDesc, JSON_KEY_LEVEL, &bandLevel)) {
                    ACSDK_ERROR(LX("deserializeEqualizerStateFailed").d("reason", "Invalid band level").d("band", bandName));
                    return SuccessResult<EqualizerState>::failure();
                }
                state.bandLevels[band] = static_cast<int>(bandLevel);
            }
            string modeName;
            if (!retrieveValue(document, JSON_KEY_MODE, &modeName)) state.mode = EqualizerMode::NONE;
            else {
                SuccessResult<EqualizerMode> modeResult = stringToEqualizerMode(modeName);
                if (!modeResult.isSucceeded()) {
                    ACSDK_ERROR(LX("deserializeEqualizerStateFailed").d("reason", "Invalid mode").d("band", modeName));
                    return SuccessResult<EqualizerState>::failure();
                }
                state.mode = modeResult.value();
            }
            return SuccessResult<EqualizerState>::success(state);
        }
    }
}