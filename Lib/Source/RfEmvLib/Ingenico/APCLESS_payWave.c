/**
 * \file
 * This module manages a Visa payWave transaction.
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


/////////////////////////////////////////////////////////////////
//// Includes ///////////////////////////////////////////////////
#include "sdk_tplus.h"
#include "APCLESS.h"

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

#define __APCLESS_PAYWAVE_CARD_REPRESENT_TO           20    ///< Time out in seconds for representing the card

/////////////////////////////////////////////////////////////////
//// Global data definition /////////////////////////////////////

#if 0

static unsigned char __APCLESS_payWave_caKey51Modulus[] =
     {0xDB, 0x5F, 0xA2, 0x9D, 0x1F, 0xDA, 0x8C, 0x16, 0x34, 0xB0, 0x4D, 0xCC, 0xFF, 0x14, 0x8A, 0xBE,
      0xE6, 0x3C, 0x77, 0x20, 0x35, 0xC7, 0x98, 0x51, 0xD3, 0x51, 0x21, 0x07, 0x58, 0x6E, 0x02, 0xA9,
      0x17, 0xF7, 0xC7, 0xE8, 0x85, 0xE7, 0xC4, 0xA7, 0xD5, 0x29, 0x71, 0x0A, 0x14, 0x53, 0x34, 0xCE,
      0x67, 0xDC, 0x41, 0x2C, 0xB1, 0x59, 0x7B, 0x77, 0xAA, 0x25, 0x43, 0xB9, 0x8D, 0x19, 0xCF, 0x2C,
      0xB8, 0x0C, 0x52, 0x2B, 0xDB, 0xEA, 0x0F, 0x1B, 0x11, 0x3F, 0xA2, 0xC8, 0x62, 0x16, 0xC8, 0xC6,
      0x10, 0xA2, 0xD5, 0x8F, 0x29, 0xCF, 0x33, 0x55, 0xCE, 0xB1, 0xBD, 0x3E, 0xF4, 0x10, 0xD1, 0xED,
      0xD1, 0xF7, 0xAE, 0x0F, 0x16, 0x89, 0x79, 0x79, 0xDE, 0x28, 0xC6, 0xEF, 0x29, 0x3E, 0x0A, 0x19,
      0x28, 0x2B, 0xD1, 0xD7, 0x93, 0xF1, 0x33, 0x15, 0x23, 0xFC, 0x71, 0xA2, 0x28, 0x80, 0x04, 0x68,
      0xC0, 0x1A, 0x36, 0x53, 0xD1, 0x4C, 0x6B, 0x48, 0x51, 0xA5, 0xC0, 0x29, 0x47, 0x8E, 0x75, 0x7F};

static unsigned char __APCLESS_payWave_caKey51Exponent[] = {0x03};

#endif

static unsigned char __APCLESS_payWave_caKey92Modulus[] =
{
	0x99, 0x6A, 0xF5, 0x6F, 0x56, 0x91, 0x87, 0xD0,
	0x92, 0x93, 0xC1, 0x48, 0x10, 0x45, 0x0E, 0xD8,
	0xEE, 0x33, 0x57, 0x39, 0x7B, 0x18, 0xA2, 0x45,
	0x8E, 0xFA, 0xA9, 0x2D, 0xA3, 0xB6, 0xDF, 0x65,
	0x14, 0xEC, 0x06, 0x01, 0x95, 0x31, 0x8F, 0xD4,
	0x3B, 0xE9, 0xB8, 0xF0, 0xCC, 0x66, 0x9E, 0x3F,
	0x84, 0x40, 0x57, 0xCB, 0xDD, 0xF8, 0xBD, 0xA1,
	0x91, 0xBB, 0x64, 0x47, 0x3B, 0xC8, 0xDC, 0x9A,
	0x73, 0x0D, 0xB8, 0xF6, 0xB4, 0xED, 0xE3, 0x92,
	0x41, 0x86, 0xFF, 0xD9, 0xB8, 0xC7, 0x73, 0x57,
	0x89, 0xC2, 0x3A, 0x36, 0xBA, 0x0B, 0x8A, 0xF6,
	0x53, 0x72, 0xEB, 0x57, 0xEA, 0x5D, 0x89, 0xE7,
	0xD1, 0x4E, 0x9C, 0x7B, 0x6B, 0x55, 0x74, 0x60,
	0xF1, 0x08, 0x85, 0xDA, 0x16, 0xAC, 0x92, 0x3F,
	0x15, 0xAF, 0x37, 0x58, 0xF0, 0xF0, 0x3E, 0xBD,
	0x3C, 0x5C, 0x2C, 0x94, 0x9C, 0xBA, 0x30, 0x6D,
	0xB4, 0x4E, 0x6A, 0x2C, 0x07, 0x6C, 0x5F, 0x67,
	0xE2, 0x81, 0xD7, 0xEF, 0x56, 0x78, 0x5D, 0xC4,
	0xD7, 0x59, 0x45, 0xE4, 0x91, 0xF0, 0x19, 0x18,
	0x80, 0x0A, 0x9E, 0x2D, 0xC6, 0x6F, 0x60, 0x08,
	0x05, 0x66, 0xCE, 0x0D, 0xAF, 0x8D, 0x17, 0xEA,
	0xD4, 0x6A, 0xD8, 0xE3, 0x0A, 0x24, 0x7C, 0x9F
};

static unsigned char __APCLESS_payWave_caKey92Exponent[] = {0x03};

static unsigned char __APCLESS_payWave_caKey94Modulus[] =
{
	0xAC, 0xD2, 0xB1, 0x23, 0x02, 0xEE, 0x64, 0x4F,
	0x3F, 0x83, 0x5A, 0xBD, 0x1F, 0xC7, 0xA6, 0xF6,
	0x2C, 0xCE, 0x48, 0xFF, 0xEC, 0x62, 0x2A, 0xA8,
	0xEF, 0x06, 0x2B, 0xEF, 0x6F, 0xB8, 0xBA, 0x8B,
	0xC6, 0x8B, 0xBF, 0x6A, 0xB5, 0x87, 0x0E, 0xED,
	0x57, 0x9B, 0xC3, 0x97, 0x3E, 0x12, 0x13, 0x03,
	0xD3, 0x48, 0x41, 0xA7, 0x96, 0xD6, 0xDC, 0xBC,
	0x41, 0xDB, 0xF9, 0xE5, 0x2C, 0x46, 0x09, 0x79,
	0x5C, 0x0C, 0xCF, 0x7E, 0xE8, 0x6F, 0xA1, 0xD5,
	0xCB, 0x04, 0x10, 0x71, 0xED, 0x2C, 0x51, 0xD2,
	0x20, 0x2F, 0x63, 0xF1, 0x15, 0x6C, 0x58, 0xA9,
	0x2D, 0x38, 0xBC, 0x60, 0xBD, 0xF4, 0x24, 0xE1,
	0x77, 0x6E, 0x2B, 0xC9, 0x64, 0x80, 0x78, 0xA0,
	0x3B, 0x36, 0xFB, 0x55, 0x43, 0x75, 0xFC, 0x53,
	0xD5, 0x7C, 0x73, 0xF5, 0x16, 0x0E, 0xA5, 0x9F,
	0x3A, 0xFC, 0x53, 0x98, 0xEC, 0x7B, 0x67, 0x75,
	0x8D, 0x65, 0xC9, 0xBF, 0xF7, 0x82, 0x8B, 0x6B,
	0x82, 0xD4, 0xBE, 0x12, 0x4A, 0x41, 0x6A, 0xB7,
	0x30, 0x19, 0x14, 0x31, 0x1E, 0xA4, 0x62, 0xC1,
	0x9F, 0x77, 0x1F, 0x31, 0xB3, 0xB5, 0x73, 0x36,
	0x00, 0x0D, 0xFF, 0x73, 0x2D, 0x3B, 0x83, 0xDE,
	0x07, 0x05, 0x2D, 0x73, 0x03, 0x54, 0xD2, 0x97,
	0xBE, 0xC7, 0x28, 0x71, 0xDC, 0xCF, 0x0E, 0x19,
	0x3F, 0x17, 0x1A, 0xBA, 0x27, 0xEE, 0x46, 0x4C,
	0x6A, 0x97, 0x69, 0x09, 0x43, 0xD5, 0x9B, 0xDA,
	0xBB, 0x2A, 0x27, 0xEB, 0x71, 0xCE, 0xEB, 0xDA,
	0xFA, 0x11, 0x76, 0x04, 0x64, 0x78, 0xFD, 0x62,
	0xFE, 0xC4, 0x52, 0xD5, 0xCA, 0x39, 0x32, 0x96,
	0x53, 0x0A, 0xA3, 0xF4, 0x19, 0x27, 0xAD, 0xFE,
	0x43, 0x4A, 0x2D, 0xF2, 0xAE, 0x30, 0x54, 0xF8,
	0x84, 0x06, 0x57, 0xA2, 0x6E, 0x0F, 0xC6, 0x17
};

static unsigned char __APCLESS_payWave_caKey94Exponent[] = {0x03};

/////////////////////////////////////////////////////////////////
//// Static functions definition ////////////////////////////////

static int __APCLESS_payWave_AddpayWaveSpecificData(T_SHARED_DATA_STRUCT* dataStruct);
//static int __APCLESS_payWave_IsPinOnLineRequested(T_SHARED_DATA_STRUCT* resultDataStruct);
static int __APCLESS_payWave_IsSignatureRequested (T_SHARED_DATA_STRUCT* resultDataStruct);
static void __APCLESS_payWave_DebugActivation(int activate);
static int __APCLESS_payWave_DisplaySreenWithBalance(unsigned long screenId, T_SHARED_DATA_STRUCT* dataStruct);
//static int __APCLESS_payWave_CheckISPConditions(T_SHARED_DATA_STRUCT* sharedStructure);
//static int __APCLESS_payWave_WaitClessCard(T_SHARED_DATA_STRUCT* dataStruct);
//static void __APCLESS_payWave_StopClessCard(void);
//static int __APCLESS_payWave_ManageAfterTrn(T_SHARED_DATA_STRUCT* dataStruct, int* stepExecuted);
//static int __APCLESS_payWave_CallAuthorisationHost(T_SHARED_DATA_STRUCT* dataStruct);
//static int __APCLESS_payWave_ManageOnLineAuthorisation(T_SHARED_DATA_STRUCT* dataStruct);
void __APCLESS_payWave_CopyCAKey(INT iIndex, T_SHARED_DATA_STRUCT* outputDataStruct);

/////////////////////////////////////////////////////////////////
//// Functions //////////////////////////////////////////////////


/**
 * Fill buffer with specific payWave for transaction.
 * @param[out] dataStruct Shared exchange structure filled with the specific payWave data.
 * @return
 *     - \a TRUE if correctly performed.
 *     - \a FALSE if an error occurred.
 */
