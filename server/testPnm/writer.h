#ifndef WRITER_H
#define WRITER_H
#include <QTcpSocket>
#include <QByteArray>

inline void writeNow(QTcpSocket *socket, const QByteArray & data){
    QByteArray begins;
    int length = data.size();
    for (unsigned i = 0; i < sizeof(int); ++i){
        begins += *(reinterpret_cast<char *>(&length) + i);
    }
    socket->write(begins);
    socket->write(data);
    socket->flush();
}

template <class T>
inline void addToQByteArray(T t, QByteArray & data){
    //qDebug() << t;
    for (unsigned i = 0; i < sizeof(t); ++i){
        data += *(reinterpret_cast<char *>(&t) + i);
    }
}

#endif // WRITER_H
