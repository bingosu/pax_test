/*
 * libCTLS.c
 *
 *  Created on: Sep 25, 2016
 *      Author: share_pc
 */

#include <time.h>

#include "libEmvCTLS.h"
#include "apMem.h"
#include "libMtiEmvApi.h"

extern INT bcdfromulong(UCHAR *bcd, INT size, ULONG value);
extern void vdGetDate(unsigned char *buf, size_t bufLen);
extern void vdGetTime(unsigned char *buf, size_t bufLen);

char gszCurrecyCode[3+1];

char cMessageFlag = 0x00;

int g_iPaypassKeyIndex = 0;

void vdSetCTLSTransactionData(EMV_CTLS_START_TYPE *xSelectInput, char *szAmount)
{
	// @@EB REFUND PAYWAVE
//@@WAY PAYWAVE REFUND 20200130
//#if 1
#if 0
	if (mtiGetRfEmvParmInfo()->iTranType == MTI_EMV_TRAN_VOID)
	{
		xSelectInput->TransType =  EMV_ADK_TRAN_TYPE_REFUND;
	}
	else
	{
		xSelectInput->TransType =  EMV_ADK_TRAN_TYPE_GOODS_SERVICE;
	}
#else
	xSelectInput->TransType =  EMV_ADK_TRAN_TYPE_GOODS_SERVICE;
#endif
	//
//@@WAY PAYWAVE REFUND 20200130

	xSelectInput->Info_Included_Data[0] |= INPUT_CTLS_SEL_TTYPE;    //raise the flag

	bcdfromulong (xSelectInput->TXN_Data.Amount, sizeof(xSelectInput->TXN_Data.Amount),(unsigned long)atol(szAmount));
	dbuf("TXN_Data.Amount", xSelectInput->TXN_Data.Amount, sizeof(xSelectInput->TXN_Data.Amount));
	xSelectInput->Info_Included_Data[0] |= INPUT_CTLS_SEL_AMOUNT;    //Availability bit

	xSelectInput->TXN_Data.ExpTrans = 0x02;
	xSelectInput->Info_Included_Data[0] |= INPUT_CTLS_SEL_CUREXPONENT;

	xSelectInput->TXN_Data.CurrencyTrans[0] = 0x03;
	xSelectInput->TXN_Data.CurrencyTrans[1] = 0x60;

	xSelectInput->Info_Included_Data[0] |= INPUT_CTLS_SEL_AMOUNT_CURRENCY;

	xSelectInput->TXN_Data.Online_Switch = FALSE;
	xSelectInput->Info_Included_Data[1] |= INPUT_CTLS_SEL_ONLINE_SWITCH;    //raise the flags

	memset(xSelectInput->TxnOptions, 0, sizeof(xSelectInput->TxnOptions));
	//xSelectInput->TxnOptions[2] |=CLTRXOP_STOP_ON_CHKSUM_DIFF;
	//xSelectInput->TxnOptions[2] |= CLTRXOP_L1_ERROR_CALLBACK;  //TODO: This flag must be activated to detect text fallback EMV_ADK_TXT_2_CARDS_IN_FIELD. However is not working properly.
	xSelectInput->TxnOptions[2] |= CLTRXOP_OMIT_CHECKSUM_CHECK |CLTRXOP_NO_AMOUNT_PRECHECK;
	xSelectInput->Info_Included_Data[1] |= INPUT_CTLS_SEL_TXN_OPTIONS;    //raise the flag

	vdGetDate((unsigned char*) xSelectInput->TXN_Data.Date, sizeof(xSelectInput->TXN_Data.Date));
	xSelectInput->Info_Included_Data[0] |= INPUT_CTLS_SEL_DATE;    //raise the flag

	vdGetTime((unsigned char*) xSelectInput->TXN_Data.Time, sizeof(xSelectInput->TXN_Data.Time));
	xSelectInput->Info_Included_Data[0] |= INPUT_CTLS_SEL_TIME;    //raise the flag

	//For VISA PAYWAVE
	xSelectInput->TXN_Data.Additional_Result_Tags.anztag = 2;
	xSelectInput->TXN_Data.Additional_Result_Tags.tags[0] = 0x9F7C;
	xSelectInput->TXN_Data.Additional_Result_Tags.tags[1] = 0x9F6E;
	xSelectInput->Info_Included_Data[1] |= INPUT_CTLS_SEL_ADD_TAGS;    //raise the flag

//	memcpy(xSelectInput->TXN_Data.CurrencyTrans,"\x08\x62",2);
//	xSelectInput->Info_Included_Data[0] = INPUT_CTLS_SEL_AMOUNT_CURRENCY

	//xSelectInput->passthroughCardTypes = CLTRX_PASSTHROUGH_OFF;
	//xSelectInput->Info_Included_Data[1] |= INPUT_CTLS_SEL_PASSTHROUGH;
}

// ====================================================================================================================
void vdEndTransactionCTLS(const char *szStatusMessage, unsigned char ucReader, int inExitWithError)
{

/*
	dmsg("[*] cts_StopSelection start %lu",mtiGetTickCount());
	inResult = cts_StopSelection();
	dmsg("[*] cts_StopSelection end %d : %lu", inResult, mtiGetTickCount());
*/
	EMV_ADK_INFO erg = 0;

	dmsg("vdEndTransactionCTLS : %s", szStatusMessage);

	erg = EMV_CTLS_EndTransaction(0);
	dmsg("EMV_CTLS_EndTransaction reg[0x%.2x]", erg);
	EMV_CTLS_SmartPowerOff(ucReader);
	EMV_CTLS_LED(0, CONTACTLESS_LED_IDLE_BLINK);
}

void vdFormatCurrencyCode(unsigned char *pucHexCCode, char * szFormattedCCode)
{
	sprintf(szFormattedCCode,"%X%X",pucHexCCode[0], pucHexCCode[1]);
	dmsg("Currency Code [%d]", atoi(szFormattedCCode));
	switch (atoi(szFormattedCCode)){   //Add more currency codes as needed
		case 124:
			//@@CACING PENTEST strcpy(szFormattedCCode,"CA$");
			strncpy(szFormattedCCode,"CA$",3);
			break;
		case 937:
			//@@CACING PENTEST strcpy(szFormattedCCode,"Bs.");
			strncpy(szFormattedCCode,"Bs.",3);
			break;
		case 360:
			//@@CACING PENTEST strcpy(szFormattedCCode,"RP");
			strncpy(szFormattedCCode,"RP",2);
			break;
		case 392:
			//@@CACING PENTEST strcpy(szFormattedCCode,"JPY");
			strncpy(szFormattedCCode,"JPY",3);
			break;
		default:
			//@@CACING PENTEST strcpy(szFormattedCCode,"US$");
			strncpy(szFormattedCCode,"US$",3);
	}
}

