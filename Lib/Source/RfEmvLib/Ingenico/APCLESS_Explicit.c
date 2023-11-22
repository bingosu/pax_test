/**
 * \file
 * This module implements the explicit transaction flow.
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


/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

#define __APCLESS_EXPLICIT_STEP_DETECTION                 1        ///< Explicit execution step to detect the card
#define __APCLESS_EXPLICIT_STEP_SELECTION                 2        ///< Explicit execution step to select a card application
#define __APCLESS_EXPLICIT_STEP_KERNEL_EXECUTION          3        ///< Explicit execution step to run a kernel
#define __APCLESS_EXPLICIT_STEP_KERNEL_EXECUTION_BIS      4        ///< Explicit execution step to run again the kernel (used by Visa card for payWave kernel)
#define __APCLESS_EXPLICIT_STEP_CHECK_RESULT              5        ///< Explicit execution step to check kernel result ( to manage restart)
#define __APCLESS_EXPLICIT_STEP_END                       99       ///< Explicit execution step to stop processing


/////////////////////////////////////////////////////////////////
//// Global data definition /////////////////////////////////////


/////////////////////////////////////////////////////////////////
//// Static functions definition ////////////////////////////////


/////////////////////////////////////////////////////////////////
//// Functions //////////////////////////////////////////////////

void APCLESS_Explicit_DebugTag(T_SHARED_DATA_STRUCT* dataStruct)
{
	int iRet = 0, i = 0;//, iSub = 0;
	unsigned long int ilTag = 0L;
	unsigned long int ilLen = 0L;
	const unsigned char *ucppValue = NULL;

	//If you want watching to tag, add to item in below array.
#if 0
	int iaTagLists[] = {
			0x9F66,
			0x9F6C,
	};
#endif
	
	
	dmsg("=========================================================================");
	dmsg("                             T A G - D E B U G");
	dmsg("=========================================================================");
	while (1)
	{
		iRet = GTL_SharedExchange_GetNext(dataStruct, &i, &ilTag, &ilLen, &ucppValue);
		//dmsg("Index [%d]", i);
		if (iRet == STATUS_SHARED_EXCHANGE_END)
		{
			dpt();
			break;
		}
		else if (iRet == STATUS_SHARED_EXCHANGE_OK)
		{
#if 1
			dmsg("TAG[%X] LEN[%ld]", (UINT)ilTag, ilLen);
			dbuf("TAGDATA", (UCHAR*)ucppValue, (INT)ilLen);
#else
			for (iSub = 0; iSub < DIM_SIZEOF(iaTagLists); iSub++)
			{
				if (ilTag == iaTagLists[iSub])
				{
					dmsg("TAG[%X] LEN[%ld]", iaTagLists[iSub], ilLen);
					dbuf("TAGDATA", (UCHAR*)ucppValue, (INT)ilLen);
					break;
				}
			}
#endif
		}
	}

	dmsg("=========================================================================");
}

void APCLESS_Explicit_GetTrascationTag(tMtiMap *tpMap, T_SHARED_DATA_STRUCT* dataStruct)
{
	int iRet = 0, i = 0;
	//int iSub = 0;
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
 * This function performs all a contactless transaction using the explicit selection method.
 * The main steps are:
 *  - parameters initialization
 *  - card detection and application selection
 *  - contactless kernel execution
 */