#if 1
static int __APCLESS_payWave_AddpayWaveSpecificData(T_SHARED_DATA_STRUCT* dataStruct)
{
   int cr;
   int result = FALSE;
   T_KERNEL_TRANSACTION_FLOW_CUSTOM transactionFlowCustom;
   object_info_t objectInfo;
   unsigned char stepInterruption[KERNEL_PAYMENT_FLOW_STOP_LENGTH];// Bit field to stop payment flow,
                                                   // if all bit set to 0 => no stop during payment process
                                                           // if right bit set to 1 : stop after payment step number 1
   unsigned char stepCustom[KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH];   // Bit field to custom payment flow,
                                                   // if all bit set to 0 => no stop during payment process
                                                           // if right bit set to 1 : stop after payment step number 1
   if (dataStruct == NULL)
   {
      goto End;
   }

   // Init parameters
   memset(stepInterruption, 0, sizeof(stepInterruption)); // Default Value : not stop on process
   memset(stepCustom, 0, sizeof(stepCustom)); // Default Value : not stop on process

   // Customize steps :
   ADD_STEP_CUSTOM(STEP_PAYWAVE_MSD_REMOVE_CARD, stepCustom);         // To do GUI when MStripe card has been read
   ADD_STEP_CUSTOM(STEP_PAYWAVE_QVSDC_REMOVE_CARD, stepCustom);       // To do GUI when MChip card has been read
   ADD_STEP_CUSTOM(STEP_PAYWAVE_QVSDC_GET_CERTIFICATE, stepCustom);   // To get the certificate for ODA step

   // TODO: If BlackList present  ADD_STEP_CUSTOM(STEP_PAYWAVE_QVSDC_BLACK_LIST_CONTROL, StepCustom);   // To check Pan in Black list

   ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(), &objectInfo);


   // Add a tag for Do_Txn management
   cr = GTL_SharedExchange_AddTag(dataStruct, TAG_KERNEL_PAYMENT_FLOW_STOP, KERNEL_PAYMENT_FLOW_STOP_LENGTH, (const unsigned char *)stepInterruption);
   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      goto End;
   }

   // Add a tag for Do_Txn management
   memcpy(transactionFlowCustom.pucStepCustom, (void*)stepCustom, KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH);
   transactionFlowCustom.usApplicationType = objectInfo.application_type; // Kernel will call this application for customisation
   transactionFlowCustom.usServiceId = SERVICE_CUSTOM_KERNEL; // Kernel will call SERVICE_CUSTOM_KERNEL service id for customisation

   cr = GTL_SharedExchange_AddTag(dataStruct, TAG_KERNEL_PAYMENT_FLOW_CUSTOM, sizeof(T_KERNEL_TRANSACTION_FLOW_CUSTOM), (const unsigned char *)&transactionFlowCustom);
   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      goto End;
   }

   // TODO: Add Tag TAG_KERNEL_TERMINAL_SUPPORTED_LANGUAGES

   result = TRUE;

