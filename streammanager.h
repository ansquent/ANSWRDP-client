#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H

#include <QIODevice>
#include <QDataStream>
#include <QBuffer>
#include <QAbstractSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QImage>
#include <QTcpSocket>
#include <QLocalSocket>

class BlockReader
{
public:
    BlockReader(QIODevice *io)
    {
        buffer.open(QIODevice::ReadWrite);
        _stream.setVersion(QDataStream::Qt_4_8);
        _stream.setDevice(&buffer);

        quint64 blockSize;

        // Read the size.
        readMax(io, sizeof(blockSize));
        buffer.seek(0);
        _stream >> blockSize;

        // Read the rest of the data.
        readMax(io, blockSize);
        buffer.seek(sizeof(blockSize));
    }

    QDataStream& stream()
    {
        return _stream;
    }

private:
    // Blocking reads data from socket until buffer size becomes exactly n. No
    // additional data is read from the socket.
    void readMax(QIODevice *io, int n)
    {
        while (buffer.size() < n) {
            while (io->bytesAvailable() <= 0) {
                io->waitForReadyRead(100);
            }
            buffer.write(io->read(n - buffer.size()));
        }
    }
    QBuffer buffer;
    QDataStream _stream;
};

class BlockWriter
{
public:
    BlockWriter(QIODevice *io)
    {
        buffer.open(QIODevice::WriteOnly);
        this->io = io;
        _stream.setVersion(QDataStream::Qt_4_8);
        _stream.setDevice(&buffer);

        // Placeholder for the size. We will get the value
        // at the end.
        _stream << quint64(0);
    }

    ~BlockWriter()
    {
        // Write the real size.
        _stream.device()->seek(0);
        _stream << (quint64) buffer.size();

        // Flush to the device.
        io->write(buffer.buffer());
    }

    QDataStream &stream()
    {
        return _stream;
    }

private:
    QBuffer buffer;
    QDataStream _stream;
    QIODevice *io;
};

inline void receive_map(QIODevice * socket, QVariantMap & map){
    QByteArray arr;
    BlockReader(socket).stream() >> arr;
    QJsonDocument doc = QJsonDocument::fromJson(arr);
    QJsonObject obj = doc.object();
    map = obj.toVariantMap();
}

inline void send_map(QTcpSocket * socket, QVariantMap & map){
    QJsonObject obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc = QJsonDocument(obj);
    QByteArray arr = doc.toJson();
    BlockWriter(socket).stream() << arr;
    socket->flush();
}

inline void send_map(QLocalSocket * socket, QVariantMap & map){
    QJsonObject obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc = QJsonDocument(obj);
    QByteArray arr = doc.toJson();
    BlockWriter(socket).stream() << arr;
    socket->flush();
}

inline void receive_image(QIODevice * socket, QImage & image){
    QByteArray arr;
    BlockReader(socket).stream() >> arr;
    image = QImage::fromData(arr);
}

inline void send_image(QTcpSocket * socket, QImage & image){
    QByteArray arr;
    QBuffer buffer(&arr);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");
    buffer.close();
    BlockWriter(socket).stream() << arr;
    socket->flush();
}

inline void send_image(QLocalSocket * socket, QImage & image){
    QByteArray arr;
    QBuffer buffer(&arr);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");
    buffer.close();
    BlockWriter(socket).stream() << arr;
    socket->flush();
}

#endif
