/*
 * Copyright (c) 2011 Oliver Lau <oliver@ersatzworld.net>
 * $Id$
 */

#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
