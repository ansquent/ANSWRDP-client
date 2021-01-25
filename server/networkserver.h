#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include "networksocket.h"
#include <list>
#include <QTcpServer>
#include "user.h"

class NetworkServer:public QObject
{
    Q_OBJECT
public:
    NetworkServer();
private:
    QTcpServer *tcpServer;
    NetworkSocket *socket;
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
