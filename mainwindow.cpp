#include "mainwindow.h"
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QThread>
#include <proto.h>
#include <QPaintEvent>
#include <QTcpSocket>
#include "constants.h"
#include "xwin.h"

extern QTcpSocket * sock;
UI * xwin_ui;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setMouseTracking(true);
    panel = new QLabel(this);
    sock = new QTcpSocket();
    char server[256] = "192.168.93.129";
    uint32 flags = RDP_LOGON_NORMAL;
    char domain[256] = {0};
    char password[256] = "123456";
    char shell[256] = {0};
    char directory[256] = {0};
    if (!rdp_connect(server, flags, domain, password, shell, directory))
        error("rdp_connect failed");
    memset(password, 0, sizeof(password));
    xwin_ui = new UI(this, 800, 600, 32);
    exec();
}

int MainWindow::exec(){
    while (true){
        rdp_main_loop();
        QApplication::processEvents();
    }
}

void MainWindow::keyPressEvent(QKeyEvent * event){
    uint32 ev_time = time(NULL);
    if (handle_special_keys(event->key(), ev_time, True))
        return;
    if (event->nativeScanCode() == 0)
        return;
    //ensure_remote_modifiers(ev_time, tr);
    rdp_send_scancode(ev_time, RDP_KEYPRESS, event->nativeScanCode());
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
    uint32 ev_time = time(NULL);
    if (handle_special_keys(event->key(), ev_time, False))
        return;
    if (event->nativeScanCode() == 0)
        return;
    rdp_send_scancode(ev_time, RDP_KEYRELEASE, event->nativeScanCode());
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
                               MOUSE_FLAG_MOVE, event->x(), event->y());
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    uint16 flags = MOUSE_FLAG_DOWN;
    uint16 button = xkeymap_translate_button(event->button());
    if (button == 0)
        return;
    rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
               flags | button, event->x(), event->y());
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    uint16 flags = 0;
    uint16 button = xkeymap_translate_button(event->button());
    if (button == 0)
        return;
    rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
               flags | button, event->x(), event->y());
}

void MainWindow::wheelEvent(QWheelEvent *event){
    uint16 flags = 0;
    uint16 button;
    if (event->delta() > 0){
        button = MOUSE_FLAG_BUTTON4;
    }
    else {
        button = MOUSE_FLAG_BUTTON5;
    }
    rdp_send_input(time(NULL), RDP_INPUT_MOUSE,
               flags | button, event->x(), event->y());
}

QLabel * MainWindow::getPanel(){
    return panel;
}

MainWindow::~MainWindow()
{
    rdp_disconnect();
    delete xwin_ui;
}

