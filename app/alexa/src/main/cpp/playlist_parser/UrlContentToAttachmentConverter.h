#ifndef ALEXA_CLIENT_SDK_PLAYLISTPARSER_INCLUDE_PLAYLISTPARSER_URLCONTENTTOATTACHMENTCONVERTER_H_
#define ALEXA_CLIENT_SDK_PLAYLISTPARSER_INCLUDE_PLAYLISTPARSER_URLCONTENTTOATTACHMENTCONVERTER_H_

#include <atomic>
#include <memory>
#include <avs/attachment/InProcessAttachment.h>
#include <avs/attachment/InProcessAttachmentReader.h>
#include <avs/attachment/InProcessAttachmentWriter.h>
#include <sdkinterfaces/HTTPContentFetcherInterfaceFactoryInterface.h>
#include <util/playlist_parser/PlaylistParserObserverInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include "ContentDecrypter.h"
#include "PlaylistParser.h"

namespace alexaClientSDK {
    namespace playlistParser {

        class UrlContentToAttachmentConverter
                : public avsCommon::utils::playlistParser::PlaylistParserObserverInterface
                , public avsCommon::utils::RequiresShutdown {
        public:
            class ErrorObserverInterface {
            public:
                virtual ~ErrorObserverInterface() = default;
                virtual void onError() = 0;
            };
            class WriteCompleteObserverInterface {
            public:
                virtual ~WriteCompleteObserverInterface() = default;
                virtual void onWriteComplete() = 0;
            };
            static std::shared_ptr<UrlContentToAttachmentConverter> create(
                std::shared_ptr<avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory,
                const std::string& url,
                std::shared_ptr<ErrorObserverInterface> observer,
                std::chrono::milliseconds startTime = std::chrono::milliseconds::zero(),
                std::shared_ptr<WriteCompleteObserverInterface> writeCompleteObserver = nullptr,
                size_t numOfReaders = 1);
            std::shared_ptr<avsCommon::avs::attachment::InProcessAttachment> getAttachment();
            std::chrono::milliseconds getStartStreamingPoint();
            std::chrono::milliseconds getDesiredStreamingPoint();
            void doShutdown() override;
        private:
            UrlContentToAttachmentConverter(
                std::shared_ptr<avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory,
                const std::string& url,
                std::shared_ptr<ErrorObserverInterface> observer,
                std::chrono::milliseconds startTime,
                std::shared_ptr<WriteCompleteObserverInterface> writeCompleteObserver,
                size_t numOfReaders);
            void onPlaylistEntryParsed(int requestId, avsCommon::utils::playlistParser::PlaylistEntry playlistEntry) override;
            void notifyError();
            void notifyWriteComplete();
            bool writeDecryptedUrlContentIntoStream(
                std::string url,
                std::vector<std::string> headers,
                avsCommon::utils::playlistParser::EncryptionInfo encryptionInfo,
                std::shared_ptr<avsCommon::sdkInterfaces::HTTPContentFetcherInterface> contentFetcher);
            bool download(
                const std::string& url,
                const std::vector<std::string>& headers,
                std::shared_ptr<avsCommon::avs::attachment::AttachmentWriter> streamWriter,
                std::shared_ptr<avsCommon::sdkInterfaces::HTTPContentFetcherInterface> contentFetcher);
            bool download(
                const std::string& url,
                const std::vector<std::string>& headers,
                ByteVector* content,
                std::shared_ptr<avsCommon::sdkInterfaces::HTTPContentFetcherInterface> contentFetcher);
            bool readContent(std::shared_ptr<avsCommon::avs::attachment::AttachmentReader> reader, ByteVector* content);
            bool shouldDecrypt(const avsCommon::utils::playlistParser::EncryptionInfo& encryptionInfo) const;
            void closeStreamWriter();
            const std::chrono::milliseconds m_desiredStreamPoint;
            std::shared_ptr<avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface> m_contentFetcherFactory;
            std::shared_ptr<PlaylistParser> m_playlistParser;
            std::shared_ptr<avsCommon::avs::attachment::InProcessAttachment> m_stream;
            std::shared_ptr<avsCommon::avs::attachment::AttachmentWriter> m_streamWriter;
            std::shared_ptr<ErrorObserverInterface> m_observer;
            std::shared_ptr<WriteCompleteObserverInterface> m_writeCompleteObserver;
            std::mutex m_mutex;
            std::atomic<bool> m_shuttingDown;
            std::shared_ptr<ContentDecrypter> m_contentDecrypter;
            std::promise<std::chrono::milliseconds> m_startStreamingPointPromise;
            std::shared_future<std::chrono::milliseconds> m_startStreamingPointFuture;
            std::chrono::milliseconds m_runningTotal;
            bool m_startedStreaming;
            bool m_streamWriterClosed;
            avsCommon::utils::threading::Executor m_executor;
        };
    }
}
#endif