#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QDialog>
#include <QTcpSocket>
#include <QLabel>
#include "loginview.h"
#include <QMouseEvent>
#include "message.h"

namespace Ui {
class Videoview;
}

class Videoview : public QDialog
{
    Q_OBJECT

public:
    explicit Videoview(QWidget *parent, QTcpSocket *,
                       Loginview *);
    ~Videoview();

private:
    Ui::Videoview *ui;
    QTcpSocket * socket;
    QWidget * myParent;
    std::list<Message> messageQueue;

    void postMessages();

    QByteArray buffer, length_buffer;
    int expected_length;
    QLabel *panel;
public slots:
    void doDisconnect();
    void drawImage();
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void keyPressEvent(QKeyEvent *) override;
};

#endif // VIDEOVIEW_H
