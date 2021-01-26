#ifndef USER_H
#define USER_H

#include <QString>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <QSqlDatabase>
#include <QSqlQuery>

class UserManager
{
public:
    explicit UserManager(QString);

    bool login(QString, QString);
    bool reg(QString, QString);
    bool runProgram(QString);
    bool queryUser(QString, QString, QString &, QString &);
private:
    QString server;
    QString username;
    QString password;
    QString win32username;
    QString win32password;

    QSqlDatabase sqldb;
    QSqlQuery query;
    bool checkUser(QString, QString);
    bool createUser(QString, QString, QString &, QString &);
    bool deleteUser(QString, QString);
#ifdef _WIN32
    HANDLE token;
#endif
};

#endif // USER_H
