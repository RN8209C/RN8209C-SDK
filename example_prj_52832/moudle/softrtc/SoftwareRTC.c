#include "bsp.h"
#include "SoftwareRTC.h"
#include "stdint.h"

RTC_UTCTimeStruct time;
/*********************************************************************
 * Get month length
 *********************************************************************/
unsigned char monthLength( unsigned char lpyr, unsigned char mon )
{
    unsigned char days = 31;

    if ( mon == 1 ) // feb 2
    {
        days = ( 28 + lpyr );
    } else {
        if ( mon > 6 ) // aug-dec 8-12
        {
            mon--;
        }

        if ( mon & 1 ) {
            days = 30;
        }
    }

    return ( days );
}
//0---6对应周1到周日
uint8_t GetWeek(uint16_t year,uint8_t month,uint8_t day)
{
    const uint8_t table_week[12]= {0,3,3,6,1,4,6,2,5,0,3,5};
    uint16_t temp2;
    uint8_t yearH,yearL;
    uint8_t week;
    yearH=year/100;
    yearL=year%100;
    month=month%13;
    // 如果为21世纪,年份数加100

    if (yearH>19)yearL+=100;

    // 所过闰年数只算1900年以后的

    temp2=yearL+yearL/4;

    temp2=temp2%7;

    temp2=temp2+day+table_week[month-1];

    if (yearL%4==0&&month<3)temp2--;

    if((temp2%7) == 0)
    {
        temp2 = 7;
        week=temp2;
    }
    else
    {
        week=(temp2%7);
    }

    week--;
    return week;
}
/**************************************************************************
* Calculte UTCTime
***************************************************************************/
void ConvertToUTCTime( RTC_UTCTimeStruct *tm, UTCTime secTime )
{
    if((secTime<SEC_2017)||(secTime>SEC_2099))
    {
        secTime=SEC_2017;
    }

    // calculate the time less than a day - hours, minutes, seconds
    {
        unsigned int day = secTime % DAY;
        tm->seconds = day % 60UL;
        tm->minutes = (day % 3600UL) / 60UL;
        tm->hour = day / 3600UL;
    }

    // Fill in the calendar - day, month, year
    {
        unsigned short numDays = secTime / DAY;
        tm->year = BEGYEAR;
        while ( numDays >= YearLength( tm->year ) )
        {
            numDays -= YearLength( tm->year );
            tm->year++;
        }

        tm->month = 0;
        while ( numDays >= monthLength( IsLeapYear( tm->year ), tm->month ))
        {
            numDays -= monthLength( IsLeapYear( tm->year ), tm->month );
            tm->month++;
        }

        tm->day = numDays;
    }

    tm->day++;
    tm->month++;

    //0---6对应周1到周日
    tm->week=GetWeek(tm->year,tm->month,tm->day);

}

UTCTime convert_time_to_Second(RTC_UTCTimeStruct time_in)													/*将RTC转化为秒*/
{
    unsigned int i = 0;
    UTCTime offset = 0;

    time_in.year-=2000;

    //day time
    offset += time_in.seconds;
    offset += time_in.minutes * 60;
    offset += time_in.hour * 60 * 60;

    unsigned  int leapYear = IsLeapYear(time_in.year);

    offset += DAY * (time_in.day - 1);

    for(i = 0; i < time_in.month - 1; ++i)
    {   //month start from 1
        offset += monthLength(leapYear,i) * DAY;
    }

    for(i = 0; i< time_in.year ; ++i)
    {
        if(IsLeapYear(i))
        {
            offset += (DAY * 366);
        } else
        {
            offset += (DAY * 365);
        }
    }

    return offset;

}





