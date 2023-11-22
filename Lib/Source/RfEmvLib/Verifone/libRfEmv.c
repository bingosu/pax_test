
#include "libEmvCTLS.h"
#include "apMem.h"

#define XML_FILE_CTLS_TERM	"EMV_CTLS_TERMINAL.XML"
#define XML_FILE_CTLS_APP	"EMV_CTLS_APPLICATIONS.XML"
#define XML_FILE_CTLS_KEYS	"EMV_CTLS_KEYS.XML"

tEmvParamInfo g_tRfEmvParamInfo;

unsigned char ATR[256] = {0,};
unsigned long LenATR = 0L;

static VOID xmlFileDbg(CHAR *cpFileName)
{
	FILE *file;
	INT iOffset = 0, iReadSize = 0;
	CHAR *readBuff = NULL;
	CHAR cReadData;
	struct stat fileInfo;
	INT iIdx = 0, iLineIdx = 0;
	CHAR caLineBuff[200] = {0,};

	memset(&fileInfo, 0x00, sizeof(fileInfo));

	if(cpFileName == NULL)
	{
		dmsg("cpFilePath is NULL!");
		return;
	}

	stat(cpFileName, &fileInfo);

	if(fileInfo.st_size < 1)
	{
		dmsg("%s size is zero or not exist!!!", cpFileName);
		return;
	}

	readBuff = (CHAR *)mtiMalloc(fileInfo.st_size);
	memset(readBuff, 0x00, fileInfo.st_size);

	file = fopen(cpFileName, "r");
	while(1)
	{
		iReadSize = fread(readBuff+(iOffset++), 1, 1, file);
		if(iReadSize < 1)
			break;
	}

	dmsg("FILE[%s] Contents...", cpFileName);
	memset(caLineBuff, 0x00, sizeof(caLineBuff));
	iLineIdx = 0;
	for(iIdx = 0; iIdx < iOffset; iIdx++)
	{
		if(readBuff[iIdx] != '\n' && iLineIdx < sizeof(caLineBuff) - 1)
			caLineBuff[iLineIdx++] = readBuff[iIdx];
		else
		{
			dmsg("%s",caLineBuff);
			memset(caLineBuff, 0x00, sizeof(caLineBuff));
			iLineIdx = 0;
		}
	}
	dmsg("------------------------------");
	free(readBuff);
	fclose(file);
}

//-------------------------------------------------------------------------------------------

// Send XML Configuration to Reader
//-------------------------------------------------------------------------------------------
static EMV_ADK_INFO XMLConfToReader(void)
{
   EMV_ADK_INFO erg;

   erg = EMV_CTLS_ApplyConfiguration(CTLS_APPLY_CFG_ALL | CTLS_APPLY_CFG_FORCE);
   if(erg != EMV_ADK_OK)
   {
      LOGF_INFO("EMV_CTLS_ApplyConfiguration returned %d.", erg);
	  // note that at this point, app may want to exit framework
   }
   return erg;
}

void v_EndTransaction(const char *szStatusMessage, int inExitWithError)
{
//	if (inExitWithError)
//		sysBeepError(BEEPVOLUME);
	dmsg("[v_EndTransaction] start : %s",szStatusMessage);

	EMV_CTLS_SmartPowerOff(0);
	return;
}

//@@battery_c680
INT mtiRfEmvPreparing(BOOL doCTLS)
{
	if (mtiGetHwType() == C680)
	{
		if (v_InitFramework() == EMV_ADK_OK)
		{
			dpt();
			if (doCTLS)
			{
				if(v_SetCTLSTerminalData() != EMV_ADK_OK)
					return RTN_ERROR;

				if(v_SetCTLSApplicationData() != EMV_ADK_OK)
					return RTN_ERROR;
			}
			return RTN_SUCCESS;
		}
	}

	return RTN_ERROR;
}
//@@battery_c680

#if 0 // @@WAY PAYWAVE C680 20191101
tEmvParamInfo *mtiRfEmvInitialize()
{
	mtiMemset(&g_tRfEmvParamInfo, 0, sizeof(tEmvParamInfo));

	v_InitFramework();

	return &g_tRfEmvParamInfo;
}
#else
tEmvParamInfo *mtiRfEmvInitialize()
{
	mtiMemset(&g_tRfEmvParamInfo, 0, sizeof(tEmvParamInfo));
	if (mtiGetHwType() != C680)
	{
		dpt();
		v_InitFramework();
	}
	else
	{
		dpt();
	}

	return &g_tRfEmvParamInfo;
}
#endif

tEmvParamInfo *mtiGetRfEmvParmInfo()
{
	return &g_tRfEmvParamInfo;
}

