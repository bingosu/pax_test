/**
* \file
* \brief This file shows the interface of the EMV Level 2 kernel.
*
* \author ------------------------------------------------------------------------------\n
* \author INGENICO Technical Software Department\n
* \author ------------------------------------------------------------------------------\n
* \author Copyright (c) 2013 - 2016 Ingenico.\n
* \author 28-32 boulevard de Grenelle 75015 Paris, France.\n
* \author All rights reserved.\n
* \author This source program is the property of the INGENICO Company mentioned above\n
* \author and may not be copied in any form or by any means, whether in part or in whole,\n
* \author except under license expressly granted by such INGENICO company.\n
* \author All copies of this source program, whether in part or in whole, and\n
* \author whether modified or not, must display this and all other\n
* \author embedded copyright and ownership notices in full.\n
**/

/////////////////////////////////////////////////////////////////

#ifndef __EMV_API_H__
//! \cond
#define __EMV_API_H__
//! \endcond

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

#ifndef SWIG  
	//! \brief Check if DLL is present
	//! \return  1 if DLL is present 0 otherwise 
	int LIBEMVAPI_IsPresent(void);
#endif

//! \addtogroup EMVAPI_ApplicationSelection
//! An application will use this API to perform the EMVCo application selection process in the <b>Telium explicit selection</b> mode.

//! \addtogroup EMVAPI_ApplicationSelection
//! \{

//! \brief EMV application selection object.
typedef struct __EMVAS_Object_s *EMVAS_Object_t;

//! \}

//! \addtogroup EMVAPI_Transaction_Functions
//! \{

//! \brief EMV transaction object.
typedef struct __EMV_Object_s *EMV_Object_t;

//! \brief List of all the transaction steps.
typedef enum
{
	EMV_STEP_NONE									= 0,		//!< No step.
	EMV_STEP_INIT									= 1,		//!< Initialisation transaction step. See \ref EMVAPI_STEP_Init.
	EMV_STEP_FINAL_SELECT							= 2,		//!< Final Select transaction step. See \ref EMVAPI_STEP_FinalSelect.
	EMV_STEP_INITIATE_APPLICATION_PROCESSING		= 3,		//!< Initiate Application Processing transaction step. See \ref EMVAPI_STEP_InitiateApplicationProcessing.
	EMV_STEP_READ_APPLICATION_DATA					= 4,		//!< Read Application Data transaction step. See \ref EMVAPI_STEP_ReadApplicationData.
	EMV_STEP_OFFLINE_DATA_AUTHENTICATION			= 5,		//!< Offline Data Authentication transaction step. See \ref EMVAPI_STEP_OfflineDataAuthentication.
	EMV_STEP_PROCESSING_RESTRICTION					= 6,		//!< Processing Restriction transaction step. See \ref EMVAPI_STEP_ProcessingRestriction.
	EMV_STEP_CARDHOLDER_VERIFICATION				= 7,		//!< Cardholder Verification transaction step. See \ref EMVAPI_STEP_CarholderVerification.
	EMV_STEP_TERMINAL_RISK_MANAGEMENT				= 8,		//!< Terminal Risk Management transaction step. See \ref EMVAPI_STEP_TerminalRiskManagement.
	EMV_STEP_TERMINAL_ACTION_ANALYSIS				= 9,		//!< Terminal Action Analysis transaction step. See \ref EMVAPI_STEP_TerminalActionAnalysis.
	EMV_STEP_CARD_ACTION_ANALYSIS					= 10,		//!< Card Action Analysis transaction step. See \ref EMVAPI_STEP_CardActionAnalysis.
	EMV_STEP_ONLINE_PROCESSING						= 11,		//!< Online Processing transaction step. See \ref EMVAPI_STEP_OnlineProcessing.
	EMV_STEP_ISSUER_TO_CARD_SCRIPTS_PROCESSING1		= 12,		//!< Issuer To Card Scripts Processing (before 2nd GENERATE AC) transaction step. See \ref EMVAPI_STEP_IssuerToCardScriptsProcessing1.
	EMV_STEP_COMPLETION								= 13,		//!< Completion transaction step. See \ref EMV_STEP_COMPLETION.
	EMV_STEP_ISSUER_TO_CARD_SCRIPTS_PROCESSING2		= 14,		//!< Issuer To Card Scripts Processing (after 2nd GENERATE AC) transaction step. See \ref EMVAPI_STEP_IssuerToCardScriptsProcessing2.
	EMV_STEP_END									= 15		//!< End transaction step. See \ref EMV_STEP_END.
} EMV_TransactionStep_t;

