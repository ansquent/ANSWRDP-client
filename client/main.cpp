#include "loginview.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Loginview w;
    w.show();
    return a.exec();
}