INT mtiRfEmvSetParam(BOOL bDoClear)
{
	if(bDoClear!= FALSE)
	{
		remove(XML_FILE_CTLS_TERM);
		remove(XML_FILE_CTLS_APP);
		remove(XML_FILE_CTLS_KEYS);
		SVC_WAIT(100);

		dmsg("EMV CTLS XML Files all Removed.");

		if(TRUE==bDoClear)
			return RTN_SUCCESS;
	}

	//@@battery_c680
	if(v_SetCTLSTerminalData() != EMV_ADK_OK)
		return RTN_ERROR;
	//@@battery_c680

	//@@battery_c680
	if(v_SetCTLSApplicationData() != EMV_ADK_OK)
		return RTN_ERROR;
	//@@battery_c680

	//@@battery_c680
	dmsg("===== XMLConfToReader before ...........................");
	//@@battery_c680

	XMLConfToReader();

	//@@battery_c680
	dmsg("===== XMLConfToReader after ...........................");
	//@@battery_c680

	//XML DEBUG
	dmsg("EMV CTLS TERMINAL CONFIGUARATION");
	xmlFileDbg("I:1/EMV_CTLS_Terminal.xml");

	dmsg("EMV CTLS APP CONFIGUARATION");
	xmlFileDbg("I:1/EMV_CTLS_Applications.xml");

	dmsg("EMV CTLS APP KEYS");
	xmlFileDbg("I:1/EMV_CTLS_KEYS.xml");

	return RTN_SUCCESS;
}


INT mtiRfEmvSession(tEmvParamInfo *tpRfEmvParamInfo)
{
	INT iRet = MTI_EMV_TR_SUCCESS;

	EMV_ADK_INFO erg;

	//mtiRfEmvSetParam();



	dmsg("mtiRfEmvSession start");

#if 1
//	for(;;)
	{

		dmsg("Start TR : %lu",mtiGetTickCount());

		erg =  RfEMVTransaction(10);

		dmsg("RfEMVTransaction RET[%d]", erg);

		dmsg("End TR : %lu",mtiGetTickCount());

		if ((erg==EMV_ADK_OK)||(erg==EMV_ADK_TC))
		{
			dmsg("\n\n----------EMV Transaction ended successfully!!!");
		}
		else
		{
			dmsg("\n\n----------EMV Transaction ended abnormally!!!");
		}
	}
#else
	for(;;)
	{
		memset(info, 0x00, sizeof(info));
		rval = EMV_CTLS_SmartReset(0x01, info, &len);
		dmsg("rval:%02x  info:%02x%02x%02x%02x  len:%d", rval, info[0],info[1],info[2],info[3],len);

	   if(mtiGetKeyPress()==KEY_CANCEL)
        	return EMV_ADK_ABORT;

		mtiSleep(30);
	}
#endif
	switch(erg)
	{
	/*
		case EMV_ADK_TXN_CTLS_EMV_USE_OTHER_CARD:
			{
				dmsg("ALAN - RETRY CARD");
				iRet = MTI_EMV_TR_CARD_PROCESSING_ERROR;
			}
			break;
*/
		case EMV_ADK_OK:
			iRet = MTI_EMV_ONLINE_APPROVED;
			break;
		case EMV_ADK_TC:
			iRet = MTI_EMV_TR_SUCCESS;
			break;
		case EMV_ADK_2_CTLS_CARDS:
			iRet = MTI_EMV_TR_2CARD_TAP_ERR;
			break;
		case EMV_ADK_FALLBACK:
		case EMV_ADK_TXN_CTLS_EMV_USE_OTHER_CARD:
			iRet = MTI_EMV_TR_TRY_IC_TRAN;
			break;
		//@@WAY PARAM PAYPASS 20210127
		case EMV_ADK_TXN_CTLS_EMPTY_LIST: 
			iRet = MTI_EMV_TR_TRY_INSERT_CARD;
			break;
		//
		case EMV_ADK_ABORT:
			iRet = MTI_EMV_TR_CANCEL;
			break;
		default:
			iRet = MTI_EMV_TR_UNKNOWN_ERROR;
	}


	return iRet;
}

VOID mtiRfEmvFinalize(tEmvParamInfo *tpRfEmvParamInfo)
{
	//mtiMemset(tpRfEmvParamInfo, 0, sizeof(tpRfEmvParamInfo));

	dmsg("[*] EMV_CTLS_Exit_Framework start : %lu",mtiGetTickCount());

	if (mtiGetHwType() == C680)
	{
		dpt();
		EMV_CTLS_Exit_Framework();
		EMV_CTLS_SmartPowerOff(0);
	}
	dmsg("[*] EMV_CTLS_Exit_Framework end : %lu",mtiGetTickCount());

	return;
}

