//
// Created by User on 2021/1/22.
//

#include "rdpthread.h"
#include "rdpinvoker.h"
#include "rdpxwin.h"
#include "rdptcp.h"
#include <QQueue>
#include <QMouseEvent>
#include <QString>
#include <QMessageBox>
#include "util.h"

RDPThread::RDPThread(int width, int height, int bpp,
                     QString hostname, QString username, QString password){
    this->setWindowTitle("Main - Rdesktop");
    setGeometry(QRect(0, 0, width, height));
    panel = new QLabel(this);
    panel->setGeometry(0, 0, width, height);
    setAttribute(Qt::WA_Hover, true);
    this->width = width;
    this->height = height;
    this->bpp = bpp;
    willclose = false;
    strcpy(this->server, hostname.toStdString().c_str());
    strcpy(this->username, username.toStdString().c_str());
    strcpy(this->password, password.toStdString().c_str());

    tcptool = new RDPTcpTool();
    xwin_ui = new RDPXWin(width, height, bpp);
    client = new RDPInvoker(xwin_ui, tcptool, this->server, this->username);
    uint32 flags = RDP_LOGON_NORMAL | RDP_LOGON_AUTO;
    char domain[256] = {0};
    char shell[256] = {0};
    char directory[256] = {0};
    if (!client->rdp_connect(this->server, flags, domain, this->password, shell, directory)) {
        info("Connect failed.");
        this->destroy();
    }
    memset(this->password, 0, sizeof(this->password));
    timer = new QTimer();
    timer->setInterval(10);
    connect(timer, SIGNAL(timeout()), this, SLOT(run()));
    timer->start();
    info("Initialize finished.");
}

void RDPThread::run() {
    if (willclose) {
        return;
    } else if (tcptool->get_ready()) {
        client->rdp_main_loop();
        panel->setPixmap(*client->getUi()->getPixmap());
    } else {
        tcptool->trynext();
    }
}

void RDPThread::keyPressEvent(QKeyEvent *event) {
    QKeyEvent *realEvent = (QKeyEvent *) event;
    int scancode = realEvent->nativeScanCode();
#ifdef linux
    scancode -= client->getminkeycode();
#endif
    uint32 ev_time = time(nullptr);
//            if (client->handle_special_keys(realEvent->nativeVirtualKey(), ev_time, true))
//                return;
    if (scancode == 0)
        return;
    info("keycode = %d\n", scancode);
    client->rdp_send_scancode(ev_time, RDP_KEYPRESS, scancode);
}

void RDPThread::keyReleaseEvent(QKeyEvent *event) {
    QKeyEvent *realEvent = (QKeyEvent *) event;
    int scancode = realEvent->nativeScanCode();
#ifdef linux
    scancode -= client->getminkeycode();
#endif
    uint32 ev_time = time(nullptr);
//            if (client->handle_special_keys(realEvent->nativeVirtualKey(), ev_time, false))
//                return;
    if (scancode == 0)
        return;
    info("keycode = %d\n", scancode);
    client->rdp_send_scancode(ev_time, RDP_KEYRELEASE, scancode);
}

void RDPThread::mouseMoveEvent(QMouseEvent *realEvent) {
    client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                           MOUSE_FLAG_MOVE, realEvent->x(), realEvent->y());
}

void RDPThread::mousePressEvent(QMouseEvent *realEvent) {
    uint16 flags = MOUSE_FLAG_DOWN;
    uint16 button = RDPInvoker::xkeymap_translate_button(realEvent->button());
    if (button == 0)
        return;
    client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                                flags | button, realEvent->x(), realEvent->y());
}

void RDPThread::mouseReleaseEvent(QMouseEvent *realEvent) {
    uint16 flags = 0;
    uint16 button = RDPInvoker::xkeymap_translate_button(realEvent->button());
    if (button == 0)
        return;
    client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                           flags | button, realEvent->x(), realEvent->y());
}

void RDPThread::mouseDoubleClickEvent(QMouseEvent *realEvent) {
    uint16 flags = MOUSE_FLAG_DOWN;
    uint16 button = RDPInvoker::xkeymap_translate_button(realEvent->button());
    if (button == 0)
        return;
    client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                                flags | button, realEvent->x(), realEvent->y());
}

void RDPThread::wheelEvent(QWheelEvent *realEvent) {
    uint16 flags = 0;
    uint16 button;
    if (realEvent->delta() > 0) {
        button = MOUSE_FLAG_BUTTON4;
    } else {
        button = MOUSE_FLAG_BUTTON5;
    }
    client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                           flags | button, realEvent->x(), realEvent->y());
}

void RDPThread::setClose() {
    willclose = true;
}

RDPThread::~RDPThread() {
    delete client;
    delete tcptool;
    delete xwin_ui;
}