static void setOnlineTransactionTag(EMV_CTLS_TRANSRES_TYPE* pxTransRes)
{
	unsigned int i;
	char buf[256] = {0,};

	tMtiMap *tpMap = &mtiGetRfEmvParmInfo()->tEmvTagMap;

	dmsg("[setOnlineTransactionTag] start");


	//AID
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_CTLS_9F06_AID)
	{
		dbuf("AID 9F06:",pxTransRes->T_9F06_AID.AID, pxTransRes->T_9F06_AID.aidlen);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_AID_TERMINAL, pxTransRes->T_9F06_AID.AID, pxTransRes->T_9F06_AID.aidlen);
	}

	//PAN
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_5A_PAN)
	{
		for(i=0; i<sizeof(pxTransRes->T_5A_PAN); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5A_PAN[i]);
		dmsg("PAN 5A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLI_PAN, pxTransRes->T_5A_PAN, sizeof(pxTransRes->T_5A_PAN));

	}

	//Expire Date
    if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_5F24_APPEXPDATE) {
        for(i=0; i<sizeof(pxTransRes->T_5F24_AppExpDate); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_5F24_AppExpDate[i]);
        dmsg("Expiry data 5F24: %s\n",buf);
        mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLI_EXPIRATION_DATE, pxTransRes->T_5F24_AppExpDate, sizeof(pxTransRes->T_5F24_AppExpDate));
    }


	//PAN SEQ NUM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_5F34_PAN_SEQ_NUMBER)
	{
		for(i=0; i<sizeof(pxTransRes->T_5F34_PANSequenceNo); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5F34_PANSequenceNo[i]);

		dmsg("PAN sequ. no. 5F34: %s",buf);

		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLI_PAN_SEQUENCE_NUMBER, pxTransRes->T_5F34_PANSequenceNo,
				sizeof(pxTransRes->T_5F34_PANSequenceNo));
	}

	//TRACK 2
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_57_DATA_TRACK2)
	{
		for(i=0; i<pxTransRes->T_57_DataTrack2.tr2len; i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_57_DataTrack2.tr2data[i]);
		dmsg("Track2 57: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRACK_2_EQU_DATA, pxTransRes->T_57_DataTrack2.tr2data,
				pxTransRes->T_57_DataTrack2.tr2len);
	}

	//CRYPTOGRAM INFO
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_9F27_CRYPTINFO)
	{
		dmsg("Cryptogram info 9F27: 0x%02X",pxTransRes->T_9F27_CryptInfo);
		if(!memcmp(pxTransRes->T_9F06_AID.AID,"\xA0\x00\x00\x00\x04",5))	//Store - MASTER
		{
			mtiMapPutBytes(tpMap, MTI_EMV_TAG_CRYPTOGRAM_INFO_DATA, &pxTransRes->T_9F27_CryptInfo, sizeof(pxTransRes->T_9F27_CryptInfo));
		}
	}

	//CRYPTOGRAM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_9F26_CRYPTOGRAMM)
	{
		dmsg("Cryptogram 9F26: 0x%02X",pxTransRes->T_9F26_Cryptogramm);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLICATION_CRYPTOGRAM, pxTransRes->T_9F26_Cryptogramm,
				sizeof(pxTransRes->T_9F26_Cryptogramm));
	}

	//TVR
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_95_TVR){
		for(i=0; i<sizeof(pxTransRes->T_95_TVR); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_95_TVR[i]);
		dmsg("TVR 95: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TVR, pxTransRes->T_95_TVR, sizeof(pxTransRes->T_95_TVR));
	}

	//CVM RESULT
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_9F34_CVM_RES){
		for(i=0; i<sizeof(pxTransRes->T_9F34_CVM_Res); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F34_CVM_Res[i]);
		dmsg("CVM RES 9F34: %s",buf);
		if(!memcmp(pxTransRes->T_9F06_AID.AID,"\xA0\x00\x00\x00\x04",5))	//Store - MASTER
		{
			mtiMapPutBytes(tpMap, MTI_EMV_TAG_CVM_RESULTS, pxTransRes->T_9F34_CVM_Res,
					sizeof(pxTransRes->T_9F34_CVM_Res));
		}
	}

	//AMOUNT AUTH NUM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_CTLS_9F02_AMOUNT)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F02_TXNAmount); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F02_TXNAmount[i]);

		dmsg("Txn amount 9F02: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_AMOUNT_AUTH_NUM, pxTransRes->T_9F02_TXNAmount, sizeof(pxTransRes->T_9F02_TXNAmount));
	}

	//AMOUNT OTHER NUM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_CTLS_9F03_CB_AMOUNT)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F03_TXNAdditionalAmount); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F03_TXNAdditionalAmount[i]);

		dmsg("Txn other amount 9F03: %s",buf);
		if(!memcmp(pxTransRes->T_9F06_AID.AID,"\xA0\x00\x00\x00\x03",5))	//Store - VISA
		{
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_AMOUNT_OTHER_NUM, pxTransRes->T_9F03_TXNAdditionalAmount,
				sizeof(pxTransRes->T_9F03_TXNAdditionalAmount));
		}
	}

	//TRANSACTION CURRENCY CODE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_5F2A_TRANS_CURRENCY)
	{
		for(i=0; i<sizeof(pxTransRes->T_5F2A_CurrencyTrans); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5F2A_CurrencyTrans[i]);

		dmsg("Txn Currency Code 5F2A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRANSACTION_CURRENCY_CODE, pxTransRes->T_5F2A_CurrencyTrans,
				sizeof(pxTransRes->T_5F2A_CurrencyTrans));
	}

	//TRANSACTION DATE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_9A_DATE)
	{
		for(i=0; i<sizeof(pxTransRes->T_9A_Date); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9A_Date[i]);

		dmsg("Txn DATE 9A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRANSACTION_DATE, pxTransRes->T_9A_Date, sizeof(pxTransRes->T_9A_Date));
	}

	//UNPREDICTABLE NUMBER
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_9F37_RANDOM_NUMBER)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F37_RandomNumber); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F37_RandomNumber[i]);

		dmsg("Txn RANDOM NUM 9F37: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_UNPREDICTABLE_NUMBER, pxTransRes->T_9F37_RandomNumber,
				sizeof(pxTransRes->T_9F37_RandomNumber));
	}

	//AIP
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_82_AIP){
		for(i=0; i<sizeof(pxTransRes->T_82_AIP); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_82_AIP[i]);
		dmsg("AIP 82: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_AIP, pxTransRes->T_82_AIP, sizeof(pxTransRes->T_82_AIP));
	}

	//DF_NAME
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_84_DFNAME){
		for(i=0; i < pxTransRes->T_84_DFName.aidlen; i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_84_DFName.AID[i]);
		dmsg("DF NAME 84: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_DF_NAME, pxTransRes->T_84_DFName.AID, pxTransRes->T_84_DFName.aidlen);
	}

	//ISSUER APPLI DATA
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_9F10_DATAISSUER){
		for(i=0; i < pxTransRes->T_9F10_DataIssuer.issDataLen; i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F10_DataIssuer.issData[i]);
		dmsg("ISSUER DATA 9F10: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_ISSUER_APPLI_DATA, pxTransRes->T_9F10_DataIssuer.issData,
				pxTransRes->T_9F10_DataIssuer.issDataLen);
	}

	//ATC
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_9F36_ATC){
		for(i=0; i<sizeof(pxTransRes->T_9F36_ATC); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F36_ATC[i]);
		dmsg("ATC 9F36: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_ATC, pxTransRes->T_9F36_ATC, sizeof(pxTransRes->T_9F36_ATC));
	}

	//TRANSACTION TYPE
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_9C_TRANSTYPE)
	{
		dmsg("Txn type 9C: 0x%02X",pxTransRes->T_9C_TransType);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRANSACTION_TYPE, &pxTransRes->T_9C_TransType, sizeof(pxTransRes->T_9C_TransType));
	}

	//TERMINAL COUNTRY CODE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_9F1A_TERM_COUNTRY_CODE){
		for(i=0; i<sizeof(pxTransRes->T_9F1A_TermCountryCode); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F1A_TermCountryCode[i]);
		dmsg("TERMINAL COUNTRY CODE 9F1A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TERMINAL_COUNTRY_CODE, pxTransRes->T_9F1A_TermCountryCode, sizeof(pxTransRes->T_9F1A_TermCountryCode));
	}

	//TERMINAL CAPABILITY
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_9F33_TERMCAP){
		for(i=0; i<sizeof(pxTransRes->T_9F33_TermCap); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F33_TermCap[i]);
		dmsg("TERM CAPABILITIES 9F33: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TERMINAL_CAPABILITIES, pxTransRes->T_9F33_TermCap, sizeof(pxTransRes->T_9F33_TermCap));
	}

	//TERMINAL TYPE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_9F35_TERMTYP){
		dmsg("TERMINAL TYPE 9F35: 0x%02X",pxTransRes->T_9F35_TermTyp);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TERMINAL_TYPE, &pxTransRes->T_9F35_TermTyp, sizeof(pxTransRes->T_9F35_TermTyp));
	}

	//IDF SERIAL NUMBER
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_9F1E_IFDSERIALNUMBER){
		for(i=0; i<sizeof(pxTransRes->T_9F1E_IFDSerialNumber); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F1E_IFDSerialNumber[i]);
		dmsg("IDF SERIAL NUM 9F1E: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_IFD_SERIAL_NUMBER, pxTransRes->T_9F1E_IFDSerialNumber,
				sizeof(pxTransRes->T_9F1E_IFDSerialNumber));
	}

	//APPLI CURRENCY CODE
	//VERIFONE Not Support Anymore...

	//ISSUER COUNTRY CODE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_CTLS_5F28_ISSCOUNTRYCODE){
		for(i=0; i<sizeof(pxTransRes->T_5F28_IssCountryCode); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5F28_IssCountryCode[i]);
		dmsg("ISSUER COUNTRY CODE 5F28: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_ISSUER_COUNTRY_CODE, pxTransRes->T_5F28_IssCountryCode,
				sizeof(pxTransRes->T_5F28_IssCountryCode));
	}

	//9F7C
	//9F6E
	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_CTLS_ADDITIONAL_TAGS)
	{
		UCHAR *ucpBuff = pxTransRes->Add_TXN_Tags;

		dbuf("TRX_CTLS_ADDITIONAL_TAGS:",pxTransRes->Add_TXN_Tags, sizeof(pxTransRes->Add_TXN_Tags));

		iSerialBytesToTagMap(tpMap, ucpBuff, 0, EMV_ADK_ADD_TAG_SIZE);
	}

	//9F21
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_9F21_TIME)
	{
		dbuf("TRANSACTION TIME 9F21:", pxTransRes->T_9F21_Time, sizeof(pxTransRes->T_9F21_Time));
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRANSACTION_TIME, pxTransRes->T_9F21_Time,
				sizeof(pxTransRes->T_9F21_Time));
	}

	//9F40
	if(pxTransRes->T_DF61_Info_Received_Data[4] & TRX_CTLS_9F40_TERMCAP)
	{
		dbuf("ADD TERMINAL CAP 9F40:", pxTransRes->T_9F40_AddTermCap, sizeof(pxTransRes->T_9F40_AddTermCap));
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES, pxTransRes->T_9F40_AddTermCap,
				sizeof(pxTransRes->T_9F40_AddTermCap));
	}

	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_CTLS_APPNAME)
	{
		dmsg("APPNAME 50 / 9F12: %s",pxTransRes->AppName);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLICATION_LABEL, pxTransRes->AppName, sizeof(pxTransRes->AppName));

		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLI_PREFERRED_NAME, pxTransRes->AppName, sizeof(pxTransRes->AppName));
	}
	dmsg("EMV TAG MAP DATA");
	dbgMapEle(tpMap);
	dmsg("[setOnlineTransactionTag] end");
}

static void setFinalCryptogram(EMV_CTLS_TRANSRES_TYPE* pxTransRes)
{
	char buf[256] = {0,};
	tMtiMap *tpMap = &mtiGetRfEmvParmInfo()->tEmvTagMap;

	dmsg("[setFinalCryptogram] start");

	//CRYPTOGRAM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_9F26_CRYPTOGRAMM)
	{
		dmsg("Cryptogram 9F26: 0x%02X",pxTransRes->T_9F26_Cryptogramm);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLICATION_CRYPTOGRAM, pxTransRes->T_9F26_Cryptogramm,
				sizeof(pxTransRes->T_9F26_Cryptogramm));
	}
}

void CallbackInit_OutTLV(unsigned char *pucReceive, unsigned short *psReceiveSize)
{
  if(pucReceive != NULL && *psReceiveSize >= 2)
  {
    memcpy(pucReceive, "\xF0\x00", 2);
    *psReceiveSize = 2;
  }
}

