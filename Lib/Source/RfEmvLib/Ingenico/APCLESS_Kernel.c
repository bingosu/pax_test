/**
 * \file
 * This module is used to fill/initialise the kernel data buffer.
 *
 * \author  Ingenico
 * \author  Copyright (c) 2012 Ingenico
 *
 * \author  Ingenico has intellectual property rights relating to the technology embodied \n
 *       in this software. In particular, and without limitation, these intellectual property rights may\n
 *       include one or more patents.\n
 *       This software is distributed under licenses restricting its use, copying, distribution, and\n
 *       and decompilation. No part of this software may be reproduced in any form by any means\n
 *       without prior written authorization of Ingenico.
 */

#include "sdk_tplus.h"
#include "APCLESS.h"
#include "apMem.h"
#include "apMtiCommonApp.h"



/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////


/////////////////////////////////////////////////////////////////
//// Global data definitions ////////////////////////////////////

/////////////////////////////////////////////////////////////////
//// Static data definitions ////////////////////////////////////
#if 0 //@@WAY CLOSED, 20190901
static unsigned char __APCLESS_PayPass_caKeyFAModulus[] =
  { 0xA9, 0x0F, 0xCD, 0x55, 0xAA, 0x2D, 0x5D, 0x99, 0x63, 0xE3, 0x5E, 0xD0, 0xF4, 0x40, 0x17, 0x76,
	0x99, 0x83, 0x2F, 0x49, 0xC6, 0xBA, 0xB1, 0x5C, 0xDA, 0xE5, 0x79, 0x4B, 0xE9, 0x3F, 0x93, 0x4D,
	0x44, 0x62, 0xD5, 0xD1, 0x27, 0x62, 0xE4, 0x8C, 0x38, 0xBA, 0x83, 0xD8, 0x44, 0x5D, 0xEA, 0xA7,
	0x41, 0x95, 0xA3, 0x01, 0xA1, 0x02, 0xB2, 0xF1, 0x14, 0xEA, 0xDA, 0x0D, 0x18, 0x0E, 0xE5, 0xE7,
	0xA5, 0xC7, 0x3E, 0x0C, 0x4E, 0x11, 0xF6, 0x7A, 0x43, 0xDD, 0xAB, 0x5D, 0x55, 0x68, 0x3B, 0x14,
	0x74, 0xCC, 0x06, 0x27, 0xF4, 0x4B, 0x8D, 0x30, 0x88, 0xA4, 0x92, 0xFF, 0xAA, 0xDA, 0xD4, 0xF4,
	0x24, 0x22, 0xD0, 0xE7, 0x01, 0x35, 0x36, 0xC3, 0xC4, 0x9A, 0xD3, 0xD0, 0xFA, 0xE9, 0x64, 0x59,
	0xB0, 0xF6, 0xB1, 0xB6, 0x05, 0x65, 0x38, 0xA3, 0xD6, 0xD4, 0x46, 0x40, 0xF9, 0x44, 0x67, 0xB1,
	0x08, 0x86, 0x7D, 0xEC, 0x40, 0xFA, 0xAE, 0xCD, 0x74, 0x0C, 0x00, 0xE2, 0xB7, 0xA8, 0x85, 0x2D };

static unsigned char __APCLESS_PayPass_caKeyFAExponent[] = {0x03};

static unsigned char __APCLESS_PayPass_caKeyEFModulus[] =
     {	0xA1, 0x91, 0xCB, 0x87, 0x47, 0x3F, 0x29, 0x34,
		0x9B, 0x5D, 0x60, 0xA8, 0x8B, 0x3E, 0xAE, 0xE0,
		0x97, 0x3A, 0xA6, 0xF1, 0xA0, 0x82, 0xF3, 0x58,
		0xD8, 0x49, 0xFD, 0xDF, 0xF9, 0xC0, 0x91, 0xF8,
		0x99, 0xED, 0xA9, 0x79, 0x2C, 0xAF, 0x09, 0xEF,
		0x28, 0xF5, 0xD2, 0x24, 0x04, 0xB8, 0x8A, 0x22,
		0x93, 0xEE, 0xBB, 0xC1, 0x94, 0x9C, 0x43, 0xBE,
		0xA4, 0xD6, 0x0C, 0xFD, 0x87, 0x9A, 0x15, 0x39,
		0x54, 0x4E, 0x09, 0xE0, 0xF0, 0x9F, 0x60, 0xF0,
		0x65, 0xB2, 0xBF, 0x2A, 0x13, 0xEC, 0xC7, 0x05,
		0xF3, 0xD4, 0x68, 0xB9, 0xD3, 0x3A, 0xE7, 0x7A,
		0xD9, 0xD3, 0xF1, 0x9C, 0xA4, 0x0F, 0x23, 0xDC,
		0xF5, 0xEB, 0x7C, 0x04, 0xDC, 0x8F, 0x69, 0xEB,
		0xA5, 0x65, 0xB1, 0xEB, 0xCB, 0x46, 0x86, 0xCD,
		0x27, 0x47, 0x85, 0x53, 0x0F, 0xF6, 0xF6, 0xE9,
		0xEE, 0x43, 0xAA, 0x43, 0xFD, 0xB0, 0x2C, 0xE0,
		0x0D, 0xAE, 0xC1, 0x5C, 0x7B, 0x8F, 0xD6, 0xA9,
		0xB3, 0x94, 0xBA, 0xBA, 0x41, 0x9D, 0x3F, 0x6D,
		0xC8, 0x5E, 0x16, 0x56, 0x9B, 0xE8, 0xE7, 0x69,
		0x89, 0x68, 0x8E, 0xFE, 0xA2, 0xDF, 0x22, 0xFF,
		0x7D, 0x35, 0xC0, 0x43, 0x33, 0x8D, 0xEA, 0xA9,
		0x82, 0xA0, 0x2B, 0x86, 0x6D, 0xE5, 0x32, 0x85,
		0x19, 0xEB, 0xBC, 0xD6, 0xF0, 0x3C, 0xDD, 0x68,
		0x66, 0x73, 0x84, 0x7F, 0x84, 0xDB, 0x65, 0x1A,
		0xB8, 0x6C, 0x28, 0xCF, 0x14, 0x62, 0x56, 0x2C,
		0x57, 0x7B, 0x85, 0x35, 0x64, 0xA2, 0x90, 0xC8,
		0x55, 0x6D, 0x81, 0x85, 0x31, 0x26, 0x8D, 0x25,
		0xCC, 0x98, 0xA4, 0xCC, 0x6A, 0x0B, 0xDF, 0xFF,
		0xDA, 0x2D, 0xCC, 0xA3, 0xA9, 0x4C, 0x99, 0x85,
		0x59, 0xE3, 0x07, 0xFD, 0xDF, 0x91, 0x50, 0x06,
		0xD9, 0xA9, 0x87, 0xB0, 0x7D, 0xDA, 0xEB, 0x3B };

static unsigned char __APCLESS_PayPass_caKeyEFExponent[] = {0x03};

static unsigned char __APCLESS_PayPass_caKeyF1Modulus[] =
     {	0xA0, 0xDC, 0xF4, 0xBD, 0xE1, 0x9C, 0x35, 0x46,
		0xB4, 0xB6, 0xF0, 0x41, 0x4D, 0x17, 0x4D, 0xDE,
		0x29, 0x4A, 0xAB, 0xBB, 0x82, 0x8C, 0x5A, 0x83,
		0x4D, 0x73, 0xAA, 0xE2, 0x7C, 0x99, 0xB0, 0xB0,
		0x53, 0xA9, 0x02, 0x78, 0x00, 0x72, 0x39, 0xB6,
		0x45, 0x9F, 0xF0, 0xBB, 0xCD, 0x7B, 0x4B, 0x9C,
		0x6C, 0x50, 0xAC, 0x02, 0xCE, 0x91, 0x36, 0x8D,
		0xA1, 0xBD, 0x21, 0xAA, 0xEA, 0xDB, 0xC6, 0x53,
		0x47, 0x33, 0x7D, 0x89, 0xB6, 0x8F, 0x5C, 0x99,
		0xA0, 0x9D, 0x05, 0xBE, 0x02, 0xDD, 0x1F, 0x8C,
		0x5B, 0xA2, 0x0E, 0x2F, 0x13, 0xFB, 0x2A, 0x27,
		0xC4, 0x1D, 0x3F, 0x85, 0xCA, 0xD5, 0xCF, 0x66,
		0x68, 0xE7, 0x58, 0x51, 0xEC, 0x66, 0xED, 0xBF,
		0x98, 0x85, 0x1F, 0xD4, 0xE4, 0x2C, 0x44, 0xC1,
		0xD5, 0x9F, 0x59, 0x84, 0x70, 0x3B, 0x27, 0xD5,
		0xB9, 0xF2, 0x1B, 0x8F, 0xA0, 0xD9, 0x32, 0x79,
		0xFB, 0xBF, 0x69, 0xE0, 0x90, 0x64, 0x29, 0x09,
		0xC9, 0xEA, 0x27, 0xF8, 0x98, 0x95, 0x95, 0x41,
		0xAA, 0x67, 0x57, 0xF5, 0xF6, 0x24, 0x10, 0x4F,
		0x6E, 0x1D, 0x3A, 0x95, 0x32, 0xF2, 0xA6, 0xE5,
		0x15, 0x15, 0xAE, 0xAD, 0x1B, 0x43, 0xB3, 0xD7,
		0x83, 0x50, 0x88, 0xA2, 0xFA, 0xFA, 0x7B, 0xE7 };

