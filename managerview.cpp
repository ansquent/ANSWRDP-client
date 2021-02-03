#include "managerview.h"
#include "ui_managerview.h"
#include "mainview.h"
#include <QTcpSocket>
#include <QVariantMap>
#include <QInputDialog>
#include <QMessageBox>
#include "streammanager.h"
#include <QDir>

Managerview::Managerview(QWidget *parent, QTcpSocket * socket, QVariantMap obj):
    QDialog(parent),
    ui(new Ui::Managerview)
{
    ui->setupUi(this);
    this->socket = socket;
    flushresult(obj);
}

Managerview::~Managerview()
{
    delete ui;
}

void Managerview::flushresult(QVariantMap obj){
    if (obj.value("running").toString() == "true"){
        QString programname = obj.value("programname").toString();
        ui->infoLabel->setText("您现在正在运行的程序是:\n" + programname);
        ui->closeButton->setEnabled(true);
        ui->gotoButton->setEnabled(true);
        ui->launchButton->setEnabled(false);
    }
    else {
        ui->infoLabel->setText("您现在没有运行任何程序。");
        ui->closeButton->setEnabled(false);
        ui->gotoButton->setEnabled(false);
        ui->launchButton->setEnabled(true);
    }
}

void Managerview::on_launchButton_clicked()
{
    bool ok;
    QString programname = QInputDialog::getText(this, "启动程序",
                                       "请输入需要启动的程序的路径",  QLineEdit::Normal,
                                                "", &ok);
    if (ok){
        QVariantMap sendpacket;
        sendpacket.insert("command", "runprogram");
        sendpacket.insert("programname", programname);
        send_map(socket, sendpacket);
    }
    QVariantMap receivepacket;
    receive_map(socket, receivepacket);
    flushresult(receivepacket);
}

void Managerview::on_closeButton_clicked()
{
    int choose = QMessageBox::warning(nullptr, "关闭程序", "你确定要关闭正在运行的程序吗？可能会丢失数据。",
                                      QMessageBox::Yes | QMessageBox::No);
    if (choose == QMessageBox::Yes){
        QVariantMap sendpacket;
        sendpacket.insert("command", "closeprogram");
        send_map(socket, sendpacket);
    }
    QVariantMap receivepacket;
    receive_map(socket, receivepacket);
    flushresult(receivepacket);
}

void Managerview::on_gotoButton_clicked()
{
    QVariantMap sendpacket;
    sendpacket.insert("command", "gotoprogram");
    send_map(socket, sendpacket);
    Mainview *mainview = new Mainview(nullptr, socket);
    mainview->show();
    this->close();
}
