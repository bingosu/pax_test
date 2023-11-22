
#include "libMtiEmvApi.h"

#define IC_CTRL_MAX_HALDER_COUNT			5

tEmvParamInfo g_tEmvParamInfo;
Telium_File_t* g_taTfCam[IC_CTRL_MAX_HALDER_COUNT] = { NULL };
UCHAR g_ucaPwrFlg[IC_CTRL_MAX_HALDER_COUNT] = { FALSE };

INT mtiInitializeICController(INT iCtrlType)
{
	INT i = 0;
	UCHAR ucFlag = FALSE;
	CHAR *cpFile = NULL;

	for (i = 0; i < IC_CTRL_MAX_HALDER_COUNT; i++)
	{
		if (g_taTfCam[i] == NULL)
		{
			ucFlag  = TRUE;
			break;
		}
	}

	if (ucFlag)
	{
		switch (iCtrlType)
		{
			case SLOT_MAIN_ICC:
				cpFile = "CAM0";
				break;

			case SLOT_SAM1_ICC:
				cpFile = "SAM1";
				break;

			case SLOT_SAM2_ICC:
				cpFile = "SAM2";
				break;
			//@@WAY BNI PHASE II 20200917
			case SLOT_SAM3_ICC:
				cpFile = "SAM3";
				break;
			//
		}

		g_taTfCam[i] = Telium_Fopen(cpFile, "rw");
		return i;
	}
	else
		return INVALID_HANDLER;
}

INT mtiIsCardPresent(INT iHandler)
{
	UCHAR ucStatus = 0;

	if (g_taTfCam[iHandler] == NULL)
		return FALSE;

	Telium_Status(g_taTfCam[iHandler], &ucStatus);
	if ((ucStatus & CAM_PRESENT) != 0)
		return TRUE;
	else
		return FALSE;
}

INT mtiICPowerOn(INT iHandler, UCHAR *ucpCardATR, INT *iCardATRLen)
{
	INT iRet = 0;
	HISTORIC tHistoric;

	if (g_taTfCam[iHandler] == NULL)
		return RTN_IC_CTRL_NOT_INIT_CTRL;

	/***
	iRet = Telium_EMV_power_on(g_taTfCam[iHandler], &tHistoric  );
	if(iRet == 5) 		// Power once more time  on VPP problem detected
		iRet = Telium_EMV_power_on(g_taTfCam[iHandler], &tHistoric);
	else if (iRet == 2)
		iRet = Telium_Power_on(g_taTfCam[iHandler], &tHistoric);
		***/

	iRet = Telium_Power_on(g_taTfCam[iHandler], &tHistoric);
	if(iRet == 5) 		// Power once more time  on VPP problem detected
		iRet = Telium_EMV_power_on(g_taTfCam[iHandler], &tHistoric);

	dmsg("Telium_EMV_power_on() result = %d", iRet);
	dmsg("* - ok 0");
	dmsg("* - invalid card 2 Answer To Reset is not EMV compliant");
	dmsg("* - card is mute 3");
	dmsg("* - VCC or VPP problem 4/5") ;
	dmsg("* - communication problem 6");
	dmsg("* - card removed 7");

	switch (iRet)
	{
		case 0:
			mtiMemcpy(ucpCardATR, tHistoric.historic, tHistoric.length);
			*iCardATRLen = tHistoric.length;

			g_ucaPwrFlg[iHandler] = TRUE;
			iRet = RTN_SUCCESS;
			break;

		case 1:
		case 7:
			iRet = RTN_IC_CTRL_CARD_REMOVED;
			break;

		case 4:
		case 5:
			iRet = RTN_IC_CTRL_CARD_ERROR;
			break;

		case 3:
			iRet = RTN_IC_CTRL_CARD_NOT_RESPONSE;
			break;

		default:
			iRet = RTN_IC_CTRL_UNKNOWN_ERROR;
			break;
	}

	return iRet;
}

INT mtiICPowerOff(INT iHandler)
{
	if (g_taTfCam[iHandler] != NULL)
	{
		if (g_ucaPwrFlg[iHandler])
		{
			g_ucaPwrFlg[iHandler] = FALSE;
			Telium_Power_down(g_taTfCam[iHandler]);
		}
	}

	return RTN_SUCCESS;
}

