/**
* \file
* \brief This module implements the EMV API functionalities related to online processing
* (Authorisation, Referral, Online Processing, Issuer Scripts and the 2nd Generate AC).
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
#include "APEMV_ServicesEmv_5_OnlineProcessing.h"


//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

////  variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

//! \brief Called when the application must request an authorisation from the acquirer.
//! \param[in] inputTlvTree Input TlvTree that usually contains data for authorisation message.
//! \param[out] outputTlvTree Output TlvTree to be filled with TAG_EMV_INT_AUTHORISATION_RESULT tag.
int APEMV_ServicesEmv_Authorisation(TLV_TREE_NODE inputTlvTree, TLV_TREE_NODE outputTlvTree)
{
	int iRet = 0;
	unsigned char ucaAuthResCode[2 + 1];
	unsigned char ucaAuthoResult[1];

	tMtiMap *tpTagMap = &mtiGetEmvParmInfo()->tEmvTagMap;
	unsigned char *ucpTag = NULL;
	int iTagLen = 0;

	// To avoid warnings because 'inputTlvTree' is not used
	(void)inputTlvTree;

	ASSERT(inputTlvTree != NULL);
	ASSERT(outputTlvTree != NULL);

	memset(ucaAuthResCode, 0, sizeof(ucaAuthResCode));
	iRet = mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_ONLINE, ucaAuthResCode, NULL);
	if (iRet == MTI_EMV_ONLINE_APPROVED)
	{
		dmsg("ucaAuthResCode [%s]", ucaAuthResCode);

		ucaAuthoResult[0] = 0x01;
		VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_AUTHORISATION_RESULT, ucaAuthoResult, 1) != NULL);
		VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_AUTHORISATION_RESPONSE_CODE, ucaAuthResCode, 2) != NULL);

		ucpTag = mtiMapGetBytes(tpTagMap, TAG_EMV_AUTHORISATION_CODE, &iTagLen);
		if (iTagLen > 0)
		{
			dbuf("TAG_EMV_AUTHORISATION_CODE", ucpTag, iTagLen);
			VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_AUTHORISATION_CODE, ucpTag, iTagLen) != NULL);
		}

		ucpTag = mtiMapGetBytes(tpTagMap, TAG_EMV_ISSUER_AUTHENTICATION_DATA, &iTagLen);
		if (iTagLen > 0)
		{
			dbuf("TAG_EMV_ISSUER_AUTHENTICATION_DATA", ucpTag, iTagLen);
			VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_ISSUER_AUTHENTICATION_DATA, ucpTag, iTagLen) != NULL);
		}

		return EMV_STATUS_SUCCESS;
	}
	else if (iRet == MTI_EMV_ONLINE_SUSPEND || iRet == MTI_EMV_ONLINE_DENIAL)
	{
		ucaAuthoResult[0] = 0x02;
		VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_AUTHORISATION_RESULT, ucaAuthoResult, 1) != NULL);

		return EMV_STATUS_ONLINE_DECLINED;
	}

	return EMV_STATUS_CANCEL;
}

//! \brief Called when the application must request a voice referral from the acquirer.
//! \param[in] inputTlvTree Input TlvTree.
//! \param[out] outputTlvTree Output TlvTree.
void APEMV_ServicesEmv_VoiceReferral(TLV_TREE_NODE inputTlvTree, TLV_TREE_NODE outputTlvTree)
{
	// To avoid warnings because 'inputTlvTree' is not used
	(void)inputTlvTree;
	// To avoid warnings because 'outputTlvTree' is not used
	(void)outputTlvTree;

	ASSERT(inputTlvTree != NULL);
	ASSERT(outputTlvTree != NULL);

	// TODO: Perform the voice referral
}

//! \brief Execute the online processing step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_OnlineProcessing(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_INT_TRANSACTION_STATUS };
	TLV_TREE_NODE inputAuthorisation;
	INT iResult = 0;

	// inputAuthorisation usually contains the data for authorisation message.
	inputAuthorisation = TlvTree_New(0);
	if (inputAuthorisation == NULL)
	{
		return EMV_STATUS_NOT_ENOUGH_MEMORY;
	}

	// Perform online authorisation. inputTlvTree is filled with TAG_EMV_INT_AUTHORISATION_RESULT tag.
	iResult = APEMV_ServicesEmv_Authorisation(inputAuthorisation, inputTlvTree);
	dmsg("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ APEMV_ServicesEmv_Authorisation() iResult = %d", iResult);
	if (iResult == EMV_STATUS_CANCEL || iResult == EMV_STATUS_ONLINE_DECLINED)
	{
		return iResult;
	}

	// Call the EMV API to perform Online Processing
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_ONLINE_PROCESSING, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	EPSTOOL_TlvTree_Release(&inputAuthorisation);
	return emvStatus;
}

//! \brief Execute the issuer script step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_ISP1(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_INT_TRANSACTION_STATUS };
	tMtiMap *tpTagMap = &mtiGetEmvParmInfo()->tEmvTagMap;
	unsigned char *ucpTag = NULL;
	int iTagLen = 0;

	// TODO: Add scripts in TAG_EMV_ISSUER_SCRIPT_TEMPLATE_1 tags
	ucpTag = mtiMapGetBytes(tpTagMap, TAG_EMV_ISSUER_SCRIPT_TEMPLATE_1, &iTagLen);
	if (iTagLen > 0)
	{
		dbuf("TAG_EMV_ISSUER_SCRIPT_TEMPLATE_1", ucpTag, iTagLen);
		VERIFY(TlvTree_AddChild(inputTlvTree, TAG_EMV_ISSUER_SCRIPT_TEMPLATE_1, ucpTag, iTagLen) != NULL);
	}

	// Call the EMV API to perform issuer scripts
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_ISSUER_TO_CARD_SCRIPTS_PROCESSING1, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}

//! \brief Execute the completion step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_Completion(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_INT_TRANSACTION_STATUS };
	tMtiMap *tpTagMap = &mtiGetEmvParmInfo()->tEmvTagMap;
	unsigned char *ucpTag = NULL;
	int iTagLen = 0;

	ucpTag = mtiMapGetBytes(tpTagMap, TAG_EMV_ISSUER_AUTHENTICATION_DATA, &iTagLen);
	if (iTagLen > 0)
	{
		dbuf("TAG_EMV_ISSUER_AUTHENTICATION_DATA", ucpTag, iTagLen);
		VERIFY(TlvTree_AddChild(inputTlvTree, TAG_EMV_ISSUER_AUTHENTICATION_DATA, ucpTag, iTagLen) != NULL);
	}

	// Call the EMV API to send the 2nd GENERATE AC command
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_COMPLETION, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}

//! \brief Execute the issuer script step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_ISP2(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_INT_TRANSACTION_STATUS };
	tMtiMap *tpTagMap = &mtiGetEmvParmInfo()->tEmvTagMap;
	unsigned char *ucpTag = NULL;
	int iTagLen = 0;

	// TODO: Add scripts in TAG_EMV_ISSUER_SCRIPT_TEMPLATE_2 tags
	ucpTag = mtiMapGetBytes(tpTagMap, TAG_EMV_ISSUER_SCRIPT_TEMPLATE_2, &iTagLen);
	if (iTagLen > 0)
	{
		dbuf("TAG_EMV_ISSUER_SCRIPT_TEMPLATE_2", ucpTag, iTagLen);
		VERIFY(TlvTree_AddChild(inputTlvTree, TAG_EMV_ISSUER_SCRIPT_TEMPLATE_2, ucpTag, iTagLen) != NULL);
	}

	// Call the EMV API to perform issuer scripts
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_ISSUER_TO_CARD_SCRIPTS_PROCESSING2, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}

