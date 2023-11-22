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

#ifndef __EMV_API_TAGS_H__
//! \cond
#define __EMV_API_TAGS_H__
//! \endcond

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

//// Variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

//9F918C00 -> 9F918C7F
//BF918C00 -> BF918C7F

//! \addtogroup EMVAPI_Tags_ICS
//! \{
//! The values of the following tags must match the ones described in the EMVCo ICS file.
//! Most of the tags have a default value if not specified.

#define TAG_EMV_OPTION_PSE													0x9F918C00		//!< Enable usage of the PSE.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_CARDHOLDER_CONFIRMATION								0x9F918C01		//!< Indicate if the cardholder confirmation is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_PREFERRED_DISPLAY_ORDER								0x9F918C02		//!< Indicate if a preferred display order is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_PARTIAL_AID_SELECTION								0x9F918C03		//!< Indicate if the partial AID selection is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_MULTI_LANGUAGE										0x9F918C04		//!< Indicate if the terminal supports multi-language.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_EMV_LANGUAGE_SELECTION_METHOD						0x9F918C05		//!< Indicate if the EMV language selection method is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_COMMON_CHARACTER_SET									0x9F918C06		//!< Indicate if the common character set is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_MAX_CA_PUBLIC_KEY									0x9F918C07		//!< Indicate the maximum length of the CA Public Key.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_EXPONENTS											0x9F918C08		//!< Indicate the supported CA Public Key exponents.
#define TAG_EMV_OPTION_REVOCATION_OF_IPK_CERTIFICATE						0x9F918C09		//!< Indicate if the revocation of Issuer Public Key certificate is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_CERTIFICATE_REVOCATION_LIST_FORMAT					0x9F918C0A		//!< Format of of the Issuer Public Key certificate revocation list.
#define TAG_EMV_OPTION_MANUAL_ACTION_WHEN_CAPK_LOADING_FAILS				0x9F918C0B		//!< Indicate if a manual operation is needed when a CA Public Key Checksum is wrong.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_CAPK_VERIFIED_WITH_CHECKSUM							0x9F918C0C		//!< Indicate if the CA Public Keys are verified using a SHA-1 checksum.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_BYPASS_PIN_ENTRY										0x9F918C0D		//!< Indicate if PIN Bypass is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_SUBSEQUENT_PIN_BYPASS								0x9F918C0E		//!< Indicate if Subsequent PIN Bypass is required or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_GET_DATA_PIN_TRY_COUNTER								0x9F918C0F		//!< Indicate if a GET DATA command is issued to retrieve the PIN try counter.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_FAIL_CVM												0x9F918C10		//!< Indicate if the "Fail CVM" method is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_AMOUNT_KNOWN_BEFORE_CVM								0x9F918C11		//!< Indicate if the amount is known or not before the CVM processing.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_FLOOR_LIMIT_CHECKING									0x9F918C12		//!< Indicate if the floor limit checking is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_RANDOM_TRANSACTION_SELECTION							0x9F918C13		//!< Indicate if the random transaction selection is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_VELOCITY_CHECKING									0x9F918C14		//!< Indicate if the velocity checking is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_TRANSACTION_LOG										0x9F918C15		//!< Indicate if the transaction log is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_EXCEPTION_FILE										0x9F918C16		//!< Indicate if the exception file is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_TRM_IRRESPECTIVE_OF_AIP								0x9F918C17		//!< Indicate if the Terminal Risk Management irrespective of AIP setting or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_TAC_SUPPORTED										0x9F918C18		//!< Indicate if the Terminal Action Codes are supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_TAC_CAN_BE_CHANGED									0x9F918C19		//!< Indicate if the Terminal Action Codes can be changed or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_TAC_CAN_BE_DELETED									0x9F918C1A		//!< Indicate if the Terminal Action Codes can be deleted/disabled or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_DEFAULT_ACTION_CODES_PRIOR_1ST_GEN_AC				0x9F918C1B		//!< Indicate if an online only terminal processes TAC/IAC-Default prior the first GENERATE AC or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_DEFAULT_ACTION_CODES_AFTER_1ST_GEN_AC				0x9F918C1C		//!< Indicate if an online only terminal processes TAC/IAC-Default Default after the first GENERATE AC or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_TAC_IAC_DEFAULT_SKIPPED_WHEN_UNABLE_TO_GO_ONLINE		0x9F918C1D		//!< Indicate if an offline only terminal skips the TAC/IAC-Default checking and requests for AAC when unable to go online or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_TAC_IAC_NORMAL_PROCESSING_WHEN_UNABLE_TO_GO_ONLINE	0x9F918C1E		//!< Indicate if an offline only terminal processes as normal the TAC/IAC-Default checking when unable to go online or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_CDA_FAILURE_DETECTED_PRIOR_TAA						0x9F918C1F		//!< Indicate if a CDA failure must be detected prior Terminal Action Analysis or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_CDA_MODE												0x9F918C20		//!< Indicate the CDA Mode.<br>Value shall be 0x01, 0x02, 0x03 or 0x04.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_FORCED_ONLINE										0x9F918C21		//!< Indicate if the merchant can force the transaction to be online or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_FORCED_ACCEPTANCE									0x9F918C22		//!< Indicate if the merchant can force the acceptance of the transaction or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_ADVICES												0x9F918C23		//!< Indicate if advices are supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_ISSUER_REFERRALS										0x9F918C24		//!< Indicate if issuer referrals are supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_BATCH_DATA_CAPTURE									0x9F918C25		//!< Indicate if a batch data capture is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_ONLINE_DATA_CAPTURE									0x9F918C26		//!< Indicate if an online data capture is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_POS_ENTRY_MODE										0x9F918C27		//!< Indicate the POS entry mode.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_PIN_PAD												0x9F918C28		//!< Indicate if a pinpad is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_AMOUNT_AND_PIN_ENTERED_ON_SAME_KEYPAD				0x9F918C29		//!< Indicate if the amount and the PIN are entered on the same keypad or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_COMBINED_READER										0x9F918C2A		//!< Indicate if the ICC/magstripe readers are combined or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_COMBINED_READER_MAGSTRIPE_READ_FIRST					0x9F918C2B		//!< In case of a combined reader, indicate if the magstripe is read first or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_ACCOUNT_TYPE_SELECTION								0x9F918C2C		//!< Indicate if the account type selection is supported or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_ON_FLY_SCRIPT_PROCESSING								0x9F918C2D		//!< Indicate if the terminal processes script 'on fly' or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_ISSUER_SCRIPT_DEVICE_LIMIT							0x9F918C2E		//!< Indicate the issuer script device limit.<br>	- Format : b.<br>	- Length : 4.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_INTERNAL_DATE_MANAGEMENT								0x9F918C2F		//!< Indicate if the terminal has an internal date management or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_DEFAULT_DDOL											0x9F918C30		//!< Indicate if the terminal has a default DDOL or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_OPTION_DEFAULT_TDOL											0x9F918C31		//!< Indicate if the terminal has a default TDOL or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.