static unsigned char __APCLESS_PayPass_caKeyF1Exponent[] = {0x03};
#endif //@@WAY CLOSED, 20190901
/////////////////////////////////////////////////////////////////
//// Static functions definition ////////////////////////////////

static int __APCLESS_Kernel_CopyAIDParamToSharedBuffer(int aidIdentifier, T_SHARED_DATA_STRUCT* buffer);

/////////////////////////////////////////////////////////////////
//// Functions //////////////////////////////////////////////////

extern unsigned long __APCLESS_ParamTrn_ulAmount;

/**
 * Copy all the tags of an AID to a shared buffer.
 * @param[in] aidIdentifier identifies the AID parameters set (correspond to \a TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER)
 * @param[out] buffer shared buffer where the parameters are copied to.
 * @return
 *     - \a TRUE if correctly performed (data found and \a outSharedBuffer filled).
 *     - \a FALSE if an error occurred.
 */
 //@@WAY, 20190520 PAYPASS
#if 1
static int __APCLESS_Kernel_CopyAIDParamToSharedBuffer(int aidIdentifier, T_SHARED_DATA_STRUCT* buffer)
{
	int result = FALSE;
   	//@@WAY PARAM PAYPASS 20210127 unsigned char termCapabilities[] = {0xE0, 0xF8, 0xC8};
   	tMtiMap *tpTagData = &g_tMtiPayWaveMap;
   	tMtiMap *tpTagDataPayPass = &g_tMtiPayPassMap;
   	tMtiMap *tpTranData = &g_tMtiTranParamMap;
	tMtiMap *tpTagDataJspeedy = &g_tMtiJspeedyMap; //@@WAY JCB Cless
   	//tMtiMap tMainMap;
   	//tMtiMap *tpMainMap = &tMainMap;
   	UCHAR *pchEMVTagData = NULL;
	UCHAR *pchPaypassTrnLimitNoDcv = NULL;
	UCHAR *pchPaypassTrnLimitDcv = NULL;
   	INT iLength = 0;
	INT iLengthPayPassLimitNoDcv = 0;
	INT iLengthPayPassLimitDcv = 0;

   	UCHAR *pchPayPassTAC_default = NULL;
	UCHAR *pchPaypassTAC_denial = NULL;
	UCHAR *pchPaypassTAC_online = NULL;
	INT iLengthPayPassTAC_default = 0;
	INT iLengthPayPassTAC_denial = 0;
	INT iLengthPayPassTAC_online = 0;
	CHAR szTempBuff[64];
   	unsigned long ulCvmRequiredLimit = 0L;
   	unsigned long ulCtlsFloorLimit = 0L;

	//@@WAY PARAM PAYPASS 20210127
	UCHAR *ucpTranLimit = NULL;
	UCHAR *ucpFloorLimit = NULL;
	INT iTranLimitLen = 0;
	INT iFloorLimitLen = 0;
	//
	
   	// In this sample, there is no AID database. We are setting the parameters directly in the source code
   	dmsg("**** aidIdentifier = %d", aidIdentifier);
   	if(aidIdentifier == APCLESS_AID_PAYPASS_IDENTIFIER)
   	{  // Add PayPass tags
	   	unsigned char aidPaypassKernelToUse[] = {0x00, 0x02};             // PayPass kernel number = 2
		unsigned char aidPaypassAppliVerNumTerm[] = {0x00, 0x02};             // PayPass kernel number = 2
		unsigned char aidPaypassOptions[] = {0x45, 0x01, 0x00, 0x00};     // Partial AID, Zero Amount, PPSE method
		unsigned char aidDefaultUdol[] = {0x9F, 0x6A, 0x04};
		//@@WAY PARAM PAYPASS 20210127 unsigned char aidPaypassFloorLimit[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char aidPaypassSecurityCapability[] = {0x08};
		unsigned char aidPaypassCardDataInputCapability[] = {0x00};//{0xE0}; //@@WAY PARAM PAYPASS 20210127
		unsigned char aidPaypassMChipCvmCapabilityCvmRequired[] = {0x60};
		unsigned char aidPaypassMChipCvmCapabilityCvmNotRequired[] = {0x08};
		unsigned char aidPaypassMStripeCvmCapabilityCvmRequired[] = {0x01};
		unsigned char aidPaypassMStripeCvmCapabilityCvmNotRequired[] = {0x00};
		unsigned char aidPaypassKernelConfiguration[] = {0x20};
		unsigned char aidPaypassKernelTerminalSupportedLanguages[] = {0x65, 0x6E};
		unsigned char aidPaypassTermType[] = {0x21}; //@@WAY PARAM PAYPASS 20210127 {0x21};
		unsigned char aidPaypassTerminalRiskManagment[] = {0x6C, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00}; //@@WAY PARAM PAYPASS 20210127{0x6C, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
		
		pchPaypassTrnLimitNoDcv = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_TRX_NODCV_LIMIT, &iLengthPayPassLimitNoDcv);
		pchPaypassTrnLimitDcv = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_TRX_DCV_LIMIT, &iLengthPayPassLimitDcv);
		pchEMVTagData = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_CVM_REQ_LIMIT, &iLength);
		//@@WAY PARAM PAYPASS 20210127
		ucpFloorLimit = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_FLOOR_LIMIT, &iFloorLimitLen);
		//
		pchPayPassTAC_default = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_TAC_DEFAULT, &iLengthPayPassTAC_default);
		pchPaypassTAC_denial = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_TAC_DENIAL, &iLengthPayPassTAC_denial);
		pchPaypassTAC_online = mtiMapGetBytes(tpTagDataPayPass, MTI_RFEMV_TAG_PAYPASS_TAC_ONLINE, &iLengthPayPassTAC_online);
		
		if (pchEMVTagData!=NULL && 1 < iLength)
		{
			mtiMemset(szTempBuff, 0, sizeof(szTempBuff));
			mtiHtoa((UCHAR *)szTempBuff, (UCHAR *) pchEMVTagData, iLength);
			ulCvmRequiredLimit = mtiAtoll(12, (UCHAR *)szTempBuff);

			dmsg("<<<<<CVM LIMIT>>>>> = Rp. %ld", ulCvmRequiredLimit);
		}
		
//@@WAY PARAM PAYPASS 20210127
#if 1
		if (ulCvmRequiredLimit < __APCLESS_ParamTrn_ulAmount)
#else
		if (ulCvmRequiredLimit <= __APCLESS_ParamTrn_ulAmount)
