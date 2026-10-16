#ifndef NOTICE_SYNC_H
#define NOTICE_SYNC_H
#include <winhttp.h>
#include <shlobj.h>
#include "notice_parser.h"
#define NOTICE_DONE (WM_APP+2)
typedef struct {NoticeBuff rows[256];int count,failed,latest,parsed;DWORD error;} NoticeResult;
static NoticeBuff liveBuffs[MAX_CACHED_ROWS];
static int liveBuffCount=0;
static LONG fetchingNotice=0;
static ULONGLONG lastNoticeAttempt=0;
static wchar_t noticeStatus[240]=L"공지 확인 대기 중";
static wchar_t cachePath[MAX_PATH];
static int noticeRevision=0;
static int cachedRowValid(const NoticeBuff *b){
 return b->year>=2000&&b->year<=9999&&b->month>=1&&b->month<=12&&b->day>=1&&b->day<=monthLength(b->year,b->month)&&b->notice>0&&b->notice<=100000000&&
 wmemchr(b->attribute,0,16)&&wmemchr(b->element,0,16)&&wmemchr(b->type,0,48)&&wmemchr(b->title,0,100)&&b->attribute[0]&&b->element[0]&&b->type[0];
}
static void loadNoticeCache(void){
 wchar_t dir[MAX_PATH];if(FAILED(SHGetFolderPathW(NULL,CSIDL_LOCAL_APPDATA,NULL,SHGFP_TYPE_CURRENT,dir)))return;
 if(wcslen(dir)>MAX_PATH-60)return;wcscat(dir,L"\\SeondaengDigimonGuide");CreateDirectoryW(dir,NULL);swprintf(cachePath,MAX_PATH,L"%ls\\world-buffs-v1.bin",dir);
 FILE *f=_wfopen(cachePath,L"rb");if(!f)return;unsigned header[3];
 if(fread(header,sizeof(header),1,f)==1&&header[0]==0x53444246&&header[1]==1&&header[2]<=MAX_CACHED_ROWS){
  NoticeBuff *rows=calloc(header[2]+1,sizeof(NoticeBuff));if(rows){int good=fread(rows,sizeof(NoticeBuff),header[2],f)==header[2];for(unsigned i=0;i<header[2]&&good;i++)if(!cachedRowValid(&rows[i]))good=0;
   if(good){memcpy(liveBuffs,rows,header[2]*sizeof(NoticeBuff));liveBuffCount=(int)header[2];swprintf(noticeStatus,240,L"저장된 공식 일정 %d일 불러옴 · 새 공지 확인 예정",liveBuffCount);}free(rows);}
 }fclose(f);
}
static int saveNoticeCache(void){
 if(!cachePath[0])return 0;wchar_t temp[MAX_PATH];swprintf(temp,MAX_PATH,L"%ls.tmp",cachePath);FILE *f=_wfopen(temp,L"wb");if(!f)return 0;unsigned h[]={0x53444246,1,(unsigned)liveBuffCount};
 int ok=fwrite(h,sizeof(h),1,f)==1&&fwrite(liveBuffs,sizeof(NoticeBuff),liveBuffCount,f)==(size_t)liveBuffCount;if(fclose(f)!=0)ok=0;
 if(ok)ok=MoveFileExW(temp,cachePath,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;else DeleteFileW(temp);return ok;
}
static wchar_t *fetchOfficial(HINTERNET session,const wchar_t *path,DWORD *error){
 HINTERNET connection=WinHttpConnect(session,L"www.digimonrpg.com",INTERNET_DEFAULT_HTTPS_PORT,0);if(!connection){*error=GetLastError();return NULL;}
 HINTERNET request=WinHttpOpenRequest(connection,L"GET",path,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);wchar_t *wide=NULL;char *bytes=NULL;
 if(!request){*error=GetLastError();goto done;}
 if(!WinHttpSendRequest(request,L"Cache-Control: no-cache\r\n",(DWORD)-1L,WINHTTP_NO_REQUEST_DATA,0,0,0)||!WinHttpReceiveResponse(request,NULL)){*error=GetLastError();goto done;}
 DWORD status=0,size=sizeof(status);if(!WinHttpQueryHeaders(request,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&size,WINHTTP_NO_HEADER_INDEX)||status!=200){*error=status?status:GetLastError();goto done;}
 size_t used=0,cap=65536;bytes=malloc(cap+1);if(!bytes){*error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
 for(;;){DWORD amount=0;if(!WinHttpQueryDataAvailable(request,&amount)){*error=GetLastError();goto done;}if(!amount)break;
  if(used+amount>2*1024*1024){*error=ERROR_FILE_TOO_LARGE;goto done;}
  if(used+amount>cap){cap=used+amount+65536;char *b=realloc(bytes,cap+1);if(!b){*error=ERROR_NOT_ENOUGH_MEMORY;goto done;}bytes=b;}
  DWORD got=0;if(!WinHttpReadData(request,bytes+used,amount,&got)){*error=GetLastError();goto done;}if(!got)break;used+=got;
 }
 if(!used){*error=ERROR_INVALID_DATA;goto done;}bytes[used]=0;
 int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes,(int)used,NULL,0);if(!n){*error=ERROR_NO_UNICODE_TRANSLATION;goto done;}
 wide=calloc((size_t)n+1,sizeof(wchar_t));if(wide)MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes,(int)used,wide,n);else *error=ERROR_NOT_ENOUGH_MEMORY;
 done:free(bytes);if(request)WinHttpCloseHandle(request);WinHttpCloseHandle(connection);return wide;
}
static DWORD WINAPI noticeWorker(LPVOID unused){
 (void)unused;NoticeResult *result=calloc(1,sizeof(NoticeResult));if(!result){PostMessageW(win,NOTICE_DONE,0,0);return 0;}
 HINTERNET session=WinHttpOpen(L"SeondaengDigimonGuide/1.1",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
 if(!session){result->error=GetLastError();goto done;}WinHttpSetTimeouts(session,8000,8000,10000,10000);
 wchar_t *html=fetchOfficial(session,L"/Pages/Notice/Update",&result->error);if(!html)goto close;
 int ids[40];int count=parseNoticeIds(html,ids,40);free(html);if(!count){result->error=ERROR_INVALID_DATA;goto close;}
 if(count>8)count=8;result->latest=ids[0];
 for(int i=0;i<count;i++){
  wchar_t path[120];swprintf(path,120,L"/pages/Notice/Update_view?idx=%d",ids[i]);html=fetchOfficial(session,path,&result->error);
  if(!html){result->failed++;continue;}NoticeBuff rows[MAX_NOTICE_ROWS];int n=parseNotice(html,ids[i],rows,MAX_NOTICE_ROWS);free(html);
  if(n<0){result->failed++;continue;}if(n){result->parsed++;for(int j=0;j<n&&result->count<256;j++)result->rows[result->count++]=rows[j];}
 }
 close:WinHttpCloseHandle(session);
 done:if(!PostMessageW(win,NOTICE_DONE,0,(LPARAM)result))free(result);return 0;
}
static void startNoticeSync(void){
 if(InterlockedCompareExchange(&fetchingNotice,1,0))return;lastNoticeAttempt=GetTickCount64();wcscpy(noticeStatus,L"공식 업데이트 공지 확인 중…");
 HANDLE thread=CreateThread(NULL,0,noticeWorker,NULL,0,NULL);if(thread)CloseHandle(thread);else{InterlockedExchange(&fetchingNotice,0);wcscpy(noticeStatus,L"공지 확인 시작 실패 · 기존 정보 유지");}publishNoticeSync();
}
static void finishNoticeSync(NoticeResult *r){
 InterlockedExchange(&fetchingNotice,0);clockNow();
 if(!r||!r->count){swprintf(noticeStatus,240,L"%02d/%02d %02d:%02d 확인 실패 · 기존 정보 유지 (오류 %lu)",nowKst.wMonth,nowKst.wDay,nowKst.wHour,nowKst.wMinute,r?r->error:ERROR_NOT_ENOUGH_MEMORY);free(r);publishNoticeSync();return;}
 for(int i=0;i<r->count;i++){
  NoticeBuff b=r->rows[i];int found=-1;for(int j=0;j<liveBuffCount;j++)if(liveBuffs[j].year==b.year&&liveBuffs[j].month==b.month&&liveBuffs[j].day==b.day){found=j;break;}
  if(found>=0){if(b.notice>=liveBuffs[found].notice)liveBuffs[found]=b;}
  else if(liveBuffCount<MAX_CACHED_ROWS)liveBuffs[liveBuffCount++]=b;
  else{int oldest=0;for(int j=1;j<liveBuffCount;j++)if(liveBuffs[j].notice<liveBuffs[oldest].notice)oldest=j;if(b.notice>=liveBuffs[oldest].notice)liveBuffs[oldest]=b;}
 }
 int saved=saveNoticeCache();noticeRevision++;
 swprintf(noticeStatus,240,L"%02d/%02d %02d:%02d · %ls · 공지 %d개, 일정 %d일 반영%ls",nowKst.wMonth,nowKst.wDay,nowKst.wHour,nowKst.wMinute,r->failed?L"일부 공지 확인 실패":L"업데이트 완료",r->parsed,r->count,saved?L"":L" · 저장 실패");
 free(r);publishNoticeSync();
}
#endif
