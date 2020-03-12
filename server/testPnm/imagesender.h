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
#include <list>
#include "message.h"
#include <QMutex>

class ImageSender: public QThread
{
    Q_OBJECT
public:
    ImageSender(Server *, MySocket *, QMutex *, Database *);
    void run() override;
    ~ImageSender();
private:
    void checkLogin(const QByteArray & bytes, MySocket * socket);
    void sendState(MySocket * socket, int state);
    void translateMessage(QByteArray &);

    MySocket * socket;
    HWND hwnd;
    QTcpServer * imageServer;
    Database * userdb;
    QMutex * activeMutex;
public slots:
    void readImageSocketData();
    void disconnectImageSocketData();
    void startImageServer();
    void handleImageSocketData();
    void writeImageSocketData();
};

#endif // IMAGESENDER_H
