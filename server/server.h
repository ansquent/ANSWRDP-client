#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include "mysocket.h"
#include <list>
#include <QTcpServer>
#include "database.h"
#include "user.h"

class Server:public QObject
{
    Q_OBJECT
public:
    Server();
private:
    QTcpServer *tcpServer;
    MySocket *socket;
    User user;
public slots:
    void readUserSocketData();
    void disconnectUserData();
    void pause();
    void resume();
public:
    void start();
};

#endif // SOCKET_H
