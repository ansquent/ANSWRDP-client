//
// Created by User on 2021/1/22.
//

#ifndef RDESKTOP_WRAP_RDPTHREAD_H
#define RDESKTOP_WRAP_RDPTHREAD_H

#include <QThread>
#include <QEvent>
#include <QQueue>
#include <QTimer>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>

class RDPXWin;

class RDPInvoker;

class MainWindow;

class RDPTcpTool;

class RDPThread : public QMainWindow {
Q_OBJECT
private:
    RDPInvoker *client;
    RDPTcpTool *tcptool;
    RDPXWin *xwin_ui;
    bool willclose;
    int width, height, bpp;
    char server[256] = {0};
    char username[256] = {0};
    char password[256] = {0};
    QTimer *timer;
    QLabel *panel;
public:
    explicit RDPThread(int width, int height, int bpp,
                       QString hostname, QString username, QString password);

    void setClose();

    ~RDPThread() override;

private slots:
    void run();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;
};


#endif //RDESKTOP_WRAP_RDPTHREAD_H
