#include <windows.h>
#include <iostream>
#include <string>
#include <QDebug>
#include <list>
#include <string>
#include <QThread>
#include "programhandler.h"

LONG waitTime = 30;
LONG sleepTime = 1000;

std::list<LPWSTR> desktopnames;
static LPWSTR createNewDesktopName(){
    LPWSTR newStr = new WCHAR[100];
    if (desktopnames.empty()){
        wcscpy(newStr, L"a");
        desktopnames.push_back(newStr);
    }
    else {
        wcscpy(newStr, desktopnames.back());
        for (int i = 0; newStr[i]; ++i){
            if (newStr[i] < 'z'){
                newStr[i] = newStr[i] + 1;
                desktopnames.push_back(newStr);
                return newStr;
            }
        }
        wcscat(newStr, L"a");
        desktopnames.push_back(newStr);
    }
    return desktopnames.back();
}

typedef struct {
    DWORD willTID;
    HWND willhwnd;
}LPM_ENUM;

static BOOL CALLBACK EnumChildProc(_In_ HWND   hwnd, _In_ LPARAM lParam)
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

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    //printf("father: %d\n", tid);
    if (tid == ((LPM_ENUM *)lParam)->willTID) {
        RECT rect = {0, 0, 0, 0};
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
static HWND GetWindowHwndByPID(PROCESS_INFORMATION pi, HDESK hdesk)
{
    LPM_ENUM lpm;
    lpm.willTID = pi.dwThreadId;
    lpm.willhwnd = NULL;
    DWORD timer = 0;
    while (timer++ < waitTime) {
        EnumDesktopWindows(hdesk, EnumWindowsProc, (LPARAM)&lpm);
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
    LPWSTR desktopname = createNewDesktopName();
    DWORD mythreadID = GetCurrentThreadId();
    HDESK myDesk = GetThreadDesktop(mythreadID);
    HDESK hdesk = CreateDesktop(desktopname, NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(STARTUPINFO);
    //si.lpDesktop = desktopname; //注意这里

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
    HWND hwnd = GetWindowHwndByPID(pi, myDesk);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return hwnd;
}

void translateToScreenPoint(HWND hwnd, int *x, int *y,
                                                int cx_screen, int cy_screen){
    POINT screen_coords;
    screen_coords.x = *x, screen_coords.y = *y;
    ClientToScreen(hwnd, &screen_coords);
    *x = 65535 * screen_coords.x / cx_screen;
    *y = 65535 * screen_coords.y / cy_screen;
}

BOOL bringWindowToTop(HWND hWnd)
{
    HWND hFrgWnd = GetForegroundWindow();
    AttachThreadInput( GetWindowThreadProcessId(hFrgWnd, NULL), GetCurrentThreadId(), TRUE );
    SetForegroundWindow(hWnd);
    BringWindowToTop(hWnd);
    SwitchToThisWindow(hWnd, TRUE);
    AttachThreadInput(GetWindowThreadProcessId(hFrgWnd, NULL),
        GetCurrentThreadId(), FALSE);
    return TRUE;
}
