#include "util.h"
#include <QString>
#include <QDateTime>
#include "mysocket.h"
#include <QByteArray>
#include <QTcpSocket>
#include <QFile>

QString Util::letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
QString Util::letternums = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void Util::initialize(){
    qsrand(QDateTime::currentMSecsSinceEpoch());
}

QString Util::getRandomString(int length, QString range){
    QString result;
    for (int i = 0; i < length; ++i){
        result += range.at(qrand() % range.length());
    }
    return result;
}

int Util::toInt(const QByteArray & intBytes){
    //assert(intBytes.size() == sizeof(int));
    return *((const int *)(intBytes.data()));
}

bool Util::readNow(MySocket * socket){
    //指示区：一个整数，代表数据区的大小
    const int length_expected_length = sizeof (int);
    if (socket->getExpectedLength() <= 0){
        //指示区没读完，数据区大小不知
        *socket->getLengthBuffer() += socket->getMySocket()->read(length_expected_length - socket->getLengthBuffer()->size());
        if (socket->getLengthBuffer()->size() < length_expected_length){
            //需要接着读指示区
            return false;
        }
        else {
            socket->setExpectedLength(toInt(*socket->getLengthBuffer()));
            //assert(socket->getExpectedLength() >= 0);
            socket->getLengthBuffer()->clear();
        }
    }

    //接着读剩下的，是数据
    *socket->getBuffer() += socket->getMySocket()->read(socket->getExpectedLength() - socket->getBuffer()->size());
    //需要接着读数据
    //assert(socket->getBuffer()->size() <= socket->getExpectedLength());
    return socket->getBuffer()->size() == socket->getExpectedLength();
}

void Util::writeNow(QTcpSocket *socket, const QByteArray & data){
    QByteArray begins;
    int length = data.size();
    for (unsigned i = 0; i < sizeof(int); ++i){
        begins += *(reinterpret_cast<char *>(&length) + i);
    }
    socket->write(begins);
    socket->write(data);
    socket->flush();
}

void Util::writeToFile(QString filename, const QByteArray & data){
    QFile f(filename);
    if(!f.open(QIODevice::WriteOnly))
    {
        qDebug() << "Open failed." << endl;
    }
    else {
        if (f.write(data) != data.size()){
            qDebug() << "Write failed." << endl;
        }
    }
    f.close();
}
