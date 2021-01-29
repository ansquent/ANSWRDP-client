#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QDataStream>
#include <QTcpSocket>
#include <QLabel>
#include <QEvent>

class Mainview : public QMainWindow
{
    Q_OBJECT
public:
    explicit Mainview(QWidget *parent, QTcpSocket *);

private:
    QLabel * label;
    QImage image;
    QTimer * timer;
    QTcpSocket * socket;
    void paint();

private slots:
    void dispatch_image();

    void quit();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // MAINVIEW_H