void emvCallback(unsigned char *pucSend, unsigned short sSendSize, unsigned char *pucReceive, unsigned short *psReceiveSize, void* externalData)
{
  struct BTLVNode *node = NULL;
  int tag = 0;
  unsigned char init = 0;
  struct BTLVNode xBtlv;
//  int inResult;
  unsigned char gucAmountConfirmed = TRUE;

  vBTLVInit(&xBtlv,NULL);
  if((iBTLVImport(&xBtlv, pucSend, sSendSize, NULL, NULL)!=0)||((node=pxBTLVFindTag(&xBtlv, pcBTLVTagStr(TAG_F0_EMV_TEMPLATE)))==NULL)||((node = pxBTLVGetChild(node, 0))==NULL))// import TLV stream
  {
    init = 1; // import of message failed or message contains no F0-tag
  }
  if(init == 0)
  {
    sscanf(node->tcName, "%X", &tag);
    if(tag != TAG_BF7F_CTLS_CBK_TRACE)
	{
		dmsg("EMV CTLS CALLBACK FUNC TAG[0x%.4x]", tag);
	}
    switch(tag)
    {
    	case TAG_BF7F_CTLS_CBK_TRACE:{ 
			//Trace callback. This option happens when EMV_CT_INIT_OPT_TRACE or EMV_CT_INIT_OPT_TRACE are enabled during the Framework Init.
    		struct BTLVNode *x = NULL;
   		    char *str = NULL;

			if ((x = pxBTLVFindTag(node, pcBTLVTagStr(TAG_TRACE))) != NULL) // TAG_TRACE
			{
				str = (char*) malloc(x->uSize + 1);
				if (str){
					memcpy(str, x->pucData, x->uSize);
					str[x->uSize] = 0x00;
					dmsg("EMV ADK: %s", str);
					free(str);
				}
			}
			init=1;
			break;
    	}

    	//Refer to EMV_CT_Interface.h for details: e.g. Tag DF70 is what application gets from the Framework and tag DF70 is what application is supposed to send back
    	case TAG_BF0B_INIT_CALLBACK_THREAD:{
              init = 1;
              break;
    	}

    	case TAG_BF01_CBK_MERCHINFO:{ 				//Refer to EMV_CT_Interface.h for details: e.g. Tag DF70 is what application gets from the Framework and tag DF70 is what application is supposed to send back
    		unsigned char merchantInfo;
    		dmsg("----------> CALLBACK MERCHANT INFO");
    		if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF70_CBK_MERCHINFO),&merchantInfo, sizeof(merchantInfo)) == sizeof(merchantInfo))
    			printf("Merchant Info: (%#.2x)\n", merchantInfo);
    		else
    			init = 1;
    		break;
    	}

    	case TAG_BF02_CBK_AMOUNTCONF: {				// Callback for possible Amount confirmation during the transaction if not done in combination with PIN entry
    		unsigned char tucAmount[6];
    	//	char szFormattedAmount[14];

    		dmsg("----------> CALLBACK AMOUNT CONFIRMATION");
    		if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), tucAmount, sizeof(tucAmount)) == sizeof(tucAmount))
    		{
    			//inFormatAmountForDisplay(tucAmount, szFormattedAmount, sizeof(szFormattedAmount));
    			//sprintf((char *)szFormattedAmount,"%s %.2f",gszCurrecyCode,atof((char *)szFormattedAmount)); //Add currency and confirmation text
    			//inResult=inGUIInputAction(CONFIRMAMOUNTOPTION, "Confirm Amount", szFormattedAmount, NULL, 0);
    			//if(inResult == 0)
    			//	gucAmountConfirmed = TRUE;
    			vBTLVClear(&xBtlv);
    			if((node = pxBTLVAppendTag(&xBtlv, (const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0)) == NULL){
    				init = 1;
    				break;
    			}
    			if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_DF71_CBK_AMOUNTCONF), &gucAmountConfirmed, 1) == NULL){
    				init = 1;
    				break;
    			}
    		}
    		else
    			init = 1;
    		break;
    	}

    	case TAG_BF04_CBK_REDUCE_CAND: {
			  unsigned char ucSelection = 1;
			  char cAppName[EMV_ADK_MAX_AIDSUPP][17];
			  int count;
			  struct BTLVNode *x = NULL;

			  dmsg("----------> CALLBACK APPLICATION SELECTION");
			  // to read the candidate list, TAG_50_APP_LABEL has to be searched n times inside the stream
			  // ASCII, subsequently can be displayed and number 1...n includes the selection result
			  x = NULL;
			  for (count = 0;(x = pxBTLVFindNextTag(node, pcBTLVTagStr(TAG_50_APP_LABEL), x)) != NULL && count < EMV_ADK_MAX_AIDSUPP; count++){
				memset(cAppName[count], 0x00, sizeof(cAppName[count]));
				snprintf(cAppName[count], MIN(x->uSize+1,sizeof(cAppName[count])),"%s",x->pucData); // for up to 9 applications
			  }
			  vBTLVClear(&xBtlv);
			  if((node = pxBTLVAppendTag(&xBtlv,(const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0)) == NULL) {
				init = 1;
				break;
			  }
			  if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_DF75_CBK_APP_NO), &ucSelection, 1) == NULL) {
				init = 1;
				break;
			  }
			  dmsg("----------Apps Selection End");
			  break;
			}

		// Callback after Read Records to modify parameters, once we know the PAN
        case TAG_BF06_CBK_LOCAL_CHECKS: {  //Braces are needed in VOS to begin-end the case.
            unsigned char T_57_PAN[10]; // 10 hex characters = 19+1 asccii digits
            unsigned char T_9F1B_FloorLimit[4];
            unsigned char T_5F24_AppExpDate[3];
            unsigned char T_9F02_Amount[6];
            unsigned char T_9F42_Currency[2];

            dmsg("----------> CALLBACK DOMESTIC CHECK AND CHANGES");
            if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_5A_APP_PAN), T_57_PAN, sizeof(T_57_PAN)) > 0)
            {
                iBTLVExtractTag(node,pcBTLVTagStr(TAG_9F1B_TRM_FLOOR_LIMIT), T_9F1B_FloorLimit,sizeof(T_9F1B_FloorLimit));
                iBTLVExtractTag(node,pcBTLVTagStr(TAG_5F24_APP_EXP_DATE), T_5F24_AppExpDate, sizeof(T_5F24_AppExpDate));
                iBTLVExtractTag(node,pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), T_9F02_Amount,sizeof(T_9F02_Amount));
                iBTLVExtractTag(node,pcBTLVTagStr(TAG_9F42_APP_CURRENCY_CODE), T_9F42_Currency,sizeof(TAG_9F42_APP_CURRENCY_CODE));
				// More parameters can be fetched in the same way. Few tags are being fetched only for demo purposes. Refer to TAG_BF06_CBK_LOCAL_CHECKS to see all possible tags.
                vBTLVClear(&xBtlv);
                if((node = pxBTLVAppendTag(&xBtlv, (const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0)) == NULL){
                  init = 1;
                  break;
                }
                T_9F1B_FloorLimit[3]++; // Change the floorlimit value: increase 1 unit of currency
                T_9F1B_FloorLimit[3]--; // decrease 1 unit of currency
                if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_9F1B_TRM_FLOOR_LIMIT), T_9F1B_FloorLimit, sizeof(T_9F1B_FloorLimit)) == NULL){
                  init = 1;
                  break;
                }
                if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), T_9F02_Amount, sizeof(T_9F02_Amount)) == NULL){
                  init = 1;
                  break;
                }
                vdFormatCurrencyCode(T_9F42_Currency,gszCurrecyCode);
                if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_9F42_APP_CURRENCY_CODE), T_9F42_Currency, sizeof(T_9F42_Currency)) == NULL){
				  init = 1;
				  break;
				}
            }
            else
            {
                init = 1;
            }
            break;
		}
        case TAG_BF07_CBK_DCC: {					// Callback for Dynamic currency change after Read Records
        	unsigned char T_57_PAN[10]; // 19+1
            unsigned char T_9F1B_FloorLimit[4];
            unsigned char tucAmount[6];
            unsigned char T_9F42_AppCurrencyCode[2];
            unsigned char ucDccMode;

            dmsg("----------> CALLBACK DYNAMIC CURRENCY CHANGE");
            if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_5A_APP_PAN), T_57_PAN, sizeof(T_57_PAN)) > 0)
            {
            	iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F1B_TRM_FLOOR_LIMIT), T_9F1B_FloorLimit, sizeof(T_9F1B_FloorLimit));
                iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F42_APP_CURRENCY_CODE), T_9F42_AppCurrencyCode, sizeof(T_9F42_AppCurrencyCode));
                iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), tucAmount, sizeof(tucAmount));
                vBTLVClear(&xBtlv); // This Demo shows case 1: No DCC performed
                if((node = pxBTLVAppendTag(&xBtlv, (const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0)) == NULL){
                	init = 1;
                    break;
                }
                ucDccMode = MODE_DCC_NO_TRX_CONTINUE;// No DCC necessary (If DCC is necessary, change and add the parameters as mentioned above or restart the transaction with the new values)
                if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF7D_CBK_DCC_CHECK), &ucDccMode, sizeof(ucDccMode)) == NULL){
                    init = 1;
                    break;
                }
            }
            else
                init = 1;
			break;
		}
		case TAG_BF0A_CTLS_CBK_ENTER_PIN: {					// Callback for PIN entry
			unsigned char pinType, bypass, pinResult;
		//	char szPINResult[6];

			dmsg("----------> CALLBACK OFFLINE PIN ENTRY"); //PIN entry is out of the scope of this demo. A different demo will show how to handle PIN entry

			if((iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF79_CBK_PIN_INFO), &pinType, 1) == 1) && (iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF41_PIN_BYPASS), &bypass, 1) == 1))
			{
				dmsg("PIN TYPE [%d]", pinType);

				if(pinType == EMV_CT_PIN_INPUT_ONLINE)
				{
					if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_PIN, NULL, NULL) == MTI_EMV_RTN_SUSPEND)
					{
						dmsg("USER CANCEL PIN");
						//Transaction User Cancel
						pinResult = EMV_CT_PIN_INPUT_ABORT;
					}
				}

				vBTLVClear(&xBtlv);
				if((node = pxBTLVAppendTag(&xBtlv, (const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0)) == NULL){
					init = 1;
					break;
				}
				if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF79_CBK_PIN_INFO), &pinResult, 1) == NULL){
					init = 1;
					break;
				}
			}
			else
				init = 1;
			break;
		}
		//This cb is activated several times during CTLS flow with different values of leds
		case TAG_BF10_CTLS_CBK_LEDS:{
			  unsigned char ucLeds;

			  iBTLVExtractTag(node,pcBTLVTagStr(0xC8),&ucLeds, sizeof(ucLeds));
			  dmsg("---------- LED Setting: (%#.2x)", ucLeds);

			init = 1;
			break;
		}
		case TAG_BF14_CBK_TEXT_DISPLAY:{ //TODO: EMV_ADK_TXT_2_CARDS_IN_FIELD should be received when detecting 2 cards. However is not working properly even though CLTRXOP_L1_ERROR_CALLBACK is set.
			unsigned char callback_textID;
			char szDisplayMessage[21];

				memset(szDisplayMessage, 0, sizeof(szDisplayMessage));
				iBTLVExtractTag(node,pcBTLVTagStr(TAG_DF8F12_DISPLAY_TEXT),&callback_textID, sizeof(callback_textID));
				switch (callback_textID){
					case EMV_ADK_TXT_SEE_PHONE:
					case 0x6E:
						dmsg("ALAN - EMV_ADK_TXT_SEE_PHONE or 0x6E!!!");
						memcpy(szDisplayMessage, "See phone for instruction", 25);
						cMessageFlag = 0x6E;
						break;
					case EMV_ADK_TXT_RETAP_SAME:
					case EMV_ADK_TXT_RETAP_SAME_L1:
						memcpy(szDisplayMessage, "Error Reading Card", 18);
						break;
					case EMV_ADK_TXT_2_CARDS_IN_FIELD:
						dmsg("----------Callback Text Display:(%#.2x) 2 card detected in the field", callback_textID);
						memcpy(szDisplayMessage, "2 Cards Detected", 16);
						break;
					default:
						memcpy(szDisplayMessage, "**********", 10);
			  }
			  dmsg("----------Callback Text Display:(%#.2x) %s", callback_textID, szDisplayMessage);

			init = 1;
			break;
		}

		default:
			dmsg("----------Callback [%X] not defined!!!. Call back logic should be added", tag);
			init = 1;
			break;
	}
  }
  if(init == 0)
  {
	  if((*psReceiveSize = iBTLVExport(&xBtlv, pucReceive, *psReceiveSize)) <= 0){	// export TLV stream
		  init = 1; // export of message failed
	  }
  }
  vBTLVClear(&xBtlv);
  if(init != 0)
  {
	  CallbackInit_OutTLV(pucReceive, psReceiveSize);
  }
  return;
}

//-----------------------------------------------------------
// InitFramework - initialize the EMV CTLS Framework
//------------------------------------------------------------

EMV_ADK_INFO v_InitFramework(void)
{
#if 1
	unsigned long ulInitOptions = EMV_CTLS_INIT_OPT_DELETE_ALL | EMV_CTLS_INIT_OPT_CONFIG_MODE | EMV_CTLS_INIT_OPT_CALC_CHKSUM | EMV_CTLS_INIT_OPT_AUTO_RETAP | EMV_CTLS_INIT_OPT_L1_DUMP \
		| EMV_CTLS_INIT_OPT_NO_GUI_ADK | EMV_CTLS_INIT_OPT_LED_CBK_EXT;
#else
	unsigned long ulInitOptions = EMV_CTLS_INIT_OPT_DELETE_ALL | EMV_CTLS_INIT_OPT_CALC_CHKSUM | EMV_CTLS_INIT_OPT_AUTO_RETAP | EMV_CTLS_INIT_OPT_L1_DUMP \
			| EMV_CTLS_INIT_OPT_NO_GUI_ADK | EMV_CTLS_INIT_OPT_TRACE | EMV_CTLS_INIT_OPT_TRACE_CLT |
			EMV_CTLS_INIT_OPT_LED_CBK_EXT;
#endif

	void *pvExternalData = NULL;
	EMV_ADK_INFO erg;
	int numberOfAIDs = 20;
	unsigned long ulResult=0;
	unsigned char ucRes;

	erg = EMV_CTLS_Init_Framework(numberOfAIDs, emvCallback, pvExternalData, ulInitOptions,&ulResult); //Framework must be initialized before using EMV ADK APIs
	if (erg == EMV_ADK_OK)
	{
		unsigned char col_on[3] = {0, 255, 0};
		unsigned char col_off[3] = {255, 0, 0};
		dmsg("\n----------Init EMV CTLS Framework OK");

		//Only needed if not physical leds present
		ucRes=EMV_CTLS_LED_ConfigDesign(10, 10, col_off, col_on, 0, 0, 240, 30);
		dmsg("\n----------Init EMV CTLS Leds 0x%02X", ucRes);
		ucRes=EMV_CTLS_LED_SetMode(CONTACTLESS_LED_MODE_EMV);
		dmsg("\n----------Init EMV CTLS Leds Mode 0x%02X", ucRes);
		ucRes=EMV_CTLS_LED(CONTACTLESS_LED_ALL, CONTACTLESS_LED_OFF);
		dmsg("\n----------Init EMV CTLS Leds OFF 0x%02X", ucRes);

/*
		ucRes=EMV_CTLS_LED(0, CONTACTLESS_LED_IDLE_BLINK);
		dmsg("\n----------Init EMV CTLS Leds Blink 0x%02X", ucRes);
*/

	}
	else
	{
		dmsg("\n----------Init EMV CTLS framework error 0x%02X", erg);
		EMV_CTLS_Exit_Framework();
	}
	return erg;
}

