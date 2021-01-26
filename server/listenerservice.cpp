#include "listenerservice.h"
#include <QCoreApplication>
#include <QObject>
#include "networkserver.h"
#include "constants.h"

ListenerService::ListenerService(int argc, char ** argv):
    QtService<QCoreApplication>(argc, argv, "RDPListenerAndProcessCreator")
{
    setServiceDescription("A service implemented with Qt.");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

void ListenerService::start(){
    if (!server->listen(QHostAddress::Any, 8888)){
        info("Failed in 8888\n");
    }
}

void ListenerService::pause(){
    server->close();
    server->closeAll();
}

void ListenerService::resume(){
    if (!server->listen(QHostAddress::Any, 8888)){
        info("Failed in 8888\n");
    }
}
