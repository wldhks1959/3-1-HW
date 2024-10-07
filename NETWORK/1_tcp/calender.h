#include <stdio.h>

int is_leap_year(int year) // 윤년 계산
{
	if(year % 400 == 0) return 1;
	if((year % 100 != 0) && (year % 4 == 0)) return 1;
	return 0;
}

int get_day_of_month(int year,  int month)
{
    int day_of_month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    day_of_month[2] += is_leap_year(year); // 윤년이면 2월 : 29일
    return day_of_month[month];
}

// 요일 
int get_day(int year, int month)
{
	int past = 0, day;
	for(int y=1; y<year; y++) 
        past = past + 365 + is_leap_year(y);
	for(int m=1; m<month; m++) 
        past = past + get_day_of_month(year, m);
	day = (1 + past) % 7; // 요일을 반환
    return day;
}

void print_cal(int s_day, int day_num)
{
    printf(" Sun Mon Tue Wed Thu Fri Sat\n");
    for(int i = 0; i < s_day; i++) printf("    ");
    for(int day = 1, ke = s_day; day <= day_num; day++, ke++)
    {
        printf("%4d",day);
        if(ke%7 == 6) printf("\n");
    }
}

void execute(int year, int month, char *outputBuffer)
{
    int s_day = get_day(year, month);
    int day_num = get_day_of_month(year, month);
    print_cal(s_day, day_num);
   
    sprintf(outputBuffer, " Sun Mon Tue Wed Thu Fri Sat\n");
    int offset = strlen(outputBuffer);
    for(int i = 0; i < s_day; i++) offset += sprintf(outputBuffer + offset, "    ");
    for(int day = 1, ke = s_day; day <= day_num; day++, ke++)
    {
        offset += sprintf(outputBuffer + offset, "%4d", day);
        if(ke % 7 == 6) offset += sprintf(outputBuffer + offset, "\n");
    }
}