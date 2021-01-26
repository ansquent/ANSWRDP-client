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
    this->label = new QLabel(this);
    paintthread = new PaintThread(socket);
    paintthread->start();
    connect(paintthread, SIGNAL(paint()), this, SLOT(paint()));
}


void Mainview::paint(){
    QImage image = paintthread->getImage();
    QPixmap pixmap = QPixmap::fromImage(image);
    this->label->setPixmap(pixmap);
    if (this->height() != pixmap.height() || this->width() != pixmap.width()){
        this->resize(pixmap.width(), pixmap.height());
        this->label->resize(pixmap.width(), pixmap.height());
    }
    repaint();
}

void Mainview::keyPressEvent(QKeyEvent *event) {
    paintthread->push_event(new QKeyEvent(*event));
}

void Mainview::keyReleaseEvent(QKeyEvent *event) {
    paintthread->push_event(new QKeyEvent(*event));
}

void Mainview::mouseMoveEvent(QMouseEvent *event) {
    paintthread->push_event(new QMouseEvent(*event));
}

void Mainview::mousePressEvent(QMouseEvent *event) {
    paintthread->push_event(new QMouseEvent(*event));
}

void Mainview::mouseReleaseEvent(QMouseEvent *event) {
    paintthread->push_event(new QMouseEvent(*event));
}

void Mainview::mouseDoubleClickEvent(QMouseEvent *event) {
    paintthread->push_event(new QMouseEvent(*event));
}

void Mainview::wheelEvent(QWheelEvent *event) {
    paintthread->push_event(new QWheelEvent(*event));
}
