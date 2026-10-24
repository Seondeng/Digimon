#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include "WebView2.h"
#define EDITION_FOLDER L"Public"
static HWND win;
static wchar_t root[4096],userData[4096],errorText[4600];
static ICoreWebView2Controller *controller;
static ICoreWebView2 *view;
static ICoreWebView2Environment *environment;
static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler envHandler;
static int errorCount,calls,failFirst;
static wchar_t profiles[4][4096];
static void error(const wchar_t *message){errorCount++;wcsncpy_s(errorText,4600,message,_TRUNCATE);}
#include "webview_startup.h"
static HRESULT STDAPICALLTYPE factory(PCWSTR exe,PCWSTR profile,ICoreWebView2EnvironmentOptions *options,ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *handler){
 (void)exe;(void)options;(void)handler;
 if(calls>=4)return E_FAIL;
 wcsncpy_s(profiles[calls],4096,profile,_TRUNCATE);calls++;
 return failFirst&&calls==1?E_ACCESSDENIED:S_OK;
}
static LRESULT CALLBACK testProc(HWND h,UINT msg,WPARAM w,LPARAM l){
 if(msg==WEBVIEW_RETRY_MESSAGE){beginWebViewStartup();return 0;}
 return DefWindowProcW(h,msg,w,l);
}
static void pump(void){MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
static HWND window(void){return CreateWindowW(L"WebViewRecoveryTest",L"Recovery test",WS_OVERLAPPED,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);}
static int writeFile(const wchar_t *path){HANDLE h=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(h==INVALID_HANDLE_VALUE)return 0;CloseHandle(h);return 1;}
#define CHECK(x,msg) do{if(!(x)){puts(msg);return 1;}}while(0)
int main(void){
 wchar_t temp[4096],original[4096],sentinel[4096],blocked[4096],log[4096];
 CHECK(GetTempPathW(4096,temp)>0,"Temporary directory unavailable");
 _snwprintf(root,4096,L"%lsSeondaeng-Recovery-Test-%lu",temp,GetCurrentProcessId());CHECK(CreateDirectoryW(root,NULL),"Fixture directory failed");
 _snwprintf(original,4096,L"%ls\\Original",root);CHECK(CreateDirectoryW(original,NULL),"Original profile fixture failed");
 _snwprintf(sentinel,4096,L"%ls\\keep.txt",original);CHECK(writeFile(sentinel),"Saved-data fixture failed");
 WNDCLASSW wc={0};wc.lpfnWndProc=testProc;wc.hInstance=GetModuleHandleW(NULL);wc.lpszClassName=L"WebViewRecoveryTest";CHECK(RegisterClassW(&wc),"Test window class failed");
 win=window();CHECK(win,"Test window failed");
 wcsncpy_s(userData,4096,original,_TRUNCATE);webViewFactory=factory;failFirst=1;
 beginWebViewStartup();CHECK(calls==1&&webViewRetryPending,"Synchronous creation failure was not queued for recovery");
 pump();CHECK(calls==2&&webViewAttempt==1&&!webViewRetryPending&&!errorCount,"First profile recovery failed");
 CHECK(wcscmp(profiles[0],profiles[1])&&GetFileAttributesW(sentinel)!=INVALID_FILE_ATTRIBUTES,"Existing saved data was changed");
 failedWebViewStartup(E_FAIL,L"Controller callback");pump();CHECK(calls==3&&webViewAttempt==2&&!errorCount,"Controller failure did not use the second recovery profile");
 CHECK(wcscmp(profiles[1],profiles[2]),"Recovery reused the failing profile");
 failedWebViewStartup(E_FAIL,L"Controller callback");pump();CHECK(errorCount==1&&!IsWindow(win),"Recovery did not stop after bounded retries");
 CHECK(wcsstr(errorText,L"0x80004005")&&wcsstr(errorText,L"Controller callback"),"Final error lost the diagnostic code or stage");
 CHECK(GetFileAttributesW(sentinel)!=INVALID_FILE_ATTRIBUTES,"Original profile was deleted on final failure");
 RemoveDirectoryW(profiles[1]);RemoveDirectoryW(profiles[2]);
 _snwprintf(blocked,4096,L"%ls\\Blocked",root);CHECK(writeFile(blocked),"Access-blocked fixture failed");
 _snwprintf(userData,4096,L"%ls\\Profile",blocked);originalWebViewProfile[0]=0;webViewAttempt=0;webViewRetryPending=0;calls=0;errorCount=0;failFirst=0;win=window();
 beginWebViewStartup();pump();CHECK(calls==1&&webViewAttempt==2&&!errorCount,"Unusable local profile did not fall back to a writable temp profile");
 CHECK(!wcsncmp(userData,temp,wcslen(temp)),"Temp fallback used an unexpected location");
 DestroyWindow(win);RemoveDirectoryW(userData);
 DeleteFileW(sentinel);RemoveDirectoryW(original);DeleteFileW(blocked);_snwprintf(log,4096,L"%ls\\webview-startup.log",root);DeleteFileW(log);RemoveDirectoryW(root);
 puts("WebView startup recovery passed: synchronous/controller failures, fresh writable profiles, temp fallback, bounded retries, preserved saved data and HRESULT diagnostics.");
 return 0;
}
