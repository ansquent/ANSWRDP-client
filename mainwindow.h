#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QThread>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    QLabel * getPanel();
    ~MainWindow();
signals:
    void done();
public slots:
    void update();

private:
    Ui::MainWindow *ui;
    QLabel * panel;
    QTimer * timer;
};

#endif // MAINWINDOW_H
