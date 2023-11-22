
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "apMtiCommonApp.h"
#include "libemv.h"

#define PIN_INPUT_TIMEOUT 30
#define PIN_INPUT_KEY_INTERVAL_TIMEOUT 10

#define ENTER_AMOUNT_MESSAGE "Enter Amount"
#define ENTER_PIN_MESSAGE "Enter PIN"
#define REMOVE_CARD_MESSAGE "Don't forget your card"
#define REMOVE_CARD_MESSAGE2 "Remove your card"
#define RETAP_CARD_MESSAGE "Please Tap Again"
#define APPL_SELECTION_ERROR_MESSAGE "Application Selection Error"
#define FALLBACK_MESSAGE "Fallback to MagStripe Needed"
#define USER_CANCEL_MESSAGE	"User Cancel Transaction"
#define ENTER_AMOUNT_MESSAGE "Enter Amount"
#define ENTER_PIN_MESSAGE "Enter PIN"
#define ENTER_PIN_MESSAGE_RETRY "Enter PIN !RETRY!"
#define ENTER_PIN_MESSAGE_LASTTRY "Enter PIN !!LAST TRY!!"

#define SELECTED_APP_MESSAGE "Selected Ap_plication"

#define XML_FILE_TERM	"EMV_TERMINAL.XML"
#define XML_FILE_APP	"EMV_APPLICATIONS.XML"
#define XML_FILE_KEY	"EMV_KEYS.XML"

#ifndef EMV_PIN
#define EMV_PIN 0x0A
#endif

using namespace std;

tEmvParamInfo g_tEmvParamInfo;
unsigned char gucAmountConfirmed = FALSE;
unsigned char g_bReqPINOption = FALSE;

typedef unsigned char (*guiCallbackT)(void);

typedef struct {
  unsigned char  ucPinAlgo;
  unsigned char  ucAutoEnter;
  unsigned char  ucClearAll;
  unsigned long  ulEchoChar;
  unsigned long  ulBypassKey;
  long           lTimeoutMs;
  guiCallbackT	 callback;
  char           *amount;
  char           *currency;
} guiPinParam;

static char gszCurrecyCode[3+1] = {0,};
//static char *pinDispMsg = ENTER_PIN_MESSAGE;
static void setOnlineTransactionTag(EMV_CT_TRANSRES_TYPE* pxTransRes);
static void emvCallback(unsigned char *pucSend, unsigned short sSendSize, unsigned char *pucReceive, unsigned short *psReceiveSize, void* externalData);
static void vdEndTransaction(const char *szStatusMessage, unsigned char ucReader, int inExitWithError);

#if 1
unsigned char ucInputIPP_PIN(unsigned char pintype, unsigned char bypass, unsigned char* pucUnpredNb,
		unsigned char* pucPINResultData, unsigned short KeyLength, unsigned char* pucPINPublicKey, unsigned long ulPINPublicKeyExp);
#else
unsigned char ucInputPIN(unsigned char pintype, unsigned char bypass, unsigned char* pucUnpredNb, unsigned char* pucPINResultData,
		unsigned short KeyLength, unsigned char* pucPINPublicKey, unsigned long ulPINPublicKeyExp);
#endif

static BOOL bUserCanceled = FALSE;

int ginhCrypto = 0;

EMV_ADK_INFO EMVTransaction(INT *iRet);

