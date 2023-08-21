#include <stdio.h>

#define year_days(a) (((a%100!=0)&&(a%4==0))||(a%400==0))   
#define days_in_year(i) (year_days(i) ? 366 : 365)  //判断i年是否为闰年
 
unsigned long days_in_month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//一年中各月份天数
 
struct date_time 
{
	unsigned long year;
	unsigned long month;
	unsigned long day;
	unsigned long hour;
	unsigned long min;
	unsigned long sec;
};                                        //时间结构体
 

void Time_Display(u32 ticks)              //显示时间函数，ticks表示当前时间秒数
{
	struct date_time now;                 //当前时间
	unsigned long days;                   //天数
	unsigned long hms;                    //时分秒
	int i;
	
	days = ticks / 86400;                 //当前时间秒数对应的天数
	hms = ticks % 86400;                  //当前时间秒数对应的不足一天的秒数
	
	now.hour = hms / 3600;                //转换所得小时
	now.min = (hms % 3600) / 60;          //转换所得分钟
	now.sec = (hms % 3600) % 60;          //转换所得秒
	
	for(i = 1970; days > days_in_year(i); i++)    //当天数大于这一年的天数时，年份加1
		days -= days_in_year(i);
	now.year = i;                         //转换所得年份
	
	if(year_days(i))                      //判断当前年份是否为闰年，得到二月份天数
		days_in_month[2] = 29;
	else
		days_in_month[2] = 28;
	
	for(i = 1; days > days_in_month[i]; i++)      //当天数大于这一个月的天数时，月份加1
		days -= days_in_month[i];
	now.month = i;                        //转换所得月份
	
	now.day = days + 1;                   //转换所得日，因没有0日，所以加1
	
	printf("%ld-%ld-%ld  %ld:%ld:%ld\n", now.year, now.month, now.day, now.hour, now.min, now.sec);
}
 
 
 
int main(void)
{
 
	unsigned int n;
	int i, j;
	
	return 0;
}
