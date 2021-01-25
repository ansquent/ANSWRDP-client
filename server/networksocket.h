#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <QObject>
#include <QTcpSocket>

class NetworkSocket: public QObject
{
    Q_OBJECT
public:
    NetworkSocket(QTcpSocket *);

    QTcpSocket * getMySocket();

    QByteArray * getBuffer();

    QByteArray * getLengthBuffer();

    void setExpectedLength(int count);

    int getExpectedLength();

    ~NetworkSocket();

private:
    QTcpSocket * socket;
    QByteArray buffer, length_buffer;
    int expected_length;

public slots:
    void doSLOTReadyRead();
    void doSLOTDisconnect();
signals:
    void doSIGNALReadyRead();
    void doSIGNALDisconnect();
};

#endif // MYSOCKET_H