static VOID xmlFileDbg(CHAR *cpFileName)
{
	FILE *file;
	INT iOffset = 0, iReadSize = 0;
	CHAR *readBuff = NULL;
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
unsigned long ulInitOptions = EMV_CT_INIT_OPT_CONFIG_MODE | EMV_CT_INIT_OPT_L1_DUMP;

static EMV_ADK_INFO InitCTFramework(void)
{

	//unsigned long ulInitOptions = EMV_CT_INIT_OPT_CONFIG_MODE | EMV_CT_INIT_OPT_L1_DUMP | EMV_CT_INIT_OPT_TRACE |EMV_CT_INIT_OPT_TRACE_CLT;

	//unsigned long ulInitOptions = EMV_CT_INIT_OPT_CONFIG_MODE | EMV_CT_INIT_OPT_L1_DUMP | EMV_CT_INIT_OPT_NO_GUI_ADK;
	//unsigned long ulInitOptions = EMV_CT_INIT_OPT_CONFIG_MODE | EMV_CT_INIT_OPT_L1_DUMP | EMV_CT_INIT_OPT_NO_GUI_ADK \
			| EMV_CT_INIT_OPT_TRACE |EMV_CT_INIT_OPT_TRACE_CLT;

	void *pvExternalData = NULL;
	EMV_ADK_INFO erg;
	int numberOfAIDs = 20;

	//ulInitOptions |= (EMV_CT_INIT_OPT_TRACE | EMV_CT_INIT_OPT_TRACE_CLT); //Uncomment to enable framework trace

	//If XML files for CT do not exist this call create them with default values: EMV_Terminal.xml, EMV_Applications.xml, EMV_Keys.xml,
	//These files are in ./flash/ and are used by the framework to get configuration for processing EMV transactions.
	//pinDispMsg = ENTER_PIN_MESSAGE;

	erg = EMV_CT_Init_Framework(numberOfAIDs, emvCallback, pvExternalData, ulInitOptions); //Framework must be initialized before using EMV ADK APIs

	if (erg == EMV_ADK_OK)
	{
		EMV_CT_TERMDATA_TYPE emvCtTermData;

		mtiMemset(&emvCtTermData, 0, sizeof(emvCtTermData));
		EMV_CT_GetTermData(&emvCtTermData);

		dmsg("----------Init EMV CT Framework OK");

		dmsg("EMV Kernel ver [%s]", emvCtTermData.KernelVersion);
		dbuf("EMV Kernel ver ", emvCtTermData.KernelVersion, EMV_ADK_VERSION_ASCII_SIZE);
	}
	else
	{
		dmsg("----------Init CT EMV framework error 0x%02X", erg);
		EMV_CT_Exit_Framework();
	}
	return erg;
}

//------------------------------------------------------------------------------
// CTS Callbacks
//-----------------------------------------------------------------------------
// For debug purposes, trace callback

int inFormatAmountForDisplay(unsigned char *HexAmount, char *szFormattedAmount, unsigned int inFormattedAmountLength)
{
	unsigned char tucAmountAscii[13];

	if (inFormattedAmountLength<sizeof(tucAmountAscii))
		return -1;
	memset(szFormattedAmount,0,inFormattedAmountLength);
	sprintf((char *)tucAmountAscii,"%02X%02X%02X%02X%02X", HexAmount[0],HexAmount[1],HexAmount[2],HexAmount[3],HexAmount[4]);
	memcpy(szFormattedAmount,tucAmountAscii,12);
	return TRUE;
}

void CallbackInit_OutTLV(unsigned char *pucReceive, unsigned short *psReceiveSize)
{
  if(pucReceive != NULL && *psReceiveSize >= 2)
  {
    memcpy(pucReceive, "\xF0\x00", 2);
    *psReceiveSize = 2;
  }
}

void vdLogTrxResultsCT(EMV_CT_TRANSRES_TYPE* pxTransRes)
{
	unsigned int i;
	char buf[256];

	dmsg("\n----------CT Transaction Results");
	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_APPNAME)
		dmsg("Appl label: %s",pxTransRes->AppName);

	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_9F09_VERNUM){
	for(i=0; i<sizeof(pxTransRes->T_9F09_VerNum); i++)
	sprintf(&buf[2*i],"%02X",pxTransRes->T_9F09_VerNum[i]);
		dmsg("Appl VerNum: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_9F06_AID){
	for(i=0; i<pxTransRes->T_9F06_AID.aidlen && i<sizeof(pxTransRes->T_9F06_AID.AID); i++)
	sprintf(&buf[2*i],"%02X",pxTransRes->T_9F06_AID.AID[i]);
		dmsg("Terminal AID: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_84_DFNAME)
	{
		for(i=0; i<pxTransRes->T_84_DFName.aidlen && i<sizeof(pxTransRes->T_84_DFName.AID); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->T_84_DFName.AID[i]);
			dmsg("ICC AID: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_5A_PAN)
	{
		for(i=0; i<sizeof(pxTransRes->T_5A_PAN); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->T_5A_PAN[i]);
			dmsg("PAN 5A: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_57_DATA_TRACK2)
	{
		for(i=0; i<pxTransRes->T_57_DataTrack2.tr2len; i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->T_57_DataTrack2.tr2data[i]);
			dmsg("Track2 57: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_STATUSINFO)
		dmsg("StatusInfo: 0x%08lu",pxTransRes->StatusInfo);

	if(pxTransRes->T_DF61_Info_Received_Data[7] & TRX_9F02_AMOUNT)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F02_TXNAmount); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F02_TXNAmount[i]);

		dmsg("Txn amount 9F02: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_5F24_APPEXPDATE){
	for(i=0; i<sizeof(pxTransRes->T_5F24_AppExpDate); i++)
	sprintf(&buf[2*i],"%02X",pxTransRes->T_5F24_AppExpDate[i]);
	dmsg("Exp date 5F24: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_95_TVR){
	for(i=0; i<sizeof(pxTransRes->T_95_TVR); i++)
	sprintf(&buf[2*i],"%02X",pxTransRes->T_95_TVR[i]);
	dmsg("TVR 95: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_82_AIP){
	for(i=0; i<sizeof(pxTransRes->T_82_AIP); i++)
	sprintf(&buf[2*i],"%02X",pxTransRes->T_82_AIP[i]);
	dmsg("AIP 82: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_9F34_CVM_RES){
	for(i=0; i<sizeof(pxTransRes->T_9F34_CVM_Res); i++)
	sprintf(&buf[2*i],"%02X",pxTransRes->T_9F34_CVM_Res[i]);
	dmsg("CVM results 9F34: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_9B_TSI){
	for(i=0; i<sizeof(pxTransRes->T_9B_TSI); i++)
	sprintf(&buf[2*i],"%02X",pxTransRes->T_9B_TSI[i]);
	dmsg("TSI 9B: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_9C_TRANSTYPE)
	dmsg("Txn type 9C: 0x%02X",pxTransRes->T_9C_TransType);

	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_9F39_POS_ENTRY_MODE)
	dmsg("POS entry mode 9F39: 0x%02X",pxTransRes->T_9F39_POSEntryMode);

	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_5F34_PAN_SEQ_NUMBER)
	{
		for(i=0; i<sizeof(pxTransRes->T_5F34_PANSequenceNo); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->T_5F34_PANSequenceNo[i]);
		dmsg("PAN sequ. no. 5F34: %s",buf);
	}

	//TAC
	if(pxTransRes->T_DF61_Info_Received_Data[4] & TRX_TAC_DEFAULT)
	{
		for(i=0; i<sizeof(pxTransRes->TACDefault); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->TACDefault[i]);
		dmsg("TAC_DEFAULT DF23: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[4] & TRX_TAC_DENIAL)
	{
		for(i=0; i<sizeof(pxTransRes->TACDenial); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->TACDenial[i]);
		dmsg("TAC_DENIAL DF21: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[4] & TRX_TAC_ONLINE)
	{
		for(i=0; i<sizeof(pxTransRes->TACOnline); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->TACOnline[i]);
		dmsg("TAC_ONLINE DF22: %s",buf);
	}

	//IAC
	if(pxTransRes->T_DF61_Info_Received_Data[4] & TRX_9F0D_IAC_DEFAULT)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F0D_IACDefault); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->T_9F0D_IACDefault[i]);
		dmsg("IAC_DEFAULT 9F0D: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[4] & TRX_9F0E_IAC_DENIAL)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F0E_IACDenial); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->T_9F0E_IACDenial[i]);
		dmsg("IAC_DENIAL 9F0E: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[4] & TRX_9F0F_IAC_ONLINE)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F0F_IACOnline); i++)
		sprintf(&buf[2*i],"%02X",pxTransRes->T_9F0F_IACOnline[i]);
		dmsg("IAC_ONLINE 9F0F: %s",buf);
	}

	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_9F27_CRYPTINFO)
	dmsg("Cryptogram info 9F27: 0x%02X",pxTransRes->T_9F27_CryptInfo);

	dmsg("\n----------Transaction Results end\n");
}

static void setFinalCryptogram(EMV_CT_TRANSRES_TYPE* pxTransRes)
{
	char buf[256] = {0,};
	int i = 0;
	tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;

	//TVR
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_95_TVR){
		for(i=0; i<sizeof(pxTransRes->T_95_TVR); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_95_TVR[i]);
		dmsg("TVR 95: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TVR, pxTransRes->T_95_TVR, sizeof(pxTransRes->T_95_TVR));
	}

	//CRYPTOGRAM
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_9F26_CRYPTOGRAMM)
	{
		dmsg("Cryptogram 9F26: 0x%02X",pxTransRes->T_9F26_Cryptogramm);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLICATION_CRYPTOGRAM, pxTransRes->T_9F26_Cryptogramm,
				sizeof(pxTransRes->T_9F26_Cryptogramm));
	}
	
	// @@EB PRINT ENHANCEMENT
	// @@EB TSI
	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_9B_TSI)
	{
		for(i=0; i<sizeof(pxTransRes->T_9B_TSI); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9B_TSI[i]);
		dmsg("TSI 9B: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TSI, pxTransRes->T_9B_TSI, sizeof(pxTransRes->T_9B_TSI));
	}
	//

}

static void setOnlineTransactionTag(EMV_CT_TRANSRES_TYPE* pxTransRes)
{
	unsigned int i;
	char buf[256] = {0,};

	tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;

	dmsg("\n----------Mining TAG Data for CT Online Transaction...");

	//PAN
	//@@SIROH 2019-01-10 RELEASED FOR NSICCS TAG 5A
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_5A_PAN)
	{
		for(i=0; i<sizeof(pxTransRes->T_5A_PAN); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5A_PAN[i]);
		dmsg("PAN 5A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLI_PAN, pxTransRes->T_5A_PAN, sizeof(pxTransRes->T_5A_PAN));
	}
	//@@SIROH

	//PAN SEQ NUM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_5F34_PAN_SEQ_NUMBER)
	{
		for(i=0; i<sizeof(pxTransRes->T_5F34_PANSequenceNo); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5F34_PANSequenceNo[i]);

		dmsg("PAN sequ. no. 5F34: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLI_PAN_SEQUENCE_NUMBER, pxTransRes->T_5F34_PANSequenceNo,
				sizeof(pxTransRes->T_5F34_PANSequenceNo));
	}

	//TRACK 2
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_57_DATA_TRACK2)
	{
		for(i=0; i<pxTransRes->T_57_DataTrack2.tr2len; i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_57_DataTrack2.tr2data[i]);
		dmsg("Track2 57: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRACK_2_EQU_DATA, pxTransRes->T_57_DataTrack2.tr2data,
				pxTransRes->T_57_DataTrack2.tr2len);
	}

	//CRYPTOGRAM INFO
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_9F27_CRYPTINFO)
	{
		dmsg("Cryptogram info 9F27: 0x%02X",pxTransRes->T_9F27_CryptInfo);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_CRYPTOGRAM_INFO_DATA, &pxTransRes->T_9F27_CryptInfo, sizeof(pxTransRes->T_9F27_CryptInfo));
	}

	//CRYPTOGRAM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_9F26_CRYPTOGRAMM)
	{
		dmsg("Cryptogram 9F26: 0x%02X",pxTransRes->T_9F26_Cryptogramm);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLICATION_CRYPTOGRAM, pxTransRes->T_9F26_Cryptogramm,
				sizeof(pxTransRes->T_9F26_Cryptogramm));
	}

	//TVR
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_95_TVR){
		for(i=0; i<sizeof(pxTransRes->T_95_TVR); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_95_TVR[i]);
		dmsg("TVR 95: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TVR, pxTransRes->T_95_TVR, sizeof(pxTransRes->T_95_TVR));
	}

	//CVM LIST
	memset(buf, 0x00, sizeof(buf));

	if(pxTransRes->T_DF61_Info_Received_Data[6] & EMV_CT_TRX_8E_CVM_List)
	{
		for(i=0; i<sizeof(pxTransRes->T_8E_CVM_List); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_8E_CVM_List[i]);

		dmsg("CVM LIST 8E: %s",buf);
	}


	//CVM RESULT
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_9F34_CVM_RES){
		for(i=0; i<sizeof(pxTransRes->T_9F34_CVM_Res); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F34_CVM_Res[i]);
		dmsg("CVM RES 9F34: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_CVM_RESULTS, pxTransRes->T_9F34_CVM_Res, sizeof(pxTransRes->T_9F34_CVM_Res));
	}

	//AMOUNT AUTH NUM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_9F02_AMOUNT)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F02_TXNAmount); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F02_TXNAmount[i]);

		dmsg("Txn amount 9F02: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_AMOUNT_AUTH_NUM, pxTransRes->T_9F02_TXNAmount, sizeof(pxTransRes->T_9F02_TXNAmount));
	}

	//AMOUNT OTHER NUM
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[6] & TRX_9F03_CB_AMOUNT)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F03_TXNAdditionalAmount); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F03_TXNAdditionalAmount[i]);

		dmsg("Txn other amount 9F03: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_AMOUNT_OTHER_NUM, pxTransRes->T_9F03_TXNAdditionalAmount,
				sizeof(pxTransRes->T_9F03_TXNAdditionalAmount));
	}

	//TRANSACTION CURRENCY CODE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_5F2A_TRX_CURRENCY)
	{
		for(i=0; i<sizeof(pxTransRes->T_5F2A_CurrencyTrans); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5F2A_CurrencyTrans[i]);

		dmsg("Txn Currency Code 5F2A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRANSACTION_CURRENCY_CODE, pxTransRes->T_5F2A_CurrencyTrans,
				sizeof(pxTransRes->T_5F2A_CurrencyTrans));
	}

	//TRANSACTION DATE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_9A_DATE)
	{
		for(i=0; i<sizeof(pxTransRes->T_9A_Date); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9A_Date[i]);

		dmsg("Txn DATE 9A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRANSACTION_DATE, pxTransRes->T_9A_Date, sizeof(pxTransRes->T_9A_Date));
	}

	//UNPREDICTABLE NUMBER
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_9F37_RANDOM_NUMBER)
	{
		for(i=0; i<sizeof(pxTransRes->T_9F37_RandomNumber); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F37_RandomNumber[i]);

		dmsg("Txn RANDOM NUM 9F37: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_UNPREDICTABLE_NUMBER, pxTransRes->T_9F37_RandomNumber,
				sizeof(pxTransRes->T_9F37_RandomNumber));
	}

	//AIP
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_82_AIP){
		for(i=0; i<sizeof(pxTransRes->T_82_AIP); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_82_AIP[i]);
		dmsg("AIP 82: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_AIP, pxTransRes->T_82_AIP, sizeof(pxTransRes->T_82_AIP));
	}

	//DF_NAME
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_84_DFNAME){
		for(i=0; i < pxTransRes->T_84_DFName.aidlen; i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_84_DFName.AID[i]);
		dmsg("DF NAME 84: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_DF_NAME, pxTransRes->T_84_DFName.AID, pxTransRes->T_84_DFName.aidlen);
	}

	//ISSUER APPLI DATA
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_9F10_DATAISSUER){
		for(i=0; i < pxTransRes->T_9F10_DataIssuer.issDataLen; i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F10_DataIssuer.issData[i]);
		dmsg("ISSUER DATA 9F10: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_ISSUER_APPLI_DATA, pxTransRes->T_9F10_DataIssuer.issData,
				pxTransRes->T_9F10_DataIssuer.issDataLen);
	}

	//ATC
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[0] & TRX_9F36_ATC){
		for(i=0; i<sizeof(pxTransRes->T_9F36_ATC); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F36_ATC[i]);
		dmsg("ATC 9F36: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_ATC, pxTransRes->T_9F36_ATC, sizeof(pxTransRes->T_9F36_ATC));
	}

	//TRANSACTION TYPE
	if(pxTransRes->T_DF61_Info_Received_Data[1] & TRX_9C_TRANSTYPE)
	{
		dmsg("Txn type 9C: 0x%02X",pxTransRes->T_9C_TransType);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TRANSACTION_TYPE, &pxTransRes->T_9C_TransType, sizeof(pxTransRes->T_9C_TransType));
	}

	//TERMINAL COUNTRY CODE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_9F1A_TERM_COUNTRY_CODE){
		for(i=0; i<sizeof(pxTransRes->T_9F1A_TermCountryCode); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F1A_TermCountryCode[i]);
		dmsg("TERMINAL COUNTRY CODE 9F1A: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TERMINAL_COUNTRY_CODE, pxTransRes->T_9F1A_TermCountryCode, sizeof(pxTransRes->T_9F1A_TermCountryCode));
	}

	//TERMINAL CAPABILITY
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_9F33_TERMCAP){
		for(i=0; i<sizeof(pxTransRes->T_9F33_TermCap); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F33_TermCap[i]);
		dmsg("TERM CAPABILITIES 9F33: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TERMINAL_CAPABILITIES, pxTransRes->T_9F33_TermCap, sizeof(pxTransRes->T_9F33_TermCap));
	}

	//TERMINAL TYPE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_9F35_TERMTYP){
		dmsg("TERMINAL TYPE 9F35: 0x%02X",pxTransRes->T_9F35_TermTyp);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_TERMINAL_TYPE, &pxTransRes->T_9F35_TermTyp, sizeof(pxTransRes->T_9F35_TermTyp));
	}

	//IDF SERIAL NUMBER
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[2] & TRX_9F1E_IFDSERIALNUMBER){
		for(i=0; i<sizeof(pxTransRes->T_9F1E_IFDSerialNumber); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_9F1E_IFDSerialNumber[i]);
		dmsg("IDF SERIAL NUM 9F1E: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_IFD_SERIAL_NUMBER, pxTransRes->T_9F1E_IFDSerialNumber,
				sizeof(pxTransRes->T_9F1E_IFDSerialNumber));
	}

	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_APPNAME){
		for(i=0; i<sizeof(pxTransRes->AppName); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->AppName[i]);
		dmsg("APP NAME 50: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_APPLICATION_LABEL, pxTransRes->AppName,
				sizeof(pxTransRes->AppName));
	}

	//APPLI CURRENCY CODE
	//VERIFONE Not Support Anymore...


/*
	//ISSUER COUNTRY CODE
	memset(buf, 0x00, sizeof(buf));
	if(pxTransRes->T_DF61_Info_Received_Data[3] & TRX_5F28_ISSCOUNTRYCODE){
		for(i=0; i<sizeof(pxTransRes->T_5F28_IssCountryCode); i++)
			sprintf(&buf[2*i],"%02X",pxTransRes->T_5F28_IssCountryCode[i]);
		dmsg("ISSUER COUNTRY CODE 5F28: %s",buf);
		mtiMapPutBytes(tpMap, MTI_EMV_TAG_ISSUER_COUNTRY_CODE, pxTransRes->T_5F28_IssCountryCode,
				sizeof(pxTransRes->T_5F28_IssCountryCode));
	}
*/
	//CARD HOLDER NAME
	if(pxTransRes->T_DF61_Info_Received_Data[5] & TRX_5F20_CARDHOLDER)
	{
		if(pxTransRes->T_5F20_Cardholder.crdNameLen > 0)
		{
			mtiMapPutBytes(tpMap, MTI_EMV_TAG_CARDHOLDER_NAME, pxTransRes->T_5F20_Cardholder.crdName,
					(INT)(pxTransRes->T_5F20_Cardholder.crdNameLen));
		}
	}

	dmsg("\n----------Transaction Results end\n");
}

int vdSetAdditionalCTTransactionData(EMV_CT_TRANSAC_TYPE* xEMVTransType, EMV_CT_SELECT_TYPE* xEMVStartData)
{
	char amountString[32 + 1] = {0,};

	memset((void*) xEMVTransType, 0, sizeof(EMV_CT_TRANSAC_TYPE));

	// Continue to provide the remaining transaction data (this is the reason why the same "substructure" is included StartTransaction and the ContinueOffline)

	//AMOUNT Callback
	if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_AMOUNT, amountString, NULL) == MTI_EMV_RTN_CONTINUE)
	{
		bcdfromulong (xEMVTransType->TXN_Data.Amount, sizeof(xEMVTransType->TXN_Data.Amount),(unsigned long)atol(amountString));
		xEMVTransType->Info_Included_Data[1] |=INPUT_OFL_AMOUNT;
	}
	else
	{
		return EMV_ADK_USR_BCKSPC_KEY_PRESSED;
	}


	// Transaction counter (example: if the transaction counter is per terminal, you can provide with StartTransaction already, if it is per acquirer or per AID, you just know the value after final select)
	xEMVTransType->TXN_Data.TransCount[0] = 0;
	xEMVTransType->TXN_Data.TransCount[1] = 0;
	xEMVTransType->TXN_Data.TransCount[2] = 0;
	xEMVTransType->TXN_Data.TransCount[3] = 1;
	xEMVTransType->Info_Included_Data[0] |= INPUT_OFL_TXN_COUNTER;

	//xEMVTransType->TXN_Data.PreSelected_Language = EMV_ADK_LANG_NO_LANG; // no preselected language
	//xEMVTransType->Info_Included_Data[0] |= INPUT_OFL_LANGUAGE;

	xEMVTransType->TXN_Data.uc_AmountConfirmation = CONFIRM_AMOUNT_BEFORE_CVM; // amount confirmation before/after PIN entry
	xEMVTransType->Info_Included_Data[0] |= INPUT_OFL_AMOUNT_CONF;

	// Fetch additional card/transaction data by adding a tag list, just for demonstration we add terminal caps and used CAP key Index
	xEMVTransType->TXN_Data.Additional_Result_Tags.anztag = 2;
	xEMVTransType->TXN_Data.Additional_Result_Tags.tags[0] = 0x9F33;
	xEMVTransType->TXN_Data.Additional_Result_Tags.tags[1] = 0x8F;
	xEMVTransType->Info_Included_Data[0] |= INPUT_OFL_ADD_TAGS;

	// Remark: Any change of date, time, amount, force online ... is possible here as well. Also the transaction options can be updated here, e.g. additionally activate DCC and amount confirmation here
	memcpy(xEMVTransType->TxnOptions, xEMVStartData->TxnOptions, sizeof(xEMVTransType->TxnOptions));
	xEMVTransType->TxnOptions[1] |= EMV_CT_TRXOP_PIN_BYPASS_NO_SUBSEQUENT;
	xEMVTransType->TxnOptions[1] |= EMV_CT_TRXOP_DCC_CALLBACK_ALWAYS | EMV_CT_TRXOP_DCC_CALLBACK;
	xEMVTransType->Info_Included_Data[0] |=  INPUT_OFL_TXN_OPTIONS; //raise the flag

	return EMV_ADK_OK;
}

