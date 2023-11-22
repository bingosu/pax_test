/**
* \file
* \brief This module implements main the EMV API functionalities.
*
* \author Ingenico
* \author Copyright (c) 2012 Ingenico, 28-32, boulevard de Grenelle,\n
* 75015 Paris, France, All Rights Reserved.
*
* \author Ingenico has intellectual property rights relating to the technology embodied\n
* in this software. In particular, and without limitation, these intellectual property rights may\n
* include one or more patents.\n
* This software is distributed under licenses restricting its use, copying, distribution, and\n
* and decompilation. No part of this software may be reproduced in any form by any means\n
* without prior written authorisation of Ingenico.
**/

/////////////////////////////////////////////////////////////////
//// Includes ///////////////////////////////////////////////////

#include "sdk.h"
#include "schVar_def.h"
#include "SEC_interface.h"
#include "TlvTree.h"
#include "GTL_Assert.h"

#include "EPSTOOL_Convert.h"
#include "EPSTOOL_TlvTree.h"

#include "EmvLib_Tags.h"
#include "EMV_Status.h"
#include "EMV_ApiTags.h"
#include "EMV_Api.h"

#include "APEMV_ServicesEmv.h"
#include "APEMV_ServicesEmv_0_AppSel.h"
#include "APEMV_ServicesEmv_1_FinalSelect.h"
#include "APEMV_ServicesEmv_2_Init.h"
#include "APEMV_ServicesEmv_3_CVM.h"
#include "APEMV_ServicesEmv_4_ActionAnalysis.h"
#include "APEMV_ServicesEmv_5_OnlineProcessing.h"
#include "APEMV_ServicesEmv_6_Stop.h"
#include "apDefine.h"

//// Macros & preprocessor definitions //////////////////////////
#define C_DEFAULT_TIMEOUT_BEFORE_PIN_INPUT				30000
#define C_DEFAULT_TIMEOUT_BETWEEN_PIN_DIGIT_INPUT		10000

#define C_DEFAULT_HIDDEN_CHAR							'*'

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

static EMV_Status_t __APEMV_ServicesEmv_GetNextStep(TLV_TREE_NODE tagsFromDc, EMV_TransactionStep_t *stepToExecute);
static EMV_Status_t __APEMV_ServicesEmv_PerformTransaction(EMV_Object_t EMV_object);
static EMV_Status_t  __APEMV_ServicesEmv_StartTransaction(const char *cardReader, unsigned int cardReaderBufferSize, int doEmvAppliSelection);

//// Global variables ///////////////////////////////////////////

////  variables ///////////////////////////////////////////

static TLV_TREE_NODE __APEMV_ServicesEmv_TransacInfoTlvTree = NULL;											//!< Store the EMV transaction data (amount, date and time, ...).

//// Functions //////////////////////////////////////////////////

//! \brief Get the next transaction step to perform.
//! \param[in] tagsFromDc Transaction data that contain the TAG_EMV_INT_NEXT_STEP tag that indicates the next step to execute.
//! \param[out] stepToExecute Next step to execute.
//! \return EMV API status code. \a EMV_STATUS_SUCCESS if successful.
static EMV_Status_t __APEMV_ServicesEmv_GetNextStep(TLV_TREE_NODE tagsFromDc, EMV_TransactionStep_t *stepToExecute)
{
	EPSTOOL_Data_t dataStep;

	if(tagsFromDc != NULL)
	{
		if (EPSTOOL_TlvTree_FindFirstData(tagsFromDc, TAG_EMV_INT_NEXT_STEP, &dataStep) != NULL)
		{
			if ((dataStep.length == 1) && (dataStep.value != NULL))
				*stepToExecute = dataStep.value[0];
			else
				return EMV_STATUS_UNEXPECTED_ERROR;
		}
		else
			return EMV_STATUS_UNEXPECTED_ERROR;
	}

	return EMV_STATUS_SUCCESS;
}