End:
   return result;
}

#else
static int __APCLESS_payWave_AddpayWaveSpecificData(T_SHARED_DATA_STRUCT* dataStruct)
{
   return TRUE;
}
#endif

/**
 * Get the payWave signature state.
 * @param[in] resultDataStruct Structure containing the kernel output.
 * @return
 *     - \a TRUE if signature is requested
 *     - \a FALSE if signature is not requested or an error occurred.
 */
static int __APCLESS_payWave_IsSignatureRequested(T_SHARED_DATA_STRUCT* resultDataStruct)
{
   int position, cr;
   unsigned long readLength;
   const unsigned char* readValue;

   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext(resultDataStruct, &position, TAG_KERNEL_SIGNATURE_REQUESTED, &readLength, &readValue);

   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      return FALSE;
   }

   // Get the transaction outcome
   return readValue[0];
}


/**+
 * If a balance amount is available, it displays the message with the balance. Otherwise nothing is displayed.
 * @param[in] screenId screen identifier of the message to display
 * @param[in] dataStruct shared buffer containing the balance amount (\a TAG_PAYWAVE_AVAILABLE_OFFLINE_SPENDING_AMOUNT)
 * @return
 *    - \a TRUE if no problem occurred (display done).
 *    - \a FALSE if not displayed (balance not available or an error occurred).
 */
