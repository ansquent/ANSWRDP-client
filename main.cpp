#include "startui.h"
#include <QApplication>
#include <cstdlib>
#include <ctime>

void info(const char *format, ...);

int main(int argc, char *argv[]) {
    srand(time(nullptr));
    QApplication a(argc, argv);
    startUI w;
    w.show();
    return QApplication::exec();
}
