#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include "capture_protection.h"
int main(void){
 if(protectCaptureWindow(NULL)){puts("NULL window accepted");return 1;}
 for(int layered=0;layered<2;layered++){
  HWND h=CreateWindowExW(layered?WS_EX_LAYERED|WS_EX_TOOLWINDOW:0,L"STATIC",L"Capture exclusion test",WS_OVERLAPPEDWINDOW,10,10,300,180,NULL,NULL,GetModuleHandleW(NULL),NULL);
  if(!h){puts("Window creation failed");return 2;}
  if(layered)SetLayeredWindowAttributes(h,0,209,LWA_ALPHA);
  if(!protectCaptureWindow(h)){printf("Cannot enable capture protection: %lu\n",GetLastError());DestroyWindow(h);return 3;}
  DWORD affinity=0;if(!GetWindowDisplayAffinity(h,&affinity)||affinity!=0x11){printf("Unexpected affinity: %lu\n",affinity);DestroyWindow(h);return 4;}
  DestroyWindow(h);
 }
 puts("Main-style and layered overlay capture exclusion verified (0x11)");return 0;
}
