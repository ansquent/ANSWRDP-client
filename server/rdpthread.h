//
// Created by User on 2021/1/22.
//

#ifndef RDESKTOP_WRAP_RDPTHREAD_H
#define RDESKTOP_WRAP_RDPTHREAD_H

#include <QThread>
#include <QEvent>
#include <QQueue>
#include <QTimer>
#include <QDataStream>
#include <QLabel>
#include <QTcpSocket>

class RDPXWin;

class RDPInvoker;

class RDPTcpTool;

class UserManager;

class RDPThread : public QThread {
Q_OBJECT
private:
    RDPInvoker *invoker;
    RDPTcpTool *tcptool;
    RDPXWin *xwin_ui;
    int width, height, bpp;
    UserManager * manager;
    QTcpSocket *client_socket;
    qintptr socketDescriptor;
    QString hostname;

    char server[256] = {0};
    char username[256] = {0};
    char password[256] = {0};
public:
    explicit RDPThread(qintptr sockintr, int width, int height, int bpp,
                       QString hostname, UserManager * manager);

    void dispatch_message();

    ~RDPThread() override;

private:
    bool initialized;

    void initialize_rdp();

    void initialize_client();

    void run() override;

    void initialize();

    void write_qimage();
};


#endif //RDESKTOP_WRAP_RDPTHREAD_H
