/* Copyright (c) 2011 Oliver Lau <oliver@von-und-fuer-lau.de>
 * All rights reserved.
 * $Id$
 */

#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QtGlobal>

#include "videowidget.h"


VideoWidget::VideoWidget(QWidget* parent) : QWidget(parent)
{
    setAcceptDrops(true);
    setBaseSize(480, 270);
    setMinimumSize(384, 216);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mStripePos = -1;
    mStripeWidth = 1;
    mDragging = false;
    mVerticalStripe = true;
    mHistogramEnabled = true;
    mMarkMode = false;
}


int VideoWidget::stripePos(void) const
{
    if (mVerticalStripe)
        return (mDestRect.width() != 0)?  mStripePos * mImage.width()  / mDestRect.width()  : -1;
    else
        return (mDestRect.height() != 0)? mStripePos * mImage.height() / mDestRect.height() : -1;
}


void VideoWidget::calcDestRect(void) {
    if (mWindowAspectRatio < mFrameAspectRatio) {
        const int h = int(width() / mFrameAspectRatio);
        mDestRect = QRect(0, (height()-h)/2, width(), h);
    }
    else {
        const int w = int(height() * mFrameAspectRatio);
        mDestRect = QRect((width()-w)/2, 0, w, height());
    }
}


void VideoWidget::setFrameSize(const QSize& sz) {
    mFrameAspectRatio = qreal(sz.width()) / qreal(sz.height());
    calcDestRect();
    if (!mImage.isNull())
        update();
}


void VideoWidget::setStripeWidth(int stripeWidth)
{
    mStripeWidth = stripeWidth;
    if (!mImage.isNull())
        update();
}


void VideoWidget::setStripePos(int pos)
{
    mStripePos = pos;
    if (!mImage.isNull())
        update();
}


void VideoWidget::setStripeOrientation(bool vertical)
{
    mVerticalStripe = vertical;
    if (!mImage.isNull())
        update();
}

void VideoWidget::setHistogramEnabled(bool enabled)
{
    mHistogramEnabled = enabled;
    update();
}


void VideoWidget::setFrame(QImage img, Histogram histogram)
{
    mImage = img;
    mHistogram = histogram;
    update();
}


void VideoWidget::resizeEvent(QResizeEvent* e)
{
    mWindowAspectRatio = qreal(e->size().width()) / qreal(e->size().height());
    calcDestRect();
    update();
}


void VideoWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    // draw background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 30, 30));
    painter.drawRect(0, 0, width(), height());
    if (!mImage.isNull())
        painter.drawImage(mDestRect, mImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    if (mHistogramEnabled) {
        const int hh = 128, x0 = 8, y0 = 8;
        const qreal hs = (qreal)hh / mHistogram.maxBrightness();
        painter.setPen(QColor(0xff, 0xff, 0xff, 0x66));
        painter.setBrush(QColor(0xff, 0xff, 0xff, 0x66));
        painter.drawRect(x0, y0, mHistogram.data().size(), hh);
        painter.setPen(QColor(0x33, 0x33, 0x33, 0x80));
        painter.setBrush(Qt::NoBrush);
        const HistogramData& d = mHistogram.data();
        for (int i = 0; i < d.count(); ++i)
            painter.drawLine(x0+i, y0+hh, x0+i, y0+hh-(int)(d[i]*hs));
        painter.setPen(QColor(0x00, 0x00, 0x00, 0x80));
        painter.drawText(x0+6, y0+14, QString("%1").arg(mHistogram.totalBrightness()));
    }
    painter.setPen(QColor(0x33, 0xff, 0x00, 0xcc));
    painter.setBrush(QColor(0x33, 0xcc, 0x00, 0xcc));
    painter.drawRect(mMouseStartPos.x(), mMouseStartPos.y(), mMouseEndPos.x()-mMouseStartPos.x(), mMouseEndPos.y()-mMouseStartPos.y());
    // draw stripe or direction marker
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    if (mVerticalStripe) {
        if (mStripePos >= 0) {
            painter.setBrush(QColor(0xff, 0x00, 0x00, 0xcc));
            painter.drawRect(mStripePos + mDestRect.x(), mDestRect.y(), mStripeWidth, mDestRect.height());
        }
        else {
            painter.setPen(QColor(0x00, 0x00, 0xff, 0x99));
            painter.setBrush(QColor(0x00, 0x00, 0xff, 0x66));
            QPainterPath path;
            path.moveTo(width()/2+width()/20, height()/2);
            path.lineTo(width()/2-width()/20, height()/2-height()/10);
            path.lineTo(width()/2-width()/20, height()/2+height()/10);
            path.lineTo(width()/2+width()/20, height()/2);
            painter.drawPath(path);
        }
    }
    else {
        if (mStripePos >= 0) {
            painter.setBrush(QColor(0xff, 0x00, 0x00, 0xcc));
            painter.drawRect(mDestRect.x(), mStripePos + mDestRect.y(), mDestRect.width(), mStripeWidth);
        }
        else {
            painter.setPen(QColor(0x00, 0x00, 0xff, 0x99));
            painter.setBrush(QColor(0x00, 0x00, 0xff, 0x66));
            QPainterPath path;
            path.moveTo(width()/2,            height()/2+height()/20);
            path.lineTo(width()/2-width()/20, height()/2-height()/20);
            path.lineTo(width()/2+width()/20, height()/2-height()/20);
            path.lineTo(width()/2,            height()/2+height()/20);
            painter.drawPath(path);
        }
    }
}


