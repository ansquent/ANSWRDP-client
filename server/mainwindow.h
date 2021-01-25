#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QEvent>

class RDPXWin;

class RDPThread;

class RDPInvoker;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *, QString, QString, QString);

    ~MainWindow() override;

private:
    QLabel *panel;
    RDPThread *rdpThread;

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:

    void paint();

};

#endif // MAINWINDOW_H