EMV_ADK_INFO ContinueOnlineCT(EMV_CT_HOST_TYPE *xOnlineInput, EMV_CT_TRANSRES_TYPE *xTransRes, INT *iSubRet)
{
	EMV_ADK_INFO erg;
	INT iRet = 0;
	CHAR caResCode[3] = {0,};
	tMtiMap *tpTagMap = &mtiGetEmvParmInfo()->tEmvTagMap;
	UCHAR *ucpTag71_IsserScript = NULL;
	UCHAR *ucpTag72_IsserScript = NULL;
	INT iOffset = 0;
	UCHAR ucAuthData[256] = {0,};

	//Store Tag Data to MAP for Online Transaction
	setOnlineTransactionTag(xTransRes);

#if 1
	iRet = mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_ONLINE, caResCode, NULL);
	dmsg("MTI_EMV_STEP_NEED_ONLINE iRet [%d]", iRet);

//	if(iRet == MTI_EMV_RTN_SUSPEND || MTI_EMV_ONLINE_SUSPEND)
	if(iRet == MTI_EMV_RTN_SUSPEND)
		bUserCanceled = TRUE;

	if (iRet == MTI_EMV_ONLINE_APPROVED)
	{
		CHAR *appCode = NULL;
		//CHAR *authCode = NULL;
		UCHAR *ucpTag;
		INT iTagLen = 0;

		xOnlineInput->OnlineResult = TRUE;

		appCode = mtiMapGetString(tpTagMap, MTI_EMV_TAG_AUTHORISATION_RESPONSE_CODE);

		if(appCode == NULL)
			memcpy(xOnlineInput->AuthResp, "00", 2);
		else
			memcpy(xOnlineInput->AuthResp, appCode, 2);

		xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_ONLINE_AC;

#if 0
		iTagLen = 0;
		authCode = mtiMapGetString(tpTagMap, MTI_EMV_TAG_AUTHORISATION_CODE);
		if (authCode != NULL)
		{
			dmsg("TAG_89_AUTH_CODE [%s]", authCode);
			memcpy(xOnlineInput->AuthorizationCode, authCode, M_SIZEOF(EMV_CT_HOST_TYPE, AuthorizationCode));
			xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_AUTHCODE;
		}
#endif
#if 0
		ucpTag = mtiMapGetBytes(tpTagMap, TAG_89_AUTH_CODE, &iTagLen);

		if (iTagLen > 0 && ucpTag != NULL)
		{
			dbuf("TAG_89_AUTH_CODE", ucpTag, iTagLen);
			memcpy(xOnlineInput->AuthorizationCode, ucpTag, M_SIZEOF(EMV_CT_HOST_TYPE, AuthorizationCode));
			xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_AUTHCODE;
		}
#endif
		iTagLen = 0;
		ucpTag = mtiMapGetBytes(tpTagMap, TAG_91_ISS_AUTH_DATA, &iTagLen);
		if (iTagLen > 0 && ucpTag != NULL)
		{
			dbuf("TAG_91_ISS_AUTH_DATA", ucpTag, iTagLen);
			ucAuthData[0] = 0x91;
			ucAuthData[1] = iTagLen;
			mtiMemcpy(ucAuthData + 2, ucpTag, iTagLen);
			dbuf("TAG_91_ISS_AUTH_DATA AFTER PROC", ucAuthData, iTagLen + 2);
			xOnlineInput->AuthData = ucAuthData;
			xOnlineInput->LenAuth = iTagLen + 2;
			xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_AUTHDATA;
		}

		iTagLen = 0;
		ucpTag = mtiMapGetBytes(tpTagMap, TAG_71_ISS_SCRIPT_TPLT_1, &iTagLen);
		if (iTagLen > 0 && ucpTag != NULL)
		{
			iOffset = 0;
			if(*ucpTag != 0x71)
			{
				iOffset = 2;
			}
			ucpTag71_IsserScript = (UCHAR *) mtiMalloc(iTagLen + iOffset);
			mtiMemset(ucpTag71_IsserScript, 0x00, iTagLen + iOffset);

			if(*ucpTag != 0x71)
			{
				ucpTag71_IsserScript[0] = 0x71;
				ucpTag71_IsserScript[1] = iTagLen;
			}
			mtiMemcpy(ucpTag71_IsserScript + iOffset, ucpTag, iTagLen);

			dbuf("TAG_71_ISS_SCRIPT_TPLT_1", ucpTag71_IsserScript, iTagLen + iOffset);

			xOnlineInput->ScriptCritData = ucpTag71_IsserScript;
			xOnlineInput->LenScriptCrit = iTagLen + iOffset;
			xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_SCRIPTCRIT;
		}

		iTagLen = 0;
		ucpTag = mtiMapGetBytes(tpTagMap, TAG_72_ISS_SCRIPT_TPLT_2, &iTagLen);
		if (iTagLen > 0 && ucpTag != NULL)
		{
			iOffset = 0;
			if(*ucpTag != 0x72)
			{
				iOffset = 2;
			}

			ucpTag72_IsserScript = (UCHAR *) mtiMalloc(iTagLen + iOffset);
			mtiMemset(ucpTag72_IsserScript, 0x00, iTagLen + iOffset);

			if(*ucpTag != 0x72)
			{
				ucpTag72_IsserScript[0] = 0x72;
				ucpTag72_IsserScript[1] = iTagLen;
			}
			mtiMemcpy(ucpTag72_IsserScript + iOffset, ucpTag, iTagLen);

			dbuf("TAG_72_ISS_SCRIPT_TPLT_2", ucpTag72_IsserScript, iTagLen + iOffset);

			xOnlineInput->ScriptUnCritData = ucpTag72_IsserScript;
			xOnlineInput->LenScriptUnCrit = iTagLen + iOffset;
			xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_SCRIPTUNCRIT;
		}

	}
/*
	else if (iRet == MTI_EMV_ONLINE_DENIAL)
	{
		xOnlineInput->OnlineResult = FALSE;
	}
*/
	else
	{
		dmsg("Can not do EMV Process...");
		*iSubRet = MTI_EMV_TR_UNKNOWN_ERROR;
		// @@EB MALLOC MODIFY
                if (ucpTag71_IsserScript != NULL)
		{
			mtiFree(ucpTag71_IsserScript);
		}

		if (ucpTag72_IsserScript != NULL)
		{
			mtiFree(ucpTag72_IsserScript);
		}
                //
		return EMV_ADK_ABORT;
	}
#else
	xOnlineInput->OnlineResult = TRUE;

	memcpy(xOnlineInput->AuthResp, "\x30\x30", 2);
	xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_ONLINE_AC;

#endif
	//xOnlineInput holds data get from the host. Here only basic data for demo purposes is being set
	xOnlineInput->Info_Included_Data[0] |= INPUT_ONL_ONLINE_RESP;

	dmsg("----------EMV_CT_ContinueOnline...");
	erg = EMV_CT_ContinueOnline(xOnlineInput, xTransRes);
        // @@EB MALLOC MODIFY
        if (ucpTag71_IsserScript != NULL)
	{
		mtiFree(ucpTag71_IsserScript);
	}

	if (ucpTag72_IsserScript != NULL)
	{
		mtiFree(ucpTag72_IsserScript);
	}
        //

	switch (erg)
	{
		case EMV_ADK_TC:
			dmsg("----------CT Transaction approved online");
			return EMV_ADK_OK;
		default:
			dmsg("----------CT Transaction online error %02X", erg);
			break;
	}
	return erg;
}

