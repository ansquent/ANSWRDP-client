#include "usermanager.h"
#include <QString>
#include <Windows.h>
#include <lm.h>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QVariant>
#include "constants.h"

#ifdef _WIN32
#ifndef _WIN64
#pragma comment(lib,"../server/third_party/win32/lib/x86/NetAPI32.Lib")
#pragma comment(lib,"../server/third_party/win32/lib/x86/UserEnv.Lib")
#pragma comment(lib,"../server/third_party/win32/lib/x86/AdvAPI32.Lib")
#else
#pragma comment(lib,"../server/third_party/win32/lib/x64/NetAPI32.Lib")
#pragma comment(lib,"../server/third_party/win32/lib/x64/UserEnv.Lib")
#pragma comment(lib,"../server/third_party/win32/lib/x64/AdvAPI32.Lib")
#endif
#endif

UserManager::UserManager(QString server):token(nullptr){
    //QCoreApplication::addLibraryPath("C:\\Qt\\Qt5.13.2\\5.13.2\\mingw73_32\\plugins");
    this->server = server;
    sqldb = QSqlDatabase::addDatabase("QSQLITE");
    sqldb.setHostName("127.0.0.1");
    QString filename = QDir::currentPath() +QString("/")+ QString("user.db");
    sqldb.setDatabaseName(filename);
    if(!sqldb.open()){
        info("Unable to open database: %s", sqldb.lastError().text().toStdString().c_str());
        exit(-1);
    }else{
        info("Database connection established");
    }

    query = QSqlQuery(sqldb);

    bool hasUserTable = false;
    QStringList tables = sqldb.tables();
    QStringListIterator itr(tables);

    while (itr.hasNext()){
        QString name = itr.next();
        info("name: %s", name.toStdString().c_str());
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
            info("Error: Fail to create table. %s", query.lastError().text().toStdString().c_str());
            exit(-1);
        }

        else
        {
            info("Table created!");
        }
    }
}

bool UserManager::login(QString username, QString password){
    if (queryUser(username, password, this->win32username, this->win32password)){
        this->username = username;
        this->password = password;

        WCHAR win32username[1024] = {0},
              win32password[1024] = {0};
        this->win32username.toWCharArray(win32username);
        this->win32password.toWCharArray(win32password);
        if (!LogonUserW(win32username, nullptr, win32password, LOGON32_LOGON_BATCH,
                       LOGON32_PROVIDER_DEFAULT, &token)){
            info("LogonUser Failed!");
            return false;
        }
        return true;
    }
    return false;
}

bool UserManager::reg(QString username, QString password){
    if (!createUser(username, password, this->win32username, this->win32password)){
        info("Failed Create User in Database!");
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
        deleteUser(username, password);
        createUser(username, password, this->win32username, this->win32password);
        this->win32username.toWCharArray(win32username);
        this->win32password.toWCharArray(win32password);
        new_user.usri1_name = win32username; // Allocates the username
        new_user.usri1_password = win32password; // Allocates the password
        re = NetUserAdd(nullptr, 1, (LPBYTE)&new_user, nullptr);
    }

    if(re == NERR_Success){
        info("Succeeded Create Net User!");
    }
    else{
        info("Failed Create Net User! %d", re);
        return false;
    }    

    LOCALGROUP_MEMBERS_INFO_3 account;
    account.lgrmi3_domainandname = win32username;

    re = NetLocalGroupAddMembers(nullptr, L"Administrators", 3, (LPBYTE)&account, 1);
    if (re == NERR_Success || re == ERROR_MEMBER_IN_ALIAS) {
       info("Succeeded Add to local Group!");
    }
    else {
        info("Failed Add to local Group! %d\n", re);
        return false;
    }

    return true;
}

bool UserManager::runProgram(QString programName){
    WCHAR win32ProgramName[1024] = {0};
    programName.toWCharArray(win32ProgramName);
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = TRUE;
    info("token: %d", token);
    BOOL bRet = CreateProcessAsUserW(token, nullptr, win32ProgramName,
                         nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE,
                         nullptr, nullptr, &si, &pi);
    DWORD err = GetLastError();
    if (!bRet){
        info("err: %d CreateProcessAsUser Failed!", err);
        return false;
    }
    else {
        info("CreateProcessAsUser Succeeded!");
        return true;
    }
}

bool UserManager::createUser(QString username, QString password, QString & win32username, QString & win32password){
    if (!checkUser(username, password)){
        win32username = getRandomLetters(20);
        win32password = getRandomLetterNums(20);
        info("win32username %s", win32username.toStdString().c_str());
        info("win32password %s", win32password.toStdString().c_str());
        QString execQueryStr = QString("INSERT INTO user(win32username, win32password,"
                                            "username, password) VALUES(\"%1\", \"%2\", \"%3\", \"%4\");")
                                            .arg(win32username)
                                            .arg(win32password)
                                            .arg(username)
                                            .arg(password);
        if(!query.exec(execQueryStr))
        {
            info("Query error: %d", query.lastError().text().toStdString().c_str());
            exit(-1);
        }
        return true;
    }
    return false;
}

bool UserManager::checkUser(QString username, QString password){
    QString execQueryStr = QString("select * from user where username=\"%1\" and password=\"%2\"").arg(username).arg(password);
    if(!query.exec(execQueryStr))
    {
        info("Query error: %d", query.lastError().text().toStdString().c_str());
        exit(-1);
    }
    return query.next();
}

bool UserManager::queryUser(QString username, QString password, QString & win32username, QString & win32password){
    if (checkUser(username, password)){
        win32username = query.value(1).toString();
        win32password = query.value(2).toString();
        return true;
    }
    return false;
}

bool UserManager::deleteUser(QString username, QString password){
    if (checkUser(username, password)){
        QString execQueryStr = QString("DELETE FROM user where username=\"%1\""
                                            "and password=\"%2\";")
                                            .arg(username)
                                            .arg(password);
        if(!query.exec(execQueryStr))
        {
            info("Query error: %d", query.lastError().text().toStdString().c_str());
            exit(-1);
        }
        return true;
    }
    return false;
}
