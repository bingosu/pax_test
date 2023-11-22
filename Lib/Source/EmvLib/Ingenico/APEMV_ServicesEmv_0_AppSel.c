/**
* \file
* \brief This module implements the EMV API functionalities related to the Application Selection.
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

#include "EPSTOOL_TlvTree.h"
#include "EPSTOOL_PinEntry.h"

#include "EmvLib_Tags.h"
#include "EMV_Status.h"
#include "EMV_ApiTags.h"
#include "EMV_Api.h"

#include "APEMV_ServicesEmv.h"
#include "APEMV_ServicesEmv_0_AppSel.h"

//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//! \brief Structure that contain all information concerning an AID of the candidate list.
typedef struct
{
	int aidLength;							//!< The length of the AID.
	unsigned char aid[16];					//!< The AID.
	char applicationLabel[16 + 1];			//!< The Application Label.
	unsigned char issuerCodeTableIndex;		//!< The Issuer Code Table Index.
	char preferredName[16 + 1];				//!< The Application Preferred Name.
	char languagePreference[8 + 1];			//!< The Language Preference.
	unsigned char priority;					//!< The Application Priority.
	unsigned char cardholderConfirmation;	//!< The cardholder confirmation flag. If \a TRUE, a confirmation is mandatory to use this AID.
} __APEMV_ServicesEmv_AidInfo_t;

//// Static function definitions ////////////////////////////////

static void __APEMV_ServicesEmv_GetAidList(TLV_TREE_NODE outputTlvTree);
static int __APEMV_ServicesEmv_GetCandidateList(TLV_TREE_NODE inputTlvTree, __APEMV_ServicesEmv_AidInfo_t **aids, int *numOfAids);

//// Global variables ///////////////////////////////////////////

////  variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

//! \brief Retrieves the list of supported AIDs by the terminal.
//! \param[out] outputTlvTree Output TlvTree that must be filled with the list of terminal supported AIDs.
static void __APEMV_ServicesEmv_GetAidList(TLV_TREE_NODE outputTlvTree)
{
	static const unsigned char ucaEnabled[1] = { 0x01 };
	TLV_TREE_NODE aidEntryNode;
	tMtiMap tMapAid;
	tMtiMap *tpMapAid = &tMapAid;
	int iRet = 0, i = 0, iKey = 0, iLen = 0;
	unsigned char *ucpValue = NULL;

	ASSERT(outputTlvTree != NULL);

	mtiMapInit(tpMapAid);

	iRet = mtiGetEmvParmInfo()->FuncInputAIDListCallback(tpMapAid);
	if (iRet)
	{
		// Enable support of PSE
		VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_OPTION_PSE, &ucaEnabled, sizeof(ucaEnabled)) != NULL);

		// AID Copy
		for (i = 0; i < tpMapAid->iMapCount; i++)
		{
			iKey = MemCheck(tpMapAid, i);
			iLen = 0;
			ucpValue = mtiMapGetBytes(tpMapAid, iKey, &iLen);

			if (iLen > 0)
			{
				// Gives the list of the supported AIDs
				aidEntryNode = TlvTree_AddChild(outputTlvTree, TAG_EMV_INT_AID_ENTRY, NULL, 0);
				if (aidEntryNode != NULL)
				{
					VERIFY(TlvTree_AddChild(aidEntryNode, TAG_EMV_AID_TERMINAL, ucpValue, iLen) != NULL);
					VERIFY(TlvTree_AddChild(aidEntryNode, TAG_EMV_OPTION_PARTIAL_AID_SELECTION, ucaEnabled, sizeof(ucaEnabled)) != NULL);
				}
			}
		}
	}

	mtiMapClear(tpMapAid);
}


//! \brief Reformat the candidate list from a TlvTree into an array of \ref __APEMV_ServicesEmv_AidInfo_t.
//! \param[in] inputTlvTree Contain the candidate list in the TlvTree format.
//! \param[out] aids The candidate list in an array of \ref __APEMV_ServicesEmv_AidInfo_t.
//! It is a dynamically allocated buffer. It shall be freed by the caller using \a ufree.
//! \param[out] numOfAids The number of AIDs in the candidate list.
//! \return \a TRUE if the conversion is successful, \a FALSE otherwise (not enough memory...).
//! Note that in case of an empty list, the function returns \a TRUE, but \a aids is set to NULL.
static int __APEMV_ServicesEmv_GetCandidateList(TLV_TREE_NODE inputTlvTree, __APEMV_ServicesEmv_AidInfo_t **aids, int *numOfAids)
{
	int result;
	int maxNumberOfAids;
	TLV_TREE_NODE aidNode;
	TLV_TREE_NODE node;
	EPSTOOL_Data_t data;
	__APEMV_ServicesEmv_AidInfo_t *currentAid;

	ASSERT(inputTlvTree != NULL);
	ASSERT(aids != NULL);
	ASSERT(numOfAids != NULL);

	*aids = NULL;
	*numOfAids = 0;

	// Count the number of AIDs in the candidate list
	maxNumberOfAids = 0;
	aidNode = EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_INT_AID_ENTRY, NULL);
	while(aidNode != NULL)
	{
		maxNumberOfAids++;
		aidNode = EPSTOOL_TlvTree_FindNextData(aidNode, TAG_EMV_INT_AID_ENTRY, NULL);
	}

	if (maxNumberOfAids <= 0)
	{
		// No AID within the candidate list!
		result = TRUE;
	}
	else
	{
		// Allocate memory to store the candidate list
		*aids = umalloc(maxNumberOfAids * sizeof(__APEMV_ServicesEmv_AidInfo_t));
		if (*aids != NULL)
		{
			memclr(*aids, maxNumberOfAids * sizeof(__APEMV_ServicesEmv_AidInfo_t));

			// Parse input TlvTree to fill the candidate list array
			*numOfAids = 0;
			currentAid = *aids;
			aidNode = EPSTOOL_TlvTree_FindFirstData(inputTlvTree, TAG_EMV_INT_AID_ENTRY, &data);
			while((aidNode != NULL) && (*numOfAids < maxNumberOfAids))
			{
				// Store the AID
				memcpy(currentAid->aid, data.value, data.length);
				currentAid->aidLength = data.length;

				// Parse all tags within this AID
				node = EPSTOOL_TlvTree_GetFirstChildData(aidNode, &data);
				while((node != NULL) && (*numOfAids < maxNumberOfAids))
				{
					if ((data.length > 0) && (data.value != NULL))
					{
						switch(data.tag)
						{
						case  TAG_EMV_AID_CARD   :
							// Store the aid
							if (data.length <= sizeof(currentAid->aid) - 1)
							{
								memcpy(currentAid->aid, data.value, data.length);
								currentAid->aid[data.length] = '\0';
								currentAid->aidLength = data.length;
							}
							else
							{
								memcpy(currentAid->aid, data.value, sizeof(currentAid->aid) - 1);
								currentAid->aid[sizeof(currentAid->aid) - 1] = '\0';
								currentAid->aidLength = data.length;
							}
							break;
						case TAG_EMV_APPLICATION_LABEL:
							// Store the Application Label (and carefully manage length to avoid buffer overflow)
							if (data.length <= sizeof(currentAid->applicationLabel) - 1)
							{
								memcpy(currentAid->applicationLabel, data.value, data.length);
								currentAid->applicationLabel[data.length] = '\0';
							}
							else
							{
								memcpy(currentAid->applicationLabel, data.value, sizeof(currentAid->applicationLabel) - 1);
								currentAid->applicationLabel[sizeof(currentAid->applicationLabel) - 1] = '\0';
							}
							break;

						case TAG_EMV_APPLI_PREFERED_NAME:
							// Store the Application Preferred Name (and carefully manage length to avoid buffer overflow)
							if (data.length <= sizeof(currentAid->preferredName) - 1)
							{
								memcpy(currentAid->preferredName, data.value, data.length);
								currentAid->preferredName[data.length] = '\0';
							}
							else
							{
								memcpy(currentAid->preferredName, data.value, sizeof(currentAid->preferredName) - 1);
								currentAid->preferredName[sizeof(currentAid->preferredName) - 1] = '\0';
							}
							break;

						case TAG_EMV_LANGUAGE_PREFERENCE:
							// Store the Cardholder Languages (and carefully manage length to avoid buffer overflow)
							if (data.length <= sizeof(currentAid->languagePreference) - 1)
							{
								memcpy(currentAid->languagePreference, data.value, data.length);
								currentAid->languagePreference[data.length] = '\0';
							}
							else
							{
								memcpy(currentAid->languagePreference, data.value, sizeof(currentAid->languagePreference) - 1);
								currentAid->languagePreference[sizeof(currentAid->languagePreference) - 1] = '\0';
							}
							break;

						case TAG_EMV_ISSUER_CODE_TABLE_INDEX:
							// Store the Issuer Code Table Index
							currentAid->issuerCodeTableIndex = *(data.value);
							break;

						case TAG_EMV_APPLI_PRIORITY_INDICATOR:
							// Store the Application Priority with the Cardholder Confirmation Indicator
							currentAid->priority = (*(data.value) & 0x0f);
							currentAid->cardholderConfirmation = ((*(data.value) & 0x80) != 0);
							break;

						default:
							break;
						}
					}

					node = EPSTOOL_TlvTree_GetNextData(node, &data);
				}

				// Next AID
				(*numOfAids)++;
				currentAid++;
				aidNode = EPSTOOL_TlvTree_FindNextData(aidNode, TAG_EMV_INT_AID_ENTRY, &data);
			}

			result = TRUE;
		}
		else
		{
			// Not enough memory
			result = FALSE;
		}
	}

	// TODO: Sort the candidate list 'aids' in a proprietary way

	if (!result)
	{
		// In case of error
		// Free memory and return an empty candidate list
		if (*aids != NULL)
		{
			ufree(*aids);
			*aids = NULL;
		}
		*numOfAids = 0;
	}

	return result;
}

//! \brief Called when the cardholder must select the AID to use.
//! \param[in] AS_object The application selection object to use.
//! \param[out] outputTlvTree Output TlvTree that must contain the selected AID in TAG_EMV_AID_TERMINAL.
EMV_Status_t APEMV_ServicesEmv_MenuSelect(EMVAS_Object_t AS_object, TLV_TREE_NODE outputTlvTree)
{
	EMV_Status_t emvStatus;
	TLV_TREE_NODE candidateList;
	int numOfAids;
	// @@EB MALLOC MODIFY
#if 1
	__APEMV_ServicesEmv_AidInfo_t *aids = NULL;
#else
	__APEMV_ServicesEmv_AidInfo_t *aids;
#endif
	tMtiMap tAidList;
	tMtiMap *tpAidList = &tAidList;
	int i, iSelIndex = 0;
	CHAR caTemp[128 + 1];
	CHAR *cpTemp = caTemp;

	ASSERT(outputTlvTree != NULL);

	emvStatus = EMV_STATUS_SUCCESS;

	mtiMapInit(tpAidList);

	// Call the EMV API to get the candidate list
	candidateList = NULL;
	emvStatus = EMVAS_GetCandidateList(AS_object, &candidateList);
	if((emvStatus == EMV_STATUS_SUCCESS) && (candidateList != NULL))
	{
		// Convert the candidate list from TlvTree into an array of structures
		if (__APEMV_ServicesEmv_GetCandidateList(candidateList, &aids, &numOfAids))
		{
			if (numOfAids > 0)
			{
				if ((numOfAids == 1) && (!aids[0].cardholderConfirmation))
				{
					// There is only one AID and no confirmation is required
					//  => select it
					VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_AID_TERMINAL, aids[0].aid, aids[0].aidLength) !=  NULL);
				}
				else
				{
					// Application label input into the mtiMap.
					dmsg("** AID Count = %d", numOfAids);
					for (i = 0; i < numOfAids; i++)
					{
						mtiMemset(caTemp, 0, sizeof(caTemp));
						cpTemp = caTemp;

						cpTemp += mtiHtoa((UCHAR*)cpTemp, aids[i].aid, aids[i].aidLength * 2);
						*cpTemp++ = '|';
						mtiStrcpy(cpTemp, aids[i].applicationLabel, 16);
						dmsg("[%02d:%s]", i, caTemp);

						mtiMapPutString(tpAidList, tpAidList->iMapCount + 1, caTemp);
					}

					// Otherwise, display a screen with the list of AIDs
					iSelIndex = 0;
					if (mtiGetEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_SELECT_AID, tpAidList, &iSelIndex) == MTI_EMV_RTN_CONTINUE)
					{
						dmsg("**** iSelIndex = %d", iSelIndex);
						// Add the selected AID into the output TlvTree
						VERIFY(TlvTree_AddChild(outputTlvTree, TAG_EMV_AID_TERMINAL, aids[iSelIndex].aid, aids[iSelIndex].aidLength) !=  NULL);
					}
					else
					{
						// Cancelled
						emvStatus = EMV_STATUS_CANCEL;
					}
				}
			}
			else
			{
				// Empty candidate list
				emvStatus = EMV_STATUS_SUCCESS;
			}
		}
		else
		{
			// Error (not enough memory to convert TlvTree into an array of structures)
			emvStatus = EMV_STATUS_NOT_ENOUGH_MEMORY;
		}

		// Free the memory
		if (aids != NULL)
		{
			ufree(aids);
			aids = NULL;
		}
	}

	mtiMapClear(tpAidList);
	EPSTOOL_TlvTree_Release(&candidateList);
	return emvStatus;
}


//! \brief Execute the application selection step.
//! \param[in] AS_object The application selection object to use.
//! \return  Can return any value of EMV_Status_t.
EMV_Status_t APEMV_ServicesEmv_ApplicationSelection(EMVAS_Object_t AS_object)
{
	EMV_Status_t emvStatus;
	TLV_TREE_NODE inputTlvTree;
	TLV_TREE_NODE outputTlvTree;

	emvStatus = EMV_STATUS_UNKNOWN;

	// Create the input data
	inputTlvTree = TlvTree_New(0);
	if(inputTlvTree != NULL)
	{
		// Get the transaction data (amount, date and time, ...)
		if(APEMV_ServicesEmv_GetTransactionData(inputTlvTree))
		{
			// Get the supported AIDs
			__APEMV_ServicesEmv_GetAidList(inputTlvTree);

			// Call the EMV API to perform application selection
			emvStatus = EMVAS_ApplicationSelection(AS_object, inputTlvTree, &outputTlvTree);
		}
		else
		{
			emvStatus = EMV_STATUS_UNEXPECTED_ERROR;
		}
	}
	else
	{
		emvStatus = EMV_STATUS_NOT_ENOUGH_MEMORY;
	}

	EPSTOOL_TlvTree_Release(&inputTlvTree);
	EPSTOOL_TlvTree_Release(&outputTlvTree);

	return emvStatus;
}
