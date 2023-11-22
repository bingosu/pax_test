/**
* \file
* \brief This module implements the EMV API functionalities related to Cardholder Verification.
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
#include "APEMV_ServicesEmv_3_CVM.h"


//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

//// variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////


//! \brief Called when the cardholder must be authenticated.
//! \param[in] inputTlvTree Input TlvTree that especially contains the method to use in TAG_EMV_INT_CV_STATUS.
//! \param[out] outputTlvTree Output TlvTree must be filled with at least TAG_EMV_INT_CV_RESULT (set it to EMV_INT_CV_RESULT_SUCCESS to terminate CVM).
//! Others tags may be given depending on the method such as TAG_EMV_INT_PIN_ENTRY_LENGTH for PIN.
//! \return  Can return any value of EMV_Status_t.
EMV_Status_t APEMV_ServicesEmv_StepCardholderVerification(TLV_TREE_NODE inputTlvTree, TLV_TREE_NODE outputTlvTree)
{
	EMV_Status_t emvStatus;
	EPSTOOL_Data_t dataCvStatus;
	EPSTOOL_Data_t dataIssuerCountryCode;
	unsigned char cvStatus;
	int signature, cvm_req;
	int methodResult;
	int iRet = 0;

	ASSERT(inputTlvTree != NULL);
	ASSERT(outputTlvTree != NULL);

	emvStatus = EMV_STATUS_SUCCESS;

	signature = FALSE;

	if (EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_ISSUER_COUNTRY_CODE, &dataIssuerCountryCode) != NULL)
	{
		dbuf("TAG_EMV_ISSUER_COUNTRY_CODE", dataIssuerCountryCode.value, dataIssuerCountryCode.length);
		mtiMapPutBytes(&mtiGetEmvParmInfo()->tEmvTagMap, (INT)dataIssuerCountryCode.tag, dataIssuerCountryCode.value, dataIssuerCountryCode.length);
	}

	// Retrieve TAG_EMV_INT_CV_STATUS
	if (EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_INT_CV_STATUS, &dataCvStatus) != NULL)
	{
		if ((dataCvStatus.length == 2) && (dataCvStatus.value != NULL))
		{
			cvStatus = dataCvStatus.value[0];

			dmsg("cvStatus [%02X]", cvStatus);
			if((cvStatus & EMV_INT_CV_STATUS_SIGNATURE) != 0)
			{
				dmsg("********** EMV_INT_CV_STATUS_SIGNATURE **********");
				mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_SIGN, NULL, NULL);
				signature = TRUE;
			}

			if((cvStatus & EMV_INT_CV_STATUS_END) != 0)
			{
				if (signature)
				{
					if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_SIGN, NULL, NULL) == MTI_EMV_RTN_CONTINUE)
					{
						// Only signature is required
						methodResult = EMV_INT_CV_RESULT_SUCCESS;
					}
					else
					{
						// Transaction cancel.
						methodResult = EMV_INT_CV_RESULT_CANCEL;
					}
				}
			}
			// OFFLINE PIN
			else if((cvStatus & EMV_INT_CV_STATUS_OFFLINE_PIN) != 0)
			{
#if 1
				if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_BEFORE_OFFLINE_PIN_INPUT, NULL, NULL) ==
						MTI_EMV_RTN_PIN_BYPASS)
				{
					methodResult = EMV_INT_CV_RESULT_BYPASS;
					emvStatus = EMV_STATUS_SUCCESS;
				}
				else
				{
					if ((cvStatus & EMV_INT_CV_STATUS_OFFLINE_PIN_WRONG ) != 0)
					{
						dmsg("**** EMV_INT_CV_STATUS_OFFLINE_PIN_WRONG ****");
						mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_WRONG_PIN, NULL, NULL);
					}

					if((cvStatus & EMV_INT_CV_STATUS_LAST_ATTEMPT ) != 0)
					{
						dmsg("**** EMV_INT_CV_STATUS_LAST_ATTEMPT ****");
						mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_LAST_ATTEMP_PIN, NULL, NULL);
					}

					dmsg("**** OFFLINE PIN REQUIRED!! ****");
					iRet = __APEMV_InputSecurePinEntry(outputTlvTree, 0);
					if (iRet == RTN_SUCCESS)
					{
						mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_OFFLINE_PIN_INPUT, NULL, NULL);
					}

					// Application CVM accommodated.
					if (iRet == RTN_SUCCESS)
					{
						emvStatus = EMV_STATUS_SUCCESS;
						methodResult = EMV_INT_CV_RESULT_SUCCESS;
					}
					else if (iRet == RTN_SKIP)
					{
						emvStatus = EMV_STATUS_SUCCESS;
						methodResult = EMV_INT_CV_RESULT_BYPASS;
					}
					else
					{
						emvStatus = EMV_STATUS_CANCEL;
						methodResult = EMV_INT_CV_RESULT_CANCEL;
					}
				}
#else
				methodResult = EMV_INT_CV_RESULT_BYPASS;
				emvStatus = EMV_STATUS_SUCCESS;
#endif
			}
			else if((cvStatus & EMV_INT_CV_STATUS_ONLINE_PIN) != 0)
			{
				// Request a signature if requested and if PIN entry was done successfully
				if ((signature) && (methodResult == EMV_INT_CV_RESULT_SUCCESS))
				{
					// Request signature
					cvm_req = MTI_EMV_STEP_NEED_SIGN;
				}
				else
				{
					//Online PIN confirm. so "PIN-bypass" not allowed.
					cvm_req = MTI_EMV_STEP_NEED_PIN;
				}

				iRet = mtiGetEmvParmInfo()->FuncUICallback(cvm_req, NULL, NULL);
				if (iRet == MTI_EMV_RTN_CONTINUE)
				{
					// Application CVM accommodated.
					methodResult = EMV_INT_CV_RESULT_SUCCESS;
				}
				// Modifications due to adding and changing return value (MTI_EMV_TRN_SIGN)
				else if ((iRet == MTI_EMV_RTN_PIN_BYPASS) || (iRet == MTI_EMV_TRN_SIGN))
				{
					dpt();
					// PIN BY PASS
					methodResult = EMV_INT_CV_RESULT_BYPASS;
				}
				else
				{
					// Transaction cancel.
					methodResult = EMV_INT_CV_RESULT_CANCEL;
					emvStatus = EMV_STATUS_CANCEL;
				}
			}
			else if((cvStatus & EMV_INT_CV_STATUS_PROPRIETARY_METHOD) != 0)
			{
				// The selected method is a proprietary one
				methodResult = EMV_INT_CV_RESULT_SUCCESS;
			}
			else
			{
				// Unknown method
				methodResult = EMV_INT_CV_RESULT_SUCCESS;
			}

			// Set TAG_EMV_INT_CV_RESULT to indicate the result to the EMV API
			VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_CV_RESULT, &methodResult, 1) != NULL);
		}
		else
		{
			emvStatus = EMV_STATUS_UNEXPECTED_ERROR;
		}
	}
	else
	{
		emvStatus = EMV_STATUS_UNEXPECTED_ERROR;
	}

	return emvStatus;
}

//! \brief Execute the cardholder verification step of an EMV transaction.
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
EMV_Status_t APEMV_ServicesEmv_CardholderVerification(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel)
{
	EMV_Status_t emvStatus;
	static const EPSTOOL_Tag_t tagsToGet[] = { TAG_EMV_INT_NEXT_STEP, TAG_EMV_INT_CV_STATUS, TAG_EMV_ISSUER_COUNTRY_CODE };

	// Call the EMV API to perform cardholder verification
	emvStatus = EMV_ExecuteStep(EMV_object, EMV_STEP_CARDHOLDER_VERIFICATION, inputTlvTree, sizeof(tagsToGet) / sizeof(tagsToGet[0]), tagsToGet, tagsKernelToAppli, tagsRequestedByKernel);

	return emvStatus;
}