static void emvCallback(unsigned char *pucSend, unsigned short sSendSize, unsigned char *pucReceive, unsigned short *psReceiveSize, void* externalData)
{
	struct BTLVNode *node = NULL;
	int tag = 0;
	unsigned char init = 0;
	struct BTLVNode xBtlv;
	//int inResult;

	vBTLVInit(&xBtlv,NULL);
	if((iBTLVImport(&xBtlv, pucSend, sSendSize, NULL, NULL)!=0)||((node=pxBTLVFindTag(&xBtlv, pcBTLVTagStr(TAG_F0_EMV_TEMPLATE)))==NULL)||((node = pxBTLVGetChild(node, 0))==NULL))// import TLV stream
	{
		init = 1; // import of message failed or message contains no F0-tag
	}

	if(init == 0)
	{
		sscanf(node->tcName, "%X", &tag);

		//dmsg("EMV CALLBACK FUNC TAG[0x%.4X]", tag);

		switch(tag)
		{
			case TAG_BF7F_CTLS_CBK_TRACE:
				{ //Trace callback. This option happens when EMV_CT_INIT_OPT_TRACE or EMV_CT_INIT_OPT_TRACE are enabled during the Framework Init.
					struct BTLVNode *x = NULL;
					char *str = NULL;

					//dmsg("### CALLBACK TAG_BF7F_CTLS_CBK_TRACE ####");

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

				//Refer to EMV_CT_Interface.h for details: \
				e.g. Tag DF70 is what application gets from the Framework and tag DF70 is what application is supposed to send back
			case TAG_BF0B_INIT_CALLBACK_THREAD:
				{
					dmsg("### CALLBACK CALLBACK_THREAD ####");
					init = 1;
					break;
				}

			case TAG_BF01_CBK_MERCHINFO:
				{
					//Refer to EMV_CT_Interface.h for details: \
					e.g. Tag DF70 is what application gets from the Framework and tag DF70 is what application is supposed to send back
					unsigned char merchantInfo;

					dmsg("### CALLBACK MERCHANT INFO ####");

					if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF70_CBK_MERCHINFO),&merchantInfo, sizeof(merchantInfo)) == sizeof(merchantInfo))
					printf("Merchant Info: (%#.2x)\n", merchantInfo);
					else
					init = 1;
					break;
				}

			case TAG_BF02_CBK_AMOUNTCONF:
				{
					// Callback for possible Amount confirmation during the transaction if not done in combination with PIN entry
					unsigned char tucAmount[6];
					char szFormattedAmount[14];
					long lAmt = 0L;

					dmsg("### CALLBACK AMOUNT CONFIRMATION ####");

					if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), tucAmount, sizeof(tucAmount)) == sizeof(tucAmount))
					{
						inFormatAmountForDisplay(tucAmount, szFormattedAmount, sizeof(szFormattedAmount));
						/*
						sprintf((char *)szFormattedAmount,"%s %.2f",gszCurrecyCode,atof((char *)szFormattedAmount)); //Add currency and confirmation text
						inResult=inGUIInputAction(CONFIRMAMOUNTOPTION, "Confirm Amount", szFormattedAmount, NULL, 0);
						if(inResult == 0)
						*/
						dmsg("AMOUNT [%s]", szFormattedAmount);

						lAmt = strtol(szFormattedAmount, NULL, 10);

						if(lAmt < 1)
						{
							dmsg("AMOUNT Error < 0");
							gucAmountConfirmed = FALSE;
						}
						else
							gucAmountConfirmed = TRUE;

						vBTLVClear(&xBtlv);
						if((node = pxBTLVAppendTag(&xBtlv, (const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0)) == NULL)
						{
							init = 1;
							break;
						}

						if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_DF71_CBK_AMOUNTCONF), &gucAmountConfirmed, 1) == NULL)
						{
							init = 1;
							break;
						}
					}
					else
						init = 1;
					break;
				}

			case TAG_BF04_CBK_REDUCE_CAND:
			{
				unsigned char ucSelection = 1;
				char cAppName[EMV_ADK_MAX_AIDSUPP][17];
				//const unsigned char* AID[EMV_ADK_MAX_AIDSUPP];
				//unsigned lenAID[EMV_ADK_MAX_AIDSUPP];
				//EMV_CT_CANDIDATE_DATA_TYPE candidateData[EMV_CT_COMMON_CANDS];
				tMtiMap tAidList;
				tMtiMap *tpAidList = &tAidList;

				int count;
				struct BTLVNode *x = NULL;
				struct BTLVNode *xpp = NULL;

				CHAR caTemp[128 + 1];
				CHAR *cpTemp = caTemp;

				dmsg("### CALLBACK APPLICATION SELECTION ####");
				mtiMapInit(tpAidList);

				/*for (count = 0; (x = pxBTLVFindNextTag(node, pcBTLVTagStr(TAG_DF04_AID), x)) != NULL && count < EMV_ADK_MAX_AIDSUPP; count++){
				AID[count] = x->pucData;
				lenAID[count] = x->uSize;
				}*/
				// to read the candidate list, TAG_50_APP_LABEL has to be searched n times inside the stream
				// ASCII, subsequently can be displayed and number 1...n includes the selection result
				x = NULL;
				xpp = NULL;
				for (count = 0;(x = pxBTLVFindNextTag(node, pcBTLVTagStr(TAG_50_APP_LABEL), x)) != NULL && count < EMV_ADK_MAX_AIDSUPP; count++)
				{
					mtiMemset(caTemp, 0, sizeof(caTemp));
					cpTemp = caTemp;

					if ((xpp = pxBTLVFindNextTag(node, pcBTLVTagStr(TAG_DF04_AID), xpp)) != NULL)
					{
						cpTemp += mtiHtoa((UCHAR*)cpTemp, xpp->pucData, xpp->uSize * 2);
						*cpTemp++ = '|';
						snprintf(cpTemp	, MIN(x->uSize+1,sizeof(cAppName[count])),"%s",x->pucData); // for up to 9 applications
						mtiMapPutString(tpAidList, tpAidList->iMapCount + 1, caTemp);
					}

					//memset(cAppName[count], 0x00, sizeof(cAppName[count]));
					//snprintf(cAppName[count], MIN(x->uSize+1,sizeof(cAppName[count])),"%s",x->pucData); // for up to 9 applications
					//mtiMapPutString(tpAidList, tpAidList->iMapCount + 1, cAppName[count]);




				}

				//ucSelection= inGUIInputAction(LISTSELECTION, "Application Selection", cp, NULL, count);
				dmsg("Application Selection");
				{
					int idx;
					int ikey = 0;
					for(idx = 0; idx < tpAidList->iMapCount; idx++)
					{
						ikey = MemCheck(tpAidList, idx);
						dmsg("IDX[%d] - Application Name[%s]", idx, mtiMapGetString(tpAidList, ikey));
					}
				}

				dmsg("----------Default Selection");

				ucSelection = mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_SELECT_AID, tpAidList, &count);
				mtiMapClear(tpAidList);
				if (ucSelection == MTI_EMV_RTN_CONTINUE)
				{
					ucSelection = count + 1;
				}
				else
				{
					ucSelection = 0;
					bUserCanceled = TRUE;
				}

				dmsg("ucSelection[%d]", ucSelection);

				//UI_ShowMessage("", (const char *) cAppName[ucSelection - 1]/*UI_STR_SELECTED*/);
				//inGUIInputAction(DISPLAYMESSAGE, SELECTED_APP_MESSAGE, cAppName[ucSelection - 1], NULL, 0);

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
			case TAG_BF06_CBK_LOCAL_CHECKS:
				{  //Braces are needed in VOS to begin-end the case.
					unsigned char T_57_PAN[10] = {0,}; // 10 hex characters = 19+1 asccii digits
					unsigned char T_9F1B_FloorLimit[4] = {0,};
					unsigned char T_5F24_AppExpDate[3] = {0,};
					unsigned char T_9F02_Amount[6] = {0,};
					unsigned char T_9F42_Currency[2] = {0,};
					unsigned char T_5F28_IssCountryCode[2] = {0,};
					//unsigned char T_9F1A_TerminalCountryCode[2] = { 0, };
					//unsigned char ucPanStr[30] = {0,};
					int iPanLen = 0, i =0, i57TagLen = 0;
					unsigned char cStat = 0;
					unsigned char ucaTemp[50 + 1];

					tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;

					dmsg("### CALLBACK DOMESTIC CHECK AND CHANGES ####");

					i57TagLen = iBTLVExtractTag(node, pcBTLVTagStr(TAG_5A_APP_PAN), T_57_PAN, sizeof(T_57_PAN));
					dmsg("**** iBTLVExtractTag(node, pcBTLVTagStr(TAG_5A_APP_PAN), T_57_PAN, sizeof(T_57_PAN)) = %d", i57TagLen);
					if(i57TagLen > 0)
					{
						iBTLVExtractTag(node,pcBTLVTagStr(TAG_9F1B_TRM_FLOOR_LIMIT), T_9F1B_FloorLimit,sizeof(T_9F1B_FloorLimit));
						iBTLVExtractTag(node,pcBTLVTagStr(TAG_5F24_APP_EXP_DATE), T_5F24_AppExpDate, sizeof(T_5F24_AppExpDate));
						iBTLVExtractTag(node,pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), T_9F02_Amount,sizeof(T_9F02_Amount));
						iBTLVExtractTag(node,pcBTLVTagStr(TAG_9F42_APP_CURRENCY_CODE), T_9F42_Currency,sizeof(TAG_9F42_APP_CURRENCY_CODE));
						iBTLVExtractTag(node,pcBTLVTagStr(TAG_5F28_ISS_COUNTRY_CODE), T_5F28_IssCountryCode,sizeof(TAG_9F42_APP_CURRENCY_CODE));

						// More parameters can be fetched in the same way. \
						Few tags are being fetched only for demo purposes. Refer to TAG_BF06_CBK_LOCAL_CHECKS to see all possible tags.
						vBTLVClear(&xBtlv);
						if((node = pxBTLVAppendTag(&xBtlv, (const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0)) == NULL)
						{
							init = 1;
							break;
						}

						//FLOOR LIMIT
						dbuf("T_9F1B_FloorLimit", T_9F1B_FloorLimit, sizeof(T_9F1B_FloorLimit));

						//@@CACING PENTEST strcpy(gszCurrecyCode, "RP");
						strncpy(gszCurrecyCode, "RP",2);
						//vdFormatCurrencyCode(T_9F42_Currency,gszCurrecyCode);

						dbuf("CARD PAN", T_57_PAN, sizeof(T_57_PAN));
						//iPanLen = sizeof(T_57_PAN) - 2;

						mtiMemset(ucaTemp, 0, sizeof(ucaTemp));
						mtiHtoa(ucaTemp, T_57_PAN, i57TagLen);

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
						mtiHtoa(&ucaTemp[i], T_5F24_AppExpDate, 2);
						iPanLen = mtiStrlen(40, ucaTemp);
						
						if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_CONFIRM_PAN, ucaTemp, &iPanLen) != MTI_EMV_RTN_CONTINUE)
						{
							cStat = EMV_ADK_USR_BCKSPC_KEY_PRESSED;		//ABORT
							bUserCanceled = TRUE;
							if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_DF76_CBK_MANIPUL_TRX), &cStat, 1) == NULL)
							{
								init = 1;
								break;
							}
						}

						//CARD ISSUER_COUNTRY_CODE
						dbuf("5F28 ISS_COUNTRY_CODE", T_5F28_IssCountryCode, sizeof(T_5F28_IssCountryCode));
						if(tpMap != NULL)
						{
							mtiMapPutBytes(tpMap, MTI_EMV_TAG_ISSUER_COUNTRY_CODE, T_5F28_IssCountryCode, sizeof(T_5F28_IssCountryCode));
						}

#if 0
						T_9F1B_FloorLimit[3]++; // Change the floorlimit value: increase 1 unit of currency
						T_9F1B_FloorLimit[3]--; // decrease 1 unit of currency

						if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_9F1B_TRM_FLOOR_LIMIT), T_9F1B_FloorLimit, sizeof(T_9F1B_FloorLimit))
								== NULL)
						{
							init = 1;
							break;
						}

						if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), T_9F02_Amount, sizeof(T_9F02_Amount))
								== NULL)
						{
							init = 1;
							break;
						}
						/*
						vdFormatCurrencyCode(T_9F42_Currency,gszCurrecyCode);
						if(pxBTLVAppendTag(node, (const char *) pcBTLVTagStr(TAG_9F42_APP_CURRENCY_CODE), T_9F42_Currency, sizeof(T_9F42_Currency))
								== NULL)
						{
							init = 1;
							break;
						}
						*/
#endif
					}
					else
					{
						init = 1;
					}
					break;
				}
			case TAG_BF09_CBK_CARDHOLDERINFO:
				{
					dmsg("### CALLBACK CARDHOLDER CHECK EVENT ####");
					eCardholderInfo cardHoldInfo;
					if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF6E_CBK_CARDHOLDERINFO), &cardHoldInfo, sizeof(cardHoldInfo)) > 0)
					{
						switch(cardHoldInfo)
						{
						case eEMVCrdWrongPIN:
							dmsg("WRONG PIN");
							break;
						case eEMVCrdCorrectPIN:
							dmsg("CORRECT PIN");
							break;
						case eEMVCrdAppChange:
							dmsg("APP is Changed");
							break;
						case eEMVCrdLastTryPIN:
							dmsg("LAST TRY PIN");
							break;
						default:
							dmsg("cardHoldInfo [0x%.2x] is not available", cardHoldInfo);
							break;
						}
					}

				}
				break;
			case TAG_BF07_CBK_DCC:
				{
					// Callback for Dynamic currency change after Read Records
					unsigned char T_57_PAN[10]; // 19+1
					unsigned char T_9F1B_FloorLimit[4];
					unsigned char tucAmount[6];
					unsigned char T_9F42_AppCurrencyCode[2];
					unsigned char ucDccMode;

					dmsg("### CALLBACK DCC EVENT ####");
					if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_5A_APP_PAN), T_57_PAN, sizeof(T_57_PAN)) > 0)
					{
						iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F1B_TRM_FLOOR_LIMIT), T_9F1B_FloorLimit, sizeof(T_9F1B_FloorLimit));
						iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F42_APP_CURRENCY_CODE), T_9F42_AppCurrencyCode, sizeof(T_9F42_AppCurrencyCode));
						iBTLVExtractTag(node, pcBTLVTagStr(TAG_9F02_NUM_AMOUNT_AUTH), tucAmount, sizeof(tucAmount));
						vBTLVClear(&xBtlv); // This Demo shows case 1: No DCC performed

						if((node = pxBTLVAppendTag(&xBtlv, (const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0))
								 == NULL)
						{
							init = 1;
							break;
						}

						ucDccMode = MODE_DCC_NO_TRX_CONTINUE;// No DCC necessary (If DCC is necessary, change and add the parameters as mentioned above or restart the transaction with the new values)

						if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF7D_CBK_DCC_CHECK), &ucDccMode, sizeof(ucDccMode))
								== NULL)
						{
							init = 1;
							break;
						}
					}
					else
						init = 1;
					break;
				}

			// ON/OFFLINE PIN
			case TAG_BF08_CBK_PIN:
