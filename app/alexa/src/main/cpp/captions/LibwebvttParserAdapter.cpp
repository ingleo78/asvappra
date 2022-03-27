#include <algorithm>
#include <map>
#include <iostream>
#include <thread>
#include <logger/LoggerUtils.h>
#include <webvtt/webvtt/parser.h>
#include <webvtt/webvtt/node.h>
#include "LibwebvttParserAdapter.h"
#include "CaptionData.h"

namespace alexaClientSDK {
    namespace captions {
        static const string TAG("LibwebvttParserAdapter");
        static const int WEBVTT_CALLBACK_ERROR = -1;
        #define LX(event) LogEntry(TAG, event)
        static shared_ptr<CaptionFrameParseListenerInterface> g_parseListener;
        static vector<MediaPlayerSourceId> g_captionSourceIds;
        static map<MediaPlayerSourceId, milliseconds> g_captionIdsToLastEndTime;
        static shared_ptr<LibwebvttParserAdapter> m_libwebvttParserAdapterInstance;
        static mutex g_mutex;
        shared_ptr<LibwebvttParserAdapter> LibwebvttParserAdapter::getInstance() {
            lock_guard<std::mutex> lock(g_mutex);
            if (!m_libwebvttParserAdapterInstance) {
                m_libwebvttParserAdapterInstance = shared_ptr<LibwebvttParserAdapter>(new LibwebvttParserAdapter());
            }
            return m_libwebvttParserAdapterInstance;
        }
        void buildStyles(stringstream& cleanText, vector<TextStyle>& styles, const webvtt_node& node) {
            if (node.kind == WEBVTT_HEAD_NODE) {
                int dataLength = static_cast<int>(node.data.internal_data->length);
                for (int i = 0; i < dataLength; i++) {
                    buildStyles(cleanText, styles, *node.data.internal_data->children[i]);
                }
            } else if (node.kind == WEBVTT_TEXT) {
                string childNodeText = static_cast<const char*>(webvtt_string_text(&node.data.text));
                cleanText << childNodeText;
                ACSDK_DEBUG9(LX("Node").d("kind", "WEBVTT_TEXT").sensitive("text", childNodeText));
            } else if (node.kind == WEBVTT_ITALIC || node.kind == WEBVTT_BOLD || node.kind == WEBVTT_UNDERLINE) {
                auto styleStart = TextStyle(styles.back());
                styleStart.charIndex = cleanText.str().length();
                int childNodeCount = static_cast<int>(node.data.internal_data->length);
                for (int i = 0; i < childNodeCount; i++) {
                    buildStyles(cleanText, styles, *node.data.internal_data->children[i]);
                }
                auto styleEnd = TextStyle(styles.back());
                styleEnd.charIndex = cleanText.str().length();
                switch(node.kind) {
                    case WEBVTT_ITALIC:
                        styleStart.activeStyle.m_italic = true;
                        styleEnd.activeStyle.m_italic = false;
                        break;
                    case WEBVTT_BOLD:
                        styleStart.activeStyle.m_bold = true;
                        styleEnd.activeStyle.m_bold = false;
                        break;
                    case WEBVTT_UNDERLINE:
                        styleStart.activeStyle.m_underline = true;
                        styleEnd.activeStyle.m_underline = false;
                        break;
                    default: break;
                }
                styles.emplace_back(styleStart);
                styles.emplace_back(styleEnd);
            } else { ACSDK_DEBUG9(LX("unsupportedNode").sensitive("kind", node.kind)); }
        }
        void WEBVTT_CALLBACK onCueParsed(void* userdata, webvtt_cue* cue) {
            ACSDK_DEBUG7(LX(__func__));
            string body_text(static_cast<const char*>(webvtt_string_text(&cue->body)));
            auto start_time = static_cast<uint64_t>(cue->from);
            auto end_time = static_cast<uint64_t>(cue->until);
            stringstream cleanText;
            vector<TextStyle> styles;
            const webvtt_node* head = cue->node_head;
            if (head != nullptr) {
                styles.emplace_back(TextStyle{0, Style()});
                buildStyles(cleanText, styles, *head);
            } else { ACSDK_WARN(LX("libwebvtt returned a null node for style information.")); }
            unique_lock<std::mutex> lock(g_mutex);
            const uint64_t captionId = *static_cast<MediaPlayerSourceId*>(userdata);
            ACSDK_DEBUG9(LX("captionContentToCaptionIdLookup").d("captionId", captionId).d("userdataVoidPtr", userdata));
            auto delayItr = g_captionIdsToLastEndTime.find(captionId);
            auto delayMs = milliseconds(0);
            if (delayItr == g_captionIdsToLastEndTime.end()) {
                ACSDK_WARN(LX("captionDelayInaccurate").d("reason", "lastEndTimeUnknown"));
            } else delayMs = milliseconds(start_time) - delayItr->second;
            g_captionIdsToLastEndTime[captionId] = milliseconds(end_time);
            ACSDK_DEBUG9(LX("captionTimesCalculated").d("delayMs", delayMs.count()).d("startTime", start_time).d("endTime", end_time));
            lock.unlock();
            if (g_parseListener != nullptr) {
                vector<CaptionLine> captionLines;
                CaptionLine tempLine = CaptionLine{cleanText.str(), styles};
                stringstream ss(cleanText.str());
                string textLine;
                while(getline(ss, textLine)) {
                    vector<CaptionLine> split = tempLine.splitAtTextIndex(textLine.length());
                    captionLines.emplace_back(split[0]);
                    if (split.size() == 2) tempLine = split[1];
                }
                CaptionFrame caption_frame = CaptionFrame(captionId,milliseconds(end_time - start_time), delayMs, captionLines);
                g_parseListener->onParsed(caption_frame);
                ACSDK_DEBUG9(LX("libwebvttSentParsedCaptionFrame"));
            } else { ACSDK_WARN(LX("libwebvttCannotSendParsedCaptionFrame").d("reason", "parseListenerIsNull")); }
        }
        int WEBVTT_CALLBACK onParseError(void* userdata, webvtt_uint line, webvtt_uint col, webvtt_error errcode) {
            const uint64_t captionId = *static_cast<MediaPlayerSourceId*>(userdata);
            ACSDK_ERROR(LX("libwebvttError").d("line", line).d("col", col).d("error code", errcode)
                .d("error message", webvtt_strerror(errcode)).d("captionId", captionId).d("userdataVoidPtr", userdata));
            return WEBVTT_CALLBACK_ERROR;
        }
        void LibwebvttParserAdapter::parse(CaptionFrame::MediaPlayerSourceId captionId, const CaptionData& captionData) {
            ACSDK_DEBUG7(LX(__func__));
            webvtt_parser vtt;
            webvtt_status result;
            auto captionContentVp = static_cast<const void*>(captionData.content.c_str());
            unique_lock<mutex> lock(g_mutex);
            g_captionSourceIds.emplace_back(captionId);
            size_t sourceIdIndex = g_captionSourceIds.size() - 1;
            g_captionIdsToLastEndTime.emplace(captionId,milliseconds(0));
            lock.unlock();
            ACSDK_DEBUG9(LX("captionContentToCaptionIdCreation").d("content_cp_to_vp", captionContentVp).d("captionId", captionId));
            result = webvtt_create_parser((webvtt_cue_fn)&onCueParsed, &onParseError, &g_captionSourceIds[sourceIdIndex], &vtt);
            if (result != WEBVTT_SUCCESS) {
                ACSDK_ERROR(LX("failed to create WebVTT parser").d("webvtt_status", result));
                return;
            }
            result = webvtt_parse_chunk(vtt, captionContentVp, static_cast<webvtt_uint>(captionData.content.length()));
            if (result != WEBVTT_SUCCESS) {
                ACSDK_ERROR(LX("WebVTT parser failed to parse"));
                return;
            }
            ACSDK_DEBUG9(LX("libwebvttFinished"));
            webvtt_finish_parsing(vtt);
            webvtt_delete_parser(vtt);
        }
        void LibwebvttParserAdapter::addListener(shared_ptr<CaptionFrameParseListenerInterface> listener) {
            ACSDK_DEBUG7(LX(__func__));
            g_parseListener = listener;
        }
        void LibwebvttParserAdapter::releaseResourcesFor(MediaPlayerSourceId captionId) {
            ACSDK_DEBUG7(LX(__func__).d("captionId", captionId));
            lock_guard<mutex> lock(g_mutex);
            for (auto it1 = g_captionSourceIds.begin(); it1 != g_captionSourceIds.end(); ++it1) {
                if (captionId == *it1) {
                    g_captionSourceIds.erase(it1);
                    break;
                }
            }
            auto it2 = g_captionIdsToLastEndTime.find(captionId);
            if (it2 != g_captionIdsToLastEndTime.end()) g_captionIdsToLastEndTime.erase(it2);
        }
    }
}