//! \}

//! \addtogroup EMVAPI_Tags_Internal
//! \{

// Common tags
#define TAG_EMV_INT_COMMANDS_TO_GET											0x9F918C40 		//!< Indicates the commands/response to retrieve when calling the \ref EMV_GetCommands or the \ref EMVAS_GetCommands function.<br>See \ref TAG_EMV_INT_COMMANDS_TO_GET_values for the possible values.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_CARD_COMMAND											0x9F918C41 		//!< Contains the APDU command.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_CARD_RESPONSE											0x9F918C42 		//!< Contains the APDU response.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_CMD_BEGIN_TIME											0x9F918C43 		//!< Contains the absolute tick before the command. Can be used for profiling.<br>	- Format : b.<br>	- Length : 4.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_CMD_END_TIME											0x9F918C44 		//!< Contains the absolute tick after the command. Can be used for profiling.<br>	- Format : b.<br>	- Length : 4.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_L1_DRIVER_ERROR											0x9F918C45 		//!< Contains the response code of EMV Level 1 to the last command. See SDK 30 documentation.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_MODE													0x9F918C46 		//!< Indicate French EMV transaction flow. Disabled by default. Give 0x01 to enable French treatments.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).

#define TAG_EMV_INT_NEXT_STEP												0x9F918C48		//!< Indicates the next step the application wants to execute (optional). If the given step is allowed according to any order allowed by EMVCo, the next step will be the requested one, else it will be the default one. The list of the tags required by the kernel for the next step is set according to this choice. The \ref TAG_EMV_INT_NEXT_STEP tag can be requested in the output tags sent by the kernel.<br>Use this tag to adapt the transaction flow (for example to execute Terminal Risk Management step just after the Read Application Data).<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal or kernel.
#define TAG_EMV_INT_PROPRIETARY_TAGS										0x9F918C49		//!< Enables to give the description of a proprietary tag, in order to be correctly stored, and eventually used by any DOL, but not ignored by the EMV kernel.<br>List of proprietary tag definition, with 9 bytes for each tag :<br> - 4 first bytes : Proprietary tag ID.<br>  - 2 bytes : Minimum length.<br>  - 2 bytes : Maximum length.<br>  - 1 byte : Mask.<br>	- Source : Terminal.<br><br>
//!< Notes<br>
//!<  - the minimum length cannot be set to zero.<br>
//!<  - if minimum length = maximum length it means that the tag value has a fixed length.<br>
//!<  - Mask:
//!< <table><tr><th>B8</th><th>B7</th><th>B6</th><th>B5</th><th>B4</th><th>B3</th><th>B2</th><th>B1</th><th>Meaning</th></tr>
//!< <tr><td>0</td><td>0</td><td>0</td><td></td><td></td><td></td><td></td><td></td><td>Data type Alphanumeric</td></tr>
//!< <tr><td>0</td><td>0</td><td>1</td><td></td><td></td><td></td><td></td><td></td><td>Data type Alphanumeric Special</td></tr>
//!< <tr><td>0</td><td>1</td><td>0</td><td></td><td></td><td></td><td></td><td></td><td>Data type Binary</td></tr>
//!< <tr><td>0</td><td>1</td><td>1</td><td></td><td></td><td></td><td></td><td></td><td>Data type Compressed Numeric</td></tr>
//!< <tr><td>1</td><td>0</td><td>0</td><td></td><td></td><td></td><td></td><td></td><td>Data type Numeric</td></tr>
//!< <tr><td>1</td><td>0</td><td>1</td><td></td><td></td><td></td><td></td><td></td><td>RFU</td></tr>
//!< <tr><td>1</td><td>1</td><td>0</td><td></td><td></td><td></td><td></td><td></td><td>RFU</td></tr>
//!< <tr><td>1</td><td>1</td><td>1</td><td></td><td></td><td></td><td></td><td></td><td>RFU</td></tr>
//!< <tr><td></td><td></td><td></td><td>0</td><td>0</td><td></td><td></td><td></td><td>Source Terminal</td></tr>
//!< <tr><td></td><td></td><td></td><td>0</td><td>1</td><td></td><td></td><td></td><td>Source ICC</td></tr>
//!< <tr><td></td><td></td><td></td><td>1</td><td>0</td><td></td><td></td><td></td><td>Source Issuer</td></tr>
//!< <tr><td></td><td></td><td></td><td>1</td><td>1</td><td></td><td></td><td></td><td>Source Issuer/Terminal</td></tr>
//!< <tr><td></td><td></td><td></td><td></td><td></td><td>x</td><td>x</td><td>x</td><td>RFU</td></tr>
//!< </table>

