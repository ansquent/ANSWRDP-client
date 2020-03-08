#ifndef READER_H
#define READER_H
#include <QByteArray>
#include <QTcpSocket>
#include "mysocket.h"

inline int toInt(const QByteArray & intBytes){
    //assert(intBytes.size() == sizeof(int));
    return *((const int *)(intBytes.data()));
}

inline bool readNow(MySocket * socket){
    //指示区：一个整数，代表数据区的大小
    const int length_expected_length = sizeof (int);
    if (socket->getExpectedLength() <= 0){
        //指示区没读完，数据区大小不知
        *socket->getLengthBuffer() += socket->getMySocket()->read(length_expected_length - socket->getLengthBuffer()->size());
        if (socket->getLengthBuffer()->size() < length_expected_length){
            //需要接着读指示区
            return false;
        }
        else {
            socket->setExpectedLength(toInt(*socket->getLengthBuffer()));
            //assert(socket->getExpectedLength() >= 0);
            socket->getLengthBuffer()->clear();
        }
    }

    //接着读剩下的，是数据
    *socket->getBuffer() += socket->getMySocket()->read(socket->getExpectedLength() - socket->getBuffer()->size());
    //需要接着读数据
    //assert(socket->getBuffer()->size() <= socket->getExpectedLength());
    return socket->getBuffer()->size() == socket->getExpectedLength();
}

#endif // READER_H
