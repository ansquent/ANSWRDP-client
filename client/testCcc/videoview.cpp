#include "videoview.h"
#include "ui_videoview.h"
#include <QTcpSocket>
#include <QLabel>
#include <QImage>
#include <QThread>
#include "reader.h"
#include "writer.h"
#include <QMessageBox>
#include "loginview.h"
#include <QMouseEvent>
#include "config.h"
#include "message.h"

Videoview::Videoview(QWidget *parent, QTcpSocket * mySocket,
                     Loginview *realParent) :
    QDialog(parent),
    ui(new Ui::Videoview),
    socket(mySocket),
    myParent(realParent),
    expected_length(-1)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    buffer.clear();
    panel = new QLabel(this);
    panel->setGeometry(0, 0, 0, 0);
    connect(socket, SIGNAL(readyRead()), this, SLOT(drawImage()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(doDisconnect()));
    //试图丢弃开始时的包
    drawImage();
    postMessages();
}

void Videoview::drawImage(){
    if (readNow(socket, &expected_length, buffer, length_buffer)){
        QImage qimage;
        qimage.loadFromData(buffer);
        QPixmap qp = QPixmap::fromImage(qimage);
        this->setFixedSize(qp.width(), qp.height());
        panel->setFixedSize(qp.width(), qp.height());
        panel->setPixmap(qp);

        buffer.clear();
        length_buffer.clear();
        expected_length = -1;
        postMessages();
    }
}

void Videoview::doDisconnect(){
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(drawImage()));
    disconnect(socket, SIGNAL(disconnected()), this, SLOT(doDisconnect()));
    QMessageBox::warning(this, "Connect aborted.", "You are offline.");
    this->setVisible(false);
    delete this;
}

Videoview::~Videoview()
{
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(drawImage()));
    disconnect(socket, SIGNAL(disconnected()), this, SLOT(doDisconnect()));
    socket->close();
    myParent->setEnabled(true);
    delete ui;
}

void Videoview::mousePressEvent(QMouseEvent * event){
    Message message;
    switch (event->button()) {
    case Qt::LeftButton:
        message.message_type = MOUSEPRESSEVENT;
        message.message_info.mousepressevent.x = event->x();
        message.message_info.mousepressevent.y = event->y();
        break;
    case Qt::RightButton:
        message.message_type = MOUSERIGHTPRESSEVENT;
        message.message_info.mouserightpressevent.x = event->x();
        message.message_info.mouserightpressevent.y = event->y();
        break;
    case Qt::MiddleButton:
        message.message_type = MOUSEMIDDLEPRESSEVENT;
        message.message_info.mousemiddlepressevent.x = event->x();
        message.message_info.mousemiddlepressevent.y = event->y();
        break;
    default:
        return;
    }

    messageQueue.push_back(message);
}

void Videoview::mouseMoveEvent(QMouseEvent * event){
    Message message;
    message.message_type = MOUSEMOVEEVENT;
    message.message_info.mousemoveevent.x = event->x();
    message.message_info.mousemoveevent.y = event->y();
    messageQueue.push_back(message);
}

void Videoview::mouseReleaseEvent(QMouseEvent * event){
    Message message;
    switch (event->button()) {
    case Qt::LeftButton:
        message.message_type = MOUSERELEASEEVENT;
        message.message_info.mousereleaseevent.x = event->x();
        message.message_info.mousereleaseevent.y = event->y();
        break;
    case Qt::RightButton:
        message.message_type = MOUSERIGHTRELEASEEVENT;
        message.message_info.mouserightreleaseevent.x = event->x();
        message.message_info.mouserightreleaseevent.y = event->y();
        break;
    case Qt::MiddleButton:
        message.message_type = MOUSEMIDDLERELEASEEVENT;
        message.message_info.mousemiddlereleaseevent.x = event->x();
        message.message_info.mousemiddlereleaseevent.y = event->y();
        break;
    default:
        return;
    }
    messageQueue.push_back(message);
}

void Videoview::keyPressEvent(QKeyEvent * event){
    Message message;
    message.message_type = KEYDOWNEVENT;
    message.message_info.keydownevent.keycode = event->key();
    messageQueue.push_back(message);
}

void Videoview::postMessages(){
    QByteArray messages;
    messages.clear();
    for (std::list<Message>::iterator it = messageQueue.begin();
         it != messageQueue.end();
         ++it){
        messages += static_cast<char>(it->message_type);
        switch(it->message_type){
        case KEYDOWNEVENT:
            addToQByteArray(it->message_info.keydownevent.keycode, messages);
            break;
        case MOUSEPRESSEVENT:
            addToQByteArray(it->message_info.mousepressevent.x, messages);
            addToQByteArray(it->message_info.mousepressevent.y, messages);
            break;
        case MOUSEMOVEEVENT:
            addToQByteArray(it->message_info.mousemoveevent.x, messages);
            addToQByteArray(it->message_info.mousemoveevent.y, messages);
            break;
        case MOUSERELEASEEVENT:
            addToQByteArray(it->message_info.mousereleaseevent.x, messages);
            addToQByteArray(it->message_info.mousereleaseevent.y, messages);
            break;
        case MOUSERIGHTPRESSEVENT:
            addToQByteArray(it->message_info.mouserightpressevent.x, messages);
            addToQByteArray(it->message_info.mouserightpressevent.y, messages);
            break;
        case MOUSEMIDDLEPRESSEVENT:
            addToQByteArray(it->message_info.mousemiddlepressevent.x, messages);
            addToQByteArray(it->message_info.mousemiddlepressevent.y, messages);
            break;
        default:
            break;
        }
    }
    messageQueue.clear();
    writeNow(socket, messages);
}