#endif
		{
			mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
		}
		else
		{
			mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, TRUE);
		}

		dmsg("<<<<<CVM LIMIT FLAG>>>>> = %d", mtiMapGetInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM));

      	//TAG_PAYPASS_INT_MERCHANT_FORCE_ONLINE
      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_KERNEL_TO_USE, sizeof(aidPaypassKernelToUse), aidPaypassKernelToUse) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_OPTIONS, sizeof(aidPaypassOptions), aidPaypassOptions) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_DEFAULT_UDOL, sizeof(aidDefaultUdol), aidDefaultUdol) != STATUS_SHARED_EXCHANGE_OK) return result;
		//@@WAY PARAM PAYPASS 20210127
		#if 1
		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, iFloorLimitLen, ucpFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
		#else
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, sizeof(aidPaypassFloorLimit), aidPaypassFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
		#endif
		//
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CLESS_TRANSACTION_LIMIT_NO_DCV, iLengthPayPassLimitNoDcv, pchPaypassTrnLimitNoDcv) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CLESS_TRANSACTION_LIMIT_DCV, iLengthPayPassLimitDcv, pchPaypassTrnLimitDcv) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, iLength, pchEMVTagData) != STATUS_SHARED_EXCHANGE_OK) return result;

	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_SECURITY_CAPABILITY, sizeof(aidPaypassSecurityCapability), aidPaypassSecurityCapability) != STATUS_SHARED_EXCHANGE_OK) return result;
	  	//@@WAY PARAM PAYPASS 20210127 if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_CAPABILITIES, sizeof(termCapabilities), termCapabilities) != STATUS_SHARED_EXCHANGE_OK) return result;

	  if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_DEFAULT, iLengthPayPassTAC_default, pchPayPassTAC_default) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_DENIAL, iLengthPayPassTAC_denial, pchPaypassTAC_denial) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_ONLINE, iLengthPayPassTAC_online, pchPaypassTAC_online) != STATUS_SHARED_EXCHANGE_OK) return result;

	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CARD_DATA_INPUT_CAPABILITY, sizeof(aidPaypassCardDataInputCapability), aidPaypassCardDataInputCapability) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MCHIP_CVM_CAPABILITY_CVM_REQUIRED, sizeof(aidPaypassMChipCvmCapabilityCvmRequired), aidPaypassMChipCvmCapabilityCvmRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MCHIP_CVM_CAPABILITY_CVM_NOT_REQUIRED, sizeof(aidPaypassMChipCvmCapabilityCvmNotRequired), aidPaypassMChipCvmCapabilityCvmNotRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
	  //@@WAY PAYPASS 2
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MSTRIPE_CVM_CAPABILITY_CVM_REQUIRED, sizeof(aidPaypassMStripeCvmCapabilityCvmRequired), aidPaypassMStripeCvmCapabilityCvmRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MSTRIPE_CVM_CAPABILITY_CVM_NOT_REQUIRED, sizeof(aidPaypassMStripeCvmCapabilityCvmNotRequired), aidPaypassMStripeCvmCapabilityCvmNotRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
	  //
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_KERNEL_CONFIGURATION, sizeof(aidPaypassKernelConfiguration), aidPaypassKernelConfiguration) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_APPLI_VERSION_NUMBER_TERM, sizeof(aidPaypassAppliVerNumTerm), aidPaypassAppliVerNumTerm) != STATUS_SHARED_EXCHANGE_OK) return result;

      if(GTL_SharedExchange_AddTag(buffer, TAG_KERNEL_TERMINAL_SUPPORTED_LANGUAGES, sizeof(aidPaypassKernelTerminalSupportedLanguages), aidPaypassKernelTerminalSupportedLanguages) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_TYPE, sizeof(aidPaypassTermType), aidPaypassTermType) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_RISK_MANAGEMENT_DATA, sizeof(aidPaypassTerminalRiskManagment), aidPaypassTerminalRiskManagment) != STATUS_SHARED_EXCHANGE_OK) return result;

      	result = TRUE;
   }
   else if(aidIdentifier == APCLESS_AID_PAYWAVE_IDENTIFIER)
   {
     // Add payWave tags
      unsigned char aidpayWaveKernelToUse[] = {0x00, 0x03};             // Visa payWave kernel number = 1
      unsigned char aidpayWaveOptions[] = {0x01, 0x01, 0x00, 0x00};     // Partial AID, PPSE method
      unsigned char aidpayWaveTTQ[] =     {0x36, 0x00, 0x40, 0x00};     // Cless MSD + qVSDC & onlinePIN + sign supported + CVN17 + ISP
      unsigned char aidpayWaveFddaVer[] = {0x00, 0x01};
      	//@@WAY PARAM PAYPASS 20210127 unsigned char aidpayWaveFloorLimit[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
      	unsigned char aidpayWaveTermType[] = {0x21}; //@@WAY PARAM PAYPASS 20210127 {0x21};

      T_PRE_PROCESSING_ADD_RESULT_AID tPreAddResult;

		//@@WAY PARAM PAYPASS 20210127
	  	ucpTranLimit = mtiMapGetBytes(tpTagData, MTI_RFEMV_TAG_PAYWAVE_TRX_LIMIT, &iTranLimitLen);
		ucpFloorLimit = mtiMapGetBytes(tpTagData, MTI_RFEMV_TAG_PAYWAVE_FLOOR_LIMIT, &iFloorLimitLen);
		//
		pchEMVTagData = mtiMapGetBytes(tpTagData, MTI_RFEMV_TAG_PAYWAVE_CVM_LIMIT, &iLength);

		if (pchEMVTagData!=NULL && 1 < iLength)
		{
			mtiMemset(szTempBuff, 0, sizeof(szTempBuff));
			mtiHtoa((UCHAR *)szTempBuff, (UCHAR *)pchEMVTagData, iLength);
			ulCvmRequiredLimit = mtiAtoll(12, (UCHAR *)szTempBuff);

			dmsg("<<<<<CVM LIMIT>>>>> = Rp. %ld", ulCvmRequiredLimit);
		}

	  if (ulCtlsFloorLimit < __APCLESS_ParamTrn_ulAmount)
      {
    	  aidpayWaveTTQ[1] |= PAYWAVE_TTQ_BYTE_2_ONLINE_CRYPTOGRAM_REQUIRED;
      }

      if (ulCvmRequiredLimit <= __APCLESS_ParamTrn_ulAmount)
      {
    	  aidpayWaveTTQ[1] |= PAYWAVE_TTQ_BYTE_2_CVM_REQUIRED;
		  mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
      }
	  else
	  {
	  	  mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, TRUE);
	  }

	  dmsg("<<<<<CVM LIMIT FLAG>>>>> = %d", mtiMapGetInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM));

      dbuf("=== aidpayWaveTTQ", aidpayWaveTTQ, 4);

      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_KERNEL_TO_USE, sizeof(aidpayWaveKernelToUse), aidpayWaveKernelToUse) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_OPTIONS, sizeof(aidpayWaveOptions), aidpayWaveOptions) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYWAVE_TERMINAL_TRANSACTION_QUALIFIERS, sizeof(aidpayWaveTTQ), aidpayWaveTTQ) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYWAVE_TERM_SUPPORTED_FDDA_VERSIONS, sizeof(aidpayWaveFddaVer), aidpayWaveFddaVer) != STATUS_SHARED_EXCHANGE_OK) return result;
		//@@WAY PARAM PAYPASS 20210127
		#if 1
		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, iFloorLimitLen, ucpFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_TRANSACTION_LIMIT, iTranLimitLen, ucpTranLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
		#else
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_CAPABILITIES, sizeof(termCapabilities), termCapabilities) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, sizeof(aidpayWaveFloorLimit), aidpayWaveFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
		#endif
		//
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, iLength, pchEMVTagData) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_TYPE, sizeof(aidpayWaveTermType), aidpayWaveTermType) != STATUS_SHARED_EXCHANGE_OK) return result;

	  tPreAddResult.bStatusCheckRequested = 1;
	  tPreAddResult.bClessCvmLimitExceeded = 1;
		//@@WAY PARAM PAYPASS 20210127
		#if 1
		tPreAddResult.bClessFloorLimitExceeded = 1;
	  	tPreAddResult.bClessTransactionLimitExceeded = 1;
		#else
	  tPreAddResult.bClessFloorLimitExceeded = 0;
	  tPreAddResult.bClessTransactionLimitExceeded = 0;
		#endif
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_ADDITIONAL_RESULTS, sizeof(tPreAddResult), (unsigned char *)&tPreAddResult) != STATUS_SHARED_EXCHANGE_OK) return result;

	  //TAG_PAYWAVE_AVAILABLE_OFFLINE_SPENDING_AMOUNT
      result = TRUE;
   }
  	//@@WAY JCB Cless
	else if(aidIdentifier == APCLESS_AID_JSPEEDY_IDENTIFIER)
	{
		unsigned char aidJSpeedyKernelToUse[] = {0x00, 0x05};											// JSpeedy kernel number = 1
		unsigned char aidJSpeedyOptions[] = {0x05, 0x01, 0x00, 0x00};									// Partial AID & Zero amount + EP allowed
		unsigned char aidJSpeedyCombinaison[] = {0x7B, 0x00};											// see JCB Contactless IC Terminal Specification V1.2 Annex A.3
		unsigned char aidJSpeedyImplemenationOptions[] = {0x1F};										// 0x01: EMV: 0x02: LEGACY; 0x04: OFFLINE_DATA_AUTH; 0x08: EXCEPT_FILE_CHECK; 0x10: ISSUER_UPDATE
		unsigned char aidJSpeedyTermInterChange[]= {0xF2, 0x80, 0x00};									// see JCB Contactless IC Terminal Specification V1.2 Annex A.6
		unsigned char aidJspeedyTermType[] = {0x21};													// TAG_EMV_TERMINAL_TYPE
		unsigned char aidJspeedyAcqID[] = {0x00, 0x00, 0x00, 0x16, 0x28, 0x02}; 						// TAG_EMV_ACQUIRER_IDENTIFIER
		unsigned char aidJspeedyMercNameAndLocation[] = {0x00}; 										// TAG_EMV_MERCHANT_NAME_AND_LOCATION
		
		ucpTranLimit = mtiMapGetBytes(tpTagDataJspeedy, MTI_RFEMV_TAG_JSPEEDY_TRX_LIMIT, &iTranLimitLen);
		pchEMVTagData = mtiMapGetBytes(tpTagDataJspeedy, MTI_RFEMV_TAG_JSPEEDY_CVM_LIMIT, &iLength);
		ucpFloorLimit = mtiMapGetBytes(tpTagDataJspeedy, MTI_RFEMV_TAG_JSPEEDY_FLOOR_LIMIT, &iFloorLimitLen);

		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, iFloorLimitLen, ucpFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, iLength, pchEMVTagData) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_TRANSACTION_LIMIT, iTranLimitLen, ucpTranLimit) != STATUS_SHARED_EXCHANGE_OK) return result;

		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_KERNEL_TO_USE, sizeof(aidJSpeedyKernelToUse), aidJSpeedyKernelToUse) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_OPTIONS, sizeof(aidJSpeedyOptions), aidJSpeedyOptions) != STATUS_SHARED_EXCHANGE_OK) return result;

		if(GTL_SharedExchange_AddTag(buffer, TAG_JCB_COMBINAISON_OPTIONS, sizeof(aidJSpeedyCombinaison), aidJSpeedyCombinaison) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_JCB_IMPLEMENTATION_OPTIONS, sizeof(aidJSpeedyImplemenationOptions), aidJSpeedyImplemenationOptions) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_JCB_TERMINAL_INTERCHANGE_PROFILE_S, sizeof(aidJSpeedyTermInterChange), aidJSpeedyTermInterChange) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_TYPE, sizeof(aidJspeedyTermType), aidJspeedyTermType) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_ACQUIRER_IDENTIFIER, sizeof(aidJspeedyAcqID), aidJspeedyAcqID) != STATUS_SHARED_EXCHANGE_OK) return result;
		if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_MERCHANT_NAME_AND_LOCATION, sizeof(aidJspeedyMercNameAndLocation), aidJspeedyMercNameAndLocation) != STATUS_SHARED_EXCHANGE_OK) return result;

		mtiMemset(szTempBuff, 0, sizeof(szTempBuff));
		mtiHtoa((UCHAR *)szTempBuff, pchEMVTagData, 6);
		ulCvmRequiredLimit = mtiAtoll(12, (UCHAR *)szTempBuff);
		if (__APCLESS_ParamTrn_ulAmount > ulCvmRequiredLimit)
		{
			mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
		}
		else
		{
			mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, TRUE);
		}

		result = TRUE;

	}
	//
   return result;
}
#else
static int __APCLESS_Kernel_CopyAIDParamToSharedBuffer(int aidIdentifier, T_SHARED_DATA_STRUCT* buffer)
{
   int result = FALSE;
   unsigned char termCapabilities[] = {0xE0, 0xF8, 0xC8};

   tMtiMap *tpTagData = &g_tMtiPayWaveMap;
   tMtiMap *tpTranData = &g_tMtiTranParamMap;
   tMtiMap tMainMap;
   tMtiMap *tpMainMap = &tMainMap;
   UCHAR *pchEMVTagData = NULL;
   INT iLength = 0;
   CHAR szTempBuff[64];
   //unsigned long ulCvmRequiredLimit = 1000000L;
   unsigned long ulCvmRequiredLimit = 0L;
   unsigned long ulCtlsFloorLimit = 0L;


   pchEMVTagData = mtiMapGetBytes(tpTagData, MTI_RFEMV_TAG_PAYWAVE_CVM_LIMIT, &iLength);

   if (pchEMVTagData!=NULL && 1 < iLength)
   {
		mtiMemset(szTempBuff, 0, sizeof(szTempBuff));
		mtiHtoa(szTempBuff, pchEMVTagData, iLength);
		ulCvmRequiredLimit = mtiAtoll(12, szTempBuff);

		dmsg("<<<<<CVM LIMIT>>>>> = Rp. %ld", ulCvmRequiredLimit);
   }


   // In this sample, there is no AID database. We are setting the parameters directly in the source code

   dmsg("**** aidIdentifier = %d", aidIdentifier);
   if(aidIdentifier == APCLESS_AID_PAYPASS_IDENTIFIER)
   {  // Add PayPass tags
	   unsigned char aidPaypassKernelToUse[] = {0x00, 0x02};             // PayPass kernel number = 2
		 unsigned char aidPaypassAppliVerNumTerm[] = {0x00, 0x02};             // PayPass kernel number = 2
		 unsigned char aidPaypassOptions[] = {0x45, 0x01, 0x00, 0x00};     // Partial AID, Zero Amount, PPSE method
		 unsigned char aidDefaultUdol[] = {0x9F, 0x6A, 0x04};
		 unsigned char aidPaypassFloorLimit[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		 unsigned char aidPaypassTrnLimitNoDcv[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};
		 unsigned char aidPaypassTrnLimitDcv[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};
		 unsigned char aidPaypassCvmRequiredLimit[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
		 unsigned char aidPaypassSecurityCapability[] = {0x08};

   #if 0
		 unsigned char aidPaypassEmvIntTacDefault[] = {0x00, 0x00, 0x00, 0x00, 0x00};
		 unsigned char aidPaypassEmvIntTacDenial[] = {0x00, 0x00, 0x00, 0x00, 0x00};
		 unsigned char aidPaypassEmvIntTacOnline[] = {0x00, 0x00, 0x00, 0x00, 0x00};
   #else
	  unsigned char aidPaypassEmvIntTacDenial[] = { 0x00, 0x00, 0x00, 0x00, 0x00};
	  unsigned char aidPaypassEmvIntTacOnline[] = { 0xFC, 0x50, 0x9C, 0x88, 0x00 };
	  unsigned char aidPaypassEmvIntTacDefault[] = { 0xFC, 0x50, 0x9C, 0x88, 0x00 };

   #endif


		 //unsigned char aidPaypassCardDataInputCapability[] = {0x00};
	  unsigned char aidPaypassCardDataInputCapability[] = {0xE0};
		 unsigned char aidPaypassMChipCvmCapabilityCvmRequired[] = {0x60};
		 unsigned char aidPaypassMChipCvmCapabilityCvmNotRequired[] = {0x08};

   #if 0
	  unsigned char aidPaypassKernelConfiguration[] = {0x20};	//
	  unsigned char aidPaypassCAPKIndexList[] = {0xF3, 0xEF, 0xF1};
   #else
	  unsigned char aidPaypassKernelConfiguration[] = {0xA0};	// msd not support, on device cvm support
	  unsigned char aidPaypassCAPKIndexList[] = {0xFA, 0xEF, 0xF1};
   #endif
		 unsigned char aidPaypassKernelTerminalSupportedLanguages[] = {0x65, 0x6E};
		 unsigned char aidPaypassTermType[] = {0x21};
		 unsigned char aidPaypassTerminalRiskManagment[] = {0x6C, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};

		 if (ulCvmRequiredLimit <= __APCLESS_ParamTrn_ulAmount)
      	 {
		  	mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
      	 }
	  	 else
	  	 {
	  	  	mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, TRUE);
	 	 }

      //TAG_PAYPASS_INT_MERCHANT_FORCE_ONLINE

      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_KERNEL_TO_USE, sizeof(aidPaypassKernelToUse), aidPaypassKernelToUse) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_OPTIONS, sizeof(aidPaypassOptions), aidPaypassOptions) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_DEFAULT_UDOL, sizeof(aidDefaultUdol), aidDefaultUdol) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, sizeof(aidPaypassFloorLimit), aidPaypassFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;

	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CLESS_TRANSACTION_LIMIT_NO_DCV, sizeof(aidPaypassTrnLimitNoDcv), aidPaypassTrnLimitNoDcv) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CLESS_TRANSACTION_LIMIT_DCV, sizeof(aidPaypassTrnLimitDcv), aidPaypassTrnLimitDcv) != STATUS_SHARED_EXCHANGE_OK) return result;
	  //if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, sizeof(aidPaypassCvmRequiredLimit), aidPaypassCvmRequiredLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, iLength, pchEMVTagData) != STATUS_SHARED_EXCHANGE_OK) return result;

	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_SECURITY_CAPABILITY, sizeof(aidPaypassSecurityCapability), aidPaypassSecurityCapability) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_CAPABILITIES, sizeof(termCapabilities), termCapabilities) != STATUS_SHARED_EXCHANGE_OK) return result;

	  if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_DEFAULT, sizeof(aidPaypassEmvIntTacDefault), aidPaypassEmvIntTacDefault) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_DENIAL, sizeof(aidPaypassEmvIntTacDenial), aidPaypassEmvIntTacDenial) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_ONLINE, sizeof(aidPaypassEmvIntTacOnline), aidPaypassEmvIntTacOnline) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CARD_DATA_INPUT_CAPABILITY, sizeof(aidPaypassCardDataInputCapability), aidPaypassCardDataInputCapability) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MCHIP_CVM_CAPABILITY_CVM_REQUIRED, sizeof(aidPaypassMChipCvmCapabilityCvmRequired), aidPaypassMChipCvmCapabilityCvmRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MCHIP_CVM_CAPABILITY_CVM_NOT_REQUIRED, sizeof(aidPaypassMChipCvmCapabilityCvmNotRequired), aidPaypassMChipCvmCapabilityCvmNotRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_KERNEL_CONFIGURATION, sizeof(aidPaypassKernelConfiguration), aidPaypassKernelConfiguration) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_INT_SUPPORTED_CAPK_INDEX_LIST, sizeof(aidPaypassCAPKIndexList), aidPaypassCAPKIndexList) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_APPLI_VERSION_NUMBER_TERM, sizeof(aidPaypassAppliVerNumTerm), aidPaypassAppliVerNumTerm) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_KERNEL_TO_USE, sizeof(aidPaypassKernelToUse), aidPaypassKernelToUse) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_OPTIONS, sizeof(aidPaypassOptions), aidPaypassOptions) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_DEFAULT_UDOL, sizeof(aidDefaultUdol), aidDefaultUdol) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, sizeof(aidPaypassFloorLimit), aidPaypassFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CLESS_TRANSACTION_LIMIT_NO_DCV, sizeof(aidPaypassTrnLimitNoDcv), aidPaypassTrnLimitNoDcv) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CLESS_TRANSACTION_LIMIT_DCV, sizeof(aidPaypassTrnLimitDcv), aidPaypassTrnLimitDcv) != STATUS_SHARED_EXCHANGE_OK) return result;
      //if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, sizeof(aidPaypassCvmRequiredLimit), aidPaypassCvmRequiredLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_SECURITY_CAPABILITY, sizeof(aidPaypassSecurityCapability), aidPaypassSecurityCapability) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_CAPABILITIES, sizeof(termCapabilities), termCapabilities) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_DEFAULT, sizeof(aidPaypassEmvIntTacDefault), aidPaypassEmvIntTacDefault) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_DENIAL, sizeof(aidPaypassEmvIntTacDenial), aidPaypassEmvIntTacDenial) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_INT_TAC_ONLINE, sizeof(aidPaypassEmvIntTacOnline), aidPaypassEmvIntTacOnline) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_CARD_DATA_INPUT_CAPABILITY, sizeof(aidPaypassCardDataInputCapability), aidPaypassCardDataInputCapability) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MCHIP_CVM_CAPABILITY_CVM_REQUIRED, sizeof(aidPaypassMChipCvmCapabilityCvmRequired), aidPaypassMChipCvmCapabilityCvmRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_MCHIP_CVM_CAPABILITY_CVM_NOT_REQUIRED, sizeof(aidPaypassMChipCvmCapabilityCvmNotRequired), aidPaypassMChipCvmCapabilityCvmNotRequired) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_KERNEL_CONFIGURATION, sizeof(aidPaypassKernelConfiguration), aidPaypassKernelConfiguration) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYPASS_INT_SUPPORTED_CAPK_INDEX_LIST, sizeof(aidPaypassCAPKIndexList), aidPaypassCAPKIndexList) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_KERNEL_TERMINAL_SUPPORTED_LANGUAGES, sizeof(aidPaypassKernelTerminalSupportedLanguages), aidPaypassKernelTerminalSupportedLanguages) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_TYPE, sizeof(aidPaypassTermType), aidPaypassTermType) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_RISK_MANAGEMENT_DATA, sizeof(aidPaypassTerminalRiskManagment), aidPaypassTerminalRiskManagment) != STATUS_SHARED_EXCHANGE_OK) return result;

      result = TRUE;

   }
   else if(aidIdentifier == APCLESS_AID_PAYWAVE_IDENTIFIER)
   {
     // Add payWave tags
      unsigned char aidpayWaveKernelToUse[] = {0x00, 0x03};             // Visa payWave kernel number = 1
      unsigned char aidpayWaveOptions[] = {0x01, 0x01, 0x00, 0x00};     // Partial AID, PPSE method
      unsigned char aidpayWaveTTQ[] =     {0x36, 0x00, 0x40, 0x00};     // Cless MSD + qVSDC & onlinePIN + sign supported + CVN17 + ISP
      //unsigned char aidpayWaveTTQ[] =     {0x36, 0x20, 0x00, 0x00};     // Cless MSD + qVSDC & onlinePIN + sign supported + CVN17 + ISP
      unsigned char aidpayWaveFddaVer[] = {0x00, 0x01};
      unsigned char aidpayWaveFloorLimit[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
      unsigned char aidpayWaveCvmRequiredLimit[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
      ///unsigned char aidPaypassFloorLimit[]= {0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
      unsigned char aidpayWaveTermType[] = {0x21};
      //unsigned char aidpayWaveCountryCode[] = {0x03, 0x60};

      T_PRE_PROCESSING_ADD_RESULT_AID tPreAddResult;

      if (ulCtlsFloorLimit < __APCLESS_ParamTrn_ulAmount)
      {
    	  aidpayWaveTTQ[1] |= PAYWAVE_TTQ_BYTE_2_ONLINE_CRYPTOGRAM_REQUIRED;
      }

      if (ulCvmRequiredLimit <= __APCLESS_ParamTrn_ulAmount)
      {
    	  aidpayWaveTTQ[1] |= PAYWAVE_TTQ_BYTE_2_CVM_REQUIRED;
		  mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
      }
	  else
	  {
	  	  mtiMapPutInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM, TRUE);

	  }

	  dmsg("<<<<<CVM LIMIT FLAG>>>>> = %d", mtiMapGetInt(tpTranData, KEY_CLESS_CVM_LIMIT_NOCVM));

      dbuf("=== aidpayWaveTTQ", aidpayWaveTTQ, 4);

      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_KERNEL_TO_USE, sizeof(aidpayWaveKernelToUse), aidpayWaveKernelToUse) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_OPTIONS, sizeof(aidpayWaveOptions), aidpayWaveOptions) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYWAVE_TERMINAL_TRANSACTION_QUALIFIERS, sizeof(aidpayWaveTTQ), aidpayWaveTTQ) != STATUS_SHARED_EXCHANGE_OK) return result;
      //TAG_EP_TERMINAL_TRANSACTION_QUALIFIERS
      //if(GTL_SharedExchange_AddTag(buffer, TAG_EP_TERMINAL_TRANSACTION_QUALIFIERS, sizeof(aidpayWaveTTQ), aidpayWaveTTQ) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_PAYWAVE_TERM_SUPPORTED_FDDA_VERSIONS, sizeof(aidpayWaveFddaVer), aidpayWaveFddaVer) != STATUS_SHARED_EXCHANGE_OK) return result;
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_CAPABILITIES, sizeof(termCapabilities), termCapabilities) != STATUS_SHARED_EXCHANGE_OK) return result;

      if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_FLOOR_LIMIT, sizeof(aidpayWaveFloorLimit), aidpayWaveFloorLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
      //if(GTL_SharedExchange_AddTag(buffer, TAG_PAYWAVE_CARD_CVM_LIMIT, sizeof(aidpayWaveCvmRequiredLimit), aidpayWaveCvmRequiredLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
      //if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, sizeof(aidpayWaveCvmRequiredLimit), aidpayWaveCvmRequiredLimit) != STATUS_SHARED_EXCHANGE_OK) return result;
	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, iLength, pchEMVTagData) != STATUS_SHARED_EXCHANGE_OK) return result;
	  
      if(GTL_SharedExchange_AddTag(buffer, TAG_EMV_TERMINAL_TYPE, sizeof(aidpayWaveTermType), aidpayWaveTermType) != STATUS_SHARED_EXCHANGE_OK) return result;

	  tPreAddResult.bStatusCheckRequested = 1;
	  tPreAddResult.bClessCvmLimitExceeded = 1;
	  tPreAddResult.bClessFloorLimitExceeded = 0;
	  tPreAddResult.bClessTransactionLimitExceeded = 0;

	  if(GTL_SharedExchange_AddTag(buffer, TAG_EP_AID_ADDITIONAL_RESULTS, sizeof(tPreAddResult), (unsigned char *)&tPreAddResult) != STATUS_SHARED_EXCHANGE_OK) return result;

	  //TAG_PAYWAVE_AVAILABLE_OFFLINE_SPENDING_AMOUNT
      result = TRUE;
   }


   return result;
}
#endif