//! \}

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

//// Variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

//! \addtogroup EMVAPI_ApplicationSelection
//! \{

//! \brief Create a new EMV application selection object.
//! \param[out] object The new created object ID or a \a NULL value if the creation failed.
//! \param[in] cardReader The card reader to use. It could be CAM0 CAM1 or CAM2.
//! \param[in] cardReaderBufferSize Size of the cardReader buffer, to avoid buffer overflow on the \a cardReader string if not zero terminated.
//! \return  Usually return :
//!	- \ref EMV_STATUS_SUCCESS :   The object has been created.
//! - \ref EMV_STATUS_NOT_ENOUGH_MEMORY : There is not enough memory to create the object.
//! - \ref EMV_STATUS_NOT_ALLOWED : Cannot retrieve the application that called this function (OS issue).
//! - \ref EMV_STATUS_COMPONENT_ERROR : Cannot communicate with the EMV API software component.
//! \note All EMV application selection object can only be used by the application that created it.
//! If another application tries to use, destroy ... it, it will get the status \ref EMV_STATUS_NOT_ALLOWED.
EMV_Status_t EMVAS_Create(EMVAS_Object_t *object, const char *cardReader, size_t cardReaderBufferSize);


//! \brief Destroy a previously created EMV application selection object.
//! \param[in] object The EMV application selection object to destroy.
//! \return  Usually return :
//! - \ref EMV_STATUS_SUCCESS : The object has been destroyed.
//! - \ref EMV_STATUS_INVALID_OBJECT : \a object is not a valid EMV application selection object.
//! - \ref EMV_STATUS_NOT_ALLOWED : \a object has been created by another application, so it cannot be used.
//! - \ref EMV_STATUS_COMPONENT_ERROR : Cannot communicate with the EMV API software component.
EMV_Status_t EMVAS_Destroy(EMVAS_Object_t object);


//! \brief Perform the EMV application selection process, either by using PSE or by using List Of AIDs.
//! \param[in] object The EMV application selection object to use.
//! \param[in] input_tags A TlvTree that contains the following input data :
//!	- \ref TAG_EMV_OPTION_PSE : flag to enable or disable the support of PSE (optional, enabled by default).
//!	- \ref TAG_EMV_INT_AS_ALLOW_BLOCKED_AID : a flag to indicate if the AID could be selected even if it is blocked (disabled by default).
//!	- \ref TAG_EMV_INT_AID_ENTRY for each supported AID
//! 	- \ref TAG_EMV_AID_TERMINAL : the AID (mandatory).
//! 	- \ref TAG_EMV_OPTION_PARTIAL_AID_SELECTION : a flag to indicate if partial match is supported or not (optional, supported by default).
//! \param[out] output_tags If not \a NULL, the function creates a new TlvTree object and fill it with the following data :
//!	- \ref TAG_EMV_INT_AS_METHOD_USED : the application selection method used. See \ref TAG_EMV_INT_AS_METHOD_USED_values for the possible values.
//!	- \ref TAG_EMV_INT_CARD_RESPONSE : the response of the SELECT PSE command (conditional, present if PSE allowed).
//!	- \ref TAG_EMV_LANGUAGE_PREFERENCE : the Language Preference (conditional, given only if received from the card during PSE).
//!	- \ref TAG_EMV_ISSUER_CODE_TABLE_INDEX : the Issuer Code Table Index (conditional, given only if received from the card during PSE).
//!	- The Candidate List. \ref TAG_EMV_INT_AID_ENTRY for each AID.
//!		- \ref TAG_EMV_AID_CARD : the AID (mandatory).
//!		- \ref TAG_EMV_INT_CARD_RESPONSE : the response of the SELECT command (conditional, given only if List Of AIDs is used).
//!		- \ref TAG_EMV_INT_AID_STATUS : the status of the AID. See \ref TAG_EMV_INT_AID_STATUS_values for the possible values.
//!		- \ref TAG_EMV_APPLICATION_LABEL : the Application Label (conditional, given only if received from the card).
//!		- \ref TAG_EMV_APPLI_PREFERRED_NAME : the Application Preferred Name (conditional, given only if received from the card).
//!		- \ref TAG_EMV_APPLI_PRIORITY_INDICATOR : the Application Priority Indicator (conditional, given only if received from the card).
//!		- \ref TAG_EMV_DICTIONARY_DISCR_TEMPLATE : the Directory Discretionary Template (conditional, given only if PSE is used and if received from the card).
//!		- \ref TAG_EMV_PDOL : the PDOL (conditional, given only if List of AIDs is used and if received from the card).
//!		- \ref TAG_EMV_LANGUAGE_PREFERENCE : the Language Preference (conditional, given only if received from the card).
//!		- \ref TAG_EMV_ISSUER_CODE_TABLE_INDEX : the Issuer Code Table Index (conditional, given only if received from the card).
//!		- \ref TAG_EMV_FCI_ISSUER_DISCRET_DATA : the FCI Issuer Discretionary Data (conditional, given only if List of AIDs is used and if received from the card).
//!		- \ref TAG_EMV_LOG_ENTRY : the Log Entry (conditional, given only if List of AIDs is used and if received from the card).
//! \return  Can return any value of \ref EMV_Status_t.<br>
//! \note The application is in charge to destroy the created TlvTree \a output_tags.
//! \note The application must keep the returned values to be able to give them to the transaction module.
EMV_Status_t EMVAS_ApplicationSelection(EMVAS_Object_t object, TLV_TREE_NODE input_tags, TLV_TREE_NODE *output_tags);


