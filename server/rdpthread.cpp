#include "rdpthread.h"
#include "rdpinvoker.h"
#include "rdpxwin.h"
#include "rdptcp.h"
#include <QQueue>
#include <QString>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QImage>
#include <QTcpSocket>
#include <QThread>
#include "usermanager.h"
#include <QDataStream>
#include "constants.h"
#include "streammanager.h"

RDPThread::RDPThread(qintptr socketDescriptor, int width, int height, int bpp,
                     QString hostname, UserManager * manager){
    initialized = false;

    this->socketDescriptor = socketDescriptor;
    this->width = width;
    this->height = height;
    this->bpp = bpp;
    this->hostname = hostname;
    this->manager = manager;
}

void RDPThread::initialize_rdp(){
    this->tcptool = new RDPTcpTool();
    this->xwin_ui = new RDPXWin(width, height, bpp);
    strcpy(this->server, hostname.toStdString().c_str());
    this->invoker = new RDPInvoker(xwin_ui, tcptool, this->server, this->username);
    uint32 flags = RDP_LOGON_NORMAL | RDP_LOGON_AUTO;
    char domain[256] = {0};
    char shell[256] = {0};
    char directory[256] = {0};
    if (!invoker->rdp_connect(this->server, flags, domain, this->password, shell, directory)) {
        info("Connect failed.");
        this->tcptool->tcp_disconnect();
        deleteLater();
    }
    memset(this->password, 0, sizeof(this->password));
}

void RDPThread::initialize_client(){
    this->client_socket = new QTcpSocket();
    this->client_socket->setSocketDescriptor(socketDescriptor);
    QString username, password;
    QString command;
    BlockReader(client_socket).stream() >> command >> username >> password;
    info("command=%s username=%s password=%s", command.toStdString().c_str(),
         username.toStdString().c_str(), password.toStdString().c_str());
    if (command == "login"){
       if (manager->login(username, password)){
//            if (!manager->runProgram(obj.value("programname").toString())){
//                addToQByteArray(OPEN_PROGRAM_FAIL, returns);
//            }
//            else {
//                addToQByteArray(LOGIN_USER_SUCCESS, returns);
//            }
           QString win32username, win32password;
           manager->queryUser(username, password, win32username, win32password);
           BlockWriter(client_socket).stream() << QString::fromLatin1("success");
           client_socket->flush();
           strcpy(this->username, win32username.toStdString().c_str());
           strcpy(this->password, win32password.toStdString().c_str());
           return;
       }
       else {
//           BlockWriter(client_socket).stream() << QString::fromLatin1("fail");
           //debug
           QString win32username, win32password;
           BlockWriter(client_socket).stream() << QString::fromLatin1("success");
           client_socket->flush();
           strcpy(this->username, "Administrator");
           strcpy(this->password, "123456");
           return;
       }
    }
    else if (command == "register"){
        if (manager->reg(username, password)){
            BlockWriter(client_socket).stream() << QString::fromLatin1("success");
        }
        else {
            BlockWriter(client_socket).stream() << QString::fromLatin1("fail");
        }
    }
    else {
        BlockWriter(client_socket).stream() << QString::fromLatin1("not defined");
    }
    client_socket->flush();
    client_socket->close();
    deleteLater();
}

void RDPThread::initialize(){
    initialize_client();
    initialize_rdp();
    initialized = true;
    info("Initialize finished.");
}

void RDPThread::run() {
    if (!initialized){
        initialize();
    }
    qDebug() << "Thread id:" << this->currentThreadId();
    static int i = 0;
    while (true){
        if (tcptool->get_closed()){
            break;
        }
        else if (tcptool->get_ready()) {
            invoker->rdp_main_loop();
            write_qimage();
        } else {
            tcptool->trynext();
        }

        if (!(client_socket->isOpen() && client_socket->isValid())){
            break;
        }
        else if (client_socket->bytesAvailable() > 0){
            dispatch_message();
        } else {
            client_socket->waitForReadyRead(0);
        }
    }

    client_socket->disconnectFromHost();
    client_socket->close();
    tcptool->disconnect();
    deleteLater();
}

void RDPThread::write_qimage(){
    QImage image = xwin_ui->getPixmap()->toImage();
    qDebug() << "image size: " << image.sizeInBytes();
    BlockWriter(client_socket).stream() << image;
    client_socket->flush();
}

void RDPThread::dispatch_message() {
    int type;
    BlockReader(client_socket).stream() >> type;
    if (type == MOUSEPRESSEVENT) {
        int button, x, y;
        BlockReader(client_socket).stream() >> button >> x >> y;
        info("press button=%d x=%d y=%d\n", button, x, y);
        uint16 flags = MOUSE_FLAG_DOWN;
        uint16 nativebutton = RDPInvoker::xkeymap_translate_button(Qt::MouseButton(button));
        if (nativebutton == 0)
            return;
        invoker->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE, flags | nativebutton, x, y);
    } else if (type == MOUSESCROLLEVENT) {
        int delta, x, y;
        BlockReader(client_socket).stream() >> delta >> x >> y;
        uint16 flags = 0;
        uint16 button;
        if (delta > 0) {
            button = MOUSE_FLAG_BUTTON4;
        } else {
            button = MOUSE_FLAG_BUTTON5;
        }
        info("scroll delta=%d x=%d y=%d\n", delta, x, y);
        invoker->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE, flags | button, x, y);
    } else if (type == MOUSERELEASEEVENT) {
        int button, x, y;
        BlockReader(client_socket).stream() >> button >> x >> y;
        uint16 flags = 0;
        uint16 nativebutton = RDPInvoker::xkeymap_translate_button(Qt::MouseButton(button));
        if (button == 0)
            return;
        info("release button=%d x=%d y=%d\n", button, x, y);
        invoker->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE, flags | nativebutton, x, y);
    } else if (type == MOUSEMOVEEVENT) {
        int x, y;
        BlockReader(client_socket).stream() >> x >> y;
        info("move x=%d y=%d\n", x, y);
        invoker->rdp_send_input(time(nullptr), RDP_INPUT_MOUSE, MOUSE_FLAG_MOVE, x, y);
    } else if (type == KEYDOWNEVENT) {
        int scancode, nativekey;
        BlockReader(client_socket).stream() >> scancode >> nativekey;
#ifdef linux
        scancode -= invoker->getminkeycode();
#endif
        uint32 ev_time = time(nullptr);
//            if (client->handle_special_keys(realEvent->nativeVirtualKey(), ev_time, true))
//                return;
        if (scancode == 0)
            return;
        info("keycode = %d\n", scancode);
        invoker->rdp_send_scancode(ev_time, RDP_KEYPRESS, scancode);
    } else if (type == KEYUPEVENT) {
        int scancode, nativekey;
        BlockReader(client_socket).stream() >> scancode >> nativekey;
#ifdef linux
        scancode -= invoker->getminkeycode();
#endif
        uint32 ev_time = time(nullptr);
//            if (client->handle_special_keys(realEvent->nativeVirtualKey(), ev_time, false))
//                return;
        if (scancode == 0)
            return;
        info("keycode = %d\n", scancode);
        invoker->rdp_send_scancode(ev_time, RDP_KEYRELEASE, scancode);
    }
}

RDPThread::~RDPThread() {
    client_socket->disconnectFromHost();
    client_socket->close();
    delete client_socket;
    delete invoker;
    delete tcptool;
    delete xwin_ui;
}

