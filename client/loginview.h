#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDataStream>
#include "mainview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Loginview; }
QT_END_NAMESPACE

class Loginview : public QMainWindow
{
    Q_OBJECT

public:
    explicit Loginview(Mainview *, QTcpSocket *);
    ~Loginview();

private slots:
    void on_regButton_clicked();

    void on_loginButton_clicked();

private:
    Ui::Loginview *ui;
    QTcpSocket * socket;
    Mainview * mainview;
};
#endif // LOGINVIEW_H
