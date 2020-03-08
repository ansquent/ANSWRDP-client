#ifndef USERINFO_H
#define USERINFO_H
#include <QString>
#include <windows.h>
#include "mysocket.h"

typedef struct userinfo_t{
    QString username;
    ::HWND hwnd;
    MySocket * socket;
}Userinfo;

#endif // USERINFO_H
