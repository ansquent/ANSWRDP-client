#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWin>
#include <QPicture>
#include <windows.h>
#include "newprogram.h"
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_startButton_clicked()
{
    server = new Server();
    ui->startButton->setEnabled(false);
}
