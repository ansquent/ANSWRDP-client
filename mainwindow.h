#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QEvent>
#include "xwin.h"
#include "client.h"
#include "tcp.h"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    QLabel *getPanel();

    ~MainWindow();

public slots:
    void update();

private:
    QLabel *panel;
    XWin_Ui *xwin_ui;
    Client *client;
    TcpTool *tcptool;

protected:
    void keyPressEvent(QKeyEvent *event);

    void keyReleaseEvent(QKeyEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);
};

#endif // MAINWINDOW_H
