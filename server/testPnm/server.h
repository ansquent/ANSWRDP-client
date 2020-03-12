#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include "mysocket.h"
#include <list>
#include <QTcpServer>
#include "userinfo.h"
#include "database.h"

class Server:public QObject
{
    Q_OBJECT
public:
    Server();
private:
    void sendState(MySocket *, int);
    void checkReg(const QByteArray & bytes, MySocket * socket);

    QTcpServer *imageServer, *userServer;
    std::list<Userinfo> connectedUsers;
    Database db;
    QMutex imagerActiveMutex;
public slots:
    void startUserServer();
    void startImageServer();
    void readUserSocketData();
    void disconnectUserData();
};

#endif // SOCKET_H
