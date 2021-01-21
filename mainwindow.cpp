#include "mainwindow.h"
#include <QLabel>
#include <QThread>
#include <QPaintEvent>
#include <QTimer>
#include "constants.h"
#include "xwin.h"
void info(const char *format, ...);
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
    panel = new QLabel(this);
    tcptool = new TcpTool();
    xwin_ui = new XWin_Ui(this, 800, 600, 32);
    client = new Client(xwin_ui, tcptool);
    char server[256] = "192.168.93.129";
    uint32 flags = RDP_LOGON_NORMAL;
    char domain[256] = {0};
    char password[256] = "123456";
    char shell[256] = {0};
    char directory[256] = {0};
    if (!client->rdp_connect(server, flags, domain, password, shell, directory))
        exit(-1);
    memset(password, 0, sizeof(password));

    QTimer * timer = new QTimer();
    timer->setInterval(10);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
//    uint32 ev_time = time(NULL);
//    if (client->handle_special_keys(event->key(), ev_time, True))
//        return;
//    if (event->nativeScanCode() == 0)
//        return;
//    //ensure_remote_modifiers(ev_time, tr);
//    client->rdp_send_scancode(ev_time, RDP_KEYPRESS, event->nativeScanCode());
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
//    uint32 ev_time = time(NULL);
//    if (client->handle_special_keys(event->key(), ev_time, False))
//        return;
//    if (event->nativeScanCode() == 0)
//        return;
//    client->rdp_send_scancode(ev_time, RDP_KEYRELEASE, event->nativeScanCode());
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
//    client->rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
//                           MOUSE_FLAG_MOVE, event->x(), event->y());
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    static int filecount = 0;
    QString filename = QString::number(++filecount) + ".jpg";
    info("clicked. Filename = %s", filename.toStdString().c_str());
    xwin_ui->getPixmap()->save(filename);
//    uint16 flags = MOUSE_FLAG_DOWN;
//    uint16 button = client->xkeymap_translate_button(event->button());
//    if (button == 0)
//        return;
//    client->rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
//                           flags | button, event->x(), event->y());
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
//    uint16 flags = 0;
//    uint16 button = client->xkeymap_translate_button(event->button());
//    if (button == 0)
//        return;
//    client->rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
//                           flags | button, event->x(), event->y());
}

void MainWindow::wheelEvent(QWheelEvent *event) {
//    uint16 flags = 0;
//    uint16 button;
//    if (event->delta() > 0) {
//        button = MOUSE_FLAG_BUTTON4;
//    } else {
//        button = MOUSE_FLAG_BUTTON5;
//    }
//    client->rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
//                           flags | button, event->x(), event->y());
}

QLabel *MainWindow::getPanel() {
    return panel;
}

MainWindow::~MainWindow() {
    client->rdp_disconnect();
    delete xwin_ui;
}

void MainWindow::update() {
    client->rdp_main_loop();
}

