/*
 * libSound.c
 *
 *  Created on: 2016. 8. 12.
 *      Author: kjyoo
 */

#include "libMtiCommonApi.h"

VOID mtiSoundBeep(INT iDuration, INT iCnt)
{
#if 0
	INT iLoop = 0;
	for(;iLoop < iCnt; iLoop++)
	{
		Beep(0x07, 0x05, iDuration/10, BEEP_ON);
		mtiSleep(200);
		Beep(0x07, 0x05, iDuration/10, BEEP_OFF);
	}
	Beep(0x07, 0x05, iDuration/10, BEEP_OFF);
#endif	
}

VOID mtiSoundDoremifa()
{
	CHAR cNote = 0x00; 	//C Note
	INT iLoop = 0;

	for(;iLoop < 4; iLoop++)
	{
		Beep(cNote, 0x05, 8, BEEP_ON);
		mtiSleep(100);
		Beep(cNote, 0x05, 8, BEEP_OFF);
		cNote += 2;
	}
}

VOID mtiSoundWarning(INT iDuration, INT iCnt)
{
	INT iLoop = 0;
	for(;iLoop < iCnt; iLoop++)
	{
		Beep(0x01, 0x01, iDuration/10, BEEP_ON);
		mtiSleep(200);
		Beep(0x01, 0x01, iDuration/10, BEEP_OFF);
	}
	Beep(0x01, 0x05, iDuration/10, BEEP_OFF);
}
