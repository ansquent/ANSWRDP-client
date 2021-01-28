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
    connect(socket, SIGNAL(disconnected()), this, SLOT(close()));
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
        BlockReader(socket).stream() >> image;
        paint();
    }
}

void Mainview::keyPressEvent(QKeyEvent *event) {
    BlockWriter(socket).stream() << KEYDOWNEVENT;
    int scancode = event->nativeScanCode();
    int nativekey = event->nativeVirtualKey();
    BlockWriter(socket).stream() << scancode << nativekey;
}

void Mainview::keyReleaseEvent(QKeyEvent *event) {
    BlockWriter(socket).stream() << KEYUPEVENT;
    int scancode = event->nativeScanCode();
    int nativekey = event->nativeVirtualKey();
    BlockWriter(socket).stream() << scancode << nativekey;
}

void Mainview::mouseMoveEvent(QMouseEvent *event) {
    BlockWriter(socket).stream() << MOUSEMOVEEVENT;
    int x = event->x(), y = event->y();
    BlockWriter(socket).stream() << x << y;
}

void Mainview::mousePressEvent(QMouseEvent *event) {
    BlockWriter(socket).stream() << MOUSEPRESSEVENT;
    int button = event->button(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << button << x << y;
}

void Mainview::mouseReleaseEvent(QMouseEvent *event) {
    BlockWriter(socket).stream() << MOUSERELEASEEVENT;
    int button = event->button(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << button << x << y;
}

void Mainview::mouseDoubleClickEvent(QMouseEvent *event) {
    BlockWriter(socket).stream() << MOUSEPRESSEVENT;
    int button = event->button(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << button << x << y;
}

void Mainview::wheelEvent(QWheelEvent *event) {
    BlockWriter(socket).stream() << MOUSESCROLLEVENT;
    int delta = event->delta(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << delta << x << y;
}
