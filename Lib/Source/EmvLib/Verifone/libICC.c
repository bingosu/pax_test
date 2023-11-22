/*
 * LibICC.c
 *
 *  Created on: 2016. 10. 14.
 *      Author: kjyoo
 */
#include "libMtiCommonApi.h"
#include "cardslot.h"

#define  E_INV_PARAM			(char) -14
#define  E_HIST_BYTES			(char) -15
#define  E_IF_BYTES				(char) -16

#include <EMV/EMV_CT_Interface.h>
#include <EMV/EMV_Common_Interface.h>

#define IC_CTRL_MAX_HANDLER_COUNT			5

typedef struct _tIccHnd
{
	ULONG ulHnd;
	INT	iCtrlType;
} tIccHnd;

tIccHnd g_ICCHnd[IC_CTRL_MAX_HANDLER_COUNT] = { { 0, 0 }, };


static char* getIccErrStr(unsigned char err)
{
	static char *iccErrMsg = NULL;

	switch(err)
	{
		case E_INV_PARAM:
			iccErrMsg = "E_INV_PARAM";
			break;
		case E_SLOT_INITIALIZE:
			iccErrMsg = "E_SLOT_INITIALIZE";
			break;
		case E_SLOT_SELECT:
			iccErrMsg = "E_SLOT_SELECT";
			break;
		case E_NUL_RESP_BUF:
			iccErrMsg = "E_NUL_RESP_BUF";
			break;
		case CARD_ABSENT:
			iccErrMsg = "CARD_ABSENT";
			break;
		case E_ICC_RESET:
			iccErrMsg = "E_ICC_RESET";
			break;
		case E_TRANSMIT:
			iccErrMsg = "E_TRANSMIT";
			break;
		case E_ERROR_OVERFLOW:
			iccErrMsg = "E_ERROR_OVERFLOW";
			break;
		default:
			iccErrMsg = "UNKNOWN ERROR";
	}
	return iccErrMsg;
}
INT mtiInitializeICController(INT iCtrlType)
{
	INT i = 0;
	UCHAR ucFlag = FALSE;
	CHAR cRet = 0x00;
	ULONG ulSlot;

	for (i = 0; i < IC_CTRL_MAX_HANDLER_COUNT; i++)
	{
		if (g_ICCHnd[i].ulHnd == NULL)
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
				ulSlot = CUSTOMER_CARD;
				break;

			case SLOT_SAM1_ICC:
				ulSlot = MERCHANT_SLOT_1;
				break;

			case SLOT_SAM2_ICC:
				ulSlot = MERCHANT_SLOT_2;
				break;

			case SLOT_SAM3_ICC:
				ulSlot = MERCHANT_SLOT_3;
				break;
			default:
				dmsg("Invalid argument iCtrlType [%d]",iCtrlType);
				return RTN_ERROR;
		}

		//if(iCtrlType != SLOT_MAIN_ICC)
		{
			cRet = Init_CardSlot(ulSlot);
			//dmsg("Init_CardSlot ret [0x%.2x]",cRet);

			if(cRet != CARDSLOT_SUCCESS)
			{
				dmsg("Init_CardSlot Failed [0x%.2x]",cRet);
				return INVALID_HANDLER;
			}
		}
		g_ICCHnd[i].ulHnd = ulSlot;
		g_ICCHnd[i].iCtrlType = iCtrlType;
		//dmsg("i [%d] g_ICCHnd[i].ulHnd[%d]",i, g_ICCHnd[i].ulHnd);
		return i;
	}
	else
	{
		dmsg("No valid handler");
		return INVALID_HANDLER;
	}
}

VOID mtiFinalizeICController(INT iHandler)
{
	CHAR cRet;

	//dmsg("mtiFinalizeICController - iHandler [%d] g_ICCHnd[iHandler].ulHnd[%d]",iHandler, g_ICCHnd[iHandler].ulHnd);
	if (g_ICCHnd[iHandler].ulHnd != NULL)
	{
		//if(g_ICCHnd[iHandler].iCtrlType != SLOT_MAIN_ICC)
		{
			cRet = Close_CardSlot(g_ICCHnd[iHandler].ulHnd);
			//dmsg("Close_CardSlot Ret [0x%.2x]",cRet);
		}
		g_ICCHnd[iHandler].ulHnd = NULL;
		g_ICCHnd[iHandler].iCtrlType = NULL;
	}
	else
	{
		dmsg("ICC is already Finalized");
	}
}

