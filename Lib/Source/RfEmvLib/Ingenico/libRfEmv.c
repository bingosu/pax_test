/*
 * libRfEmv.c
 *
 *  Created on: 2016. 9. 28.
 *      Author: JoonoHany
 */

#include "libMtiEmvApi.h"
#include "apMtiCommonApp.h"
#include "APCLESS.h"

INT g_iIsClessOpen = FALSE;
INT g_iIsStartCless = FALSE;
tEmvParamInfo g_tRfEmvParamInfo;

INT mtiInitializeCtls()
{
	INT iRet = RTN_ERROR;

	if (g_iIsClessOpen)
	{
		return iRet;
	}

	iRet = TPass_GetDeviceType();
	if (iRet == TPASS_CLESS_TYPE_NONE || iRet == TPASS_CLESS_TYPE_UNKNOWN)
	{
		dmsg("TPass_GetDeviceType is failed. [%d]", iRet);
		return RTN_ERROR;
	}

	iRet = ClessEmv_OpenDriver();
	if (iRet == CL_OK)
	{
		g_iIsClessOpen = TRUE;
		g_iIsStartCless = FALSE;
		iRet = RTN_SUCCESS;
	}
	else
	{
		dmsg("ClessEmv_OpenDriver is failed.");
		iRet = RTN_ERROR;
	}

	return iRet;
}

INT mtiCtlsDetectCard()
{
	INT iRet = RTN_ERROR;

	if (g_iIsClessOpen)
	{
		if (!g_iIsStartCless)
		{
			if (ClessEmv_DetectCardsStart(1, CL_TYPE_AB) == CL_OK)
				g_iIsStartCless = TRUE;
			else
				iRet = RTN_CANCEL;
		}
	}

	if (g_iIsStartCless)
	{
		iRet = (CLESS == Telium_Ttestall(CLESS, 1));
	}
	else
	{
		iRet = RTN_CANCEL;
	}

	return iRet;
}

INT mtiCtlsGetCardUid(UCHAR *ucpCardUid, INT *iCardUidLen)
{
	INT iRet = 0;
	unsigned char ucaUID[12] = {0,};

	iRet = ClessEmv_GetUid(ucaUID);
	if(iRet != CL_OK)
	{
		dmsg("ClessEmv_GetUid iRet[%d]", iRet);
		*iCardUidLen = 0;
		iRet = RTN_ERROR;
	}
	else
	{
		//VERIFONE & VIVOTEC CARD UID Response Emulation for compatibility.

		INT iIdx = sizeof(ucaUID) - 1;
		INT iOffset = 0;

			//GET LENGTH
		for(; iIdx >= 0 ; iIdx--)
			if(ucaUID[iIdx] != 0x00)
				break;
		iIdx++;

		ucpCardUid[iOffset++] = CL_TYPE_AB;
		ucpCardUid[iOffset++] = iIdx;
		memcpy(ucpCardUid + iOffset, ucaUID, iIdx);
		iOffset += iIdx;
		*iCardUidLen = iOffset;
		dbuf("CTLS CARD UID", ucpCardUid, *iCardUidLen);
		iRet = RTN_SUCCESS;
	}

	return iRet;
}

INT mtiCtlsSendApdu(UCHAR *ucpApduCmd, INT iApduCmdLen, UCHAR *ucpResData, INT *iResDatalen)
{
	INT iRet = 0;
	INT iErrType = 0;
	UINT uiRespLen = 258; //@@WAY, 20190502 PREPAID
	
	dbuf("ClessEmv_ApduEx Send Data", ucpApduCmd, iApduCmdLen);
#if 1 // @@WAY, 20190502 PREPAID
	iRet = ClessEmv_ApduEx(0, ucpApduCmd, (UINT) iApduCmdLen, ucpResData, &uiRespLen, &iErrType);
#else
	iRet = ClessEmv_ApduEx(0, ucpApduCmd, (UINT)iApduCmdLen, ucpResData, (UINT *)iResDatalen, &iErrType);
#endif

	if(iRet != CL_OK)
	{
		dmsg("ClessEmv_ApduEx iRet[%d] iErrType[%d]", iRet, iErrType);
		iRet = RTN_ERROR;
	}
	else
	{
		*iResDatalen = (INT) uiRespLen; //@@WAY, 20190502 PREPAID
		dbuf("ClessEmv_ApduEx Recv Data", ucpResData, *iResDatalen);
		iRet = RTN_SUCCESS;
	}
	return iRet;
}

