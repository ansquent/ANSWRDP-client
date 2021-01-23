#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QEvent>

class XWin_Ui;

class RDPThread;

class Client;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:
    QLabel *panel;
    RDPThread *rdpThread;

protected:
    void keyPressEvent(QKeyEvent *event);

    void keyReleaseEvent(QKeyEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

private slots:
    void paint();
};

#endif // MAINWINDOW_H
