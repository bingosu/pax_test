/*
 * libSound.cpp
 *
 *  Created on: 2016. 8. 12.
 *      Author: kjyoo
 */

#include "libMtiCommonApi.h"

#include <sysinfo/sysinfo.h>
#include <sysinfo/sysbeep.h>

using namespace vfisysinfo;

VOID mtiSoundBeep(INT iDuration, INT iCnt)
{
	INT iLoop = 0;
	for(;iLoop < iCnt; iLoop++)
	{
		sysBeepNormal(100);
		SVC_WAIT(300);
	}
}

VOID mtiSoundDoremifa()
{
	INT iaDoremifa[4] = { 39, 41, 43, 44}; //C, D, E, F
	INT iNoteIdx = 0;

	for(;iNoteIdx < 4; iNoteIdx++)
	{
		sysPlaySound(iaDoremifa[iNoteIdx], 800, 100);
		SVC_WAIT(200);
	}
}

VOID mtiSoundWarning(INT iDuration, INT iCnt)
{
	INT iLoop = 0;
	for(;iLoop < iCnt; iLoop++)
	{
		sysBeepError(100);
		SVC_WAIT(300);

	}
}
