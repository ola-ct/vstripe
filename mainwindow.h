/* Copyright (c) 2011 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 * $Id$
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QString>
#include <QMainWindow>


#include "videoreaderthread.h"
#include "videowidget.h"
#include "picturewidget.h"
#include "markableslider.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = NULL);
    ~MainWindow();

    QSize minimumSizeHint(void) const { return QSize(720, 576); }
    QSize sizeHint(void) const { return QSize(1280, 720); }

    static const QString Company;
    static const QString AppName;

public slots:
    void openVideoFile(void);
    void closeVideoFile(void);
    void openRecentFile(void);
    void loadVideoFile(void);
    void decodingFinished(void);
    void togglePictureWidget(bool);
    void frameChanged(int);
    void forward(int nFrames = 1);
    void backward(int nFrames = 1);
    void fastForward(void);
    void fastBackward(void);
    void setMarkA(void);
    void setMarkB(void);
    void savePicture(void);
    void showPercentReady(int);
    void frameReady(QImage, int);
    void renderButtonClicked(void);
    void setParamsButtonClicked(void);
    void about(void);
    void pictureWidthSet(int);


protected:
    void closeEvent(QCloseEvent*);

private: // variables
    Ui::MainWindow* ui;
    QString mVideoFileName;
    MarkableSlider* mFrameSlider;
    VideoWidget* mVideoWidget;
    PictureWidget* mPictureWidget;
    VideoReaderThread* mVideoReaderThread;
    int markA;
    int markB;
    int mStripeWidth; // Streifen dieser Breite (Pixel) werden von jedem eingelesenen Frame behalten
    qreal mFrameSkip; // so viel Frames werden pro Frame beim Einlesen übersprungen
    bool mFixedStripe; // true: der Streifen bleibt fest an der gewählten Position; false: der Streifen bewegt sich mit jedem Frame um mStripeWidth Pixel weiter
    int mFrameCount;
    QImage mFrame;
    int mEffectiveFrameNumber;
    int mEffectiveFrameTime;

    static const int MaxRecentFiles = 5;
    QAction* recentFileActs[MaxRecentFiles];

private: // methods
    void showPictureWidget(void);
    void hidePictureWidget(void);
    void render(void);
    void stopRendering(void);
    void enableGuiButtons(void);
    void disableGuiButtons(void);
    void restoreAppSettings(void);
    void saveAppSettings(void);
    void setCurrentFile(const QString& fileName);
    void updateRecentFileActions(void);
    QString strippedName(const QString& fullFileName);
};

#endif // MAINWINDOW_H
