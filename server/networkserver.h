#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <list>
#include <QTcpServer>
#include <QVector>

class RDPThread;

class UserManager;

class NetworkServer:public QTcpServer
{
    Q_OBJECT
public:
    NetworkServer(UserManager *);
    void incomingConnection(qintptr handle) override;
    void closeAll();
    ~NetworkServer() override;
private:
    QVector<RDPThread *> threads;
    UserManager * manager;
};

#endif // SOCKET_H
