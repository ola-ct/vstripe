/* Copyright (c) 2011 Oliver Lau <oliver@von-und-fuer-lau.de>
 * All rights reserved.
 * $Id$
 */

#undef WITH_KINETIC_SCROLLING

#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QScrollArea>
#include <QScrollBar>
#include <QClipboard>
#include "picturewidget.h"

#include <qmath.h>


#ifdef WITH_KINETIC_SCROLLING
const qreal PictureWidget::KineticFriction = 0.75;
const int PictureWidget::KineticTimeInterval = 30;
const int PictureWidget::MaxKineticPoints = 5;
#endif

PictureWidget::PictureWidget(QWidget* parent) :
        QWidget(parent),
        mShowCurves(true),
        mBrightnessData(NULL),
        mRedData(NULL),
        mGreenData(NULL),
        mBlueData(NULL),
        mAvgBrightness(-1),
        mAvgRed(-1),
        mAvgGreen(-1),
        mAvgBlue(-1),
        mMinBrightness(-1),
        mMinRed(-1),
        mMinGreen(-1),
        mMinBlue(-1),
        mStripePos(-1),
        mStripeVertical(true),
        mDragging(false),
#ifdef WITH_KINETIC_SCROLLING
        mKineticTimer(0),
        mNumMoveEvents(0),
#endif
        mScrollArea(NULL)
{
    resetPanAndZoom();
}


void PictureWidget::copyImageToClipboard(void)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setImage(mImage);
}


void PictureWidget::showCurves(bool enabled)
{
    mShowCurves = enabled;
    update();
}


void PictureWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_C && (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
        copyImageToClipboard();
}


void PictureWidget::resizeEvent(QResizeEvent*)
{
    // ...
}


void PictureWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        Q_ASSERT(mScrollArea != NULL);
        setCursor(Qt::ClosedHandCursor);
        mDragStartPos = event->pos();
        mDragging = true;
#ifdef WITH_KINETIC_SCROLLING
        mMouseMoveTimer.start();
        mKineticStartPos = mousePosInScrollArea();
        mNumMoveEvents = 0;
        mKineticMousePos.clear();
        mKineticMouseTime.clear();
#endif
    }
}


void PictureWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (mDragging) {
        mDragging = false;
        setCursor(Qt::OpenHandCursor);
#ifdef WITH_KINETIC_SCROLLING
        if (mNumMoveEvents > MaxKineticPoints) {
            for (int i = 0; i < MaxKineticPoints; ++i)
                qDebug() << "P" << mKineticMousePos[i] << mKineticMouseTime[i];
            QPoint mouseDiff = mKineticMousePos.last() - mousePosInScrollArea();
            qDebug() << "mouseDiff =" << mouseDiff;
            int dt = mMouseMoveTimer.elapsed();
            qDebug() << "mouse velocity =" << 1e-3 * mouseDiff / dt << " px/s";
            mVelocity = 1000 * mouseDiff / dt / KineticTimeInterval;
            qDebug() << "mVelocity =" << mVelocity;
            if (mKineticTimer == 0)
                mKineticTimer = startTimer(KineticTimeInterval);
        }
#endif
    }
}


void PictureWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (mDragging) {
        scrollBy(mDragStartPos - event->pos());
#ifdef WITH_KINETIC_SCROLLING
        ++mNumMoveEvents;
        mKineticMousePos.push_back(event->pos());
        mKineticMouseTime.push_back(mMouseMoveTimer.elapsed());
        if (mKineticMousePos.count() > MaxKineticPoints) {
            mKineticMousePos.pop_front();
            mKineticMouseTime.pop_front();
        }
#endif
    }
}


void PictureWidget::wheelEvent(QWheelEvent* event)
{
    mMouseSteps += event->delta() / 16;
    mZoom = pow(1.1, mMouseSteps);
    setZoom(mZoom);
}


#ifdef WITH_KINETIC_SCROLLING
void PictureWidget::timerEvent(QTimerEvent*)
{
    if (mVelocity.manhattanLength() < 1) {
        killTimer(mKineticTimer);
        mKineticTimer = 0;
        mVelocity = QPointF(0, 0);
    }
    else {
        scrollBy(mVelocity.toPoint());
        mVelocity *= KineticFriction;
    }
    qDebug() << "mVelocity =" << mVelocity;
}
#endif

void PictureWidget::scrollBy(const QPoint& d)
{
    Q_ASSERT(mScrollArea != NULL);
    mScrollArea->horizontalScrollBar()->setValue(mScrollArea->horizontalScrollBar()->value() + d.x());
    mScrollArea->verticalScrollBar()->setValue(mScrollArea->verticalScrollBar()->value() + d.y());
}


void PictureWidget::setZoom(qreal zoom)
{
    mZoom = zoom;
    setMinimumSize(mImage.size() * mZoom);
    resize(mImage.size() * mZoom);
    update();
}


