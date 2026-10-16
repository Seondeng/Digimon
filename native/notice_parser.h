#ifndef NOTICE_PARSER_H
#define NOTICE_PARSER_H
#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>
#include "date_nav.h"
#define MAX_NOTICE_ROWS 32
#define MAX_CACHED_ROWS 1024
typedef struct {int year,month,day,notice;wchar_t attribute[16],element[16],type[48],title[100];} NoticeBuff;
static const wchar_t *findIgnoreCase(const wchar_t *text,const wchar_t *needle){
 size_t n=wcslen(needle);for(;*text;text++){size_t i=0;while(i<n&&text[i]&&towlower(text[i])==towlower(needle[i]))i++;if(i==n)return text;}return NULL;
}
static wchar_t *compactHtml(const wchar_t *html){
 size_t len=wcslen(html),o=0;wchar_t *out=calloc(len+1,sizeof(wchar_t));if(!out)return NULL;
 for(size_t i=0;i<len;){
  if(html[i]=='<'){
   if(!wcsncmp(html+i,L"<script",7)||!wcsncmp(html+i,L"<style",6)){const wchar_t *end=findIgnoreCase(html+i,html[i+2]=='c'?L"</script":L"</style");if(!end)break;i=(size_t)(end-html);}
   while(i<len&&html[i]!='>')i++;if(i<len)i++;continue;
  }
  wchar_t ch=html[i++];
  if(ch=='&'){
   size_t end=i;while(end<len&&end-i<15&&html[end]!=';')end++;
   if(end<len&&html[end]==';'){
    wchar_t ent[20]={0};wmemcpy(ent,html+i,end-i);int parsed=1;
    if(!wcscmp(ent,L"nbsp")||!wcscmp(ent,L"ensp")||!wcscmp(ent,L"emsp"))ch=' ';
    else if(!wcscmp(ent,L"amp"))ch='&';else if(!wcscmp(ent,L"quot"))ch='"';else if(!wcscmp(ent,L"apos"))ch='\'';
    else if(ent[0]=='#'){wchar_t *e;unsigned long n=wcstoul(ent+(ent[1]=='x'||ent[1]=='X'?2:1),&e,ent[1]=='x'||ent[1]=='X'?16:10);if(*e||n==0||n>65535)parsed=0;else ch=(wchar_t)n;}
    else parsed=0;if(parsed)i=end+1;
   }
  }
  if(!iswspace(ch)&&ch!=0xa0&&ch!=0x200b&&ch!=0xfeff)out[o++]=ch;
 }out[o]=0;return out;
}
static int parseNoticeIds(const wchar_t *html,int *ids,int cap){
 int count=0;const wchar_t *p=html;
 while((p=findIgnoreCase(p,L"Update_view?idx="))!=NULL){p+=16;long v=wcstol(p,NULL,10);if(v<=0||v>100000000)continue;int dup=0;for(int i=0;i<count;i++)if(ids[i]==v)dup=1;if(!dup&&count<cap)ids[count++]=(int)v;}
 for(int i=0;i<count;i++)for(int j=i+1;j<count;j++)if(ids[j]>ids[i]){int t=ids[i];ids[i]=ids[j];ids[j]=t;}return count;
}
static int rowDate(const wchar_t *p,int *m,int *d,const wchar_t **after){
 if(*p<'0'||*p>'9')return 0;wchar_t *q;long month=wcstol(p,&q,10);if(month<1||month>12||*q!=L'월')return 0;
 long day=wcstol(q+1,&q,10);if(day<1||day>31||*q!=L'일'||(q[1]!=L':'&&q[1]!=L'：'))return 0;
 *m=(int)month;*d=(int)day;*after=q+2;return 1;
}
static int copySpan(wchar_t *out,int cap,const wchar_t *a,const wchar_t *b){if(b<=a||b-a>=cap)return 0;wmemcpy(out,a,b-a);out[b-a]=0;return 1;}
static int readEffect(const wchar_t *start,const wchar_t *end,const wchar_t *percent,wchar_t *out,int cap){
 const wchar_t *p=wcsstr(start,percent);if(!p||p>=end)return 0;return copySpan(out,cap,start,p);
}
static int parseNotice(const wchar_t *html,int notice,NoticeBuff *out,int cap){
 wchar_t *text=compactHtml(html);if(!text)return -1;const wchar_t *begin=wcsstr(text,L"월드버프변경");if(!begin){free(text);return 0;}
 const wchar_t *end=wcsstr(begin,L"오전6시");if(!end||end-begin>40000){free(text);return -1;}
 int year=0,startMonth=0;for(const wchar_t *p=begin;p<end;p++)if(*p>='0'&&*p<='9'){wchar_t *q;long y=wcstol(p,&q,10);if(y>=2000&&y<=9999&&*q==L'년'){year=(int)y;long m=wcstol(q+1,&q,10);if(m>=1&&m<=12&&*q==L'월')startMonth=(int)m;break;}}
 if(!year||!startMonth){free(text);return -1;}int count=0,previousMonth=startMonth;
 for(const wchar_t *p=begin;p<end;p++){
  int m,d;const wchar_t *body;if(!rowDate(p,&m,&d,&body))continue;
  if(previousMonth==12&&m==1)year++;else if(m<previousMonth){free(text);return -1;}
  if(d>monthLength(year,m)||count>=cap){free(text);return -1;}previousMonth=m;
  const wchar_t *rowEnd=end;for(const wchar_t *q=body;q<end;q++){int mm,dd;const wchar_t *unused;if(rowDate(q,&mm,&dd,&unused)){rowEnd=q;break;}}
  const wchar_t *s1=wcschr(body,'/'),*s2=s1?wcschr(s1+1,'/'):NULL,*s3=s2?wcschr(s2+1,'/'):NULL;
  if(!s1||!s2||!s3||s3>=rowEnd){free(text);return -1;}
  NoticeBuff b={0};b.year=year;b.month=m;b.day=d;b.notice=notice;
  if(!copySpan(b.title,100,body,s1)||!readEffect(s1+1,s2,L"10%",b.attribute,16)||!readEffect(s2+1,s3,L"20%",b.element,16)||!readEffect(s3+1,rowEnd,L"40%",b.type,48)){free(text);return -1;}
  if(wcscmp(b.attribute,L"백신")&&wcscmp(b.attribute,L"데이터")&&wcscmp(b.attribute,L"바이러스")&&wcscmp(b.attribute,L"불명")){free(text);return -1;}
  size_t n=wcslen(b.element);if(n>=2&&!wcscmp(b.element+n-2,L"속성"))b.element[n-2]=0;
  if(!b.element[0]||!b.type[0]){free(text);return -1;}
  if(count){NoticeBuff *last=&out[count-1];CalendarDate next={last->year,last->month,last->day};shiftCalendarDate(&next,1);if(next.year!=year||next.month!=m||next.day!=d){free(text);return -1;}}
  out[count++]=b;p=rowEnd-1;
 }
 free(text);return count?count:-1;
}
#endif