static EMV_ADK_INFO v_cfgCardCTLS(tMtiMap *tpAidParamMap)
{
	INT iLen;
	UCHAR *ucpData;
	EMV_CTLS_APPLI_TYPE aid;
	EMV_CTLS_APPLIDATA_TYPE aidData;
	UCHAR *ucpMapEmvData = NULL;
	INT iMapEmvDataLen = 0;

	mtiMemset(&aid, 0x00, sizeof(EMV_CTLS_APPLI_TYPE));
	mtiMemset(&aidData, 0x00, sizeof(EMV_CTLS_APPLIDATA_TYPE));

	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_AID_TERMINAL, &iLen);

	if(ucpData && iLen > 0)
	{
		aid.aidlen = (UCHAR) iLen;
		memcpy(aid.AID, ucpData, iLen);
		dmsg("AID LEN[%d]", aid.aidlen);
		dbuf("AID", ucpData, iLen);
	}
	// common


	/*
	//MTI_EMV_TAG_INT_MAX_TARGET_PERC_BIASED_RAND_SEL
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_MAX_TARGET_PERC_BIASED_RAND_SEL, &iLen);
	memcpy(&aidData.MaxTargetPercentage, ucpData, iLen);
	aidData.Info_Included_Data[6]|=INPUT_CTLS_APL_MAX_TARGET_PERCENTAGE;

	//MTI_EMV_TAG_INT_TARGET_PERC_RAND_SEL
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TARGET_PERC_RAND_SEL, &iLen);
	memcpy(&aidData.TargetPercentage, ucpData, iLen);
	aidData.Info_Included_Data[6]|=INPUT_CTLS_APL_TARGET_PERCENTAGE;
	 */
	//MTI_EMV_TAG_TERMINAL_FLOOR_LIMIT

	//MTI_EMV_TAG_INT_THRESHOLD_VALUE_BIASED_RAND_SEL
	//memset(aidData.ThresholdBCD, 0x00, 4);
	//aidData.Info_Included_Data[6]|=INPUT_CTLS_APL_THRESHOLD_BCD;

	// paypass
	if(!memcmp(aid.AID,"\xA0\x00\x00\x00\x04",5))
	{
	    aidData.CL_Modes = CL_MC_PAYPASS_CHIP;
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_SUPPORTED_CTLS;
		//@@WAY PARAM PAYPASS 20210127
#if 1
		ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TERMINAL_CAPABILITIES, &iLen);
		memcpy(aidData.TermCap, ucpData, iLen);  // PayPass does not allow noCVM above CMV limit
#else
		memcpy(aidData.TermCap,"\xE0\x60\x08",3);  // PayPass does not allow noCVM above CMV limit
#endif
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_TERM_CAPS;

		//MTI_RFEMV_TAG_PAYPASS_SEC_CAPABILITY
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_SEC_CAPABILITY, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen == 1)
		{
			aidData.ChipCVM_belowLimit = *ucpMapEmvData;
		}
		else
			aidData.ChipCVM_belowLimit = 0x08;

		dmsg("MTI_RFEMV_TAG_PAYPASS_SEC_CAPABILITY [0x%.2x]", aidData.ChipCVM_belowLimit);

		aidData.Info_Included_Data[4] |= INPUT_CTLS_APL_CHIP_CVM_BELOW;

		//MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen == 2)
		{
			memcpy(aidData.CHPVerNum, ucpMapEmvData, iMapEmvDataLen);
		}
		else
			memcpy(aidData.CHPVerNum,"\x00\x02",2);

		dbuf("MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM", aidData.CHPVerNum, sizeof(aidData.CHPVerNum));

		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CHPVERSION;

		//@@CACING PENTEST strcpy((char*)aidData.AppName,"MasterCard");
		strncpy((char*)aidData.AppName,"MasterCard",10);
	    aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_NAME;

		aidData.App_FlowCap[0] = 0x00;
		aidData.App_FlowCap[1] = 0x00;
		aidData.App_FlowCap[2] = CTLS_CHECK_INCONS_TRACK2_PAN;
#if 0
		aidData.App_FlowCap[3] = CTLS_DISABLE_MOBILE_CVM;
#else
		aidData.App_FlowCap[3] = 0x00;
#endif

		aidData.App_FlowCap[4] = 0x00;
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_FLOW_CAPS;	//Availability bit

		//MTI_RFEMV_TAG_PAYPASS_CVM_REQ_LIMIT
#if 0
		memcpy(aidData.CL_CVM_Soft_Limit,"\xFF\xFF\xFF\xFF",4);
		dbuf("MTI_RFEMV_TAG_PAYPASS_CVM_REQ_LIMIT", aidData.CL_CVM_Soft_Limit, sizeof(aidData.CL_CVM_Soft_Limit));
#else
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_CVM_REQ_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			if(val > 0)
				memcpy( aidData.CL_CVM_Soft_Limit, &val, sizeof(val));
			else
				memcpy(aidData.CL_CVM_Soft_Limit,"\xFF\xFF\xFF\xFF",4);

			dbuf("MTI_RFEMV_TAG_PAYPASS_CVM_REQ_LIMIT", aidData.CL_CVM_Soft_Limit,
					sizeof(aidData.CL_CVM_Soft_Limit));
		}
		else
		{
			//memcpy( aidData.CL_CVM_Soft_Limit,"\x00\x00\x27\x10",4);  // PASS Disable, Required by Cert. -> 10000
			memcpy(aidData.CL_CVM_Soft_Limit,"\xFF\xFF\xFF\xFF",4);
		}
#endif
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CVM_LIMIT;

		//MTI_RFEMV_TAG_PAYPASS_TRX_NODCV_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_TRX_NODCV_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.CL_Ceiling_Limit, &val, sizeof(val));
		}
		else
		{
			//memcpy(aidData.CL_Ceiling_Limit,"\x00\x00\x4E\x20",4);  // 20000
			memcpy(aidData.CL_Ceiling_Limit,"\xFF\xFF\xFF\xFF",4);
		}
		dbuf("MTI_RFEMV_TAG_PAYPASS_TRX_NODCV_LIMIT", aidData.CL_Ceiling_Limit, sizeof(aidData.CL_Ceiling_Limit));

		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CEILING_LIMIT;

		//MTI_RFEMV_TAG_PAYPASS_TRX_DCV_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_TRX_DCV_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.CL_Ceiling_LimitMobile, &val, sizeof(val));
		}
		else
		{
			//memcpy(aidData.CL_Ceiling_LimitMobile,"\x00\x00\x4E\x20",4);  // 20000
			memcpy(aidData.CL_Ceiling_LimitMobile,"\xFF\xFF\xFF\xFF",4);
		}
		dbuf("MTI_RFEMV_TAG_PAYPASS_TRX_DCV_LIMIT", aidData.CL_Ceiling_LimitMobile, sizeof(aidData.CL_Ceiling_LimitMobile));

		aidData.Info_Included_Data[4] |= INPUT_CTLS_APL_CEIL_LIMIT_MOBILE;

		//MTI_RFEMV_TAG_PAYPASS_FLOOR_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_FLOOR_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			if(val > 0)
				memcpy(aidData.FloorLimit, &val, sizeof(val));
			else
				memcpy(aidData.FloorLimit,"\xFF\xFF\xFF\xFF",4);
			dbuf("MTI_RFEMV_TAG_PAYPASS_FLOOR_LIMIT", aidData.FloorLimit,
					sizeof(aidData.FloorLimit));
		}
		else
		{
			//memcpy(aidData.FloorLimit,"\x00\x00\x00\x00",4);  // Disable
			memcpy(aidData.FloorLimit,"\xFF\xFF\xFF\xFF",4);  // Disable
		}

		dbuf("MTI_RFEMV_TAG_PAYPASS_FLOOR_LIMIT", aidData.FloorLimit, sizeof(aidData.FloorLimit));

		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_FLOOR_LIMIT;

		//TAC - Denial, MTI_RFEMV_TAG_PAYPASS_TAC_DENIAL
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_TAC_DENIAL, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			memcpy(aidData.TACDenial, ucpMapEmvData, iMapEmvDataLen);
		}
		else
			memcpy(aidData.TACDenial,"\x00\x00\x00\x00\x00",5);

		dbuf("MTI_RFEMV_TAG_PAYPASS_TAC_DENIAL", aidData.TACDenial, sizeof(aidData.TACDenial));

		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_TAC_DENIAL;

		//TAC - Default, MTI_RFEMV_TAG_PAYPASS_TAC_DEFAULT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_TAC_DEFAULT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			memcpy(aidData.TACDefault, ucpMapEmvData, iMapEmvDataLen);
		}
		else
			memcpy(aidData.TACDefault,"\xFC\x50\x9C\x88\x00",5);

		dbuf("MTI_RFEMV_TAG_PAYPASS_TAC_DEFAULT", aidData.TACDefault, sizeof(aidData.TACDefault));
		
		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_TAC_DEFAULT;

		//TAC - Online, MTI_RFEMV_TAG_PAYPASS_TAC_ONLINE
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_RFEMV_TAG_PAYPASS_TAC_ONLINE, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			memcpy(aidData.TACOnline, ucpMapEmvData, iMapEmvDataLen);
		}
		else
			memcpy(aidData.TACOnline,"\xFC\x50\x9C\x88\x00",5);

		dbuf("MTI_RFEMV_TAG_PAYPASS_TAC_ONLINE", aidData.TACOnline, sizeof(aidData.TACOnline));

		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_TAC_ONLINE;

		//ADDTIONAL TAG
			//9F1D, MTI_EMV_TAG_TERMINAL_RISK_MANAGEMENT_DATA
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYPASS_PARAM, MTI_EMV_TAG_TERMINAL_RISK_MANAGEMENT_DATA, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			memcpy(aidData.Additional_Tags_TRM, "\x9F\x1D\x08", 3);
			memcpy(aidData.Additional_Tags_TRM + 3, ucpMapEmvData, iMapEmvDataLen);
		}
		else
			memcpy(aidData.Additional_Tags_TRM,"\x9F\x1D\x08\x6C\xFC\x80\x00\x00\x00\x00\x00", 11);

		dbuf("MTI_EMV_TAG_TERMINAL_RISK_MANAGEMENT_DATA", aidData.Additional_Tags_TRM, 3 + iMapEmvDataLen);

		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_ADD_TAGS ;

	}
	// paywave
	else if(!memcmp(aid.AID,"\xA0\x00\x00\x00\x03",5))
	{
		aidData.CL_Modes = CL_VISA_CHIP;
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_SUPPORTED_CTLS;

		//MTI_RFEMV_TAG_PAYWAVE_CVM_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYWAVE_PARAM, MTI_RFEMV_TAG_PAYWAVE_CVM_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.CL_CVM_Soft_Limit, &val, sizeof(val));
		}
		else
		{
			//memcpy( aidData.CL_CVM_Soft_Limit,"\x00\x00\x4E\x20",4);  // 200 RP
			memcpy(aidData.CL_CVM_Soft_Limit,"\xFF\xFF\xFF\xFF",4);
		}
		dbuf("MTI_RFEMV_TAG_PAYWAVE_CVM_LIMIT", aidData.CL_CVM_Soft_Limit, sizeof(aidData.CL_CVM_Soft_Limit));

		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CVM_LIMIT;

		//TRANSACTION LIMIT (VISA NOT USE THIS)
		//@@WAY JCB Cless
#if 1
		//MTI_RFEMV_TAG_PAYWAVE_TRX_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYWAVE_PARAM, MTI_RFEMV_TAG_PAYWAVE_TRX_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.CL_Ceiling_Limit, &val, sizeof(val));
		}
		else
		{
			//memcpy( aidData.CL_CVM_Soft_Limit,"\x00\x00\x4E\x20",4);  // 200 RP
			memcpy(aidData.CL_CVM_Soft_Limit,"\xFF\xFF\xFF\xFF",4);
		}
		dbuf("MTI_RFEMV_TAG_PAYWAVE_TRX_LIMIT", aidData.CL_Ceiling_Limit, sizeof(aidData.CL_Ceiling_Limit));