int __APEMV_InputSecurePinEntry(TLV_TREE_NODE outputTlvTree, int bypass)
{
	T_SEC_ENTRYCONF stEntryConfig;
	int iRet = RTN_CANCEL, iEndEnterPIN = 0;

	stEntryConfig.ucEchoChar = C_DEFAULT_HIDDEN_CHAR;
	//stEntryConfig.ucMinDigits = 4;
	stEntryConfig.ucMinDigits = 0;
	//stEntryConfig.ucMaxDigits = 12;
	stEntryConfig.ucMaxDigits = 6;
	stEntryConfig.iFirstCharTimeOut = C_DEFAULT_TIMEOUT_BEFORE_PIN_INPUT;
	stEntryConfig.iInterCharTimeOut = C_DEFAULT_TIMEOUT_BETWEEN_PIN_DIGIT_INPUT;

	//PIN Entry Initilize
	iRet = SEC_PinEntryInit (&stEntryConfig, C_SEC_PINCODE);
	if (iRet == OK)
	{
		//PIN Entry Processing
		unsigned char ucPinOut;
		unsigned int uiEventToWait = 0;
		int iToContinue = 0;
		char caEcho[DISP_MAX_COULUM + 1];
		char *cpEcho = NULL;
		char *cpEchoSt = NULL;
		int iUpdateFlag = FALSE;
		INT iPinLength = FALSE;

		tDispContents taMenu[6];
		tDispContents *tpMenu = NULL;
		tDispContents *tpKeyEnt = NULL;

		MZERO(caEcho, sizeof(caEcho));
		MZERO(taMenu, sizeof(taMenu));
		tpMenu = taMenu;

		tpMenu->iAttribute = DISP_OPT_TITLE;
		tpMenu->iAlignment = DISP_ALIGN_CENTER;
		SCPY(tpMenu->caContents, "PIN VERIFY", DISP_MAX_COULUM); tpMenu++;

		tpMenu->iAlignment = DISP_ALIGN_CENTER;
		SCPY(tpMenu->caContents, "                ", DISP_MAX_COULUM); tpMenu++;

		tpMenu->iAlignment = DISP_ALIGN_CENTER;
		SCPY(tpMenu->caContents, "ENTER YOUR PIN", DISP_MAX_COULUM); tpMenu++;

		tpKeyEnt = tpMenu;
		tpMenu->iAlignment = DISP_ALIGN_CENTER;

		//sprintf(caEcho, " [            ] ");
		sprintf(caEcho, "    [      ]    ");
		/*siroh*/
		//cpEcho = &caEcho[2];
		cpEcho = &caEcho[5];
		cpEchoSt = cpEcho;
		SCPY(tpKeyEnt->caContents, caEcho, DISP_MAX_COULUM); tpMenu++;

		mtiShowDialogDisp(taMenu, tpMenu - taMenu, NULL);

		// Initialisation
		iToContinue = TRUE;
		iEndEnterPIN = FALSE;
		iRet = RTN_CANCEL;
		while (!iEndEnterPIN)
		{
			iUpdateFlag = FALSE;

			iRet = SEC_PinEntry(&uiEventToWait,&ucPinOut, &iToContinue);

			dmsg(" SEC_PinEntry : iRet[%d], ucPinOut[%d]", iRet, ucPinOut);
			
			if (iRet == OK)
			{
				switch (ucPinOut)
				{
					case C_DEFAULT_HIDDEN_CHAR:
						*cpEchoSt++ = C_DEFAULT_HIDDEN_CHAR;
						iUpdateFlag = TRUE;
						break;

					case T_VAL:
						iPinLength = cpEchoSt - cpEcho;
						dmsg("**** iPinLength = %d", iPinLength);
						iEndEnterPIN = TRUE;

						if (!iPinLength)
						{
							if (bypass)
							{
								iRet = RTN_SKIP;
							}
							else
							{
								iRet = RTN_CANCEL;

								iRet = SEC_PinEntryInit (&stEntryConfig, C_SEC_PINCODE);
								if (iRet != OK)
								{
									iEndEnterPIN = TRUE;
									break;
								}
								iToContinue = TRUE;
								iPinLength = 0;
								iUpdateFlag = TRUE;
								iEndEnterPIN = FALSE;
							}
						}
						else
						{
							TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_PIN_ENTRY_LENGTH, &iPinLength, 1);
							iRet = RTN_SUCCESS;
						}
						break;

					case T_ANN:
						iEndEnterPIN = TRUE;
						iRet = RTN_CANCEL;
						break;

					case T_CORR:
						if (cpEchoSt - cpEcho > 0)
						{
							*--cpEchoSt = ' ';
							iUpdateFlag = TRUE;
						}
						break;

					default:
						break;
				}

				if (iUpdateFlag)
				{
					SCPY(tpKeyEnt->caContents, caEcho, DISP_MAX_COULUM);
					mtiShowDialogDisp(taMenu, tpMenu - taMenu, NULL);
				}
			}
			else
			{
				iEndEnterPIN = TRUE;
			}

			dmsg("END iEndEnterPIN [%d]", iEndEnterPIN);
			
		}
	}
	else
	{
		iRet = RTN_CANCEL;
	}

	dmsg("SEC_PinEntryInit() Result = %d", iRet);
	
	return iRet;
}