/**
 * Add the generic transaction data in the kernel shared buffer (date, time, amount, etc).
 * @param[out] kernelSharedData Shared exchange structure filled with the generic transaction data.
 * @return
 *      - \a TRUE if correctly performed.
 *      - \a FALSE if an error occurred.
 */
int APCLESS_Kernel_AddTransactionGenericData(T_SHARED_DATA_STRUCT* kernelSharedData)
{
   int result, cr;
   unsigned char transactionType;
   NO_SERIE serial;

   result = FALSE;

   if(kernelSharedData == NULL)
   {
      return result;
   }

   // Add the transaction date
   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_TRANSACTION_DATE, 3, APCLESS_ParamTrn_GetDate());
   if(cr != STATUS_SHARED_EXCHANGE_OK)
   {
      return result;
   }

   // Add the transaction time
   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_TRANSACTION_TIME, 3, APCLESS_ParamTrn_GetTime());
   if(cr != STATUS_SHARED_EXCHANGE_OK)
   {
      return result;
   }

   // Add the amount and the currency code (if present)
   if(APCLESS_ParamTrn_GetAmountBcd() != NULL)
   {
      // Add the transaction amount numeric (bcd)
      cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_AMOUNT_AUTH_NUM, 6, APCLESS_ParamTrn_GetAmountBcd());
      if (cr != STATUS_SHARED_EXCHANGE_OK)
      {
         return result;
      }

      // Add the transaction amount binary
      cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_AMOUNT_AUTH_BIN, 4, APCLESS_ParamTrn_GetAmountBin());
      if (cr != STATUS_SHARED_EXCHANGE_OK)
      {
         return result;
      }

      // Add the transaction amount other ... todo if necessary
	  //@@WAY PARAM PAYPASS 20210127
      cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_AMOUNT_OTHER_NUM, 6, (unsigned char *)"\x00\x00\x00\x00\x00\x00");
      if (cr != STATUS_SHARED_EXCHANGE_OK)
      {
         return result;
      }
	  //

      // Add the currency code
      cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_TRANSACTION_CURRENCY_CODE, 2, APCLESS_ParamTrn_GetCurrencyCode());
      if (cr != STATUS_SHARED_EXCHANGE_OK)
      {
         return result;
      }

      // Add the currency exponent
      cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_TRANSACTION_CURRENCY_EXPONENT, 1, APCLESS_ParamTrn_GetCurrencyExponent());
      if (cr != STATUS_SHARED_EXCHANGE_OK)
      {
         return result;
      }

      // Add the currency code
	  cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_TERMINAL_COUNTRY_CODE, 2, APCLESS_ParamTrn_GetCurrencyCode());
	  if (cr != STATUS_SHARED_EXCHANGE_OK)
	  {
	     return result;
	  }
   }

   // Add the internal transaction type
   transactionType = APCLESS_ParamTrn_GetTransactionType();
   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_TRANSACTION_TYPE, 1, &transactionType);
   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      return result;
   }

   // Add the standard transaction type
   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_TRANSACTION_TYPE, 1, &transactionType);
   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      return result;
   }

   // Extract Serial Number and add it to the data
   PSQ_Give_Serial_Number(serial);
   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_IFD_SERIAL_NUMBER, 8, (unsigned char *)serial);
   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      return result;
   }

   // Add the transaction sequence counter
   if (APCLESS_ParamTrn_AddTscToSharedBuffer(kernelSharedData) == FALSE)
   {
      return result;
   }

   result = TRUE;
   return result;
}


