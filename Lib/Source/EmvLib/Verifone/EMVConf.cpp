//----------------------------------------------------------------------------
//
// EMVConf.cpp -	CT and CTLS Terminal and Application configuration APIs
//              	November 2015
//
//----------------------------------------------------------------------------
/*****************************************************************************
	 Copyright (C) 2015 by VeriFone Inc. All rights reserved.

	 No part of this software may be used, stored, compiled, reproduced,
	 modified, transcribed, translated, transmitted, or transferred, in
	 any form or by any means  whether electronic, mechanical,  magnetic,
	 optical, or otherwise, without the express prior written permission
							  of VeriFone, Inc.
******************************************************************************/
#include "libemv.h"
#include "apMem.h"


using namespace std;

static UCHAR ucaTID[8] = {0,};

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

#if 1
static EMV_ADK_INFO cfgCardCT(tMtiMap *tpAidParamMap)
{
	INT iLen;
	UCHAR *ucpData;
	EMV_CT_APPLI_TYPE aid;
	EMV_CT_APPLIDATA_TYPE aidData;

	mtiMemset(&aid, 0x00, sizeof(EMV_CT_APPLI_TYPE));
	mtiMemset(&aidData, 0x00, sizeof(EMV_CT_APPLIDATA_TYPE));

	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_AID_TERMINAL, &iLen);

	if(ucpData)
	{
		aid.aidlen = (UCHAR) iLen;
		memcpy(aid.AID, ucpData, iLen);
	}

	//Update some parameters for demo purposes
	/*
	memcpy(aidData.App_CountryCodeTerm, "\x08\x40", 2);
	aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_COUNTRY_CODE;
	*/

	//MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_APPLI_VERSION_NUMBER_TERM, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.VerNum, ucpData, iLen);
		aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_VERSION;
	}

	//MTI_EMV_TAG_TERMINAL_FLOOR_LIMIT
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TERMINAL_FLOOR_LIMIT, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		if(memcmp(ucpData, "\x00\x00\x00\x00", 4) == 0)
			memcpy(aidData.FloorLimit, ucpData, 1);
		else
			memcpy(aidData.FloorLimit, ucpData, iLen);
		aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_FLOOR_LIMIT;
	}

	//MTI_EMV_TAG_INT_THRESHOLD_VALUE_BIASED_RAND_SEL
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_THRESHOLD_VALUE_BIASED_RAND_SEL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.Threshhold, ucpData, iLen);
		aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_THRESH;
	}

	//MTI_EMV_TAG_INT_TARGET_PERC_RAND_SEL
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TARGET_PERC_RAND_SEL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(&aidData.TargetPercentage, ucpData, iLen);
		aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_TARGET;
	}

	//MTI_EMV_TAG_INT_MAX_TARGET_PERC_BIASED_RAND_SEL
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_MAX_TARGET_PERC_BIASED_RAND_SEL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(&aidData.MaxTargetPercentage, ucpData, iLen);
		aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_MAXTARGET;
	}

	//MTI_EMV_TAG_INT_DEFAULT_DDOL
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_DEFAULT_DDOL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		aidData.Default_DDOL.dollen = (UCHAR) iLen;
		memcpy(aidData.Default_DDOL.DOL, ucpData, iLen);
		aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_DDOL;
	}

	//MTI_EMV_TAG_INT_TAC_DENIAL
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TAC_DENIAL, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TACDenial, ucpData, iLen);
		aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_TAC_DENIAL;
	}
/*
	dmsg("TAC_DENIAL DUMP");
	dbuf(NULL, ucpData, iLen);
*/

	//MTI_EMV_TAG_INT_TAC_ONLINE
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TAC_ONLINE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TACOnline, ucpData, iLen);
		aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_TAC_ONLINE;
	}
/*
	dmsg("TAC_ONLINE DUMP");
	dbuf(NULL, ucpData, iLen);
*/

	//MTI_EMV_TAG_INT_TAC_DEFAULT
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_INT_TAC_DEFAULT, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.TACDefault, ucpData, iLen);
		aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_TAC_DEFAULT;
	}
