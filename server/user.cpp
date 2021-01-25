#include "user.h"
#include "database.h"
#include <QString>
#include <Windows.h>
#include <lm.h>
#include <QDebug>
#include "util.h"

User::User():token(nullptr){}

bool User::login(QString username, QString password){
    if (Database::getDatabase()->queryUser(username, password, this->win32username, this->win32password)){
        this->username = username;
        this->password = password;

        WCHAR win32username[1024] = {0},
              win32password[1024] = {0};
        this->win32username.toWCharArray(win32username);
        this->win32password.toWCharArray(win32password);
        if (!LogonUserW(win32username, nullptr, win32password, LOGON32_LOGON_BATCH,
                       LOGON32_PROVIDER_DEFAULT, &token)){
            qDebug() << "LogonUser Failed!";
            return false;
        }
        return true;
    }
    return false;
}

bool User::reg(QString username, QString password){
    if (!Database::getDatabase()->createUser(username, password, this->win32username, this->win32password)){
        qDebug() << "Failed Create User in Database!";
        return false;
    }
    this->username = username;
    this->password = password;

    USER_INFO_1 new_user;
    memset(&new_user, 0, sizeof(new_user));
    WCHAR win32username[1024] = {0},
          win32password[1024] = {0},
          win32comment[1024] = {0};
    this->win32username.toWCharArray(win32username);
    this->win32password.toWCharArray(win32password);
    QString("RDP Promote").toWCharArray(win32comment);
    new_user.usri1_name = win32username; // Allocates the username
    new_user.usri1_password = win32password; // Allocates the password
    new_user.usri1_priv = USER_PRIV_USER; // Sets the account type to USER_PRIV_USER
    new_user.usri1_home_dir = nullptr; // We didn't supply a Home Directory
    new_user.usri1_comment = win32comment; // Comment on the User
    new_user.usri1_script_path = nullptr; // We didn't supply a Logon Script Path
    NET_API_STATUS re = NetUserAdd(nullptr, 1, (LPBYTE)&new_user, nullptr);

    while (re == NERR_UserExists || re == NERR_PasswordTooShort || re == NERR_PasswordTooRecent){
        Database::getDatabase()->deleteUser(username, password);
        Database::getDatabase()->createUser(username, password, this->win32username, this->win32password);
        this->win32username.toWCharArray(win32username);
        this->win32password.toWCharArray(win32password);
        new_user.usri1_name = win32username; // Allocates the username
        new_user.usri1_password = win32password; // Allocates the password
        re = NetUserAdd(nullptr, 1, (LPBYTE)&new_user, nullptr);
    }

    if(re == NERR_Success){
        qDebug() << "Succeeded Create Net User!";
    }
    else{
        qDebug() << "Failed Create Net User!";
        qDebug() << re;
        return false;
    }    

    LOCALGROUP_MEMBERS_INFO_3 account;
    account.lgrmi3_domainandname = win32username;

    re = NetLocalGroupAddMembers(nullptr, L"Administrators", 3, (LPBYTE)&account, 1);
    if (re == NERR_Success || re == ERROR_MEMBER_IN_ALIAS) {
       qDebug() << "Succeeded Add to local Group!";
    }
    else {
        qDebug() << "Failed Add to local Group!";
        qDebug() << re;
        return false;
    }

    return true;
}

bool User::runProgram(QString programName){
    WCHAR win32ProgramName[1024] = {0};
    programName.toWCharArray(win32ProgramName);
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = TRUE;
    qDebug() << "token" << token;
    BOOL bRet = CreateProcessAsUserW(token, nullptr, win32ProgramName,
                         nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE,
                         nullptr, nullptr, &si, &pi);
    DWORD err = GetLastError();
    if (!bRet){
        qDebug() << "err" << err;
        qDebug() << "CreateProcessAsUser Failed!";
        return false;
    }
    else {
        qDebug() << "CreateProcessAsUser Succeeded!";
        return true;
    }
}
