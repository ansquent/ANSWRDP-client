//
// Created by User on 2021/1/22.
//

#ifndef RDESKTOP_WRAP_RDPTHREAD_H
#define RDESKTOP_WRAP_RDPTHREAD_H

#include <QThread>
#include <QEvent>
#include <QQueue>
#include <QTimer>
#include <QMainWindow>

class RDPXWin;

class RDPInvoker;

class MainWindow;

class RDPTcpTool;

class RDPThread : public QThread {
Q_OBJECT
private:
    RDPInvoker *client;
    QQueue<QEvent *> events;
    RDPTcpTool *tcptool;
    RDPXWin *xwin_ui;
    bool willclose;
    int width, height, bpp;
    char server[256] = {0};
    char username[256] = {0};
    char password[256] = {0};
public:
    explicit RDPThread(int width, int height, int bpp,
                       QString hostname, QString username, QString password);

    void run() override;

    void push_event(QEvent *);

    RDPInvoker *getClient();

    void setClose();

    ~RDPThread() override;

private slots:

    void dispatch_message();

signals:

    void paint();
};


#endif //RDESKTOP_WRAP_RDPTHREAD_H
