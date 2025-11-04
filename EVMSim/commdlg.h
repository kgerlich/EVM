/*
 * commdlg.h stub
 *
 * Minimal stub for Windows common dialogs header.
 * The actual common dialog functions are already handled in windows-compat.h.
 */

#ifndef __COMMDLG_H__
#define __COMMDLG_H__

#ifdef _WIN32
    /* Windows build - include actual header */
    #include <commdlg.h>
#else
    /* Non-Windows build - already handled by windows-compat.h stubs */
#endif

#endif /* __COMMDLG_H__ */
