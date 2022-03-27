#ifndef ALEXA_CLIENT_SDK_MEDIAPLAYER_GSTREAMERMEDIAPLAYER_INCLUDE_MEDIAPLAYER_MEDIAPLAYER_H_
#define ALEXA_CLIENT_SDK_MEDIAPLAYER_GSTREAMERMEDIAPLAYER_INCLUDE_MEDIAPLAYER_MEDIAPLAYER_H_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>
#include <gstreamer/gst/gst.h>
#include <gstreamer/libs/app/gstappsrc.h>
#include <gstreamer/libs/base/gstbasesink.h>
#include <gstreamer/libs/controller/gsttimedvaluecontrolsource.h>
#include <sdkinterfaces/Audio/EqualizerInterface.h>
#include <sdkinterfaces/HTTPContentFetcherInterfaceFactoryInterface.h>
#include <sdkinterfaces/SpeakerInterface.h>
#include <media_player/MediaPlayerInterface.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <util/MediaType.h>
#include <util/playlist_parser/PlaylistParserInterface.h>
#include <playlist_parser/UrlContentToAttachmentConverter.h>
#include "OffsetManager.h"
#include "PipelineInterface.h"
#include "SourceInterface.h"
#include "SourceObserverInterface.h"

namespace alexaClientSDK {
    namespace mediaPlayer {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace attachment;
        using namespace audio;
        using namespace mediaPlayer;
        using namespace playlistParser;
        using playlistParser::UrlContentToAttachmentConverter;
        using mediaPlayer::MediaPlayerObserverInterface;
        typedef vector<TagKeyValueType> VectorOfTags;
        class MediaPlayer : public RequiresShutdown, public MediaPlayerInterface, public SpeakerInterface, private PipelineInterface,
                            public EqualizerInterface, public ErrorObserverInterface, public WriteCompleteObserverInterface,
                            public SourceObserverInterface, public enable_shared_from_this<MediaPlayer> {
        public:
            static shared_ptr<MediaPlayer> create(shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory = nullptr,
                                                  bool enableEqualizer = false, std::string name = "", bool enableLiveMode = false);
            ~MediaPlayer();
            SourceId setSource(shared_ptr<AttachmentReader> attachmentReader, const AudioFormat* format = nullptr,
                               const SourceConfig& config = emptySourceConfig()) override;
            SourceId setSource(const string& url, milliseconds offset = milliseconds::zero(), const SourceConfig& config = emptySourceConfig(),
                               bool repeat = false, const PlaybackContext& playbackContext = PlaybackContext()) override;
            SourceId setSource(shared_ptr<istream> stream, bool repeat = false, const SourceConfig& config = emptySourceConfig(),
                               MediaType format = MediaType::UNKNOWN) override;
            bool play(SourceId id) override;
            bool stop(SourceId id) override;
            bool pause(SourceId id) override;
            bool resume(SourceId id) override;
            uint64_t getNumBytesBuffered() override;
            milliseconds getOffset(SourceId id) override;
            Optional<MediaPlayerState> getMediaPlayerState(SourceId id) override;
            void addObserver(shared_ptr<MediaPlayerObserverInterface> observer) override;
            void removeObserver(shared_ptr<MediaPlayerObserverInterface> observer) override;
            bool setVolume(int8_t volume) override;
            bool setMute(bool mute) override;
            bool getSpeakerSettings(SpeakerInterface::SpeakerSettings* settings) override;
            void setAppSrc(GstAppSrc* appSrc) override;
            GstAppSrc* getAppSrc() const override;
            void setDecoder(GstElement* decoder) override;
            GstElement* getDecoder() const override;
            GstElement* getPipeline() const override;
            guint queueCallback(const std::function<gboolean()>* callback) override;
            guint attachSource(GSource* source) override;
            gboolean removeSource(guint tag) override;
            void onError() override;
            void onWriteComplete() override;
            void onFirstByteRead() override;
            void doShutdown() override;
        private:
            struct AudioPipeline {
                GstAppSrc* appsrc;
                GstElement* decoder;
                GstElement* decodedQueue;
                GstElement* converter;
                GstElement* volume;
                GstElement* fadeIn;
                GstElement* resample;
                GstElement* caps;
                GstElement* equalizer;
                GstElement* audioSink;
                GstElement* pipeline;
                AudioPipeline() : appsrc{nullptr}, decoder{nullptr}, decodedQueue{nullptr}, converter{nullptr}, volume{nullptr}, fadeIn{nullptr},
                                  resample{nullptr}, caps{nullptr}, equalizer{nullptr}, audioSink{nullptr}, pipeline{nullptr} {};
            };
            MediaPlayer(shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory, bool enableEqualizer, string name,
                        bool enableLiveMode);
            bool configureSource(const SourceConfig& config);
            void workerLoop();
            bool init();
            static gboolean onCallback(const function<gboolean()>* callback);
            bool setupPipeline();
            void tearDownTransientPipelineElements(bool notifyStop);
            void resetPipeline();
            static void onPadAdded(GstElement* src, GstPad* pad, gpointer mediaPlayer);
            void handlePadAdded(promise<void>* promise, GstElement* src, GstPad* pad);
            static gboolean onBusMessage(GstBus* bus, GstMessage* msg, gpointer mediaPlayer);
            gboolean handleBusMessage(GstMessage* message);
            unique_ptr<const VectorOfTags> collectTags(GstMessage* message);
            void sendStreamTagsToObserver(unique_ptr<const VectorOfTags> vectorOfTags);
            void handleSetAttachmentReaderSource(shared_ptr<AttachmentReader> reader, const SourceConfig& config, promise<SourceId>* promise,
                                                 const AudioFormat* audioFormat = nullptr, bool repeat = false);
            void handleSetUrlSource(const string& url, milliseconds offset, const SourceConfig& config, promise<SourceId>* promise,
                                    bool repeat);
            void handleSetIStreamSource(shared_ptr<istream> stream, bool repeat, const SourceConfig& config, promise<SourceId>* promise);
            void handleSetVolumeInternal(gdouble gstVolume);
            void handleSetVolume(promise<bool>* promise, int8_t volume);
            void handleAdjustVolume(promise<bool>* promise, int8_t delta);
            void handleSetMute(promise<bool>* promise, bool mute);
            void handleGetSpeakerSettings(promise<bool>* promise, SpeakerInterface::SpeakerSettings* settings);
            void handlePlay(SourceId id, promise<bool>* promise);
            void handleStop(SourceId id, promise<bool>* promise);
            void handlePause(SourceId id, promise<bool>* promise);
            void handleResume(SourceId id, promise<bool>* promise);
            void handleGetOffset(SourceId id, promise<milliseconds>* promise);
            milliseconds handleGetOffsetImmediately(SourceId id);
            MediaPlayerState getMediaPlayerStateInternal(SourceId id);
            void handleAddObserver(promise<void>* promise, shared_ptr<MediaPlayerObserverInterface> observer);
            void handleRemoveObserver(promise<void>* promise, shared_ptr<MediaPlayerObserverInterface> observer);
            void sendPlaybackStarted();
            void sendPlaybackFinished();
            void sendPlaybackPaused();
            void sendPlaybackResumed();
            void sendPlaybackStopped();
            void sendPlaybackError(const ErrorType& type, const string& error);
            void sendBufferingComplete();
            void sendBufferUnderrun();
            void sendBufferRefilled();
            bool queryIsSeekable(bool* isSeekable);
            bool queryBufferPercent(gint* percent);
            bool seek();
            bool validateSourceAndId(SourceId id);
            static gboolean onErrorCallback(gpointer pointer);
            static gboolean onWriteCompleteCallback(gpointer pointer);
            milliseconds getCurrentStreamOffset();
            void cleanUpSource();
            void setEqualizerBandLevels(EqualizerBandLevelMap bandLevelMap) override;
            int getMinimumBandLevel() override;
            int getMaximumBandLevel() override;
            int clampEqualizerLevel(int level);
            mutex m_operationMutex;
            gdouble m_lastVolume;
            bool m_isMuted;
            shared_ptr<UrlContentToAttachmentConverter> m_urlConverter;
            shared_ptr<AttachmentReader> m_parkedReader;
            OffsetManager m_offsetManager;
            shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> m_contentFetcherFactory;
            bool m_equalizerEnabled;
            AudioPipeline m_pipeline;
            GMainLoop* m_mainLoop;
            thread m_mainLoopThread;
            guint m_busWatchId;
            GMainContext* m_workerContext;
            bool m_playbackStartedSent;
            bool m_playbackFinishedSent;
            bool m_isPaused;
            bool m_isBufferUnderrun;
            unordered_set<shared_ptr<MediaPlayerObserverInterface>> m_playerObservers;
            shared_ptr<SourceInterface> m_source;
            SourceId m_currentId;
            bool m_isFakeSink;
            bool m_playPending;
            bool m_pausePending;
            bool m_resumePending;
            bool m_pauseImmediately;
            milliseconds m_offsetBeforeTeardown;
            const bool m_isLiveMode;
        };
    }
}
#endif