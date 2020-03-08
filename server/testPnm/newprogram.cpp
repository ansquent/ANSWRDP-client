#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <QDebug>

LONG waitTime = 30;
LONG sleepTime = 1000;

typedef struct {
    DWORD willTID;
    HWND willhwnd;
}LPM_ENUM;

BOOL CALLBACK EnumChildProc(_In_ HWND   hwnd, _In_ LPARAM lParam)
{
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    //printf("child: %d\n", tid);
    if (tid == ((LPM_ENUM*)lParam)->willTID) {
        RECT rect = {0};
        GetWindowRect(hwnd, &rect);
        if (rect.left < rect.right){
            ((LPM_ENUM*)lParam)->willhwnd = hwnd;
            return FALSE;
        }
    }
    EnumChildWindows(hwnd, EnumChildProc, lParam);
    return TRUE;
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    //printf("father: %d\n", tid);
    if (tid == ((LPM_ENUM *)lParam)->willTID) {
        RECT rect = {0};
        GetWindowRect(hwnd, &rect);
        if (rect.left < rect.right){
            ((LPM_ENUM*)lParam)->willhwnd = hwnd;
            return FALSE;
        }
    }
    EnumChildWindows(hwnd, EnumChildProc, lParam);
    return TRUE;
}
// 通过进程ID获取窗口句柄
HWND GetWindowHwndByPID(PROCESS_INFORMATION pi)
{
    LPM_ENUM lpm;
    lpm.willTID = pi.dwThreadId;
    lpm.willhwnd = NULL;
    DWORD timer = 0;
    while (timer++ < waitTime) {
        EnumWindows(EnumWindowsProc, (LPARAM)&lpm);
        if (lpm.willhwnd != NULL) {
            return lpm.willhwnd;
        }
        else {
            Sleep(sleepTime);
        }
    }
    return lpm.willhwnd;
}

HWND openNewProgram(WCHAR * programname) {
    //StartInteractiveClientProcess(username, NULL, password, program);
    //*hDesk = CreateDesktop(desktopname, NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
    HANDLE hProcess = 0;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(STARTUPINFO);
    //si.lpDesktop = desktopname; //注意这里

    //DWORD mythreadID = GetCurrentThreadId();
    //HDESK myDesk = GetThreadDesktop(mythreadID);

    if( !CreateProcess( NULL,   // No module name (use command line)
            programname,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory
            &si,            // Pointer to STARTUPINFO structure
            &pi )           // Pointer to PROCESS_INFORMATION structure
        )
        return 0;
    Sleep(sleepTime);
    HWND hwnd;

    //SetThreadDesktop(*hDesk);
    //SwitchDesktop(*hDesk);
    hwnd = GetWindowHwndByPID(pi);
    //SavePrintWindowToFile(hwnd);
    //SetThreadDesktop(myDesk); //显示原桌面
    //SwitchDesktop(myDesk);
    //getWindowRect(hwnd);
    qDebug() << "success\n";
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return hwnd;
}

void closeNewProgram(HDESK hDesk) {
    CloseDesktop(hDesk);
    CloseHandle(hDesk);
}
