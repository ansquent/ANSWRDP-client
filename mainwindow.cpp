#include "mainwindow.h"
#include "ui_mainwindow.h"
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
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    panel = new QLabel(this);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->setInterval(10);
    timer->start();

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
}

void MainWindow::update(){
    rdp_main_loop();
}

QLabel * MainWindow::getPanel(){
    return panel;
}

MainWindow::~MainWindow()
{
    rdp_disconnect();
    delete xwin_ui;
    delete ui;
}

