#ifndef DATABASE_H
#define DATABASE_H

#include <map>
#include <windows.h>
#include "newprogram.h"
#include <thread>
#include <QMutex>
#include <QString>
#include "configs.h"

class Database {
private:
    std::map<QString, QString> username_password;
    std::map<QString, HWND> username_hwnd;
    QMutex this_mutex;
public:

    int login(const QString & username,
               const QString & password){
        this_mutex.lock();
        bool result = LOGIN_USER_FAIL;
        if (username_password.find(username) != username_password.end() &&
            username_password[username] == password){
            result = LOGIN_USER_SUCCESS;
            goto clean;
        }

        clean:
            this_mutex.unlock();
            return result;
    }

    int createUser(const QString & username,
                   const QString & password,
                   const QString & programname){
        this_mutex.lock();
        int return_status = CREATE_USER_SUCCESS;
        HWND hwnd;
        WCHAR * lConst = nullptr;

        if (username_password.find(username) != username_password.end()){
            return_status = CREATE_USER_EXIST_ERROR;
            goto clean;
        }
        username_password[username] = password;
        lConst = new WCHAR[CHARARRAY_LENGTH];
        memset(lConst, 0, sizeof(WCHAR) * CHARARRAY_LENGTH);
        programname.toWCharArray(lConst);
        hwnd = openNewProgram(lConst);
        if (!hwnd){
            return_status = CREATE_USER_NOTEXIST_PROGRAM_ERROR;
            goto clean;
        }
        username_hwnd[username] = hwnd;

        clean:
            this_mutex.unlock();
            delete []lConst;
            return return_status;
    }

    HWND get_hwnd(const QString & username){
        this_mutex.lock();
        HWND result = username_hwnd[username];
        this_mutex.unlock();
        return result;
    }
};

#endif // DATABASE_H
