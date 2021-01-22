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
#include <QTimer>

void info(const char *format, ...);

RDPThread::RDPThread(MainWindow * w): QThread(w){
    client = NULL;
    event_process_timer = NULL;
    window = w;
}

[[noreturn]] void RDPThread::run() {
    TcpTool * tcptool = new TcpTool();
    XWin_Ui * xwin_ui = new XWin_Ui(800, 600, 32);
    client = new Client(xwin_ui, tcptool);
    char server[256] = "192.168.192.128";
    uint32 flags = RDP_LOGON_NORMAL;
    char domain[256] = {0};
    char password[256] = "123456";
    char shell[256] = {0};
    char directory[256] = {0};
    if (!client->rdp_connect(server, flags, domain, password, shell, directory))
        exit(-1);
    memset(password, 0, sizeof(password));

//    event_process_timer = new QTimer();
//    event_process_timer->setInterval(10);
//    connect(event_process_timer, SIGNAL(timeout()), this, SLOT(dispatch_message()));
//    event_process_timer->start();

    info("Initialize finished.");

    while (true){
        dispatch_message();
        client->rdp_main_loop();
        window->getPanel()->setPixmap(*client->getUi()->getPixmap());
    }
}

void RDPThread::dispatch_message(){
    info("in event process...");
    while (!events.empty()){
        QEvent * event = events.dequeue();
        info("event detected...");

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

RDPThread::~RDPThread() {
    event_process_timer->stop();
    delete event_process_timer;
    delete client;
}

