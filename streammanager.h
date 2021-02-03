#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H
#include <QIODevice>
#include <QVariant>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImage>
#include <QBuffer>

inline void receive_map(QTcpSocket * socket, QVariantMap & map){
    int expected_length = sizeof(int);

    char * buffer = new char[expected_length];
    char * p = buffer;
    while (expected_length > 0){
        int len = socket->read(p, expected_length);
        if (len <= 0){
            socket->waitForReadyRead(-1);
        }
        else {
            expected_length -= len;
            p += len;
        }
    }
    expected_length = *((int *)(buffer));
    delete []buffer;

    buffer = new char[expected_length];
    p = buffer;
    while (expected_length > 0){
        int len = socket->read(p, expected_length);
        if (len <= 0){
            socket->waitForReadyRead(-1);
        }
        else {
            expected_length -= len;
            p += len;
        }
    }
    QByteArray arr = QByteArray(buffer, (p - buffer));

    delete []buffer;
    QJsonDocument doc = QJsonDocument::fromJson(arr);
    QJsonObject obj = doc.object();
    map = obj.toVariantMap();
}

inline void receive_image(QTcpSocket * socket, QImage & image){
    int expected_length = sizeof(int);

    char * buffer = new char[expected_length];
    char * p = buffer;
    while (expected_length > 0){
        int len = socket->read(p, expected_length);
        if (len <= 0){
            socket->waitForReadyRead(-1);
        }
        else {
            expected_length -= len;
            p += len;
        }
    }
    expected_length = *((int *)(buffer));
    delete []buffer;

    buffer = new char[expected_length];
    p = buffer;
    while (expected_length > 0){
        int len = socket->read(p, expected_length);
        if (len <= 0){
            socket->waitForReadyRead(-1);
        }
        else {
            expected_length -= len;
            p += len;
        }
    }
    QByteArray arr = QByteArray(buffer, (p - buffer));

    delete []buffer;
    image = QImage::fromData(arr);
}

inline void send_map(QTcpSocket * socket, QVariantMap & map){
    QJsonObject obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc = QJsonDocument(obj);
    QByteArray arr = doc.toJson();
    QByteArray len;
    int length = arr.size();
    for (unsigned i = 0; i < sizeof(int); ++i){
        len += *(reinterpret_cast<char *>(&length) + i);
    }

    socket->write(len);
    socket->write(arr);
    socket->flush();
}

inline void send_image(QTcpSocket * socket, QImage & image){
    QByteArray arr;
    QBuffer buffer(&arr);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");
    buffer.close();
    QByteArray len;
    int length = arr.size();
    for (unsigned i = 0; i < sizeof(int); ++i){
        len += *(reinterpret_cast<char *>(&length) + i);
    }
    length = socket->write(len);
    socket->waitForBytesWritten();
    length = socket->write(arr);
    socket->flush();
}

#endif // STREAMMANAGER_H
