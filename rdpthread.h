//
// Created by User on 2021/1/22.
//

#ifndef RDESKTOP_WRAP_RDPTHREAD_H
#define RDESKTOP_WRAP_RDPTHREAD_H

#include <QThread>
#include <QEvent>
#include <QQueue>
#include <QTimer>
class XWin_Ui;
class Client;
class MainWindow;
class TcpTool;

class RDPThread : public QThread {
Q_OBJECT
private:
    Client * client;
    MainWindow * window;
    QQueue<QEvent *> events;
    TcpTool * tcptool;
    bool willclose;
public:
    explicit RDPThread(MainWindow *);

    void run() override;

    void push_event(QEvent *);

    Client * getClient();

    void setClose();
private slots:
    void dispatch_message();
};


#endif //RDESKTOP_WRAP_RDPTHREAD_H
