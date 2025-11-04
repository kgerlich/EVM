/*
 * windows-compat.h
 *
 * Portable Windows API compatibility layer for cross-platform compilation.
 * Provides minimal replacements for Windows APIs used by the simulator,
 * allowing the code to compile on both Windows and WASM/Emscripten targets.
 */

#ifndef __WINDOWS_COMPAT_H__
#define __WINDOWS_COMPAT_H__

#ifdef _WIN32
    /* Windows build - use actual Windows headers */
    #include <windows.h>
#else
    /* Non-Windows build (WASM, Linux, etc.) - provide minimal stubs */

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>

    /* Define _fastcall as nothing for platforms that don't support it */
    #ifndef _fastcall
    #define _fastcall
    #endif

    /* Handle inline assembly - not supported in WASM, skip the blocks */
    /* This uses a for loop that never enters to skip the inline asm blocks safely */
    #define __asm for(;;) break; __asm_placeholder:  /* Inline assembly is not supported in WASM */
    #define __emit(x)  /* Skip __emit directives */

    /* Basic type definitions */
    typedef unsigned char BYTE;
    typedef unsigned short WORD;
    typedef unsigned long DWORD;
    typedef int BOOL;
    typedef void *HANDLE;
    typedef void *HWND;
    typedef void *HDC;
    typedef void *HINSTANCE;
    typedef unsigned long COLORREF;
    typedef void *HFONT;
    typedef void *HMENU;
    typedef unsigned int UINT;
    typedef unsigned int UINT_PTR;
    typedef unsigned long ULONG;
    typedef char *LPSTR;
    typedef const char *LPCSTR;
    typedef int INT;
    typedef void *HMODULE;

    /* WIN32_FIND_DATA structure for file enumeration */
    typedef struct {
        unsigned long dwFileAttributes;
        char cFileName[260];
        char cAlternateFileName[14];
    } WIN32_FIND_DATA;

    #define INVALID_HANDLE_VALUE ((void *)-1)

    /* Boolean constants */
    #define FALSE 0
    #define TRUE 1
    #ifndef NULL
    #define NULL ((void *)0)
    #endif

    /* Message box constants and stub */
    #define MB_OK 0x00000000
    #define MB_OKCANCEL 0x00000001
    #define MB_YESNO 0x00000004
    #define MB_ICONERROR 0x00000010
    #define MB_ICONWARNING 0x00000020
    #define MB_ICONQUESTION 0x00000020
    #define MB_ICONINFORMATION 0x00000040
    #define MB_ICONSTOP 0x00000010
    #define MB_TOPMOST 0x00040000
    #define IDOK 1
    #define IDCANCEL 2
    #define IDYES 6
    #define IDNO 7

    static inline int MessageBox(HWND hWnd, const char *lpText,
                                 const char *lpCaption, unsigned int uType)
    {
        fprintf(stderr, "[%s] %s\n", lpCaption ? lpCaption : "Message",
                lpText ? lpText : "");
        return IDOK;
    }

    /* Window management stubs */
    #define CreateWindowEx(a,b,c,d,e,f,g,h,i,j,k,l) ((HWND)0)
    #define CreateDialog(a,b,c,d) ((HWND)0)
    #define ShowWindow(a,b)
    #define UpdateWindow(a)
    #define GetWindowRect(a,b)
    #define SetWindowPos(a,b,c,d,e,f,g)
    #define GetSystemMetrics(a) 0
    #define InvalidateRect(a,b,c)
    #define GetDC(a) ((HDC)0)
    #define ReleaseDC(a,b) 0
    #define RegisterClass(a) 0
    #define LoadIcon(a,b) ((HANDLE)0)
    #define LoadCursor(a,b) ((HANDLE)0)
    #define LoadMenu(a,b) ((HMENU)0)
    #define GetStockObject(a) ((HANDLE)0)

    /* Dialog and messaging stubs */
    #define SendMessage(a,b,c,d) 0
    #define PostMessage(a,b,c,d) 0
    #define SendDlgItemMessage(a,b,c,d,e) 0
    #define SetDlgItemText(a,b,c)
    #define SetDlgItemInt(a,b,c,d)
    #define GetDlgItemInt(a,b,c,d) 0
    #define GetDlgItem(a,b) ((HWND)0)
    #define EnableWindow(a,b)

    /* Timer stubs */
    #define SetTimer(a,b,c,d) (UINT_PTR)1
    #define KillTimer(a,b)

    /* Graphics stubs */
    #define TextOut(a,b,c,d,e)
    #define SelectObject(a,b) ((HANDLE)0)
    #define SetTextColor(a,b) 0
    #define SetBkColor(a,b) 0
    #define RGB(r,g,b) ((COLORREF)0)

    /* Global memory stubs - provide simple malloc wrappers */
    #define GMEM_MOVEABLE 0x0002
    #define GMEM_FIXED 0x0000

    static inline void *GlobalAlloc(unsigned int flags, size_t size)
    {
        return malloc(size);
    }

    static inline void *GlobalLock(void *hMem)
    {
        return hMem;
    }

    static inline int GlobalUnlock(void *hMem)
    {
        return 0;
    }

    static inline void *GlobalFree(void *hMem)
    {
        if (hMem) {
            free(hMem);
        }
        return (void *)0;
    }

    static inline size_t GlobalSize(void *hMem)
    {
        return 0;  /* Cannot determine size in malloc-based implementation */
    }

    /* INI file stubs - provide simple configuration interface */
    static inline unsigned int GetPrivateProfileString(
        const char *lpAppName,
        const char *lpKeyName,
        const char *lpDefault,
        char *lpReturnedString,
        unsigned int nSize,
        const char *lpFileName)
    {
        if (lpDefault && lpReturnedString) {
            strncpy(lpReturnedString, lpDefault, nSize - 1);
            lpReturnedString[nSize - 1] = '\0';
            return strlen(lpReturnedString);
        }
        if (lpReturnedString) {
            lpReturnedString[0] = '\0';
        }
        return 0;
    }

    static inline unsigned int GetPrivateProfileInt(
        const char *lpAppName,
        const char *lpKeyName,
        int nDefault,
        const char *lpFileName)
    {
        return nDefault;
    }

    /* Threading types and stubs */
    typedef unsigned long DWORD_PTR;
    typedef unsigned int (*LPTHREAD_START_ROUTINE)(void *);
    typedef void *HANDLE;
    #define INFINITE 0xFFFFFFFF

    static inline HANDLE CreateThread(
        void *lpThreadAttributes,
        size_t dwStackSize,
        LPTHREAD_START_ROUTINE lpStartAddress,
        void *lpParameter,
        unsigned long dwCreationFlags,
        unsigned long *lpThreadId)
    {
        return NULL;  /* Threading not supported in WASM */
    }

    static inline int TerminateThread(HANDLE hThread, unsigned long dwExitCode)
    {
        return 0;
    }

    static inline int CloseHandle(HANDLE hObject)
    {
        return 1;
    }

    static inline unsigned long WaitForSingleObject(HANDLE hHandle, unsigned long dwMilliseconds)
    {
        return 0;  /* Always "signaled" in stub */
    }

    /* Resource loading stubs */
    #define MAKEINTRESOURCE(i) ((void*)(long)(i))
    #define IS_INTRESOURCE(r) (((unsigned long)(r) >> 16) == 0)

    /* File enumeration stubs - no file system in WASM */
    static inline void *FindFirstFile(const char *lpFileName, WIN32_FIND_DATA *lpFindFileData)
    {
        return INVALID_HANDLE_VALUE;  /* No files found in WASM */
    }

    static inline int FindNextFile(void *hFindFile, WIN32_FIND_DATA *lpFindFileData)
    {
        return 0;  /* No more files */
    }

    static inline int FindClose(void *hFindFile)
    {
        return 1;
    }

    /* Dynamic library loading stubs - no plugins in WASM */
    static inline void *LoadLibrary(const char *lpLibFileName)
    {
        return NULL;  /* No DLL plugins in WASM */
    }

    static inline void *GetProcAddress(void *hModule, const char *lpProcName)
    {
        return NULL;  /* No DLL plugins in WASM */
    }

    static inline int FreeLibrary(void *hLibModule)
    {
        return 1;
    }

    /* String formatting stub - for wsprintf compatibility */
    static inline int wsprintf(char *lpOut, const char *lpFmt, ...)
    {
        return 0;  /* Minimal implementation */
    }

#endif /* _WIN32 */

#endif /* __WINDOWS_COMPAT_H__ */