static int __APEMV_GetTrasactionTag(EMV_Object_t EMV_object)
{
	static const EPSTOOL_Tag_t tagsToGet[] = {
			TAG_EMV_TRACK_2_EQU_DATA,
			TAG_EMV_CRYPTOGRAM_INFO_DATA,
			TAG_EMV_APPLICATION_CRYPTOGRAM,
			TAG_EMV_TVR,
			TAG_EMV_TSI,	// @@EB PRINT ENHANCEMENT	// @@EB TSI
			TAG_EMV_CVM_RESULTS,
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

			TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER,
			TAG_EMV_APPLICATION_LABEL,
			TAG_EMV_APPLI_PREFERRED_NAME,
			TAG_EMV_CARDHOLDER_NAME,
			TAG_EMV_APPLI_PAN,
			TAG_EMV_APPLICATION_LABEL,
			TAG_EMV_APPLI_VERSION_NUMBER_TERM,
	};	//16.11.11 Requirment Roh.

	TLV_TREE_NODE tlvTree;
	TLV_TREE_NODE hNode;
	tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;

	// Retrieve the PAN and the PAN Sequence Counter
	EMV_GetTags(EMV_object, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, &tlvTree);
	if(tlvTree != NULL)
	{
		hNode = TlvTree_GetFirstChild(tlvTree);

		/***
		dmsg("================================================================================================");
		dmsg("================================================================================================");
		***/
		while(hNode != NULL)
		{
			/***
			dmsg("TAG [%X]", TlvTree_GetTag(hNode));
			dbuf("VALUE", TlvTree_GetData(hNode), TlvTree_GetLength(hNode));
			***/
			mtiMapPutBytes(tpMap, TlvTree_GetTag(hNode), TlvTree_GetData(hNode), TlvTree_GetLength(hNode));
			// Next tag
			hNode = TlvTree_GetNext(hNode);
		}
		/***
		dmsg("================================================================================================");
		dmsg ("================================================================================================");
		***/

		// Free the memory
		EPSTOOL_TlvTree_Release(&tlvTree);
		return EMV_STATUS_SUCCESS;
	}

	return EMV_STATUS_UNEXPECTED_ERROR;
}

static int __APEMV_GetCryptogramInfomationData(EMV_Object_t EMV_object)
{
	static const EPSTOOL_Tag_t tagsToGet[] = {
			TAG_EMV_APPLICATION_CRYPTOGRAM,
			TAG_EMV_CRYPTOGRAM_INFO_DATA,
	};	//16.11.11 Requirment Roh.

	TLV_TREE_NODE tlvTree;
	TLV_TREE_NODE hNode;
	tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;
	UCHAR *ucpTemp = NULL;
	INT iLen = 0;

	// Retrieve the PAN and the PAN Sequence Counter
	EMV_GetTags(EMV_object, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, &tlvTree);
	if(tlvTree != NULL)
	{
		hNode = TlvTree_GetFirstChild(tlvTree);

		dmsg("================================================================================================");
		dmsg("================================================================================================");
		while(hNode != NULL)
		{
			dmsg("TAG [%X]", TlvTree_GetTag(hNode));
			dbuf("VALUE", TlvTree_GetData(hNode), TlvTree_GetLength(hNode));

			mtiMapPutBytes(tpMap, TlvTree_GetTag(hNode), TlvTree_GetData(hNode), TlvTree_GetLength(hNode));
			// Next tag
			hNode = TlvTree_GetNext(hNode);
		}
		dmsg("================================================================================================");
		dmsg ("================================================================================================");


		// Free the memory
		EPSTOOL_TlvTree_Release(&tlvTree);

		ucpTemp = mtiMapGetBytes(tpMap, TAG_EMV_CRYPTOGRAM_INFO_DATA, &iLen);
		if (iLen > 0)
		{
			dmsg("TAG_EMV_CRYPTOGRAM_INFO_DATA [%X]", ucpTemp[0]);
			if ((ucpTemp[0] & 0xF0) == 0)
			{
				return EMV_STATUS_2NDGENAC_RESULT_AAC;
			}
			else
			{
				return EMV_STATUS_SUCCESS;
			}
		}
	}

	return EMV_STATUS_UNEXPECTED_ERROR;
}


