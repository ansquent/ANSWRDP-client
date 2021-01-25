#ifndef LISTENERSERVICE_H
#define LISTENERSERVICE_H

#include <QCoreApplication>
#include <QtService>
#include <QApplication>
#include <QTcpServer>
#include "server.h"

class ListenerService: public QtService<QCoreApplication>
{
public:    
    ListenerService(int, char **);
protected:
    void start();
    void pause();
    void resume();
private:
    Server * server;
};

#endif // LISTENERSERVICE_H
