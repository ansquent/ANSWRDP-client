#include <windows.h>
#include <string>

#ifndef NEW_PROGRAM_PROCESSOR
#define NEW_PROGRAM_PROCESSOR

HWND openNewProgram(WCHAR * programname);
void closeNewProgram(HDESK hDesk);

#endif // !NEW_PROGRAM_PROCESSOR

