#ifndef DATE_NAV_H
#define DATE_NAV_H
typedef struct {int year,month,day;} CalendarDate;
static int monthLength(int year,int month){
 static const int lengths[]={31,28,31,30,31,30,31,31,30,31,30,31};
 return lengths[month-1]+(month==2 && year%4==0 && (year%100!=0 || year%400==0));
}
static int shiftCalendarDate(CalendarDate *d,int direction){
 if(direction>0){
  if(d->year==9999 && d->month==12 && d->day==31)return 0;
  if(++d->day>monthLength(d->year,d->month)){d->day=1;if(++d->month>12){d->month=1;d->year++;}}
 }else if(direction<0){
  if(d->year==1601 && d->month==1 && d->day==1)return 0;
  if(--d->day<1){if(--d->month<1){d->month=12;d->year--;}d->day=monthLength(d->year,d->month);}
 }return 1;
}
static CalendarDate gameDateAt(CalendarDate date,int hour){if(hour<6)shiftCalendarDate(&date,-1);return date;}
#endif
