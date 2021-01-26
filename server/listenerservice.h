#ifndef LISTENERSERVICE_H
#define LISTENERSERVICE_H

#include <QCoreApplication>
#include <QtService>
#include <QApplication>
#include <QTcpServer>

class NetworkServer;

class ListenerService: public QtService<QCoreApplication>
{
public:    
    ListenerService(int, char **);
protected:
    void start();
    void pause();
    void resume();
private:
    NetworkServer * server;
};

#endif // LISTENERSERVICE_H
