#include <QApplication>
#include <cstdlib>
#include <ctime>
#include "networkserver.h"
#include "usermanager.h"
int main(int argc, char *argv[]) {
    srand(time(nullptr));
    QApplication a(argc, argv);
    UserManager *manager = new UserManager("127.0.0.1");
    NetworkServer * server = new NetworkServer(manager);
    server->listen(QHostAddress::Any, 8888);
    return QApplication::exec();
}
