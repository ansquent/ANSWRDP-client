#include "mainview.h"
#include <QDataStream>
#include <QTcpSocket>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include "constants.h"
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QMessageBox>
#include "streammanager.h"

Mainview::Mainview(QWidget *parent, QTcpSocket * socket)
    : QMainWindow(parent)
{
    this->socket = socket;
    this->label = new QLabel(this);
    this->timer = new QTimer(this);
    this->timer->setInterval(10);
    this->timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(dispatch_image()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(quit()));
}

void Mainview::quit(){
    QMessageBox::warning(nullptr, "结束共享", "连接已经断开");
    socket->disconnectFromHost();
    socket->close();
    delete socket;
    this->close();
}


void Mainview::paint(){
    QPixmap pixmap = QPixmap::fromImage(image);
    this->label->setPixmap(pixmap);
    if (this->height() != pixmap.height() || this->width() != pixmap.width()){
        this->resize(pixmap.width(), pixmap.height());
        this->label->resize(pixmap.width(), pixmap.height());
    }
    repaint();
}

void Mainview::dispatch_image(){
    while (socket->bytesAvailable() > 0){
        receive_image(socket, image);
        paint();
    }
}

void Mainview::keyPressEvent(QKeyEvent *event) {
    int scancode = event->nativeScanCode();
    int nativekey = event->nativeVirtualKey();
    QVariantMap sendpacket;
    sendpacket.insert("type", KEYDOWNEVENT);
    sendpacket.insert("scancode", scancode);
    sendpacket.insert("nativekey", nativekey);
    send_map(socket, sendpacket);
}

void Mainview::keyReleaseEvent(QKeyEvent *event) {
    int scancode = event->nativeScanCode();
    int nativekey = event->nativeVirtualKey();
    QVariantMap sendpacket;
    sendpacket.insert("type", KEYUPEVENT);
    sendpacket.insert("scancode", scancode);
    sendpacket.insert("nativekey", nativekey);
    send_map(socket, sendpacket);
}

void Mainview::mouseMoveEvent(QMouseEvent *event) {
    int x = event->x(), y = event->y();
    QVariantMap sendpacket;
    sendpacket.insert("type", MOUSEMOVEEVENT);
    sendpacket.insert("x", x);
    sendpacket.insert("y", y);
    send_map(socket, sendpacket);
}

void Mainview::mousePressEvent(QMouseEvent *event) {
    int button = event->button(), x = event->x(), y = event->y();
    QVariantMap sendpacket;
    sendpacket.insert("type", MOUSEPRESSEVENT);
    sendpacket.insert("button", button);
    sendpacket.insert("x", x);
    sendpacket.insert("y", y);
    send_map(socket, sendpacket);
}

void Mainview::mouseReleaseEvent(QMouseEvent *event) {
    int button = event->button(), x = event->x(), y = event->y();
    QVariantMap sendpacket;
    sendpacket.insert("type", MOUSERELEASEEVENT);
    sendpacket.insert("button", button);
    sendpacket.insert("x", x);
    sendpacket.insert("y", y);
    send_map(socket, sendpacket);
}

void Mainview::mouseDoubleClickEvent(QMouseEvent *event) {
    int button = event->button(), x = event->x(), y = event->y();
    QVariantMap sendpacket;
    sendpacket.insert("type", MOUSEPRESSEVENT);
    sendpacket.insert("button", button);
    sendpacket.insert("x", x);
    sendpacket.insert("y", y);
    send_map(socket, sendpacket);
}

void Mainview::wheelEvent(QWheelEvent *event) {
    int delta = event->delta(), x = event->x(), y = event->y();
    QVariantMap sendpacket;
    sendpacket.insert("type", MOUSESCROLLEVENT);
    sendpacket.insert("delta", delta);
    sendpacket.insert("x", x);
    sendpacket.insert("y", y);
    send_map(socket, sendpacket);
}