static int __APCLESS_payWave_DisplaySreenWithBalance(unsigned long screenId, T_SHARED_DATA_STRUCT* dataStruct)
{
   int isFound;
   unsigned char* info;

   isFound = APCLESS_Tools_SharedBufferRetrieveInfo(dataStruct, TAG_PAYWAVE_AVAILABLE_OFFLINE_SPENDING_AMOUNT, &info);
   if (isFound)
   {
	  dbuf("TAG_PAYWAVE_AVAILABLE_OFFLINE_SPENDING_AMOUNT", info, 6);

      APCLESS_Gui_DisplayScreenWithBalance(screenId, "EUR", 2, info, 6);

      return TRUE;
   }

   return FALSE;
}


/**
 * Activate or not the payWave kernel debug features.
 * @param[in] activate
 *    - TRUE : activate the PayPass kernel traces
 *    - FASLE : deactivate the PayPass kernel traces
 */
static void __APCLESS_payWave_DebugActivation(int activate)
{
   T_SHARED_DATA_STRUCT* sharedStructure;
   int result;
   unsigned char debugMode = 0x00;

   if (activate)
      debugMode = KERNEL_DEBUG_MASK_TRACES;   // Provide debug information in the Trace.exe tool

   sharedStructure = GTL_SharedExchange_InitShared(256);

   if (sharedStructure != NULL)
   {
      result = GTL_SharedExchange_AddTag(sharedStructure, TAG_KERNEL_DEBUG_ACTIVATION, 1, &debugMode);

      if (result == STATUS_SHARED_EXCHANGE_OK)
      {
         result = payWave_DebugManagement(sharedStructure);
      }

      // Destroy the shared buffer
      GTL_SharedExchange_DestroyShare(sharedStructure);
   }
}

/**
 * Get the Visa payWave CA Key 51 parameters.
 * @param[out] outputDataStruct Shared buffer filled with the CA Public Key data.
 */
void __APCLESS_payWave_CopyCAKey(INT iIndex, T_SHARED_DATA_STRUCT* outputDataStruct)
{
   int cr;
   /***
   // Copy Modulus
   cr = GTL_SharedExchange_AddTag(outputDataStruct, TAG_EMV_INT_CAPK_MODULUS, sizeof(__APCLESS_payWave_caKey51Modulus), __APCLESS_payWave_caKey51Modulus);
   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      GTL_SharedExchange_ClearEx(outputDataStruct, FALSE);
      return;
   }

   // Copy Exponent
   cr = GTL_SharedExchange_AddTag(outputDataStruct, TAG_EMV_INT_CAPK_EXPONENT, sizeof(__APCLESS_payWave_caKey51Exponent), __APCLESS_payWave_caKey51Exponent);
   if (cr != STATUS_SHARED_EXCHANGE_OK)
   {
      GTL_SharedExchange_ClearEx(outputDataStruct, FALSE);
      return;
   }
   **/

   if (iIndex == 0x92)
   {
	   cr = GTL_SharedExchange_AddTag(outputDataStruct, TAG_EMV_INT_CAPK_MODULUS,
			   sizeof(__APCLESS_payWave_caKey92Modulus), __APCLESS_payWave_caKey92Modulus);

	  if (cr != STATUS_SHARED_EXCHANGE_OK)
	  {
		 GTL_SharedExchange_ClearEx(outputDataStruct, FALSE);
		 return;
	  }

	  // Copy Exponent
	  cr = GTL_SharedExchange_AddTag(outputDataStruct, TAG_EMV_INT_CAPK_EXPONENT,
			  sizeof(__APCLESS_payWave_caKey92Exponent), __APCLESS_payWave_caKey92Exponent);
	  if (cr != STATUS_SHARED_EXCHANGE_OK)
	  {
		 GTL_SharedExchange_ClearEx(outputDataStruct, FALSE);
		 return;
	  }
   }

   if (iIndex == 0x94)
   {
	   cr = GTL_SharedExchange_AddTag(outputDataStruct, TAG_EMV_INT_CAPK_MODULUS,
			   sizeof(__APCLESS_payWave_caKey94Modulus), __APCLESS_payWave_caKey94Modulus);

	  if (cr != STATUS_SHARED_EXCHANGE_OK)
	  {
		 GTL_SharedExchange_ClearEx(outputDataStruct, FALSE);
		 return;
	  }

	  // Copy Exponent
	  cr = GTL_SharedExchange_AddTag(outputDataStruct, TAG_EMV_INT_CAPK_EXPONENT,
			  sizeof(__APCLESS_payWave_caKey94Exponent), __APCLESS_payWave_caKey94Exponent);
	  if (cr != STATUS_SHARED_EXCHANGE_OK)
	  {
		 GTL_SharedExchange_ClearEx(outputDataStruct, FALSE);
		 return;
	  }
   }
}


