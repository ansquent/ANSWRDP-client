#ifndef PAINTTHREAD_H
#define PAINTTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QImage>
#include <QQueue>
#include <QEvent>

class PaintThread : public QThread
{
    Q_OBJECT
public:
    PaintThread(QTcpSocket *);

    void run() override;

    QImage getImage();

    void push_event(QEvent *);

    void dispatch_message();

    ~PaintThread() override;
private:
    QTcpSocket * socket;
    QImage image;
    QQueue<QEvent *> events;

signals:
    void paint();
};

#endif // PAINTTHREAD_H