//! \brief Remove a given AID from the candidate list.
//! \param[in] object The EMV application selection object to use.
//! \param[in] input_tags A TlvTree that contains the following input data :
//!	- \ref TAG_EMV_INT_AID_ENTRY
//!		- \ref TAG_EMV_AID_TERMINAL : the AID to remove from the candidate list.
//! \param[out] output_tags If not \a NULL, the function creates a new TlvTree object and fill it with the following data :
//!	- \ref TAG_EMV_INT_AS_METHOD_USED : the application selection method used. See \ref TAG_EMV_INT_AS_METHOD_USED_values for the possible values.
//!	- \ref TAG_EMV_INT_CARD_RESPONSE : the response of the SELECT PSE command (conditional, present if PSE allowed).
//!	- \ref TAG_EMV_LANGUAGE_PREFERENCE : the Language Preference (conditional, given only if received from the card during PSE).
//!	- \ref TAG_EMV_ISSUER_CODE_TABLE_INDEX : the Issuer Code Table Index (conditional, given only if received from the card during PSE).
//!	- The new Candidate List. \ref TAG_EMV_INT_AID_ENTRY for each AID.
//!		- \ref TAG_EMV_AID_CARD : the AID (mandatory).
//!		- \ref TAG_EMV_INT_CARD_RESPONSE : the response of the SELECT command (conditional, given only if List Of AIDs is used).
//!		- \ref TAG_EMV_INT_AID_STATUS : the status of the AID. See \ref TAG_EMV_INT_AID_STATUS_values for the possible values.
//!		- \ref TAG_EMV_APPLICATION_LABEL : the Application Label (conditional, given only if received from the card).
//!		- \ref TAG_EMV_APPLI_PREFERRED_NAME : the Application Preferred Name (conditional, given only if received from the card).
//!		- \ref TAG_EMV_APPLI_PRIORITY_INDICATOR : the Application Priority Indicator (conditional, given only if received from the card).
//!		- \ref TAG_EMV_DICTIONARY_DISCR_TEMPLATE : the Directory Discretionary Template (conditional, given only if PSE is used and if received from the card).
//!		- \ref TAG_EMV_PDOL : the PDOL (conditional, given only if List of AIDs is used and if received from the card).
//!		- \ref TAG_EMV_LANGUAGE_PREFERENCE : the Language Preference (conditional, given only if received from the card).
//!		- \ref TAG_EMV_ISSUER_CODE_TABLE_INDEX : the Issuer Code Table Index (conditional, given only if received from the card).
//!		- \ref TAG_EMV_FCI_ISSUER_DISCRET_DATA : the FCI Issuer Discretionary Data (conditional, given only if List of AIDs is used and if received from the card).
//!		- \ref TAG_EMV_LOG_ENTRY : the Log Entry (conditional, given only if List of AIDs is used and if received from the card).
//! \return  Can return any value of \ref EMV_Status_t.<br>
//! \note The application is in charge to destroy the created TlvTree \a output_tags.
EMV_Status_t EMVAS_RemoveAid(EMVAS_Object_t object, TLV_TREE_NODE input_tags, TLV_TREE_NODE *output_tags);


