#include "mainwindow.h"
#include <QApplication>
#include <cstdlib>
#include <ctime>
#include "util.h"

int main(int argc, char *argv[]) {
    srand(time(nullptr));
    QApplication a(argc, argv);
    MainWindow w(nullptr, "192.168.199.129", "Administrator", "123456");
    w.show();
    return QApplication::exec();
}