/*
	dmsg("TAC_DEFAULT DUMP");
	dbuf(NULL, ucpData, iLen);
*/

/*
	memcpy(aidData.MerchIdent, "\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31", 15);
	aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_MERCHANT_IDENT;

	//TID
	memcpy(aidData.TermIdent, ucaTID, M_SIZEOF(EMV_CT_APPLIDATA_TYPE, TermIdent));
	aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_TID;

	memcpy(aidData.App_FlowCap,"\x1F\x1F\x22\x00\x00",5);			//B3b6 activates amount confirmation for cards with CVM signature active
	aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_FLOW_CAPS;	//Availability bit
*/
	//EMV_CT_CHECK_INCONS_TRACK2_PAN | EMV_CT_CONF_AMOUNT_PIN | EMV_CT_DOMESTIC_CHECK = 0x1C
	aidData.App_FlowCap[0] = TRANSACTION_LOG | FORCE_RISK_MANAGEMENT | PIN_BYPASS;
	aidData.App_FlowCap[1] = EMV_CT_CHECK_INCONS_TRACK2_PAN | EMV_CT_CONF_AMOUNT_PIN | EMV_CT_DOMESTIC_CHECK;
	aidData.App_FlowCap[2] = EMV_CT_SDA_SELECTED_TVR_ON;
	aidData.App_FlowCap[3] = 0x00;
	aidData.App_FlowCap[4] = 0x00;
	aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_FLOW_CAPS;	//Availability bit
	//dbuf("EMV_CT_INPUT_APL_FLOW_CAPS", aidData.App_FlowCap, 5);

	//TERM CAP
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_TERMINAL_CAPABILITIES, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.App_TermCap, ucpData, iLen);
		dbuf("MTI_EMV_TAG_TERMINAL_CAPABILITIES", ucpData, iLen);
		aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_TERM_CAPS;
	}

	//TERM ADD CAP
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.App_TermAddCap, ucpData, iLen);
		dbuf("MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES", ucpData, iLen);
		aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_ADD_TERM_CAPS;
	}

	//CAT CODE
	ucpData = mtiMapGetBytes(tpAidParamMap, MTI_EMV_TAG_MERCHANT_CATEGORY_CODE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(aidData.BrKey, ucpData, iLen);
		aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_MERCHANT_CATCODE;
	}

	aidData.CDAProcessing = CDA_EMV_MODE_1;
	aidData.Info_Included_Data[3]|=EMV_CT_INPUT_APL_CDA;

	return EMV_CT_SetAppliData(EMV_ADK_SET_ONE_RECORD, &aid, &aidData);
}

static EMV_ADK_INFO storeCAPKey_CT(UCHAR *ucpRID)
{
	EMV_ADK_INFO emvRet = EMV_ADK_OK;
	INT iIdx, iRet = 0, iKeyIdxListNum = 0;
	UCHAR ucaRID[5] = {0,};
	UCHAR ucpKeyIndexList[100];
	tMtiMap tMapCAPKey;
	tMtiMap *tpMapCAPKey = &tMapCAPKey;

	mtiMapInit(tpMapCAPKey);

	//Get RID 5 Byte
	memcpy(ucaRID, ucpRID, 5);	//Only Use 5 Bytes RID Value in VERIFONE EMV

	mtiMapPutBytes(tpMapCAPKey, MTI_EMV_TAG_AID_TERMINAL, ucpRID, 7);

	iGetCAPKeyIdxList(tpMapCAPKey, ucpKeyIndexList, sizeof(ucpKeyIndexList), &iKeyIdxListNum);
	//dbuf(NULL, ucpKeyIndexList, sizeof(ucpKeyIndexList));

	for(iIdx = 0; iIdx < iKeyIdxListNum; iIdx++)
	{
		EMV_CT_CAPKEY_STRUCT thisKey={0};
		UCHAR *ucpKeyData = NULL, *ucpExponent = NULL;
		INT iKeyLen = 0, iExpLen = 0;

		mtiMapPutBytes(tpMapCAPKey, MTI_EMV_TAG_CA_PUBLIC_KEY_INDEX_CARD, &ucpKeyIndexList[iIdx], sizeof(UCHAR));

		iRet = mtiGetEmvParmInfo()->FuncInputCAPKCallback(tpMapCAPKey);

		if(iRet)
		{
			ucpExponent = mtiMapGetBytes(tpMapCAPKey, MTI_EMV_TAG_INT_CAPK_EXPONENT, &iExpLen);
			ucpKeyData = mtiMapGetBytes(tpMapCAPKey, MTI_EMV_TAG_INT_CAPK_MODULUS, &iKeyLen);

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

		    emvRet = EMV_CT_StoreCAPKey(EMV_ADK_SET_ONE_RECORD, &thisKey);
		    //dbuf("CAPK RID", ucaRID, sizeof(ucaRID));
		    //dmsg("Set CT Set CAP Keys Data KeyIdx[0x%.2x]: (%#.2x)\n", ucpKeyIndexList[iIdx], emvRet);
//		    if (emvRet != EMV_ADK_OK)
//		        break;
		}

		if(tpMapCAPKey != NULL)
		{
			mtiMapClear(tpMapCAPKey);
			mtiMapPutBytes(tpMapCAPKey, MTI_EMV_TAG_AID_TERMINAL, ucpRID, 7);
		}
	}

	return emvRet;
}

