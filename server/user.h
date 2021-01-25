#ifndef USER_H
#define USER_H

#include <QString>
#include "database.h"
#include <Windows.h>

class User
{
public:
    explicit User();

    bool login(QString, QString);
    bool reg(QString, QString);
    bool runProgram(QString);
private:
    QString username;
    QString password;
    QString win32username;
    QString win32password;

    HANDLE token;
};

#endif // USER_H
