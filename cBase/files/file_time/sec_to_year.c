#include <stdio.h>

/*****************************************************************************************************************************
*功能：秒与时间的互相转换 
*	秒转换成时间（SecondToTime()）：以1970年1月1日0时0分0秒为起始时刻，将增加的秒数以起始时刻为基准计算时间（Time_Type Time）
*	时间转换成秒（TimeToSecond()）：以1970年1月1日0时0分0秒为起始时刻，将设定时刻（Time_Type Time1）以起始时刻为基准计算秒差值
*****************************************************************************************************************************/

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#define DayToSecond 		86400   /* 一天等于86400秒 */
#define HourToSecond 		3600    /* 一小时等于3600秒 */
#define MinuteToSecond		60      /* 一分钟等于60秒 */
#define LeapYear			366     /* 闰年有366天 */
#define CommonYear			365     /* 平年有365天 */
#define LeapFeb             29      /* 闰年的2月 */
#define CommonFeb           28		/* 平年的2月 */

/* DayToSecond*CommonYear*2 + DayToSecond*59 + DayToSecond - 1 = 68,255,999 */
static u32 s_Second = DayToSecond*CommonYear*2 + DayToSecond*59 + DayToSecond - 1;/* 结果：1972.2.29 23：59：59（1972年是闰年）*/

typedef struct
{
	u16 year;		/* 定义时间：年 */
	u8 month;		/* 定义时间：月 */
	u8 date;		/* 定义时间：日 */
	u8 hour;		/* 定义时间：时 */
	u8 minute;		/* 定义时间：分 */
	u8 second;		/* 定义时间：秒 */
}Time_Type;

static Time_Type Time =	/* 定义一个结构体Time，表示时间，用于将秒转换成时间 */
{
		.year = 1970,
		.month = 1,
		.date = 1,
		.hour = 0,
		.minute = 0,
		.second = 0
};

static Time_Type Time1 =	/* 定义一个结构体Time1，表示时间，用于将时间转换成秒 */
{
		.year = 2023,
		.month = 8,
		.date = 21,
		.hour = 17,
		.minute = 24,
		.second = 00
};


void SecondToTime(); /* 定义将秒转换成时间函数 */
u32 TimeToSecond(); /* 定义将时间转换成秒函数 */

int main()
{
	u32 reval_TimeToSecond;
	
	/* 将秒转换成时间 */
    SecondToTime();	

	/* 将秒转换成时间的结果打印出来 */
    printf(" 年   月   日  时   分   秒\n");
    printf("%d . %d . %d  %d : %d : %d\n",Time.year,Time.month,Time.date,Time.hour,Time.minute,Time.second);
	
	/* 将时间转换成秒 */
	reval_TimeToSecond = TimeToSecond();
	
	printf("\n1972年2月29日23时59分59秒时刻距离1970年1月1日0时0分0秒时刻 %d 秒\n", reval_TimeToSecond);/* 结果正确：68,255,999 */


    return 0;
}

/* 将秒转换成时间函数 */
void SecondToTime()
{
	static u16 daycnt = 0;
	u32 temp = 0;
	u16 temp1 = 0;
	u8 mon_table[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; /* 平年的月份日期表 */

	temp = s_Second/DayToSecond;   /* 得到天数(秒数对应的) */
	if(daycnt != temp){	/* 超过一天了 */
		daycnt = temp;
		temp1 = 1970;  /* 从1970年开始 */
		while(temp >= CommonYear){
		     if(((temp1 % 4 == 0) && (temp1 % 100 != 0)) || (temp1 % 400 == 0)){ /* 是闰年 */
			     if(temp >= LeapYear)temp-=LeapYear;/* 闰年的秒钟数 */
			     else break;
		     }
		     else temp-=CommonYear;       /* 平年 */
		     temp1++;
		}
		Time.year = temp1; /* 得到年份 */
		temp1=0;
		while(temp >= CommonFeb){/* 超过了一个月 */
			if((((Time.year % 4 == 0) && (Time.year % 100 != 0)) || (Time.year % 400 == 0))&&temp1 == 1){ /* 判断当年是不是闰年且是不是2月份 */
				if(temp >= LeapFeb)temp-=LeapFeb;/* 闰年的秒钟数 */
				else break;
			}else{
	            if(temp >= mon_table[temp1])temp-=mon_table[temp1];/* 平年 */
	            else break;
			}
			temp1++;
		}
		Time.month = temp1+1;/* 得到月份 */
		Time.date = temp+1;  /* 得到日期 */
	}
	temp = s_Second%DayToSecond;     /* 得到秒数 */
	Time.hour = temp/HourToSecond;     /* 得到小时数 */
	Time.minute = (temp%HourToSecond)/MinuteToSecond; /* 得到分钟数 */
	Time.second = (temp%HourToSecond)%MinuteToSecond; /* 得到秒钟数 */
}

/* 定义将时间转换成秒函数 */
u32 TimeToSecond()
{
	u32 second_cnt;
	u32 day = 0;
	u32 hour;
	u32 minute;
	u32 second;
	u32 i;
	u8 mon_table[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; /* 平年的月份日期表 */
	

	for(i = 1970; i < Time1.year; i++) /* 从1970年到设定年份有多少闰年平年 */
	{
		if(((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))
		{
			day += 366;
		}
		else
		{
			day += 365;
		}
	}

	if(((Time1.year % 4 == 0) && (Time1.year % 100 != 0)) || (Time1.year % 400 == 0)) /* 判断设定年份是不是闰年 */
	{
		mon_table[1]++;
	}

	for(i = 0; i < Time1.month - 1; i++)
	{
	  day += mon_table[i];
	}

	day += Time1.date - 1;

	hour = day * 24 + Time1.hour;
	minute = hour * 60 + Time1.minute;
	second = minute * 60 + Time1.second;

	return second;
}

