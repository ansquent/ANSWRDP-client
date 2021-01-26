#include "networkserver.h"
#include <QTcpServer>
#include <QThread>
#include "rdpthread.h"
#include "constants.h"

NetworkServer::NetworkServer(UserManager * manager){
    this->manager = manager;
}

void NetworkServer::incomingConnection(qintptr socketDescriptor)
{
    info("SocketDescriptor: %d", socketDescriptor);
    RDPThread * thread = new RDPThread(socketDescriptor, 800, 600, 32,
                                    "192.168.199.129", manager);
    threads.push_back(thread);
    thread->start();
}

void NetworkServer::closeAll(){
    for (int i = 0; i < threads.size(); ++i){
        threads[i]->quit();
        threads[i]->wait();
        delete threads[i];
    }
    threads.clear();
}

NetworkServer::~NetworkServer(){
    closeAll();
}


