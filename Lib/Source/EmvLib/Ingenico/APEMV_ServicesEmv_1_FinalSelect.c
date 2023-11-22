/**
* \file
* \brief This module implements the EMV API functionalities related to the Final Selection.
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
#include "SEC_interface.h"
#include "GL_GraphicLib.h"
#include "TlvTree.h"
#include "GTL_Assert.h"

#include "EPSTOOL_Convert.h"
#include "EPSTOOL_TlvTree.h"
#include "EPSTOOL_PinEntry.h"

#include "EmvLib_Tags.h"
#include "EMV_Status.h"
#include "EMV_ApiTags.h"
#include "EMV_Api.h"

#include "APEMV_ServicesEmv.h"
#include "APEMV_ServicesEmv_0_AppSel.h"
#include "APEMV_ServicesEmv_1_FinalSelect.h"


//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

static void __APEMV_ServicesEmv_GetAidData(TLV_TREE_NODE outputTlvTree);
static void __APEMV_ServicesEmv_MenuChooseLanguage(TLV_TREE_NODE inputTlvTree);

//// Global variables ///////////////////////////////////////////

////  variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////


//! \brief Retrieves the parameters linked with an AID.
//! \param[out] outputTlvTree Output TlvTree that must be filled with the AID parameters.
static void __APEMV_ServicesEmv_GetAidData(TLV_TREE_NODE outputTlvTree)
{
	tMtiMap tMapAid;
	tMtiMap *tpMapAid = &tMapAid;
	TLV_TREE_NODE hNode = NULL;
	int iRet = 0, i = 0, iKey = 0, iLen = 0;
	unsigned char *ucpValue = NULL;

	ASSERT(outputTlvTree != NULL);

	mtiMapInit(tpMapAid);

	hNode = TlvTree_GetFirstChild(outputTlvTree);
	while(hNode != NULL)
	{
		if (TlvTree_GetTag(hNode) == TAG_EMV_AID_TERMINAL)
		{
			mtiMapPutBytes(tpMapAid, TlvTree_GetTag(hNode), TlvTree_GetData(hNode), TlvTree_GetLength(hNode));

			iRet = mtiGetEmvParmInfo()->FuncInputAIDParamCallback(tpMapAid);
			mtiMapRemove(tpMapAid, TlvTree_GetTag(hNode));
			if (iRet)
			{
				for (i = 0; i < tpMapAid->iMapCount; i++)
				{
					iKey = MemCheck(tpMapAid, i);
					iLen = 0;
					ucpValue = mtiMapGetBytes(tpMapAid, iKey, &iLen);

					if (iLen > 0)
					{
						VERIFY(TlvTree_AddChild(outputTlvTree, iKey, ucpValue, iLen) != NULL);
					}
				}
			}
			break;
		}

		// Next TAG
		hNode = TlvTree_GetNext(hNode);
	}

	mtiMapClear(tpMapAid);
}


//! \brief Called when the cardholder must select the language to use.
//! \param[in] inputTlvTree Input TlvTree that contains the TAG_EMV_INT_CARDHOLDER_LANGUAGES tag.
static void __APEMV_ServicesEmv_MenuChooseLanguage(TLV_TREE_NODE inputTlvTree)
{
	// TODO Select the language to use
}

//! \brief Called after the whole application select process.
//! \param[out] outputTlvTree Output TlvTree.
EMV_Status_t APEMV_ServicesEmv_GetAmount(TLV_TREE_NODE outputTlvTree)
{
	EMV_Status_t emvStatus;
	//@@WAY FIXING AMOUNT TAG 9F02	
#if 1
	unsigned long long amount;
#else
	unsigned long amount;
#endif

	char amountString[32 + 1];

	emvStatus = EMV_STATUS_SUCCESS;

	// Is the amount known?
	if (!APEMV_ServicesEmv_AmountIsSet())
	{
		memset(amountString, 0, sizeof(amountString));

		// Neither Amount Binary nor Amount Numeric is set
		// => Ask for the amount
		if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_AMOUNT, amountString, NULL) == MTI_EMV_RTN_CONTINUE)
		{
			//@@WAY FIXING AMOUNT TAG 9F02	
#if 1
			amount = mtiAtoll(mtiStrlen(sizeof(amountString), amountString), (UCHAR *) amountString);
#else
			EPSTOOL_Convert_AsciiToUl(amountString, -1, &amount);
#endif
			if (!APEMV_ServicesEmv_AmountSet(amount, outputTlvTree))
			{
				// Cannot set the amount (not enough memory)
				emvStatus = EMV_STATUS_NOT_ENOUGH_MEMORY;
			}
		}
		else
		{
			// The amount entry has been cancelled
			emvStatus = EMV_STATUS_CANCEL;
		}
	}
	return emvStatus;
}

// Account Type Selection
#define  ACCOUNT_TYPE_DEFAULT						0x00 	//!< Default account type
#define  ACCOUNT_TYPE_SAVINGS						0x10	//!< Saving account
#define  ACCOUNT_TYPE_CHEQUE_DEBIT					0x20	//!< Check or debit account
#define  ACCOUNT_TYPE_CREDIT						0x30	//!< Credit account

//! \brief Called when the cardholder must select the account to use.
//! \param[out] outputTlvTree Output TlvTree that must contain the selected account type in TAG_EMV_ACCOUNT_TYPE.
EMV_Status_t APEMV_ServicesEmv_MenuAccountType(TLV_TREE_NODE outputTlvTree)
{
	unsigned char accountType;
	int ivalue = 0;
	EMV_Status_t emvStatus;

	if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_SELECT_TRAN_TYPE, &ivalue, NULL) == MTI_EMV_RTN_CONTINUE)
	{
		if (ivalue == MTI_EMV_TRAN_SALE)
			accountType = ACCOUNT_TYPE_CREDIT;
		else
			accountType = ACCOUNT_TYPE_DEFAULT;

		VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_ACCOUNT_TYPE, &accountType, sizeof(accountType)) != NULL);
		emvStatus = EMV_STATUS_SUCCESS;
	}
	else
		emvStatus = EMV_STATUS_CANCEL;

	return emvStatus;
}

//! \brief Execute the final select step of an EMV transaction.
//! \param[in] EMV_object The EMV transaction object to use.
//! \param[in] inputTlvTree The input transaction data.
//! \param[out] tagsKernelToAppli If not \a NULL, the function creates a new TlvTree object and fill it
//! with the tags and values requested by the application.
//! If no tag is requested by the application or if the EMV kernel does not know any value of the requested tag,
//!  no TlvTree is created (so value is set to \a NULL).
//! \param[out] tagsRequestedByKernel If not \a NULL, the function creates a new TlvTree object and fill it
//! with the tags (with empty values) that the application shall provide to the next transaction step.
//! If no tag is requested by the EMV kernel, no TlvTree is created (so value is set to \a NULL).
//! \return  Can return any value of EMV_Status_t.
//! note The application is in charge to destroy the created TlvTree \a tagsKernelToAppli and \a tagsRequestedByKernel.
EMV_Status_t APEMV_ServicesEmv_FinalSelect(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
			TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	EPSTOOL_Data_t dataStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_INT_CARDHOLDER_LANGUAGES, TAG_EMV_INT_AID_STATUS, TAG_EMV_APPLI_PAN };

	emvStatus = EMV_STATUS_UNKNOWN;

	// Get the EMV global data (terminal type, ...)
	APEMV_ServicesEmv_GetGlobalParam(inputTlvTree);

	// Get the transaction data (amount, date, ...)
	if(APEMV_ServicesEmv_GetTransactionData(inputTlvTree))
	{
		// Get the parameters linked with an AID
		__APEMV_ServicesEmv_GetAidData(inputTlvTree);

		// Call the EMV API to perform final selection
		emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_FINAL_SELECT, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

		dmsg("EMV_STEP_FINAL_SELECT = %d", emvStatus);
		if(emvStatus == EMV_STATUS_SUCCESS)
			__APEMV_ServicesEmv_MenuChooseLanguage(*tagsKernelToAppli);
		else
		{
			if (EPSTOOL_TlvTree_FindFirstData(*tagsKernelToAppli, TAG_EMV_INT_AID_STATUS, &dataStatus) != NULL)
			{
				if (dataStatus.length > 0)
				{
					dbuf("TAG_EMV_INT_AID_STATUS", dataStatus.value, dataStatus.length);
				}
			}
		}
	}
	else
	{
		emvStatus = EMV_STATUS_UNEXPECTED_ERROR;
	}


	return emvStatus;
}