// =============================================================================
EMV_ADK_INFO SetCTApplicationData()
{
	EMV_ADK_INFO erg=EMV_ADK_OK;
	tMtiMap tMapAidList;
	tMtiMap *tpMapAidList = &tMapAidList;

	tMtiMap tMapAidParam;
	tMtiMap *tpMapAidParam = &tMapAidParam;

	INT iRet = 0, i = 0, iKey = 0, iLen = 0;
	UCHAR *ucpValue = NULL;

	// Clear AID
	erg = EMV_CT_SetAppliData(EMV_ADK_CLEAR_ALL_RECORDS, NULL, NULL);
	if (erg != EMV_ADK_OK)
	{
		dmsg("----------Clear CT Appl Data error: (%#.2x)", erg);
		return erg;
	}

	mtiMapInit(tpMapAidList);
	mtiMapInit(tpMapAidParam);

	iRet = mtiGetEmvParmInfo()->FuncInputAIDListCallback(tpMapAidList);

	if (iRet)
	{
		INT isCleared = FALSE;
		// AID Copy
		EMV_CT_APPLI_TYPE aidData;
		EMV_CT_APPLIDATA_TYPE appData;
		for (i = 0; i < tpMapAidList->iMapCount; i++)
		{
			iKey = MemCheck(tpMapAidList, i);
			iLen = 0;
			ucpValue = mtiMapGetBytes(tpMapAidList, iKey, &iLen);

			mtiMemset(&aidData, 0x00, sizeof(aidData));
			mtiMemset(&appData, 0x00, sizeof(appData));
			if (iLen > 0)
			{
				//Application Data Set
				mtiMapPutBytes(tpMapAidParam, MTI_EMV_TAG_AID_TERMINAL, ucpValue, iLen);
				mtiMemcpy(aidData.AID, ucpValue, iLen);
				aidData.aidlen = iLen;
				iRet = mtiGetEmvParmInfo()->FuncInputAIDParamCallback(tpMapAidParam);

				if(iRet)
				{
					erg = cfgCardCT(tpMapAidParam);
					dmsg("Set CT AID Data IDX[%d] ErrorCode[0x%.2x]", i, erg);
				}

				if(isCleared == FALSE)	//Only Working Once...
				{
					// Clear CAP keys
					erg = EMV_CT_StoreCAPKey(EMV_ADK_CLEAR_ALL_RECORDS, NULL);
					dmsg("Set CT Clear CAP Keys Data: (%#.2x)\n", erg);
					isCleared = TRUE;
				}

				//CAP Key Store
				storeCAPKey_CT(ucpValue);
			}
			mtiMapClear(tpMapAidParam);

			//Application Config Debug
#if 0
			//Debug Set Application Config
			EMV_CT_GetAppliData(EMV_ADK_READ_AID, &aidData, &appData);
			dmsg("-------------------------------");
			dmsg("AID");
			dbuf(NULL, aidData.AID, aidData.aidlen);
			dmsg("*******************************");
			dmsg("TAC_DENIAL");
			dbuf(NULL, appData.TACDenial, 5);
			dmsg("*******************************");
			dmsg("TAC_DEFAULT");
			dbuf(NULL, appData.TACDefault, 5);
			dmsg("*******************************");
			dmsg("TAC_ONLINE");
			dbuf(NULL, appData.TACOnline, 5);
			dmsg("-------------------------------");
#endif
		}
	}



	mtiMapClear(tpMapAidList);

	dmsg("----------Set CT Application Data: OK");

	return erg;
}


