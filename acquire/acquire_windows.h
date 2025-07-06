#ifndef ACQUIRE_WINDOWS_H
#define ACQUIRE_WINDOWS_H

typedef int BOOL;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *PVOID;
typedef PVOID HANDLE;
typedef HANDLE HWND;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef unsigned char BYTE;
typedef CHAR *LPSTR;
typedef const CHAR *LPCSTR;
typedef wchar_t WCHAR;
typedef const WCHAR *LPCWSTR;

#include <minwindef.h>

#include <minwinbase.h>

#include <basetsd.h>

#endif /* ACQUIRE_WINDOWS_H */
