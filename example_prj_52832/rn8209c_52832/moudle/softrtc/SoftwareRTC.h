//SoftwareRTC
#ifndef __SOFTWARTE_RTC_H
#define __SOFTWARTE_RTC_H

#define DAY             86400UL  // 24 hours * 60 minutes * 60 seconds
#define BEGYEAR         2000     // UTC started at 00:00:00 January 1, 2000
#define IsLeapYear(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define YearLength(yr) (IsLeapYear(yr) ? 366 : 365)





// To be used with
typedef struct
{
		unsigned short year;    // 2000+
		unsigned char month;    // 1-12
		unsigned char day;      // 1-31
		unsigned char hour;     // 0-23
		unsigned char minutes;  // 0-59
		unsigned char seconds;  // 0-59
    unsigned char week;  // 0-6  对应周一到周日
    
}RTC_UTCTimeStruct;

typedef unsigned int UTCTime; /* used to store the second counts for RTC */

extern RTC_UTCTimeStruct time;
void ConvertToUTCTime(RTC_UTCTimeStruct *tm, UTCTime secTime );								/*将秒转化成RTC*/
UTCTime convert_time_to_Second(RTC_UTCTimeStruct time);													/*将RTC转化为秒*/


#endif 