/**
 * Find the correct AID parameters (to perform the transaction) according to the application selection result and add
 * the parameters to the kernel shared buffer.
 * @param[in] selectionSharedData Shared buffer containing the application selection results.
 * @param[out] kernelSharedData Shared buffer to be filled with the correct AID parameters (depending on the Application Selection results).
 * @param[out] kernelToUse Indicates the kernel to be used for the transaction.
 * @return
 *      - \a TRUE if correctly performed.
 *      - \a FALSE if an error occurred.
 */
 //@@WAY, 20190520 PAYPASS
#if 1
int APCLESS_Kernel_AddAidRelatedData(T_SHARED_DATA_STRUCT* selectionSharedData, T_SHARED_DATA_STRUCT* kernelSharedData, int* kernelToUse)
{
   	int result = FALSE;
   	int cr, cr2;
   	int position;
   	int aidIndex = 0xff;
   	unsigned long readTag;
   	unsigned long readLength;
   	const unsigned char* readValue;
   	T_SHARED_DATA_STRUCT candidateElementTags;
   	unsigned char* cardAid;
   	unsigned int  cardAidLength;

	tMtiMap tMapCAPK;
	tMtiMap *tpMapCAPK = &tMapCAPK;
	//@@WAY PARAM PAYPASS 20210127
#if 1
	int iRet = 0;
	UCHAR ucpKeyIndexList[100];
	int iKeyIdxListNum = 0;
#else
	UCHAR ucpKeyIndexList[100];
	int iKeyIdxListNum = 0, iIdx = 0, iRet = 0;
#endif
	//
#define END_PROC()													\
   	   if(result == FALSE) {										\
		  GTL_SharedExchange_ClearEx (kernelSharedData, FALSE); }	\
   		/*//@@WAY PARAM PAYPASS 20210127*/return result;

   if ((kernelToUse) != NULL)
      *kernelToUse = DEFAULT_EP_KERNEL_UNKNOWN;

   if ((selectionSharedData == NULL) || (kernelSharedData == NULL))
   {
      END_PROC();
   }

   // TODO: Find the good Candidate Element in the Candidate List sent by EntryPoint
   // (it is possible to have several candidate element at the same time)
   // For this sample, we just use the first candidate element (no further check).
   position = SHARED_EXCHANGE_POSITION_NULL;
   if (GTL_SharedExchange_FindNext(selectionSharedData, &position, TAG_EP_CANDIDATE_LIST_ELEMENT, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }

   // init the candidateElementTags shared buffer with TAG_EP_CANDIDATE_LIST_ELEMENT found
   if (GTL_SharedExchange_InitEx(&candidateElementTags, readLength, readLength, (unsigned char *)readValue) != STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }

   // Get the Kernel to Use
   position = SHARED_EXCHANGE_POSITION_NULL;
   if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_EP_KERNEL_TO_USE, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }
   *kernelToUse = readValue[1] + (readValue[0] << 8);

   // Get the AID proprietary identifier
   position = SHARED_EXCHANGE_POSITION_NULL;
   if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }
   aidIndex = readValue[3] + (readValue[2] << 8) + (readValue[1] << 16) + (readValue[0] << 24);

   // Get the AID from TAG_EMV_AID_CARD or from TAG_EMV_DF_NAME
   position = SHARED_EXCHANGE_POSITION_NULL;
   if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_EMV_AID_CARD, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      position = SHARED_EXCHANGE_POSITION_NULL;
      if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_EMV_DF_NAME, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }
   }
   cardAid = (unsigned char*)readValue;
   cardAidLength = readLength;
	
   // Add the AID's parameters to the kernel shared buffer
   if(__APCLESS_Kernel_CopyAIDParamToSharedBuffer(aidIndex, kernelSharedData) == FALSE)
   {  // an error occurs
      END_PROC();
   }