#else
		memcpy(aidData.CL_Ceiling_Limit,"\xFF\xFF\xFF\xFF",4);  // Disable
#endif
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CEILING_LIMIT;

		//MTI_RFEMV_TAG_PAYWAVE_FLOOR_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYWAVE_PARAM, MTI_RFEMV_TAG_PAYWAVE_FLOOR_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.FloorLimit, &val, sizeof(val));
		}
		else
			memcpy(aidData.FloorLimit,"\xFF\xFF\xFF\xFF",4);  // Disable

		dbuf("MTI_RFEMV_TAG_PAYWAVE_FLOOR_LIMIT", aidData.FloorLimit, sizeof(aidData.FloorLimit));

		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_FLOOR_LIMIT;

		aidData.RetapFieldOff = 0x05;
		aidData.Info_Included_Data[6] |= INPUT_CTLS_APL_RETAP_FIELD_OFF;


#if 0
		// Dynamic reader limits for VpTP 3.1.0
		aidData.VisaDRLParams[0].Index = 1;
		memcpy(aidData.VisaDRLParams[0].Floorlimit,"\x00\x00\x0B\xB9",4);  // 30.01
		aidData.VisaDRLParams[0].ucAppPrgIdLen = 5;
		memcpy(aidData.VisaDRLParams[0].Application_PRG_ID,"\x31\x02\x68\x26\x20",aidData.VisaDRLParams[0].ucAppPrgIdLen);
		memcpy(aidData.VisaDRLParams[0].TXNlimit,"\x99\x99\x99\x99\x99\x99",6);
		memcpy(aidData.VisaDRLParams[0].CVMlimit,"\x00\x00\x00\x00\x30\x01",6);
		aidData.VisaDRLParams[1].Index = 2;
		memcpy(aidData.VisaDRLParams[1].Floorlimit,"\x00\x00\x03\xE9",4);  // 10.01
		aidData.VisaDRLParams[1].ucAppPrgIdLen = 8;
		memcpy(aidData.VisaDRLParams[1].Application_PRG_ID,"\x31\x02\x68\x26\x12\x00\x00\x03",aidData.VisaDRLParams[1].ucAppPrgIdLen);
		memcpy(aidData.VisaDRLParams[1].TXNlimit,"\x99\x99\x99\x99\x99\x99",6);
		memcpy(aidData.VisaDRLParams[1].CVMlimit,"\x00\x00\x00\x00\x15\x01",6);
		aidData.VisaDRLParams[2].Index = 3;
		memcpy(aidData.VisaDRLParams[2].Floorlimit,"\x00\x00\x09\xC5",4);  // 25.01
		aidData.VisaDRLParams[2].ucAppPrgIdLen = 5;
		memcpy(aidData.VisaDRLParams[2].Application_PRG_ID,"\x31\x02\x68\x26\x12",aidData.VisaDRLParams[2].ucAppPrgIdLen);
		memcpy(aidData.VisaDRLParams[2].TXNlimit,"\x99\x99\x99\x99\x99\x99",6);
		memcpy(aidData.VisaDRLParams[2].CVMlimit,"\x00\x00\x00\x00\x15\x01",6);
		aidData.VisaDRLParams[3].Index = 4;
		memcpy(aidData.VisaDRLParams[3].Floorlimit,"\x00\x00\x05\xDD",4);  // 15.01
		aidData.VisaDRLParams[3].ucAppPrgIdLen = 5;
		memcpy(aidData.VisaDRLParams[3].Application_PRG_ID,"\x31\x02\x68\x26\x00",aidData.VisaDRLParams[3].ucAppPrgIdLen);
		memcpy(aidData.VisaDRLParams[3].TXNlimit,"\x99\x99\x99\x99\x99\x99",6);
		memcpy(aidData.VisaDRLParams[3].CVMlimit,"\x00\x00\x00\x00\x20\x01",6);
		aidData.Info_Included_Data[3] |= INPUT_CTLS_APL_VISA_DRL;
#endif
		//MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYWAVE_PARAM, MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen == 2)
		{
			memcpy(aidData.CHPVerNum, ucpMapEmvData, iMapEmvDataLen);
			//dbuf("MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM", aidData.CHPVerNum, sizeof(aidData.CHPVerNum));
		}
		else
			memcpy(aidData.CHPVerNum,"\x00\x8C",2);
		
		dbuf("MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM", aidData.CHPVerNum, sizeof(aidData.CHPVerNum));
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CHPVERSION;

		//@@CACING PENTEST strcpy((char*)aidData.AppName,"Visa");
		strncpy((char*)aidData.AppName,"Visa",4);
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_NAME;

		//TTQ, MTI_EMV_TAG_TTQ
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_PAYWAVE_PARAM, MTI_EMV_TAG_TTQ, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			memcpy(aidData.TTQ, ucpMapEmvData, iMapEmvDataLen);
			//dbuf("MTI_EMV_TAG_TTQ", aidData.TTQ, sizeof(aidData.TTQ));
		}
		else
			memcpy(aidData.TTQ,"\x36\x00\x40\x00",4);

		dbuf("MTI_EMV_TAG_TTQ", aidData.TTQ, sizeof(aidData.TTQ));

		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_TTQ;

		//memcpy(aidData.App_FlowCap,"\x04\x1C\x00\x00\x00",5);			//B3b6 activates amount confirmation for cards with CVM signature active
		aidData.App_FlowCap[0] = CTLS_STATUS_CHECK_ENABLE;
		aidData.App_FlowCap[1] = 0x00;
		aidData.App_FlowCap[2] = CTLS_CHECK_INCONS_TRACK2_PAN;  //@@WAY JCB Cless | CTLS_SKIP_TXN_LIMIT_CHECK;
		aidData.App_FlowCap[3] = 0x00;
		aidData.App_FlowCap[4] = 0x00;
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_FLOW_CAPS;	//Availability bit
	}
	//@@WAY JCB Cless
	else if(!memcmp(aid.AID, "\xA0\x00\x00\x00\x65", 5))
	{
		aidData.CL_Modes = CL_JCB;
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_SUPPORTED_CTLS;		

		memcpy(aidData.CHPVerNum,"\x00\x05",2);
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CHPVERSION;

		strcpy((char*)aidData.AppName,"JCB");
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_NAME;

		//MTI_RFEMV_TAG_JSPEEDY_CVM_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_JSPEEDY_PARAM, MTI_RFEMV_TAG_JSPEEDY_CVM_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.CL_CVM_Soft_Limit, &val, sizeof(val));
		}
		else
		{
			memcpy(aidData.CL_CVM_Soft_Limit,"\xFF\xFF\xFF\xFF",4);
		}
		dbuf("MTI_RFEMV_TAG_JSPEEDY_CVM_LIMIT", aidData.CL_CVM_Soft_Limit, sizeof(aidData.CL_CVM_Soft_Limit));

		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CVM_LIMIT;

		//MTI_RFEMV_TAG_JSPEEDY_TRX_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_JSPEEDY_PARAM, MTI_RFEMV_TAG_JSPEEDY_TRX_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.CL_Ceiling_Limit, &val, sizeof(val));
		}
		else
		{
			memcpy(aidData.CL_CVM_Soft_Limit,"\xFF\xFF\xFF\xFF",4);
		}
		dbuf("MTI_RFEMV_TAG_JSPEEDY_TRX_LIMIT", aidData.CL_Ceiling_Limit, sizeof(aidData.CL_Ceiling_Limit));
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_CEILING_LIMIT;

		//MTI_RFEMV_TAG_JSPEEDY_FLOOR_LIMIT
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(MAP_JSPEEDY_PARAM, MTI_RFEMV_TAG_JSPEEDY_FLOOR_LIMIT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL && iMapEmvDataLen > 0)
		{
			INT val = 0;
			mtiBcdToInt(ucpMapEmvData, &val, iMapEmvDataLen);
			memcpy(aidData.FloorLimit, &val, sizeof(val));
		}
		else
			memcpy(aidData.FloorLimit,"\xFF\xFF\xFF\xFF",4);  // Disable

		dbuf("MTI_RFEMV_TAG_JSPEEDY_FLOOR_LIMIT", aidData.FloorLimit, sizeof(aidData.FloorLimit));
		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_FLOOR_LIMIT;

		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TAC_ONLINE, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL)
		{
			dbuf("MTI_EMV_TAG_INT_TAC_ONLINE", ucpMapEmvData, iMapEmvDataLen);
			memcpy(aidData.TACOnline, ucpMapEmvData, iMapEmvDataLen);
		}
		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_TAC_ONLINE;
		
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TAC_DEFAULT, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL)
		{
			dbuf("MTI_EMV_TAG_INT_TAC_DEFAULT", ucpMapEmvData, iMapEmvDataLen);
			memcpy(aidData.TACDefault, ucpMapEmvData, iMapEmvDataLen);			
		}
		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_TAC_DEFAULT;
		
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TAC_DENIAL, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL)
		{
			dbuf("MTI_EMV_TAG_INT_TAC_DENIAL", ucpMapEmvData, iMapEmvDataLen);
			memcpy(aidData.TACDenial, ucpMapEmvData, iMapEmvDataLen);
		}
		aidData.Info_Included_Data[1] |= INPUT_CTLS_APL_TAC_DENIAL;

		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TERMINAL_CAPABILITIES, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL)
		{
			dbuf("MTI_EMV_TAG_TERMINAL_CAPABILITIES", ucpMapEmvData, iMapEmvDataLen);
			memcpy(aidData.TermCap, ucpMapEmvData, iMapEmvDataLen);
		}
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_TERM_CAPS;
		
		ucpMapEmvData = NULL;
		iMapEmvDataLen = 0;
		ucpMapEmvData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES, &iMapEmvDataLen);
		if(ucpMapEmvData != NULL)
		{
			dbuf("MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES", ucpMapEmvData, iMapEmvDataLen);
			memcpy(aidData.TermAddCap, ucpMapEmvData, iMapEmvDataLen);
		}
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_ADD_TERM_CAPS;

		ucpData = NULL;
		iLen = 0;
		ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TERMINAL_CAPABILITIES, &iLen);
		memcpy(aidData.TermCap, ucpData, iLen);
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_TERM_CAPS;

		ucpData = NULL;
		iLen = 0;
		ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES, &iLen);
		memcpy(aidData.TermCap, ucpData, iLen);
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_ADD_TERM_CAPS;

		aidData.App_FlowCap[0] = 0x00;
		aidData.App_FlowCap[1] = 0x00;
		aidData.App_FlowCap[2] = CTLS_CHECK_INCONS_TRACK2_PAN;
		aidData.App_FlowCap[3] = 0x00;
		aidData.App_FlowCap[4] = 0x00;
		aidData.Info_Included_Data[2] |= INPUT_CTLS_APL_FLOW_CAPS;

	}
	//
	else
	{
		dmsg("THIS IS NOT RF EMV AID");
		return EMV_ADK_NOT_ALLOWED;
	}