#if 0
			{
					struct BTLVNode *x = NULL;
					unsigned char pinType, pinResult, ucAbout;
					unsigned char* pucUnpredNb = NULL;
					unsigned char* pucPINPublicKey = NULL;
					unsigned char pucPINResult[32];
					unsigned short KeyLength = 0;
					unsigned long PINPublicKeyExp = 0;
					unsigned char buf[4];
					unsigned char bypass = 1;
					
					int iRet = 0;
					tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;
					int iLength;
					unsigned char pinbyPassPermit = 1;
					unsigned char needPIN = 0;

					dmsg("### CALLBACK PIN INPUT EVENT ####");

					if((iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF79_CBK_PIN_INFO),&pinType, 1) == 1)
							&& (iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF41_PIN_BYPASS),&bypass, 1) == 1))
					{
						if((x = pxBTLVFindTag(node,pcBTLVTagStr(TAG_9F37_UNPREDICTABLE_NB))) != NULL)
						{
							pucUnpredNb = x->pucData;
						}
						if((x = pxBTLVFindTag(node,pcBTLVTagStr(TAG_DF7A_CBK_PIN_KEY_DATA))) != NULL)
						{
							KeyLength = x->uSize;
							pucPINPublicKey = x->pucData;
						}
						if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF7B_CBK_PIN_KEY_EXP),buf, 4) == 4)
						{
							PINPublicKeyExp = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
						}

						vBTLVClear(&xBtlv);
						if((node = pxBTLVAppendTag(&xBtlv,(const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0))== NULL)
						{
							init = 1;
							break;
						}

						dmsg("----------> PIN TYPE: [%d], PINBYPASS PERMISSION : [%d]", pinType, pinbyPassPermit);

						if(pinType == EMV_CT_PIN_INPUT_ONLINE)
						{
							dmsg("# EVENT [EMV_CT_PIN_INPUT_ONLINE] #");
							iRet = mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_PIN, NULL, NULL);
							dmsg("MTI_EMV_STEP_NEED_PIN iRet [%d]", iRet);

							if (iRet == MTI_EMV_RTN_SUSPEND)
							{
								dmsg("USER CANCEL ONLINE PIN");
								//Transaction User Cancel
								pinResult = EMV_CT_PIN_INPUT_ABORT;
							}
							else if(iRet == MTI_EMV_RTN_PIN_BYPASS)
							{
								pinResult = EMV_CT_PIN_INPUT_BYPASS;
							}
							else if(iRet == MTI_EMV_RTN_CONTINUE)
							{
								pinResult = EMV_CT_PIN_INPUT_OKAY;
							}
						}
						// OFFLINE PIN
						else
						{
							dmsg("# EVENT [EMV_CT_PIN_INPUT_OFFLINE] #");

#if 0
							pinbyPassPermit = checkNeedOnlinePIN(tpMap);
							
							dmsg("# ONLINE PIN REQUIRE CHECK VALUE : [%d]", pinbyPassPermit);

							// pin input selection
							if(0==pinbyPassPermit)
							{
								eCardholderInfo cardHoldInfo;
								iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF6E_CBK_CARDHOLDERINFO), &cardHoldInfo, sizeof(cardHoldInfo));
								
								dpt_n_dmsg("#### OFFLINE PIN STATUS : %d", cardHoldInfo);

								if(eEMVCrdWrongPIN==cardHoldInfo || eEMVCrdLastTryPIN==cardHoldInfo)
								{
									iRet = MTI_EMV_TRN_PIN;
								}
								else
								{
			
									// Check only once !!
									if(FALSE==g_bReqPINOption)
										iRet = mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_OPTION_USER_SEL_PINBYPASS, NULL, NULL);
								}

								if (iRet == MTI_EMV_RTN_SUSPEND)
								{
									dmsg("USER CANCEL PIN");
									pinResult = EMV_CT_PIN_INPUT_ABORT;
									needPIN = 0;
								}
								else if(iRet == MTI_EMV_TRN_SIGN)
								{
									needPIN = 0;
									pinResult = EMV_CT_PIN_INPUT_BYPASS;
								}
								else
								{
									g_bReqPINOption = TRUE;
									needPIN = 1;
									bypass = 0;
								}								
							}
							// domestic
							else if(1==pinbyPassPermit)
							{
								// skip offline pin input !!! 
								needPIN = 0;
								pinResult = EMV_CT_PIN_INPUT_BYPASS;
							}
							// international 
							else
							{
								needPIN = 1;
								bypass = 1;
							}

							dmsg("# PIN PAD TYPE SELECTION [NEED PIN : %d], [PINBYPASS : %d] #", needPIN, bypass);
	
							// finally request the PIN
							if(1==needPIN)
							{
								/*
								if((getIsADKGUI() == FALSE) && ((pinType == EMV_CT_PIN_INPUT_ENCIPHERED) || (pinType == EMV_CT_PIN_INPUT_PLAIN)))
								{
									startADKGUI();
								}
								*/

								if(getIsADKGUI() == FALSE)
								{
									startADKGUI();
								}

								uiSetPropertyInt(UI_PROP_TIMEOUT, (int) PIN_INPUT_TIMEOUT * 1000);
								uiSetPropertyInt(UI_PROP_PIN_INTERCHAR_TIMEOUT, (int) 10 * 1000);
								//
								SVC_WAIT(100);

								pinResult = ucInputPIN(pinType, bypass, pucUnpredNb, pucPINResult, KeyLength, pucPINPublicKey, PINPublicKeyExp);

								dmsg("# ucInputPIN Result [%d] #", pinResult);

								uiSetPropertyInt(UI_PROP_UPDATE_EVENTS, 1);

								/*
								if((getIsADKGUI() == TRUE) && ((pinType == EMV_CT_PIN_INPUT_ENCIPHERED) || (pinType == EMV_CT_PIN_INPUT_PLAIN)))
								{
									stopADKGUI();
								}
								*/

								if(getIsADKGUI() == TRUE)
								{
									stopADKGUI();
								}
							}
#else
							//pinResult = EMV_CT_PIN_INPUT_BYPASS;
// BY Allen for offline PIN processing when bank UAT
//      if PIN baseed tranaction flag is off then entry PIN without bypass
							if (mtiMapGetInt(MAP_EDC_PARAM, KEY_PARAM_FEA_PINBASEDTR) == TRUE)
							{
								pinResult = EMV_CT_PIN_INPUT_BYPASS;
							}
							else
							{
								eCardholderInfo cardHoldInfo;
								iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF6E_CBK_CARDHOLDERINFO), &cardHoldInfo, sizeof(cardHoldInfo));

								dpt_n_dmsg("#### OFFLINE PIN STATUS : %d", cardHoldInfo);

								if(eEMVCrdWrongPIN==cardHoldInfo || eEMVCrdLastTryPIN==cardHoldInfo)
								{
									iRet = MTI_EMV_TRN_PIN;
								}

								if(getIsADKGUI() == FALSE)
								{
									startADKGUI();
								}

								uiSetPropertyInt(UI_PROP_TIMEOUT, (int) PIN_INPUT_TIMEOUT * 1000);
								uiSetPropertyInt(UI_PROP_PIN_INTERCHAR_TIMEOUT, (int) 10 * 1000);
								//
								SVC_WAIT(100);

								pinResult = ucInputPIN(pinType, bypass, pucUnpredNb, pucPINResult, KeyLength, pucPINPublicKey, PINPublicKeyExp);

								dmsg("# ucInputPIN Result [%d] #", pinResult);

								uiSetPropertyInt(UI_PROP_UPDATE_EVENTS, 1);

								/*
								if((getIsADKGUI() == TRUE) && ((pinType == EMV_CT_PIN_INPUT_ENCIPHERED) || (pinType == EMV_CT_PIN_INPUT_PLAIN)))
								{
									stopADKGUI();
								}
								*/

								if(getIsADKGUI() == TRUE)
								{
									stopADKGUI();
								}
							}
#endif

						}

						dmsg("PIN RESULT [0x%.2x]", pinResult);
						if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF79_CBK_PIN_INFO), &pinResult, 1) == NULL)
						{
							init = 1;
							break;
						}

						if((pinResult == EMV_CT_PIN_INPUT_OKAY) && ((pinType == EMV_CT_PIN_INPUT_ENCIPHERED ||
								pinType == EMV_CT_PIN_INPUT_PLAIN)))
						{
							if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF6F_CBK_PIN_ICC_RESP), pucPINResult, 2) == NULL)
							{
								init = 1;
								break;
							}
						}
					}
					else
						init = 1;
					break;
				}
#else
				{
					struct BTLVNode *x = NULL;
					unsigned char pinType, pinResult;
					unsigned char* pucUnpredNb = NULL;
					unsigned char* pucPINPublicKey = NULL;
					unsigned char pucPINResult[32];
					unsigned short KeyLength = 0;
					unsigned long PINPublicKeyExp = 0;
					unsigned char buf[4];
					unsigned char bypass = 0;

					int iRet = 0;
					tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;
					//int iLength;
					unsigned char pinbyPassPermit = 1;
					//unsigned char needPIN = 0;

					dmsg("### CALLBACK PIN INPUT EVENT ####");

					if((iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF79_CBK_PIN_INFO),&pinType, 1) == 1)
							&& (iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF41_PIN_BYPASS),&bypass, 1) == 1))
					{
						if((x = pxBTLVFindTag(node,pcBTLVTagStr(TAG_9F37_UNPREDICTABLE_NB))) != NULL)
						{
							pucUnpredNb = x->pucData;
						}
						if((x = pxBTLVFindTag(node,pcBTLVTagStr(TAG_DF7A_CBK_PIN_KEY_DATA))) != NULL)
						{
							KeyLength = x->uSize;
							pucPINPublicKey = x->pucData;
						}
						if(iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF7B_CBK_PIN_KEY_EXP),buf, 4) == 4)
						{
							PINPublicKeyExp = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
						}

						vBTLVClear(&xBtlv);
						if((node = pxBTLVAppendTag(&xBtlv,(const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0))== NULL)
						{
							init = 1;
							break;
						}

						dmsg("----------> PIN TYPE: [%d], PINBYPASS PERMISSION : [%d]", pinType, pinbyPassPermit);

						if(pinType == EMV_CT_PIN_INPUT_ONLINE)
						{
							dmsg("# EVENT [EMV_CT_PIN_INPUT_ONLINE] #");
							iRet = mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_PIN, NULL, NULL);
							dmsg("MTI_EMV_STEP_NEED_PIN iRet [%d]", iRet);

							if (iRet == MTI_EMV_RTN_SUSPEND)
							{
								dmsg("USER CANCEL ONLINE PIN");
								//Transaction User Cancel
								pinResult = EMV_CT_PIN_INPUT_ABORT;
							}
							else if(iRet == MTI_EMV_RTN_PIN_BYPASS)
							{
								pinResult = EMV_CT_PIN_INPUT_BYPASS;
							}
							else if(iRet == MTI_EMV_RTN_CONTINUE)
							{
								pinResult = EMV_CT_PIN_INPUT_OKAY;
							}
						}
						// OFFLINE PIN
						else
						{
							dmsg("# EVENT [EMV_CT_PIN_INPUT_OFFLINE] #");

#if 0
							pinbyPassPermit = checkNeedOnlinePIN(tpMap);

							dmsg("# ONLINE PIN REQUIRE CHECK VALUE : [%d]", pinbyPassPermit);

							// pin input selection
							if(0==pinbyPassPermit)
							{
								eCardholderInfo cardHoldInfo;
								iBTLVExtractTag(node, pcBTLVTagStr(TAG_DF6E_CBK_CARDHOLDERINFO), &cardHoldInfo, sizeof(cardHoldInfo));

								dpt_n_dmsg("#### OFFLINE PIN STATUS : %d", cardHoldInfo);

								if(eEMVCrdWrongPIN==cardHoldInfo || eEMVCrdLastTryPIN==cardHoldInfo)
								{
									iRet = MTI_EMV_TRN_PIN;
								}
								else
								{

									// Check only once !!
									if(FALSE==g_bReqPINOption)
										iRet = mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_OPTION_USER_SEL_PINBYPASS, NULL, NULL);
								}

								if (iRet == MTI_EMV_RTN_SUSPEND)
								{
									dmsg("USER CANCEL PIN");
									pinResult = EMV_CT_PIN_INPUT_ABORT;
									needPIN = 0;
								}
								else if(iRet == MTI_EMV_TRN_SIGN)
								{
									needPIN = 0;
									pinResult = EMV_CT_PIN_INPUT_BYPASS;
								}
								else
								{
									g_bReqPINOption = TRUE;
									needPIN = 1;
									bypass = 0;
								}
							}
							// domestic
							else if(1==pinbyPassPermit)
							{
								// skip offline pin input !!!
								needPIN = 0;
								pinResult = EMV_CT_PIN_INPUT_BYPASS;
							}
							// international
							else
							{
								needPIN = 1;
								bypass = 1;
							}

							dmsg("# PIN PAD TYPE SELECTION [NEED PIN : %d], [PINBYPASS : %d] #", needPIN, bypass);

							// finally request the PIN
							if(1==needPIN)
							{
								/*
								if((getIsADKGUI() == FALSE) && ((pinType == EMV_CT_PIN_INPUT_ENCIPHERED) || (pinType == EMV_CT_PIN_INPUT_PLAIN)))
								{
									startADKGUI();
								}
								*/

								if(getIsADKGUI() == FALSE)
								{
									startADKGUI();
								}

								uiSetPropertyInt(UI_PROP_TIMEOUT, (int) PIN_INPUT_TIMEOUT * 1000);
								uiSetPropertyInt(UI_PROP_PIN_INTERCHAR_TIMEOUT, (int) 10 * 1000);
								//
								SVC_WAIT(100);

								pinResult = ucInputPIN(pinType, bypass, pucUnpredNb, pucPINResult, KeyLength, pucPINPublicKey, PINPublicKeyExp);

								dmsg("# ucInputPIN Result [%d] #", pinResult);

								uiSetPropertyInt(UI_PROP_UPDATE_EVENTS, 1);

								/*
								if((getIsADKGUI() == TRUE) && ((pinType == EMV_CT_PIN_INPUT_ENCIPHERED) || (pinType == EMV_CT_PIN_INPUT_PLAIN)))
								{
									stopADKGUI();
								}
								*/

								if(getIsADKGUI() == TRUE)
								{
									stopADKGUI();
								}
							}