// Application Selection tags
#define TAG_EMV_INT_AS_ALLOW_BLOCKED_AID									0x9F918C4A		//!< Indicates if the selection of a blocked AID is allowed.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_AS_CARDHOLDER_CONFIRMATION								0x9F918C4B 		//!< Boolean indicating if the cardholder confirmation is requested : 0 : No, 1 : Yes.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_AS_METHOD_USED											0x9F918C4C 		//!< Indicates the method used for application selection.<br>See \ref TAG_EMV_INT_AS_METHOD_USED_values for the possible values.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_AID_STATUS												0x9F918C4D 		//!< Indicates the status of an AID.<br>See \ref TAG_EMV_INT_AID_STATUS_values for the possible values.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).

// Transaction tags

#define TAG_EMV_INT_APPLICATION_LANGUAGES									0x9F918C4E		//!< Contains languages supported by the application. On multiple of 2 bytes represented by 2 alphabetical characters according to ISO 639.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal.
#define TAG_EMV_INT_CARDHOLDER_LANGUAGES									0x9F918C4F		//!< Contains the common languages between the card and the application to be chosen by application. On multiple of 2 bytes represented by 2 alphabetical characters according to ISO 639.<br>	- Format : b.<br>	- Length : 2.<br>	- Source : Terminal (kernel).

