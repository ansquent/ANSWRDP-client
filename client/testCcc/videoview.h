#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QDialog>
#include <QTcpSocket>
#include <QLabel>
#include "loginview.h"

namespace Ui {
class Videoview;
}

class Videoview : public QDialog
{
    Q_OBJECT

public:
    explicit Videoview(QWidget *parent, QTcpSocket *,
                       Loginview *);
    ~Videoview() override;

private:
    Ui::Videoview *ui;
    QTcpSocket * socket;
    QWidget * myParent;

    void sendEmptyBag();

    QByteArray buffer, length_buffer;
    int expected_length;
    QLabel *panel;
public slots:
    void doDisconnect();
    void drawImage();
};

#endif // VIDEOVIEW_H
