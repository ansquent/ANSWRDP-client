#include "listenerservice.h"
#include <QCoreApplication>
#include <QObject>
#include "util.h"

ListenerService::ListenerService(int argc, char ** argv):
    QtService<QCoreApplication>(argc, argv, "RDPListenerAndProcessCreator")
{
    setServiceDescription("A service implemented with Qt.");
    setServiceFlags(QtServiceBase::CanBeSuspended);
    server = new NetworkServer();
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
