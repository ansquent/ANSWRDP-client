#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Loginview; }
QT_END_NAMESPACE

class Loginview : public QMainWindow
{
    Q_OBJECT

public:
    Loginview(QWidget *parent = nullptr);
    ~Loginview();

private slots:
    void on_regButton_clicked();

    void doDisconnected();

    void readRegSocketData();

    void readLoginSocketData();

    void on_loginButton_clicked();

private:

    void checkReg(const QByteArray &);
    void checkLogin(const QByteArray &);

    Ui::Loginview *ui;
    QTcpSocket * socket;

    QByteArray buffer, length_buffer;
    int expected_length;
};
#endif // LOGINVIEW_H
