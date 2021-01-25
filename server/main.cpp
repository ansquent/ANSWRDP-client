#include <QApplication>
#include <cstdlib>
#include <ctime>
#include "util.h"
#include "rdpthread.h"
#define MAX 2
int main(int argc, char *argv[]) {
    srand(time(nullptr));
    QApplication a(argc, argv);
    QThread threads[MAX];
    RDPThread * window[MAX];
    window[0] = new RDPThread(800, 600, 32, "192.168.199.129", "User", "123456");
    window[0]->show();
    window[1] = new RDPThread(800, 600, 32, "192.168.199.129", "Administrator", "123456");
    window[1]->show();
    return QApplication::exec();
}
