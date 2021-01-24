//
// Created by User on 2021/1/22.
//

#include "rdpthread.h"
#include "client.h"
#include "xwin.h"
#include "tcp.h"
#include <QQueue>
#include <QMouseEvent>

void info(const char *format, ...);

RDPThread::RDPThread(int width, int height, int bpp) {
    this->width = width;
    this->height = height;
    this->bpp = bpp;
    client = nullptr;
    willclose = false;
    tcptool = nullptr;
    xwin_ui = nullptr;
}

void RDPThread::run() {
    tcptool = new TcpTool();
    xwin_ui = new XWin_Ui(width, height, bpp);
    char server[256] = "192.168.199.129";
    char username[256] = "Administrator";
    client = new Client(xwin_ui, tcptool, server, username);
    uint32 flags = RDP_LOGON_NORMAL | RDP_LOGON_AUTO;
    char domain[256] = {0};
    char password[256] = "123456";
    char shell[256] = {0};
    char directory[256] = {0};
    if (!client->rdp_connect(server, flags, domain, password, shell, directory)) {
        deleteLater();
        return;
    }
    memset(password, 0, sizeof(password));

    info("Initialize finished.");
    while (true) {
        if (willclose) {
            deleteLater();
            return;
        }
        dispatch_message();
    }
}

void RDPThread::dispatch_message() {
    if (tcptool->get_ready()) {
        client->rdp_main_loop();
        emit paint();
    } else {
        tcptool->trynext();
    }
    static int i = 0;
    while (!events.empty()) {
        QEvent *event = events.dequeue();
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *realEvent = (QMouseEvent *) event;
            uint16 flags = MOUSE_FLAG_DOWN;
            uint16 button = Client::xkeymap_translate_button(realEvent->button());
            if (button == 0)
                return;
            getClient()->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                                        flags | button, realEvent->x(), realEvent->y());
        } else if (event->type() == QEvent::Wheel) {
            QWheelEvent *realEvent = (QWheelEvent *) event;
            uint16 flags = 0;
            uint16 button;
            if (realEvent->delta() > 0) {
                button = MOUSE_FLAG_BUTTON4;
            } else {
                button = MOUSE_FLAG_BUTTON5;
            }
            client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                                   flags | button, realEvent->x(), realEvent->y());
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *realEvent = (QMouseEvent *) event;
            uint16 flags = 0;
            uint16 button = Client::xkeymap_translate_button(realEvent->button());
            if (button == 0)
                return;
            client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                                   flags | button, realEvent->x(), realEvent->y());
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *realEvent = (QMouseEvent *) event;
            client->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE,
                                   MOUSE_FLAG_MOVE, realEvent->x(), realEvent->y());
        } else if (event->type() == QEvent::KeyPress) {
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
        } else if (event->type() == QEvent::KeyRelease) {
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
        delete event;
    }
}

void RDPThread::push_event(QEvent *event) {
    events.enqueue(event);
}

Client *RDPThread::getClient() {
    return client;
}

void RDPThread::setClose() {
    willclose = true;
}

RDPThread::~RDPThread() {
    while (!events.empty()) {
        QEvent *event = events.dequeue();
        delete event;
    }
    delete client;
    delete tcptool;
    delete xwin_ui;
}

