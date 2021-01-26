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
#include "streammanager.h"

Mainview::Mainview(QWidget *parent, QTcpSocket * socket)
    : QMainWindow(parent)
{
    this->socket = socket;
    this->label = new QLabel(this);
}

void Mainview::start(){
    connect(socket, SIGNAL(readyRead()), this, SLOT(paint()));
}

void Mainview::paint(){
    QImage image;
    BlockReader(socket).stream() >> image;
    qDebug() << "image size: " << image.sizeInBytes();
    if (height() != image.height() || width() != image.width()){
        resize(image.width(), image.height());
        label->resize(image.width(), image.height());
    }
    label->setPixmap(QPixmap::fromImage(image));
    repaint();
}

void Mainview::keyPressEvent(QKeyEvent *event) {
    BlockWriter(socket).stream() << KEYDOWNEVENT;
    int scancode = event->nativeScanCode();
    int nativekey = event->nativeVirtualKey();
    BlockWriter(socket).stream() << scancode << nativekey;
    socket->flush();
}

void Mainview::keyReleaseEvent(QKeyEvent *event) {
    BlockWriter(socket).stream() << KEYUPEVENT;
    int scancode = event->nativeScanCode();
    int nativekey = event->nativeVirtualKey();
    BlockWriter(socket).stream() << scancode << nativekey;
    socket->flush();
}

void Mainview::mouseMoveEvent(QMouseEvent *event) {
    BlockWriter(socket).stream() << MOUSEMOVEEVENT;
    int x = event->x(), y = event->y();
    BlockWriter(socket).stream() << x << y;
    socket->flush();
}

void Mainview::mousePressEvent(QMouseEvent *event) {
    qDebug() << "mouse press";
    BlockWriter(socket).stream() << MOUSEPRESSEVENT;
    int button = event->button(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << button << x << y;
    socket->flush();
}

void Mainview::mouseReleaseEvent(QMouseEvent *event) {
    BlockWriter(socket).stream() << MOUSERELEASEEVENT;
    int button = event->button(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << button << x << y;
    socket->flush();
}

void Mainview::mouseDoubleClickEvent(QMouseEvent *event) {
    BlockWriter(socket).stream() << MOUSEPRESSEVENT;
    int button = event->button(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << button << x << y;
    socket->flush();
}

void Mainview::wheelEvent(QWheelEvent *event) {
    BlockWriter(socket).stream() << MOUSESCROLLEVENT;
    int delta = event->delta(), x = event->x(), y = event->y();
    BlockWriter(socket).stream() << delta << x << y;
    socket->flush();
}