//@@WAY PARAM PAYPASS 20210127
#if 1
	mtiMapInit(tpMapCAPK);
	mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, cardAid, cardAidLength);
	iRet = initEMVAIDParameter(tpMapCAPK);
	if(iRet)
	{
		UCHAR *ucpAddTag = NULL;
		INT iAddTagLen = 0;

		ucpAddTag = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_TERMINAL_CAPABILITIES, &iAddTagLen);
		dbuf("MTI_EMV_TAG_TERMINAL_CAPABILITIES", ucpAddTag, iAddTagLen);
		if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_TERMINAL_CAPABILITIES, iAddTagLen, ucpAddTag) != STATUS_SHARED_EXCHANGE_OK) 
		{
			mtiMapClear(tpMapCAPK);
			END_PROC();
		}

		ucpAddTag = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES, &iAddTagLen);
		dbuf("MTI_EMV_TAG_ADD_TERMINAL_CAPABILITIES", ucpAddTag, iAddTagLen);
		if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_ADD_TERMINAL_CAPABILITIES, iAddTagLen, ucpAddTag) != STATUS_SHARED_EXCHANGE_OK) 
		{
			mtiMapClear(tpMapCAPK);
			END_PROC();
		}
		
		//@@WAY JCB Cless
		if(aidIndex == APCLESS_AID_JSPEEDY_IDENTIFIER)
		{
			ucpAddTag = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_TAC_ONLINE, &iAddTagLen);
			dbuf("MTI_EMV_TAG_INT_TAC_ONLINE", ucpAddTag, iAddTagLen);
			if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_TAC_ONLINE, iAddTagLen, ucpAddTag) != STATUS_SHARED_EXCHANGE_OK) 
			{
				mtiMapClear(tpMapCAPK);
				END_PROC();
			}

			ucpAddTag = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_TAC_DEFAULT, &iAddTagLen);
			dbuf("MTI_EMV_TAG_INT_TAC_DEFAULT", ucpAddTag, iAddTagLen);
			if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_TAC_DEFAULT, iAddTagLen, ucpAddTag) != STATUS_SHARED_EXCHANGE_OK) 
			{
				mtiMapClear(tpMapCAPK);
				END_PROC();
			}
			
			ucpAddTag = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_TAC_DENIAL, &iAddTagLen);
			dbuf("MTI_EMV_TAG_INT_TAC_DENIAL", ucpAddTag, iAddTagLen);
			if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_TAC_DENIAL, iAddTagLen, ucpAddTag) != STATUS_SHARED_EXCHANGE_OK)
			{
				mtiMapClear(tpMapCAPK);
				END_PROC();
			}
		}
		//
	}

	if(aidIndex == APCLESS_AID_PAYPASS_IDENTIFIER)
	{
		mtiMapClear(tpMapCAPK);
		mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, cardAid, cardAidLength);
		iGetCAPKeyIdxList(tpMapCAPK, ucpKeyIndexList, sizeof(ucpKeyIndexList), &iKeyIdxListNum);
		dbuf("iGetCAPKeyIdxList", ucpKeyIndexList, iKeyIdxListNum);
		if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_PAYPASS_INT_SUPPORTED_CAPK_INDEX_LIST, iKeyIdxListNum, ucpKeyIndexList) != STATUS_SHARED_EXCHANGE_OK)
		{
			mtiMapClear(tpMapCAPK);
			END_PROC();
		}
	}
	else if(aidIndex == APCLESS_AID_JSPEEDY_IDENTIFIER)
	{
		mtiMapClear(tpMapCAPK);
		mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, cardAid, cardAidLength);
		iGetCAPKeyIdxList(tpMapCAPK, ucpKeyIndexList, sizeof(ucpKeyIndexList), &iKeyIdxListNum);
		dbuf("iGetCAPKeyIdxList", ucpKeyIndexList, iKeyIdxListNum);
		if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_JCB_INT_SUPPORTED_CAPK_INDEX_LIST, iKeyIdxListNum, ucpKeyIndexList) != STATUS_SHARED_EXCHANGE_OK)
		{
			mtiMapClear(tpMapCAPK);
			END_PROC();
		}
	}
	mtiMapClear(tpMapCAPK);