#if 0

static void __APCLESS_payWave_CheckLimit(T_SHARED_DATA_STRUCT* dataStruct)
{

}

#endif


void APCLESS_payWave_AllData(T_SHARED_DATA_STRUCT* dataStruct)
{
	int iRet = 0, i = 0;

	unsigned long int ilTag = 0L;
	unsigned long int ilLen = 0L;
	const unsigned char *ucppValue;

	dmsg("=========================================================================");
	while (1)
	{
		dmsg("**** Index [%d]", i);
		iRet = GTL_SharedExchange_GetNext(dataStruct, &i, &ilTag, &ilLen, &ucppValue);
		if (iRet == STATUS_SHARED_EXCHANGE_END)
		{
			break;
		}
		else if (iRet == STATUS_SHARED_EXCHANGE_OK)
		{
			dmsg("TAG [%X]", (int)ilTag);
			dbuf((char*)"VALUE", (unsigned char* )ucppValue, (int)ilLen);
		}
	}
	dmsg("=========================================================================");
}



void APCLESS_Explicit_DebugTag(T_SHARED_DATA_STRUCT* dataStruct);

void APCLESS_CheckCTQ()
{
	int iRet = 0, i = 0, iPINFlag = 0;
	unsigned long int ilTag = 0L;
	unsigned long int ilLen = 0L;
	const unsigned char *ucppValue = NULL;
	T_SHARED_DATA_STRUCT* tmpShared;

	// Not used
	//tMtiMap *tpMap = mtiGetRfEmvParmInfo()->tpMainProcMap;

	tmpShared = GTL_SharedExchange_InitShared(2048);
	payWave_GetAllData(tmpShared); // Get all the kernel data

	dmsg("=========================================================================");

	while (1)
	{
		iRet = GTL_SharedExchange_GetNext(tmpShared, &i, &ilTag, &ilLen, &ucppValue);
		if (iRet == STATUS_SHARED_EXCHANGE_END)
		{
			dpt();
			break;
		}
		else if (iRet == STATUS_SHARED_EXCHANGE_OK)
		{
			dpt();
			dmsg("CHECK TAG [%X]", (INT)ilTag);
			if (ilTag == TAG_PAYWAVE_CARD_TRANSACTION_QUALIFIERS)
			{
				dbuf("TAG_PAYWAVE_CARD_TRANSACTION_QUALIFIERS", (UCHAR*)ucppValue, (INT)ilLen);

				if ((ucppValue[0] & PAYWAVE_CTQ_BYTE_1_ONLINE_PIN_REQUIRED)
						== PAYWAVE_CTQ_BYTE_1_ONLINE_PIN_REQUIRED)
				{
					dpt();
					mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_PIN_FLAG, NULL, NULL);
					iPINFlag = TRUE;
				}

				if ((ucppValue[0] & PAYWAVE_CTQ_BYTE_1_SIGNATURE_REQUIRED)
						== PAYWAVE_CTQ_BYTE_1_SIGNATURE_REQUIRED)
				{
					dpt();
					if (iPINFlag)
					{
						dpt();
						mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_SIGN, NULL, NULL);
					}
				}


				break;
			}
		}
	}

	dmsg("=========================================================================");

	GTL_SharedExchange_DestroyLocal(tmpShared);
}

