#include <time.h>
#include "libMtiCommonApi.h"

#include <sys/time.h>
#include <sysinfo/sysinfo.h>

using namespace vfisysinfo;

INT mtiGetDateTime(UCHAR *ucCurTime, INT iTimeBuffLen)
{
	INT iRet = 0;
	CHAR caCurTime[14+1] = {0,};

	if(ucCurTime == NULL)
	{
		dmsg("ucCurTime is NULL.");
		return RTN_ERROR;
	}

	if(iTimeBuffLen < 14)
	{
		dmsg("iTimeBuffLen < 14");
		return RTN_ERROR;
	}

	iRet = sysGetPropertyString(SYS_PROP_RTC, caCurTime, sizeof(caCurTime));
	if(iRet != SYS_ERR_OK)
	{
		dmsg("iRet[%d]", iRet);
		return RTN_ERROR;
	}

	//dmsg("caCurTime:[%s]", caCurTime);


	mtiStrcpy(ucCurTime, caCurTime, 14);

	return RTN_SUCCESS;
}

INT mtiSetDateTimeForInni(UCHAR *ucSetTime, INT iTimeBuffLen, INT iHour)
{
	INT ihClockDev = 0;
	INT iRet = 0;
	INT iOffset = 0;
	CHAR caCurTime[14+1] = {0,};
	time_t t;
	struct tm *t_new;


	//YYYYMMDDhhmmss
	if(iTimeBuffLen < 14)
	{
		dmsg("iTimeBuffLen < 14");
		return RTN_ERROR;
	}

	if(mtiStrlen(14, ucSetTime) < 14)
	{
		dmsg("ucSetTime Len < 14");
		return RTN_ERROR;
	}

	dmsg("SET TIME by [%s], HOURE [%d]", ucSetTime, iHour);

	iRet = sysSetPropertyString(SYS_PROP_RTC, (const CHAR *)ucSetTime);

	if (iRet != SYS_ERR_OK)
	{
		dmsg("iRet[%d]", iRet);
		return RTN_ERROR;
	}

	if (0!=iHour)
	{
		time(&t);
		
		t_new = gmtime(&t);

		t += (iHour * 60 * 60);
		t_new = gmtime(&t);

		sprintf(caCurTime, "%04d%02d%02d%02d%02d%02d",
			t_new->tm_year + 1900, t_new->tm_mon + 1, t_new->tm_mday, t_new->tm_hour, t_new->tm_min, t_new->tm_sec);

		dmsg("CONV DATE : %s", caCurTime);

		// @@EB FIX YEAR - CHANGE AGAIN TO TERMINAL YEAR
		memcpy(caCurTime, ucSetTime, 4);
		//

		iRet = sysSetPropertyString(SYS_PROP_RTC, (const CHAR *)caCurTime);

		if (iRet != SYS_ERR_OK)
		{
			dmsg("iRet[%d]", iRet);
			return RTN_ERROR;
		}
	}

	return RTN_SUCCESS;
}

INT mtiSetDateTime(UCHAR *ucSetTime, INT iTimeBuffLen)
{
	return mtiSetDateTimeForInni(ucSetTime, iTimeBuffLen, 0);
}
