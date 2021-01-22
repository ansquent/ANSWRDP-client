//
// Created by User on 2021/1/22.
//

#ifndef RDESKTOP_WRAP_RDPTHREAD_H
#define RDESKTOP_WRAP_RDPTHREAD_H

#include <QThread>
class XWin_Ui;
class Client;

class RDPThread : public QThread {

private:
    Client * client;
public:
    explicit RDPThread();

    [[noreturn]] void run() override;

    Client * getClient();

    ~RDPThread() override;
};


#endif //RDESKTOP_WRAP_RDPTHREAD_H
