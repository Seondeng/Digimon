#ifndef UNICODE
#define UNICODE
#endif
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include "WebView2.h"
#include "release_id.h"
#include "update_config.h"
#ifdef FULL_EDITION
#define EDITION_LABEL L"전체용"
#define EDITION_FOLDER L"Full"
#define EDITION_PROFILE L"WebView2-Full"
#else
#define EDITION_LABEL L"배포용"
#define EDITION_FOLDER L"Public"
#define EDITION_PROFILE L"WebView2-Public"
#endif
static HWND win;
static HINSTANCE instance;
static ICoreWebView2Controller *controller;
static ICoreWebView2 *view;
static ICoreWebView2Environment *environment;
static wchar_t root[4096],indexPath[4096],userData[4096];
static int pageReady=0,noticeStarted=0;
static int overlayMode=0;
static wchar_t overlayView[64];
static int overlayDragging=0;static POINT overlayDragCursor;static RECT overlayDragRect;
typedef struct {wchar_t id[64];HANDLE process;DWORD pid;} OverlayChild;
static OverlayChild overlayChildren[16];

static SYSTEMTIME nowKst;
static void clockNow(void){FILETIME ft;GetSystemTimeAsFileTime(&ft);ULARGE_INTEGER n;n.LowPart=ft.dwLowDateTime;n.HighPart=ft.dwHighDateTime;n.QuadPart+=9ULL*60*60*10000000;ft.dwLowDateTime=n.LowPart;ft.dwHighDateTime=n.HighPart;FileTimeToSystemTime(&ft,&nowKst);}
static void publishNoticeSync(void);
#include "notice_sync.h"
typedef struct {wchar_t *text;size_t used,cap;} JsonWriter;
static void jsonAppend(JsonWriter *w,const wchar_t *s){size_t n=wcslen(s);if(w->used+n>=w->cap)return;wmemcpy(w->text+w->used,s,n);w->used+=n;w->text[w->used]=0;}
static void jsonString(JsonWriter *w,const wchar_t *s){jsonAppend(w,L"\"");for(;*s;s++){wchar_t a[8];if(*s=='"'||*s=='\\'){a[0]='\\';a[1]=*s;a[2]=0;}else if(*s<32){swprintf(a,8,L"\\u%04x",(unsigned)*s);}else{a[0]=*s;a[1]=0;}jsonAppend(w,a);}jsonAppend(w,L"\"");}
static void publishNoticeSync(void){if(!pageReady||!view)return;JsonWriter w={0};w.cap=(size_t)(liveBuffCount+1)*2048+4096;w.text=calloc(w.cap,sizeof(wchar_t));if(!w.text)return;jsonAppend(&w,L"{\"kind\":\"world-buffs\",\"status\":");jsonString(&w,noticeStatus);jsonAppend(&w,L",\"rows\":[");for(int i=0;i<liveBuffCount;i++){NoticeBuff *b=&liveBuffs[i];wchar_t date[32],num[32];if(i)jsonAppend(&w,L",");_snwprintf(date,32,L"%04d-%02d-%02d",b->year,b->month,b->day);jsonAppend(&w,L"{\"date\":");jsonString(&w,date);jsonAppend(&w,L",\"title\":");jsonString(&w,b->title);jsonAppend(&w,L",\"attribute\":");jsonString(&w,b->attribute);jsonAppend(&w,L",\"element\":");jsonString(&w,b->element);jsonAppend(&w,L",\"type\":");jsonString(&w,b->type);jsonAppend(&w,L",\"notice\":");_snwprintf(num,32,L"%d",b->notice);jsonAppend(&w,num);jsonAppend(&w,L"}");}jsonAppend(&w,L"]}");ICoreWebView2_PostWebMessageAsJson(view,w.text);free(w.text);}

