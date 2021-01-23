#include "mainwindow.h"
#include <QApplication>
#include <cstdlib>
#include <ctime>

void info(const char *format, ...);

int main(int argc, char *argv[]) {
    srand(time(NULL));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