//==============================================================================================================================
//Information change/added here will be updated in the EMV_Terminal.xml configuration file.
EMV_ADK_INFO SetCTTerminalData(void)
{
	tMtiMap tMapParam;
	tMtiMap *tpMapParam = &tMapParam;

	EMV_ADK_INFO erg = EMV_ADK_OK;
	EMV_CT_TERMDATA_TYPE termData = {0};

	UCHAR *ucpData;
	INT iRet = 0, iLen;

	mtiMapInit(tpMapParam);
	iRet = mtiGetEmvParmInfo()->FuncInputGlobalParamCallback(tpMapParam);
	if(iRet == NULL)
	{
		dmsg("tpMapParam mtiGetEmvParmInfo is failed...");
		mtiMapClear(tpMapParam);
		return EMV_ADK_ABORT;
	}

	//Terminal type: attended terminal, offline with online capability
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TERMINAL_TYPE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		termData.TermTyp = *ucpData;
		termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_TYPE;
	}

	//MTI_EMV_TAG_TERMINAL_COUNTRY_CODE
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TRANSACTION_CURRENCY_CODE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(termData.TermCountryCode, ucpData, iLen);
		termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_COUNTRY_CODE;
	}

	//MTI_EMV_TAG_INT_CURRENCY_CODE
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TRANSACTION_CURRENCY_CODE, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(termData.CurrencyTrans, ucpData, iLen);
		termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_CURRENCY;
	}

	//Currency exponent
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TRANSACTION_CURRENCY_EXPONENT, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		termData.ExpTrans = *ucpData;
		termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_EXP_CURRENCY;
	}

	//Language
	termData.SuppLang[0] = EMV_ADK_LANG_ENGLISH; //Up to EMV_ADK_MAX_LANG can be set up
	termData.Info_Included_Data[1] |= EMV_CT_INPUT_TRM_LANGUAGES;

	//IFD SERIAL NUM
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_IFD_SERIAL_NUMBER, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(termData.IFDSerialNumber, ucpData, iLen);
		termData.Info_Included_Data[1] |= EMV_CT_INPUT_TRM_IFD_SERIAL;
	}

	//TID for Application Configuration
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TERMINAL_IDENTIFICATION, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(termData.TermIdent, ucpData, iLen);
		termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_ID;
	}

	//TERM CAP
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_TERMINAL_CAPABILITIES, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(termData.TermCap, ucpData, iLen);
		dbuf("MTI_EMV_TAG_TERMINAL_CAPABILITIES", ucpData, iLen);
		termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_CAPABILITIES;
	}

	//TERM ADDCAP
	ucpData = mtiMapGetBytes(tpMapParam, MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES, &iLen);
	if(ucpData != NULL && iLen > 0)
	{
		memcpy(termData.TermAddCap, ucpData, iLen);
		dbuf("MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES", ucpData, iLen);
		termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_ADD_CAPS;
	}

	mtiMapClear(tpMapParam);

	erg = EMV_CT_SetTermData(&termData);

	if (erg != EMV_ADK_OK){
	   dmsg("----------Set CT Term Data Error (%#.2x)", erg);
	   EMV_CT_Exit_Framework();
	   return erg;
	}

	dmsg("----------Set CT Term Data OK");

	return erg;
}
#else
static unsigned char cfgVisaCTLS();

