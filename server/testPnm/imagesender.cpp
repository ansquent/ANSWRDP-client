#include "imagesender.h"
#include <QTcpSocket>
#include <windows.h>
#include <QtWin>
#include <QPicture>
#include <QBuffer>
#include "reader.h"
#include "writer.h"

ImageSender::ImageSender(Server * _server, MySocket * _tSocket,
                         Database & _db):
    socket(_tSocket),
    db(_db)
{

}


void ImageSender::run(){
    startImageServer();
    exec();
}

void ImageSender::sendState(MySocket * socket, int state){
    writeNow(socket->getMySocket(), state);
}

void ImageSender::checkLogin(const QByteArray & bytes, MySocket * socket){
    QList<QByteArray> reads = bytes.split('\n');

    QString username(reads[0]),
            password(reads[1]);
    int result;
    if ((result = db.login(username, password)) == LOGIN_USER_SUCCESS){
        //qDebug() << "login success\n";
        sendState(socket, result);  //握手成功
        hwnd = db.get_hwnd(username);

        disconnect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));

        disconnect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readImageSocketData()));

        connect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));

        connect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(sendImageData()));
        sendImageData();
    }
    else {
        //qDebug() << "login fail\n";
        sendState(socket, result);
    }
}

void ImageSender::readImageSocketData(){
    //qDebug() << "In readImageSocketData\n";
    if (readNow(socket)){
        checkLogin(*socket->getBuffer(), socket);
        socket->getBuffer()->clear();
        socket->getLengthBuffer()->clear();
        socket->setExpectedLength(-1);
    }
}

void ImageSender::disconnectImageData(){
    //qDebug() << "----quit----\n";
    disconnect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    disconnect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));

    disconnect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readImageSocketData()));
    disconnect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectImageData()));
    socket->getMySocket()->close();
    quit();
    wait();
    delete this;
}

void ImageSender::startImageServer(){
    //qDebug() << "Connect image\n";

    connect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    connect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));

    connect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readImageSocketData()));
    connect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectImageData()));
}


void ImageSender::sendImageData(){
    //qDebug() << "in sendImageData()\n";
    socket->getMySocket()->readAll();

    HDC MDC;
    HBITMAP hMemBmp;
    RECT r = {0};

    GetWindowRect(hwnd, &r);
    if (r.left >= r.right){
        disconnectImageData();
        return;
    }
    //qDebug() << r.top << "\n" << r.bottom << "\n" << r.left << "\n" << r.right << "\n";
    HDC hdc = GetWindowDC(hwnd);
    MDC = CreateCompatibleDC(hdc);
    hMemBmp = CreateCompatibleBitmap(hdc, r.right - r.left, r.bottom - r.top);
    SelectObject(MDC, hMemBmp);
    PrintWindow(hwnd, MDC, 0);
    QImage image = QtWin::imageFromHBITMAP(hMemBmp);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    writeNow(socket->getMySocket(), ba);
}

ImageSender::~ImageSender(){}
