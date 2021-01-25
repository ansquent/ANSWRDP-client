#ifndef UTIL_H
#define UTIL_H
#include <QByteArray>
#include <QTcpSocket>
#include <QString>
#include <QFile>
#include "constants.h"

inline void info(const char *format, ...) {
    char buf[500];
    va_list vlist;
    va_start(vlist, format);
    vsprintf(buf, format, vlist);
    va_end(vlist);
    fprintf(stderr, "INFO: %s\n", buf);
}

inline int toInt(const QByteArray & intBytes){
    //assert(intBytes.size() == sizeof(int));
    return *((const int *)(intBytes.data()));
}

inline bool readNow(QTcpSocket * socket, int expected_length, QByteArray & remains,
             QByteArray & length_buffer){
    //指示区：一个整数，代表数据区的大小
    const int length_expected_length = sizeof (int);
    if (expected_length <= 0){
        //指示区没读完，数据区大小不知
        length_buffer += socket->read(length_expected_length - length_buffer.size());
        if (length_buffer.size() < length_expected_length){
            //需要接着读指示区
            return false;
        }
        else {
            expected_length = toInt(length_buffer);
            //assert(*expected_length >= 0);
            length_buffer.clear();
        }
    }

    //接着读剩下的，是数据
    remains += socket->read(expected_length - remains.size());
    //需要接着读数据
    //assert(remains.size() <= *expected_length);
    return remains.size() == expected_length;
}

inline void writeNow(QTcpSocket *socket, const QByteArray & data){
    QByteArray begins;
    int length = data.size();
    for (unsigned i = 0; i < sizeof(int); ++i){
        begins += *(reinterpret_cast<char *>(&length) + i);
    }
    socket->write(begins);
    socket->write(data);
    socket->flush();
}

template <class T>
inline void addToQByteArray(T t, QByteArray & data){
    for (unsigned i = 0; i < sizeof(t); ++i){
        data += *(reinterpret_cast<char *>(&t) + i);
    }
}

inline QString getRandomLetters(int length){
    QString letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString result;
    for (int i = 0; i < length; ++i){
        result += letters.at(qrand() % letters.length());
    }
    return result;
}

inline QString getRandomLetterNums(int length){
    QString letternums = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    QString result;
    for (int i = 0; i < length; ++i){
        result += letternums.at(qrand() % letternums.length());
    }
    return result;
}

inline void writeToFile(QString filename, const QByteArray & data){
    QFile f(filename);
    if(!f.open(QIODevice::WriteOnly))
    {
        info("Open failed\n");
    }
    else {
        if (f.write(data) != data.size()){
            info("Write failed\n");
        }
    }
    f.close();
}
#endif // UTIL_H
