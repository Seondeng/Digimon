/* Retry WebView startup with a fresh writable profile; never erase saved data. */
#define WEBVIEW_RETRY_MESSAGE (WM_APP+76)
typedef HRESULT(STDAPICALLTYPE *WebViewFactory)(PCWSTR,PCWSTR,ICoreWebView2EnvironmentOptions*,ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
static WebViewFactory webViewFactory;
static int webViewAttempt=0,webViewRetryPending=0,webViewFatalFailure=0;
static wchar_t originalWebViewProfile[4096];
static void beginWebViewStartup(void);
static void logWebViewStartup(HRESULT hr,const wchar_t *stage){
 wchar_t path[4096];_snwprintf(path,4096,L"%ls\\webview-startup.log",root);
 FILE *file=NULL;if(_wfopen_s(&file,path,L"a, ccs=UTF-8")||!file)return;
 fwprintf(file,L"attempt=%d stage=%ls hr=0x%08lX\n",webViewAttempt,stage,(unsigned long)hr);fclose(file);
}
static int writableWebViewProfile(const wchar_t *path){
 int made=SHCreateDirectoryExW(NULL,path,NULL);if(made!=ERROR_SUCCESS&&made!=ERROR_ALREADY_EXISTS&&made!=ERROR_FILE_EXISTS)return 0;
 DWORD attrs=GetFileAttributesW(path);if(attrs==INVALID_FILE_ATTRIBUTES||!(attrs&FILE_ATTRIBUTE_DIRECTORY))return 0;
 wchar_t probe[4096];_snwprintf(probe,4096,L"%ls\\startup-probe-%lu-%llu.tmp",path,GetCurrentProcessId(),GetTickCount64());
 HANDLE h=CreateFileW(probe,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,NULL);
 if(h==INVALID_HANDLE_VALUE)return 0;CloseHandle(h);return 1;
}
static int chooseRecoveryProfile(int attempt){
 wchar_t base[4096];
 if(attempt==1){wcsncpy_s(base,4096,originalWebViewProfile,_TRUNCATE);if(!PathRemoveFileSpecW(base))return 0;}
 else{DWORD n=GetTempPathW(4096,base);if(!n||n>=4096)return 0;}
 _snwprintf(userData,4096,L"%ls\\Seondaeng-WebView2-%ls-Recovery-%lu-%llu-%d",base,EDITION_FOLDER,GetCurrentProcessId(),GetTickCount64(),attempt);
 return writableWebViewProfile(userData);
}
static void failedWebViewStartup(HRESULT hr,const wchar_t *stage){
 if(!IsWindow(win)||webViewRetryPending)return;
 logWebViewStartup(hr,stage);
 while(webViewAttempt<2){
  webViewAttempt++;
  if(chooseRecoveryProfile(webViewAttempt)){
   webViewRetryPending=1;
   if(PostMessageW(win,WEBVIEW_RETRY_MESSAGE,0,0))return;
   webViewRetryPending=0;break;
  }
 }
 wchar_t message[4600];_snwprintf(message,4600,L"화면 초기화를 다시 시도했지만 열지 못했습니다.\n오류 코드: 0x%08lX (%ls)\n\nPC방 관리자에게 Microsoft Edge WebView2 Runtime 설치·복구와 프로그램 실행 허용 여부를 확인해 주세요.\n\n진단 기록: %ls\\webview-startup.log",(unsigned long)hr,stage,root);
 webViewFatalFailure=1;error(message);DestroyWindow(win);
}
static void beginWebViewStartup(void){
 if(!IsWindow(win))return;
 webViewRetryPending=0;
 if(controller){ICoreWebView2Controller_Close(controller);}
 if(view){ICoreWebView2_Release(view);view=NULL;}
 if(controller){ICoreWebView2Controller_Release(controller);controller=NULL;}
 if(environment){ICoreWebView2Environment_Release(environment);environment=NULL;}
 if(!originalWebViewProfile[0])wcsncpy_s(originalWebViewProfile,4096,userData,_TRUNCATE);
 if(!writableWebViewProfile(userData)){failedWebViewStartup(E_ACCESSDENIED,L"Profile");return;}
 if(!webViewFactory){failedWebViewStartup(HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND),L"Loader");return;}
 HRESULT hr=webViewFactory(NULL,userData,NULL,&envHandler);
 if(FAILED(hr))failedWebViewStartup(hr,L"Environment request");
}