void PictureWidget::resetPanAndZoom(void)
{
    if (mScrollArea) {
        mScrollArea->horizontalScrollBar()->setValue(0);
        mScrollArea->verticalScrollBar()->setValue(0);
    }
    mMouseSteps = 0;
    setZoom(1.0);
}


#ifdef WITH_KINETIC_SCROLLING
QPoint PictureWidget::mousePosInScrollArea(void) const
{
    Q_ASSERT(mScrollArea != NULL);
    return QPoint(mScrollArea->horizontalScrollBar()->value(), mScrollArea->verticalScrollBar()->value());
}
#endif


void PictureWidget::setScrollArea(QScrollArea* scrollArea)
{
    mScrollArea = scrollArea;
}


void PictureWidget::setPicture(const QImage& img, int stripePos, bool stripeVertical)
{
    mImage = img;
    mStripePos = stripePos;
    mStripeVertical = stripeVertical;
    update();
    setMinimumSize(mImage.size() * mZoom);
    resize(mImage.size() * mZoom);
}


void PictureWidget::setBrightnessData(
        const BrightnessData* brightness,
        const BrightnessData* red,
        const BrightnessData* green,
        const BrightnessData* blue,
        qreal avgBrightness,
        qreal avgRed,
        qreal avgGreen,
        qreal avgBlue,
        qreal minBrightness,
        qreal minRed,
        qreal minGreen,
        qreal minBlue)
{
    mBrightnessData = brightness;
    mRedData = red;
    mGreenData = green;
    mBlueData = blue;
    mAvgBrightness = avgBrightness;
    mAvgRed = avgRed;
    mAvgGreen = avgGreen;
    mAvgBlue = avgBlue;
    mMinBrightness = minBrightness;
    mMinRed = minRed;
    mMinGreen = minGreen;
    mMinBlue = minBlue;
    update();
}


void PictureWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.scale(mZoom, mZoom);
    painter.drawImage(QPoint(0, 0), mImage);
    painter.setBrush(Qt::NoBrush);
    if (mStripePos >= 0) {
        painter.setPen(Qt::red);
        if (mStripeVertical)
            painter.drawLine(mStripePos, 0, mStripePos, mImage.height());
        else
            painter.drawLine(0, mStripePos, mImage.width(), mStripePos);
    }
    if (mShowCurves && mBrightnessData && !mBrightnessData->isEmpty() && mMinBrightness >= 0) {
        const int y0l = mImage.height()     + mMinBrightness;
        const int y0r = mImage.height()*1/4 + mMinRed;
        const int y0g = mImage.height()*2/4 + mMinGreen;
        const int y0b = mImage.height()*3/4 + mMinBlue;
        painter.setPen(QColor(0xcc, 0xcc, 0xcc, 0xa0));
        if (mAvgBrightness >= 0)
            painter.drawLine(QLineF(0, y0l-mAvgBrightness, mImage.width(), y0l-mAvgBrightness));
        if (mAvgRed >= 0)
            painter.drawLine(QLineF(0, y0r-mAvgRed, mImage.width(), y0r-mAvgRed));
        if (mAvgGreen >= 0)
            painter.drawLine(QLineF(0, y0g-mAvgGreen, mImage.width(), y0g-mAvgGreen));
        if (mAvgBlue >= 0)
            painter.drawLine(QLineF(0, y0b-mAvgBlue, mImage.width(), y0b-mAvgBlue));

        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath lPath;
        lPath.moveTo(0, y0l);
        painter.setPen(QPen(QColor(0xff, 0xff, 0xff, 0xa0), 1.2));
        for (int i = 0; i < mBrightnessData->size(); ++i)
            lPath.lineTo(i, y0l-(*mBrightnessData)[i]);
        painter.drawPath(lPath);

        QPainterPath rPath;
        rPath.moveTo(0, y0r);
        painter.setPen(QPen(QColor(0xff, 0x00, 0x00, 0xa0), 1.2));
        for (int i = 0; i < mRedData->size(); ++i)
            rPath.lineTo(i, y0r-(*mRedData)[i]);
        painter.drawPath(rPath);

        QPainterPath gPath;
        gPath.moveTo(0, y0g);
        painter.setPen(QPen(QColor(0x00, 0xff, 0x00, 0xa0), 1.2));
        for (int i = 0; i < mGreenData->size(); ++i)
            gPath.lineTo(i, y0g-(*mGreenData)[i]);
        painter.drawPath(gPath);

        QPainterPath bPath;
        bPath.moveTo(0, y0b);
        painter.setPen(QPen(QColor(0x00, 0x00, 0xff, 0xa0), 1.2));
        for (int i = 0; i < mBlueData->size(); ++i)
            bPath.lineTo(i, y0b-(*mBlueData)[i]);
        painter.drawPath(bPath);

    }
}