extern int g_iRetGuiCustomisation;
extern int g_iRetGuiStep;
int APCLESS_Explicit_DoTransaction(tMtiMap *tpMap)
{
   int iCr = 0, iRet = 0;
   TLV_TREE_NODE selectionTlvTree = NULL;
   T_SHARED_DATA_STRUCT* selectionSharedData = NULL;
   T_SHARED_DATA_STRUCT* kernelSharedBuffer = NULL;
   unsigned char iEnd = FALSE, iCancel = FALSE, ucTemp = 0;
   int iKernelToUse = 0;
   int iStep = 0;
   int iTemp = 0;
   int iEvent = 0;
   tMtiMap tMapTemp;
   Telium_File_t* keyboard;
   int iPrepare = FALSE;

   if (tpMap == NULL)
   {
	   return RTN_ERROR;
   }

   mtiMapInit(&tMapTemp);

   // ********** Initialize transaction parameters **************
   // Set the transaction parameters

   if (mtiGetRfEmvParmInfo()->iTranType == MTI_EMV_TRAN_VOID)
   {
	   APCLESS_ParamTrn_SetTransactionType(APCLESS_TRANSACTION_TYPE_REFUND);
   }
   else
   {
	   APCLESS_ParamTrn_SetTransactionType(APCLESS_TRANSACTION_TYPE_DEBIT);
   }

   APCLESS_ParamTrn_SetAmount((ULONG)mtiMapGetLLong(tpMap, MTI_RFEMV_KEY_AMOUNT));  //set the transaction amount 1.23 EUR

   if (mtiGetRfEmvParmInfo()->FuncInputGlobalParamCallback(&tMapTemp) == 0)
   {
	   APCLESS_ParamTrn_SetCurrencyCode(mtiMapGetBytes(&tMapTemp, MTI_EMV_TAG_INT_CURRENCY_CODE, &iTemp));

	   ucTemp = mtiMapGetBytes(&tMapTemp, MTI_EMV_TAG_TRANSACTION_CURRENCY_EXPONENT, &iTemp)[0];
	   APCLESS_ParamTrn_SetCurrencyExponent(ucTemp);
   }
   mtiMapClear(&tMapTemp);

   // Use terminal current date
   APCLESS_ParamTrn_SetDateTime();

   // Indicate the selection is explicit
   APCLESS_Selection_SetMethod(APCLESS_SELECTION_EXPLICIT);

   // Set the transaction kernel to "unknown"
   APCLESS_ParamTrn_SetCurrentPaymentScheme(APCLESS_SCHEME_UNKNOWN);

   // Indicate no tap is in progress
   APCLESS_ParamTrn_SetDoubleTapInProgress(FALSE);

   // ********** Initialize card detection & selection parameters **************
   // create selection TLV tree (for selection input parameters)
   selectionTlvTree = TlvTree_New(0);
   if (selectionTlvTree != NULL)
   {
	   // Create the selection shared buffer (for selection result)
	  selectionSharedData = GTL_SharedExchange_InitShared(10240);
	  if (selectionSharedData != NULL)
	  {
		  // Create the kernel shared buffer
		 kernelSharedBuffer = GTL_SharedExchange_InitShared(C_SHARED_KERNEL_BUFFER_SIZE);
		 if (kernelSharedBuffer != NULL)
		 {
			 // Set and load data for application explicit selection
			iCr = APCLESS_Selection_GiveInfo(&selectionTlvTree, TRUE);
			if (iCr == TRUE)
			{
				iCr = Cless_ExplicitSelection_LoadData(selectionTlvTree);
			    if (iCr == CLESS_STATUS_OK)
			    {
			       // Set the customization function to be called for GUI customization
				   g_iRetGuiCustomisation = 0;
				   iCr = Cless_ExplicitSelection_Custom_RegistrationForGui(&APCLESS_Selection_GuiCustomisation);

				   // Perform the selection pre-processing
				   iCr = Cless_ExplicitSelection_EntryPoint_TransactionPreProcessing();
				   if (iCr == CLESS_STATUS_OK)
				   {
					   iPrepare = TRUE;
				   }
			    }
			}
		 }
	  }
   }

   if (iPrepare)
   {
	   // ********** Execute transaction steps (detection, selection, kernel execution) **************
	   iStep = __APCLESS_EXPLICIT_STEP_DETECTION;
	   g_iRetGuiCustomisation = 0;
	   while(iStep < __APCLESS_EXPLICIT_STEP_END)
	   {
		  dmsg("**** Step = %d", iStep);
		  dmsg("******************* g_iRetGuiCustomisation = %d", g_iRetGuiCustomisation);
		  switch(iStep)
		  {
			 // ********** Execute card detection **************
			 case __APCLESS_EXPLICIT_STEP_DETECTION:

				// Clear the selection Shared buffer (for selection result)
				GTL_SharedExchange_ClearEx(selectionSharedData, FALSE);

				// Launch the card detection
				iCr = Cless_ExplicitSelection_GlobalCardDetection();
				if (iCr != CLESS_STATUS_OK)
				{
				   iStep = __APCLESS_EXPLICIT_STEP_END;
				   break;
				}

				// Wait contact'less event (or cancel if red key is pressed)
				iEnd = FALSE;
				iCancel = FALSE;

				keyboard = Telium_Fopen("KEYBOARD", "r");

				do
				{
				   iEvent = Telium_Ttestall(KEYBOARD | CLESS, 0);
/*
				   dmsg("******************* g_iRetGuiCustomisation = %d", g_iRetGuiCustomisation);
				   if (g_iRetGuiCustomisation == CLESS_CUST_STOP)
				   {
					   if (g_iRetGuiStep == CLESS_CUST_GUI_COLLISION_STEP_ID)
					   {
						   dpt();
						   iRet = MTI_EMV_TR_2CARD_TAP_ERR;
					   }
					   else
					   {
						   dpt();
						   iRet = MTI_EMV_TR_CARD_ERROR;
					   }
						iStep = __APCLESS_EXPLICIT_STEP_END;
						break;
				   }
*/
				   if (iEvent & CLESS)
				   {
					  iEnd = TRUE;
				   }
				   else   // event  KEYBOARD
				   {
					  if (Telium_Getc(keyboard) == T_ANN)
					  {
						 Cless_ExplicitSelection_GlobalCardDetectionCancel();
						 iCancel = TRUE;
						 iEnd = TRUE;
						 dpt();
					  }
				   }
				} while(!iEnd);

				if (keyboard != NULL)
					Telium_Fclose (keyboard);

				// Check if card detection has been canceled
				if(iCancel == TRUE)
				{
				   iStep = __APCLESS_EXPLICIT_STEP_END;
				   break;
				}

				// Get the card detection result
				iCr = Cless_Generic_CardDetectionGetResults(selectionSharedData, TRUE);
				if (iCr != CLESS_STATUS_OK)
				{
				   iStep = __APCLESS_EXPLICIT_STEP_END;
				   break;
				}

				iStep = __APCLESS_EXPLICIT_STEP_SELECTION;
				break;

			 // ********** Execute card selection **************
			 case __APCLESS_EXPLICIT_STEP_SELECTION:
				// Launch application selection
				iCr = Cless_ExplicitSelection_Selection_ApplicationSelectionProcess(selectionSharedData);
				dmsg("Cless_ExplicitSelection_Selection_ApplicationSelectionProcess() Result = %X", iCr);
				if (iCr != CLESS_STATUS_OK)
				{
				   if(iCr == CLESS_STATUS_COMMUNICATION_ERROR)
				   {  // communication error -> retry card detection & selection
					  iCr = CLESS_CR_MANAGER_RESTART;
					  iStep = __APCLESS_EXPLICIT_STEP_CHECK_RESULT;
				   }
				   else
				   {
					   dpt();
					  iStep = __APCLESS_EXPLICIT_STEP_END;
				   }
				   break;
				}

				iStep = __APCLESS_EXPLICIT_STEP_KERNEL_EXECUTION;
				break;

			 // ********** Execute a kernel **************
			 case __APCLESS_EXPLICIT_STEP_KERNEL_EXECUTION:

				// First clear the buffer to be used with the contactless kernel
				GTL_SharedExchange_ClearEx (kernelSharedBuffer, FALSE);

				// Add the generic transaction data (previously saved) in the shared buffer (date, time, amount, etc).
				if (!APCLESS_Kernel_AddTransactionGenericData(kernelSharedBuffer))
				{
				   iStep = __APCLESS_EXPLICIT_STEP_END;
				   break;
				}

				// Add AID related data to the shared buffer used with kernels
				if (!APCLESS_Kernel_AddAidRelatedData (selectionSharedData, kernelSharedBuffer, &iKernelToUse))
				{
				   iStep = __APCLESS_EXPLICIT_STEP_END;
				   break;
				}

				// Call kernel in relationship with AID
				switch (iKernelToUse)
				{
				   case DEFAULT_EP_KERNEL_PAYPASS :
					  iCr = APCLESS_PayPass_PerformTransaction(kernelSharedBuffer, tpMap);
					  break;

				   case DEFAULT_EP_KERNEL_VISA :
					  iCr = APCLESS_payWave_PerformTransaction(kernelSharedBuffer, tpMap);
					  break;
				   //@@WAY JCB Cless
				   case DEFAULT_EP_KERNEL_JCB_C5:
				   	  iCr = APCLESS_JCB_PerformTransaction(kernelSharedBuffer);
					  break;
				   //
				}

				iStep = __APCLESS_EXPLICIT_STEP_CHECK_RESULT;
				break;

			 // ********** Check (kernel) result (restart needed ?) **************
			 case __APCLESS_EXPLICIT_STEP_CHECK_RESULT:

				dmsg("__APCLESS_EXPLICIT_STEP_CHECK_RESULT() - Result = %d", iCr);
				switch(iCr)
				{
				   case CLESS_CR_MANAGER_RESTART_CHIP_ONLY:
					   iRet = MTI_EMV_TR_TRY_IC_TRAN;
					   iStep = __APCLESS_EXPLICIT_STEP_END;
					   dpt();
					   break;

				   case CLESS_CR_MANAGER_REMOVE_AID:
					  // Restart with application selection
					  APCLESS_ParamTrn_SetCurrentPaymentScheme(APCLESS_SCHEME_UNKNOWN);
					  iStep = __APCLESS_EXPLICIT_STEP_SELECTION;
					  dpt();
					  //iRet = MTI_EMV_TR_CANCEL; //@@WAY PARAM PAYPASS 20210127
					  break;

				   case CLESS_CR_MANAGER_RESTART_DOUBLE_TAP:
					   dpt();
					  // Restart with card detection
					  Cless_ExplicitSelection_DetectionPrepareForRestart(TRUE);
					  APCLESS_ParamTrn_SetCurrentPaymentScheme(APCLESS_SCHEME_UNKNOWN);
					  iStep = __APCLESS_EXPLICIT_STEP_DETECTION;
					  /**
					  dmsg("WAIT....START");
					  mtiSleep(TIMEOUT_30S);
					  dmsg("WAIT....STOP");
					  **/
						 /**
					  iRet = mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_SHOW_MESSAGE,
							  "PLS RE-TAP CARD", "TO TERMINAL");**/
					  //iRet = MTI_EMV_TR_CANCEL; //@@WAY PARAM PAYPASS 20210127
					  break;

				   case CLESS_CR_MANAGER_RESTART:
				   case CLESS_CR_MANAGER_RESTART_NO_MESSAGE_BEFORE_RETRY:
					   dpt();
					  // Restart with card detection
					  Cless_ExplicitSelection_DetectionPrepareForRestart(FALSE);
					  APCLESS_ParamTrn_SetCurrentPaymentScheme(APCLESS_SCHEME_UNKNOWN);
					  iStep = __APCLESS_EXPLICIT_STEP_DETECTION;
					  //iRet = MTI_EMV_TR_CANCEL; //@@WAY PARAM PAYPASS 20210127	  
					  break;
				   //@@WAY PARAM PAYPASS 20210127	  
				   //case CLESS_CR_MANAGER_RESTART_WO_CLESS:
				   	  //iRet = MTI_EMV_TR_TRY_INSERT_CARD;
					  //iStep = __APCLESS_EXPLICIT_STEP_END;
				   	  //break;
				   //
				   default:
					  // Other result value -> end processing
					  iStep = __APCLESS_EXPLICIT_STEP_END;
					  iRet = MTI_EMV_TR_SUCCESS;
					  break;
				}

				break;

			 // ******* Default case **********
			 default:    // iStep value unknown -> end processing
				iStep = __APCLESS_EXPLICIT_STEP_END;
				break;
		  }

		  dpt();
	   }
   }

   // Wait end of message display
   APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_WAIT_END_DISPLAY);

   // Release the TLV Tree if allocated
   if (selectionTlvTree != NULL)
   {
      TlvTree_Release(selectionTlvTree);
      selectionTlvTree = NULL;
   }

   // Release shared buffer if allocated
   if (selectionSharedData != NULL)
      GTL_SharedExchange_DestroyShare(selectionSharedData);

   if (kernelSharedBuffer != NULL)
      GTL_SharedExchange_DestroyShare(kernelSharedBuffer);

   // Clear the detection/selection resources
   Cless_ExplicitSelection_ClearGlobalData();

   // Reset APCLESS transaction parameters
   APCLESS_Selection_SetMethod(APCLESS_SELECTION_UNKNOWN);
   APCLESS_ParamTrn_SetCurrentPaymentScheme(APCLESS_SCHEME_UNKNOWN);
   APCLESS_ParamTrn_SetDoubleTapInProgress(FALSE);

   // Set the LEDs into the idle state
   APCLESS_Gui_IndicatorIdle();

   if (iCancel == TRUE)
   {
	   iRet = MTI_EMV_TR_CANCEL;
   }
   dmsg("######################## APCLESS_Explicit_DoTransaction() Result = %d", iRet);
   return iRet;
}