// =============================================================================
static unsigned char cfgMasterCardCT()
{
  EMV_CT_APPLI_TYPE aid = { 7, {0xA0,0x00,0x00,0x00,0x04,0x10,0x10} };
  EMV_CT_APPLIDATA_TYPE aidData;

  memset(&aidData,0,sizeof(EMV_CT_APPLIDATA_TYPE));
  //Update some parameters for demo purposes
  memcpy(aidData.App_CountryCodeTerm, "\x08\x40", 2);
  aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_COUNTRY_CODE;
  memcpy(aidData.FloorLimit, "\x00\x00\x2A\xF8", 4);  //110
  aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_FLOOR_LIMIT;
  memcpy(aidData.MerchIdent, "\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31", 15);
  aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_MERCHANT_IDENT;
  memcpy(aidData.TermIdent, "\x31\x31\x31\x31\x31\x31\x31\x31", 8);
  aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_TID;
  memcpy(aidData.App_FlowCap,"\x1F\x1F\x22\x00\x00",5);			//B3b6 activates amount confirmation for cards with CVM signature active
  aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_FLOW_CAPS;	//Availability bit

  return EMV_CT_SetAppliData(EMV_ADK_SET_ONE_RECORD, &aid, &aidData);
}

// =============================================================================
static unsigned char cfgVisaCT()
{
  EMV_CT_APPLI_TYPE aid = { 7, {0xA0,0x00,0x00,0x00,0x03,0x10,0x10} };
  EMV_CT_APPLIDATA_TYPE aidData;

  memset(&aidData,0,sizeof(EMV_CT_APPLIDATA_TYPE));
  //Update some parameters for demo purposes
  memcpy(aidData.App_CountryCodeTerm, "\x08\x40", 2);
  aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_COUNTRY_CODE;
  memcpy(aidData.FloorLimit, "\x00\x00\x2A\xF8", 4);  //110
  aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_FLOOR_LIMIT;
  memcpy(aidData.MerchIdent, "\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31", 15);
  aidData.Info_Included_Data[1]|=EMV_CT_INPUT_APL_MERCHANT_IDENT;
  memcpy(aidData.TermIdent, "\x31\x31\x31\x31\x31\x31\x31\x31", 8);
  aidData.Info_Included_Data[0]|=EMV_CT_INPUT_APL_TID;
  memcpy(aidData.App_FlowCap,"\x1F\x1F\x22\x00\x00",5);			//B3b6 activates amount confirmation for cards with CVM signature active
  aidData.Info_Included_Data[2]|=EMV_CT_INPUT_APL_FLOW_CAPS;	//Availability bit

  return EMV_CT_SetAppliData(EMV_ADK_SET_ONE_RECORD, &aid, &aidData);
}

// =============================================================================
static unsigned char cfgMasterCardCTLS()
{
	EMV_CTLS_APPLI_TYPE aid = { 7, {0xA0,0x00,0x00,0x00,0x04,0x10,0x10} };
	EMV_CTLS_APPLIDATA_TYPE aidData = {0};

	memset(&aidData, 0, sizeof(EMV_CTLS_APPLIDATA_TYPE));
	aidData.CL_Modes = CL_MC_PAYPASS_COMBINED;						//This parameter must be set before calling EMV_CTLS_SetAppliData()
	aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_SUPPORTED_CTLS;

	memcpy(aidData.App_CountryCodeTerm, "\x08\x40", 2);				//Few parameters are updated here only for demo purposes
	aidData.Info_Included_Data[2]|=INPUT_CTLS_APL_COUNTRY_CODE;
	memcpy(aidData.FloorLimit, "\x00\x00\x2A\xF8", 4);				//110
	aidData.Info_Included_Data[1]|=INPUT_CTLS_APL_FLOOR_LIMIT;
	memcpy(aidData.CL_Ceiling_Limit, "\x00\x00\x05\xDC", 4);  //999. Above this amount transactions are denied.
	aidData.Info_Included_Data[0]|=INPUT_CTLS_APL_CEILING_LIMIT;
	memcpy(aidData.MerchIdent, "\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31", 15);
	aidData.Info_Included_Data[1]|=INPUT_CTLS_APL_MERCHANT_IDENT;
	memcpy(aidData.TermIdent, "\x31\x31\x31\x31\x31\x31\x31\x31", 8);
	aidData.Info_Included_Data[0]|=INPUT_CTLS_APL_TID;

	memcpy(aidData.CL_CVM_Soft_Limit, "\x00\x01\x86\xA0", 4);
	aidData.Info_Included_Data[0]|=INPUT_CTLS_APL_CVM_LIMIT;


	return EMV_CTLS_SetAppliData(EMV_ADK_SET_ONE_RECORD, &aid, &aidData);
}

