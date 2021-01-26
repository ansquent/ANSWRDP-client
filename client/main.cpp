#include "loginview.h"
#include <QApplication>
#include "mainview.h"
#include <QTcpSocket>
#include <QDataStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTcpSocket * socket = new QTcpSocket();
    Mainview mainview(nullptr, socket);
    mainview.hide();
    Loginview loginview(&mainview, socket);
    loginview.show();
    return a.exec();
}