#else
	mtiMapInit(tpMapCAPK);
	mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, cardAid, cardAidLength);
	iGetCAPKeyIdxList(tpMapCAPK, ucpKeyIndexList, sizeof(ucpKeyIndexList), &iKeyIdxListNum);

	if(aidIndex == APCLESS_AID_PAYPASS_IDENTIFIER)
	{
		if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_PAYPASS_INT_SUPPORTED_CAPK_INDEX_LIST, iKeyIdxListNum, ucpKeyIndexList) != STATUS_SHARED_EXCHANGE_OK)
		{
			mtiMapClear(tpMapCAPK);
			END_PROC();
		}
	}

	for(iIdx = 0; iIdx < iKeyIdxListNum; iIdx++)
	{
		UCHAR *ucpKeyData = NULL, *ucpExponent = NULL;
		INT iKeyLen = 0, iExpLen = 0;

		mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_CA_PUBLIC_KEY_INDEX_CARD, &ucpKeyIndexList[iIdx], sizeof(UCHAR));

		iRet = initEMVCAPK(tpMapCAPK);
		if(iRet)
		{
			ucpExponent = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_EXPONENT, &iExpLen);
			ucpKeyData = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_MODULUS, &iKeyLen);

			if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_MODULUS, iKeyLen, ucpKeyData) != STATUS_SHARED_EXCHANGE_OK)
			{
				mtiMapClear(tpMapCAPK);
				END_PROC();
			}

			if(GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_EXPONENT, iExpLen, ucpExponent) != STATUS_SHARED_EXCHANGE_OK)
			{
				mtiMapClear(tpMapCAPK);
				END_PROC();
			}
		}
	}
	mtiMapClear(tpMapCAPK);