// ODA
#define TAG_EMV_INT_ISSUER_PUBLIC_KEY_DATA									0x9F918C50		//!< Contains the retrieved Issuer Public Key Data.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_ISSUER_PUBLIC_KEY_SERIAL_NUMBER							0x9F918C51		//!< Contains the Issuer Public Key Data certificate serial number. Used in the certificate revocation list.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_ICC_PUBLIC_KEY_DATA										0x9F918C52		//!< Contains the retrieved ICC Public Key Data.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_SDA_DATA												0x9F918C53		//!< Contains the plaintext data used for the SDA checking.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_DDA_DATA												0x9F918C54		//!< Contains the plaintext data used for the DDA or CDA checking.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_DATA_AUTHENTICATION_CODE								0x9F918C55		//!< Contains the Data Authentication Code. Computed during the Offline Data Authentication.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_CERTIFICATE_REVOCATION_LIST								0x9F918C56 		//!< A TlvTree node that contains a list of Issuer Public Key Certificate to revoke : \ref TAG_EMV_INT_CERTIFICATE_REVOCATION_LIST_RID, \ref TAG_EMV_INT_CERTIFICATE_REVOCATION_LIST_INDEX, \ref TAG_EMV_INT_CERTIFICATE_REVOCATION_LIST_INDEX..<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal.
#define TAG_EMV_INT_CERTIFICATE_REVOCATION_LIST_RID							0x9F918C57		//!< Contains the RID of an Issuer Public Key Revocation List entry.<br>	- Format : b.<br>	- Length : 5.<br>	- Source : Terminal.
#define TAG_EMV_INT_CERTIFICATE_REVOCATION_LIST_INDEX						0x9F918C58		//!< Contains the index of an Issuer Public Key Revocation List entry.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_CERTIFICATE_REVOCATION_LIST_SERIAL_NB					0x9F918C59		//!< Contains the certificate serial number of an Issuer Public Key Revocation List entry.<br>	- Format : b.<br>	- Length : 3.<br>	- Source : Terminal.

// CV
#define TAG_EMV_INT_CV_RESULT												0x9F918C5A		//!< Indicate the result of the execution of the Cardholder Verification method.<br>See \ref TAG_EMV_INT_CV_RESULT_values for the possible values.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_PIN_ENTRY_LENGTH										0x9F918C5B		//!< Indicate the length of the entered PIN.<br>Value must be between 4 and 12.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_CV_STATUS												0x9F918C5C		//!< Indicate the status of the Cardholder Verification step.<br>See \ref TAG_EMV_INT_CV_STATUS_values for bit masks on the first byte. Byte 2 is always 0x00 (RFU).<br>	- Format : b.<br>	- Length : 2.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_REMAINING_PIN                                           0x9F918C5D      //!< Pin tries remaining.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_CV_PROPRIETARY_METHOD_RESULT							0x9F918C5E		//!< Indicate the result of the execution of the Cardholder Verification Proprietary method.<br>See \ref TAG_EMV_INT_CV_RESULT_values for the possible values.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_CV_PROPRIETARY_SUPPORTED_METHODS						0x9F918C5F		//!< Contains the list of the supported proprietary methods for the cardholder verification.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal.
#define TAG_EMV_INT_CV_METHOD_PERFORMED										0x9F918C60		//!< Indicate the cardholder method that the application has to execute (offline/online PIN entry, ...).<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).