//! \brief Retrieves the candidate list.
//! \param[in] object The EMV application selection object to use.
//! \param[out] output_tags If not \a NULL, the function creates a new TlvTree object and fill it with the following data :
//!	- \ref TAG_EMV_INT_AS_METHOD_USED : the application selection method used. See \ref TAG_EMV_INT_AS_METHOD_USED_values for the possible values.
//!	- \ref TAG_EMV_INT_CARD_RESPONSE : the response of the SELECT PSE command (conditional, present if PSE allowed).
//!	- \ref TAG_EMV_LANGUAGE_PREFERENCE : the Language Preference (conditional, given only if received from the card during PSE).
//!	- \ref TAG_EMV_ISSUER_CODE_TABLE_INDEX : the Issuer Code Table Index (conditional, given only if received from the card during PSE).
//!	- The Candidate List. \ref TAG_EMV_INT_AID_ENTRY for each AID.
//!		- \ref TAG_EMV_AID_CARD : the AID (mandatory).
//!		- \ref TAG_EMV_INT_CARD_RESPONSE : the response of the SELECT command (conditional, given only if List Of AIDs is used).
//!		- \ref TAG_EMV_INT_AID_STATUS : the status of the AID. See \ref TAG_EMV_INT_AID_STATUS_values for the possible values.
//!		- \ref TAG_EMV_APPLICATION_LABEL : the Application Label (conditional, given only if received from the card).
//!		- \ref TAG_EMV_APPLI_PREFERRED_NAME : the Application Preferred Name (conditional, given only if received from the card).
//!		- \ref TAG_EMV_APPLI_PRIORITY_INDICATOR : the Application Priority Indicator (conditional, given only if received from the card).
//!		- \ref TAG_EMV_DICTIONARY_DISCR_TEMPLATE : the Directory Discretionary Template (conditional, given only if PSE is used and if received from the card).
//!		- \ref TAG_EMV_PDOL : the PDOL (conditional, given only if List of AIDs is used and if received from the card).
//!		- \ref TAG_EMV_LANGUAGE_PREFERENCE : the Language Preference (conditional, given only if received from the card).
//!		- \ref TAG_EMV_ISSUER_CODE_TABLE_INDEX : the Issuer Code Table Index (conditional, given only if received from the card).
//!		- \ref TAG_EMV_FCI_ISSUER_DISCRET_DATA : the FCI Issuer Discretionary Data (conditional, given only if List of AIDs is used and if received from the card).
//!		- \ref TAG_EMV_LOG_ENTRY : the Log Entry (conditional, given only if List of AIDs is used and if received from the card).
//! \return  Can return any value of \ref EMV_Status_t.<br>
//! \note The application is in charge to destroy the created TlvTree \a output_tags.
EMV_Status_t EMVAS_GetCandidateList(EMVAS_Object_t object, TLV_TREE_NODE *output_tags);


//! \brief Retrieves the application selection commands issued to the card with their responses.
//! \param[in] object The EMV application selection object to use.
//! \param[in] input_tags A TlvTree that contains the following input data :
//!	- \ref TAG_EMV_INT_COMMANDS_TO_GET : A data indicating the commands to get. It can be:
//!		- \ref EMVAS_GET_CMD_PSE : the SELECT PSE command/response.
//!		- \ref EMVAS_GET_CMD_PSE_ALL : all the PSE process commands/responses.
//!		- \ref EMVAS_GET_CMD_LIST_OF_AIDS : the List Of AIDs commands/responses.
//!		- \ref EMVAS_GET_CMD_ALL : all the commands/responses.
//! \param[out] output_tags If not \a NULL, the function creates a new TlvTree object and fill it with the required commands/responses. For each command/response :
//!	- \ref TAG_EMV_INT_CMD_ENTRY : that contains:
//!		- \ref TAG_EMV_INT_CARD_COMMAND : the command that has been sent.
//!		- \ref TAG_EMV_INT_CARD_RESPONSE : the received response.
//!		- \ref TAG_EMV_INT_CMD_BEGIN_TIME : the time code of when the command has been sent.
//!		- \ref TAG_EMV_INT_CMD_END_TIME : time code of when the response has been received.
//!		- \ref TAG_EMV_INT_L1_DRIVER_ERROR : the level 1 driver error if any.
//! \return Can return any value of \ref EMV_Status_t.<br>
//! \note The application is in charge to destroy the created TlvTree \a output_tags.
EMV_Status_t EMVAS_GetCommands(EMVAS_Object_t object, TLV_TREE_NODE input_tags, TLV_TREE_NODE *output_tags);

