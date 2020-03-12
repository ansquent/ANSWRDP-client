#include "imagesender.h"
#include <QTcpSocket>
#include <windows.h>
#include <QPicture>
#include <QBuffer>
#include "reader.h"
#include "writer.h"
#include "message.h"
#include "include/hook.h"
#include <QPixmap>
#include <set>
#include "programhandler.h"
#include <vector>

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat=0);

ImageSender::ImageSender(Server * _server, MySocket * _tSocket, QMutex * _mutex,
                         Database * _db):
    socket(_tSocket),
    activeMutex(_mutex),
    userdb(_db)
{

}

void ImageSender::run(){
    startImageServer();
    exec();
}

void ImageSender::sendState(MySocket * socket, int state){
    QByteArray data;
    addToQByteArray(state, data);
    writeNow(socket->getMySocket(), data);
}

void ImageSender::checkLogin(const QByteArray & bytes, MySocket * socket){
    QList<QByteArray> reads = bytes.split('\n');

    QString username(reads[0]),
            password(reads[1]);
    int result;
    if ((result = userdb->login(username, password)) == LOGIN_USER_SUCCESS){
        sendState(socket, result);  //握手成功
        hwnd = userdb->get_hwnd(username);
        disconnect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
        disconnect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readImageSocketData()));
        connect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
        connect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(handleImageSocketData()));
        writeImageSocketData();
    }
    else {
        sendState(socket, result);
    }
}

void ImageSender::readImageSocketData(){
    if (readNow(socket)){
        checkLogin(*socket->getBuffer(), socket);
        socket->getBuffer()->clear();
        socket->getLengthBuffer()->clear();
        socket->setExpectedLength(-1);
    }
}

void ImageSender::disconnectImageSocketData(){
    disconnect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    disconnect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));

    disconnect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(handleImageSocketData()));
    disconnect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readImageSocketData()));
    disconnect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectImageSocketData()));
    socket->getMySocket()->close();
    quit();
    wait();
    delete this;
}

void ImageSender::startImageServer(){
    connect(socket->getMySocket(), SIGNAL(readyRead()), socket, SLOT(doSLOTReadyRead()));
    connect(socket->getMySocket(), SIGNAL(disconnected()), socket, SLOT(doSLOTDisconnect()));

    connect(socket, SIGNAL(doSIGNALReadyRead()), this, SLOT(readImageSocketData()));
    connect(socket, SIGNAL(doSIGNALDisconnect()), this, SLOT(disconnectImageSocketData()));
}

void ImageSender::translateMessage(QByteArray & bytes){
    int pointer = 0;
    std::vector<INPUT> inputs;
    if (*reinterpret_cast<unsigned char *>(bytes.data() + pointer) <= 0){
        return;
    }
    int cx_screen = ::GetSystemMetrics(SM_CXSCREEN);
    int cy_screen = ::GetSystemMetrics(SM_CYSCREEN);

    while (pointer < bytes.size()){
        //足够信赖的tcp连接，而不会丢包
        unsigned char message_type = *reinterpret_cast<unsigned char *>(bytes.data() + pointer);
        pointer = pointer + sizeof(unsigned char);
        int keycode;
        int x, y;

        INPUT t_input;
        ZeroMemory(&t_input, sizeof(INPUT));
        qDebug() << message_type;
        switch (message_type) {
            case KEYDOWNEVENT:
                keycode = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                t_input.type = INPUT_KEYBOARD;
                t_input.ki.wVk = keycode;
                t_input.ki.time = GetCurrentTime();
            break;
            case MOUSEPRESSEVENT:
                x = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                y = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                translateToScreenPoint(hwnd, &x, &y, cx_screen, cy_screen);
                t_input.type = INPUT_MOUSE;
                t_input.mi.dx = x;
                t_input.mi.dy = y;
                t_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
            break;
            case MOUSERIGHTPRESSEVENT:
                x = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                y = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                translateToScreenPoint(hwnd, &x, &y, cx_screen, cy_screen);
                t_input.type = INPUT_MOUSE;
                t_input.mi.dx = x;
                t_input.mi.dy = y;
                t_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN;
            break;
            case MOUSEMIDDLEPRESSEVENT:
                x = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                y = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                translateToScreenPoint(hwnd, &x, &y, cx_screen, cy_screen);
                t_input.type = INPUT_MOUSE;
                t_input.mi.dx = x;
                t_input.mi.dy = y;
                t_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN;
            break;
            case MOUSEMOVEEVENT:
                x = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                y = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                translateToScreenPoint(hwnd, &x, &y, cx_screen, cy_screen);
                t_input.type = INPUT_MOUSE;
                t_input.mi.dx = x;
                t_input.mi.dy = y;
                t_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
            break;
            case MOUSERELEASEEVENT:
                x = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                y = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                translateToScreenPoint(hwnd, &x, &y, cx_screen, cy_screen);
                t_input.type = INPUT_MOUSE;
                t_input.mi.dx = x;
                t_input.mi.dy = y;
                t_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
            break;
            case MOUSERIGHTRELEASEEVENT:
                x = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                y = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                translateToScreenPoint(hwnd, &x, &y, cx_screen, cy_screen);
                t_input.type = INPUT_MOUSE;
                t_input.mi.dx = x;
                t_input.mi.dy = y;
                t_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP;
            break;
            case MOUSEMIDDLERELEASEEVENT:
                x = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                y = *(int *)(bytes.data() + pointer);
                pointer += sizeof(int);
                translateToScreenPoint(hwnd, &x, &y, cx_screen, cy_screen);
                t_input.type = INPUT_MOUSE;
                t_input.mi.dx = x;
                t_input.mi.dy = y;
                t_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP;
            break;
        }

        inputs.push_back(t_input);
    }
    /* To do */
    activeMutex->lock();
    if (GetForegroundWindow() != hwnd){
        BringWindowToTop(hwnd);
    }
    SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
    activeMutex->unlock();
}

void ImageSender::handleImageSocketData(){
    if (!readNow(socket)){
        return;
    }

    translateMessage(*socket->getBuffer());
    socket->getBuffer()->clear();
    socket->getLengthBuffer()->clear();
    socket->setExpectedLength(-1);
    writeImageSocketData();
}

void ImageSender::writeImageSocketData(){
    HDC MDC;
    HBITMAP hbitmap;
    RECT r = {0, 0, 0, 0};
    GetWindowRect(hwnd, &r);

    if (r.left >= r.right || r.top >= r.bottom){
        disconnectImageSocketData();
        return;
    }

    HDC hdc = GetWindowDC(hwnd);
    MDC = CreateCompatibleDC(hdc);
    hbitmap = CreateCompatibleBitmap(hdc, r.right - r.left, r.bottom - r.top);
    SelectObject(MDC, hbitmap);
    PrintWindow(hwnd, MDC, 0);

    QImage image(qt_pixmapFromWinHBITMAP(hbitmap).toImage());
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    writeNow(socket->getMySocket(), ba);
}

ImageSender::~ImageSender(){}
