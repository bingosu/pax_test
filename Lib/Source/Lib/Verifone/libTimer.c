
#include "libMtiCommonApi.h"

#define MAX_TIMER_COUNT						32

static ULONG g_mtllTimer[MAX_TIMER_COUNT];
static ULONG g_mulTimeout[MAX_TIMER_COUNT];
static INT g_ucInitTimerVar = FALSE;

INT mtiStartTimer(ULONG ulTimeout)
{
	INT i = 0;

	if (!g_ucInitTimerVar)
	{
		g_ucInitTimerVar = TRUE;

		mtiMemset(g_mtllTimer, 0, sizeof(g_mtllTimer));
		mtiMemset(g_mulTimeout, 0, sizeof(g_mulTimeout));
	}

	for (i = 0; i < MAX_TIMER_COUNT; i++)
	{
		if (g_mulTimeout[i] == 0UL)
		{
			g_mulTimeout[i] = ulTimeout;
			mtiResetTimer(i + 1);
			return i + 1;
		}
	}

	return -1;
}

VOID mtiResetTimer(INT iTimerID)
{
	if (iTimerID > 0)
	{
		g_mtllTimer[iTimerID - 1] = read_ticks();
		g_mtllTimer[iTimerID - 1] += (ULONG) g_mulTimeout[iTimerID - 1];
	}
}

INT mtiIsTimeout(INT iTimerID)
{
	ULONG llTimerEnd, llTimer;

	if (iTimerID > 0)
	{
		llTimerEnd = g_mtllTimer[iTimerID - 1];
		llTimer = read_ticks();

		if(llTimer < llTimerEnd)
			return FALSE;
		else
		{
			mtiStopTimer(iTimerID);
			return TRUE;
		}
	}
	else
	{
		return TRUE;
	}
}

VOID mtiStopTimer(INT iTimerID)
{
	if (iTimerID > 0)
	{
			g_mtllTimer[iTimerID - 1] = 0LL;
			g_mulTimeout[iTimerID - 1] = 0UL;
	}
}

VOID mtiSleep(ULONG ulTimeout)
{
	SVC_WAIT((unsigned int)ulTimeout);
}

ULONG mtiGetTickCount()
{
	return (ULONG)read_ticks();
}
