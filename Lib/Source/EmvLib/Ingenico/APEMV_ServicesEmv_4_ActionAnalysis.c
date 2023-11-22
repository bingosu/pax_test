/**
* \file
* \brief This module implements the EMV API functionalities related to action analysis
* (Terminal Risk Management, Terminal Action Analysis and Card Action Analysis).
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
#include "APEMV_ServicesEmv_4_ActionAnalysis.h"

//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

static void __APEMV_ServicesEmv_GetLastTransaction(EMV_Object_t EMV_object, TLV_TREE_NODE outputTlvTree);

//// Global variables ///////////////////////////////////////////

////  variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

//! \brief Retrieve the last transaction performed with the same card.
//! \param[in] EMV_object The EMV transaction object to use.
//! \param[out] outputTlvTree Output TlvTree. If a transaction is found, it must be filled with TAG_EMV_INT_LAST_TRANSACTION_AMOUNT.
static void __APEMV_ServicesEmv_GetLastTransaction(EMV_Object_t EMV_object, TLV_TREE_NODE outputTlvTree)
{
	//EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_APPLI_PAN, TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER };
	TLV_TREE_NODE tlvTree;
	//TLV_TREE_NODE nodePan;
	//TLV_TREE_NODE nodePanSeqNumber;
	//EPSTOOL_Data_t dataPan;
	//EPSTOOL_Data_t dataPanSeqNumber;

	// To avoid warnings because 'outputTlvTree' is not used
	(void)outputTlvTree;

	// Retrieve the PAN and the PAN Sequence Counter
	/*emvStatus =*/ EMV_GetTags(EMV_object, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, &tlvTree);
	if(tlvTree != NULL)
	{
		// TODO: Uncomment if the PAN is needed
		//nodePan = EPSTOOL_TlvTree_FindFirstData(tlvTree, TAG_EMV_APPLI_PAN, &dataPan);
		// TODO: Uncomment if the PAN Sequence Number is needed
		//nodePanSeqNumber = EPSTOOL_TlvTree_FindFirstData(tlvTree, TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER, &dataPanSeqNumber);

		// TODO: Get the last transaction amount
		// Set tag TAG_EMV_INT_LAST_TRANSACTION_AMOUNT

		// Free the memory
		EPSTOOL_TlvTree_Release(&tlvTree);
	}
}

//! \brief Execute the terminal risk management step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_TerminalRiskManagement(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP };

	// Display message "Please wait"
	mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_PLEASE_WAIT, NULL, NULL);

	__APEMV_ServicesEmv_GetLastTransaction(EMV_object, inputTlvTree);

	// Call the EMV API to perform Terminal Risk Management
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_TERMINAL_RISK_MANAGEMENT, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}

//! \brief Execute the terminal action analysis step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_TerminalActionAnalysis(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP };

	// Call the EMV API to perform Terminal Action Analysis
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_TERMINAL_ACTION_ANALYSIS, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}

//! \brief Execute the card action analysis step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_CardActionAnalysis(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_INT_TRANSACTION_STATUS};

	// Call the EMV API to perform Card Action Analysis
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_CARD_ACTION_ANALYSIS, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	//Offline authorisaction check.
	{
		static const EPSTOOL_Tag_t tagsToGet[] = {
				TAG_EMV_APPLICATION_CRYPTOGRAM,
				TAG_EMV_CRYPTOGRAM_INFO_DATA,
		};	//16.11.11 Requirment Roh.

		TLV_TREE_NODE tlvTree;
		TLV_TREE_NODE hNode;
		UCHAR *ucpTemp = NULL;

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

				if (TlvTree_GetTag(hNode) == TAG_EMV_CRYPTOGRAM_INFO_DATA)
				{
					if (TlvTree_GetLength(hNode) > 0)
					{
						ucpTemp = (UCHAR*)TlvTree_GetData(hNode);
						if ((*ucpTemp & 0x40) == 0x40)
						{
							return EMV_STATUS_OFFLINE_AUTHED;
						}
					}
				}

				// Next tag
				hNode = TlvTree_GetNext(hNode);
			}
			dmsg("================================================================================================");
			dmsg("================================================================================================");

			// Free the memory
			EPSTOOL_TlvTree_Release(&tlvTree);
		}
	}

	return emvStatus;
}