// TRM
#define TAG_EMV_INT_LAST_TRANSACTION_AMOUNT									0x9F918C61		//!< Indicate if the amount of the last transaction performed with this card of this terminal. Both 32 and 64 bits binary numbers are accepted. Use 64 bits numbers for huge amounts (currencies with low value).<br>	- Format : b.<br>	- Length : 4 or 8.<br>	- Source : Terminal.

// TAA
#define TAG_EMV_INT_MERCHANT_FORCED_ONLINE									0x9F918C62		//!< Indicate if the merchant forced the transaction to be online.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_PAN_IN_EXCEPTION_FILE									0x9F918C63		//!< Indicate if the card is in the exception file (black list).<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_TERMINAL_ACTION_ANALYSIS_RESULT							0x9F918C64		//!< Result of the Terminal Action Analysis.<br>Value can be TC, ARQC or AAC.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).

// CAA
#define TAG_EMV_INT_FORCE_ONLINE_AFTER_1ST_GEN_AC							0x9F918C65		//!< Indicate if the transaction must go online even if GENERATE AC answered 6985 or AAC.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_MERCHANT_FORCE_DECLINE									0x9F918C66		//!< Indicate if the merchant forced the transaction to be declined.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_GENERATE_AC_CONTROL_PARAMETER							0x9F918C67		//!< Generate AC Reference Control Parameter sent on the Generate AC command.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_ADVICE_REQUIRED 										0x9F918C68		//!< Indicate to the application if an advice message must be created.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).

// C
#define TAG_EMV_INT_AUTHORISATION_RESULT									0x9F918C69		//!< The result of the authorisation.<br> Value is 0x00 (declined), 0x01 (accepted) or 0x02 (unable to go online).<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.
#define TAG_EMV_INT_MERCHANT_FORCE_ACCEPTANCE								0x9F918C6A		//!< Indicate if the merchant forced the acceptance or not.<br>Value shall be 0x00 or 0x01.<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal.

#define TAG_EMV_INT_TRANSACTION_STATUS										0x9F918C6B		//!< Indicate the status of the transaction.<br>Value is 0x00 (declined) or 0x01 (approved).<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_TAC_IAC_DEFAULT_RESULT									0x9F918C6C		//!< Indicate the result of the TAC/IAC Default analysis.<br>Value is 0x00 (declined) or 0x01 (approved).<br>	- Format : b.<br>	- Length : 1.<br>	- Source : Terminal (kernel).




// Application Selection templates
#define TAG_EMV_INT_AID_ENTRY												0xBF918C00 		//!< A TlvTree node that contains information about an AID.<br>When the source is the terminal it contains the following tags : \ref TAG_EMV_AID_TERMINAL, \ref TAG_EMV_OPTION_PARTIAL_AID_SELECTION.<br>When the source is the kernel it contains the following tags : \ref TAG_EMV_AID_CARD, \ref TAG_EMV_INT_CARD_RESPONSE, \ref TAG_EMV_INT_AID_STATUS, \ref TAG_EMV_INT_AID_STATUS and other data returned by the card during application selection.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal or kernel.
#define TAG_EMV_INT_CMD_ENTRY												0xBF918C01 		//!< A TlvTree node that contains information about commands/responses (\ref TAG_EMV_INT_CARD_COMMAND, \ref TAG_EMV_INT_CARD_RESPONSE, \ref TAG_EMV_INT_CMD_BEGIN_TIME, \ref TAG_EMV_INT_CMD_END_TIME, \ref TAG_EMV_INT_L1_DRIVER_ERROR).<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).
#define TAG_EMV_INT_TRANSACTION_LOG_ENTRY									0xBF918C02 		//!< A TlvTree node that contains information about Transaction Log.<br>	- Format : b.<br>	- Length : variable.<br>	- Source : Terminal (kernel).

