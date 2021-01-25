#ifndef MESSAGE_H
#define MESSAGE_H
#include <QByteArray>
#include <QTcpSocket>
#include <QString>

#define CHARARRAY_LENGTH 1024

/* User logins */
#define CREATE_USER_SUCCESS 0
#define CREATE_USER_FAIL 1
#define OPEN_PROGRAM_FAIL 2
#define LOGIN_USER_SUCCESS 3
#define LOGIN_USER_FAIL 4
#define NO_COMMAND 5

/* Messages */
#define KEYDOWNEVENT 3
#define KEYUPEVENT 4
#define MOUSEPRESSEVENT 5
#define MOUSERIGHTPRESSEVENT 6
#define MOUSEMIDDLEPRESSEVENT 7
#define MOUSEMOVEEVENT 8
#define MOUSESCROLLEVENT 9
#define MOUSERELEASEEVENT 10
#define MOUSERIGHTRELEASEEVENT 11
#define MOUSEMIDDLERELEASEEVENT 12

#define ScrollUp 0
#define ScrollDown -1

QString letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
QString letternums = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

typedef struct keydownevent {
    int keycode;
} KeyDownEvent;

typedef struct keyupevent {
    int keycode;
} KeyUpEvent;

typedef struct mousepressevent_t{
    int x;
    int y;
} MousePressEvent;

typedef struct mousemoveevent_t{
    int x;
    int y;
} MouseMoveEvent;

typedef struct mouserightpressevent_t{
    int x;
    int y;
} MouseRightPressEvent;

typedef struct mousemiddlepressevent_t{
    int x;
    int y;
} MouseMiddlePressEvent;

typedef struct mousescrollevent_t{
    unsigned char pos;
} MouseScrollEvent;

typedef struct mousereleaseevent_t{
    int x;
    int y;
} MouseReleaseEvent;

typedef struct mouserightreleaseevent_t{
    int x;
    int y;
} MouseRightReleaseEvent;

typedef struct mousemiddlereleaseevent_t{
    int x;
    int y;
} MouseMiddleReleaseEvent;

typedef union event_t{
    KeyDownEvent keydownevent;
    KeyUpEvent keyupevent;
    MousePressEvent mousepressevent;
    MouseMoveEvent mousemoveevent;
    MouseScrollEvent mousescrollevent;
    MouseRightPressEvent mouserightpressevent;
    MouseMiddlePressEvent mousemiddlepressevent;
    MouseReleaseEvent mousereleaseevent;
    MouseRightReleaseEvent mouserightreleaseevent;
    MouseMiddleReleaseEvent mousemiddlereleaseevent;
} EventInfo;

typedef struct message_t {
    unsigned char message_type;
    EventInfo message_info;
} Message;

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

#endif // MESSAGE_H