static void error(const wchar_t *msg){MessageBoxW(win,msg,L"디지몬 RPG · 제작자 선댕",MB_OK|MB_ICONERROR);}
static int exists(const wchar_t *p){return GetFileAttributesW(p)!=INVALID_FILE_ATTRIBUTES;}
static int writeResource(WORD id,const wchar_t *path){HRSRC r=FindResourceW(instance,MAKEINTRESOURCEW(id),RT_RCDATA);if(!r)return 0;DWORD len=SizeofResource(instance,r);const BYTE *p=LockResource(LoadResource(instance,r));HANDLE f=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)return 0;DWORD offset=0;while(offset<len){DWORD wrote=0,n=len-offset;if(n>1048576)n=1048576;if(!WriteFile(f,p+offset,n,&wrote,NULL)||wrote==0){CloseHandle(f);return 0;}offset+=wrote;}CloseHandle(f);return 1;}
static void startAppUpdate(int manual){
 wchar_t target[4096],script[4096],system[4096],ps[4096],pid[32],cmd[8192];
 if(!GetModuleFileNameW(NULL,target,4096))return;
 _snwprintf(script,4096,L"%ls\\auto-update.ps1",root);
 if(!writeResource(202,script))return;
 GetSystemDirectoryW(system,4096);_snwprintf(ps,4096,L"%ls\\WindowsPowerShell\\v1.0\\powershell.exe",system);
 _snwprintf(pid,32,L"%lu",GetCurrentProcessId());
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_TARGET",target);
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_SCRIPT",script);
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_VERSION",APP_VERSION);
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_OWNER",UPDATE_OWNER);
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_REPO",UPDATE_REPOSITORY);
 #ifdef FULL_EDITION
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_EDITION",L"full");
 #else
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_EDITION",L"public");
 #endif
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_PID",pid);
 SetEnvironmentVariableW(L"SEONDAENG_UPDATE_MANUAL",manual?L"1":L"0");
 _snwprintf(cmd,8192,L"\"%ls\" -NoProfile -NonInteractive -STA -WindowStyle Hidden -Command \"& ([scriptblock]::Create([IO.File]::ReadAllText($env:SEONDAENG_UPDATE_SCRIPT)))\"",ps);
 STARTUPINFOW si={0};si.cb=sizeof(si);PROCESS_INFORMATION pi={0};
 if(CreateProcessW(ps,cmd,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)){CloseHandle(pi.hThread);CloseHandle(pi.hProcess);}
 else if(manual)error(L"업데이트 확인을 시작하지 못했습니다.");
 const wchar_t *keys[]={L"TARGET",L"SCRIPT",L"VERSION",L"OWNER",L"REPO",L"EDITION",L"PID",L"MANUAL"};
 for(int i=0;i<8;i++){wchar_t key[80];_snwprintf(key,80,L"SEONDAENG_UPDATE_%ls",keys[i]);SetEnvironmentVariableW(key,NULL);}
}
static int prepareApp(void){
 wchar_t local[4096],zip[4096],marker[4096],dll[4096],system[4096],ps[4096];
 if(FAILED(SHGetFolderPathW(NULL,CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,local)))return 0;
 _snwprintf(root,4096,L"%ls\\SeondaengDigimonGuide\\%ls-%ls",local,EDITION_FOLDER,RELEASE_ID);
 _snwprintf(indexPath,4096,L"%ls\\index.html",root);_snwprintf(marker,4096,L"%ls\\ready.txt",root);_snwprintf(dll,4096,L"%ls\\WebView2Loader.dll",root);
 _snwprintf(userData,4096,L"%ls\\SeondaengDigimonGuide\\%ls",local,EDITION_PROFILE);
 if(exists(marker)&&exists(indexPath)&&exists(dll))return 1;
 if(SHCreateDirectoryExW(NULL,root,NULL)!=ERROR_SUCCESS&&!exists(root))return 0;
 _snwprintf(zip,4096,L"%ls\\payload.zip",root);
 if(!writeResource(201,zip))return 0;
 GetSystemDirectoryW(system,4096);_snwprintf(ps,4096,L"%ls\\WindowsPowerShell\\v1.0\\powershell.exe",system);
 SetEnvironmentVariableW(L"SEONDAENG_PAYLOAD",zip);SetEnvironmentVariableW(L"SEONDAENG_DEST",root);
 wchar_t cmd[8192];_snwprintf(cmd,8192,L"\"%ls\" -NoProfile -NonInteractive -Command \"$ErrorActionPreference='Stop'; Expand-Archive -LiteralPath $env:SEONDAENG_PAYLOAD -DestinationPath $env:SEONDAENG_DEST -Force\"",ps);
 STARTUPINFOW si={0};si.cb=sizeof(si);PROCESS_INFORMATION pi={0};
 if(!CreateProcessW(ps,cmd,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi))return 0;
 DWORD code=1;for(;;){DWORD status=MsgWaitForMultipleObjects(1,&pi.hProcess,FALSE,INFINITE,QS_ALLINPUT);if(status==WAIT_OBJECT_0)break;if(status!=WAIT_OBJECT_0+1)break;MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){if(m.message==WM_QUIT){CloseHandle(pi.hThread);CloseHandle(pi.hProcess);return 0;}TranslateMessage(&m);DispatchMessageW(&m);}}
 GetExitCodeProcess(pi.hProcess,&code);CloseHandle(pi.hThread);CloseHandle(pi.hProcess);SetEnvironmentVariableW(L"SEONDAENG_PAYLOAD",NULL);SetEnvironmentVariableW(L"SEONDAENG_DEST",NULL);DeleteFileW(zip);
 if(code||!exists(indexPath)||!exists(dll))return 0;
 HANDLE f=CreateFileW(marker,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);return 1;
}