INT mtiICPowerOn(INT iHandler, UCHAR *ucpCardATR, INT *iCardATRLen)
{
	UCHAR dwValue = 0;
	UCHAR cRet = 0;
	CHAR cIfLen, cHistLen;
	UCHAR ATR[128] = {0,};

	if(g_ICCHnd[iHandler].ulHnd == NULL)
	{
		dmsg("iHandler[%d] is not initialized.", iHandler);
		return RTN_ERROR;
	}
#if 0
	if(g_ICCHnd[iHandler].iCtrlType == SLOT_MAIN_ICC)
	{
		cRet = EMV_CT_SmartReset(0, ucpCardATR, (ULONG *)iCardATRLen);
		dmsg("ATR");
		dbuf(NULL, ucpCardATR, *iCardATRLen);
		SVC_WAIT(10);
		if(cRet != EMV_ADK_SMART_STATUS_OK || *iCardATRLen < 1)
			return RTN_ERROR;
	}
	else
#endif
	{
		if(g_ICCHnd[iHandler].ulHnd == CUSTOMER_CARD)
		{
			dwValue = ISO_7816;
			cRet = Set_Card_Type(g_ICCHnd[iHandler].ulHnd, ASYNC, dwValue );
			dmsg("Set_Card_Type Ret [0x%.2x]",cRet);
			SVC_WAIT(100);
		}

		//RESET
//@@WAY BRI SAM ISSUE
#if 1
		cRet = Reset_CardSlot( g_ICCHnd[iHandler].ulHnd, RESET );
		dmsg("Cold Reset_CardSlot Ret [0x%.2x] [%d]",cRet, (INT)cRet);
		if(cRet != CARDSLOT_SUCCESS)
		{
			if(cRet == E_ICC_RESET)
			{
				Set_Card_Type( g_ICCHnd[iHandler].ulHnd, ASYNC, ISO_7816 );
				cRet = Reset_CardSlot( g_ICCHnd[iHandler].ulHnd, RESET );
				dmsg("Cold Reset_CardSlot Ret [0x%.2x] [%d]",cRet, (INT)cRet);
				if(cRet != CARDSLOT_SUCCESS)
				{
					dmsg("Reset is failed too.");
					return RTN_ERROR;
				}
			}
			else
			{
				dmsg("Reset is failed.");
				return RTN_ERROR;
			}
		}
#else
		cRet = Reset_CardSlot( g_ICCHnd[iHandler].ulHnd, RESET );
		dmsg("Cold Reset_CardSlot Ret [0x%.2x] [%d]",cRet, (INT)cRet);
		if(cRet != CARDSLOT_SUCCESS)
		{
			if(cRet == 0xE4)
			{
				dmsg("CARD IS NOT INSERTED");
				return RTN_IC_CTRL_CARD_REMOVED;
			}
			dmsg("Cold Reset is failed. try to warm reset!")
			SVC_WAIT(100);
			cRet = Reset_CardSlot( g_ICCHnd[iHandler].ulHnd, RESET_WARM );
			dmsg("Warm Reset_CardSlot Ret [0x%.2x] [%d]",cRet, (INT)cRet);

			if(cRet != CARDSLOT_SUCCESS)
			{
				dmsg("Warm Reset is failed too.");
				return RTN_ERROR;
			}
		}
#endif

		SVC_WAIT(100);

		cRet = Get_Protocol(g_ICCHnd[iHandler].ulHnd);
		dmsg("Get_Protocol Ret [0x%.2x]",cRet);
		if((cRet != PROTOCOL_T0) && (cRet != PROTOCOL_T1)){
			dmsg("Protocol is not allowed");
			return RTN_ERROR;
		}
		SVC_WAIT(100);
		cIfLen = Get_Interface_Bytes(g_ICCHnd[iHandler].ulHnd, ATR);
		if(cIfLen < 1)
		{
			dmsg("Get_Interface_Bytes is Failed, [%d]",cIfLen);
			return RTN_ERROR;
		}
		dbuf("Get_Interface_Bytes", ATR, cIfLen);

		//Get Historical Bytes
		SVC_WAIT(100);
		cHistLen = Get_Historical_Bytes(g_ICCHnd[iHandler].ulHnd, ATR + cIfLen);
		if(cHistLen < 1)
		{
			dmsg("Get_Historical_Bytes is Failed, [%d]",cHistLen);
			return RTN_ERROR;
		}
		dbuf("Get_Historical_Bytes", (UCHAR *)(ATR + cIfLen), cHistLen);

		dbuf("ATR", ATR, cIfLen+cHistLen);

		*iCardATRLen = cIfLen+cHistLen;
		mtiMemcpy(ucpCardATR, ATR, *iCardATRLen);
	}
	return RTN_SUCCESS;
}

INT mtiICPowerOff(INT iHandler)
{
	CHAR cRet = 0;

	if(g_ICCHnd[iHandler].ulHnd == NULL)
	{
		dmsg("iHandler[%d] is not initialized.", iHandler);
		return RTN_ERROR;
	}
#if 0
	if(g_ICCHnd[iHandler].iCtrlType == SLOT_MAIN_ICC)
	{
		cRet = EMV_CT_SmartPowerOff(0);

		if(cRet != EMV_ADK_SMART_STATUS_OK)
		{
			dmsg("----------EMV_CT_SmartPowerOff error [%d]", cRet);
		}
	}
	else
#endif
	{
		cRet = Terminate_CardSlot(g_ICCHnd[iHandler].ulHnd, SWITCH_OFF_CARD);
		dmsg("Terminate_CardSlot Ret [%d]",(INT)cRet);
	}
	return RTN_SUCCESS;

}

