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

extern QTcpSocket * sock;

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
    if (!ui_init(this))
        error("ui_init failed");
    if (!rdp_connect(server, flags, domain, password, shell, directory))
        error("rdp_connect failed");
    memset(password, 0, sizeof(password));
    if (!ui_create_window())
    {
        error("ui_create_window failed");
    }
}

void MainWindow::update(){
    rdp_main_loop();
}

QLabel * MainWindow::getPanel(){
    return panel;
}

MainWindow::~MainWindow()
{
    ui_destroy_window();
    rdp_disconnect();
    ui_deinit();
    delete ui;
}

