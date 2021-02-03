#include "loginview.h"
#include "ui_loginview.h"
#include <QTcpSocket>
#include <QMessageBox>
#include "constants.h"
#include <QDataStream>
#include <QFile>
#include <QVariantMap>
#include <QJsonDocument>
#include "streammanager.h"
#include "managerview.h"

Loginview::Loginview()
    : QMainWindow(nullptr)
    , ui(new Ui::Loginview)
{
    ui->setupUi(this);
    setFixedSize(this->width(), this->height());
    socket = new QTcpSocket();
    ui->iPassword_login->setEchoMode(QLineEdit::Password);
    ui->iPassword_reg->setEchoMode(QLineEdit::Password);
}

Loginview::~Loginview()
{
    delete ui;
}

void Loginview::on_regButton_clicked()
{
    QString hostname = ui->iHostname_reg->text();
    socket->connectToHost(hostname, 8888);
    if(!socket->waitForConnected(30))    {
        QMessageBox::warning(nullptr, "消息", "连接失败！请重新连接", QMessageBox::Yes);
        socket->close();
        return;
    }
    QVariantMap sendpacket;
    sendpacket.insert("command", "register");
    sendpacket.insert("username", ui->iUsername_reg->text());
    sendpacket.insert("password", ui->iPassword_reg->text());
    send_map(socket, sendpacket);
    QVariantMap receivepacket;
    receive_map(socket, receivepacket);
    if (receivepacket.value("result") == "success"){
        QMessageBox::information(nullptr, "提示", "注册成功");
    }
    else {
        QMessageBox::critical(nullptr, "提示", "注册失败");
    }    
    socket->close();
}


void Loginview::on_loginButton_clicked()
{
    QString hostname = ui->iHostname_login->text() ;
    socket->connectToHost(hostname, 8888);
    if(!socket->waitForConnected(30))    {
        QMessageBox::warning(nullptr, "消息","连接失败！请重新连接", QMessageBox::Yes);
        return;
    }
    QVariantMap sendpacket;
    sendpacket.insert("command", "login");
    sendpacket.insert("username", ui->iUsername_login->text());
    sendpacket.insert("password", ui->iPassword_login->text());
    send_map(socket, sendpacket);
    QVariantMap receivepacket;
    receive_map(socket, receivepacket);
    if (receivepacket.value("result") == "success"){
        Managerview * managerview = new Managerview(nullptr, socket, receivepacket);
        QMessageBox::information(nullptr, "提示", "登录成功");
        managerview->show();
        this->close();
    }
    else {
        QMessageBox::critical(nullptr, "提示", "登录失败");
    }
}

void Loginview::on_aboutLabel_clicked()
{
    QMessageBox::information(nullptr, "关于本产品", "ANSW·RDP Version 1.0\nPowered By Northbank Inc.\n基于RDesktop项目进行修改和制作，请支持开源");
}
