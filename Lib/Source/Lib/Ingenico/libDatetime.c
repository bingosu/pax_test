
#include "libMtiCommonApi.h"

INT mtiGetDateTime(UCHAR *ucCurTime, INT iTimeBuffLen)
{
	Telium_Date_t tGetDate;
	INT iOffset = 0;

	if(iTimeBuffLen != 14)
		return RTN_ERROR;

	if(Telium_Read_date(&tGetDate) != 0)
		return RTN_ERROR;

	//Year - YY
	ucCurTime[iOffset++] = '2';
	ucCurTime[iOffset++] = '0';

	//Year - YY
	iOffset += mtiMemcpy(ucCurTime+iOffset, tGetDate.year, sizeof(tGetDate.year));
	//Month
	iOffset += mtiMemcpy(ucCurTime+iOffset, tGetDate.month, sizeof(tGetDate.month));
	//Day
	iOffset += mtiMemcpy(ucCurTime+iOffset, tGetDate.day, sizeof(tGetDate.day));
	//Hour
	iOffset += mtiMemcpy(ucCurTime+iOffset, tGetDate.hour, sizeof(tGetDate.hour));
	//Minute
	iOffset += mtiMemcpy(ucCurTime+iOffset, tGetDate.minute, sizeof(tGetDate.minute));
	//Second
	iOffset += mtiMemcpy(ucCurTime+iOffset, tGetDate.second, sizeof(tGetDate.second));
	/*
	dpt();
	dbuf(0, ucCurTime, 14);
	*/
	return RTN_SUCCESS;
}

typedef struct
{
	UINT year;
	UCHAR month;
	UCHAR day;
	UCHAR hours;
	UCHAR minutes;
	UCHAR seconds;
} DateTime;

ULLONG convertDateToUnixTime(DateTime *date)
{
	UINT y;
	UINT m;
	UINT d;
	ULLONG t;

	//Year
	y = date->year;
	//Month of year
	m = date->month;
	//Day of month
	d = date->day;

	//January and February are counted as months 13 and 14 of the previous year
	if (m <= 2)
	{
		m += 12;
		y -= 1;
	}

	//Convert years to days
	t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
	//Convert months to days
	t += (30 * m) + (3 * (m + 1) / 5) + d;
	//Unix time starts on January 1st, 1970
	t -= 719561;
	//Convert days to seconds
	t *= 86400;
	//Add hours, minutes and seconds
	t += (3600 * date->hours) + (60 * date->minutes) + date->seconds;

	//Return Unix time
	return t;
}

void convertUnixTimeToDate(ULLONG t, DateTime *date)
{
	UINT a;
	UINT b;
	UINT c;
	UINT d;
	UINT e;
	UINT f;

	//Negative Unix time values are not supported
	if (t < 1)
		t = 0;

	//Clear milliseconds
	//date->milliseconds = 0;

	//Retrieve hours, minutes and seconds
	date->seconds = t % 60;
	t /= 60;
	date->minutes = t % 60;
	t /= 60;
	date->hours = t % 24;
	t /= 24;

	//Convert Unix time to date
	a = (UINT)((4 * t + 102032) / 146097 + 15);
	b = (UINT)(t + 2442113 + a - (a / 4));
	c = (20 * b - 2442) / 7305;
	d = b - 365 * c - (c / 4);
	e = d * 1000 / 30601;
	f = d - e * 30 - e * 601 / 1000;

	//January and February are counted as months 13 and 14 of the previous year
	if (e <= 13)
	{
		c -= 4716;
		e -= 1;
	}
	else
	{
		c -= 4715;
		e -= 13;
	}

	//Retrieve year, month and day
	date->year = c;
	date->month = e;
	date->day = f;

	//Calculate day of week
	//date->dayOfWeek = computeDayOfWeek(c, e, f);
}


INT mtiSetDateTimeForInni(UCHAR *ucSetTime, INT iTimeBuffLen, INT iHour)
{
	//YYYYMMDDhhmmss
	Telium_Date_t tSetDate, tConvSetDate;
	DateTime tCurrDate, tNewDate;
	INT iOffset = 0;
	ULLONG llConvTime;
	CHAR chTemp[16] = { '\0', };

	if(iTimeBuffLen != 14)
		return RTN_ERROR;

	//Year
	iOffset += 2;	//Skip YY
	iOffset += mtiMemcpy(tSetDate.year, ucSetTime + iOffset, 2);
	//Month
	iOffset += mtiMemcpy(tSetDate.month, ucSetTime + iOffset, 2);
	//Day
	iOffset += mtiMemcpy(tSetDate.day, ucSetTime + iOffset, 2);
	//Hour
	iOffset += mtiMemcpy(tSetDate.hour, ucSetTime + iOffset, 2);
	//Minute
	iOffset += mtiMemcpy(tSetDate.minute, ucSetTime + iOffset, 2);
	//Second
	iOffset += mtiMemcpy(tSetDate.second, ucSetTime + iOffset, 2);

	if(Telium_Ctrl_date(&tSetDate) != 0)
		return RTN_ERROR;

	if (0 != iHour)
	{
		tCurrDate.year = mtiAtoi(2, tSetDate.year) + 2000;
		tCurrDate.month = mtiAtoi(2, tSetDate.month);
		tCurrDate.day = mtiAtoi(2, tSetDate.day);
		tCurrDate.hours = mtiAtoi(2, tSetDate.hour);
		tCurrDate.minutes = mtiAtoi(2, tSetDate.minute);
		tCurrDate.seconds = mtiAtoi(2, tSetDate.second);

		llConvTime = convertDateToUnixTime(&tCurrDate);

		llConvTime += (iHour * 60 * 60);

		convertUnixTimeToDate(llConvTime, &tNewDate);

		sprintf(chTemp, "%02d%02d%02d%02d%02d%02d", 
			tNewDate.day, tNewDate.month, tNewDate.year - 2000, tNewDate.hours, tNewDate.minutes, tNewDate.seconds);

		dmsg("mtiSetDateTimeForInni : %s", chTemp);

		mtiMemcpy(&tConvSetDate, chTemp, sizeof(Telium_Date_t));

		// @@EB FIX YEAR - CHANGE AGAIN TO TERMINAL YEAR
		memcpy(tConvSetDate.year, tSetDate.year, 2);
		//

		if (Telium_Ctrl_date(&tConvSetDate) != 0)
			return RTN_ERROR;

		mtiMemcpy(&tSetDate, &tConvSetDate, sizeof(Telium_Date_t));
	}

	if (Telium_Write_date(&tSetDate) != 0)
		return RTN_ERROR;

	/*
	dpt();
	dbuf(0, ucSetTime, 14);
	*/

	return RTN_SUCCESS;
}

INT mtiSetDateTime(UCHAR *ucSetTime, INT iTimeBuffLen)
{
	return mtiSetDateTimeForInni(ucSetTime, iTimeBuffLen, 0);
}