static int validOverlayId(const wchar_t *id){
 const wchar_t *ids[]={L"maze-hard",L"royal-base",L"waterspace",L"dark-hard",L"xuanwumon",L"baihumon",L"zhuqiaomon",L"azulongmon",L"infinite-mountain",L"materials",L"maze-materials",L"dark-materials",L"xuanwu-materials",L"baihu-materials",L"zhuqiao-materials",L"spike-mountain",L"unforest",L"spike-materials"
#ifdef FULL_EDITION
 ,L"underground-hard",L"hellsfield",L"quiz",L"underground-materials"
#endif
 };
 for(size_t i=0;i<sizeof(ids)/sizeof(ids[0]);i++)if(!wcscmp(id,ids[i]))return 1;
 return 0;
}
static void publishOverlayStatus(void){
 if(!view||!pageReady||overlayMode)return;
 JsonWriter out={0};out.cap=4096;out.text=calloc(out.cap,sizeof(wchar_t));if(!out.text)return;
 jsonAppend(&out,L"{\"kind\":\"overlay-status\",\"open\":[");int count=0;
 for(int i=0;i<16;i++)if(overlayChildren[i].process&&WaitForSingleObject(overlayChildren[i].process,0)==WAIT_TIMEOUT){if(count++)jsonAppend(&out,L",");jsonString(&out,overlayChildren[i].id);}
 jsonAppend(&out,L"]}");ICoreWebView2_PostWebMessageAsJson(view,out.text);free(out.text);
}
static BOOL CALLBACK closeOverlayWindow(HWND h,LPARAM pid){DWORD owner=0;GetWindowThreadProcessId(h,&owner);if(owner==(DWORD)pid)PostMessageW(h,WM_CLOSE,0,0);return TRUE;}
static void closeNativeOverlay(const wchar_t *id){for(int i=0;i<16;i++)if(overlayChildren[i].process&&(!id||!wcscmp(overlayChildren[i].id,id)))EnumWindows(closeOverlayWindow,(LPARAM)overlayChildren[i].pid);}
static void openNativeOverlay(const wchar_t *id){
 if(overlayMode||!validOverlayId(id))return;
 int slot=-1;
 for(int i=0;i<16;i++){
  OverlayChild *child=&overlayChildren[i];
  if(child->process&&WaitForSingleObject(child->process,0)!=WAIT_TIMEOUT){CloseHandle(child->process);child->process=NULL;child->pid=0;}
  if(child->process&&!wcscmp(child->id,id)){publishOverlayStatus();return;}
  if(!child->process&&slot<0)slot=i;
 }
 if(slot<0)return;
 wchar_t exe[4096],cmd[8192];if(!GetModuleFileNameW(NULL,exe,4096))return;
 _snwprintf(cmd,8192,L"\"%ls\" --overlay %ls",exe,id);
 STARTUPINFOW si={0};si.cb=sizeof(si);PROCESS_INFORMATION pi={0};
 if(CreateProcessW(exe,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)){
  OverlayChild *child=&overlayChildren[slot];wcsncpy(child->id,id,63);child->id[63]=0;child->pid=pi.dwProcessId;child->process=pi.hProcess;CloseHandle(pi.hThread);publishOverlayStatus();
 }else error(L"오버레이 창을 열지 못했습니다. 프로그램 파일 위치를 확인해 주세요.");
}
static void checkOverlayChildren(void){int changed=0;for(int i=0;i<16;i++)if(overlayChildren[i].process&&WaitForSingleObject(overlayChildren[i].process,0)!=WAIT_TIMEOUT){CloseHandle(overlayChildren[i].process);overlayChildren[i].process=NULL;changed=1;}if(changed)publishOverlayStatus();}
#define BASIC(T,P) \
static HRESULT STDMETHODCALLTYPE P##QI(T *self,REFIID iid,void **out){if(!out)return E_POINTER;*out=NULL;if(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_##T)){*out=self;return S_OK;}return E_NOINTERFACE;} \
static ULONG STDMETHODCALLTYPE P##Add(T *self){(void)self;return 2;} \
static ULONG STDMETHODCALLTYPE P##Release(T *self){(void)self;return 1;}
BASIC(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,Env)
BASIC(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,Ctrl)
BASIC(ICoreWebView2NewWindowRequestedEventHandler,New)
BASIC(ICoreWebView2NavigationStartingEventHandler,Nav)
BASIC(ICoreWebView2NavigationCompletedEventHandler,Loaded)
BASIC(ICoreWebView2WebMessageReceivedEventHandler,Message)
static int isExternal(const wchar_t *uri){return !wcsncmp(uri,L"https://",8)||!wcsncmp(uri,L"http://",7);}
static HRESULT STDMETHODCALLTYPE onNew(ICoreWebView2NewWindowRequestedEventHandler *self,ICoreWebView2 *sender,ICoreWebView2NewWindowRequestedEventArgs *args){(void)self;(void)sender;LPWSTR uri=NULL;ICoreWebView2NewWindowRequestedEventArgs_put_Handled(args,TRUE);if(SUCCEEDED(ICoreWebView2NewWindowRequestedEventArgs_get_Uri(args,&uri))&&uri){if(isExternal(uri)||!wcsncmp(uri,L"file:///",8))ShellExecuteW(win,L"open",uri,NULL,NULL,SW_SHOWNORMAL);CoTaskMemFree(uri);}return S_OK;}
static HRESULT STDMETHODCALLTYPE onNav(ICoreWebView2NavigationStartingEventHandler *self,ICoreWebView2 *sender,ICoreWebView2NavigationStartingEventArgs *args){(void)self;(void)sender;LPWSTR uri=NULL;if(SUCCEEDED(ICoreWebView2NavigationStartingEventArgs_get_Uri(args,&uri))&&uri){if(isExternal(uri)){ICoreWebView2NavigationStartingEventArgs_put_Cancel(args,TRUE);ShellExecuteW(win,L"open",uri,NULL,NULL,SW_SHOWNORMAL);}CoTaskMemFree(uri);}return S_OK;}
static ICoreWebView2NewWindowRequestedEventHandlerVtbl newVT={NewQI,NewAdd,NewRelease,onNew};
static ICoreWebView2NewWindowRequestedEventHandler newHandler={&newVT};
static ICoreWebView2NavigationStartingEventHandlerVtbl navVT={NavQI,NavAdd,NavRelease,onNav};
static ICoreWebView2NavigationStartingEventHandler navHandler={&navVT};
static HRESULT STDMETHODCALLTYPE onLoaded(ICoreWebView2NavigationCompletedEventHandler *self,ICoreWebView2 *sender,ICoreWebView2NavigationCompletedEventArgs *args){(void)self;(void)sender;BOOL ok=FALSE;ICoreWebView2NavigationCompletedEventArgs_get_IsSuccess(args,&ok);if(ok){pageReady=1;if(overlayMode){SetLayeredWindowAttributes(win,0,209,LWA_ALPHA);SetWindowPos(win,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);return S_OK;}publishOverlayStatus();SetTimer(win,103,500,NULL);publishNoticeSync();if(!noticeStarted){noticeStarted=1;startNoticeSync();SetTimer(win,101,30*60*1000,NULL);startAppUpdate(0);SetTimer(win,102,60*60*1000,NULL);}}return S_OK;}
static HRESULT STDMETHODCALLTYPE onMessage(ICoreWebView2WebMessageReceivedEventHandler *self,ICoreWebView2 *sender,ICoreWebView2WebMessageReceivedEventArgs *args){(void)self;(void)sender;LPWSTR message=NULL,source=NULL;ICoreWebView2WebMessageReceivedEventArgs_get_Source(args,&source);wchar_t expected[8192];DWORD length=8192;int allowed=source&&SUCCEEDED(UrlCreateFromPathW(indexPath,expected,&length,0));if(allowed){size_t n=wcslen(expected);allowed=!wcsncmp(source,expected,n)&&(source[n]==0||source[n]=='#');}if(allowed&&SUCCEEDED(ICoreWebView2WebMessageReceivedEventArgs_TryGetWebMessageAsString(args,&message))&&message){if(!wcscmp(message,L"world-buff-sync"))startNoticeSync();else if(!wcscmp(message,L"app-update-check"))startAppUpdate(1);
else if(!wcscmp(message,L"overlay-status"))publishOverlayStatus();
else if(!wcsncmp(message,L"overlay-open:",13))openNativeOverlay(message+13);
else if(!wcsncmp(message,L"overlay-close:",14)){closeNativeOverlay(message+14);}
else if(overlayMode&&!wcscmp(message,L"overlay-close-self"))PostMessageW(win,WM_CLOSE,0,0);
else if(overlayMode&&!wcscmp(message,L"overlay-drag-start")){if(GetCursorPos(&overlayDragCursor)&&GetWindowRect(win,&overlayDragRect))overlayDragging=1;}
else if(overlayMode&&!wcscmp(message,L"overlay-drag-move")){if(overlayDragging){POINT cursor;if(GetCursorPos(&cursor))SetWindowPos(win,NULL,overlayDragRect.left+cursor.x-overlayDragCursor.x,overlayDragRect.top+cursor.y-overlayDragCursor.y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);}}
else if(overlayMode&&!wcscmp(message,L"overlay-drag-end")){overlayDragging=0;}
else if(overlayMode&&!wcsncmp(message,L"overlay-opacity:",16)){const wchar_t *v=message+16;wchar_t *end=NULL;long alpha=wcstol(v,&end,10);if(end&&*end==0&&alpha>=25&&alpha<=100)SetLayeredWindowAttributes(win,0,(BYTE)((alpha*255)/100),LWA_ALPHA);}
}if(source)CoTaskMemFree(source);if(message)CoTaskMemFree(message);return S_OK;}
static ICoreWebView2NavigationCompletedEventHandlerVtbl loadedVT={LoadedQI,LoadedAdd,LoadedRelease,onLoaded};
static ICoreWebView2NavigationCompletedEventHandler loadedHandler={&loadedVT};
static ICoreWebView2WebMessageReceivedEventHandlerVtbl messageVT={MessageQI,MessageAdd,MessageRelease,onMessage};
static ICoreWebView2WebMessageReceivedEventHandler messageHandler={&messageVT};
static HRESULT STDMETHODCALLTYPE onController(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler *self,HRESULT hr,ICoreWebView2Controller *result){(void)self;if(FAILED(hr)||!result){error(L"화면을 만들 수 없습니다. 프로그램을 다시 실행해 주세요.");return S_OK;}controller=result;ICoreWebView2Controller_AddRef(controller);ICoreWebView2Controller_get_CoreWebView2(controller,&view);RECT bounds;GetClientRect(win,&bounds);ICoreWebView2Controller_put_Bounds(controller,bounds);ICoreWebView2Controller_put_IsVisible(controller,TRUE);ICoreWebView2Settings *settings=NULL;if(SUCCEEDED(ICoreWebView2_get_Settings(view,&settings))){ICoreWebView2Settings_put_AreDevToolsEnabled(settings,FALSE);ICoreWebView2Settings_put_IsWebMessageEnabled(settings,TRUE);ICoreWebView2Settings_put_IsStatusBarEnabled(settings,FALSE);ICoreWebView2Settings_put_AreDefaultContextMenusEnabled(settings,FALSE);ICoreWebView2Settings_Release(settings);}EventRegistrationToken token;ICoreWebView2_add_NewWindowRequested(view,&newHandler,&token);ICoreWebView2_add_NavigationStarting(view,&navHandler,&token);ICoreWebView2_add_NavigationCompleted(view,&loadedHandler,&token);ICoreWebView2_add_WebMessageReceived(view,&messageHandler,&token);wchar_t url[8192];DWORD n=8192;if(SUCCEEDED(UrlCreateFromPathW(indexPath,url,&n,0))){if(overlayMode){size_t used=wcslen(url);_snwprintf(url+used,8192-used,L"#companion?view=%ls",overlayView);}ICoreWebView2_Navigate(view,url);}else error(L"프로그램 자료 경로를 열 수 없습니다.");SetWindowTextW(win,L"디지몬 RPG · 제작자 선댕 · " EDITION_LABEL);return S_OK;}
static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl ctrlVT={CtrlQI,CtrlAdd,CtrlRelease,onController};
static ICoreWebView2CreateCoreWebView2ControllerCompletedHandler ctrlHandler={&ctrlVT};
static HRESULT STDMETHODCALLTYPE onEnvironment(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *self,HRESULT hr,ICoreWebView2Environment *result){(void)self;if(FAILED(hr)||!result){error(L"Microsoft Edge WebView2 Runtime이 필요합니다.\nMicrosoft 공식 사이트에서 Evergreen Runtime을 설치한 뒤 다시 실행해 주세요.\nhttps://developer.microsoft.com/microsoft-edge/webview2/");return S_OK;}environment=result;ICoreWebView2Environment_AddRef(environment);ICoreWebView2Environment_CreateCoreWebView2Controller(environment,win,&ctrlHandler);return S_OK;}
static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl envVT={EnvQI,EnvAdd,EnvRelease,onEnvironment};
static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler envHandler={&envVT};
static LRESULT CALLBACK proc(HWND h,UINT msg,WPARAM w,LPARAM l){switch(msg){case NOTICE_DONE:finishNoticeSync((NoticeResult*)l);return 0;case WM_TIMER:if(w==101)startNoticeSync();if(w==102)startAppUpdate(0);if(w==103)checkOverlayChildren();return 0;case WM_SIZE:if(controller){RECT r;GetClientRect(h,&r);ICoreWebView2Controller_put_Bounds(controller,r);}return 0;case WM_MOVE:if(controller)ICoreWebView2Controller_NotifyParentWindowPositionChanged(controller);return 0;case WM_DPICHANGED:{RECT *r=(RECT*)l;SetWindowPos(h,NULL,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);return 0;}case WM_DESTROY:if(!overlayMode)closeNativeOverlay(NULL);KillTimer(h,103);for(int i=0;i<16;i++)if(overlayChildren[i].process){if(!overlayMode)WaitForSingleObject(overlayChildren[i].process,3000);CloseHandle(overlayChildren[i].process);overlayChildren[i].process=NULL;}pageReady=0;KillTimer(h,101);KillTimer(h,102);if(controller)ICoreWebView2Controller_Close(controller);if(view){ICoreWebView2_Release(view);view=NULL;}if(controller){ICoreWebView2Controller_Release(controller);controller=NULL;}if(environment){ICoreWebView2Environment_Release(environment);environment=NULL;}PostQuitMessage(0);return 0;case WM_PAINT:{PAINTSTRUCT p;HDC dc=BeginPaint(h,&p);RECT r;GetClientRect(h,&r);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(234,193,133));DrawTextW(dc,L"디지몬 RPG를 준비하고 있습니다…",-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);EndPaint(h,&p);return 0;}}return DefWindowProcW(h,msg,w,l);}
int WINAPI wWinMain(HINSTANCE h,HINSTANCE previous,LPWSTR args,int show){(void)previous;(void)args;int argc=0;LPWSTR *argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(argv&&argc>1&&!wcscmp(argv[1],L"--overlay")){if(argc!=3||!validOverlayId(argv[2])){LocalFree(argv);return 1;}overlayMode=1;wcsncpy(overlayView,argv[2],63);overlayView[63]=0;}if(argv)LocalFree(argv);instance=h;SetProcessDPIAware();HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;WNDCLASSW wc={0};wc.lpfnWndProc=proc;wc.hInstance=h;wc.lpszClassName=L"SeondaengPublicGuide";wc.hCursor=LoadCursorW(NULL,IDC_ARROW);wc.hIcon=LoadIconW(h,MAKEINTRESOURCEW(1));wc.hbrBackground=CreateSolidBrush(RGB(11,17,31));RegisterClassW(&wc);int w=GetSystemMetrics(SM_CXSCREEN)-60,hgt=GetSystemMetrics(SM_CYSCREEN)-90;if(w>1240)w=1240;if(hgt>920)hgt=920;if(overlayMode){w=460;hgt=680;}win=CreateWindowExW(overlayMode?(WS_EX_TOPMOST|WS_EX_LAYERED|WS_EX_TOOLWINDOW):0,wc.lpszClassName,L"디지몬 RPG · 제작자 선댕 · 준비 중",overlayMode?(WS_POPUP|WS_THICKFRAME):WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,w,hgt,NULL,NULL,h,NULL);if(overlayMode&&win)SetLayeredWindowAttributes(win,0,209,LWA_ALPHA);if(!win){CoUninitialize();return 1;}ShowWindow(win,show);UpdateWindow(win);if(!prepareApp()){error(L"프로그램 자료를 준비하지 못했습니다.\n사용자 폴더의 쓰기 권한과 Windows PowerShell 실행 가능 여부를 확인해 주세요.");DestroyWindow(win);CoUninitialize();return 1;}loadNoticeCache();wchar_t dll[4096];_snwprintf(dll,4096,L"%ls\\WebView2Loader.dll",root);HMODULE loader=LoadLibraryW(dll);if(!loader){error(L"WebView2Loader.dll을 불러올 수 없습니다.");DestroyWindow(win);CoUninitialize();return 1;}typedef HRESULT(STDAPICALLTYPE *CreateEnvironment)(PCWSTR,PCWSTR,ICoreWebView2EnvironmentOptions*,ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);CreateEnvironment create=(CreateEnvironment)GetProcAddress(loader,"CreateCoreWebView2EnvironmentWithOptions");if(!create||FAILED(create(NULL,userData,NULL,&envHandler))){error(L"Microsoft Edge WebView2 Runtime 설치가 필요합니다.\n설치 후 다시 실행해 주세요.");DestroyWindow(win);CoUninitialize();return 1;}MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}CoUninitialize();return 0;}


