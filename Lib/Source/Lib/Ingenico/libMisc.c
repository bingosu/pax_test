
#include "libMtiCommonApi.h"

#if 0
#include <Cless_LowLevel.h>
#endif

VOID mtiGetRamStatus(UINT *uiMaxBytes, UINT *uiFreeBytes)
{
	INT iSizeFlash, iSizeRam, iRamFree, iFlashFreeCode, iFlashFreeData, iSizeBlocksKO;

	iSizeFlash = iSizeRam = iRamFree = iFlashFreeCode = iFlashFreeData = iSizeBlocksKO = 0;
	if (TM_GetInfoMemorySize(&iSizeFlash, &iSizeRam, &iRamFree, &iFlashFreeCode, &iFlashFreeData, &iSizeBlocksKO) == OK)
	{
		dmsg("iSizeFlash = %d", iSizeFlash);
		dmsg("iSizeRam = %d", iSizeRam);
		dmsg("iRamFree = %d", iRamFree);
		dmsg("iFlashFreeCode = %d", iFlashFreeCode);
		dmsg("iFlashFreeData = %d", iFlashFreeData);
		dmsg("iSizeBlocksKO = %d", iSizeBlocksKO);

		*uiMaxBytes = (UINT)iSizeRam;
		*uiFreeBytes = (UINT)iRamFree;
	}
	else
	{
		dmsg("TM_GetInfoMemorySize() ERROR!!")
	}
}

INT mtiGetTerminalSerial(CHAR *cpOut)
{
	INT iRet = 0;

#ifdef __DEBUG_ON__
	static BOOL bForceSet = FALSE;

	//CHAR *pszForceSerial = "16049CT25126509";

	if(0==memcmp(cpOut, "rjtlrl", 6))
	{
		if(FALSE==bForceSet)
		{
			bForceSet = TRUE;
		}
		else
		{
			bForceSet = FALSE;
		}
		return RTN_SUCCESS;
	}
	
	if(TRUE==bForceSet)
	{
		//strcpy(cpOut, pszForceSerial);
		return RTN_SUCCESS;
	}

	iRet = PSQ_Give_Full_Serial_Number((unsigned char*)cpOut);
	if (!iRet)
	{
		return RTN_SUCCESS;
	}
	else
	{
		return RTN_ERROR;
	}

#else

	iRet = PSQ_Give_Full_Serial_Number((unsigned char*)cpOut);
	if (!iRet)
	{
		return RTN_SUCCESS;
	}
	else
	{
		return RTN_ERROR;
	}
#endif
}

CHAR *monthList[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec"};

INT mtiGetTerminalVersion(CHAR *cpOut)
{
	UCHAR ucProduct;
	CHAR *cpVer = cpOut;
	UCHAR ucRange = 0;

	cpVer += mtiMemcpy(cpVer, "IG", 2);

	ucRange = PSQ_Get_product_type(&ucProduct);
	//dmsg("** ucProduct = %d", ucProduct);
	//dmsg("** ucRange = %d", ucRange);
	switch(ucRange)
	{
		case TYPE_TERMINAL_IWL220:
			cpVer += mtiMemcpy(cpVer, "02", 2);
			break;

		case TYPE_TERMINAL_ICT220:
			cpVer += mtiMemcpy(cpVer, "03", 2);
			break;

		case TYPE_TERMINAL_ICT250:
			cpVer += mtiMemcpy(cpVer, "04", 2);
			break;

		// added by @@WAY, 18111
		case TYPE_TERMINAL_M2500:
			cpVer += mtiMemcpy(cpVer, "06", 2);
			break;
		case TYPE_TERMINAL_M3500:
			cpVer += mtiMemcpy(cpVer, "07", 2);
			break;
		// added by @@WAY, 18111

		default:
			return RTN_ERROR;
	}

	{
		INT iIdx, iMonth = 1;
		CHAR caDATE[20] = {0,};
		CHAR caMonth[10] = {0,};
		CHAR caDay[10] = {0,};
		CHAR caYear[10] = {0,};

		sprintf(caDATE, "%s", __DATE__);

		sscanf(caDATE, "%s %s %s", caMonth, caDay, caYear);

		for(iIdx = 0; iIdx < 12; iIdx++)
			if(!mtiMemcmp(monthList[iIdx], caMonth, 3))
				iMonth = iIdx + 1;

		iMonth = 8;	// @@EB MARK
		cpVer += sprintf(cpVer, "%s%02d%s", "21", iMonth, REL_NUM);
	}

	//cpVer += mtiMemcpy(cpVer, APP_VER, 5);
	return RTN_SUCCESS;
}

INT mtiGetHwSupport()
{
	INT iRet = 0;

	iRet = PSQ_Get_Cless_Capabilities();

	dmsg("PSQ_Get_Cless_Capabilities() Result = %X", iRet);
	if (CHK_BIT(iRet, INTERNAL_CLESS) || CHK_BIT(iRet, TELIUM_PASS_CLESS))
	{
		return HW_SUPPORT_CTLS;
	}

	return 0;
}

INT mtiSetHwConfig(INT iType, INT iValue)
{
#ifndef __TETRA__

	INT iRet = 0;

	if (iType == MISC_DEVICE_CTLS)
	{
		if (iValue == MISC_DEVICE_TRUN_ON)
		{
			iRet = PSQ_update_ClessReader(CLESS_ON, INTERNAL_TYPE);
		}
		else
		{
			iRet = PSQ_update_ClessReader(CLESS_OFF, INTERNAL_TYPE);
		}
	}
#endif
	return RTN_SUCCESS;
}

INT mtiGetHwConfig(INT iType)
{
	INT iRet = 0, iTurn = 0;

	iRet = PSQ_Get_Cless_Capabilities();

	dmsg("PSQ_Get_Cless_Capabilities() Result = %X", iRet);
	if (CHK_BIT(iRet, CLESS_ACTIVATED))
	{
		iTurn |= MISC_DEVICE_TRUN_ON;
	}

	return iTurn;
}

static INT g_iBacklightInten = 0;

INT mtiSysSleep(INT iOnOff)
{
	INT iTemp = HWCNF_GetBacklightIntensity();

	if (iTemp != 0xFFFF)
	{
		g_iBacklightInten = iTemp;
	}

	if (iOnOff == TRUE)
		HWCNF_SetBacklightIntensity(0xFFFF);
	else
		HWCNF_SetBacklightIntensity(g_iBacklightInten);

	return RTN_SUCCESS;
}

VOID mtiSysReboot()
{
	Telium_Systemfioctl(SYS_FIOCTL_SYSTEM_RESTART, NULL);
}

VOID mtiDebugStart(CHAR *pszIP, INT nPort)
{
	debugStart(nPort);
}

VOID mitDebugStop()
{
	debugStop();
}
