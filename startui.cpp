#include "startui.h"
#include "ui_startui.h"
#include <QMessageBox>
#include "mainwindow.h"

startUI::startUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::startUI)
{
    ui->setupUi(this);    
    this->setWindowTitle("Login - Rdesktop");
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
}

startUI::~startUI()
{
    delete ui;
}

void startUI::on_buttonBox_accepted()
{
    QString hostname = ui->addressEdit->text();
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    MainWindow * mwindow = new MainWindow(nullptr, hostname, username, password);
    mwindow->show();
    this->destroy();
}

void startUI::on_buttonBox_rejected()
{
    this->destroy();
}