//! \}


//! \addtogroup TAG_EMV_INT_AS_METHOD_USED_values
//! \{

// Possible values for tag TAG_EMV_INT_AS_METHOD_USED
#define EMVAS_METHOD_PSE											(0)		//!< The PSE method was used.
#define EMVAS_METHOD_LIST_OF_AIDS									(1)		//!< The List Of AIDs method was used.

//! \}

//! \addtogroup TAG_EMV_INT_COMMANDS_TO_GET_values
//! \{

#define EMVAS_GET_CMD_PSE											(0)		//!< Get the SELECT PSE command/response sent during the application selection process.
#define EMVAS_GET_CMD_PSE_ALL										(1)		//!< Get all the PSE process commands/responses sent during the application selection process.
#define EMVAS_GET_CMD_LIST_OF_AIDS									(2)		//!< Get the List Of AIDs commands/responses sent during the application selection process.
#define EMVAS_GET_CMD_ALL											(3)		//!< Get all the application selection commands/responses sent during the application selection process.
#define EMV_GET_CMD_LAST_COMMAND									(4)		//!< Get the last command/response sent during the transaction process.
#define EMV_GET_CMD_LAST_STEP										(5)		//!< Get the commands/responses of the last called step of a transaction.
#define EMV_GET_CMD_ALL												(6)		//!< Get the all the command/response sent during the transaction process.

//! \}


//! \addtogroup TAG_EMV_INT_AID_STATUS_values
//! \{

#define EMVAS_AID_STATUS_OK											(0)		//!< Status of an AID: OK.
#define EMVAS_AID_STATUS_APPLICATION_BLOCKED						(1)		//!< Status of an AID: application blocked.
#define EMVAS_AID_STATUS_UNKNOWN									(2)		//!< Status of an AID: unknown.

//! \}

//! \addtogroup TAG_EMV_INT_CV_RESULT_values
//! \{

#define EMV_INT_CV_RESULT_SUCCESS									(0)		//!< The method has been successful performed.
#define EMV_INT_CV_RESULT_FAIL										(1)		//!< The method has failed (can only be used for proprietary methods).
#define EMV_INT_CV_RESULT_CANCEL									(2)		//!< The method has been canceled (cancel key, card removed ...).
#define EMV_INT_CV_RESULT_TIMEOUT									(3)		//!< A timeout occurred (usually during PIN entry).
#define EMV_INT_CV_RESULT_BYPASS									(4)		//!< The cardholder bypassed the PIN.
#define EMV_INT_CV_RESULT_PINPAD_OUT_OF_ORDER						(5)		//!< The pinpad is out of order..

//! \}

//! \addtogroup TAG_EMV_INT_CV_STATUS_values
//! \{

#define EMV_INT_CV_STATUS_END										(0x01)	//!< The Cardholder Verification is terminated.
#define EMV_INT_CV_STATUS_SIGNATURE									(0x02)	//!< A signature is requested.
#define EMV_INT_CV_STATUS_ONLINE_PIN								(0x04)	//!< An online PIN entry must be done.
#define EMV_INT_CV_STATUS_OFFLINE_PIN								(0x08)	//!< An offline PIN entry must be done.
#define EMV_INT_CV_STATUS_LAST_ATTEMPT								(0x10)	//!< It is the last attempt of the offline PIN entry.
#define EMV_INT_CV_STATUS_OFFLINE_PIN_OK							(0x20)	//!< The entered offline PIN has been verified and is valid.
#define EMV_INT_CV_STATUS_OFFLINE_PIN_WRONG							(0x40)	//!< The entered offline PIN has been verified and is wrong.
#define EMV_INT_CV_STATUS_PROPRIETARY_METHOD						(0x80)	//!< A proprietary method must be executed by the application.

//! \}

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
