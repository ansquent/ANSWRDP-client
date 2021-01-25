#include "networksocket.h"
#include <QTcpSocket>

NetworkSocket::NetworkSocket(QTcpSocket * mysocket):
    socket(mysocket),
    expected_length(-1)
{
    buffer.clear();
}

QTcpSocket * NetworkSocket::getMySocket(){
    return socket;
}

QByteArray * NetworkSocket::getBuffer(){
    return &buffer;
}

QByteArray * NetworkSocket::getLengthBuffer(){
    return &length_buffer;
}

void NetworkSocket::setExpectedLength(int count){
    expected_length = count;
}

int NetworkSocket::getExpectedLength(){
    return expected_length;
}

void NetworkSocket::doSLOTReadyRead(){
    emit doSIGNALReadyRead();
}

void NetworkSocket::doSLOTDisconnect(){
    emit doSIGNALDisconnect();
}

NetworkSocket::~NetworkSocket(){

}
