#include "loginview.h"
#include "ui_loginview.h"
#include <QTcpSocket>
#include <QMessageBox>
#include "config.h"
#include "reader.h"
#include "writer.h"
#include <QJsonDocument>
#include <QJsonObject>

Loginview::Loginview(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Loginview)
    , socket(new QTcpSocket())
    , expected_length(-1)
{
    ui->setupUi(this);
    buffer.clear();
    connect(socket, SIGNAL(disconnected()), this, SLOT(doDisconnected()));
}

void Loginview::doDisconnected(){
    socket->close();
}

Loginview::~Loginview()
{
    delete ui;
}

void Loginview::checkReg(const QByteArray & result){
    if (result.at(0) == CREATE_USER_SUCCESS){
        QMessageBox::information(this, "Success", QString::number(result.at(0)), QMessageBox::Yes);
    }
    else {
        QMessageBox::information(this, "Failed", QString::number(result.at(0)), QMessageBox::Yes);
    }
}

void Loginview::checkLogin(const QByteArray & result){
    if (result.at(0) == LOGIN_USER_SUCCESS){
        QMessageBox::information(this, "Success", QString::number(result.at(0)), QMessageBox::Yes);
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readLoginSocketData()));
        this->setEnabled(false);
    }
    else {
        QMessageBox::information(this, "Failed", QString::number(result.at(0)), QMessageBox::Yes);
    }
}

void Loginview::readRegSocketData(){
    if (readNow(socket, &expected_length, buffer, length_buffer)){
        checkReg(buffer);
        buffer.clear();
        length_buffer.clear();
        expected_length = -1;
        socket->close();
    }
}

void Loginview::on_regButton_clicked()
{
    QString hostname = ui->iHostname_reg->text() ;
    socket->close();
    socket->connectToHost(hostname, 8888);
    if(!socket->waitForConnected(30000))    {
        QMessageBox::warning(this,"消息","连接失败！请重新连接",QMessageBox::Yes);
        return;
    }

    QString username = ui->iUsername_reg->text() ;
    QString password = ui->iPassword_reg->text() ;

    //QString sends = username + '\n' + password + '\n' + programname;
    QJsonObject obj;
    obj.insert("username", username);
    obj.insert("password", password);
    obj.insert("command", "register");

    writeNow(socket, QJsonDocument(obj).toJson());

    connect(socket, SIGNAL(readyRead()), this, SLOT(readRegSocketData()));
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(readLoginSocketData()));
}

void Loginview::readLoginSocketData(){    
    if (readNow(socket, &expected_length, buffer, length_buffer)){
        checkLogin(buffer);
        buffer.clear();
        length_buffer.clear();
        expected_length = -1;
    }
}

void Loginview::on_loginButton_clicked()
{
    QString hostname = ui->iHostname_login->text() ;
    socket->close();
    socket->connectToHost(hostname, 8888);
    if(!socket->waitForConnected(30000))    {
        QMessageBox::warning(this,"消息","连接失败！请重新连接",QMessageBox::Yes);
        return;
    }

    QString username = ui->iUsername_login->text() ;
    QString password = ui->iPassword_login->text() ;
    QString programname = ui->iProgram_login->text();

    QJsonObject obj;
    obj.insert("username", username);
    obj.insert("password", password);
    obj.insert("programname", programname);
    obj.insert("command", "login");

    writeNow(socket, QJsonDocument(obj).toJson());

    disconnect(socket, SIGNAL(readyRead()), this, SLOT(readRegSocketData()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readLoginSocketData()));
}
