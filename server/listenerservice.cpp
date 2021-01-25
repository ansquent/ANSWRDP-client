#include "listenerservice.h"
#include <QDebug>
#include <QCoreApplication>
#include <QObject>
#include "util.h"

ListenerService::ListenerService(int argc, char ** argv):
    QtService<QCoreApplication>(argc, argv, "RDPListenerAndProcessCreator")
{
    setServiceDescription("A service implemented with Qt.");
    setServiceFlags(QtServiceBase::CanBeSuspended);
    server = new Server();
    Util::writeToFile("C:\\1.txt", QString("service").toUtf8());
}

void ListenerService::start(){
    server->start();
}

void ListenerService::pause(){
    server->pause();
}

void ListenerService::resume(){
    server->resume();
}
