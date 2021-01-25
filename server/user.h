#ifndef USER_H
#define USER_H

#include <QString>
#include <Windows.h>
#include <QSqlDatabase>
#include <QSqlQuery>

class User
{
public:
    explicit User(QString);

    bool login(QString, QString);
    bool reg(QString, QString);
    bool runProgram(QString);
private:
    QString username;
    QString password;
    QString win32username;
    QString win32password;

    QSqlDatabase sqldb;
    QSqlQuery query;
    bool checkUser(QString, QString);
    bool createUser(QString, QString, QString &, QString &);
    bool queryUser(QString, QString, QString &, QString &);
    bool deleteUser(QString, QString);

    HANDLE token;
};

#endif // USER_H