#if 0
	//MTI_EMV_TAG_INT_TAC_DENIAL
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TAC_DENIAL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TACDenial, ucpData, iLen);
		aidData.Info_Included_Data[1]|=INPUT_CTLS_APL_TAC_DENIAL;
	}

	//MTI_EMV_TAG_INT_TAC_ONLINE
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TAC_ONLINE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TACOnline, ucpData, iLen);
		aidData.Info_Included_Data[1]|=INPUT_CTLS_APL_TAC_ONLINE;
	}

	//MTI_EMV_TAG_INT_TAC_DEFAULT
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TAC_DEFAULT, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TACDefault, ucpData, iLen);
		aidData.Info_Included_Data[1]|=INPUT_CTLS_APL_TAC_DEFAULT;
	}

	//MTI_EMV_TAG_TERMINAL_CAPABILITIES
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TERMINAL_CAPABILITIES, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TermCap, ucpData, iLen);
		aidData.Info_Included_Data[2]|=INPUT_CTLS_APL_TERM_CAPS;
	}

	//MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TermAddCap, ucpData, iLen);
		aidData.Info_Included_Data[2]|=INPUT_CTLS_APL_ADD_TERM_CAPS;
	}

	//MTI_EMV_TAG_APPLI_CURRENCY_CODE
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TERMINAL_COUNTRY_CODE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.App_CountryCodeTerm, ucpData, iLen);
		aidData.Info_Included_Data[2]|=INPUT_CTLS_APL_COUNTRY_CODE;
	}

	//MTI_EMV_TAG_INT_THRESHOLD_VALUE_BIASED_RAND_SEL
	/*
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_THRESHOLD_VALUE_BIASED_RAND_SEL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.Threshhold, ucpData, iLen);
		aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_THRESH;
	}
	*/

	//MTI_EMV_TAG_INT_TARGET_PERC_RAND_SEL
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TARGET_PERC_RAND_SEL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(&aidData.TargetPercentage, ucpData, iLen);
		aidData.Info_Included_Data[6]|=INPUT_CTLS_APL_TARGET_PERCENTAGE;
	}

	//MTI_EMV_TAG_INT_MAX_TARGET_PERC_BIASED_RAND_SEL
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_MAX_TARGET_PERC_BIASED_RAND_SEL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(&aidData.MaxTargetPercentage, ucpData, iLen);
		aidData.Info_Included_Data[6]|=INPUT_CTLS_APL_MAX_TARGET_PERCENTAGE;
	}

	//CAT CODE
	ucpData = NULL;
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_MERCHANT_CATEGORY_CODE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.BrKey, ucpData, iLen);
		aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_MERCHANT_CATCODE;
	}
#endif

	return EMV_CTLS_SetAppliData(EMV_ADK_SET_ONE_RECORD, &aid, &aidData);

}

static tMtiMap g_tMapCapkCache;
static tMtiMap *g_tpMapCapkCache = &g_tMapCapkCache;
static UCHAR g_ucCacheInit = FALSE;

static EMV_ADK_INFO storeCAPKey_CTLS(UCHAR *ucpRID)
{
	EMV_ADK_INFO emvRet = EMV_ADK_OK;
	INT iIdx, iRet = 0, iKeyIdxListNum = 0;
	UCHAR ucaRID[5] = {0,};
	UCHAR ucpKeyIndexList[100];
	tMtiMap tMapCAPKey;
	tMtiMap *tpMapCAPKey = &tMapCAPKey;
	INT iCount = 1;
	INT iLen;
	UCHAR *ucCAP;

	if (g_ucCacheInit == FALSE)
	{
		mtiMapInit(g_tpMapCapkCache);
		mtiMapInit(tpMapCAPKey);

		//Get RID 5 Byte
		memcpy(ucaRID, ucpRID, 5);	//Only Use 5 Bytes RID Value in VERIFONE EMV

		mtiMapPutBytes(tpMapCAPKey, MTI_EMV_TAG_AID_TERMINAL, ucpRID, 7);

		iGetCAPKeyIdxList(tpMapCAPKey, ucpKeyIndexList, sizeof(ucpKeyIndexList), &iKeyIdxListNum);
		//dbuf(NULL, ucpKeyIndexList, sizeof(ucpKeyIndexList));

		for(iIdx = 0; iIdx < iKeyIdxListNum; iIdx++)
		{
			EMV_CTLS_CAPKEY_TYPE thisKey={0};
			UCHAR *ucpKeyData = NULL, *ucpExponent = NULL;
			INT iKeyLen = 0, iExpLen = 0;


			mtiMapPutBytes(tpMapCAPKey, MTI_EMV_TAG_CA_PUBLIC_KEY_INDEX_CARD, &ucpKeyIndexList[iIdx], sizeof(UCHAR));

			iRet = mtiGetRfEmvParmInfo()->FuncInputCAPKCallback(tpMapCAPKey);
			//dmsg("FuncInputCAPKCallback iRet[%d]", iRet);
			if(iRet)
			{
				ucpExponent = mtiMapGetBytes(tpMapCAPKey, MTI_EMV_TAG_INT_CAPK_EXPONENT, &iExpLen);
				ucpKeyData = mtiMapGetBytes(tpMapCAPKey, MTI_EMV_TAG_INT_CAPK_MODULUS, &iKeyLen);

				//dmsg("ucpKeyData hex dump");
				//dbuf(NULL, ucpKeyData, iKeyLen);

				if(ucpKeyData == NULL || ucpExponent == NULL)
				{
					dmsg("%s - tpMapCAPKey is NULL!", __func__);
					return EMV_ADK_ABORT;
				}

				thisKey.RID                = ucaRID;
				thisKey.Index              = ucpKeyIndexList[iIdx];
				thisKey.Key                = ucpKeyData;
				thisKey.KeyLen             = iKeyLen;
				thisKey.Exponent           = ucpExponent[0];
				thisKey.Hash               = NULL;
				thisKey.noOfRevocEntries   = 0;
				thisKey.RevocEntries       = NULL;

				mtiMapPutBytes(g_tpMapCapkCache, iCount++, (UCHAR*)&thisKey, sizeof(EMV_CTLS_CAPKEY_TYPE));
				emvRet = EMV_CTLS_StoreCAPKey(EMV_ADK_SET_ONE_RECORD, &thisKey);
				dbuf("EMV_CTLS_StoreCAPKey", (UCHAR*)&thisKey, sizeof(EMV_CTLS_CAPKEY_TYPE));
				dmsg("Set CTLS Set CAP Keys Data no. %u: (%#.2x)\n", iIdx, emvRet);
				if (emvRet != EMV_ADK_OK)
					break;
			}

			if(tpMapCAPKey != NULL)
			{
				mtiMapClear(tpMapCAPKey);
				mtiMapPutBytes(tpMapCAPKey, MTI_EMV_TAG_AID_TERMINAL, ucpRID, 7);
			}
		}

		g_ucCacheInit = TRUE;
	}
	else
	{
		EMV_CTLS_CAPKEY_TYPE thisKey={0};

		for(iIdx = 0; iIdx < iCount; iIdx++)
		{
			iLen = 0;
			ucCAP = mtiMapGetBytes(g_tpMapCapkCache, iIdx + 1, &iLen);

			mtiMemset((UCHAR*)&thisKey, 0, sizeof(EMV_CTLS_CAPKEY_TYPE));
			mtiMemcpy((UCHAR*)&thisKey, ucCAP, iLen);

			emvRet = EMV_CTLS_StoreCAPKey(EMV_ADK_SET_ONE_RECORD, &thisKey);
			dbuf("Stored CAPK via CACHE", ucCAP, iLen);
			dmsg("[CACHE] Set CTLS Set CAP Keys Data no. %u: (%#.2x)\n", iIdx, emvRet);
			if (emvRet != EMV_ADK_OK)
				break;
		}
	}

	return emvRet;
}

// =============================================================================
EMV_ADK_INFO v_SetCTLSApplicationData(void)
{
	EMV_ADK_INFO erg=EMV_ADK_OK;
	tMtiMap tMapAidList;
	tMtiMap *tpMapAidList = &tMapAidList;

	tMtiMap tMapAidParam;
	tMtiMap *tpMapAidParam = &tMapAidParam;

	INT iRet = 0, i = 0, iKey = 0, iLen = 0;
	UCHAR *ucpValue = NULL;

	// Clear AID
	erg = EMV_CTLS_SetAppliData(EMV_ADK_CLEAR_ALL_RECORDS, NULL, NULL);
	if (erg != EMV_ADK_OK)
	{
		dmsg("----------Clear CTLS Appl Data error: (%#.2x)", erg);
		return erg;
	}

	mtiMapInit(tpMapAidList);
	mtiMapInit(tpMapAidParam);

	iRet = mtiGetRfEmvParmInfo()->FuncInputAIDListCallback(tpMapAidList);

	if (iRet)
	{
		INT isCleared = FALSE;
		// AID Copy
		for (i = 0; i < tpMapAidList->iMapCount; i++)
		{
			iKey = MemCheck(tpMapAidList, i);
			iLen = 0;
			ucpValue = mtiMapGetBytes(tpMapAidList, iKey, &iLen);

			if (iLen > 0)
			{
				//Application Data Set
				mtiMapPutBytes(tpMapAidParam, MTI_EMV_TAG_AID_TERMINAL, ucpValue, iLen);

				iRet = mtiGetRfEmvParmInfo()->FuncInputAIDParamCallback(tpMapAidParam);

				if(iRet)
				{
					erg = v_cfgCardCTLS(tpMapAidParam);
					dmsg("Set CTLS AID Data IDX[%d] ErrorCode[0x%.2x]", i, erg);
				}

				if(isCleared == FALSE)	//Only Working Once...
				{
					// Clear CAP keys
					erg = EMV_CTLS_StoreCAPKey(EMV_ADK_CLEAR_ALL_RECORDS, NULL);
					dmsg("Set CTLS Clear CAP Keys Data: (%#.2x)\n", erg);
					isCleared = TRUE;
				}

				if(erg == EMV_ADK_OK)
				{
					//CAP Key Store
					storeCAPKey_CTLS(ucpValue);
				}
				if(erg == EMV_ADK_NOT_ALLOWED)
					erg = EMV_ADK_OK;
			}
			mtiMapClear(tpMapAidParam);

		}
	}

	mtiMapClear(tpMapAidList);
	dmsg("----------Set CT Application Data: OK");
	return erg;
}

//==============================================================================================================================
//Information change/added here will be updated in the EMV_Terminal.xml configuration file.
EMV_ADK_INFO v_SetCTLSTerminalData(void)
	{
	tMtiMap tMapParam;
	tMtiMap *tpMapParam = &tMapParam;

	EMV_ADK_INFO erg = EMV_ADK_OK;
	EMV_CTLS_TERMDATA_TYPE termData = {0};

	UCHAR *ucpData;
	INT iRet = 0, iLen;

	mtiMapInit(tpMapParam);
	iRet = mtiGetRfEmvParmInfo()->FuncInputGlobalParamCallback(tpMapParam);
	if(iRet == NULL)
	{
		dmsg("tpMapParam mtiGetRfEmvParmInfo is failed...");
		mtiMapClear(tpMapParam);
		return EMV_ADK_ABORT;
	}

	termData.CL_Modes_Supported = CL_VISA_COMBINED | CL_MC_PAYPASS_COMBINED | CL_JCB; //@@WAY JCB Cless //Supported contactless modes
	termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_SUPPORTED_CTLS; //Availability bit. Indicates that termData.CL_Modes_Supported is set

	termData.TermTyp = EMV_ADK_TT_ATTENDED_ONL_ONLY; //Terminal type: attended terminal, online capability
	termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_TYPE;

	//Country Code

	//MTI_EMV_TAG_INT_CURRENCY_CODE

	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TERMINAL_COUNTRY_CODE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		dbuf("MTI_EMV_TAG_TERMINAL_COUNTRY_CODE", ucpData, iLen);
		memcpy(termData.CountryCodeTerm, ucpData, iLen);
		termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_COUNTRY_CODE;
	}

	//MTI_EMV_TAG_INT_CURRENCY_CODE
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_INT_CURRENCY_CODE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		dbuf("MTI_EMV_TAG_INT_CURRENCY_CODE", ucpData, iLen);
		memcpy(termData.CurrencyTrans, ucpData, iLen);
		termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_CURRENCY;
	}

	//Currency exponent
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TRANSACTION_CURRENCY_EXPONENT, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		dbuf("MTI_EMV_TAG_TRANSACTION_CURRENCY_EXPONENT", ucpData, iLen);
		//termData.ExpTrans = *ucpData;
		termData.ExpTrans = 0x02;
		termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_EXP_CURRENCY;
	}

	termData.SuppLang[0] = EMV_ADK_LANG_ENGLISH;
	termData.SuppLang[1] = EMV_ADK_LANG_SPANISH;
	termData.Info_Included_Data[1] |= INPUT_CTLS_TRM_LANGUAGES;

	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_IFD_SERIAL_NUMBER, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(termData.IFDSerialNumber, ucpData, iLen);
		//memcpy(termData.IFDSerialNumber,"12345678",8);
		termData.Info_Included_Data[1] |= INPUT_CTLS_TRM_IFD_SERIAL;
	}
	// no setting of FlowOptions
	termData.FlowOptions[0] = INPUT_CTLS_TRM_FLOWOPT_VISA_WAVE;
	termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_FLOW_OPTIONS;  // Force setting even if something different exists in stored xml file

	mtiMapClear(tpMapParam);

	erg = EMV_CTLS_SetTermData(&termData);
	if (erg != EMV_ADK_OK){
		dmsg("----------Set CTLS Term Data Error (%#.2x)", erg);
	   return erg;
	}

	dmsg("----------Set CTLS Term Data OK");

	return erg;
}

