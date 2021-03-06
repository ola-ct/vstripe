/* Copyright (c) 2011 Oliver Lau <oliver@von-und-fuer-lau.de>
 * All rights reserved.
 * $Id$
 */

#ifndef __VIDEOREADERTHREAD_H_
#define __VIDEOREADERTHREAD_H_

#include <QThread>
#include <QString>
#include <QImage>
#include <QFile>
#include <QVector>

#include "videowidget.h"
#include "histogram.h"


class IAbstractVideoDecoder;


class VideoReaderThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoReaderThread(QObject* parent = NULL);
    ~VideoReaderThread();

    void setSource(IAbstractVideoDecoder*);
    void startReading(int firstFrameNumber, int numFrames, qreal frameDelta);
    void stopReading(void);

    const Histogram& histogram(void) const { return mHistogram; }
    void calcHistogram(const QImage& img);

public slots:
    void setHistogramRegion(const QRect&);

signals:
    void percentReady(int);
    void frameReady(QImage, Histogram, int, int, int);

protected:
    void run(void);

private: // methods
    void close(void);

private: // variables
    IAbstractVideoDecoder* mDecoder;
    volatile bool mAbort;
    int mMaxFrameCount;
    qreal mFrameNumber;
    qreal mFrameDelta;
    bool mHistogramEnabled;
    Histogram mHistogram;
    QRect mHistogramRegion;
    QImage mCurrentFrame;
};

#endif // __VIDEOREADERTHREAD_H_
