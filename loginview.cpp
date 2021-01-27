#include "loginview.h"
#include "ui_loginview.h"
#include <QTcpSocket>
#include <QMessageBox>
#include "constants.h"
#include <QDataStream>
#include <QFile>
#include "streammanager.h"

Loginview::Loginview()
    : QMainWindow(nullptr)
    , ui(new Ui::Loginview)
{
    ui->setupUi(this);
    this->setWindowTitle("登录");
    socket = new QTcpSocket();
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

    QString command = "register";
    QString username = ui->iUsername_reg->text() ;
    QString password = ui->iPassword_reg->text() ;


    BlockWriter(socket).stream() << command << username << password;
    socket->flush();

    QString result;
    BlockReader(socket).stream() >> result;
    qDebug() << "result = " << result;
    if (result == "success"){
        QMessageBox::information(nullptr, "提示", "注册成功");
    }
    else {
        QMessageBox::critical(nullptr, "提示", "注册失败");
    }    
    socket->close();
}


void Loginview::on_loginButton_clicked()
{
    QString hostname = ui->iHostname_reg->text() ;
    socket->connectToHost(hostname, 8888);
    if(!socket->waitForConnected(30))    {
        QMessageBox::warning(nullptr, "消息","连接失败！请重新连接", QMessageBox::Yes);
        return;
    }

    QString command = "login";
    QString username = ui->iUsername_login->text() ;
    QString password = ui->iPassword_login->text() ;

    BlockWriter(socket).stream() << command << username << password;

    socket->flush();

    QString result;
    BlockReader(socket).stream() >> result;
    if (result == "success"){
        QMessageBox::information(nullptr, "提示", "登录成功");
        Mainview * mainview = new Mainview(nullptr, socket);
        mainview->show();
        this->close();
    }
    else {
        QMessageBox::critical(nullptr, "提示", "登录失败");
        socket->close();
    }
}