//! \brief Retrieves the default configuration of the EMV kernel. This default configuration is used during implicit selection.
//! \param[out] output_tags If not \a NULL, the function creates a new TlvTree object and fill it with the default configuration of the EMV kernel.
//! The only supported parameter is \ref TAG_EMV_OPTION_PSE.
//! \return Can return any value of \ref EMV_Status_t.<br>
//! \note The application is in charge to destroy the created TlvTree \a output_tags.
EMV_Status_t EMVAS_GetDefaultParams(TLV_TREE_NODE *output_tags);

//! \brief Change the default configuration of the EMV kernel. This default configuration is used during implicit selection.
//! \param[in] input_tags A TlvTree that contains the default configuration of the EMV kernel. The only supported parameter is \ref TAG_EMV_OPTION_PSE.
//! \return Can return any value of \ref EMV_Status_t.<br>
EMV_Status_t EMVAS_SetDefaultParams(TLV_TREE_NODE input_tags);

//! \}

//! \addtogroup EMVAPI_Transaction_Functions
//! \{

//! \brief Create a new EMV transaction object.
//! \param[out] object The new created object ID or a \a NULL value if the creation failed.
//! \param[in] cardReader The card reader to use. It could be CAM0 CAM1 or CAM2.
//! \param[in] cardReaderBufferSize Size of the cardReader buffer, to avoid buffer overflow on the \a cardReader string if not zero terminated.
//! \return  Usually return:
//! - \ref EMV_STATUS_SUCCESS : The object has been created.
//! - \ref EMV_STATUS_NOT_ENOUGH_MEMORY : There is not enough memory to create the object.
//! - \ref EMV_STATUS_NOT_ALLOWED : Cannot retrieve the application that called this function (OS issue).
//! - \ref EMV_STATUS_COMPONENT_ERROR : Cannot communicate with the EMV API software component.
//! \note All EMV transaction object can only be used by the application that created it.
//! If another application tries to use, destroy ... it, it will get the status EMV_STATUS_NOT_ALLOWED.
EMV_Status_t EMV_Create(EMV_Object_t *object, const char *cardReader, size_t cardReaderBufferSize);

//! \brief Destroy a previously created EMV transaction object.
//! \param[in] object The EMV transaction object to destroy.
//! \return  Usually return:
//! - \ref EMV_STATUS_SUCCESS : The object has been destroyed.
//! - \ref EMV_STATUS_INVALID_OBJECT : \a object is not a valid EMV transaction object.
//! - \ref EMV_STATUS_NOT_ALLOWED : \a object has been created by another application, so it cannot be used.
//! - \ref EMV_STATUS_COMPONENT_ERROR : Cannot communicate with the EMV API software component.
EMV_Status_t EMV_Destroy(EMV_Object_t object);

//! \brief Execute a step of an EMV transaction.
//! \param[in] object The EMV transaction object to use.
//! \param[in] step The transaction step to execute. See EMV_TransactionStep_t.
//! \param[in] tagsAppliToKernel Contains any tag that the application wants to give to the EMV kernel.
//! Can be \a NULL if the application has no tag to give.
//! \param[in] numOfTagsRequestedByAppli Number of tags pointed by \a tagsRequestedByAppli.
//! \param[in] tagsRequestedByAppli Pointer to \a numOfTagsRequestedByAppli unsigned long values.
//! It represents the tags that the application wants to have their values.
//! Values of these tags will be given back into \a tagsKernelToAppli.
//! Can be \a NULL if \a numOfTagsRequestedByAppli is 0.
//! \param[out] tagsKernelToAppli If not \a NULL, the function creates a new TlvTree object and fill it
//! with the tags and values requested by the application using \a tagsRequestedByAppli.
//! If no tag is requested by the application or if the EMV kernel does not know any value of the requested tag,
//!  no TlvTree is created (so value is set to \a NULL).
//! \param[out] tagsRequestedByKernel If not \a NULL, the function creates a new TlvTree object and fill it
//! with the tags (with empty values) that the application shall provide to the next transaction step (using the \a tagsAppliToKernel parameter).
//! If no tag is requested by the EMV kernel, no TlvTree is created (so value is set to \a NULL).
//! \return  Can return any value of \ref EMV_Status_t.
//! note The application is in charge to destroy the created TlvTree \a tagsKernelToAppli and \a tagsRequestedByKernel.
EMV_Status_t EMV_ExecuteStep(EMV_Object_t object, EMV_TransactionStep_t step, TLV_TREE_NODE tagsAppliToKernel,
		unsigned int numOfTagsRequestedByAppli, const unsigned long *tagsRequestedByAppli,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel);


