#ifndef MANAGERVIEW_H
#define MANAGERVIEW_H

#include <QDialog>
#include <QTcpSocket>
#include <QVariantMap>
#include <QTcpSocket>

namespace Ui {
class Managerview;
}

class Managerview : public QDialog
{
    Q_OBJECT

public:
    explicit Managerview(QWidget *parent, QTcpSocket * socket, QVariantMap obj);
    ~Managerview();

private slots:
    void on_launchButton_clicked();

    void on_closeButton_clicked();

    void on_gotoButton_clicked();

private:
    Ui::Managerview *ui;
    QTcpSocket * socket;

    void flushresult(QVariantMap obj);
};

#endif // MANAGERVIEW_H