void APCLESS_payWave_GetTrascationTag(tMtiMap *tpMap)
{
	int iRet = 0, i = 0/*, iSub = 0*/;
	unsigned long int ilTag = 0L;
	unsigned long int ilLen = 0L;
	const unsigned char *ucppValue = NULL;
	/***
	int iaTagLists[] = {
			TAG_EMV_APPLI_PAN,
			TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER,
			TAG_EMV_TRACK_2_EQU_DATA,
			TAG_EMV_CRYPTOGRAM_INFO_DATA,
			TAG_EMV_APPLICATION_CRYPTOGRAM,
			TAG_EMV_TVR, TAG_EMV_CVM_RESULTS,
			TAG_EMV_AMOUNT_AUTH_NUM,
			TAG_EMV_AMOUNT_OTHER_NUM,
			TAG_EMV_TRANSACTION_CURRENCY_CODE,
			TAG_EMV_TRANSACTION_DATE,
			TAG_EMV_UNPREDICTABLE_NUMBER,
			TAG_EMV_AIP,
			TAG_EMV_DF_NAME,
			TAG_EMV_ISSUER_APPLI_DATA,
			TAG_EMV_ATC,
			TAG_EMV_TRANSACTION_TYPE,
			TAG_EMV_TERMINAL_COUNTRY_CODE,
			TAG_EMV_TERMINAL_CAPABILITIES,
			TAG_EMV_TERMINAL_TYPE,
			TAG_EMV_IFD_SERIAL_NUMBER,
			TAG_EMV_APPLI_CURRENCY_CODE,
			TAG_EMV_ISSUER_COUNTRY_CODE,
			0
	};
	***/
	T_SHARED_DATA_STRUCT* tmpShared;

	tmpShared = GTL_SharedExchange_InitShared(2048);
	payWave_GetAllData(tmpShared); // Get all the kernel data

	dmsg("=========================================================================");

	while (1)
	{
		iRet = GTL_SharedExchange_GetNext(tmpShared, &i, &ilTag, &ilLen, &ucppValue);
		if (iRet == STATUS_SHARED_EXCHANGE_END)
		{
			break;
		}
		else if (iRet == STATUS_SHARED_EXCHANGE_OK)
		{
			dmsg("TAG[%X] LEN[%ld]", (UINT)ilTag, ilLen);
			dbuf("TAGDATA", (UCHAR*)ucppValue, (INT)ilLen);

			mtiMapPutBytes(tpMap, (INT)ilTag, (UCHAR*)ucppValue, (INT)ilLen);
		}
	}

	dmsg("=========================================================================");
	GTL_SharedExchange_DestroyLocal(tmpShared);
}

/**
 * Calls the payWave kernel to perform the transaction.
 * @param[in] dataStruct Data buffer to be filled and used for payWave transaction.
 * @return payWave kernel result.
 *
 * @note gui with colored LEDs is not implemented
 */
