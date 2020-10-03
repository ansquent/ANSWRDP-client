#include "database.h"
#include <QString>
#include <QSqlDatabase>
#include <QDebug>
#include <cstdlib>
#include <QSqlError>
#include "util.h"
#include <QCoreApplication>
Database * Database::db = new Database("127.0.0.1");

Database::Database(const QString & ip)
{
    QCoreApplication::addLibraryPath("C:\\Qt\\Qt5.13.2\\5.13.2\\mingw73_32\\plugins");
    sqldb = QSqlDatabase::addDatabase("QSQLITE");
    sqldb.setHostName(ip);
    sqldb.setDatabaseName("C:\\Users\\Administrator\\Desktop\\mydatabase.db");
    if(!sqldb.open()){
        qDebug() << "Unable to open database";
        qDebug() << sqldb.lastError();
        exit(-1);
    }else{
        qDebug() << "Database connection established";
    }

    query = QSqlQuery(sqldb);

    bool hasUserTable = false;
    QStringList tables = sqldb.tables();
    QStringListIterator itr(tables);

    while (itr.hasNext()){
        QString name = itr.next();
        qDebug() << name;
        if (name == "user"){
            hasUserTable = true;
            break;
        }
    }
    if (!hasUserTable){
        if(!query.exec("create table user(id int primary key,"
                       "win32username text, "
                       "win32password text, "
                       "username text, "
                       "password text);"))
        {
            qDebug() << "Error: Fail to create table."<< query.lastError();
            exit(-1);
        }

        else
        {
            qDebug() << "Table created!";
        }
    }
}

bool Database::createUser(QString username, QString password, QString & win32username, QString & win32password){
    if (!checkUser(username, password)){
        win32username = Util::getRandomString(20, Util::letters);
        win32password = Util::getRandomString(20, Util::letternums);
        qDebug() << win32username;
        qDebug() << win32password;
        QString execQueryStr = QString("INSERT INTO user(win32username, win32password,"
                                            "username, password) VALUES(\"%1\", \"%2\", \"%3\", \"%4\");")
                                            .arg(win32username)
                                            .arg(win32password)
                                            .arg(username)
                                            .arg(password);
        if(!query.exec(execQueryStr))
        {
            qDebug() << query.lastError();
            exit(-1);
        }
        return true;
    }
    return false;
}

bool Database::checkUser(QString username, QString password){
    QString execQueryStr = QString("select * from user where username=\"%1\" and password=\"%2\"").arg(username).arg(password);
    if(!query.exec(execQueryStr))
    {
        qDebug() << query.lastError();
        exit(-1);
    }
    return query.next();
}

bool Database::queryUser(QString username, QString password, QString & win32username, QString & win32password){
    if (checkUser(username, password)){
        int id = query.value(0).toInt();
        win32username = query.value(1).toString();
        win32password = query.value(2).toString();
        return true;
    }
    return false;
}

bool Database::deleteUser(QString username, QString password){
    if (checkUser(username, password)){
        QString execQueryStr = QString("DELETE FROM user where username=\"%1\""
                                            "and password=\"%2\";")
                                            .arg(username)
                                            .arg(password);
        if(!query.exec(execQueryStr))
        {
            qDebug() << query.lastError();
            exit(-1);
        }
        return true;
    }
    return false;
}

Database::~Database(){
    sqldb.close();
}
