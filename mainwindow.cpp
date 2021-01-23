#include "mainwindow.h"
#include <QLabel>
#include <QThread>
#include <QPaintEvent>
#include <QTimer>
#include <QThread>
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
    setAttribute( Qt::WA_Hover, true);
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
//    uint32 ev_time = time(NULL);
//    if (client->handle_special_keys(event->key(), ev_time, True))
//        return;
//    if (event->nativeScanCode() == 0)
//        return;
//    //ensure_remote_modifiers(ev_time, tr);
//    client->rdp_send_scancode(ev_time, RDP_KEYPRESS, event->nativeScanCode());
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
//    uint32 ev_time = time(NULL);
//    if (client->handle_special_keys(event->key(), ev_time, False))
//        return;
//    if (event->nativeScanCode() == 0)
//        return;
//    client->rdp_send_scancode(ev_time, RDP_KEYRELEASE, event->nativeScanCode());
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

void MainWindow::wheelEvent(QWheelEvent *event) {
//    uint16 flags = 0;
//    uint16 button;
//    if (event->delta() > 0) {
//        button = MOUSE_FLAG_BUTTON4;
//    } else {
//        button = MOUSE_FLAG_BUTTON5;
//    }
//    client->rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
//                           flags | button, event->x(), event->y());
}

MainWindow::~MainWindow() {
    rdpThread->setClose();
}

void MainWindow::paint() {
    panel->setPixmap(*rdpThread->getClient()->getUi()->getPixmap());
    repaint();
}


