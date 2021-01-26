#include "paintthread.h"
#include "streammanager.h"
#include <QImage>
#include "constants.h"
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

PaintThread::PaintThread(QTcpSocket * socket)
{
    this->socket = socket;
    socket->moveToThread(this);
}


void PaintThread::run(){
    while (true){
        if (!(socket->isOpen() && socket->isValid())){
            return;
        }
        else if (socket->bytesAvailable() > 0){
            BlockReader(socket).stream() >> image;
            emit paint();
        }
        else {
            socket->waitForReadyRead(0);
        }
        dispatch_message();
    }
}

void PaintThread::dispatch_message(){
    while (!events.empty()){
        QEvent * event = events.dequeue();
        if (event->type() == QEvent::KeyPress){
            QKeyEvent * realEvent = (QKeyEvent *)event;
            BlockWriter(socket).stream() << KEYDOWNEVENT;
            int scancode = realEvent->nativeScanCode();
            int nativekey = realEvent->nativeVirtualKey();
            BlockWriter(socket).stream() << scancode << nativekey;
        }
        else if (event->type() == QEvent::KeyRelease){
            QKeyEvent * realEvent = (QKeyEvent *)event;
            BlockWriter(socket).stream() << KEYUPEVENT;
            int scancode = realEvent->nativeScanCode();
            int nativekey = realEvent->nativeVirtualKey();
            BlockWriter(socket).stream() << scancode << nativekey;
        }
        else if (event->type() == QEvent::MouseButtonPress ||
                 event->type() == QEvent::MouseButtonDblClick){
            QMouseEvent * realEvent = (QMouseEvent *)event;
            BlockWriter(socket).stream() << MOUSEPRESSEVENT;
            int button = realEvent->button(), x = realEvent->x(), y = realEvent->y();
            BlockWriter(socket).stream() << button << x << y;
        }
        else if (event->type() == QEvent::MouseMove){
            QMouseEvent * realEvent = (QMouseEvent *)event;
            BlockWriter(socket).stream() << MOUSEMOVEEVENT;
            int x = realEvent->x(), y = realEvent->y();
            BlockWriter(socket).stream() << x << y;
        }
        else if (event->type() == QEvent::MouseButtonRelease){
            QMouseEvent * realEvent = (QMouseEvent *)event;
            BlockWriter(socket).stream() << MOUSERELEASEEVENT;
            int button = realEvent->button(), x = realEvent->x(), y = realEvent->y();
            BlockWriter(socket).stream() << button << x << y;
        }
        else if (event->type() == QEvent::Wheel){
            QWheelEvent * realEvent = (QWheelEvent *)event;
            BlockWriter(socket).stream() << MOUSESCROLLEVENT;
            int delta = realEvent->delta(), x = realEvent->x(), y = realEvent->y();
            BlockWriter(socket).stream() << delta << x << y;
        }

        socket->flush();
    }
}

void PaintThread::push_event(QEvent * event){
    events.enqueue(event);
}

QImage PaintThread::getImage(){
    return image;
}

PaintThread::~PaintThread(){
    socket->disconnectFromHost();
    socket->close();
    while (!events.empty()){
        QEvent * event = events.dequeue();
        delete event;
    }
}