INT mtiICSendApdu(INT iHandler, UCHAR *ucpApduCmd, INT iApduCmdLen, UCHAR *ucpResData, INT *iResDatalen)
{
	INT iRet = 0, iSlen = 0, iRlen = 0;
	UCHAR ucaSend[256];
	UCHAR ucaRecv[1024];
	T_APDU tApduSend;
	T_APDU tApduRecv;

	if (g_taTfCam[iHandler] == NULL)
		return RTN_IC_CTRL_NOT_INIT_CTRL;

	mtiMemset(ucaSend, 0, sizeof(ucaSend));
	mtiMemset(ucaRecv, 0, sizeof(ucaRecv));

	iSlen = iApduCmdLen < sizeof(ucaSend) ? iApduCmdLen : sizeof(ucaSend);
	mtiMemcpy(ucaSend, ucpApduCmd, iSlen);

	tApduSend.data = ucaSend;
	tApduSend.length = iSlen;
	tApduRecv.data = ucaRecv;

	dmsg("SEND >");
	dbuf("tApduSend.data", tApduSend.data, tApduSend.length);
	iRet = Telium_EMV_apdu(g_taTfCam[iHandler], &tApduSend, &tApduRecv);
	if (!iRet)
	{
		dmsg("RECV <");
		dbuf("tApduRecv.data", tApduRecv.data, tApduRecv.length);
		iRlen = tApduRecv.length;

		if(tApduRecv.data[tApduRecv.length - 2] == 0x61) // Retrieve APDU response data
		{
			mtiMemset(ucaSend, 0, sizeof(ucaSend));
			mtiMemcpy(ucaSend, "\x00\xC0\x00\x00", 4);
			ucaSend[0] = ucpApduCmd[0];
			ucaSend[4] = tApduRecv.data[tApduRecv.length - 1];
			tApduSend.length = 5;

			tApduRecv.data = &ucaRecv[iRlen - 2];

			dmsg("Get Response Data - SEND >");
			dbuf("tApduSend.data", tApduSend.data, tApduSend.length);
			iRet = Telium_EMV_apdu(g_taTfCam[iHandler], &tApduSend, &tApduRecv);

			if (!iRet)
			{
				dmsg("Get Response Data - RECV <");
				dbuf("tApduRecv.data", tApduRecv.data, tApduRecv.length);
				iRlen += tApduRecv.length - 2;
			}
		}
	}

	switch (iRet)
	{
		case 0:
			mtiMemcpy(ucpResData, ucaRecv, iRlen);
			*iResDatalen = iRlen;
			iRet = RTN_SUCCESS;
			break;

		case 1:
		case 7:
			iRet = RTN_IC_CTRL_CARD_REMOVED;
			break;

		case 2:
		case 4:
		case 5:
			iRet = RTN_IC_CTRL_CARD_ERROR;
			break;

		case 3:
			iRet = RTN_IC_CTRL_CARD_NOT_RESPONSE;
			break;

		default:
			iRet = RTN_IC_CTRL_UNKNOWN_ERROR;
			break;
	}

	return iRet;
}


VOID mtiFinalizeICController(INT iHandler)
{
	if (g_taTfCam[iHandler] != NULL)
	{
		Telium_Fclose(g_taTfCam[iHandler]);
		g_taTfCam[iHandler] = NULL;
	}
}

tEmvParamInfo *mtiEmvInitialize()
{
	mtiMemset(&g_tEmvParamInfo, 0, sizeof(tEmvParamInfo));
	return &g_tEmvParamInfo;
}

tEmvParamInfo *mtiGetEmvParmInfo()
{
	return &g_tEmvParamInfo;
}

INT mtiEmvSession(tEmvParamInfo *tpEmvParamInfo)
{
	TLV_TREE_NODE inputTlvTree = TlvTree_New(0);
	CHAR caCardReader[8 + 1];
	INT iRet = RTN_ERROR;
	UCHAR ucValue = 0;

	switch (tpEmvParamInfo->iEmvCtrlChannel)
	{
		case SLOT_MAIN_ICC:
			memset(caCardReader, 0, sizeof(caCardReader)); // added by WAY
			mtiStrcpy((char *)caCardReader,"CAM0", 4);
			break;
	}

	ucValue = 0x00;
	if (TlvTree_AddChild(inputTlvTree, TAG_EMV_INT_TRANSACTION_TYPE, &ucValue, 1) == NULL)
	{
		ASSERT(FALSE);
		return MTI_EMV_TR_UNKNOWN_ERROR;
	}

	ucValue = 0x00;
	if (TlvTree_AddChild(inputTlvTree, TAG_EMV_TRANSACTION_TYPE, &ucValue, 1) == NULL)
	{
		ASSERT(FALSE);
		return MTI_EMV_TR_UNKNOWN_ERROR;
	}

	iRet = APEMV_ServicesEmv_DoTransaction(caCardReader, mtiStrlen(8, caCardReader), TRUE, inputTlvTree);
	dmsg("APEMV_ServicesEmv_DoTransaction() Result = %d", iRet);

	switch (iRet)
	{
		case EMV_STATUS_SUCCESS:
			iRet = MTI_EMV_TR_SUCCESS;
			break;

		case EMV_STATUS_CANCEL:
			iRet = MTI_EMV_TR_CANCEL;
			break;

		case EMV_STATUS_CARD_PROCESSING_ERROR:
			iRet = MTI_EMV_TR_CARD_PROCESSING_ERROR;
			break;

		case EMV_STATUS_MISSING_MANDATORY_TERM_DATA:
		case EMV_STATUS_CARD_DATA_ERROR:
			iRet = MTI_EMV_TR_CARD_DATA_ERROR;
			break;

		case EMV_STATUS_CARD_BLOCKED:
			iRet = MTI_EMV_TR_CARD_BLOCKED;
			break;

		case EMV_STATUS_CARD_REMOVED:
			iRet = MTI_EMV_TR_CARD_REMOVED;
			break;

		case EMV_STATUS_SERVICE_NOT_ALLOWED:
			iRet = MTI_EMV_TR_SERVICE_NOT_ALLOWED;
			break;

		case EMV_STATUS_INVALID_CARD:
			iRet = MTI_EMV_TR_INVALID_CARD;
			break;

		case EMV_STATUS_SELECT_ANOTHER_AID:
			iRet = MTI_EMV_TR_AID_SELECT_ERROR;
			break;

		case EMV_STATUS_2NDGENAC_RESULT_AAC:
			iRet = MTI_EMV_TR_CARD_AAC;
			break;

		default:
			iRet = MTI_EMV_TR_UNKNOWN_ERROR;
			break;
	}

	EPSTOOL_TlvTree_Release(&inputTlvTree);
	dmsg("mtiEmvSession() Result %d", iRet);
	return iRet;
}

INT mtiEmvSetParam(INT bDoClear)
{
	return RTN_SUCCESS;
}

VOID mtiEmvFinalize(tEmvParamInfo *tpEmvParamInfo)
{
	mtiMemset(tpEmvParamInfo, 0, sizeof(tpEmvParamInfo));
}


