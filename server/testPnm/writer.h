#ifndef WRITER_H
#define WRITER_H
#include <QTcpSocket>
#include <QByteArray>

inline void writeNow(QTcpSocket *socket, const QByteArray & data){
    QByteArray begins;
    int length = data.size();
    for (int i = 0; i < sizeof(int); ++i){
        begins += *(((char *)(&length)) + i);
    }
    socket->write(begins);
    socket->write(data);
    socket->flush();
}

inline void writeNow(QTcpSocket *socket, int data){
    QByteArray result;
    int length = sizeof(int);
    for (int i = 0; i < sizeof(int); ++i){
        result += *(((char *)(&length)) + i);
    }
    for (int i = 0; i < sizeof(int); ++i){
        result += *(((char *)(&data)) + i);
    }
    socket->write(result);
    socket->flush();
}

#endif // WRITER_H
