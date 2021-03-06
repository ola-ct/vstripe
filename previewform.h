/* Copyright (c) 2011 Oliver Lau <oliver@von-und-fuer-lau.de>
 * All rights reserved.
 * $Id$
 */

#ifndef __PREVIEWFORM_H_
#define __PREVIEWFORM_H_

#include <QWidget>
#include <QSlider>
#include <QDial>
#include "picturewidget.h"
#include "kineticscroller.h"

namespace Ui {
    class PreviewForm;
}

class PreviewForm : public QWidget
{
    Q_OBJECT

public:
    PreviewForm(QWidget* parent = NULL);
    ~PreviewForm();
    void setSizeConstraints(const QSize& minimumSize, const QSize& optimumSize, const QSize& defaultSize, bool stripeIsFixed);
    PictureWidget* pictureWidget(void) { return mPictureWidget; }
    QSlider* brightnessSlider(void);
    QSlider* redSlider(void);
    QSlider* greenSlider(void);
    QSlider* blueSlider(void);
    QDial* factorDial(void);
    qreal amplificationCorrection(void) const;
    void setPictureSize(const QSize&);

public slots:
    void resetRGBLCorrections(void);
    void amplificationChanged(void);
    void choosePictureSize(void);

signals:
    void correctionsChanged(void);
    void visibilityChanged(bool);
    void pictureSizeChanged(QSize);

protected:
    void closeEvent(QCloseEvent*);
    void keyPressEvent(QKeyEvent*);

private:
    Ui::PreviewForm* ui;
    PictureWidget* mPictureWidget;
    bool mStripeIsVertical;
    bool mStripeIsFixed;
    QSize mDefaultSize;
    QSize mOptimumSize;
    static const QString WinTitle;
    KineticScroller* mKineticScroller;

};

#endif // __PREVIEWFORM_H_
