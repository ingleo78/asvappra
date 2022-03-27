#include <algorithm>
#include <iterator>
#include <utility>
#include <logger/LogEntry.h>
#include <logger/Logger.h>
#include <util/string/StringUtils.h>
#include "CaptionManager.h"
#include "CaptionTimingAdapter.h"
#include "TimingAdapterFactory.h"

namespace alexaClientSDK {
    namespace captions {
        static const std::string TAG = "CaptionManager";
        #define LX(event) LogEntry(TAG, event)
        CaptionManager::CaptionManager(shared_ptr<CaptionParserInterface> parser, shared_ptr<TimingAdapterFactory> timingAdapterFactory) :
                                       RequiresShutdown{TAG}, m_parser{parser}, m_timingFactory{timingAdapterFactory} {}
        shared_ptr<CaptionManager> CaptionManager::create(shared_ptr<CaptionParserInterface> parser, shared_ptr<TimingAdapterFactory> timingAdapterFactory) {
            ACSDK_DEBUG7(LX(__func__));
            if (!parser) {
                ACSDK_ERROR(LX("captionManagerCreateFailed").d("reason", "captionParserIsNull"));
                return nullptr;
            }
            if (!timingAdapterFactory) timingAdapterFactory = make_shared<TimingAdapterFactory>();
            auto captionManager = shared_ptr<CaptionManager>(new CaptionManager(parser, timingAdapterFactory));
            parser->addListener(captionManager);
            return captionManager;
        }
        void CaptionManager::doShutdown() {
            ACSDK_DEBUG7(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            for (auto& player : m_mediaPlayers) {
                player->removeObserver(shared_from_this());
                player.reset();
            }
            for (auto& adapterPair : m_timingAdaptersBySourceIds) {
                auto& adapter = adapterPair.second;
                adapter->reset();
                adapter.reset();
            }
            m_timingAdaptersBySourceIds.clear();
            m_parser.reset();
        }
        void CaptionManager::onCaption(MediaPlayerSourceId sourceId, const CaptionData& captionData) {
            ACSDK_DEBUG7(LX(__func__));
            ACSDK_DEBUG5(LX("sendingCaptionDataToParser").d("sourceId", sourceId));
            m_parser->parse(sourceId, captionData);
        }
        void CaptionManager::setCaptionPresenter(const shared_ptr<CaptionPresenterInterface>& presenter) {
            ACSDK_DEBUG7(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_presenter = presenter;
        }
        void CaptionManager::setMediaPlayers(const vector<shared_ptr<MediaPlayerInterface>>& mediaPlayers) {
            ACSDK_DEBUG7(LX(__func__));
            if (mediaPlayers.empty()) {
                ACSDK_ERROR(LX("setMediaPlayersFailed").d("reason", "noMediaPlayersAvailable"));
                return;
            }
            for (auto const& player : mediaPlayers) {
                if (!player) {
                    ACSDK_ERROR(LX("setMediaPlayersFailed").d("reason", "mediaPlayersArgumentContainsNullptr"));
                    return;
                }
            }
            lock_guard<mutex> lock(m_mutex);
            if (!m_mediaPlayers.empty()) {
                for (auto& player : m_mediaPlayers) {
                    player->removeObserver(shared_from_this());
                    player.reset();
                }
            }
            m_mediaPlayers = mediaPlayers;
            for (auto const& player : m_mediaPlayers) {
                player->addObserver(shared_from_this());
            }
            ACSDK_DEBUG5(LX("mediaPlayersAdded").d("count", mediaPlayers.size()));
        }
        void CaptionManager::onParsed(const CaptionFrame& captionFrame) {
            unique_lock<mutex> lock(m_mutex);
            auto presenterCopy = m_presenter;
            lock.unlock();
            if (!presenterCopy) {
                ACSDK_WARN(LX("parsedCaptionFrameIgnored").d("Reason", "presenterIsNull"));
                return;
            }
            CaptionLine line = CaptionLine::merge(captionFrame.getCaptionLines());
            vector<CaptionLine> wrappedCaptionLines;
            pair<bool, int> wrapIndexResult;
            bool shouldWrap = true;
            int wrapIndex = 0;
            int lineWrapIterationCount = 0;
            std::string lineText = line.text;
            while (shouldWrap && lineWrapIterationCount < CaptionFrame::getLineWrapLimit()) {
                wrapIndexResult = presenterCopy->getWrapIndex(line);
                shouldWrap = wrapIndexResult.first;
                if (shouldWrap) {
                    wrapIndex = wrapIndexResult.second;
                    for (size_t i = wrapIndex; i > 0; i--) {
                        if (' ' == lineText[i]) {
                            wrapIndex = i;
                            break;
                        }
                    }
                    if (0 == wrapIndex) wrapIndex = wrapIndexResult.second;
                    vector<CaptionLine> splitLines = line.splitAtTextIndex(wrapIndex);
                    if (splitLines.size() == 1) wrappedCaptionLines.emplace_back(splitLines.front());
                    else if (splitLines.size() == 2) {
                        wrappedCaptionLines.emplace_back(splitLines.front());
                        line = splitLines.back();
                    } else {
                        ACSDK_WARN(LX("unexpectedLineSplitResult").d("wrapIndex", wrapIndex).d("lineCount", splitLines.size()));
                        for (const auto& splitLine : splitLines) {
                            wrappedCaptionLines.emplace_back(splitLine);
                        }
                    }
                }
                lineText = line.text;
                lineWrapIterationCount++;
            }
            if (lineWrapIterationCount > CaptionFrame::getLineWrapLimit()) {
                ACSDK_WARN(LX("exceededLineWrapLimit").d("LineWrapLimit", CaptionFrame::getLineWrapLimit()));
            }
            if (!line.text.empty()) wrappedCaptionLines.emplace_back(line);
            CaptionFrame displayFrame = CaptionFrame(captionFrame.getSourceId(), captionFrame.getDuration(), captionFrame.getDelay(),
                                                     wrappedCaptionLines);
            lock.lock();
            CaptionFrame::MediaPlayerSourceId sourceId = captionFrame.getSourceId();
            ACSDK_DEBUG5(LX("sendingCaptionToTimingAdapter").d("mediaPlayerSourceId", sourceId));
            shared_ptr<CaptionTimingAdapterInterface> timingAdapterCopy;
            if (m_timingAdaptersBySourceIds.count(sourceId) == 0) {
                timingAdapterCopy = m_timingFactory->getTimingAdapter(presenterCopy);
                m_timingAdaptersBySourceIds.emplace(sourceId, timingAdapterCopy);
            } else timingAdapterCopy = m_timingAdaptersBySourceIds.at(sourceId);
            if (!timingAdapterCopy) {
                ACSDK_ERROR(LX("presentCaptionFrameFailed").d("reason", "captionTimingAdapterIsNull"));
                return;
            }
            timingAdapterCopy->queueForDisplay(displayFrame);
            lock.unlock();
            ACSDK_DEBUG5(LX("finishedOnParsed"));
        }
        void CaptionManager::onPlaybackStarted(CaptionFrame::MediaPlayerSourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG3(LX(__func__).d("id", id));
            lock_guard<mutex> lock(m_mutex);
            auto itr = m_timingAdaptersBySourceIds.find(id);
            if (itr != m_timingAdaptersBySourceIds.end()) itr->second->start();
            else logMediaStateNotHandled(__func__, "timingAdapterNotFound", id);
        }
        void CaptionManager::onPlaybackFinished(MediaPlayerSourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG3(LX(__func__).d("id", id));
            lock_guard<mutex> lock(m_mutex);
            auto itr = m_timingAdaptersBySourceIds.find(id);
            if (itr != m_timingAdaptersBySourceIds.end()) {
                ACSDK_DEBUG5(LX("resettingTimingAdapter").d("sourceId", itr->first));
                itr->second.reset();
                m_timingAdaptersBySourceIds.erase(itr);
            } else logMediaStateNotHandled(__func__, "timingAdapterNotFound", id);
            m_parser->releaseResourcesFor(id);
        }
        void CaptionManager::onPlaybackError(MediaPlayerSourceId id, const ErrorType& type, std::string error, const MediaPlayerState&) {
            ACSDK_DEBUG3(LX(__func__).d("type", type).d("error", error).d("id", id));
            lock_guard<mutex> lock(m_mutex);
            auto itr = m_timingAdaptersBySourceIds.find(id);
            if (itr != m_timingAdaptersBySourceIds.end()) {
                itr->second->stop();
                m_timingAdaptersBySourceIds.erase(itr);
            } else logMediaStateNotHandled(__func__, "timingAdapterNotFound", id);
            m_parser->releaseResourcesFor(id);
        }
        void CaptionManager::onPlaybackPaused(MediaPlayerSourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG3(LX(__func__).d("id", id));
            lock_guard<std::mutex> lock(m_mutex);
            auto itr = m_timingAdaptersBySourceIds.find(id);
            if (itr != m_timingAdaptersBySourceIds.end()) itr->second->pause();
            else logMediaStateNotHandled(__func__, "timingAdapterNotFound", id);
        }
        void CaptionManager::onPlaybackResumed(MediaPlayerSourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG3(LX(__func__).d("id", id));
            lock_guard<std::mutex> lock(m_mutex);
            auto itr = m_timingAdaptersBySourceIds.find(id);
            if (itr != m_timingAdaptersBySourceIds.end()) itr->second->start();
            else logMediaStateNotHandled(__func__, "timingAdapterNotFound", id);
        }
        void CaptionManager::onPlaybackStopped(MediaPlayerSourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG3(LX(__func__).d("id", id));
            lock_guard<std::mutex> lock(m_mutex);
            auto itr = m_timingAdaptersBySourceIds.find(id);
            if (itr != m_timingAdaptersBySourceIds.end()) itr->second->stop();
            else logMediaStateNotHandled(__func__, "timingAdapterNotFound", id);
        }
        void CaptionManager::logMediaStateNotHandled(const std::string& event, const std::string& reason, MediaPlayerSourceId id) {
            ACSDK_WARN(LX("mediaStateNotHandled").d("mediaAction", event).d("Reason", reason).d("SourceId", id)
                .d("idCount", m_timingAdaptersBySourceIds.count(id)).d("timingAdaptersPresent", m_timingAdaptersBySourceIds.size()));
        }
    }
}