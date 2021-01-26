#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QDataStream>
#include <QTcpSocket>
#include <QLabel>

class Mainview : public QMainWindow
{
    Q_OBJECT
public:
    explicit Mainview(QWidget *parent, QTcpSocket *);
    void start();

private:
    QTcpSocket * socket;
    QLabel * label;

private slots:
    void paint();

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