#else
// BY Allen for offline PIN processing when bank UAT
//      if PIN baseed tranaction flag is off then entry PIN without bypass
							if (mtiMapGetInt(MAP_EDC_PARAM, KEY_PARAM_FEA_PINBASEDTR) == TRUE)
								pinResult = EMV_CT_PIN_INPUT_BYPASS;
							else
							{
								// finally request the PIN
								//startADKGUI();
								//uiSetPropertyInt(UI_PROP_TIMEOUT, (int) PIN_INPUT_TIMEOUT * 1000);
								//uiSetPropertyInt(UI_PROP_PIN_INTERCHAR_TIMEOUT, (int) 10 * 1000);
								//
								SVC_WAIT(100);
								bypass = 0;

								ginhCrypto = open("/dev/crypto", 0);
								dmsg("-----Open /dev/crypto returned %d", ginhCrypto);

								pinResult = ucInputIPP_PIN(pinType, bypass, pucUnpredNb, pucPINResult,KeyLength, pucPINPublicKey, PINPublicKeyExp);
								//uiSetPropertyInt(UI_PROP_UPDATE_EVENTS, 1);

								close(ginhCrypto);

								//stopADKGUI();
							}
#endif

						}

						dmsg("PIN RESULT [0x%.2x]", pinResult);
						if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF79_CBK_PIN_INFO), &pinResult, 1) == NULL)
						{
							init = 1;
							break;
						}

						if((pinResult == EMV_CT_PIN_INPUT_OKAY) && ((pinType == EMV_CT_PIN_INPUT_ENCIPHERED ||
								pinType == EMV_CT_PIN_INPUT_PLAIN)))
						{
							if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF6F_CBK_PIN_ICC_RESP), pucPINResult, 2) == NULL)
							{
								init = 1;
								break;
							}
						}
					}
					else
						init = 1;
					break;
				}
