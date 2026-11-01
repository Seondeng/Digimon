#ifndef SEONDAENG_CAPTURE_PROTECTION_H
#define SEONDAENG_CAPTURE_PROTECTION_H
#include <windows.h>
/* Prefer exclusion on Windows 10 2004+; older systems can show a blank window. */
static BOOL protectCaptureWindow(HWND window){
 if(!window)return FALSE;
 if(SetWindowDisplayAffinity(window,0x00000011))return TRUE;
 return SetWindowDisplayAffinity(window,WDA_MONITOR);
}
#endif
