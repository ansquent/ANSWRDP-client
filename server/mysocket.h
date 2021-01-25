#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <QObject>
#include <QTcpSocket>

class MySocket: public QObject
{
    Q_OBJECT
public:
    MySocket(QTcpSocket *);

    QTcpSocket * getMySocket();

    QByteArray * getBuffer();

    QByteArray * getLengthBuffer();

    void setExpectedLength(int count);

    int getExpectedLength();

    ~MySocket();

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
