#include "mysocket.h"
#include <QTcpSocket>
#include <QDebug>

MySocket::MySocket(QTcpSocket * mysocket):
    socket(mysocket),
    expected_length(-1)
{
    buffer.clear();
}

QTcpSocket * MySocket::getMySocket(){
    return socket;
}

QByteArray * MySocket::getBuffer(){
    return &buffer;
}

QByteArray * MySocket::getLengthBuffer(){
    return &length_buffer;
}

void MySocket::setExpectedLength(int count){
    expected_length = count;
}

int MySocket::getExpectedLength(){
    return expected_length;
}

void MySocket::doSLOTReadyRead(){
    //qDebug() << "in mysocket \n";
    emit doSIGNALReadyRead();
}

void MySocket::doSLOTDisconnect(){
    //qDebug() << "in mysocket \n";
    emit doSIGNALDisconnect();
}

MySocket::~MySocket(){

}
