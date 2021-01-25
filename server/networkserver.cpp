#include "networkserver.h"
#include <QTcpServer>
#include "networksocket.h"
#include "util.h"
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>

NetworkServer::NetworkServer():user("127.0.0.1"){
    tcpServer = new QTcpServer();
}

void NetworkServer::start(){
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(resume()));
    if (!tcpServer->listen(QHostAddress::Any, 8888)){
        info("Failed in 8888\n");
    }
}

void NetworkServer::pause(){
    disconnect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    disconnect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));
    disconnect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readUserSocketData()));
    disconnect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectUserData()));
    delete socket;
}

void NetworkServer::resume(){
    socket = new NetworkSocket(tcpServer->nextPendingConnection());
    connect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    connect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));
    connect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readUserSocketData()));
    connect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectUserData()));
}

void NetworkServer::readUserSocketData(){
    NetworkSocket * socket = (NetworkSocket *)(sender());

    if (readNow(socket->getMySocket(), socket->getExpectedLength(),
                *socket->getBuffer(), *socket->getLengthBuffer())){
        QByteArray * result = socket->getBuffer();
        QJsonObject obj = QJsonDocument::fromJson(*result).object();        
        QByteArray returns;
        returns.clear();
        if (obj.value("command") == "login"){
            if (user.login(obj.value("username").toString(), obj.value("password").toString())){
                if (!user.runProgram(obj.value("programname").toString())){
                    addToQByteArray(OPEN_PROGRAM_FAIL, returns);
                }
                else {
                    addToQByteArray(LOGIN_USER_SUCCESS, returns);
                }
            }
            else {
                addToQByteArray(LOGIN_USER_FAIL, returns);
            }
        }
        else if (obj.value("command") == "register"){
            if (user.reg(obj.value("username").toString(), obj.value("password").toString())){
                addToQByteArray(CREATE_USER_SUCCESS, returns);
            }
            else {
                addToQByteArray(CREATE_USER_FAIL, returns);
            }
        }
        else {
            addToQByteArray(NO_COMMAND, returns);
        }
        socket->getBuffer()->clear();
        socket->getLengthBuffer()->clear();
        writeNow(socket->getMySocket(), returns);
    }
    else
        /* Do nothing */;
}

void NetworkServer::disconnectUserData(){
    NetworkSocket * socket = (NetworkSocket *)(sender());
    socket->getMySocket()->close();
}