// =============================================================================
unsigned char cfgVisaCTLS()
{
  EMV_CTLS_APPLI_TYPE aid = { 7, {0xA0,0x00,0x00,0x00,0x03,0x10,0x10} };
  EMV_CTLS_APPLIDATA_TYPE aidData = {0};

  memset(&aidData, 0, sizeof(EMV_CTLS_APPLIDATA_TYPE));

  aidData.CL_Modes = CL_VISA_COMBINED;
  aidData.Info_Included_Data[0] |= INPUT_CTLS_APL_SUPPORTED_CTLS;

  memcpy(aidData.App_CountryCodeTerm, "\x08\x40", 2); //Update some parameters for demo purposes
  aidData.Info_Included_Data[2]|=INPUT_CTLS_APL_COUNTRY_CODE;
  memcpy(aidData.FloorLimit, "\x00\x00\x2A\xF8", 4);  //110
  aidData.Info_Included_Data[1]|=INPUT_CTLS_APL_FLOOR_LIMIT;
  memcpy(aidData.CL_Ceiling_Limit, "\x00\x01\x86\xA0", 4);  //1000. Above this amount transactions are denied.
  aidData.Info_Included_Data[0]|=INPUT_CTLS_APL_CEILING_LIMIT;
  memcpy(aidData.MerchIdent, "\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31\x32\x31", 15);
  aidData.Info_Included_Data[1]|=INPUT_CTLS_APL_MERCHANT_IDENT;
  memcpy(aidData.TermIdent, "\x31\x31\x31\x31\x31\x31\x31\x31", 8);
  aidData.Info_Included_Data[0]|=INPUT_CTLS_APL_TID;

  return EMV_CTLS_SetAppliData(EMV_ADK_SET_ONE_RECORD, &aid, &aidData);
}

// =============================================================================
EMV_ADK_INFO SetCTLSApplicationData()
{
  EMV_ADK_INFO erg=EMV_ADK_OK;

  erg = EMV_CTLS_SetAppliData(EMV_ADK_CLEAR_ALL_RECORDS, NULL, NULL);
  if (erg != EMV_ADK_OK){
	  LOGF_TRACE("----------Clear CTLS Appl Data error: (%#.2x)", erg);
	  return erg;
  }
  erg = cfgMasterCardCTLS();
  if (erg != EMV_ADK_OK){
	  LOGF_TRACE("----------Config MasterCard error: (%#.2x)", erg);
	  return erg;
  }
  LOGF_TRACE("----------Config MasterCard App OK...");
  erg = cfgVisaCTLS();
  if (erg != EMV_ADK_OK){
	  LOGF_TRACE("----------Config Visa error: (%#.2x)", erg);
	  return erg;
  }
  LOGF_TRACE("----------Config Visa App OK");
  return erg;
}

// =============================================================================
EMV_ADK_INFO SetCTApplicationData()
{
  EMV_ADK_INFO erg=EMV_ADK_OK;

  erg = EMV_CT_SetAppliData(EMV_ADK_CLEAR_ALL_RECORDS, NULL, NULL);
  if (erg != EMV_ADK_OK){
	  LOGF_TRACE("----------Clear CT Appl Data error: (%#.2x)", erg);
	  return erg;
  }
  erg = cfgMasterCardCT();
  if (erg != EMV_ADK_OK){
	  LOGF_TRACE("----------Config CT MasterCard error: (%#.2x)", erg);
	  return erg;
  }
  erg = cfgVisaCT();
  if (erg != EMV_ADK_OK){
	  LOGF_TRACE("----------Config CT Visa error: (%#.2x)", erg);
	  return erg;
  }
  LOGF_TRACE("----------Set CT Application Data: OK");

  return erg;
}

