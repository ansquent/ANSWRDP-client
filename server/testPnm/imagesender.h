#ifndef IMAGESENDER_H
#define IMAGESENDER_H
#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>
#include <windows.h>
#include "userinfo.h"
#include <QImage>
#include "mysocket.h"
#include "server.h"

class ImageSender: public QThread
{
    Q_OBJECT
public:
    ImageSender(Server *, MySocket *, Database &);
    void run() override;
    ~ImageSender() override;
private:
    void checkLogin(const QByteArray & bytes, MySocket * socket);
    void sendState(MySocket * socket, int state);

    MySocket * socket;
    HWND hwnd;

    QTcpServer * imageServer;
    Database & db;
public slots:
    void readImageSocketData();
    void disconnectImageData();
    void startImageServer();
    void sendImageData();
};

#endif // IMAGESENDER_H
