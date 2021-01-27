#include "loginview.h"
#include <QApplication>
#include "mainview.h"
#include <QTcpSocket>
#include <QDataStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Loginview loginview;
    loginview.show();
    return a.exec();
}