int APCLESS_payWave_PerformTransaction(T_SHARED_DATA_STRUCT* dataStruct, tMtiMap *tpMap)
{
   int cr;
   int result = C_CLESS_CR_END;
   unsigned char signature;
   unsigned char* info;
   int saveInBatch = FALSE;

   // Indicate payWave kernel is going to be used (for customisation purposes)
   APCLESS_ParamTrn_SetCurrentPaymentScheme(APCLESS_SCHEME_PAYWAVE);

   // Init payWave kernel debug traces
   __APCLESS_payWave_DebugActivation(APCLESS_ParamTrn_GetDebugTrace());

   if (__APCLESS_payWave_AddpayWaveSpecificData(dataStruct))
   {
      // Perform the payWave transaction
	  //dmsg("WAVE BEFORE DATA");
	  //APCLESS_payWave_AllData(dataStruct);
      cr = payWave_DoTransaction(dataStruct);

      //APCLESS_Explicit_DebugTag(dataStruct);
      //APCLESS_payWave_AllData(dataStruct);
      dmsg("payWave_DoTransaction() Result = 0x%04X", cr);

      // Specific treatment for Pin management
      //if (cr == KERNEL_STATUS_OFFLINE_APPROVED)
      {
    	  APCLESS_CheckCTQ();
    	  /**
         if (__APCLESS_payWave_IsPinOnLineRequested(dataStruct)) // If pin asked
         {
            //cr = KERNEL_STATUS_ONLINE_AUTHORISATION; // => mandatory to go on-line
        	 dpt();
        	 mtiMapPutInt(mtiGetRfEmvParmInfo()->tpMainProcMap, KEY_EXT_BIT_ONLINE_PIN_FLAG, TRUE);
         }
         **/
      }

      // Check if signature is requested or not
      signature = __APCLESS_payWave_IsSignatureRequested (dataStruct);

      // CR analyse
      if (cr & KERNEL_STATUS_STOPPED_BY_APPLICATION_MASK) // If mask has been set
      {
         // TODO: if a task has been launch to scan peripherals,
         //      analyse which event stops the kernel (chip, swipe, keyboard, ...)
      }
      else
      {
         switch (cr)
         {
            case KERNEL_STATUS_OK:
               // A good transaction state must be finished by a approved, declined, ...
               APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_ERROR_STATUS);
               break;

            case KERNEL_STATUS_OFFLINE_APPROVED:
               if (signature)
               {
                  APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_SIGNATURE_REQUIRED);
               }
               else
               {
                  if(__APCLESS_payWave_DisplaySreenWithBalance(APCLESS_SCREEN_OFFLINE_APPROVED, dataStruct) == FALSE)
                     APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_OFFLINE_APPROVED);

                  // Transaction shall be added to the batch
                  saveInBatch = TRUE;
               }

               // Print a receipt only if signature requested or merchant need to print a receipt
               // TODO: print ticket

               if (signature)
               {
                  // TODO: Ask merchant if signature is OK
                  // if yes  saveInBatch = TRUE;
               }

               break;

            case KERNEL_STATUS_OFFLINE_DECLINED:
            	//APCLESS_payWave_GetTrascationTag(&mtiGetRfEmvParmInfo()->tEmvTagMap);
                // TODO: if credit or refund are supported, look VisaWave 2.1 chapter 5.19 to manage
                // the case of a card AAC
               if(__APCLESS_payWave_DisplaySreenWithBalance(APCLESS_SCREEN_OFFLINE_DECLINED, dataStruct) == FALSE)
                  APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_OFFLINE_DECLINED);
               break;

            case KERNEL_STATUS_ONLINE_AUTHORISATION:
            	//Tag Log
            	APCLESS_payWave_GetTrascationTag(&mtiGetRfEmvParmInfo()->tEmvTagMap);
            	/**
               if(__APCLESS_payWave_ManageOnLineAuthorisation(dataStruct) == TRUE)
                  saveInBatch = TRUE;
                  **/
               break;

            case KERNEL_STATUS_USE_CONTACT_INTERFACE:
               result = CLESS_CR_MANAGER_RESTART_WO_CLESS;
               break;

            case KERNEL_STATUS_COMMUNICATION_ERROR:
               result = CLESS_CR_MANAGER_RESTART;
               break;

            case KERNEL_STATUS_REMOVE_AID:
               result = CLESS_CR_MANAGER_REMOVE_AID;
               break;

            case KERNEL_STATUS_MOBILE:
               if (APCLESS_Tools_SharedBufferRetrieveInfo(dataStruct, TAG_PAYWAVE_TERMINAL_TRANSACTION_QUALIFIERS, &info))
               {
                  if (info[0] & 0x20) // If qVSDC supported
                  {
                     ClessEmv_CloseDriver(); // Stop Cless Field
                     result = CLESS_CR_MANAGER_RESTART_DOUBLE_TAP;
                  }
                  else
                  {
                     // display error
                     APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_ERROR_STATUS);
                     result = CLESS_CR_MANAGER_END;
                  }
               }
               else
               {
                  result = CLESS_CR_MANAGER_RESTART_DOUBLE_TAP;
               }
               break;

            default:
               // For Visa Europe, no error should be displayed.
               // transaction shall be conducted over another interface
               result = CLESS_CR_MANAGER_RESTART_WO_CLESS;
               break;
         }
      }

      // Cless field must be stopped only if we don't try to work with an another AID
      if ((result != CLESS_CR_MANAGER_REMOVE_AID) && (result != CLESS_CR_MANAGER_RESTART_WO_CLESS) && (result != CLESS_CR_MANAGER_RESTART_DOUBLE_TAP))
      {
         // Deselect the card
    	 if (ClessEmv_IsDriverOpened())
         ClessEmv_DeselectCard(0, TRUE, FALSE);
      }

      // If the transaction does not restart from the beginning, set the LEDs into the idle state
      if ((result != CLESS_CR_MANAGER_RESTART) && (result != CLESS_CR_MANAGER_REMOVE_AID))
      {
         // Check if transaction shall be saved in the batch
         if (saveInBatch)
         {
            // TODO: Add record to the batch
         }

         // Increment the transaction sequence counter
         APCLESS_ParamTrn_IncrementTsc();
      }
   }

   // Additional possible processing :
   // - Perform an online authorisation if necessary
   // - Save the transaction in the batch if transaction is accepted
   // - Perform CVM processing if necessary

   // Transaction is completed, clear payWave kernel transaction data
   payWave_Clear();

   dmsg("APCLESS_payWave_PerformTransaction() Result = %d", result);
   return result;
}


