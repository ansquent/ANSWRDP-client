#ifndef STARTUI_H
#define STARTUI_H

#include <QDialog>

namespace Ui {
class startUI;
}

class startUI : public QDialog
{
    Q_OBJECT

public:
    explicit startUI(QWidget *parent = nullptr);
    ~startUI();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::startUI *ui;
};

#endif // STARTUI_H
