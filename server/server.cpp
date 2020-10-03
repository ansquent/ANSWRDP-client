#include "server.h"
#include <QTcpServer>
#include "mysocket.h"
#include "config.h"
#include <QThread>
#include "util.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "config.h"

Server::Server(){
    tcpServer = new QTcpServer();
}

void Server::start(){
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(resume()));
    if (!tcpServer->listen(QHostAddress::Any, 8888)){
        qDebug() << "Failed in 8888\n";
    }
}

void Server::pause(){
    disconnect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    disconnect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));
    disconnect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readUserSocketData()));
    disconnect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectUserData()));
    delete socket;
}

void Server::resume(){
    socket = new MySocket(tcpServer->nextPendingConnection());
    connect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    connect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));
    connect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readUserSocketData()));
    connect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectUserData()));
}

void Server::readUserSocketData(){
    MySocket * socket = (MySocket *)(sender());

    if (Util::readNow(socket)){
        QByteArray * result = socket->getBuffer();
        QJsonObject obj = QJsonDocument::fromJson(*result).object();        
        QByteArray returns;
        returns.clear();
        if (obj.value("command") == "login"){
            if (user.login(obj.value("username").toString(), obj.value("password").toString())){
                if (!user.runProgram(obj.value("programname").toString())){
                    Util::addToQByteArray(OPEN_PROGRAM_FAIL, returns);
                }
                else {
                    Util::addToQByteArray(LOGIN_USER_SUCCESS, returns);
                }
            }
            else {
                Util::addToQByteArray(LOGIN_USER_FAIL, returns);
            }
        }
        else if (obj.value("command") == "register"){
            if (user.reg(obj.value("username").toString(), obj.value("password").toString())){
                Util::addToQByteArray(CREATE_USER_SUCCESS, returns);
            }
            else {
                Util::addToQByteArray(CREATE_USER_FAIL, returns);
            }
        }
        else {
            Util::addToQByteArray(NO_COMMAND, returns);
        }
        socket->getBuffer()->clear();
        socket->getLengthBuffer()->clear();
        Util::writeNow(socket->getMySocket(), returns);
    }
    else
        /* Do nothing */;
}

void Server::disconnectUserData(){
    MySocket * socket = (MySocket *)(sender());
    socket->getMySocket()->close();
}