void VideoWidget::calcStripePos(void)
{
    if (mVerticalStripe) {
        int sPos = mMousePos.x() - mDestRect.x();
        if (sPos != mStripePos) {
            mStripePos = sPos;
            if (mStripePos >= mDestRect.width())
                mStripePos = -1;
            emit stripePosChanged(stripePos());
        }
    }
    else {
        int sPos = mMousePos.y() - mDestRect.y();
        if (sPos != mStripePos) {
            mStripePos = sPos;
            if (mStripePos >= mDestRect.height())
                mStripePos = -1;
            emit stripePosChanged(stripePos());
        }
    }
}


void VideoWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_M)
        mMarkMode = !mMarkMode;
    if (mDragging && !mMarkMode) {
        mVerticalStripe = ((event->modifiers() & Qt::ControlModifier) == 0);
        calcStripePos();
        emit stripeOrientationChanged(mVerticalStripe);
        update();
    }
}


void VideoWidget::mouseMoveEvent(QMouseEvent* event)
{
    mMousePos = event->pos();
    if (mMarkMode) {
        mMouseEndPos = event->pos();
        update();
    }
    else {
        if (mDragging) {
            bool vStripe = ((event->modifiers() & Qt::ControlModifier) == 0);
            if (vStripe != mVerticalStripe) {
                mVerticalStripe = vStripe;
                emit stripeOrientationChanged(mVerticalStripe);
            }
            calcStripePos();
            update();
        }
    }
}


void VideoWidget::mousePressEvent(QMouseEvent* event)
{
    mDragging = (event->button() == Qt::LeftButton);
    if (mDragging)
        setFocus(Qt::MouseFocusReason);
    if (mMarkMode && mDragging)  {
        mMouseStartPos = event->pos();
        mMouseEndPos = event->pos();
    }
}


void VideoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && mMarkMode) {
        mMouseEndPos = event->pos();
        update();
    }
}

void VideoWidget::dragEnterEvent(QDragEnterEvent* event)
{
    // only accept local files
    if (event->mimeData()->hasUrls()) {
        bool accepted = true;
        foreach (const QUrl& url, event->mimeData()->urls()) {
            accepted &= !url.toLocalFile().isEmpty();
            if (!accepted)
                break;
        }
        if (accepted)
            event->acceptProposedAction();
    }
}


void VideoWidget::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl>& urls = event->mimeData()->urls();
        if (!urls.isEmpty())
            emit fileDropped(urls.first().toLocalFile());
    }
}
