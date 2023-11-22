/**
* \file
* \brief This file lists all the EMV Level 2 status codes.
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

#ifndef __EMV_STATUS_H__
//! \cond
#define __EMV_STATUS_H__
//! \endcond

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//! \addtogroup EMVAPI_Status
//! \{

//! \brief EMV status codes.
typedef enum
{
	EMV_STATUS_SUCCESS									= 0,	//!< The operation was performed successfully.
	EMV_STATUS_INVALID_OBJECT							= 1,	//!< The given object is not a valid one.
	EMV_STATUS_CARD_READER_UNKOWN						= 2,	//!< The given card reader in unknown.
	EMV_STATUS_NOT_ENOUGH_MEMORY						= 3,	//!< There is not enough memory to perform the operation.
	EMV_STATUS_UNEXPECTED_ERROR							= 4,	//!< An unexpected error occurred. It shall not happen.
	EMV_STATUS_NOT_ALLOWED								= 5,	//!< The given object has been created by another application and cannot be used.
	EMV_STATUS_INVALID_PARAMETER						= 6,	//!< At least one of the input parameter or tag is not valid.
	EMV_STATUS_COMPONENT_ERROR							= 7,	//!< Cannot communication with the EMV API component.

	EMV_STATUS_CANCEL									= 8,	//!< Operation canceled.
	EMV_STATUS_CARD_PROCESSING_ERROR					= 9,	//!< Processing error.
	EMV_STATUS_CARD_DATA_ERROR							= 10,	//!< Error on a card data.
	EMV_STATUS_CARD_BLOCKED								= 11,	//!< The card is blocked.
	EMV_STATUS_CARD_REMOVED								= 12,	//!< The card has been removed.
	EMV_STATUS_SERVICE_NOT_ALLOWED						= 13,	//!< The service is not allowed.
	EMV_STATUS_INVALID_CARD								= 14,	//!< The card is not a valid one.
	EMV_STATUS_SELECT_ANOTHER_AID						= 15,	//!< Another AID must be selected.

	EMV_STATUS_KERNEL_NOT_FOUND							= 16,	//!< The EMV DC kernel is not loaded.
	EMV_STATUS_KERNEL_UNKNOWN_SERVICE					= 17,	//!< A service of the EMV DC kernel is not found.
	EMV_STATUS_KERNEL_ERROR								= 18,	//!< Cannot communicate with the EMV DC kernel.
	EMV_STATUS_KERNEL_INVALID_ANSWER					= 19,	//!< The EMV DC kernel returned an invalid answer.

	EMV_STATUS_TRANSACTION_STEP_INVALID					= 20,	//!< The requested transaction step is not valid.
	EMV_STATUS_TRANSACTION_STEP_ALREADY_PERFORMED		= 21,	//!< The requested transaction step has already been executed.
	EMV_STATUS_TRANSACTION_STEP_MANDATORY_NOT_PERFORMED	= 22,	//!< Cannot execute the requested transaction step because a mandatory step has not been executed.

	EMV_STATUS_MISSING_MANDATORY_TERM_DATA				= 23,	//!< A mandatory terminal data is missing.
	EMV_STATUS_INVALID_DATA_OBJECT_LIST					= 24,	//!< A DOL is invalid.

	EMV_STATUS_OUTPUT_BUFFER_TOO_SMALL					= 25,		//!< Internal error. Application shall not get it.
	EMV_STATUS_UNKNOWN									= 26,		//!< Unknown error.
	EMV_STATUS_CARD_ANSWER								= 27,		//!< A card answer is not valid.
	EMV_STATUS_VISA_EASY_ENTRY_CARD                     = 28,       //!< Card is Visa Easy Entry type

	EMV_STATUS_LIB_INTERFACE_ERROR                     	= 29,       //!< Error when exchanging message with the EmvApi service.

	EMV_STATUS_2NDGENAC_RESULT_AAC						= 100,		//!< 2ND-GENAC result is AAC.
	EMV_STATUS_OFFLINE_AUTHED							= 101,		//!< Offline authentication.
	EMV_STATUS_ONLINE_DECLINED							= 102,		//!< Online transaction declined.

	EMV_STATUS_LAST													//!< Last EMV status code. Just informational, it is not a valid status code.
} EMV_Status_t;

//! \}

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

//// Variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
