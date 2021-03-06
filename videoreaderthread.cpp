/* Copyright (c) 2011 Oliver Lau <oliver@von-und-fuer-lau.de>
 * All rights reserved.
 * $Id$
 */

#include <QDebug>
#include "videoreaderthread.h"
#include "project.h"

#include "videodecoder.h"
#include "webcam.h"


VideoReaderThread::VideoReaderThread(QObject* parent) :
        QThread(parent),
        mDecoder(NULL),
        mHistogramEnabled(true)
{
    /* ... */
}


VideoReaderThread::~VideoReaderThread()
{
    close();
}


void VideoReaderThread::close(void)
{
    stopReading();
}


void VideoReaderThread::setSource(IAbstractVideoDecoder* decoder)
{
    Q_ASSERT(decoder != NULL);
    Q_ASSERT(!isRunning());
    close();
    mDecoder = decoder;
}


void VideoReaderThread::startReading(int firstFrameNumber, int nFrames, qreal frameDelta)
{
    Q_ASSERT_X(firstFrameNumber != Project::INVALID_FRAME, "VideoReaderThread::startReading()", "Invalid frame number (firstFrameNumber)");
    Q_ASSERT_X(nFrames > 0, "VideoReaderThread::startReading()", "nFrames <= 0");
    Q_ASSERT_X(frameDelta > 0, "VideoReaderThread::startReading()", "frameDelta <= 0");
    stopReading();
    mFrameNumber = (qreal)firstFrameNumber;
    mMaxFrameCount = nFrames;
    mFrameDelta = frameDelta;
    mAbort = false;
    start();
}


void VideoReaderThread::stopReading(void)
{
    if (isRunning()) {
        mAbort = true;
        wait();
    }
}


void VideoReaderThread::setHistogramRegion(const QRect& rect)
{
    if (isRunning())
        return;
    mHistogramRegion = rect;
}


void VideoReaderThread::calcHistogram(const QImage& src)
{
    const QRect histoRect = mHistogramRegion.isNull()? src.rect() : mHistogramRegion;
    // downscale image (region) for faster histogram generation
    QImage img(src.copy(histoRect).scaled(histoRect.size().boundedTo(QSize(100, 100)), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    mHistogram.init(img.width() * img.height());
    for (int y = 0; y < img.height(); ++y) {
        const QRgb* d = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x, ++d)
            mHistogram.add(*d);
    }
    mHistogram.postprocess();
}


void VideoReaderThread::run(void)
{
    Q_ASSERT_X(mFrameNumber != Project::INVALID_FRAME, "VideoReaderThread::run()", "Invalid frame number (mFrameNumber)");
    qreal prevFrameNumber = mFrameNumber;
    int frameCount = 0;
    int percent = 0, prevPercent = 0;
    int effFrameNum, effFrameTime;
    mDecoder->seekFrame(mFrameNumber);
    while (!mAbort && frameCount < mMaxFrameCount) {
        int skip = int(mFrameNumber) - int(prevFrameNumber);
        if (skip > 0) {
            mDecoder->seekNextFrame(skip);
            prevFrameNumber = mFrameNumber;
        }
        mDecoder->getFrame(mCurrentFrame, &effFrameNum, &effFrameTime);
        if (skip > 0 && mHistogramEnabled)
            calcHistogram(mCurrentFrame);
        emit frameReady(mCurrentFrame, mHistogram, frameCount, effFrameNum, effFrameTime);
        ++frameCount;
        mFrameNumber += mFrameDelta;
        percent = 100 * frameCount / mMaxFrameCount;
        if (percent != prevPercent)
            emit percentReady(percent);
        prevPercent = percent;
    }
}
