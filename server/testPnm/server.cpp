#include "server.h"
#include <QTcpServer>
#include "mysocket.h"
#include "config.h"
#include <QThread>
#include "imagesender.h"
#include "reader.h"
#include "writer.h"

Server::Server(){
    userServer = new QTcpServer();
    imageServer = new QTcpServer();
    connect(userServer, SIGNAL(newConnection()), this, SLOT(startUserServer()));
    connect(imageServer, SIGNAL(newConnection()), this, SLOT(startImageServer()));
    if (!userServer->listen(QHostAddress::Any, 12345)){
        qDebug() << "Failed in 12345\n";
    }
    if (!imageServer->listen(QHostAddress::Any, 5900)){
        qDebug() << "Failed in 5900\n";
    }
}

void Server::sendState(MySocket * socket, int state){
    QByteArray data;
    addToQByteArray(state, data);
    writeNow(socket->getMySocket(), data);
}

void Server::checkReg(const QByteArray & bytes, MySocket * socket){
    QList<QByteArray> reads = bytes.split('\n');

    QString username(reads[0]),
            password(reads[1]),
            programname(reads[2]);
    int result = db.createUser(username, password, programname);
    sendState(socket, result);
}

void Server::readUserSocketData(){
    MySocket * socket = (MySocket *)(sender());
    if (readNow(socket)){
        checkReg(*socket->getBuffer(), socket);
        socket->getBuffer()->clear();
        socket->getLengthBuffer()->clear();
        socket->setExpectedLength(-1);
        socket->getMySocket()->close();
    }
}

void Server::disconnectUserData(){
    MySocket * socket = (MySocket *)(sender());
    socket->getMySocket()->close();
}

void Server::startUserServer(){
    MySocket * socket = new MySocket(userServer->nextPendingConnection());

    connect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    connect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));

    connect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readUserSocketData()));
    connect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectUserData()));
}

void Server::startImageServer(){
    ImageSender * sender = new ImageSender(this,
                                           new MySocket(imageServer->nextPendingConnection()),
                                           &imagerActiveMutex,
                                           &db);
    sender->start();
}

