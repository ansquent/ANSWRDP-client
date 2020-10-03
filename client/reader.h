#ifndef READER_H
#define READER_H
#include <QByteArray>
#include <QTcpSocket>

inline int toInt(const QByteArray & intBytes){
    //assert(intBytes.size() == sizeof(int));
    return *((const int *)(intBytes.data()));
}

inline bool readNow(QTcpSocket * socket, int * expected_length, QByteArray & remains,
             QByteArray & length_buffer){
    //指示区：一个整数，代表数据区的大小
    const int length_expected_length = sizeof (int);
    if (*expected_length <= 0){
        //指示区没读完，数据区大小不知
        length_buffer += socket->read(length_expected_length - length_buffer.size());
        if (length_buffer.size() < length_expected_length){
            //需要接着读指示区
            return false;
        }
        else {
            *expected_length = toInt(length_buffer);
            //assert(*expected_length >= 0);
            length_buffer.clear();
        }
    }

    //接着读剩下的，是数据
    remains += socket->read(*expected_length - remains.size());
    //需要接着读数据
    //assert(remains.size() <= *expected_length);
    return remains.size() == *expected_length;
}

#endif // READER_H