#endif
			case TAG_BF0A_CBK_ENTER_PIN:
			{
				//Callback for EMV PIN request. Triggered when calling EMV_CT_ContinueOffline()
				unsigned char ucAbort = EMV_CT_PIN_INPUT_OKAY;

				dmsg("----------> CALLBACK PIN ABORT");
				if(EMV_CT_SmartDetect(0) != EMV_ADK_SMART_STATUS_OK){
					dmsg("----------CARD NOT PRESENT...");
					ucAbort = EMV_CT_PIN_INPUT_ABORT;
				}

				vBTLVClear(&xBtlv);
				if((node = pxBTLVAppendTag(&xBtlv,(const char *) pcBTLVTagStr(TAG_F0_EMV_TEMPLATE), NULL, 0))== NULL)
				{
					init = 1;
					break;
				}

				if(pxBTLVAppendTag(node,(const char *) pcBTLVTagStr(TAG_DF6C_CBK_PIN_CANCEL),&ucAbort, 1) == NULL)
				{
					init = 1;
					break;
				}
				break;
			}
			case TAG_BF14_CBK_TEXT_DISPLAY:
				{
					//TODO: EMV_ADK_TXT_2_CARDS_IN_FIELD should be received when detecting 2 cards. \
					However is not working properly even though CLTRXOP_L1_ERROR_CALLBACK is set.

					unsigned char callback_textID;
					char szDisplayMessage[21];

					memset(szDisplayMessage, 0, sizeof(szDisplayMessage));
					iBTLVExtractTag(node,pcBTLVTagStr(TAG_DF8F12_DISPLAY_TEXT),&callback_textID, sizeof(callback_textID));
					switch (callback_textID)
					{
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
					//inGUIInputAction(DISPLAYMESSAGE, szDisplayMessage, (char *)RETAP_CARD_MESSAGE, NULL, 0);
					dmsg("DISPLAY MSG[%s]",szDisplayMessage);
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
		if((*psReceiveSize = iBTLVExport(&xBtlv, pucReceive, *psReceiveSize)) <= 0)
		{	// export TLV stream
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

EMV_ADK_INFO inApplicationSelectionCT(EMV_CT_SELECT_TYPE *TxDataCT, EMV_CT_SELECTRES_TYPE *xSelectResCT)
{
	EMV_ADK_INFO erg;

	do{
		erg = EMV_CT_StartTransaction(TxDataCT, xSelectResCT); // Application Selection
		if(erg == EMV_ADK_BADAPP)
		{
			dmsg("----------Appl selection BADAPP! %$.2x", erg);
			TxDataCT->InitTXN_Buildlist = REUSE_LIST_REMOVE_AID;
			TxDataCT->Info_Included_Data[0] |=  INPUT_SEL_BUILDLIST; //raise the flag
		}
	}while(erg == EMV_ADK_BADAPP);

	switch(erg)
	{
		case EMV_ADK_OK:           // application selected, everything OK
			dmsg("----------Appl selection OK!");
			break;

		case EMV_ADK_FALLBACK:     // perform fallback to magstripe
			// Add reaction to local fallback rules or other local chip
			dmsg("----------Appl selection FALLBACK! %#.2x", erg);
			break;

		case EMV_ADK_ABORT:
			dmsg("----------Appl selection ABORT! %#.2x", erg);		   // no fallback, TRX definitely finished
			break;

		case EMV_ADK_APP_BLOCKED:  // application blocked
			dmsg("----------Appl selection APP BLOCKED! %#.2x", erg);		   //  no fallbacks.
			break;
		case EMV_ADK_NOAPP:        // no application found
		default:
			// please feel free to add any other text and reaction if there is any other local chip
			dmsg("----------Appl selection UNKWN! %#.2x", erg);
			break;
	}
	return erg;
}


static void vdEndTransaction(const char *szStatusMessage, unsigned char ucReader, int inExitWithError)
{
	unsigned char uRet = 0;
	if (inExitWithError)
		mtiSoundWarning(50, 2);

	dmsg("----------Ending EMV CT Transaction");

	uRet = EMV_CT_EndTransaction(0);

	if(uRet != EMV_ADK_OK)
	{
		dmsg("----------EMV_CT_EndTransaction error [0x%.2x]", uRet);
	}


	uRet = EMV_CT_SmartPowerOff(0);

	if(uRet != EMV_ADK_SMART_STATUS_OK)
	{
		dmsg("----------EMV_CT_SmartPowerOff error [%d]", uRet);
	}


	dmsg("DISPLAY MSG[%s]", szStatusMessage);
	dmsg("DISPLAY MSG[%s]", REMOVE_CARD_MESSAGE2);

}

void vdSetCTTransactionData(EMV_CT_SELECT_TYPE* xEMVStartData, char * szAmount)
{
	memset((unsigned char*)xEMVStartData, 0x00, sizeof(EMV_CT_SELECT_TYPE));
	// You should use the TXN_Data struct and TXNOptions in the subsequent functions (ContinueOffline) as well, just update them, once a new transaction parameter is known or valid
	xEMVStartData->InitTXN_Buildlist = BUILD_NEW;
	xEMVStartData->Info_Included_Data[0] |=  INPUT_SEL_BUILDLIST; //raise the flag

	xEMVStartData->TransType = EMV_ADK_TRAN_TYPE_GOODS_SERVICE;
	xEMVStartData->Info_Included_Data[0] |=  INPUT_SEL_TTYPE; //raise the flag

	// Application Selection Parameters
	xEMVStartData->SEL_Data.No_DirectorySelect = FALSE;
	xEMVStartData->Info_Included_Data[1] |=  INPUT_SEL_NO_PSE; //raise the flag

	xEMVStartData->SEL_Data.ucCardholderConfirmation = CARD_CONF_YES; // cardholder confirmation possible and allowed
	xEMVStartData->Info_Included_Data[1] |=  INPUT_SEL_CARDCONF; //raise the flags

//	bcdfromulong (xEMVStartData->TXN_Data.Amount, sizeof(xEMVStartData->TXN_Data.Amount),(unsigned long)atol(szAmount));
//	xEMVStartData->Info_Included_Data[0] |=  INPUT_SEL_AMOUNT; //raise the flag


	xEMVStartData->TXN_Data.Force_Online = FALSE; // no force Online
	xEMVStartData->TXN_Data.Force_Acceptance = FALSE; // no force acceptance
	xEMVStartData->TXN_Data.Online_Switch = FALSE;
	xEMVStartData->Info_Included_Data[1] |=  INPUT_SEL_FORCE_ONLINE | INPUT_SEL_FORCE_ACCEPT | INPUT_SEL_ONLINE_SWITCH; //raise the flags

	xEMVStartData->TxnOptions[0] |= EMV_CT_SELOP_CBCK_APPLI_SEL;//Activate appl selection
	xEMVStartData->TxnOptions[0] |= EMV_CT_NO_LONGEST_AID_MATCH;
	xEMVStartData->TxnOptions[1] |= EMV_CT_TRXOP_AMOUNT_CONF; 	//Activate amount confirmation
	xEMVStartData->TxnOptions[1] |= EMV_CT_TRXOP_MULTIPLE_RANDOM_NUMBERS;
	xEMVStartData->TxnOptions[1] |= EMV_CT_TRXOP_PIN_BYPASS_NO_SUBSEQUENT;
	xEMVStartData->TxnOptions[2] |= EMV_CT_TRXOP_LOCAL_CHCK_CALLBACK | EMV_CT_TRXOP_CARDHINFO_CALLBACK;
	xEMVStartData->TxnOptions[3] |= EMV_CT_TRXOP_GET_POS_DECISION | EMV_CT_TRXOP_GET_OFFLINE_USER_PIN | EMV_CT_TRXOP_GET_ONLINE_USER_PIN | EMV_CT_TRXOP_PERFORM_SIGNATURE;
	xEMVStartData->Info_Included_Data[1] |=  INPUT_SEL_TXN_OPTIONS; //raise the flag

	vdGetDate((unsigned char*) xEMVStartData->TXN_Data.Date, sizeof(xEMVStartData->TXN_Data.Date));
	xEMVStartData->Info_Included_Data[0] |= INPUT_CTLS_SEL_DATE;    //raise the flag

	vdGetTime((unsigned char*) xEMVStartData->TXN_Data.Time, sizeof(xEMVStartData->TXN_Data.Time));
	xEMVStartData->Info_Included_Data[0] |= INPUT_CTLS_SEL_TIME;    //raise the flag

	xEMVStartData->Info_Included_Data[0] |=  INPUT_SEL_TIME | INPUT_SEL_DATE; //raise the flag
}


EMV_ADK_INFO EMVTransaction(INT *iRet) //Setup Transaction
{
	EMV_ADK_INFO erg;
	char szAmount[14] = {0,};
	unsigned char ucReader=0;

	erg = NULL;

	EMV_CT_SELECTRES_TYPE xSelectResCT; // result of the contact selection process
	EMV_CT_TRANSAC_TYPE AdditionalTxDataCT;
	EMV_CT_SELECT_TYPE TxDataCT; //CT Data. Parameters for start transaction (application selection processing)
	EMV_CT_TRANSRES_TYPE xEMVTransResCT;
	EMV_CT_HOST_TYPE xOnlineInputCT;
	unsigned char ATR[64] = {0,};
	unsigned long ATRLen = 0;
	unsigned char cRet = 0;

	vdSetCTTransactionData(&TxDataCT, szAmount);   //Setup CT transaction data

	cRet = EMV_CT_SmartReset(0, ATR, &ATRLen);
	if(cRet != EMV_ADK_SMART_STATUS_OK || ATRLen < 1)
		return EMV_ADK_NO_CARD;

	erg=inApplicationSelectionCT(&TxDataCT, &xSelectResCT); //Call to EMV_CT_StartTransaction()

	if (erg != EMV_ADK_OK)
	{
		vdEndTransaction(APPL_SELECTION_ERROR_MESSAGE, ucReader, TRUE);
		return erg;
	}

	//vdGetAmount(szAmount, sizeof(szAmount)); //If only CT technology is used the amount could be obtained here and updated through vdSetAdditionalCTTransactionData. When CTLS is used then amount needs to be available for transaction setup
	*iRet = vdSetAdditionalCTTransactionData(&AdditionalTxDataCT, &TxDataCT); // copy additional TXN Data, known after final select
	if(*iRet != EMV_ADK_OK)
	{
		erg = *iRet;
		vdEndTransaction(APPL_SELECTION_ERROR_MESSAGE, ucReader, TRUE);
		return erg;
	}

	memset((void*) &xEMVTransResCT, 0, sizeof(xEMVTransResCT));

	dmsg("----------EMV_CT_ContinueOffline");
	erg = EMV_CT_ContinueOffline(&AdditionalTxDataCT, &xEMVTransResCT);  //EMV transaction (offline part including 1st cryptogram)

	dmsg("----------EMV_CT_ContinueOffline %#.2x", erg);

	while(erg == EMV_ADK_BADAPP)
	{
		// at this point, it is possible to select a new application
		TxDataCT.InitTXN_Buildlist = REUSE_LIST_REMOVE_AID;
		erg = EMV_CT_StartTransaction( &TxDataCT, &xSelectResCT);    //IN:  dummy OUT: additional select information
		dmsg("----------EMV_CT_StartTransaction %#.2x", erg);
		if(erg == EMV_ADK_OK)
		{
			memset((void*) &xEMVTransResCT,  0, sizeof(EMV_CT_TRANSRES_TYPE));
			erg = EMV_CT_ContinueOffline(&AdditionalTxDataCT, &xEMVTransResCT);
			dmsg("----------EMV_CT_ContinueOffline %#.2x", erg);
		}
	}

	vdLogTrxResultsCT(&xEMVTransResCT);

	switch(erg)
	{
		case EMV_ADK_ARQC:                // go online
			dmsg("----------Tx needs to go online!");
			memset((char*)&xOnlineInputCT,0,sizeof(xOnlineInputCT));
			//In xEMVTransRes are all the data needed for the online message
			// ***Call your host here and fill out xOnlineInputCT with the answer from the host
			erg = ContinueOnlineCT(&xOnlineInputCT,&xEMVTransResCT, iRet); //EMV transaction (handling of host response including 2nd cryptogram)
			if (erg!=EMV_ADK_OK)
			{
				vdEndTransaction("Transaction Error", ucReader, TRUE);
				return erg;
			}

			break;

		case EMV_ADK_TC:                  // approved offline
			dmsg("----------Tx approved offline!");
		break;

		case EMV_ADK_FALLBACK:            // perform fallback to magstripe. Add reaction to local fallback rules or other local chip
			dmsg("----------Appl selection FALLBACK!");
			vdEndTransaction(FALLBACK_MESSAGE, ucReader, FALSE);
			return erg;

		case EMV_ADK_AAC:                 // Denied offline
			dmsg("----------Tx Declined!");
			if((xEMVTransResCT.T_9F27_CryptInfo & EMV_ADK_CARD_REQUESTS_ADVICE) == EMV_ADK_CARD_REQUESTS_ADVICE)
			{
				// vStoreAdviceData(xTrxRec); // store the advice data if it need and it'll be sent before reconcilation
			}
		case EMV_ADK_AAR:
		case EMV_ADK_ABORT:
		case EMV_ADK_INTERNAL:
		case EMV_ADK_PARAM:
		case EMV_ADK_CARDERR:
		case EMV_ADK_CVM:
		case EMV_ADK_CARDERR_FORMAT:
		default:
			vdEndTransaction("Transaction Error", ucReader, TRUE);

			return erg;
	}

	vdLogTrxResultsCT(&xEMVTransResCT);
	setFinalCryptogram(&xEMVTransResCT);

	vdEndTransaction("Transaction Approved", ucReader, FALSE);

	return erg;
}

unsigned char PINinputCallback(void)
{

/*
	if(EMV_CT_SmartDetect(0) != EMV_ADK_SMART_STATUS_OK)
	{
		dmsg("----------CARD NOT PRESENT...");
		return(true);
	}
*/

	return(false);
}

#if 1
// ====================================================================================================================
unsigned char ucInputIPP_PIN(unsigned char pintype, unsigned char bypass, unsigned char* pucUnpredNb, unsigned char* pucPINResultData, unsigned short KeyLength, unsigned char* pucPINPublicKey, unsigned long ulPINPublicKeyExp)
{
	unsigned char ucResult=0;
	unsigned char tucAmount[6] = {0};
	unsigned long TAGR;
	unsigned char Buffer[32];
	unsigned short TAGL = 0;
	int iRet,inStatus;
	PINPARAMETER stPINParameter;
	PINRESULT stPINResult;
	unsigned char dummy[8];
	CHAR caDispBuff[200] = {0,};
	INT iTimer = -1;

	if(bypass){
		dmsg("-----start PIN entry: Bypass active: YES");
	}
	else{
		dmsg("-----start PIN entry: Bypass active: NO");
	}

	TAGR = 0x9F02; //Tx Amount

	memset(Buffer,0,sizeof(Buffer));
	EMV_CT_fetchTxnTags(EMV_ADK_FETCHTAGS_NO_EMPTY, &TAGR, 1, Buffer, 32, &TAGL);
	if(TAGL > 0)
		memcpy(tucAmount, &Buffer[3], 6);
	dmsg("-----Amount extracted: %02X%02X%02X%02X%02X%02X", Buffer[3], Buffer[4], Buffer[5], Buffer[6],Buffer[7], Buffer[8]);

	if((pintype == EMV_CT_PIN_INPUT_ENCIPHERED) || (pintype == EMV_CT_PIN_INPUT_PLAIN)){
		//PIN Entry
		iRet = iPS_SelectPINAlgo(EMV_PIN); //0x0A EMV PIN
		dmsg("-----iPS_SelectPINAlgo returned %d\n", iRet);

		stPINParameter.ucMin          = 4;
		stPINParameter.ucMax          = 6;
		stPINParameter.ucEchoChar     = '*';
		stPINParameter.ucDefChar      = ' ';
		stPINParameter.ucOption       = 0x08;
		iPS_SetPINParameter(&stPINParameter);

		dmsg("-----iPS_SetPINParameter returned %d\n", iRet);


		{
			LLONG llTotal = 0LL;
			tMtiMap *tpMap = mtiGetEmvParmInfo()->tpMainProcMap;
			CHAR caSwap[50] = {0,};

			llTotal = mtiMapGetLLong(tpMap, KEY_EXT_BIT_AMOUNT);
			llTotal += mtiMapGetLLong(tpMap, KEY_EXT_BIT_TIP);

			LLongToMoney(caSwap, llTotal, "RP", NULL);
			sprintf(caDispBuff, "%s\n%s\n%s", cpBothSideAlign("",caSwap), ENTER_PIN_MESSAGE," [      ]");
			mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_SHOW_MESSAGE,
						cpBothSideAlign("AMOUNT:",""), caDispBuff);
		}

		gotoxy(9,6);
		iRet = iPS_RequestPINEntry(0, dummy);
		dmsg("-----iPS_RequestPINEntry returned %d\n", iRet);
		memset(&stPINResult,0,sizeof(PINRESULT));
		dmsg("\n-----***%s :***\n\n", ENTER_PIN_MESSAGE);

		iTimer = mtiStartTimer(TIMEOUT_30S);

		for(;;){
			iRet = iPS_GetPINResponse (&inStatus, &stPINResult);
			do{
				iRet = iPS_GetPINResponse (&inStatus, &stPINResult);
				if ((inStatus==2)&&(stPINResult.encPinBlock[0]==KEY_ENTER)&&(stPINResult.nbPinDigits < stPINParameter.ucMin)){ //Key ENTER pressed before reaching the min number of PIN digits
					if ((bypass) && (stPINResult.nbPinDigits == 0)){
						dmsg("-----PIN Bypass from customer");
						iRet = iPS_CancelPIN();
						mtiStopTimer(iTimer);
						return EMV_CT_PIN_INPUT_BYPASS;
					}
					else{
						//dmsg("-----%d DIGITS ENTERED SO FAR. PIN MUST HAVE AT LEAST 4 DIGITS!!!", stPINResult.nbPinDigits);
						SVC_WAIT(1);
					}
				}
				if ((inStatus == 0x05)||(inStatus == 0x0A)){
					iRet = iPS_CancelPIN();
					dmsg("-----iPS_CancelPIN returned %d %02X\n", iRet, stPINResult.encPinBlock[0]);
					mtiStopTimer(iTimer);
					return EMV_CT_PIN_INPUT_ABORT;
				}
				if (inStatus == 0x0C){
					iRet = iPS_CancelPIN();
					dmsg("-----iPS_CancelPIN  returned %d Time Out\n", iRet);
					mtiStopTimer(iTimer);
					return EMV_CT_PIN_INPUT_ABORT;
				}
				if(mtiIsTimeout(iTimer) == TRUE)
				{
					dmsg("External Timer Out");
					mtiStopTimer(iTimer);
					iRet = iPS_CancelPIN();
					return EMV_CT_PIN_INPUT_TIMEOUT;
				}
			}while(inStatus!=0);
			break;
		}

		ucResult = EMV_CT_Send_PIN_Offline(pucPINResultData);
		ucResult = EMV_ADK_SMART_STATUS_OK;
		dmsg("-----result of PIN offline: %d ***", ucResult);
		dmsg("-----pucPINResultData: %x %x ===", pucPINResultData[0],pucPINResultData[1]);
		switch(ucResult){
		  case EMV_ADK_SMART_STATUS_OK:

			if(pucPINResultData[0] == 0x63){//||((pucPINResultData[0] == 0x64))){
				if(pucPINResultData[1] == 0xC1){
					sprintf((char *)Buffer,"%s","INVALID PIN\n\nLAST TRY!!!");
					dmsg("-----Invalid PIN. Last Try!!!\n");
					mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_LAST_ATTEMP_PIN, NULL, NULL);
				}
				else{
					dmsg("-----Invalid PIN. Try Again\n");
					sprintf((char *)Buffer,"%s","INVALID PIN\n\nTRY AGAIN");
					mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_WRONG_PIN, NULL, NULL);
				}
				SVC_WAIT(2000);
			}

			if((pucPINResultData[0] == 0x90)&&((pucPINResultData[1] == 0x00))){
				dmsg("-----PIN OK!!!\n");
				mtiMapPutInt(mtiGetEmvParmInfo()->tpMainProcMap, KEY_EXT_BIT_PEM, PEM_IC_PIN);
				mtiMapPutInt(mtiGetEmvParmInfo()->tpMainProcMap, KEY_EXT_BIT_COMBINED_SIGN_FLAG, FALSE);
			}
			mtiStopTimer(iTimer);
			return(EMV_CT_PIN_INPUT_OKAY); // need to check SW1 SW2

		 default:
			mtiStopTimer(iTimer);
			return(EMV_CT_PIN_INPUT_COMM_ERR);
		}
	}
	else if(pintype == EMV_CT_PIN_INPUT_ONLINE) //|| (pintype == EMV_CTLS_PIN_INPUT_ONLINE))
	{
		dmsg("-----result of PIN entry: OK\n");
		return(EMV_CT_PIN_INPUT_OKAY);
	}
	else if(pintype == EMV_CT_CVM_CUSTOM)
	{
		dmsg("-----result of custom CVM method: OK");
		return(EMV_CT_PIN_INPUT_OKAY);
	}
	if(iTimer != -1)
		mtiStopTimer(iTimer);
	return(EMV_CT_PIN_INPUT_OTHER_ERR);
} // ucInputPIN