//! \brief Retrieves the commands issued to the card with their responses.
//! \param[in] object The EMV transaction object to use.
//! \param[in] input_tags A TlvTree that contains the following input data :
//!	- \ref TAG_EMV_INT_COMMANDS_TO_GET : A data indicating the commands to get. It can be:
//!		- \ref EMV_GET_CMD_LAST_COMMAND : the last command/response.
//!		- \ref EMV_GET_CMD_LAST_STEP : the commands/responses of the last called step.
//!		- \ref EMV_GET_CMD_ALL : all the command/response.
//! \param[out] output_tags If not \a NULL, the function creates a new TlvTree object and fill it with the required commands/responses. For each command/response :
//!	- \ref TAG_EMV_INT_CMD_ENTRY : that contains:
//!		- \ref TAG_EMV_INT_CARD_COMMAND : the command that has been sent.
//!		- \ref TAG_EMV_INT_CARD_RESPONSE : the received response.
//!		- \ref TAG_EMV_INT_CMD_BEGIN_TIME : the time code of when the command has been sent.
//!		- \ref TAG_EMV_INT_CMD_END_TIME : time code of when the response has been received.
//!		- \ref TAG_EMV_INT_L1_DRIVER_ERROR : the level 1 driver error if any.
//! \return  Can return any value of \ref EMV_Status_t.<br>
//! \note The application is in charge to destroy the created TlvTree \a output_tags.

EMV_Status_t EMV_GetCommands(EMV_Object_t object, TLV_TREE_NODE input_tags, TLV_TREE_NODE *output_tags);



//! \brief Retrieve the values of the requested tags.
//! \param[in] object The EMV transaction object to use.
//! \param[in] numOfTagsRequestedByAppli Number of tags pointed by \a tagsRequestedByAppli.
//! \param[in] tagsRequestedByAppli Pointer to \a numOfTagsRequestedByAppli unsigned long values.
//! It represents the tags that the application wants to have their values.
//! Values of these tags will be given back into \a tagsKernelToAppli.
//! Can be \a NULL if \a numOfTagsRequestedByAppli is 0.
//! \param[out] tagsKernelToAppli If not \a NULL, the function creates a new TlvTree object and fill it
//! with the tags and values requested by the application using \a tagsRequestedByAppli.
//! If no tag is requested by the application or if the EMV kernel does not know any value of the requested tag,
//!  no TlvTree is created (so value is set to \a NULL).
//! \return  Can return any value of \ref EMV_Status_t.
//! note The application is in charge to destroy the created TlvTree \a tagsKernelToAppli.
EMV_Status_t EMV_GetTags(EMV_Object_t object, int numOfTagsRequestedByAppli,
		const unsigned long *tagsRequestedByAppli, TLV_TREE_NODE *tagsKernelToAppli);



//! \brief Send the READ RECORD commands required to read the transaction log of the card.
//! \param[in] object The EMV transaction object to use.
//! \param[in] tagsAppliToKernel Contains any tag that the application wants to give to the EMV kernel.
//! Can be \a NULL if the application has no tag to give.
//! \param[in] numOfTagsRequestedByAppli Number of tags pointed by \a tagsRequestedByAppli.
//! \param[in] tagsRequestedByAppli Pointer to \a numOfTagsRequestedByAppli unsigned long values.
//! It represents the tags that the application wants to have their values.
//! Values of these tags will be given back into \a tagsKernelToAppli.
//! Can be \a NULL if \a numOfTagsRequestedByAppli is 0.
//! \param[out] tagsKernelToAppli If not \a NULL, the function creates a new TlvTree object and fill it
//! with \ref TAG_EMV_INT_TRANSACTION_LOG_ENTRY tags that contains the transaction log information.
//! It is also filled with the tags and values requested by the application using \a tagsRequestedByAppli.
//! \return  Can return any value of \ref EMV_Status_t.
//! note The application is in charge to destroy the created TlvTree \a tagsKernelToAppli.
EMV_Status_t EMV_ReadTransactionLog(EMV_Object_t object, TLV_TREE_NODE tagsAppliToKernel,
		unsigned int numOfTagsRequestedByAppli, const unsigned long *tagsRequestedByAppli,
		TLV_TREE_NODE *tagsKernelToAppli);
//! \}

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
