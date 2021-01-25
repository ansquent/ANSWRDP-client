#ifndef DATABASE_H
#define DATABASE_H
#include <QString>
#include <QSqlQuery>
#include <QStringList>
#include <QSqlDatabase>

class Database
{
private:
    QSqlDatabase sqldb;
    Database(const QString & ip);
    static Database * db;
    QSqlQuery query;
    bool checkUser(QString, QString);
public:    
    static Database * getDatabase(){
        return db;
    }
    bool createUser(QString, QString, QString &, QString &);
    bool queryUser(QString, QString, QString &, QString &);
    bool deleteUser(QString, QString);
    virtual ~Database();
};

#endif // DATABASE_H
