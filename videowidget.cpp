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
    mStripePos = QPoint(-1, -1);
    mStripeWidth = 1;
    mMouseButtonDown = false;
    mDrawingHistogram = false;
    mVerticalStripe = true;
    mHistogramEnabled = true;
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
    mStripePos = QPoint(pos, pos);
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


void VideoWidget::setHistogramRegion(const QRect& rect)
{
    mHistoRegion = rect;
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
    //
    // draw background
    //
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 30, 30));
    painter.drawRect(0, 0, width(), height());
    if (!mImage.isNull())
        painter.drawImage(mDestRect, mImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    //
    // draw marked histogram region
    //
    if (!mHistoRegion.isNull()) {
        if (mDrawingHistogram) {
            painter.setPen(QPen(QColor(0x30, 0xf0, 0x30, 0xc0), 1, Qt::DashLine));
            painter.setBrush(QBrush(QColor(0x30, 0xc0, 0x30, 0xc0), Qt::DiagCrossPattern));
        }
        else {
            painter.setPen(QColor(0x30, 0xf0, 0x00, 0x90));
            painter.setBrush(QColor(0x30, 0xc0, 0x00, 0x90));
        }
        QRect destRect;
        destRect.setTopLeft(toPosInWidget(mHistoRegion.topLeft()));
        destRect.setBottomRight(toPosInWidget(mHistoRegion.bottomRight()));
        painter.drawRect(destRect);
    }
    //
    // draw histogram
    //
    if (mHistogramEnabled) {
        const int hh = 128, x0 = 8, y0 = 8;
        painter.setPen(QColor(0xff, 0xff, 0xff, 0x66));
        painter.setBrush(QColor(0xff, 0xff, 0xff, 0x66));
        painter.drawRect(x0, y0, mHistogram.brightness().size(), hh);

        // painter.setRenderHint(QPainter::Antialiasing);

        const qreal scale = (qreal)hh / qMax(qMax(qMax(mHistogram.maxRed(), mHistogram.maxGreen()), mHistogram.maxBlue()), mHistogram.maxBrightness());

        QPainterPath rPath;
        painter.setPen(QColor(0xcc, 0x00, 0x00, 0x80));
        painter.setBrush(QColor(0x99, 0x00, 0x00, 0x80));
        const HistogramData& r = mHistogram.red();
        rPath.moveTo(x0, y0+hh-(int)(r[0]*scale));
        for (int i = 1; i < r.count(); ++i)
            rPath.lineTo(x0+i, y0+hh-(int)(r[i]*scale));
        rPath.lineTo(x0+r.count()-1, y0+hh);
        rPath.lineTo(x0, y0+hh);
        painter.drawPath(rPath);

        QPainterPath gPath;
        painter.setPen(QColor(0x00, 0xcc, 0x00, 0x80));
        painter.setBrush(QColor(0x00, 0x99, 0x00, 0x80));
        const HistogramData& g = mHistogram.green();
        gPath.moveTo(x0, y0+hh-(int)(g[0]*scale));
        for (int i = 1; i < g.count(); ++i)
            gPath.lineTo(x0+i, y0+hh-(int)(g[i]*scale));
        gPath.lineTo(x0+g.count()-1, y0+hh);
        gPath.lineTo(x0, y0+hh);
        painter.drawPath(gPath);

        QPainterPath bPath;
        painter.setPen(QColor(0x00, 0x00, 0xcc, 0x80));
        painter.setBrush(QColor(0x00, 0x00, 0xcc, 0x80));
        const HistogramData& b = mHistogram.blue();
        bPath.moveTo(x0, y0+hh-(int)(b[0]*scale));
        for (int i = 1; i < b.count(); ++i)
            bPath.lineTo(x0+i, y0+hh-(int)(b[i]*scale));
        bPath.lineTo(x0+b.count()-1, y0+hh);
        bPath.lineTo(x0, y0+hh);
        painter.drawPath(bPath);

        QPainterPath lPath;
        painter.setPen(QColor(0xff, 0xff, 0xff));
        painter.setBrush(Qt::NoBrush);
        const HistogramData& l = mHistogram.brightness();
        lPath.moveTo(x0, y0+hh-(int)(l[0]*scale));
        for (int i = 0; i < l.count(); ++i)
            lPath.lineTo(x0+i, y0+hh-(int)(l[i]*scale));
        painter.drawPath(lPath);

        painter.setPen(QColor(0x00, 0x00, 0x00, 0x80));
        painter.drawText(x0+6, y0+14, QString("%1").arg(mHistogram.totalBrightness()));
    }
    //
    // draw stripe or direction marker
    //
    painter.setPen(Qt::NoPen);
    QPoint sPos = toPosInWidget(mStripePos);
    if (mVerticalStripe) {
        if (mStripePos.x() >= 0) {
            painter.setBrush(QColor(0xff, 0x00, 0x00, 0xcc));
            painter.drawRect(sPos.x(), mDestRect.y(), mStripeWidth, mDestRect.height());
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
        if (mStripePos.y() >= 0) {
            painter.setBrush(QColor(0xff, 0x00, 0x00, 0xcc));
            painter.drawRect(mDestRect.x(), sPos.y(), mDestRect.width(), mStripeWidth);
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


QPoint VideoWidget::toPosInWidget(const QPoint& framePos) const
{
    return (mImage.isNull())? framePos :
            QPoint(mDestRect.x() + framePos.x() * mDestRect.width()  / mImage.width(),
                   mDestRect.y() + framePos.y() * mDestRect.height() / mImage.height());
}


QPoint VideoWidget::toPosInFrame(const QPoint& windowPos) const
{
    return (mImage.isNull())? windowPos :
            QPoint((windowPos.x() - mDestRect.x()) * mImage.width()  / mDestRect.width(),
                   (windowPos.y() - mDestRect.y()) * mImage.height() / mDestRect.height());
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


int VideoWidget::stripePos(void) const
{
    if (mVerticalStripe)
        return (mDestRect.width()  != 0)? mStripePos.x() : -1;
    else
        return (mDestRect.height() != 0)? mStripePos.y() : -1;
}


void VideoWidget::constrainMousePos(void)
{
    if (mMousePos.x() < mDestRect.x())
        mMousePos.setX(mDestRect.x());
    if (mMousePos.y() < mDestRect.y())
        mMousePos.setY(mDestRect.y());
    if (mMousePos.x() > mDestRect.bottomRight().x())
        mMousePos.setX(mDestRect.bottomRight().x()-1);
    if (mMousePos.y() > mDestRect.bottomRight().y())
        mMousePos.setY(mDestRect.bottomRight().y()-1);
}


void VideoWidget::keyPressEvent(QKeyEvent* event)
{
    if (mDraggingStripe && (event->modifiers() & Qt::AltModifier) == 0) {
        mVerticalStripe = ((event->modifiers() & Qt::ControlModifier) == 0);
        mStripePos = toPosInFrame(mMousePos);
        emit stripePosChanged(mVerticalStripe? mStripePos.x() : mStripePos.y());
        emit stripeOrientationChanged(mVerticalStripe);
        update();
    }
}


void VideoWidget::mouseMoveEvent(QMouseEvent* event)
{
    mMousePos = event->pos();
    if (mDrawingHistogram && (event->modifiers() & Qt::AltModifier)) {
        constrainMousePos();
        mMousePosInFrame = toPosInFrame(mMousePos);
        mHistoRegion.setBottomRight(mMousePosInFrame);
        update();
    }
    else if (mDraggingStripe) {
        bool vStripe = ((event->modifiers() & Qt::ControlModifier) == 0);
        if (vStripe != mVerticalStripe) {
            mVerticalStripe = vStripe;
            emit stripeOrientationChanged(mVerticalStripe);
        }
        mStripePos = toPosInFrame(mMousePos);
        emit stripePosChanged(mVerticalStripe? mStripePos.x() : mStripePos.y());
        update();
    }
}


void VideoWidget::mousePressEvent(QMouseEvent* event)
{
    mMouseButtonDown = (event->button() == Qt::LeftButton);
    mMousePos = event->pos();
    if (mMouseButtonDown) {
        setFocus(Qt::MouseFocusReason);
        if (event->modifiers() & Qt::AltModifier) {
            mDrawingHistogram = true;
            constrainMousePos();
            mMousePosInFrame = toPosInFrame(mMousePos);
            mHistoRegion.setTopLeft(mMousePosInFrame);
            mHistoRegion.setBottomRight(mMousePosInFrame);
            update();
        }
        else {
            mDraggingStripe = true;
        }
    }
}


void VideoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    mMouseButtonDown = false;
    mMousePos = event->pos();
    if (event->button() == Qt::LeftButton && mDrawingHistogram && (event->modifiers() & Qt::AltModifier)) {
        mDrawingHistogram = false;
        constrainMousePos();
        mMousePosInFrame = toPosInFrame(mMousePos);
        mHistoRegion.setBottomRight(mMousePosInFrame);
        emit histogramRegionChanged(mHistoRegion.normalized());
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
