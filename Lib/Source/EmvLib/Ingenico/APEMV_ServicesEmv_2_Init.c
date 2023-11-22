/**
* \file
* \brief This module implements the EMV API functionalities related to the transaction initialisation
* (Get Processing Options, Read Records, Offline Data Authentication and Processing Restriction).
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
#include "APEMV_ServicesEmv_1_FinalSelect.h"
#include "APEMV_ServicesEmv_2_Init.h"

//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

////  variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////


//! \brief Called just after the READ RECORD commands.
//! \param[in] inputTlvTree Input TlvTree. Contains data from read records step.
//! \param[out] outputTlvTree Output TlvTree. Usually contains the CA Public Key and PAN related data (such as exception file indicator).
void APEMV_ServicesEmv_StepReadApplicationData(TLV_TREE_NODE inputTlvTree, TLV_TREE_NODE outputTlvTree)
{
	tMtiMap tMapCAPK;
	tMtiMap *tpMapCAPK = &tMapCAPK;
	int iRet = 0,iLen = 0;
	unsigned char *ucpValue = NULL;
	TLV_TREE_NODE nodeAid;
	TLV_TREE_NODE nodeCapkIndex;
	EPSTOOL_Data_t dataAid;
	EPSTOOL_Data_t dataCaPublicKeyIndex;

	ASSERT(outputTlvTree != NULL);

	//closed and just modify it, by @@WAY 18111
#if 0	
	mtiMapInit(tpMapCAPK);

	nodeAid = EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_AID_TERMINAL, &dataAid);
	nodeCapkIndex = EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD, &dataCaPublicKeyIndex);

	if (nodeAid != NULL)
	{
		mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, dataAid.value, dataAid.length);
	}

	if (nodeCapkIndex != NULL)
	{
		mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_CA_PUBLIC_KEY_INDEX_CARD, dataCaPublicKeyIndex.value, dataCaPublicKeyIndex.length);
	}

	iRet = mtiGetEmvParmInfo()->FuncInputCAPKCallback(tpMapCAPK);
	dmsg("FuncInputCAPKCallback iRet[%d]", iRet);
	if (iRet)
	{
		ucpValue = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_EXPONENT, &iLen);
		VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_CAPK_EXPONENT, ucpValue, iLen) != NULL);

		ucpValue = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_MODULUS, &iLen);
		VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_CAPK_MODULUS, ucpValue, iLen) != NULL);
	}

	mtiMapClear(tpMapCAPK);
#else
	nodeAid = EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_AID_TERMINAL, &dataAid);
	nodeCapkIndex = EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD, &dataCaPublicKeyIndex);

	if( (nodeAid != NULL) && (nodeCapkIndex != NULL) )
	{
		mtiMapInit(tpMapCAPK);
		mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, dataAid.value, dataAid.length);
		mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_CA_PUBLIC_KEY_INDEX_CARD, dataCaPublicKeyIndex.value, dataCaPublicKeyIndex.length);

		iRet = mtiGetEmvParmInfo()->FuncInputCAPKCallback(tpMapCAPK);
		dmsg("FuncInputCAPKCallback iRet[%d]", iRet);
		if (iRet)
		{
			ucpValue = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_EXPONENT, &iLen);
			VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_CAPK_EXPONENT, ucpValue, iLen) != NULL);

			ucpValue = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_MODULUS, &iLen);
			VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_CAPK_MODULUS, ucpValue, iLen) != NULL);
		}
		mtiMapClear(tpMapCAPK);
	}
#endif
	//closed and just modify it, by @@WAY 18111

}

//! \brief Execute the init step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_Init(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP };

	// Display message "Please wait"
	mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_PLEASE_WAIT, NULL, NULL);

	// Call the EMV API to perform init
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_INIT, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}

//! \brief Execute the initiate application processing step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_InitiateApplicationSelection(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP };

	// Ask for amount entry
	emvStatus = APEMV_ServicesEmv_GetAmount(inputTlvTree);

	// Ask for account entry
	if (emvStatus == EMV_STATUS_SUCCESS)
	{
		emvStatus = APEMV_ServicesEmv_MenuAccountType(inputTlvTree);

		if (emvStatus == EMV_STATUS_SUCCESS)
		{
			// Display message "Please wait"
			mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_PLEASE_WAIT, NULL, NULL);
	
			// Call the EMV API to perform initiate application processing
			emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_INITIATE_APPLICATION_PROCESSING, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);
		}
	}

	// TODO EMV_STATUS_SELECT_ANOTHER_AID

	return emvStatus;
}

//! \brief Execute the read records step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_ReadRecords(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	// PAN --> TRACK2, for expire date
	//static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_APPLI_PAN, TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER, TAG_EMV_AID_TERMINAL, TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD };
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_TRACK_2_EQU_DATA, TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER, TAG_EMV_AID_TERMINAL, TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD };
	TLV_TREE_NODE hNode;
	INT iValLen = 0;

	UCHAR ucaTemp[1024];

	// Call the EMV API to perform read records
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_READ_APPLICATION_DATA, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);
	if(*tagsKernelToAppli != NULL)
	{
		hNode = TlvTree_GetFirstChild(*tagsKernelToAppli);
		while(hNode != NULL)
		{
			// PAN --> TRACK2, for expire date
			//if (TlvTree_GetTag(hNode) == TAG_EMV_APPLI_PAN)
			if (TlvTree_GetTag(hNode) == TAG_EMV_TRACK_2_EQU_DATA)
			{
				iValLen = TlvTree_GetLength(hNode);

				mtiMemset(ucaTemp, 0, sizeof(ucaTemp));
				mtiHtoa(ucaTemp, TlvTree_GetData(hNode), iValLen);
				iValLen = mtiStrlen(40, ucaTemp);
				
				//if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_CONFIRM_PAN, TlvTree_GetData(hNode), &iValLen) != MTI_EMV_RTN_CONTINUE)
				if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_CONFIRM_PAN, ucaTemp, &iValLen) != MTI_EMV_RTN_CONTINUE)
					emvStatus = EMV_STATUS_CANCEL;
			}

			hNode = TlvTree_GetNext(hNode);
		}
	}

	return emvStatus;
}

//! \brief Execute the offline data authentication step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_OfflineDataAuthentication(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP };

	// Call the EMV API to perform offline data authentication s
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_OFFLINE_DATA_AUTHENTICATION, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}

//! \brief Execute the processing restriction step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_ProcessingRestriction(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP };

	// Call the EMV API to perform processing restriction
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_PROCESSING_RESTRICTION, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}


