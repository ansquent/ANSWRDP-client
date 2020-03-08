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
    sendEmptyBag();
    //qDebug() << (void *)myParent << "\n";
}

void Videoview::sendEmptyBag(){
    QByteArray littlebag;
    writeNow(socket, littlebag);
}

void Videoview::drawImage(){
    //qDebug() << "In drawImage\n";
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
        sendEmptyBag();
    }
}

void Videoview::doDisconnect(){
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(drawImage()));
    disconnect(socket, SIGNAL(disconnected()), this, SLOT(doDisconnect()));
    QMessageBox::warning(this, "远程连接中断", "您已经中断登录。");
    this->setVisible(false);
    delete this;
}

Videoview::~Videoview()
{
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(drawImage()));
    disconnect(socket, SIGNAL(disconnected()), this, SLOT(doDisconnect()));
    //qDebug() << (void *)myParent << "\n";
    socket->close();
    myParent->setEnabled(true);
    delete ui;
}