#else
unsigned char ucInputPIN(unsigned char pintype, unsigned char bypass, unsigned char* pucUnpredNb, unsigned char* pucPINResultData,
		unsigned short KeyLength, unsigned char* pucPINPublicKey, unsigned long ulPINPublicKeyExp)
{
	unsigned char ucResult;
	int inResult = 0;
	unsigned long BypassKey = 0;
	unsigned char tucAmount[6] = {0, };
	char szFormattedAmount[14] = {0, };
//	guiPinParam pinParam;
	unsigned long TAGR;
	unsigned char Buffer[32];
	unsigned short TAGL = 0;
	unsigned char retryCount;
	char *pszPINMsg = ENTER_PIN_MESSAGE;
	EMV_CT_ENTER_PIN_TYPE x; 
	LLONG llAmount;
	
	//Refer to EMV_CT_ENTER_PIN_TYPE structure for information about other parameters.

	memset(&x,0x00,sizeof(x));

	if(bypass)
	{
		dmsg("----------start PIN entry: Bypass active: YES");
		BypassKey = 13; // Bypass activated with Enter Key 0 digits
	}
	else
	{
		dmsg("----------start PIN entry: Bypass active: NO");
		BypassKey = 0;
	}
	/*
	pinParam.ucPinAlgo = EMV_PIN;
	pinParam.ucAutoEnter = 0;
	pinParam.ucClearAll = 1;
	pinParam.ulEchoChar = '*';
	pinParam.ulBypassKey = BypassKey;
	pinParam.lTimeoutMs = PIN_INPUT_TIMEOUT * 1000;
	pinParam.callback = PINinputCallback;
	pinParam.currency = gszCurrecyCode;
	*/

	TAGR = 0x9F02; //Tx Amount

	/*if(pintype == EMV_CTLS_PIN_INPUT_ONLINE){
	TAGL = 0;
	EMV_CTLS_fetchTxnTags(0, &TAGR, 1, Buffer, 32, &TAGL);
	if(TAGL > 0)
	memcpy(tucAmount, &Buffer[3], 6);
	}
	else{*/
	EMV_CT_fetchTxnTags(EMV_ADK_FETCHTAGS_NO_EMPTY, &TAGR, 1, Buffer, 32, &TAGL);
	if(TAGL > 0)
		memcpy(tucAmount, &Buffer[3], 6);
	// }
	dmsg("----------Amount extracted: %02X%02X%02X%02X%02X%02X", Buffer[3], Buffer[4], Buffer[5], Buffer[6],Buffer[7], Buffer[8]);

	inFormatAmountForDisplay(tucAmount, szFormattedAmount + strlen(szFormattedAmount), sizeof(szFormattedAmount));

	llAmount = mtiAtoll(12, (UCHAR *)szFormattedAmount);

	LLongToMoney(szFormattedAmount, llAmount, gszCurrecyCode, NULL);

	//sprintf((char *)szFormattedAmount,"%s %lld",gszCurrecyCode,); //Add currency and confirmation text
	//pinParam.amount = szFormattedAmount;

	dmsg("### PIN AMOUNT MSG : %s", szFormattedAmount);

	if((pintype == EMV_CT_PIN_INPUT_ENCIPHERED) || (pintype == EMV_CT_PIN_INPUT_PLAIN))
	{
		// This solution has to be used for Verix: The PIN Entry must be done by EMV-ADK due to device ownership requirements.
		// For V/OS this can also be used but it is not required.

		// BYTE 1
		x.pcHtml = "@helper_pin.html";
		x.ucPinAlgo = EMV_PIN;
		x.ucAutoEnter = 0;
		x.ucClearAll = 1;
		x.ulEchoChar = '*';
		x.ulBypassKey = BypassKey;
		x.lTimeoutMs = PIN_INPUT_TIMEOUT * 1000;
		//x.ucAbortOnPINEntryTimeOut = GENERALTIMEOUT * 1000;
		x.ucEnableCallback = 1;

		// BYTE 2
		x.ucMaxlen = 14;
		x.ucPosX = 30;
		x.ucPosY = 70;
		x.ucInputType = EMV_CT_PIN_INPUT_SECURE;

//		x.currency = gszCurrecyCode;
		x.pcAmount = szFormattedAmount;
		//x.pcMsgPIN = pszPINMsg;
		x.pcMsgPIN = pinDispMsg;

#if 0
		x.InfoIncludedData[0] = 0xFF;
		x.InfoIncludedData[1] = 0x80;
		x.InfoIncludedData[2] = 0x12;
#else
		x.InfoIncludedData[0] = EMV_CT_ENTER_PIN_HTML | EMV_CT_ENTER_PIN_PIN_ALGO |
			EMV_CT_ENTER_PIN_AUTO_ENTER | EMV_CT_ENTER_PIN_CLEAR_ALL |\
			EMV_CT_ENTER_PIN_ECHO_CHAR | EMV_CT_ENTER_PIN_BYPASS_KEY | 
			EMV_CT_ENTER_PIN_TIMEOUT | EMV_CT_ENTER_PIN_ENABLE_CALLBACK;

		x.InfoIncludedData[1] = EMV_CT_ENTER_PIN_MAX_LEN | EMV_CT_ENTER_PIN_INPUT_TYPE |
			EMV_CT_ENTER_PIN_POS_X | EMV_CT_ENTER_PIN_POS_Y;

		x.InfoIncludedData[2] = EMV_CT_ENTER_PIN_AMOUNT | EMV_CT_ENTER_PIN_MSG_PIN;
#endif

#if 1
		//while(TRUE)
		//{
			ucResult = EMV_CT_Enter_PIN_extended(&x);

			dmsg("EMV_CT_Enter_PIN_extended ucResult[%d]", ucResult);
			if(ucResult == EMV_CT_PIN_INPUT_BYPASS)
				dmsg("----------PIN Bypass from customer");

			if(ucResult != EMV_CT_PIN_INPUT_OKAY)
				return(ucResult);

			ucResult = EMV_CT_Send_PIN_Offline(pucPINResultData);
			dmsg("----------EMV_CT_Send_PIN_Offline RESULT : [%d] ***", ucResult);
			dmsg("----------EMV_CT_Send_PIN_Offline : SW [%0X%0X] ===", pucPINResultData[0],pucPINResultData[1]);

			if(EMV_ADK_SMART_STATUS_OK==ucResult)
			{
				if(pucPINResultData[0] == 0x63)
				{
					retryCount = (pucPINResultData[1] & 0xCF);

					if(1==retryCount)
					{
						dmsg("INVALID PIN - Last Try!!!");
						pinDispMsg = ENTER_PIN_MESSAGE_LASTTRY;
						//pszPINMsg = ENTER_PIN_MESSAGE_LASTTRY;
						
						//mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_WRONG_PIN, NULL, NULL);
					}
					else if(0==retryCount)
					{
						dmsg("OFFLINE PIN BLOCKED !!!");
						return(EMV_CT_PIN_INPUT_OTHER_ERR);
					}
					else
					{
						dmsg("INVALID PIN - Try Again!!!");
						pinDispMsg = ENTER_PIN_MESSAGE_LASTTRY;
						//pszPINMsg = ENTER_PIN_MESSAGE_RETRY;
					}

					return EMV_CT_PIN_INPUT_OKAY;
				}
				else
				{
					if((pucPINResultData[0] == 0x90) &&((pucPINResultData[1] == 0x00)))
					{
						dmsg("Entered PIN is Okay.");
						return(EMV_CT_PIN_INPUT_OKAY); // need to check SW1 SW2
					}
					else
					{
						return(EMV_CT_PIN_INPUT_OTHER_ERR);
					}
				}
			}
			else if(EMV_CT_PIN_INPUT_BYPASS==ucResult)
			{
				return EMV_CT_PIN_INPUT_BYPASS;
			}
			else
			{
				return(EMV_CT_PIN_INPUT_COMM_ERR);
			}
		//}
#endif
		return(EMV_CT_PIN_INPUT_COMM_ERR);
	}
	else if(pintype == EMV_CT_PIN_INPUT_ONLINE) //|| (pintype == EMV_CTLS_PIN_INPUT_ONLINE))
	{
		/*
		// with Online PIN of CTLS there is no need to check card insertion
		//if(pintype == EMV_CTLS_PIN_INPUT_ONLINE)
		//	pinParam.callback = NULL;
		ucResult = UIEnterPin(&pinParam);
		dmsg("----------result of PIN entry: %d ===", ucResult);
		if(ucResult != EMV_CT_PIN_INPUT_OKAY)
		return(ucResult);
		// Online PIN needs to be fetched by the local/domestic VSS script handling the online PIN block
		// transmission towards the acquirer/network provider
		*/
		dmsg("pintype -> EMV_CT_PIN_INPUT_ONLINE");

		return(EMV_CT_PIN_INPUT_OKAY);

	}
	else if(pintype == EMV_CT_CVM_CUSTOM)
	{
		// custom CVM method
		dmsg("----------result of custom CVM method: OK");
		return(EMV_CT_PIN_INPUT_OKAY);
	}

	return(EMV_CT_PIN_INPUT_OTHER_ERR);
}

#endif

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
	INT iRet = MTI_EMV_TR_SUCCESS;
	EMV_ADK_INFO erg = EMV_ADK_OK;

	g_bReqPINOption = FALSE;

	bUserCanceled = FALSE;

	// Display message "Please wait"
	mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_PLEASE_WAIT, NULL, NULL);

	InitCTFramework();

	mtiEmvSetParam(FALSE);

	erg =  EMVTransaction(&iRet);

	if(erg == EMV_ADK_OK)	//TC ONLINE
	{
		iRet = MTI_EMV_TR_SUCCESS;
	}
	else	//OTHERS
	{
		switch(erg)
		{
			case EMV_ADK_APP_BLOCKED:
				iRet = MTI_EMV_TR_APP_BLOCKED;
				break;
			case EMV_ADK_NO_CARD:
			case EMV_ADK_FALLBACK:
				iRet = MTI_EMV_TR_FALLBACK;
				break;
			case EMV_ADK_TC:
				iRet = MTI_EMV_TR_OFL_SUCCESS;
				break;
			case EMV_ADK_USR_BCKSPC_KEY_PRESSED:
			case EMV_ADK_CVM:
				iRet = MTI_EMV_TR_CANCELLED;
				break;
			case EMV_ADK_ABORT:
				if(bUserCanceled == TRUE)
				{
					iRet = MTI_EMV_TR_CANCEL;
				}
				else
					iRet = MTI_EMV_TR_UNKNOWN_ERROR;
				break;
			case EMV_ADK_NOAPP:
				iRet = MTI_EMV_TR_AID_SELECT_ERROR;
				break;
			case EMV_ADK_AAC:
				iRet = MTI_EMV_TR_CARD_AAC;
				break;
			default:
				iRet = MTI_EMV_TR_UNKNOWN_ERROR;
		}
		dmsg("EMV Transaction is Failed... Error Code[0x%.2x] iRet[%d]", erg, iRet);
	}
/*
 *
#define 						2011
#define MTI_EMV_TR_INVALID_CARD							2012
#define MTI_EMV_TR_UNKNOWN_ERROR						2013
#define MTI_EMV_TR_FALLBACK								2014
#define 							2015

 */

	EMV_CT_Exit_Framework();

#if 0
	//XML DBG
	{
		CHAR buff[1024] = {0,};
		FILE *xmlFile=NULL;
		xmlFile = fopen("EMV_APPLICATIONS.XML", "r");
		fread(buff, 1, sizeof(buff), xmlFile);
		dmsg("\n\nXML[%s]\n\n",buff);
		fclose(xmlFile);
	}
#endif

	return iRet;
}

static INT isExistFile(char *fileName)
{
	struct stat fileInfo;
	CHAR caPath[50] = {0,};
	INT iRet = FALSE;

	sprintf(caPath, "I:1/%s", fileName);
	stat(caPath, &fileInfo);

	if(fileInfo.st_size > 0)
		iRet = TRUE;

	return iRet;
}

INT mtiEmvSetParam(INT bDoClear)
{
	INT iRet = RTN_SUCCESS;
	EMV_ADK_INFO erg = EMV_ADK_OK;

	//Clear All XML Files
	if(bDoClear!= FALSE)
	{
		remove(XML_FILE_TERM);
		remove(XML_FILE_APP);
		remove(XML_FILE_KEY);
		SVC_WAIT(100);

		dmsg("EMV XML Files all Removed.");
		ulInitOptions |= EMV_CT_INIT_OPT_DELETE_ALL;

		if(TRUE==bDoClear)
			return RTN_SUCCESS;
	}

	//XML Exist Check
	//Terminal Configuration
	iRet = RTN_SUCCESS;
	if((isExistFile(XML_FILE_APP) == FALSE) || (isExistFile(XML_FILE_KEY) == FALSE))
	{
		dmsg("Config EMV TERMINAL DATA");
		erg = SetCTTerminalData();
		if(erg != EMV_ADK_OK)
		{
			dmsg("----------Terminal Configuration error [x%.2x]", erg);
			EMV_CT_Exit_Framework();
			iRet = RTN_ERROR;
		}

		//Application Configuration + CAPKEY Store
		if(iRet == RTN_SUCCESS)
		{
			dmsg("Config EMV APP DATA");
			erg = SetCTApplicationData();
			if(erg != EMV_ADK_OK)
			{
				dmsg("----------Application Configuration error [x%.2x]", erg);
				EMV_CT_Exit_Framework();
				iRet = RTN_ERROR;
			}

			//Store Configuration to XML
			erg = EMV_CT_ApplyConfiguration(0);

			if(erg != EMV_ADK_OK)
			{
				dmsg("----------EMV_CT_ApplyConfiguration error [x%.2x]", erg);
				EMV_CT_Exit_Framework();
				iRet = RTN_ERROR;
			}
		}
	}
	//xmlFileDbg("I:1/EMV_TERMINAL.XML");
	//xmlFileDbg("I:1/EMV_APPLICATIONS.XML");
	xmlFileDbg("I:1/EMV_KEYS.XML");

	ulInitOptions = EMV_CT_INIT_OPT_CONFIG_MODE | EMV_CT_INIT_OPT_L1_DUMP;
	return iRet;
}

VOID mtiEmvFinalize(tEmvParamInfo *tpEmvParamInfo)
{
	mtiMemset(tpEmvParamInfo, 0, sizeof(tpEmvParamInfo));
	EMV_CT_Exit_Framework();
}




