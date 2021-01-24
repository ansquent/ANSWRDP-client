#include "mainwindow.h"
#include <QLabel>
#include <QPaintEvent>
#include <QTimer>
#include "rdpthread.h"
#include "xwin.h"
#include "client.h"

void info(const char *format, ...);

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
    setGeometry(QRect(0, 0, 1200, 800));
    panel = new QLabel(this);
    panel->setGeometry(0, 0, 1200, 800);
    rdpThread = new RDPThread(1200, 800, 32);
    rdpThread->start();
    connect(rdpThread, SIGNAL(paint()), this, SLOT(paint()));
    setAttribute(Qt::WA_Hover, true);
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
    rdpThread->push_event(new QKeyEvent(*event));
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    rdpThread->push_event(new QKeyEvent(*event));
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    rdpThread->push_event(new QMouseEvent(*event));
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    rdpThread->push_event(new QMouseEvent(*event));
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    rdpThread->push_event(new QMouseEvent(*event));
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    rdpThread->push_event(new QMouseEvent(*event));
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    rdpThread->push_event(new QWheelEvent(*event));
}

MainWindow::~MainWindow() {
    rdpThread->setClose();
}

void MainWindow::paint() {
    panel->setPixmap(*rdpThread->getClient()->getUi()->getPixmap());
    repaint();
}


