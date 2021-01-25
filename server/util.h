#ifndef UTIL_H
#define UTIL_H
#include <QString>
#include "mysocket.h"
#include <QByteArray>
#include <QTcpSocket>

class Util
{
private:
    Util();
public:
    static QString letters;
    static QString letternums;
    static void initialize();
    static QString getRandomString(int, QString);
    static int toInt(const QByteArray &);
    static bool readNow(MySocket *);
    static void writeNow(QTcpSocket *, const QByteArray &);
    template <class T>
    static void addToQByteArray(T t, QByteArray & data){
        //qDebug() << t;
        for (unsigned i = 0; i < sizeof(t); ++i){
            data += *(reinterpret_cast<char *>(&t) + i);
        }
    }
    static void writeToFile(QString, const QByteArray &);
};

#endif // UTIL_H