INT mtiFinalizeCtls()
{
	if (g_iIsClessOpen)
	{
		if (g_iIsStartCless)
		{
			g_iIsStartCless = FALSE;
			ClessEmv_DetectCardsStop();
		}

		g_iIsClessOpen = FALSE;
		ClessEmv_CloseDriver();
	}

	return RTN_SUCCESS;
}

INT mtiCtlsLEDCtrl(UCHAR ucLed, UCHAR ucOnOff)
{
	unsigned char ucSetLed = 0x00;
	INT iIdx = 0;
	struct _tLedTable
	{
		unsigned char ucLedBit;
		unsigned char ucLedVal;
	};

	struct _tLedTable tLEDTable[4] = { {CTLS_LED_1, TPASS_LED_1}, {CTLS_LED_2, TPASS_LED_2}, \
										{CTLS_LED_3, TPASS_LED_3}, {CTLS_LED_4, TPASS_LED_4},};

	while(ucLed > 0)
	{
		if(CHK_BIT(ucLed, tLEDTable[iIdx].ucLedBit))
		{
			ucSetLed = tLEDTable[iIdx].ucLedVal;
			ucLed &= ~ tLEDTable[iIdx].ucLedBit;

			if(ucOnOff == CTLS_LED_ON)
				TPass_LedsOn(ucSetLed);
			else
				TPass_LedsOff(ucSetLed);
		}
		iIdx++;
	}

	return RTN_SUCCESS;
}

tEmvParamInfo *mtiRfEmvInitialize()
{
	mtiMemset(&g_tRfEmvParamInfo, 0, sizeof(tEmvParamInfo));
	return &g_tRfEmvParamInfo;
}

tEmvParamInfo *mtiGetRfEmvParmInfo()
{
	return &g_tRfEmvParamInfo;
}

INT mtiRfEmvSession(tEmvParamInfo *tpEmvParamInfo)
{
	INT iRet = 0, iLen = 0;
	UCHAR ucaTemp[128 + 1];
	UCHAR *ucpTemp = NULL;
	tMtiMap *tpMainMap = tpEmvParamInfo->tpMainProcMap;
	tMtiMap *tpTagMap = &tpEmvParamInfo->tEmvTagMap;

#ifdef __DEBUG_ON__
	APCLESS_ParamTrn_SetDebugTrace(TRUE);
#else
	APCLESS_ParamTrn_SetDebugTrace(FALSE);
#endif

	iRet = APCLESS_Explicit_DoTransaction(tpMainMap);
	dmsg("**** APCLESS_Explicit_DoTransaction() Result = %d", iRet);
	if (iRet == MTI_EMV_TR_SUCCESS)
	{
		ucpTemp = mtiMapGetBytes(tpTagMap, MTI_EMV_TAG_CRYPTOGRAM_INFO_DATA, &iLen);
		dbuf("MTI_EMV_TAG_CRYPTOGRAM_INFO_DATA", ucpTemp, iLen);
		if (iLen > 0)
		{
			if ((ucpTemp[0] & 0xF0) > 0)
			{
				BEEP_SUCCESS();

				iLen = 0;
				ucpTemp = mtiMapGetBytes(tpTagMap, MTI_EMV_TAG_TRACK_2_EQU_DATA, &iLen);
				if (iLen > 0)
				{
					mtiMemset(ucaTemp, 0, sizeof(ucaTemp));
					mtiHtoa(ucaTemp, ucpTemp, iLen);

					mtiMapPutInt(tpMainMap, KEY_EXT_BIT_PEM, PEM_CTLS_NO_PIN);				//DEFAULT CTLS - 'NO PIN ENTRY'
					mtiMapPutString(tpMainMap, KEY_EXT_BIT_TRACK2DATA, (CHAR*)ucaTemp);		//TRACK2 DATA INSERT
				}

				mtiMapPutInt(tpMainMap, KEY_EXT_BIT_CARDREAD_FLAG, TRUE);
				iRet = MTI_EMV_TR_SUCCESS;
			}
			else
			{
				iRet = MTI_EMV_TR_DECLINED;
			}
		}
                //@@WAY JCB Cless
		else
		{
			iRet = MTI_EMV_TR_DECLINED;
		}
                //
	}

	return iRet;
}

INT mtiRfEmvSetParam(BOOL bDoClear)
{
	INT iRet = 0;

	return iRet;
}

VOID mtiRfEmvFinalize(tEmvParamInfo *tpEmvParamInfo)
{
	mtiMemset(&g_tRfEmvParamInfo, 0, sizeof(tEmvParamInfo));
}
