#include "ThumbnailExtractor.hpp"
#include "VideoRenderer.hpp"
#include <QImage>
#include <iostream>

ThumbnailExtractor::ThumbnailExtractor(QObject* parent, VideoRenderer* renderer)
    : QThread(parent)
{
    mRenderer = renderer;
}
ThumbnailExtractor::~ThumbnailExtractor()
{
}

void ThumbnailExtractor::extractThumbnail(QString file, int timestamp)
{
    // if directory, open dir and pick first file?
    mFileList << file;
    mTimestamp = timestamp;
    start();
}

void ThumbnailExtractor::extractThumbnails(QStringList list, int timestamp)
{
    mFileList = list;
    mTimestamp = timestamp;
    start();
}

void ThumbnailExtractor::run()
{
    std::cout << "Thumbnail extractor started\n";

    for (int i = 0; i < mFileList.size(); i++) {
        QString m_fileName = mFileList[i];

        AVFormatContext* pFormatCtx = NULL;
        //AVStream *pVideoStream = NULL;

        if (avformat_open_input(&pFormatCtx, m_fileName.toStdString().c_str(), 0, 0) != 0) {
            std::cout << "Error opening file " << m_fileName.toStdString().c_str() << "\n";
            continue;
        }

        if (avformat_find_stream_info(pFormatCtx, 0) < 0) {
            std::cout << "Error finding stream info\n";
            continue;
        }

        int videoStreamIndex = -1;
        for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
            if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStreamIndex < 0) {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1) {
            std::cout << "Error finding video stream\n";
            continue;
        }

        AVCodecContext* pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
        AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL) {
            std::cout << "Error unsupported video codec\n";
            continue;
        }
        if (avcodec_open2(pCodecCtx, pCodec, 0) < 0) {
            std::cout << "Error opening video codec\n";
            continue;
        }

        int w = pCodecCtx->width;
        int h = pCodecCtx->height;

        AVFrame* pFrame = av_frame_alloc();
        AVPacket packet;
        int frameDecoded = false;
        bool finished = false;

        av_seek_frame(pFormatCtx, -1, (int64_t)(mTimestamp * AV_TIME_BASE), 0);
        while (!finished && av_read_frame(pFormatCtx, &packet) >= 0) {
            if (packet.stream_index == videoStreamIndex) {
                avcodec_decode_video2(pCodecCtx, pFrame, &frameDecoded, &packet);
                if (frameDecoded) {
                    QFileInfo fileInfo(m_fileName);
                    mImageName = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + ".jpg";

                    RenderImage img;
                    img.h = h;
                    img.w = w;
                    for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i) {
                        img.data[i] = pFrame->data[i];
                        img.linesize[i] = pFrame->linesize[i];
                    }
                    img.format = pCodecCtx->pix_fmt;
                    img.filename = mImageName;

                    QMetaObject::invokeMethod(mRenderer, "renderToFile", Qt::BlockingQueuedConnection, Q_ARG(RenderImage, img));

                    finished = true;
                }
                av_packet_unref(&packet);
            } else
                av_packet_unref(&packet);
        }

        av_frame_free(&pFrame);
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatCtx);
    }

    std::cout << "Thumbnail extractor stopped\n";

    emit thumbnailsExtracted();
    mFileList.clear();
}