INT mtiInitializeCtls()
{
	EMV_CTLS_LED_SetMode(CONTACTLESS_LED_MODE_API);
	EMV_CTLS_LED(CONTACTLESS_LED_ALL, CONTACTLESS_LED_OFF);
	EMV_CTLS_LED(CONTACTLESS_LED_0, CONTACTLESS_LED_ON);

	return RTN_SUCCESS;
}

INT mtiFinalizeCtls()
{
	EMV_CTLS_SmartPowerOff(0);
	EMV_CTLS_LED_SetMode(CONTACTLESS_LED_MODE_OFF);
	EMV_CTLS_LED(CONTACTLESS_LED_ALL, CONTACTLESS_LED_OFF);
	return RTN_SUCCESS;
}

INT mtiCtlsDetectCard()
{
	unsigned char erg = NULL;

	memset(ATR, 0x00, sizeof(ATR));
	LenATR = sizeof(ATR);

	erg = EMV_CTLS_SmartReset(EMV_CTLS_RFU_1, ATR, &LenATR);
	if(erg == EMV_ADK_SMART_STATUS_OK)
	{
		dbuf("CTLS ATR", ATR, LenATR);
		return TRUE;
	}
	else
		return FALSE;
}

INT mtiCtlsGetCardUid(UCHAR *ucpCardUid, INT *iCardUidLen)
{
	INT iRet;
	if(LenATR > 0)
	{
		memcpy(ucpCardUid, ATR, LenATR);
		*iCardUidLen = (INT) LenATR;
		iRet = RTN_SUCCESS;
	}
	else
		iRet = RTN_ERROR;

	return iRet;
}

INT mtiCtlsSendApdu(UCHAR *ucpApduCmd, INT iApduCmdLen, UCHAR *ucpResData, INT *iResDatalen)
{
	unsigned char ucRet;
	unsigned short usApduCmdLen = (unsigned short) iApduCmdLen;
	unsigned short usResDataLen = 0;

	if( iApduCmdLen < 1 || ucpApduCmd == NULL)
		return RTN_ERROR;

	dbuf("EMV_CTLS_SmartISO SEND DATA", ucpApduCmd, iApduCmdLen);

	ucRet = EMV_CTLS_SmartISO(0, (unsigned short) iApduCmdLen,ucpApduCmd, &usResDataLen, ucpResData, (unsigned short)512);

	if(ucRet != EMV_ADK_SMART_STATUS_OK || usResDataLen < 1)
		return RTN_ERROR;

	dbuf("EMV_CTLS_SmartISO RES DATA", ucpResData, usResDataLen);

	*iResDatalen = (INT) usResDataLen;

	return RTN_SUCCESS;
}

INT mtiCtlsLEDCtrl(UCHAR ucLed, UCHAR ucOnOff)
{
	unsigned char ucSetLed = 0x00;
	unsigned char ucLedOnoff = 0x00;
	INT iIdx = 0;
	struct _tLedTable
	{
		unsigned char ucLedBit;
		unsigned char ucLedVal;
	};

	struct _tLedTable tLEDTable[4] = { {CTLS_LED_1, CONTACTLESS_LED_0}, {CTLS_LED_2, CONTACTLESS_LED_1},
										{CTLS_LED_3, CONTACTLESS_LED_2}, {CTLS_LED_4, CONTACTLESS_LED_3},};
	while(ucLed > 0)
	{
		if(CHK_BIT(ucLed, tLEDTable[iIdx].ucLedBit))
		{
			ucSetLed = tLEDTable[iIdx].ucLedVal;
			ucLed &= ~ tLEDTable[iIdx].ucLedBit;

			if(ucOnOff == CTLS_LED_ON)
				ucLedOnoff = CONTACTLESS_LED_ON;
			else
				ucLedOnoff = CONTACTLESS_LED_OFF;

			EMV_CTLS_LED(ucSetLed, ucOnOff);
		}

		iIdx++;

	}
	return RTN_SUCCESS;
}

// @@WAY PAYWAVE C680 20200320
INT mtiRfEmvParamConfToReader(void)
{
	EMV_ADK_INFO erg;
	INT iRet = 0;
	
	erg = XMLConfToReader();
	if(erg == EMV_ADK_OK)
	{
		dmsg("mtiRfEmvParamConfToReader success [%d]", erg);
		iRet = RTN_SUCCESS;
	}
	else
	{
		dmsg("mtiRfEmvParamConfToReader error [%d]", erg);
		iRet = RTN_ERROR;
	}
	return iRet;
}
// @@WAY PAYWAVE C680 20200320
