//
// Created by User on 2021/1/22.
//

#include "rdpthread.h"
#include "client.h"
#include "xwin.h"
#include "tcp.h"
#include "mainwindow.h"
#include <QQueue>
#include <QMouseEvent>

void info(const char *format, ...);

RDPThread::RDPThread(MainWindow * w){
    client = NULL;
    window = w;
    willclose = false;
}

void RDPThread::run() {
    tcptool = new TcpTool();
    XWin_Ui * xwin_ui = new XWin_Ui(800, 600, 32);
    char server[256] = "192.168.93.129";
    char username[256] = "Administrator";
    client = new Client(xwin_ui, tcptool, server, username);
    uint32 flags = RDP_LOGON_NORMAL | RDP_LOGON_AUTO;
    char domain[256] = {0};
    char password[256] = "123456";
    char shell[256] = {0};
    char directory[256] = {0};
    if (!client->rdp_connect(server, flags, domain, password, shell, directory))
        exit(-1);
    memset(password, 0, sizeof(password));

    info("Initialize finished.");
    while (true){
        if (willclose){
            delete client;
            client = NULL;
            return;
        }
        dispatch_message();
    }
}

void RDPThread::dispatch_message(){
    if (tcptool->get_ready()){
        client->rdp_main_loop();
        window->getPanel()->setPixmap(*client->getUi()->getPixmap());
    }
    else {
        tcptool->trynext();
    }
    static int i = 0;
    while (!events.empty()){
        QEvent * event = events.dequeue();
        info("event detected... %d", ++i);

        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * realEvent = (QMouseEvent *)event;
            uint16 flags = MOUSE_FLAG_DOWN;
            uint16 button = getClient()->xkeymap_translate_button(realEvent->button());
            if (button == 0)
                return;
            getClient()->rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
                                   flags | button, realEvent->x(), realEvent->y());
        }

    }
}

void RDPThread::push_event(QEvent *event){
    events.enqueue(event);
}

Client *RDPThread::getClient() {
    return client;
}

void RDPThread::setClose() {
    willclose = true;
}