//! \brief Perform an EMV transaction by calling EMV API from Final Select to the end of the transaction.
//! \param[in] EMV_object The EMV transaction object.
//! \return EMV API status code. \a EMV_STATUS_SUCCESS if successful.
static EMV_Status_t __APEMV_ServicesEmv_PerformTransaction(EMV_Object_t EMV_object)
{
	EMV_Status_t emvStatus;
	TLV_TREE_NODE inputTlvTree;
	TLV_TREE_NODE tagsFromDc;
	TLV_TREE_NODE tagsToKernel;
	EMV_TransactionStep_t stepToExecute;
	int endTransaction;

	emvStatus = EMV_STATUS_UNKNOWN;
	endTransaction = FALSE;
	stepToExecute = EMV_STEP_INIT;

	// Initialise a tlvtree that will contain the input data
	inputTlvTree = TlvTree_New(0);
	if (inputTlvTree == NULL)
	{
		ASSERT(FALSE);
		return EMV_STATUS_NOT_ENOUGH_MEMORY;
	}

	// Call EMV API to perform each steps of the transaction
	do
	{
		dmsg("** stepToExecute = %d", stepToExecute);
		switch (stepToExecute)
		{
			case EMV_STEP_INIT:
				emvStatus = APEMV_ServicesEmv_Init(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_FINAL_SELECT:
				emvStatus = APEMV_ServicesEmv_FinalSelect(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_INITIATE_APPLICATION_PROCESSING:
				emvStatus = APEMV_ServicesEmv_InitiateApplicationSelection(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_READ_APPLICATION_DATA:
				emvStatus = APEMV_ServicesEmv_ReadRecords(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_OFFLINE_DATA_AUTHENTICATION:
				emvStatus = APEMV_ServicesEmv_OfflineDataAuthentication(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_PROCESSING_RESTRICTION:
				emvStatus = APEMV_ServicesEmv_ProcessingRestriction(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_CARDHOLDER_VERIFICATION:
				emvStatus = APEMV_ServicesEmv_CardholderVerification(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_TERMINAL_RISK_MANAGEMENT:
				emvStatus = APEMV_ServicesEmv_TerminalRiskManagement(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_TERMINAL_ACTION_ANALYSIS:
				emvStatus = APEMV_ServicesEmv_TerminalActionAnalysis(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_CARD_ACTION_ANALYSIS:
				emvStatus = APEMV_ServicesEmv_CardActionAnalysis(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				dmsg("** EMV_STEP_CARD_ACTION_ANALYSIS - emvStatus [%d]", emvStatus);
				dpt();
				if (emvStatus == EMV_STATUS_OFFLINE_AUTHED)
				{
					if (__APEMV_GetTrasactionTag(EMV_object) == EMV_STATUS_SUCCESS)
					{
						TLV_TREE_NODE inputAuthorisation;

						dpt();
						// inputAuthorisation usually contains the data for authorisation message.
						inputAuthorisation = TlvTree_New(0);
						if (inputAuthorisation == NULL)
						{
							ASSERT(FALSE);
							return EMV_STATUS_NOT_ENOUGH_MEMORY;
						}

						// Perform online authorisation. inputTlvTree is filled with TAG_EMV_INT_AUTHORISATION_RESULT tag.
						if (APEMV_ServicesEmv_Authorisation(inputAuthorisation, inputTlvTree) == EMV_STATUS_CANCEL)
						{
							EPSTOOL_TlvTree_Release(&inputAuthorisation);
							return EMV_STATUS_CANCEL;
						}

						dpt();
						endTransaction = TRUE;
						emvStatus = EMV_STATUS_SUCCESS;

						EPSTOOL_TlvTree_Release(&inputAuthorisation);
					}
				}
				break;
			case EMV_STEP_ONLINE_PROCESSING:
				if (__APEMV_GetTrasactionTag(EMV_object) == EMV_STATUS_SUCCESS)
				{
					emvStatus = APEMV_ServicesEmv_OnlineProcessing(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				}
				else
				{
					endTransaction = TRUE;
				}
				break;
			case EMV_STEP_ISSUER_TO_CARD_SCRIPTS_PROCESSING1:
				emvStatus = APEMV_ServicesEmv_ISP1(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_COMPLETION:
				emvStatus = APEMV_ServicesEmv_Completion(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				if (emvStatus == EMV_STATUS_SUCCESS)
				{
					emvStatus = __APEMV_GetCryptogramInfomationData(EMV_object);
				}
				break;
			case EMV_STEP_ISSUER_TO_CARD_SCRIPTS_PROCESSING2:
				emvStatus = APEMV_ServicesEmv_ISP2(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
				break;
			case EMV_STEP_END:
				if (__APEMV_GetTrasactionTag(EMV_object) == EMV_STATUS_SUCCESS)
				{
					emvStatus = APEMV_ServicesEmv_End(EMV_object, inputTlvTree, &tagsFromDc, &tagsToKernel);
					if (emvStatus == EMV_STATUS_SUCCESS)
					{
						tMtiMap *tpMap = &mtiGetEmvParmInfo()->tEmvTagMap;
						UCHAR *ucpAC = NULL;
						INT iACLen = 0;

						ucpAC = mtiMapGetBytes(tpMap, TAG_EMV_CRYPTOGRAM_INFO_DATA, &iACLen);
						if (ucpAC != NULL)
						{
							if ((ucpAC[0] & 0xF0) == 0)
							{
								return EMV_STATUS_2NDGENAC_RESULT_AAC;
							}
							else
							{
								return EMV_STATUS_SUCCESS;
							}
						}
					}
				}
				else
				{
					endTransaction = TRUE;
				}
				break;
			default:
				endTransaction = TRUE;
				break;
		}
		dmsg("** emvStatus = %d", emvStatus);

		if((emvStatus == EMV_STATUS_SUCCESS) && (!endTransaction))
		{
			// Release the input data
			EPSTOOL_TlvTree_Release(&inputTlvTree);
			// Create a new tlvtree for the next call
			inputTlvTree = TlvTree_New(0);
			if (inputTlvTree == NULL)
			{
				ASSERT(FALSE);
				EPSTOOL_TlvTree_Release(&tagsFromDc);
				EPSTOOL_TlvTree_Release(&tagsToKernel);
				return EMV_STATUS_NOT_ENOUGH_MEMORY;
			}

			// Fill input data for the next call with some data returned by the kernel
			if(stepToExecute == EMV_STEP_READ_APPLICATION_DATA)
			{
				// Perform the Read Records post processing
				APEMV_ServicesEmv_StepReadApplicationData(tagsFromDc, inputTlvTree);
			}
			else if(stepToExecute == EMV_STEP_CARDHOLDER_VERIFICATION)
			{
				// Perform the Cardholder Verification post processing
				emvStatus = APEMV_ServicesEmv_StepCardholderVerification(tagsFromDc, inputTlvTree);
				if (emvStatus == EMV_STATUS_CANCEL)
				{
					break;
				}
			}

			// Get the next step to perform
			emvStatus = __APEMV_ServicesEmv_GetNextStep(tagsFromDc, &stepToExecute);
			dmsg("__APEMV_ServicesEmv_GetNextStep() Result = %d", emvStatus);
		}

		// Release output data
		EPSTOOL_TlvTree_Release(&tagsFromDc);
		EPSTOOL_TlvTree_Release(&tagsToKernel);

		if (emvStatus == EMV_STATUS_CANCEL || emvStatus == EMV_STATUS_ONLINE_DECLINED)
		{
			break;
		}
	}
	while((emvStatus == EMV_STATUS_SUCCESS) && (!endTransaction));

	// Release all the allocated tlvtree
	EPSTOOL_TlvTree_Release(&inputTlvTree);
	EPSTOOL_TlvTree_Release(&tagsFromDc);
	EPSTOOL_TlvTree_Release(&tagsToKernel);

//TestOut:
	dmsg("__APEMV_ServicesEmv_PerformTransaction() Result = %d ", emvStatus);
	return emvStatus;
}

//! \brief Start an EMV transaction by calling EMV API.
//! \param[in] cardReader The card reader to use.
//! \param[in] cardReaderBufferSize Size of the cardReader buffer.
//! \param[in] doEmvAppliSelection \a TRUE if the application selection must be done (transaction not started by the Telium Manager),
//! or \a FALSE if the application selection has already been done (transaction is started by the Telium Manager, usually called from \a GIVE_AID).
//! \return EMV API status code. \a EMV_STATUS_SUCCESS if successful.
static EMV_Status_t __APEMV_ServicesEmv_StartTransaction(const char *cardReader, unsigned int cardReaderBufferSize, int doEmvAppliSelection)
{
	EMV_Status_t emvStatus;
	EMVAS_Object_t AS_object;
	EMV_Object_t EMV_object;

	emvStatus = EMV_STATUS_SUCCESS;

	if(doEmvAppliSelection)
	{
		// Create the application selection object
		emvStatus = EMVAS_Create(&AS_object, cardReader, cardReaderBufferSize);
		dmsg("EMVAS_Create() Result = %d", emvStatus);

		// Perform application selection
		if(emvStatus == EMV_STATUS_SUCCESS)
		{
			emvStatus = APEMV_ServicesEmv_ApplicationSelection(AS_object);
			dmsg("APEMV_ServicesEmv_ApplicationSelection() Result = %d", emvStatus);

			// If the apllication selection has not been performed by the manager, ask the cardholder to select the AID to use.
			// Add the AID to select in the transaction input data
			if (emvStatus == EMV_STATUS_SUCCESS)
			{
				emvStatus = APEMV_ServicesEmv_MenuSelect(AS_object, __APEMV_ServicesEmv_TransacInfoTlvTree);
			}
		}
	}

	if(emvStatus == EMV_STATUS_SUCCESS)
	{
		// Create the EMV transaction object
		if(EMV_Create(&EMV_object, cardReader, cardReaderBufferSize) == EMV_STATUS_SUCCESS)
		{
			// Perform the transaction
			emvStatus = __APEMV_ServicesEmv_PerformTransaction(EMV_object);

			// Destroy the EMV transaction object
			VERIFY(EMV_Destroy(EMV_object) == EMV_STATUS_SUCCESS);
		}
	}

	// Destroy the application selection object
	if(doEmvAppliSelection)
		VERIFY(EMVAS_Destroy(AS_object) == EMV_STATUS_SUCCESS);

	dmsg("__APEMV_ServicesEmv_StartTransaction() Result = %d ", emvStatus);
	return emvStatus;
}


//! \brief Perform an EMV transaction by calling EMV API.
//! \param[in] cardReader The card reader to use.
//! \param[in] cardReaderBufferSize Size of the cardReader buffer.
//! \param[in] doEmvAppliSelection \a TRUE if the application selection must be done (transaction not started by the Telium Manager),
//! or \a FALSE if the application selection has already been done (transaction is started by the Telium Manager, usually called from \a GIVE_AID).
//! \param[out] inputTlvTree Input TlvTree. Usually contains transaction data (amount, date and time, ...).
//! \return EMV API status code. \a EMV_STATUS_SUCCESS if successful.
//! \remarks If date and time is not set in \a inputTlvTree, the current date and time is automatically used.
EMV_Status_t APEMV_ServicesEmv_DoTransaction(const char *cardReader, unsigned int cardReaderBufferSize, int doEmvAppliSelection, TLV_TREE_NODE inputTlvTree)
{
	EMV_Status_t emvStatus;
	Telium_Date_t currentDate;
	int dateMissing;
	int timeMissing;
	unsigned char buffer[3];

	ASSERT(inputTlvTree != NULL);

	EPSTOOL_TlvTree_Release(&__APEMV_ServicesEmv_TransacInfoTlvTree);
	__APEMV_ServicesEmv_TransacInfoTlvTree = TlvTree_New(0);
	if (__APEMV_ServicesEmv_TransacInfoTlvTree == NULL)
	{
		ASSERT(FALSE);
		return EMV_STATUS_NOT_ENOUGH_MEMORY;
	}

	// Add date and time if they are missing
	dateMissing = (EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_TRANSACTION_DATE, NULL) == NULL);
	timeMissing = (EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_TRANSACTION_TIME, NULL) == NULL);
	if ((dateMissing) || (timeMissing))
	{
		// Get the current date and time
		if (Telium_Read_date(&currentDate) == 0)
		{
			// Add the date if it is missing
			if (dateMissing)
			{
				buffer[0] = ((currentDate.year[0] - '0') << 4) | (currentDate.year[1] - '0');
				buffer[1] = ((currentDate.month[0] - '0') << 4) | (currentDate.month[1] - '0');
				buffer[2] = ((currentDate.day[0] - '0') << 4) | (currentDate.day[1] - '0');
				if (TlvTree_AddChild(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_TRANSACTION_DATE, buffer, 3) == NULL)
				{
					ASSERT(FALSE);
					EPSTOOL_TlvTree_Release(&__APEMV_ServicesEmv_TransacInfoTlvTree);
					return EMV_STATUS_NOT_ENOUGH_MEMORY;
				}
			}

			// Add the time if it is missing
			if (timeMissing)
			{
				buffer[0] = ((currentDate.hour[0] - '0') << 4) | (currentDate.hour[1] - '0');
				buffer[1] = ((currentDate.minute[0] - '0') << 4) | (currentDate.minute[1] - '0');
				buffer[2] = ((currentDate.second[0] - '0') << 4) | (currentDate.second[1] - '0');
				if (TlvTree_AddChild(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_TRANSACTION_TIME, buffer, 3) == NULL)
				{
					ASSERT(FALSE);
					EPSTOOL_TlvTree_Release(&__APEMV_ServicesEmv_TransacInfoTlvTree);
					return EMV_STATUS_NOT_ENOUGH_MEMORY;
				}
			}
		}
	}

	// Store the transaction input data
	if (!EPSTOOL_TlvTree_CopyChildren(__APEMV_ServicesEmv_TransacInfoTlvTree, inputTlvTree))
	{
		EPSTOOL_TlvTree_Release(&__APEMV_ServicesEmv_TransacInfoTlvTree);
		ASSERT(FALSE);
		return EMV_STATUS_NOT_ENOUGH_MEMORY;
	}

	// TODO: Reset globals required for the transaction

	// Start the transaction
	emvStatus = __APEMV_ServicesEmv_StartTransaction(cardReader, cardReaderBufferSize, doEmvAppliSelection);

	// Reset globals
	EPSTOOL_TlvTree_Release(&__APEMV_ServicesEmv_TransacInfoTlvTree);

	dmsg("APEMV_ServicesEmv_DoTransaction() Result = %d ", emvStatus);
	return emvStatus;
}

//! \brief Retrieve the transaction data (amount, date and time, ...).
//! \param[out] outputTlvTree TlvTree that will be filled with the transaction data.
//! \return \a TRUE if successful, \a FALSE if an error occurs (usually because not enough memory).
int APEMV_ServicesEmv_GetTransactionData(TLV_TREE_NODE outputTlvTree)
{
	ASSERT(outputTlvTree != NULL);
	ASSERT(__APEMV_ServicesEmv_TransacInfoTlvTree != NULL);

	// Get the transaction data
	return EPSTOOL_TlvTree_CopyChildren(outputTlvTree, __APEMV_ServicesEmv_TransacInfoTlvTree);
}

//! \brief Determine if the amount is known or not.
//! \return \a TRUE if the amount is known, \a FALSE if not.
int APEMV_ServicesEmv_AmountIsSet(void)
{
	TLV_TREE_NODE nodeAmountBin;
	TLV_TREE_NODE nodeAmountNum;
	EPSTOOL_Data_t dataAmountBin;
	EPSTOOL_Data_t dataAmountNum;

	ASSERT(__APEMV_ServicesEmv_TransacInfoTlvTree != NULL);

	nodeAmountBin = EPSTOOL_TlvTree_FindFirstData(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_AMOUNT_AUTH_BIN, &dataAmountBin);
	nodeAmountNum = EPSTOOL_TlvTree_FindFirstData(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_AMOUNT_AUTH_NUM, &dataAmountNum);
	if (((nodeAmountBin != NULL) && (dataAmountBin.length > 0))
		|| ((nodeAmountNum != NULL) && (dataAmountNum.length > 0)))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//! \brief Set the transaction amount.
//! \param[in] amount The amount of the transaction.
//! \param[out] outputTlvTree A TlvTree that will contain the amounts tags (binary and numeric).
//! \return \a TRUE if the amount is set, \a FALSE if an error occurs.
int APEMV_ServicesEmv_AmountSet(unsigned long long amount, TLV_TREE_NODE outputTlvTree)
{
	unsigned char amountBinary[4];
	unsigned char amountNumeric[6];
	unsigned char amountOtherBinary[4];
	unsigned char amountOtherNumeric[6];
	//unsigned char tranCurrentCode[2];

	memset(amountBinary, 0, sizeof(amountBinary));
	memset(amountNumeric, 0, sizeof(amountNumeric));
	memset(amountOtherBinary, 0, sizeof(amountOtherBinary));
	memset(amountOtherNumeric, 0, sizeof(amountOtherNumeric));

	ASSERT(__APEMV_ServicesEmv_TransacInfoTlvTree != NULL);
/****
	if (TlvTree_AddChild(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_TRANSACTION_CURRENCY_CODE, tranCurrentCode, sizeof(tranCurrentCode)) == NULL)
		return FALSE;

	if (outputTlvTree != NULL)
	{
		if (TlvTree_AddChild(outputTlvTree, TAG_EMV_TRANSACTION_CURRENCY_CODE, tranCurrentCode, sizeof(tranCurrentCode)) == NULL)
			return FALSE;
	}
****/
	if (amount <= 0xffffffff)
	{
		// Convert the amount into a 'EMV binary' number
		EPSTOOL_Convert_ULongToEmvBin((unsigned long)amount, amountBinary);

		if (TlvTree_AddChild(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_AMOUNT_AUTH_BIN, amountBinary, sizeof(amountBinary)) == NULL)
			return FALSE;

		if (outputTlvTree != NULL)
		{
			if (TlvTree_AddChild(outputTlvTree, TAG_EMV_AMOUNT_AUTH_BIN, amountBinary, sizeof(amountBinary)) == NULL)
				return FALSE;
		}

		if (TlvTree_AddChild(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_AMOUNT_OTHER_BIN, amountOtherBinary, sizeof(amountOtherBinary)) == NULL)
			return FALSE;

		if (outputTlvTree != NULL)
		{
			if (TlvTree_AddChild(outputTlvTree, TAG_EMV_AMOUNT_OTHER_BIN, amountOtherBinary, sizeof(amountOtherBinary)) == NULL)
				return FALSE;
		}
	}

	if (amount <= 999999999999ULL)
	{
		// Convert the amount into a 'EMV numeric' number
		EPSTOOL_Convert_UllToDcbNumber(amount, amountNumeric, 6);

		if (TlvTree_AddChild(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_AMOUNT_AUTH_NUM, amountNumeric, sizeof(amountNumeric)) == NULL)
			return FALSE;
		if (outputTlvTree != NULL)
		{
			if (TlvTree_AddChild(outputTlvTree, TAG_EMV_AMOUNT_AUTH_NUM, amountNumeric, sizeof(amountNumeric)) == NULL)
				return FALSE;
		}

		if (TlvTree_AddChild(__APEMV_ServicesEmv_TransacInfoTlvTree, TAG_EMV_AMOUNT_OTHER_NUM, amountOtherNumeric, sizeof(amountOtherNumeric)) == NULL)
			return FALSE;
		if (outputTlvTree != NULL)
		{
			if (TlvTree_AddChild(outputTlvTree, TAG_EMV_AMOUNT_OTHER_NUM, amountOtherNumeric, sizeof(amountOtherNumeric)) == NULL)
				return FALSE;
		}
	}
	else
	{
		// Amount is too big
		return FALSE;
	}

	return TRUE;
}

//! \brief Retrieves the EMV global parameters.
//! \param[out] outputTlvTree Output TlvTree that must be filled with the EMV global parameters.
void APEMV_ServicesEmv_GetGlobalParam(TLV_TREE_NODE outputTlvTree)
{
	static const unsigned char ucaTermOverpassAIP[1]   = { 0x01 };
	static const unsigned char ucaSupportedLanguages[] =  "en";

	tMtiMap tMapParam;
	tMtiMap *tpMapParam = &tMapParam;
	int i = 0, iKey = 0, iLen = 0;
	unsigned char *ucpValue = NULL;

	ASSERT(outputTlvTree != NULL);

	mtiMapInit(tpMapParam);
	if (mtiGetEmvParmInfo()->FuncInputGlobalParamCallback(tpMapParam))
	{
		for (i = 0; i < tpMapParam->iMapCount; i++)
		{
			iKey = MemCheck(tpMapParam, i);
			iLen = 0;
			ucpValue = mtiMapGetBytes(tpMapParam, iKey, &iLen);

			if (iLen > 0)
			{
				/***
				dmsg("TAG [%X]", iKey);
				dbuf("VALUE", ucpValue, iLen);
				***/
				VERIFY(TlvTree_AddChild(outputTlvTree, iKey, ucpValue, iLen) != NULL);
			}
		}
	}

	mtiMapClear(tpMapParam);

	//Get the ICS data
	VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_OPTION_TRM_IRRESPECTIVE_OF_AIP, ucaTermOverpassAIP, sizeof(ucaTermOverpassAIP)) != NULL);
	// Get the supported language list
	VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_APPLICATION_LANGUAGES, ucaSupportedLanguages, sizeof(ucaSupportedLanguages) - 1) != NULL);
}