/**
 * Perform the payWave kernel customisation.
 * @param[in,out] sharedData Shared buffer used for customisation.
 * @param[in] ucCustomisationStep Step to be customised.
 * @return \a KERNEL_STATUS_CONTINUE always.
 */
int APCLESS_payWave_KernelCustomiseStep(T_SHARED_DATA_STRUCT* sharedData, const unsigned char ucCustomisationStep)
{
   int result = KERNEL_STATUS_CONTINUE;
   int position;
   unsigned char keyIndex;
   unsigned char rid[5];
   unsigned long readLength;
   const unsigned char* readValue;
	UCHAR *ucpKeyData = NULL, *ucpExponent = NULL;
	INT iKeyLen = 0, iExpLen = 0, iRet = 0;
	tMtiMap tMapCAPK;
	tMtiMap *tpMapCAPK = &tMapCAPK;

    switch (ucCustomisationStep) // Steps to customise
    {
       case STEP_PAYWAVE_MSD_REMOVE_CARD:
       case STEP_PAYWAVE_QVSDC_REMOVE_CARD:
         APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_REMOVE_CARD);
         GTL_SharedExchange_ClearEx (sharedData, FALSE);
         result = KERNEL_STATUS_CONTINUE;
         break;

      case STEP_PAYWAVE_QVSDC_GET_CERTIFICATE:
		 mtiMapInit(tpMapCAPK);
         // Get the CA public key index (card)
         position = SHARED_EXCHANGE_POSITION_NULL;
         if (GTL_SharedExchange_FindNext (sharedData, &position, TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD, &readLength, (const unsigned char **)&readValue) == STATUS_SHARED_EXCHANGE_OK)
            keyIndex = readValue[0];
         else
            keyIndex = 0;
		 dbuf("TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD",(UCHAR*) readValue, readLength);
		 mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_CA_PUBLIC_KEY_INDEX_CARD, (UCHAR*) readValue,readLength);

         // Get RID value
         position = SHARED_EXCHANGE_POSITION_NULL;
         if (GTL_SharedExchange_FindNext (sharedData, &position, TAG_EMV_DF_NAME, &readLength, (const unsigned char **)&readValue) == STATUS_SHARED_EXCHANGE_OK)
            memcpy (rid, readValue, 5);
         else
            memset (rid, 0, sizeof(rid));
		 dbuf("TAG_PAYPASS_INT_RID",(UCHAR*) readValue, readLength);
		 mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, (UCHAR*) readValue, readLength);

         // Clear the output structure
         GTL_SharedExchange_ClearEx (sharedData, FALSE);

         // TODO: With rid & keyIndex look for the CA public key data (Modulus, exponent, etc) in the application parameters
         // for this sample we are just using index 51
         //if (keyIndex == 0x51)
         //{
           // __APCLESS_payWave_CopyCAKey(keyIndex, sharedData);
         //}
		 if(keyIndex != 0)
         {
			 iRet = initEMVCAPK(tpMapCAPK);
			 if(iRet)
			 {
				 ucpExponent = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_EXPONENT, &iExpLen);
				 ucpKeyData = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_MODULUS, &iKeyLen);
				 
			 	 dbuf("MTI_EMV_TAG_INT_CAPK_MODULUS", ucpKeyData, iKeyLen);
				 if(GTL_SharedExchange_AddTag(sharedData, TAG_EMV_INT_CAPK_MODULUS, iKeyLen, ucpKeyData) != STATUS_SHARED_EXCHANGE_OK)
				 {
					 mtiMapClear(tpMapCAPK);
					 GTL_SharedExchange_ClearEx(sharedData, FALSE);
					 return result;
         }
				 
			     dbuf("MTI_EMV_TAG_INT_CAPK_EXPONENT", ucpExponent, iExpLen);
				 if(GTL_SharedExchange_AddTag(sharedData, TAG_EMV_INT_CAPK_EXPONENT, iExpLen, ucpExponent) != STATUS_SHARED_EXCHANGE_OK)
				 {
					 mtiMapClear(tpMapCAPK);
					 GTL_SharedExchange_ClearEx(sharedData, FALSE);
					 return result;
				 }
			 }
		 }
		 mtiMapClear(tpMapCAPK);
         result = KERNEL_STATUS_CONTINUE;
         break;

      case STEP_PAYWAVE_QVSDC_BLACK_LIST_CONTROL:
         // TODO: Check if PAN is in the exception file
         result = KERNEL_STATUS_CONTINUE;
         break;

      // Other customisation steps could be defined if necessary

      default:
         break;
    }

    return result;
}