// =======================================================================================================================================
EMV_ADK_INFO SetCTLSTerminalData(void) //Information change/added here will be updated in the EMV_CTLS_Applications.xml configuration file.
{
   EMV_ADK_INFO erg = EMV_ADK_OK;
   EMV_CTLS_TERMDATA_TYPE termData = {0};

   termData.CL_Modes_Supported = CL_VISA_COMBINED | CL_MC_PAYPASS_COMBINED | CL_AMEX_COMBINED; //Supported contactless modes
   termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_SUPPORTED_CTLS; //Availability bit. Indicates that termData.CL_Modes_Supported is set

   termData.TermTyp = EMV_ADK_TT_ATTENDED_OFFL_ONL; //Terminal type: attended terminal, offline with online capability
   termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_TYPE;

   memcpy(termData.CountryCodeTerm, "\x08\x40", 2);  // Country code: USA
   termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_COUNTRY_CODE;

   memcpy(termData.CurrencyTrans, "\x08\x40", 2);  // Currency code: US dollar
   termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_CURRENCY;

   termData.ExpTrans = 2; //Currency exponent
   termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_EXP_CURRENCY;

   termData.SuppLang[0] = EMV_ADK_LANG_ENGLISH; //Up to EMV_ADK_MAX_LANG can be set up
   termData.SuppLang[1] = EMV_ADK_LANG_SPANISH;
   termData.Info_Included_Data[1] |= INPUT_CTLS_TRM_LANGUAGES;

   memcpy(termData.IFDSerialNumber,"12345678",8);
   termData.Info_Included_Data[1] |= INPUT_CTLS_TRM_IFD_SERIAL;

   // no setting of FlowOptions
   termData.Info_Included_Data[0] |= INPUT_CTLS_TRM_FLOW_OPTIONS;  // Force setting even if something different exists in stored xml file

   erg = EMV_CTLS_SetTermData(&termData); //This call updates information in XML file.
   if (erg != EMV_ADK_OK){
	   LOGF_TRACE("----------Set CTLS Term Data Error (%#.2x)", erg);
	   EMV_CTLS_Exit_Framework();
   	   return erg;
   }
   LOGF_TRACE("----------Set CTLS Term Data OK");
   return erg;
}

//==============================================================================================================================
EMV_ADK_INFO SetCTTerminalData(void) //Information change/added here will be updated in the EMV_Terminal.xml configuration file.
{
   EMV_ADK_INFO erg = EMV_ADK_OK;
   EMV_CT_TERMDATA_TYPE termData = {0};

   termData.TermTyp = EMV_ADK_TT_ATTENDED_OFFL_ONL; //Terminal type: attended terminal, offline with online capability
   termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_TYPE;

   memcpy(termData.TermCountryCode, "\x08\x40", 2);  // Country code: USA
   termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_COUNTRY_CODE;

   memcpy(termData.CurrencyTrans, "\x08\x40", 2);  // Currency code: US dollar
   termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_CURRENCY;

   termData.ExpTrans = 2; //Currency exponent
   termData.Info_Included_Data[0] |= EMV_CT_INPUT_TRM_EXP_CURRENCY;

   termData.SuppLang[0] = EMV_ADK_LANG_ENGLISH; //Up to EMV_ADK_MAX_LANG can be set up
   termData.SuppLang[1] = EMV_ADK_LANG_SPANISH;
   termData.Info_Included_Data[1] |= EMV_CT_INPUT_TRM_LANGUAGES;

   memcpy(termData.IFDSerialNumber,"12345678",8);
   termData.Info_Included_Data[1] |= EMV_CT_INPUT_TRM_IFD_SERIAL;

   erg = EMV_CT_SetTermData(&termData);
   if (erg != EMV_ADK_OK){
	   LOGF_TRACE("----------Set CT Term Data Error (%#.2x)", erg);
	   EMV_CTLS_Exit_Framework();
	   return erg;
   }
   LOGF_TRACE("----------Set CT Term Data OK");
   return erg;
}

#endif

