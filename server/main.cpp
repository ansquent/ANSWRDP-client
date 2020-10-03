#include <QApplication>
#include <QtService>
#include "listenerservice.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    qDebug() << "Hello world";
    ListenerService service(argc, argv);
    return service.exec();
}