INT mtiICSendApdu(INT iHandler, UCHAR *ucpApduCmd, INT iApduCmdLen, UCHAR *ucpResData, INT *iResDatalen)
{
	UCHAR uRet = 0;
	UCHAR *ucpSendBuff = NULL;
	INT iSendApduLen;
	USHORT usApduCmdLen, usResDatalen, usOutBuffLen = 512;

	if(g_ICCHnd[iHandler].ulHnd == NULL)
	{
		dmsg("iHandler[%d] is not initialized.", iHandler);
		return RTN_ERROR;
	}

#if 1
	if(ucpApduCmd[0] == 0x00 && ucpApduCmd[1] == 0xA4)	// Only AID Selection + 0x00
		iSendApduLen = iApduCmdLen + 1;
	else
#endif
		iSendApduLen = iApduCmdLen;

	ucpSendBuff = mtiMalloc(iSendApduLen);
	memset(ucpSendBuff, 0x00, iSendApduLen);
	memcpy(ucpSendBuff, ucpApduCmd, iApduCmdLen);
	usApduCmdLen = (USHORT) iSendApduLen;
	usResDatalen = 512;

	dmsg("APDU CMD iApduCmdLen[%hd] Recv Max Len[%hd]",usApduCmdLen, usResDatalen);
	dbuf("APDU CMD", ucpSendBuff, usApduCmdLen);

#if 0
	if(g_ICCHnd[iHandler].iCtrlType == SLOT_MAIN_ICC)
	{
		uRet = EMV_CT_SmartISO(0, usApduCmdLen, ucpSendBuff, &usResDatalen, ucpResData, usOutBuffLen);
		//uRet = Transmit_APDU_Extended(CUSTOMER_CARD, ucpSendBuff, usApduCmdLen, ucpResData, &usResDatalen);
		dmsg("EMV_CT_SmartISO Ret[%d] iResDatalen[%hd]", uRet, usResDatalen);

	}
	else
#endif
	{
		uRet = Transmit_APDU_Extended(g_ICCHnd[iHandler].ulHnd, ucpSendBuff, usApduCmdLen, ucpResData, &usResDatalen);
		//uRet = Transmit_APDU(g_ICCHnd[iHandler].ulHnd, ucpSendBuff, usApduCmdLen, ucpResData, &usResDatalen);
		dmsg("Transmit_APDU_Extended Slot Num[%d] Ret[%d] iResDatalen[%hd]", g_ICCHnd[iHandler].ulHnd, uRet, usResDatalen);
		if(uRet != CARDSLOT_SUCCESS)
			dmsg("Transmit_APDU_Extended ERR [%s]", getIccErrStr(uRet));
	}

	dbuf("RESP DATA", ucpResData, usResDatalen);
	*iResDatalen = (INT)usResDatalen;
	mtiFree(ucpSendBuff);

	return RTN_SUCCESS;
}

#if 0
INT mtiIsCardPresent(INT iHandler)
{
	UCHAR ucStatus = 0;

	ucStatus = EMV_CT_SmartDetect(EMV_CT_SKIP_ATR);

	if(ucStatus != EMV_ADK_SMART_STATUS_OK )
	{
		if(ucStatus != EMV_ADK_SMART_STATUS_REMOVED)
			dmsg("ucStatus [0x%.2x]", ucStatus);
		return FALSE;
	}

	return TRUE;
}
#else


INT mtiIsCardPresent(INT iHandler)
{
	CHAR cRet = 0x00;
#if 0
	if(g_ICCHnd[iHandler].iCtrlType == SLOT_MAIN_ICC)
	{
		cRet = EMV_CT_SmartDetect(EMV_CT_SKIP_ATR);
		if(cRet == EMV_ADK_SMART_STATUS_OK)
			return TRUE;
		else
			return FALSE;
	}
	else
#endif
	if(g_ICCHnd[iHandler].ulHnd > 0)
	{
		cRet = Get_Card_State(g_ICCHnd[iHandler].ulHnd);
		if(cRet == CARD_PRESENT)
		{
			return TRUE;
		}
		else if(cRet == CARD_ABSENT)
			return FALSE;
		else
		{
//			dmsg("Get_Card_State RET [%d] [0x%.x]", cRet, cRet);
			cRet = EMV_CT_SmartPowerOff(0);
			mtiFinalizeICController(iHandler);
			mtiInitializeICController(SLOT_MAIN_ICC);
			SVC_WAIT(200);
			return FALSE;
		}
	}
	else
	{
		dmsg("Already ICC iHandler[%d] Finalized.", iHandler);
		dmsg("g_ICCHnd[iHandler][%ld]", g_ICCHnd[iHandler]);
	}
	return FALSE;
}
#endif