//-------------------------------------------------------------------------------------------
// Log Transaction Results
//-------------------------------------------------------------------------------------------
void vdLogTrxResultsCTLS(EMV_CTLS_TRANSRES_TYPE* pxTransRes)
{
    int i;
    char buf[256];

	tMtiMap *tpTagData = &g_tMtiPayWaveMap;
    tMtiMap *tpTranData = &g_tMtiTranParamMap;
	tMtiMap *tpTagDataPayPass = &g_tMtiPayPassMap; //@@WAY PARAM PAYPASS 20210127
    UCHAR *pchEMVTagData = NULL;
    INT iLength = 0;
    CHAR szTempBuff[64];
    unsigned long ulCvmRequiredLimit = 0L, ulTransactionAmount = 0L;
	CHAR szTrAmount[16], szICCAID[32];

	mtiMemset(szTrAmount, 0, sizeof(szTrAmount));
	mtiMemset(szICCAID, 0, sizeof(szICCAID));

	//@@WAY PARAM PAYPASS 20210127 moved below
#if 0	
	pchEMVTagData = mtiMapGetBytes(tpTagData, MTI_RFEMV_TAG_PAYWAVE_CVM_LIMIT, &iLength);

    if (pchEMVTagData!=NULL && 1 < iLength)
    {
        mtiMemset(szTempBuff, 0, sizeof(szTempBuff));
        mtiHtoa(szTempBuff, pchEMVTagData, iLength);
        ulCvmRequiredLimit = mtiAtoll(12, szTempBuff);

        dmsg("<<<<<CVM LIMIT>>>>> = Rp. %ld", ulCvmRequiredLimit);
    }
#endif
	//


    dmsg("\n=== Transaction Results ===\n");
    if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_CTLS_APPNAME)
    	dmsg("Application label: %s\n",pxTransRes->AppName);

    if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_CTLS_9F06_AID) {
        for(i=0; i<pxTransRes->T_9F06_AID.aidlen && i<sizeof(pxTransRes->T_9F06_AID.AID); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_9F06_AID.AID[i]);
        dmsg("Terminal AID: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_84_DFNAME) {
        for(i=0; i<pxTransRes->T_84_DFName.aidlen && i<sizeof(pxTransRes->T_84_DFName.AID); i++)
            sprintf(&szICCAID[2*i],"%02X",pxTransRes->T_84_DFName.AID[i]);
        dmsg("ICC AID: %s\n",szICCAID);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_5A_PAN) {
        for(i=0; i<sizeof(pxTransRes->T_5A_PAN); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_5A_PAN[i]);
        dmsg("PAN 5A: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_57_DATA_TRACK2) {
        for(i=0; i<pxTransRes->T_57_DataTrack2.tr2len; i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_57_DataTrack2.tr2data[i]);
        dmsg("Track2 57: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_STATUSINFO) {
    	dmsg("StatusInfo: 0x%08x\n",pxTransRes->StatusInfo);

    }

    //////////
    if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_CTLS_DF8104_MC_BALANCE) {
        for(i=0; i<sizeof(pxTransRes->T_DF8104_CL_MC_BALANCE); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_DF8104_CL_MC_BALANCE[i]);
        dmsg("MasterCard PayPass3 balance before GENAC DF8104: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_CTLS_DF8105_MC_BALANCE) {
        for(i=0; i<sizeof(pxTransRes->T_DF8105_CL_MC_BALANCE); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_DF8105_CL_MC_BALANCE[i]);
        dmsg("MasterCard PayPass3 balance after GENAC DF8105: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_CTLS_9F5D_VISA_AOSA) {
        for(i=0; i<sizeof(pxTransRes->T_9F5D_CL_VISA_AOSA); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_9F5D_CL_VISA_AOSA[i]);
        dmsg("Visa Available Offline Spending Amount AOSA 9F5D: %s\n",buf);
    }
///////////
    if(pxTransRes->T_DF61_Info_Received_Data[7] & TRX_CTLS_9F02_AMOUNT) {
        for(i=0; i<sizeof(pxTransRes->T_9F02_TXNAmount); i++)
            sprintf(&szTrAmount[2*i],"%02X",pxTransRes->T_9F02_TXNAmount[i]);
        dmsg("Transaction amount 9F02: %s\n",szTrAmount);

		ulTransactionAmount = mtiAtoll(12, szTrAmount);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_5F24_APPEXPDATE) {
        for(i=0; i<sizeof(pxTransRes->T_5F24_AppExpDate); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_5F24_AppExpDate[i]);
        dmsg("Expiry data 5F24: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_95_TVR) {
        for(i=0; i<sizeof(pxTransRes->T_95_TVR); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_95_TVR[i]);
        dmsg("TVR 95: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_82_AIP) {
        for(i=0; i<sizeof(pxTransRes->T_82_AIP); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_82_AIP[i]);
        dmsg("AIP 82: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_CTLS_9F34_CVM_RES) {
        for(i=0; i<sizeof(pxTransRes->T_9F34_CVM_Res); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_9F34_CVM_Res[i]);
        dmsg("CVM results 9F34: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_CTLS_9B_TSI) {
        for(i=0; i<sizeof(pxTransRes->T_9B_TSI); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_9B_TSI[i]);
        dmsg("TSI 9B: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_9C_TRANSTYPE) {
    	dmsg("Transaction type 9C: 0x%02X\n",pxTransRes->T_9C_TransType);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_9F39_POS_ENTRY_MODE) {
    	dmsg("POS entry mode 9F39: 0x%02X\n",pxTransRes->T_9F39_POSEntryMode);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_CTLS_5F34_PAN_SEQ_NUMBER) {
        for(i=0; i<sizeof(pxTransRes->T_5F34_PANSequenceNo); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_5F34_PANSequenceNo[i]);
        dmsg("PAN sequ. no. 5F34: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[7] & TRX_CTLS_9F03_CB_AMOUNT) {
        for(i=0; i<sizeof(pxTransRes->T_9F03_TXNAdditionalAmount); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_9F03_TXNAdditionalAmount[i]);
        dmsg("Amount other 9F03: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_9F27_CRYPTINFO) {
    	dmsg("Cryptogram info 9F27: 0x%02X\n",pxTransRes->T_9F27_CryptInfo);
    }

    ////
    if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_CTLS_DF5D_CL_MAGSTRIPE) {
        for(i=0; i<pxTransRes->T_DF5D_CL_MAGSTRIPE_T2.tr2len; i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_DF5D_CL_MAGSTRIPE_T2.tr2data[i]);
        dmsg("CL magstripe data DF5E: %s\n",buf);
    }

    if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_CTLS_9F66_TTQ) {
        for(i=0; i<sizeof(pxTransRes->T_9F66_CL_TTQ); i++)
            sprintf(&buf[2*i],"%02X",pxTransRes->T_9F66_CL_TTQ[i]);
        dmsg("Visa payWave TTQ 9F66: %s\n",buf);
    }

	if(!memcmp(szICCAID,"A000000003",10))
    {
		//@@WAY PARAM PAYPASS 20210127
		pchEMVTagData = mtiMapGetBytes(tpTagData, MTI_RFEMV_TAG_PAYWAVE_CVM_LIMIT, &iLength);
		if (pchEMVTagData!=NULL && 1 < iLength)
		{
			mtiMemset(szTempBuff, 0, sizeof(szTempBuff));
			mtiHtoa(szTempBuff, pchEMVTagData, iLength);
			ulCvmRequiredLimit = mtiAtoll(12, szTempBuff);
		
			dmsg("<<<<<CVM LIMIT>>>>> = Rp. %ld", ulCvmRequiredLimit);
		}
		//
        dmsg("[PAYWAVE]SET CVM LIMIT FLAG\n");
        if (ulCvmRequiredLimit <= ulTransactionAmount) mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
        else mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, TRUE);
    }
	//@@WAY PARAM PAYPASS 20210127
	else if(!memcmp(szICCAID,"A000000004",10))
	{
		pchEMVTagData = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_CVM_REQ_LIMIT, &iLength);
		if (pchEMVTagData!=NULL && 1 < iLength)
		{
			mtiMemset(szTempBuff, 0, sizeof(szTempBuff));
			mtiHtoa(szTempBuff, pchEMVTagData, iLength);
			ulCvmRequiredLimit = mtiAtoll(12, szTempBuff);
		
			dmsg("<<<<<CVM LIMIT>>>>> = Rp. %ld", ulCvmRequiredLimit);
		}
		
        if (ulCvmRequiredLimit < ulTransactionAmount) mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
		else mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, TRUE);
	}
	//

    dmsg("=== Transaction Results end ===\n");
}

//-------------------------------------------------------------------------------------------
// Continue Offline
//-------------------------------------------------------------------------------------------
EMV_ADK_INFO ContinueOffline(EMV_CTLS_TRANSRES_TYPE* xTransRes)
{
    EMV_ADK_INFO erg = EMV_ADK_OK;
    unsigned long reqTags[] = { 0x95, 0x6f, 0xDF8115, 0xFFFF, 0xE300, 0xff8105, 0x5A, 0x9F39, 0x5F24, 0x9F6B };
    unsigned short cntTags = sizeof(reqTags) / sizeof(reqTags[0]);
    unsigned char buf[1024];
    unsigned short datLen;
    unsigned char ret = EMV_ADK_ABORT;

	do {
		if(cMessageFlag == 0x6E)
		{
			mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_SHOW_MESSAGE,
					cpGetString(KEY_STR_CTLS_TRY_PHONE_PASS), NULL);
			cMessageFlag = 0x00;
		}
        erg = EMV_CTLS_ContinueOffline(xTransRes);


        if (erg == EMV_ADK_OK) {
            EMV_CTLS_SmartPowerOff(0);
            if (xTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_STATUSINFO)
            {
            	dmsg("CTLS card read transaction has been performed successfully");
            }
        }

        if(mtiGetKeyPress()==KEY_CANCEL)
        	return EMV_ADK_ABORT;

        mtiSleep(10);
	} while ((erg == EMV_ADK_NO_CARD) || (erg == EMV_ADK_CONTINUE)); //Replace by cts_waitselection?

	dmsg("ContinueOffline, EMV_CTLS_ContinueOffline : RESULT - [x%08X]", erg);

    switch (erg) {
        case EMV_ADK_TC:
        	dmsg("Transaction offline approved");
            break;
        case EMV_ADK_ARQC:
        	dmsg("Transaction must go online");
            break;
        case EMV_ADK_AAC:
        	dmsg("Transaction offline declined");
            break;
        default:
        	dmsg("Transaction error %02X", erg);
            break;
    }

    vdLogTrxResultsCTLS(xTransRes);

 	ret = EMV_CTLS_fetchTxnTags(EMV_ADK_FETCHTAGS_NO_EMPTY, reqTags, cntTags, buf, sizeof(buf), &datLen);
	dmsg("CTLS fetch txn tags: (%#.2x)", ret);
	if (ret == EMV_ADK_OK)
	{
		// @@EB MALLOC MODIFY
#if 1
		char* cBuf = NULL;
		cBuf = malloc(2 * datLen + 1);
#else
		char* cBuf = malloc(2 * datLen + 1);
#endif
		if (cBuf)
		{
			int i;
			for (i = 0; i < datLen; i++)
				sprintf(&cBuf[2 * i], "%02X", buf[i]);
	//		dmsg("fetched tags: %s", cBuf);
			free(cBuf);
		}
	}
	else
	{
		dmsg("Error on fetch tags: %02X", ret);
	}

    return erg;
}

static void ctlsOffLineAbout(EMV_CTLS_TRANSRES_TYPE* xTransRes)
{
	EMV_CTLS_HOST_TYPE xOnlineInput={0};
	xOnlineInput.OnlineResult = FALSE;

	EMV_CTLS_ContinueOnline(&xOnlineInput, xTransRes);
}

//-------------------------------------------------------------------------------------------
// Continue Online
//-------------------------------------------------------------------------------------------
EMV_ADK_INFO ContinueOnline(EMV_CTLS_TRANSRES_TYPE* xTransRes)
{
    EMV_ADK_INFO erg;
    EMV_CTLS_HOST_TYPE xOnlineInput={0};
    int iRet=0;
    char caResCode[3] = {0,};
    tMtiMap *tpTagMap = &mtiGetRfEmvParmInfo()->tEmvTagMap;

    dmsg("[ContinueOnline] start");

	//Store Tag Data to MAP for Online Transaction
	setOnlineTransactionTag(xTransRes);

	{
		INT i;
		UCHAR *ucpTRACK2 = NULL, *ucpExDate = NULL;
		INT iTrack2Len = 0, iExDate = 0;
		UCHAR ucaTemp[100+1] = {0,};

		ucpTRACK2 = mtiMapGetBytes(tpTagMap, MTI_EMV_TAG_TRACK_2_EQU_DATA, &iTrack2Len);
		if(ucpTRACK2 == NULL)
		{
			dmsg("ucpTRACK2 is NULL");
			return EMV_ADK_ABORT;
		}

		dmsg("iTrack2Len [%d]", iTrack2Len);
		mtiMemset(ucaTemp, 0, sizeof(ucaTemp));
		mtiHtoa(ucaTemp, ucpTRACK2, iTrack2Len);
		for (i = 0; i < 20; i++)
		{
			if (ucaTemp[i] == 'D' || ucaTemp[i] == 'F' || ucaTemp[i] == 'd' || ucaTemp[i] == 'f')
			{
				break;
			}

			if (ucaTemp[i] == NULL)
			{
				break;
			}
		}
		ucaTemp[i++] = 'D';
		ucpExDate =  mtiMapGetBytes(tpTagMap, MTI_EMV_TAG_APPLI_EXPIRATION_DATE, &iExDate);
		if(ucpExDate == NULL)
		{
			dmsg("ucpExDate is NULL");
			return EMV_ADK_ABORT;
		}
		//dmsg("iExDate [%d]", iExDate);
		mtiHtoa(&ucaTemp[i], ucpExDate, 2);
		iTrack2Len = mtiStrlen(40, ucaTemp);

		dmsg("ucaTemp[%s], iTrack2Len[%d]",ucaTemp ,iTrack2Len);

		//Confirm PAN
		if (mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_CONFIRM_PAN, ucaTemp, &iTrack2Len) != MTI_EMV_RTN_CONTINUE)
		{
			return EMV_ADK_ABORT;
		}
	}

	//ONLINE PIN CHECK
    {
		if(xTransRes->T_DF61_Info_Received_Data[0] & TRX_CTLS_STATUSINFO){
			if(xTransRes->StatusInfo & EMV_ADK_SI_ONLINE_PIN_REQUIRED)
			{
				dmsg("ONLINE PIN!!!");

				if (mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_PIN, NULL, NULL) == MTI_EMV_RTN_SUSPEND)
				{
					return EMV_ADK_ABORT;
				}

			}
		}
    }
	// TO DO
	iRet = mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_ONLINE, caResCode, NULL);

	dmsg("MTI_EMV_STEP_NEED_ONLINE iRet [%d]", iRet);

	if (iRet == MTI_EMV_ONLINE_APPROVED)
	{
		UCHAR *ucpTag;
		INT iTagLen = 0;

		xOnlineInput.OnlineResult = TRUE;
		memcpy(xOnlineInput.AuthResp, caResCode,2);
		xOnlineInput.Info_Included_Data[0] |= INPUT_CTLS_ONL_AUTH_RESP;

		//ISSUER SCRIPT
		iTagLen = 0;
		ucpTag = mtiMapGetBytes(tpTagMap, TAG_71_ISS_SCRIPT_TPLT_1, &iTagLen);
		if (iTagLen > 0)
		{
			dbuf("TAG_71_ISS_SCRIPT_TPLT_1", ucpTag, iTagLen);
			xOnlineInput.ScriptData = ucpTag;
			xOnlineInput.LenScriptData = iTagLen;
			xOnlineInput.Info_Included_Data[0] |= INPUT_CTLS_ONL_SCRIPT;
		}

		iTagLen = 0;
		ucpTag = mtiMapGetBytes(tpTagMap, TAG_72_ISS_SCRIPT_TPLT_2, &iTagLen);
		if (iTagLen > 0)
		{
			dbuf("TAG_72_ISS_SCRIPT_TPLT_2", ucpTag, iTagLen);
			xOnlineInput.ScriptData = ucpTag;
			xOnlineInput.LenScriptData = iTagLen;
			xOnlineInput.Info_Included_Data[0] |= INPUT_CTLS_ONL_SCRIPT;
		}
	}
	else if (iRet == MTI_EMV_ONLINE_DENIAL)
	{
		xOnlineInput.OnlineResult = TRUE;
		memcpy(xOnlineInput.AuthResp,"05",2);
		xOnlineInput.Info_Included_Data[0] |= INPUT_CTLS_ONL_AUTH_RESP;
	}
	else
	{
		dmsg("Can not do EMV Process...");
		xOnlineInput.OnlineResult = FALSE;
		xOnlineInput.Info_Included_Data[0] |= INPUT_CTLS_ONL_ONLINE_RESP;
		return EMV_ADK_ABORT;
	}

	xOnlineInput.Info_Included_Data[0] |= INPUT_CTLS_ONL_ONLINE_RESP;

    dmsg("----------EMV_CTLS_ContinueOnline...");
    erg = EMV_CTLS_ContinueOnline(&xOnlineInput, xTransRes);
    dmsg("CTLS continue online: (%#.2x)", erg);

    switch (erg)
    {
        case EMV_ADK_TC:
            dmsg("Transaction online approved !!");
            vdLogTrxResultsCTLS(xTransRes);
            return EMV_ADK_OK;
        default:
            dmsg("Transaction online error %02X !!", erg);
            break;
    }
    return erg;
}

EMV_ADK_INFO RfEMVTransaction(int duration)
{
	EMV_ADK_INFO erg;
	EMV_CTLS_START_TYPE TxStartInputCTLS;
	EMV_CTLS_STARTRES_TYPE xSelectResCTLS;
	EMV_CTLS_TRANSRES_TYPE CTLSTransRes;
	char szAmount[14] = {0,};
	unsigned char ucReader=0;
	tMtiMap *tMainProcMap;
	long long llAmt = 0LL;

	cMessageFlag = FALSE;

	memset (&TxStartInputCTLS,0,sizeof(EMV_CTLS_START_TYPE));
	memset(&xSelectResCTLS,0,sizeof(EMV_CTLS_STARTRES_TYPE));
	memcpy(TxStartInputCTLS.Info_Included_Data, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);

	// TO DO
	//  if (mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_AMOUNT, amountString, NULL) == MTI_EMV_RTN_CONTINUE)
	//  {
	tMainProcMap = mtiGetRfEmvParmInfo()->tpMainProcMap;

	//iAmt = mtiMapGetInt(tMainProcMap, MTI_RFEMV_KEY_AMOUNT);
	llAmt = mtiMapGetLLong(tMainProcMap, KEY_EXT_BIT_AMOUNT);

	if(llAmt < 1) vdEndTransactionCTLS("Amount < 1 Error", ucReader, TRUE);

	//sprintf(szAmount, "%lld", llAmt);
	mtiLLongToA(mtiMapGetLLong(tMainProcMap, KEY_EXT_BIT_AMOUNT)*100, szAmount);
	dmsg("----------Amount = %s", szAmount);
	vdSetCTLSTransactionData(&TxStartInputCTLS, szAmount);   //Setup CTLS transaction data

	EMV_CTLS_LED_SetMode(CONTACTLESS_LED_MODE_EMV);


	dmsg("----------Contactless technology detected!!!");

	dmsg("[*] EMV_CTLS_SetupTransaction start %lu",mtiGetTickCount());
	erg = EMV_CTLS_SetupTransaction(&TxStartInputCTLS, &xSelectResCTLS); //Setup transaction
	if (erg != EMV_ADK_OK)
	{
		dmsg("----------EMV_CTLS_SetupTransaction error: (%#.2x)", erg);
		return erg;
	}

	dmsg("[*] EMV_CTLS_SetupTransaction end : %lu",mtiGetTickCount());

	dmsg("[*] ContinueOffline start %lu",mtiGetTickCount());

	erg = ContinueOffline(&CTLSTransRes);

	dmsg("[*] ContinueOffline end [%08X] : %lu", erg, mtiGetTickCount());

	switch(erg)
	{
		case EMV_ADK_ARQC:
			dmsg("[*] ContinueOnline start %lu",mtiGetTickCount());
			erg = ContinueOnline(&CTLSTransRes); //EMV transaction (handling of host response including 2nd cryptogram)
			dmsg("[*] ContinueOnline end %02x : %lu", erg, mtiGetTickCount());
			if(erg == EMV_ADK_ABORT)
			{
				ctlsOffLineAbout(&CTLSTransRes);
			}

			if (erg!=EMV_ADK_OK)
			{
				vdEndTransactionCTLS("Transaction Error", ucReader, TRUE);
				return erg;
			}
			break;

		case EMV_ADK_TC:                  // approved offline
			dmsg("----------Tx approved offline!");
			break;

		case EMV_ADK_FALLBACK:            // perform fallback to magstripe. Add reaction to local fallback rules or other local chip
			dmsg("----------Appl selection FALLBACK!");
			vdEndTransactionCTLS("Fallback Transaction", ucReader, TRUE);
			return erg;

		case EMV_ADK_AAC:                 // Denied offline
			dmsg("----------Tx Declined!");
			if((CTLSTransRes.T_9F27_CryptInfo & EMV_ADK_CARD_REQUESTS_ADVICE) == EMV_ADK_CARD_REQUESTS_ADVICE)
			{
				// vStoreAdviceData(xTrxRec); // store the advice data if it need and it'll be sent before reconcilation
			}
			dmsg("Transaction Declined");
			SVC_WAIT(10);

		case EMV_ADK_AAR:
		case EMV_ADK_ABORT:
		case EMV_ADK_INTERNAL:
		case EMV_ADK_PARAM:
		case EMV_ADK_CARDERR:
		case EMV_ADK_CVM:
		case EMV_ADK_CARDERR_FORMAT:
		case EMV_ADK_TXN_CTLS_EMV_USE_OTHER_CARD:
		default:
			vdLogTrxResultsCTLS(&CTLSTransRes);
			vdEndTransactionCTLS("Transaction Error", ucReader, TRUE);
			return erg;
	}

	setFinalCryptogram(&CTLSTransRes);
	vdLogTrxResultsCTLS(&CTLSTransRes);
	vdEndTransactionCTLS("Transaction Approved", ucReader, FALSE);

	return erg;
}
