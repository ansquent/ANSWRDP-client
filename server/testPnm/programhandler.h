#ifndef NEW_PROGRAM_H
#define NEW_PROGRAM_H

#include <windows.h>
#include <string>
#include <set>

HWND openNewProgram(WCHAR *);
BOOL bringWindowToTop(HWND);
void translateToScreenPoint(HWND, int *, int *, int, int);

#endif // !NEW_PROGRAM_H