#endif
	//
	
   // Add some of the candidate element tags to the kernel shared buffer
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr2 = STATUS_SHARED_EXCHANGE_OK;
   while (cr2 == STATUS_SHARED_EXCHANGE_OK)
   {
      cr2 = GTL_SharedExchange_GetNext(&candidateElementTags, &position, &readTag, &readLength, &readValue);
      if (cr2 == STATUS_SHARED_EXCHANGE_OK)
      {
         switch (readTag)
         {
            case TAG_EP_AID_ADDITIONAL_RESULTS:
            case TAG_EMV_AID_TERMINAL:
            case TAG_EP_CLESS_APPLI_CAPABILITY_TYPE:
            case TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER:
               cr2 = GTL_SharedExchange_AddTag(kernelSharedData, readTag, readLength, readValue);
               if (cr2 != STATUS_SHARED_EXCHANGE_OK)
               {
                  END_PROC();
               }
               break;
            default:
               break;
         }
      }
   }

   // Add Final Select SW
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext (selectionSharedData, &position, TAG_EP_FINAL_SELECT_STATUS_WORD, &readLength, &readValue);
   if (cr == STATUS_SHARED_EXCHANGE_OK)  // If tag with AID informations found
   {
      cr2 = GTL_SharedExchange_AddTag (kernelSharedData, TAG_EP_FINAL_SELECT_STATUS_WORD, readLength, readValue);
      if (cr2 != STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }
   }
   else
   {
      END_PROC();
   }

   // Add Final Select Response
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext (selectionSharedData, &position, TAG_EP_FINAL_SELECT_RESPONSE, &readLength, &readValue);
   if (cr == STATUS_SHARED_EXCHANGE_OK)  // If tag with AID informations found
   {
      cr2 = GTL_SharedExchange_AddTag (kernelSharedData, TAG_EP_FINAL_SELECT_RESPONSE, readLength, readValue);
      if (cr2 != STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }
   }
   else
   {
      // No error management, as a card could answer only a SW without data (in case of processing error)
   }

   // Add Final Select Command
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext (selectionSharedData, &position, TAG_EP_FINAL_SELECT_COMMAND_SENT, &readLength, &readValue);
   if (cr == STATUS_SHARED_EXCHANGE_OK)  // If tag with AID informations found
   {
      cr2 = GTL_SharedExchange_AddTag (kernelSharedData, TAG_EP_FINAL_SELECT_COMMAND_SENT, readLength, readValue);
      if (cr2 != STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }
	
      result = TRUE;
   }
   return result;
}
#else
int g_iPaypassKeyIndex = 0;
int APCLESS_Kernel_AddAidRelatedData(T_SHARED_DATA_STRUCT* selectionSharedData, T_SHARED_DATA_STRUCT* kernelSharedData, int* kernelToUse)
{
   int result = FALSE;
   int cr, cr2;
   int position;
   int aidIndex = 0xff;
   unsigned long readTag;
   unsigned long readLength;
   const unsigned char* readValue;
   T_SHARED_DATA_STRUCT candidateElementTags;
   unsigned char* cardAid;
   unsigned int  cardAidLength;

#define END_PROC()													\
   	   if(result == FALSE) {										\
		  GTL_SharedExchange_ClearEx (kernelSharedData, FALSE); }	\

   if ((*kernelToUse) != NULL)
      *kernelToUse = DEFAULT_EP_KERNEL_UNKNOWN;

   if ((selectionSharedData == NULL) || (kernelSharedData == NULL))
   {
      END_PROC();
   }

   // TODO: Find the good Candidate Element in the Candidate List sent by EntryPoint
   // (it is possible to have several candidate element at the same time)
   // For this sample, we just use the first candidate element (no further check).
   position = SHARED_EXCHANGE_POSITION_NULL;
   if (GTL_SharedExchange_FindNext(selectionSharedData, &position, TAG_EP_CANDIDATE_LIST_ELEMENT, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }

   // init the candidateElementTags shared buffer with TAG_EP_CANDIDATE_LIST_ELEMENT found
   if (GTL_SharedExchange_InitEx(&candidateElementTags, readLength, readLength, (unsigned char *)readValue) != STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }

   // Get the Kernel to Use
   position = SHARED_EXCHANGE_POSITION_NULL;
   if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_EP_KERNEL_TO_USE, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }
   *kernelToUse = readValue[1] + (readValue[0] << 8);

   // Get the AID proprietary identifier
   position = SHARED_EXCHANGE_POSITION_NULL;
   if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      END_PROC();
   }
   aidIndex = readValue[3] + (readValue[2] << 8) + (readValue[1] << 16) + (readValue[0] << 24);


   // Get the AID from TAG_EMV_AID_CARD or from TAG_EMV_DF_NAME
   position = SHARED_EXCHANGE_POSITION_NULL;
   if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_EMV_AID_CARD, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
   {
      position = SHARED_EXCHANGE_POSITION_NULL;
      if(GTL_SharedExchange_FindNext (&candidateElementTags, &position, TAG_EMV_DF_NAME, &readLength, &readValue)!= STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }
   }
   cardAid = (unsigned char*)readValue;
   cardAidLength = readLength;


   // Add the AID's parameters to the kernel shared buffer
   if(__APCLESS_Kernel_CopyAIDParamToSharedBuffer(aidIndex, kernelSharedData) == FALSE)
   {  // an error occurs
      END_PROC();
   }

   dmsg("**** g_iPaypassKeyIndex = %d", g_iPaypassKeyIndex);
   dmsg("All input !!!!!!!!!!!!");
   /******************************************************************************************************/
   // Copy Modulus
   //if (g_iPaypassKeyIndex == 0)
   {
	   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_MODULUS,
			   sizeof(__APCLESS_PayPass_caKeyFAModulus), __APCLESS_PayPass_caKeyFAModulus);
	   if (cr != STATUS_SHARED_EXCHANGE_OK)
	   {
		   END_PROC();
	   }
	   dpt();

	   // Copy Exponent
	   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_EXPONENT,
			   sizeof(__APCLESS_PayPass_caKeyFAExponent), __APCLESS_PayPass_caKeyFAExponent);
	   if (cr != STATUS_SHARED_EXCHANGE_OK)
	   {
		   END_PROC();
	   }
	   dpt();
   }
   //else if (g_iPaypassKeyIndex == 1)
   {
	   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_MODULUS,
			   sizeof(__APCLESS_PayPass_caKeyF1Modulus), __APCLESS_PayPass_caKeyF1Modulus);
	   if (cr != STATUS_SHARED_EXCHANGE_OK)
	   {
		   END_PROC();
	   }
	   dpt();

	   // Copy Exponent
	   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_EXPONENT,
			   sizeof(__APCLESS_PayPass_caKeyF1Exponent), __APCLESS_PayPass_caKeyF1Exponent);
	   if (cr != STATUS_SHARED_EXCHANGE_OK)
	   {
		   END_PROC();
	   }
	   dpt();
   }
   //else if (g_iPaypassKeyIndex == 2)
   {
   	   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_MODULUS,
   			   sizeof(__APCLESS_PayPass_caKeyEFModulus), __APCLESS_PayPass_caKeyEFModulus);
   	   if (cr != STATUS_SHARED_EXCHANGE_OK)
   	   {
   		   END_PROC();
   	   }
   	   dpt();

   	   // Copy Exponent
   	   cr = GTL_SharedExchange_AddTag(kernelSharedData, TAG_EMV_INT_CAPK_EXPONENT,
   			   sizeof(__APCLESS_PayPass_caKeyEFExponent), __APCLESS_PayPass_caKeyEFExponent);
   	   if (cr != STATUS_SHARED_EXCHANGE_OK)
   	   {
   		   END_PROC();
   	   }
   	   dpt();
      }
   /******************************************************************************************************/

   // Add some of the candidate element tags to the kernel shared buffer
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr2 = STATUS_SHARED_EXCHANGE_OK;
   while (cr2 == STATUS_SHARED_EXCHANGE_OK)
   {
      cr2 = GTL_SharedExchange_GetNext(&candidateElementTags, &position, &readTag, &readLength, &readValue);

      if (cr2 == STATUS_SHARED_EXCHANGE_OK)
      {
         switch (readTag)
         {
            case TAG_EP_AID_ADDITIONAL_RESULTS:
            case TAG_EMV_AID_TERMINAL:
            case TAG_EP_CLESS_APPLI_CAPABILITY_TYPE:
            case TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER:
               cr2 = GTL_SharedExchange_AddTag(kernelSharedData, readTag, readLength, readValue);
               if (cr2 != STATUS_SHARED_EXCHANGE_OK)
               {
                  END_PROC();
               }
               break;

            default:
               break;
         }
      }
   }

   // Add Final Select SW
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext (selectionSharedData, &position, TAG_EP_FINAL_SELECT_STATUS_WORD, &readLength, &readValue);
   if (cr == STATUS_SHARED_EXCHANGE_OK)  // If tag with AID informations found
   {
      cr2 = GTL_SharedExchange_AddTag (kernelSharedData, TAG_EP_FINAL_SELECT_STATUS_WORD, readLength, readValue);
      if (cr2 != STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }
   }
   else
   {
      END_PROC();
   }

   // Add Final Select Response
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext (selectionSharedData, &position, TAG_EP_FINAL_SELECT_RESPONSE, &readLength, &readValue);
   if (cr == STATUS_SHARED_EXCHANGE_OK)  // If tag with AID informations found
   {
      cr2 = GTL_SharedExchange_AddTag (kernelSharedData, TAG_EP_FINAL_SELECT_RESPONSE, readLength, readValue);
      if (cr2 != STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }
   }
   else
   {
      // No error management, as a card could answer only a SW without data (in case of processing error)
   }

   // Add Final Select Command
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext (selectionSharedData, &position, TAG_EP_FINAL_SELECT_COMMAND_SENT, &readLength, &readValue);
   if (cr == STATUS_SHARED_EXCHANGE_OK)  // If tag with AID informations found
   {
      cr2 = GTL_SharedExchange_AddTag (kernelSharedData, TAG_EP_FINAL_SELECT_COMMAND_SENT, readLength, readValue);
      if (cr2 != STATUS_SHARED_EXCHANGE_OK)
      {
         END_PROC();
      }

      result = TRUE;
   }
   return result;
}
#endif
