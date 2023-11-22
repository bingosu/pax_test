/**
* \file
* \brief EMV tags definition.
*
* \author ------------------------------------------------------------------------------\n
* \author INGENICO Technical Software Department\n
* \author ------------------------------------------------------------------------------\n
* \author Copyright (c) 2009 - 2014 INGENICO.\n
* \author 28-32 boulevard de Grenelle 75015 Paris, France.\n
* \author All rights reserved.\n
* \author This source program is the property of the INGENICO Company mentioned above\n
* \author and may not be copied in any form or by any means, whether in part or in whole,\n
* \author except under license expressly granted by such INGENICO company.\n
* \author All copies of this source program, whether in part or in whole, and\n
* \author whether modified or not, must display this and all other\n
* \author embedded copyright and ownership notices in full.\n
**/

#ifndef __EMVLIB_TAGS_H__INCLUDED__
//! \cond
#define __EMVLIB_TAGS_H__INCLUDED__
//! \endcond

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//! \addtogroup EmvDefinedTags
//! @{

#define TAG_EMV_ISSUER_IDENTIFICATION_NB			0x42			/*!< EMV - Issuer Identification Number (IIN).<br>The number that identifies the major industry and the card issuer and that forms the first part of the Primary Account Number (PAN).<br>	- Format : n 6.<br>	- Length : 3 bytes.<br>	- Source : Card. */
#define TAG_EMV_AID_CARD							0x4F			/*!< EMV - Application Identifier (AID) - card.<br>Identifies the application as described in <i>ISO/IEC 7816-5</i>.<br>	- Format : b.<br>	- Length : 5-16 bytes.<br>	- Source : Card. */
#define TAG_EMV_APPLICATION_LABEL					0x50			/*!< EMV - Application label.<br>Mnemonic associated with the AID according to <i>ISO/IEC 7816-5</i>.<br>	- Format : ans with the special character limited to space.<br>	- Length : 1-16 bytes.<br>	- Source : Card. */
#define TAG_EMV_TRACK_2_EQU_DATA					0x57			/*!< EMV - Track 2 Equivalent Data.<br>Contains the data elements of track 2 according to ISO/IEC 7813, excluding start sentinel, end sentinel, and Longitudinal Redundancy Check (LRC).<br>	- Format : b.<br>	- Length : var. up to 19 bytes.<br>	- Source : Card. */
#define TAG_EMV_APPLI_PAN							0x5A			/*!< EMV - Application Primary Account Number (PAN).<br>Valid cardholder account number.<br>	- Format : cn var. up to 19 digits..<br>	- Length : var. up to 10 bytes.<br>	- Source : Card. */
#define TAG_EMV_APPLI_TEMPLATE						0x61			/*!< EMV - Application Template.<br>Contains one or more data objects relevant to an application directory entry according to <i>ISO/IEC 7816-5</i>.<br>	- Format : b.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_FCI_TEMPLATE						0x6F			/*!< EMV - File Control Information (FCI) Template.<br>Identifies the FCI template according to <i>ISO/IEC 7816-4</i>.<br>	- Format : var.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_READ_RECORD_RESP_MESSAGE			0x70			/*!< EMV - READ RECORD Response Message.<br>Contains the contents of the record read. (Mandatory for SFIs 1-10. Response messages for SFIs 11-30 are outside the scope of EMV, but may use template '70').<br>	- Format : var.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_ISSUER_SCRIPT_TEMPLATE_1			0x71			/*!< EMV - Issuer Script Template 1.<br>Contains proprietary issuer data for transmission to the ICC before the second GENERATE AC command.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Issuer. */
#define TAG_EMV_ISSUER_SCRIPT_TEMPLATE_2			0x72			/*!< EMV - Issuer Script Template 2.<br>Contains proprietary issuer data for transmission to the ICC after the second GENERATE AC command.<br>	- Format : var.<br>	- Length : var.<br>	- Source : Issuer. */
#define TAG_EMV_DICTIONARY_DISCR_TEMPLATE			0x73			/*!< EMV - Directory Discretionary Template.<br>Issuer discretionary part of the directory according to <i>ISO/IEC 7816-5</i>.<br>	- Format : var.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_RESPONSE_MESSAGE_TEMPLATE_FORMAT_2	0x77			/*!< EMV - Response Message Template Format 2.<br>Contains the data objects (with tags and lengths) returned by the ICC in response to a command.<br>	- Format : var.<br>	- Length : var.<br>	- Source : Card. */
#define TAG_EMV_RESPONSE_MESSAGE_TEMPLATE_FORMAT_1	0x80			/*!< EMV - Response Message Template Format 1.<br>Contains the data objects (without tags and lengths) returned by the ICC in response to a command.<br>	- Format : var.<br>	- Length : var.<br>	- Source : Card. */
#define TAG_EMV_AMOUNT_AUTH_BIN						0x81			/*!< EMV - Amount, Authorised (Binary).<br>Authorised amount of the transaction (excluding adjustments).<br>	- Format : b.<br>	- Length : 4 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_AIP									0x82			/*!< EMV - Application Interchange Profile.<br>Indicates the capabilities of the card to support specific functions in the application.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Card. */
#define TAG_EMV_COMMAND_TEMPLATE					0x83			/*!< EMV - Command Template.<br>Identifies the data field of a command message.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Terminal. */
#define TAG_EMV_DF_NAME								0x84			/*!< EMV - Dedicated File (DF) Name.<br>Identifies the name of the DF as described in <i>ISO/IEC 7816-4</i>.<br>	- Format : b.<br>	- Length : 5-16 bytes.<br>	- Source : Card. */
#define TAG_EMV_ISSUER_SCRIPT_COMMAND				0x86			/*!< EMV - Issuer Script Command.<br>Contains a command for transmission to the ICC.<br>	- Format : b.<br>	- Length : var. up to 261 bytes.<br>	- Source : Issuer. */
#define TAG_EMV_APPLI_PRIORITY_INDICATOR			0x87			/*!< EMV - Application Priority Indicator.<br>Indicates the priority of a given application or group of applications in a directory.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Card. */
#define TAG_EMV_SFI									0x88			/*!< EMV - Short File Identifier (SFI).<br>Identifies the AEF referenced in commands related to a given ADF or DDF. It is a binary data object having a value in the range 1 to 30 and with the three high order bits set to zero.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Card. */
#define TAG_EMV_AUTHORISATION_CODE					0x89			/*!< EMV - Authorisation Code.<br>Value generated by the authorisation authority for an approved transaction.<br>	- Format : As defined by the Payment Systems.<br>	- Length : 6 bytes.<br>	- Source : Issuer. */
#define TAG_EMV_AUTHORISATION_RESPONSE_CODE			0x8A			/*!< EMV - Authorisation Response Code.<br>Code that defines the disposition of a message.<br>	- Format : an 2.<br>	- Length : 2 bytes.<br>	- Source : Issuer/Terminal. */
#define TAG_EMV_CDOL_1								0x8C			/*!< EMV - Card Risk Management Data Object List 1 (CDOL1).<br>List of data objects (tag and length) to be passed to the ICC in the first GENERATE AC command.<br>	- Format : b.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_CDOL_2								0x8D			/*!< EMV - Card Risk Management Data Object List 2 (CDOL2).<br>List of data objects (tag and length) to be passed to the ICC in the second GENERATE AC command.<br>	- Format : b.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_CVM_LIST_CARD						0x8E			/*!< EMV - Cardholder Verification Method (CVM) List.<br>Identifies a method of verification of the cardholder supported by the application.<br>	- Format : b.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD			0x8F			/*!< EMV - Certification Authority Public Key Index.<br>Identifies the certification authority�s public key in conjunction with the RID.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Card. */
#define TAG_EMV_ISSUER_PUBLIC_KEY_CERTIFICATE		0x90			/*!< EMV - Issuer Public Key Certificate.<br>Issuer public key certified by a certification authority.<br>	- Format : b.<br>	- Length : NCA.<br>	- Source : Card. */
#define TAG_EMV_ISSUER_AUTHENTICATION_DATA			0x91			/*!< EMV - Issuer Authentication Data.<br>Data sent to the ICC for online issuer authentication.<br>	- Format : b.<br>	- Length : 8-16 bytes.<br>	- Source : Issuer. */
#define TAG_EMV_ISSUER_PUBLIC_KEY_REMAINDER			0x92			/*!< EMV - Issuer Public Key Remainder.<br>Remaining digits of the Issuer Public Key Modulus.<br>	- Format : b.<br>	- Length : NI - NCA + 36.<br>	- Source : Card. */
#define TAG_EMV_SIGNED_STATIC_APPLI_DATA			0x93			/*!< EMV - Signed Static Application Data.<br>Digital signature on critical application parameters for SDA.<br>	- Format : b.<br>	- Length : NI.<br>	- Source : Card. */
#define TAG_EMV_AFL									0x94			/*!< EMV - Application File Locator (AFL).<br>Indicates the location (SFI, range of records) of the AEFs related to a given application.<br>	- Format : b.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_TVR									0x95			/*!< EMV - Terminal Verification Results.<br>Status of the different functions as seen from the terminal.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_TDOL								0x97			/*!< EMV - Transaction Certificate Data Object List (TDOL).<br>List of data objects (tag and length) to be used by the terminal in generating the TC Hash Value.<br>	- Format : b.<br>	- Length : var. up to 252 bytes.<br>	- Source : Card. */
#define TAG_EMV_TC_HASH_VALUE						0x98			/*!< EMV - Transaction Certificate (TC) Hash Value.<br>Result of a hash function specified in <i>EMV 4.2 Book 2, Annex B3.1</i>.<br>	- Format : b.<br>	- Length : 20 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_TRANSACTION_PIN_DATA				0x99			/*!< EMV - Transaction Personal Identification Number (PIN) Data.<br>Data entered by the cardholder for the purpose of the PIN verification.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Terminal. */
#define TAG_EMV_TRANSACTION_DATE					0x9A			/*!< EMV - Transaction Date.<br>Local date that the transaction was authorised.<br>	- Format : n 6 (YYMMDD).<br>	- Length : 3 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_TSI									0x9B			/*!< EMV - Transaction Status Information.<br>Indicates the functions performed in a transaction.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_TRANSACTION_TYPE					0x9C			/*!< EMV - Transaction Type.<br>Indicates the type of financial transaction, represented by the first two digits of the <i>ISO 8583:1987</i> Processing Code. The actual values to be used for the Transaction Type data element are defined by the relevant payment system.<br>See also \ref TAG_EMV_INT_TRANSACTION_TYPE.<br>	- Format : n 2.<br>	- Length : 1 byte.<br>	- Source : Terminal. */
#define TAG_EMV_DDF_NAME							0x9D			/*!< EMV - Directory Definition File (DDF) Name.<br>Identifies the name of a DF associated with a directory.<br>	- Format : b.<br>	- Length : 5-16 bytes.<br>	- Source : Card. */
#define TAG_EMV_FCI_PROPRIETARY_TEMPLATE			0xA5			/*!< EMV - File Control Information (FCI) Proprietary Template.<br>Identifies the data object proprietary to this specification in the FCI template according to <i>ISO/IEC 7816-4</i>.<br>	- Format : var.<br>	- Length : var.<br>	- Source : Card. */
#define TAG_EMV_CARDHOLDER_NAME						0x5F20			/*!< EMV - Cardholder Name.<br>Indicates cardholder name according to <i>ISO 7813</i>.<br>	- Format : ans 2-26.<br>	- Length : 2-26 bytes.<br>	- Source : Card. */
#define TAG_EMV_APPLI_EXPIRATION_DATE				0x5F24			/*!< EMV - Application Expiration Date.<br>Date after which application expires.<br>	- Format : n 6 (YYMMDD).<br>	- Length : 3 bytes.<br>	- Source : Card. */	
#define TAG_EMV_APPLI_EFFECTIVE_DATE				0x5F25			/*!< EMV - Application Effective Date.<br>Date from which the application may be used.<br>	- Format : n 6 (YYMMDD).<br>	- Length : 3 bytes.<br>	- Source : Card. */	
#define TAG_EMV_ISSUER_COUNTRY_CODE					0x5F28			/*!< EMV - Issuer Country Code.<br>Indicates the country of the issuer according to <i>ISO 3166</i><br>	- Format : n 3.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_TRANSACTION_CURRENCY_CODE			0x5F2A			/*!< EMV - Transaction Currency Code.<br>Indicates the currency code of the transaction according to <i>ISO 4217</i>.<br>	- Format : n 3.<br>	- Length : 2 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_LANGUAGE_PREFERENCE					0x5F2D			/*!< EMV - Language Preference.<br>1-4 languages stored in order of preference, each represented by 2 alphabetical characters according to <i>ISO 639</i><br>Note: EMVCo strongly recommends that cards be personalised with data element '5F2D' coded in lowercase, but that terminals accept the data element whether it is coded in upper or lower case.<br>	- Format : an 2.<br>	- Length : 2-8 bytes.<br>	- Source : Card. */	
#define TAG_EMV_SERVICE_CODE						0x5F30			/*!< EMV - Service Code.<br>Service code as defined in <i>ISO/IEC 7813</i> for track 1 and track 2.<br>	- Format : n 3.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER			0x5F34			/*!< EMV - Application Primary Account Number (PAN) Sequence Counter.<br>Identifies and differentiates cards with the same PAN.<br>	- Format : n 2.<br>	- Length : 1 byte.<br>	- Source : Card. */	
#define TAG_EMV_TRANSACTION_CURRENCY_EXPONENT		0x5F36			/*!< EMV - Tranasction Currency Exponent.<br>Indicates the implied position of the decimal point from the right of the transaction amount represented according to <i>ISO 4217</i>.<br>	- Format : n 1.<br>	- Length : 1 byte.<br>	- Source : Terminal. */	
#define TAG_EMV_ISSUER_URL							0x5F50			/*!< EMV - Issuer URL.<br>The URL provides the location of the Issuer�s Library Server on the Internet.<br>	- Format : ans.<br>	- Length : var.<br>	- Source : Card. */	
#define TAG_EMV_IBAN								0x5F53			/*!< EMV - International Bank Account Number (IBAN).<br>Uniquely identifies the account of a customer at a financial institution as defined in <i>ISO 13616</i>.<br>	- Format : var.<br>	- Length : var. up to 34 bytes.<br>	- Source : Card. */
#define TAG_EMV_BANK_IDENTIFIER_CODE				0x5F54			/*!< EMV - Bank Identifier Code (BIC).<br>Uniquely identifies a bank as defined in <i>ISO 9362</i>.<br>	- Format : var.<br>	- Length : 8 or 11 bytes.<br>	- Source : Card. */
#define TAG_EMV_ISSUER_COUNTRY_CODE_A2_FORMAT		0x5F55			/*!< EMV - Issuer Country Code (alpha2 format).<br>Indicates the country of the issuer as defined in <i>ISO 3166</i> (using a 2 character alphabetic code).<br>	- Format : a 2.<br>	- Length : 2 bytes.<br>	- Source : Card. */
#define TAG_EMV_ISSUER_COUNTRY_CODE_A3_FORMAT		0x5F56			/*!< EMV - Issuer Country Code (alpha3 format).<br>Indicates the country of the issuer as defined in <i>ISO 3166</i> (using a 3 character alphabetic code).<br>	- Format : a 3.<br>	- Length : 3 bytes.<br>	- Source : Card. */	
#define TAG_EMV_ACCOUNT_TYPE						0x5F57			/*!< EMV - Account Type.<br>Indicates the type of account selected on the terminal, coded as specified in <i>EMV 4.2 Book 3, Annex G</i>.<br>	- Format : n 2.<br>Length : 1 byte.<br>Source : Terminal. */	
#define TAG_EMV_ACQUIRER_IDENTIFIER					0x9F01			/*!< EMV - Acquirer Identifier.<br>Uniquely identifies the acquirer within each payment system.<br>	- Format : n 6-11.<br>Length : 6 bytes.<br>Source : Terminal. */	
#define TAG_EMV_AMOUNT_AUTH_NUM						0x9F02			/*!< EMV - Amount, Authorised (Numeric).<br>Authorised amount of the transaction (excluding adjustments).<br>	- Format : n 12.<br>	- Length : 6 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_AMOUNT_OTHER_NUM					0x9F03			/*!< EMV - Amount, Other (Numeric).<br>Secondary amount associated with the transaction representing a cashback amount.<br>	- Format : n 12.<br>	- Length : 6 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_AMOUNT_OTHER_BIN					0x9F04			/*!< EMV - Amount, Other (Binary).<br>Secondary amount associated with the transaction representing a cashback amount.<br>	- Format : b.<br>	- Length : 4 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_APPLI_DISCRET_DATA					0x9F05			/*!< EMV - Application Discretionary Data.<br>Issuer or payment system specified data relating to the application.<br>	- Format : b.<br>	- Length : 1-32 bytes.<br>	- Source : Card. */	
#define TAG_EMV_AID_TERMINAL						0x9F06			/*!< EMV - Application Identifier (AID) - terminal.<br>Identifies the application as described in <i>ISO/IEC 7816-5</i>.<br>	- Format : b.<br>	- Length : 5-16 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_APPLI_USAGE_CONTROL					0x9F07			/*!< EMV - Application Usage Control.<br>Indicates issuer�s specified restrictions on the geographic usage and services allowed for the application.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_APPLI_VERSION_NUMBER_CARD			0x9F08			/*!< EMV - Application Version Number - card.<br>Version number assigned by the payment system for the application.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_APPLI_VERSION_NUMBER_TERM			0x9F09			/*!< EMV - Application Version Number - terminal.<br>Version number assigned by the payment system for the application.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_CARDHOLDER_NAME_EXTENDED			0x9F0B			/*!< EMV - Cardholder Name Extended.<br>Indicates the whole cardholder name when greater than 26 characters using the same coding convention as in <i>ISO 7813</i>.<br>	- Format : ans 27-45.<br>	- Length : 27-45 bytes.<br>	- Source : Card. */	
#define TAG_EMV_IAC_DEFAULT							0x9F0D			/*!< EMV - Issuer Action Code - Default.<br>Specifies the issuer�s conditions that cause a transaction to be rejected if it might have been approved online, but the terminal is unable to process the transaction online.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Card. */	
#define TAG_EMV_IAC_DENIAL							0x9F0E			/*!< EMV - Issuer Action Code - Denial.<br>Specifies the issuer�s conditions that cause the denial of a transaction without attempt to go online.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Card. */	
#define TAG_EMV_IAC_ONLINE							0x9F0F			/*!< EMV - Issuer Action Code - Online.<br>Specifies the issuer�s conditions that cause a transaction to be transmitted online.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Card */	
#define TAG_EMV_ISSUER_APPLI_DATA					0x9F10			/*!< EMV - Issuer Application Data.<br>Contains proprietary application data for transmission to the issuer in an online transaction. Note: For CCD-compliant applications, Annex C, section C7 defines the specific coding of the Issuer Application Data (IAD). To avoid potential conflicts with CCD-compliant applications, it is strongly recommended that the IAD data element in an application that is not CCD-compliant should not use the coding for a CCD-compliant application.<br>	- Format : b.<br>	- Length : var. up to 32 bytes.<br>	- Source : Card. */	
#define TAG_EMV_ISSUER_CODE_TABLE_INDEX				0x9F11			/*!< EMV - Issuer Code Table Index.<br>Indicates the code table according to <i>ISO/IEC 8859</i> for displaying the Application Preferred Name.<br>	- Format : n 2.<br>	- Length : 1 byte.<br>	- Source : Card. */	
#define TAG_EMV_APPLI_PREFERRED_NAME				0x9F12			/*!< EMV - Application Preferred Name.<br>Preferred mnemonic associated with the AID.<br>	- Format : ans.<br>	- Length : 1-16 bytes.<br>	- Source : Card. */

//! \cond
#define TAG_EMV_APPLI_PREFERED_NAME					(TAG_EMV_APPLI_PREFERRED_NAME)  /*!< Obsolete, use \ref TAG_EMV_APPLI_PREFERRED_NAME */
//! \endcond

#define TAG_EMV_LAST_ONLINE_ATC_REGISTER			0x9F13			/*!< EMV - Last Online Application Transaction Counter (ATC) Register.<br>ATC value of the last transaction that went online.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_LOWER_CONSECUTIVE_OFFLINE_LIMIT		0x9F14			/*!< EMV - Lower Consecutive Offline Limit.<br>Issuer-specified preference for the maximum number of consecutive offline transactions for this ICC application allowed in a terminal with online capability.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Card. */	
#define TAG_EMV_MERCHANT_CATEGORY_CODE				0x9F15			/*!< EMV - Merchant Category Code.<br>Classifies the type of business being done by the merchant, represented according to <i>ISO 8583:1993</i> for Card Acceptor Business Code.<br>	- Format : n 4.<br>	- Length : 2 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_MERCHANT_IDENTIFIER					0x9F16			/*!< EMV - Merchant Identifier.<br>When concatenated with the Acquirer Identifier, uniquely identifies a given merchant.<br>	- Format : ans 15.<br>	- Length : 15 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_PIN_TRY_COUNTER						0x9F17			/*!< EMV - Personal Identification Number (PIN) Try Counter.<br>Number of PIN tries remaining.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Card. */	
#define TAG_EMV_ISSUER_SCRIPT_IDENTIFIER			0x9F18			/*!< EMV - Issuer Script Identifier.<br>Identification of the Issuer Script.<br>	- Format : b.<br>	- Length : 4 bytes.<br>	- Source : Issuer. */	
#define TAG_EMV_TERMINAL_COUNTRY_CODE				0x9F1A			/*!< EMV - Terminal Country Code.<br>Indicates the country of the terminal, represented according to <i>ISO 3166</i>.<br>	- Format : n 3.<br>	- Length : 2 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_TERMINAL_FLOOR_LIMIT				0x9F1B			/*!< EMV - Terminal Floor Limit.<br>Indicates the floor limit in the terminal in conjunction with the AID.<br>	- Format : b.<br>	- Length : 4 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_TERMINAL_IDENTIFICATION				0x9F1C			/*!< EMV - Terminal Identification.<br>Designates the unique location of a terminal at a merchant.<br>	- Format : an 8.<br>	- Length : 8 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_TERMINAL_RISK_MANAGEMENT_DATA		0x9F1D			/*!< EMV - Terminal Risk Management Data.<br>Application-specific value used by the card for risk management purposes.<br>	- Format : b.<br>	- Length : 1-8 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_IFD_SERIAL_NUMBER					0x9F1E			/*!< EMV - Interface Device (IFD) Serial Number.<br>Unique and permanent serial number assigned to the IFD by the manufacturer.<br>	- Format : an 8.<br>	- Length : 8 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_TRACK_1_DISCRET_DATA				0x9F1F			/*!< EMV - Track 1 Discretionary Data.<br>Discretionary part of track 1 according to <i>ISO/IEC 7813</i>.<br>	- Format : ans.<br>	- Length : var.<br>	- Source : Card. */	
#define TAG_EMV_TRACK_2_DISCRET_DATA				0x9F20			/*!< EMV - Track 2 Discretionary Data.<br>Discretionary part of track 2 according to <i>ISO/IEC 7813</i>.<br>	- Format : cn.<br>	- Length : var.<br>	- Source : Card. */	
#define TAG_EMV_TRANSACTION_TIME					0x9F21			/*!< EMV - Transaction Time.<br>Local time that the transaction was authorised.<br>	- Format : n 6 (HHMMSS).<br>	- Length : 3 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_CA_PUBLIC_KEY_INDEX_TERM			0x9F22			/*!< EMV - Certification Authority Public Key Index.<br>Identifies the certification authority�s public key in conjunction with the RID.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Terminal. */	
#define TAG_EMV_UPPER_CONSECUTIVE_OFFLINE_LIMIT		0x9F23			/*!< EMV - Upper Consecutive Offline Limit.<br>Issuer-specified preference for the maximum number of consecutive offline transactions for this ICC application allowed in a terminal without online capability.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Card. */	
#define TAG_EMV_APPLICATION_CRYPTOGRAM				0x9F26			/*!< EMV - Application Cryptogram.<br>Cryptogram returned by the ICC in response of the GENERATE AC command.<br>	- Format : b.<br>	- Length : 8 bytes.<br>	- Source : Card. */	
#define TAG_EMV_CRYPTOGRAM_INFO_DATA				0x9F27			/*!< EMV - Cryptogram Information Data.<br>Indicates the type of cryptogram and the actions to be performed by the terminal.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Card. */	
#define TAG_EMV_ICC_PIN_ENCIPH_PK_CERTIFICATE		0x9F2D			/*!< EMV - ICC PIN Encipherment Public Key Certificate.<br>ICC PIN Encipherment Public Key certified by the issuer.<br>	- Format : b.<br>	- Length : NI.<br>	- Source : Card. */	
#define TAG_EMV_ICC_PIN_ENCIPH_PK_EXPONENT			0x9F2E			/*!< EMV - ICC PIN Encipherment Public Key Exponent.<br>ICC PIN Encipherment Public Key Exponent used for PIN encipherment.<br>	- Format : b.<br>	- Length : 1 or 3 bytes.<br>	- Source : Card. */	
#define TAG_EMV_ICC_PIN_ENCIPH_PK_REMAINDER			0x9F2F			/*!< EMV - ICC PIN Encipherment Public Key Remainder.<br>Remaining digits of the ICC PIN Encipherment Public Key Modulus.<br>	- Format : b.<br>	- Length : NPE - NI + 42.<br>	- Source : Card. */	
#define TAG_EMV_ISSUER_PK_EXPONENT					0x9F32			/*!< EMV - Issuer Public Key Exponent.<br>Issuer public key exponent used for the verification of the Signed Static Application Data and the ICC Public Key Certificate.<br>	- Format : b.<br>	- Length : 1 to 3 bytes.<br>	- Source : Card. */	
#define TAG_EMV_TERMINAL_CAPABILITIES				0x9F33			/*!< EMV - Terminal Capabilities.<br>Indicates the card data input, CVM, and security capabilities of the terminal.<br>	- Format : b.<br>	- Length : 3 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_CVM_RESULTS							0x9F34			/*!< EMV - Cardholder Verification Method (CVM) Results.<br>Indicates the results of the last CVM performed.<br>	- Format : b.<br>	- Length : 3 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_TERMINAL_TYPE						0x9F35			/*!< EMV - Terminal Type.<br>Indicates the environment of the terminal, its communications capability, and its operational control. Refer to \ref EmvLibTerminalTypes about possible values.<br>	- Format : n 2.<br>	- Length : 1 byte.<br>	- Source : Terminal. */	
#define TAG_EMV_ATC									0x9F36			/*!< EMV - Application Transaction Counter (ATC).<br>Counter maintained by the application in the ICC (incrementing the ATC is managed by the ICC).<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_UNPREDICTABLE_NUMBER				0x9F37			/*!< EMV - Unpredictable Number.<br>Value to provide variability and uniqueness to the generation of a cryptogram.<br>	- Format : b.<br>	- Length : 4 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_PDOL								0x9F38			/*!< EMV - Processing Options Data Object List (PDOL).<br>Contains a list of terminal resident data objects (tags and lengths) needed by the ICC in processing the GET PROCESSING OPTIONS command.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Card. */	
#define TAG_EMV_POS_ENTRY_MODE						0x9F39			/*!< EMV - Point Of Service (POS) Entry Mode.<br>Indicates the method by which the PAN was entered, according to the first two digits of the <i>ISO 8583:1987</i> POS Entry Mode.<br>	- Format : n 2.<br>	- Length : 1 byte.<br>	- Source : Terminal. */	
#define TAG_EMV_AMOUNT_REF_CURRENCY					0x9F3A			/*!< EMV - Amount, Reference Currency.<br>Authorised amount expressed in the reference currency.<br>	- Format : b.<br>	- Length : 4 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_APPLI_REF_CURRENCY					0x9F3B			/*!< EMV - Application Reference Currency.<br>1-4 currency codes used between the terminal and the ICC when the Transaction Currency Code is different from the Application Currency Code; each code is 3 digits according to <i>ISO 4217</i>.<br>	- Format : n 3.<br>	- Length : 2-8 bytes.<br>	- Source : Card. */	
#define TAG_EMV_TRANSACTION_REF_CURRENCY_CODE		0x9F3C			/*!< EMV - Transaction Reference Currency Code.<br>Code defining the common currency used by the terminal in case the Transaction Currency Code is different from the Application Currency Code.<br>	- Format : n 3.<br>	- Length : 2 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_TRANSACTION_REF_CURRENCY_EXPONENT	0x9F3D			/*!< EMV - Transaction Reference Currency Exponent.<br>Indicates the implied position of the decimal point from the right of the transaction amount, with the Transaction Reference Currency Code represented according to <i>ISO 4217</i>.<br>	- Format : n 1.<br>	- Length : 1 byte.<br>	- Source : Terminal. */	
#define TAG_EMV_ADD_TERMINAL_CAPABILITIES			0x9F40			/*!< EMV - Additional Terminal Capabilities.<br>Indicates the data input and output capabilities of the terminal.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_TRANSACTION_SEQUENCE_COUNTER		0x9F41			/*!< EMV - Transaction Sequence Counter.<br>Counter maintained by the terminal that is incremented by one for each transaction.<br>	- Format : n 4-8.<br>	- Length : 2-4 bytes.<br>	- Source : Terminal. */	
#define TAG_EMV_APPLI_CURRENCY_CODE					0x9F42			/*!< EMV - Application Currency Code.<br>Indicates the currency in which the account is managed according to <i>ISO 4217</i>.<br>	- Format : n 3.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_APPLI_REF_CURRENCY_EXPONENT			0x9F43			/*!< EMV - Application Reference Currency Code.<br>Indicates the implied position of the decimal point from the right of the amount, for each of the 1-4 reference currencies represented according to <i>ISO 4217</i>.<br>	- Format : n 1.<br>	- Length : 1-4 bytes.<br>	- Source : Card. */	
#define TAG_EMV_APPLI_CURRENCY_EXPONENT				0x9F44			/*!< EMV - Application Currency Exponent.<br>Indicates the implied position of the decimal point from the right of the amount represented according to <i>ISO 4217</i>.<br>	- Format : n 1.<br>	- Length : 1 byte.<br>	- Source : Card. */	
#define TAG_EMV_DATA_AUTHENTICATION_CODE			0x9F45			/*!< EMV - Data Authentication Code.<br>An issuer assigned value that is retained by the terminal during the verification process of the Signed Static Application Data.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_ICC_PK_CERTIFICATE					0x9F46			/*!< EMV - ICC Public Key Certificate.<br>ICC Public Key certified by the issuer.<br>	- Format : b.<br>	- Length : NI.<br>	- Source : Card. */	
#define TAG_EMV_ICC_PK_EXPONENT						0x9F47			/*!< EMV - ICC Public Key Exponent.<br>ICC Public Key Exponent used for the verification of the Signed Dynamic Application Data.<br>	- Format : b.<br>	- Length : 1 to 3 bytes.<br>	- Source : Card. */	
#define TAG_EMV_ICC_PK_REMAINDER					0x9F48			/*!< EMV - ICC Public Key Remainder.<br>Remaining digits of the ICC Public Key Modulus.<br>	- Format : b.<br>	- Length : NIC - NI + 42.<br>	- Source : Card. */	
#define TAG_EMV_DDOL								0x9F49			/*!< EMV - Dynamic Data Authentication Data Object List (DDOL).<br>List of data objects (tag and length) to be passed to the ICC in the INTERNAL AUTHENTICATE command.<br>	- Format : b.<br>	- Length : up to 252 bytes.<br>	- Source : Card. */	
#define TAG_EMV_SDA_TAG_LIST						0x9F4A			/*!< EMV - Static Data Authentication Tag List.<br>List of tags of primitive data objects defined in this specification whose value fields are to be included in the Signed Static or Dynamic Application Data.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Card. */	
#define TAG_EMV_SIGNED_DYNAMIC_APPLI_DATA			0x9F4B			/*!< EMV - Signed Dynamic Application Data.<br>Digital signature on critical application parameters for DDA or CDA.<br>	- Format : b.<br>	- Length : NIC.<br>	- Source : Card. */	
#define TAG_EMV_ICC_DYNAMIC_NUMBER					0x9F4C			/*!< EMV - ICC Dynamic Number.<br>Time-variant number generated by the ICC, to be captured by the terminal.<br>	- Format : b.<br>	- Length : 2-8 bytes.<br>	- Source : Card. */	
#define TAG_EMV_LOG_ENTRY							0x9F4D			/*!< EMV - Log Entry.<br>Provides the SFI of the Transaction Log file and its number of records.<br>	- Format : b.<br>	- Length : 2 bytes.<br>	- Source : Card. */	
#define TAG_EMV_MERCHANT_NAME_AND_LOCATION			0x9F4E			/*!< EMV - Merchant Name and Location.<br>Indicates the name and location of the merchant.<br>	- Format : ans.<br>	- Length : var.<br>	- Source : Terminal. */	
#define TAG_EMV_LOG_FORMAT							0x9F4F			/*!< EMV - Log Format.<br>List (in tag and length format) of data objects representing the logged data elements that are passed to the terminal when a transaction log record is read.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Card. */	
#define TAG_EMV_FCI_ISSUER_DISCRET_DATA				0xBF0C			/*!< EMV - File Control Information (FCI) Issuer Discretionary Data.<br>Issuer discretionary part of the FCI.<br>	- Format : var.<br>	- Length : var. up to 222 bytes.<br>	- Source : Card. */	

//! @}


//! \addtogroup EmvInternalDefinition
//! @{

/*! EMV - Internal transaction type.<br>
It is used to indicate to the kernel the type of the transaction, so it can adapt its treatments.
There are two ways to define the transaction type:
- Only give the tag \ref TAG_EMV_TRANSACTION_TYPE (EMVCo tag 9C). In such a case, the kernel will analyse its value to determine is the transaction is debit/credit, cash or cashback. It forbids usage of others values than the ISO8583 ones for "debit", "cash" and "cashback".
- Give the tag \ref TAG_EMV_TRANSACTION_TYPE and this tag (\ref TAG_EMV_INT_TRANSACTION_TYPE). In such a case, the EMVCo tag 9C is just forwarded to the card and is not analysed by the kernel.<br>

The tag is formatted as follow:
- Values can be:
	- 0x00: Sale (standard transaction),
	- 0x01: Cash,
	- 0x02: Reversal (only supported by the contactless kernels, not supported by the EMV contact kernel),
	- 0x09: Purchase with cashbash,
	- 0x20: Refund (only supported by the contactless kernels, not supported by the EMV contact kernel).
- Format : b.
- Length : 1 byte.
- Source : Terminal. */
#define TAG_EMV_INT_TRANSACTION_TYPE				0x9F918701

#define TAG_EMV_INT_CAPK_CHECKSUM					0x9F918702		/*!< EMV - Certification Authority Public Key Check Sum.<br>A check value calculated on the concatenation of all parts of the Certification Authority Public Key (RID, Certification Authority Public Key Index, Certification Authority Public Key Modulus, Certification Authority Public Key Exponent) using SHA-1.<br>	- Format : b.<br>	- Length : 20 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_INT_CAPK_EXPONENT					0x9F918703		/*!< EMV - Certification Authority Public Key Exponent.<br>Value of the exponent part of the Certification Authority Public Key.<br>	- Format : b.<br>	- Length : 1 or 3 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_INT_CAPK_MODULUS					0x9F918704		/*!< EMV - Certification Authority Public Key Modulus.<br>Value of the modulus part of the Certification Authority Public Key.<br>	- Format : b.<br>	- Length : NCA (up to 248).<br>	- Source : Terminal. */
#define TAG_EMV_INT_DEFAULT_DDOL					0x9F918705		/*!< EMV - Default DDOL.<br>DDOL to be used for constructing the INTERNAL AUTHENTICATE command if the DDOL in the card is not present.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Terminal. */
#define TAG_EMV_INT_DEFAULT_TDOL					0x9F918706		/*!< EMV - Default TDOL.<br>TDOL to be used for generating the TC Hash Value if the TDOL in the card is not present.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Terminal. */
#define TAG_EMV_INT_MAX_TARGET_PERC_BIASED_RAND_SEL 0x9F918707		/*!< EMV - Maximum Target Percentage to be use for Biased Random Selection.<br>Value used in terminal risk management for random transaction selection.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Terminal. */
#define TAG_EMV_INT_TARGET_PERC_RAND_SEL			0x9F918708		/*!< EMV - Target Percentage to be used for Random Selection.<br>Value used in terminal risk management for random transaction selection.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Terminal. */
#define TAG_EMV_INT_TAC_DEFAULT						0x9F918709		/*!< EMV - Terminal Action Code Default.<br>Specifies the acquirer�s conditions that cause a transaction to be rejected if it might have been approved online, but the terminal is unable to process the transaction online.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_INT_TAC_DENIAL						0x9F91870A		/*!< EMV - Terminal Action Code Denial.<br>Specifies the acquirer�s conditions that cause the denial of a transaction without attempt to go online.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_INT_TAC_ONLINE						0x9F91870B		/*!< EMV - Terminal Action Code Online.<br>Specifies the acquirer�s conditions that cause a transaction to be transmitted online.<br>	- Format : b.<br>	- Length : 5 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_INT_THRESHOLD_VALUE_BIASED_RAND_SEL 0x9F91870C		/*!< EMV - Threshold Value for Biased Random Selection.<br>Value used in terminal risk management for random transaction selection.<br>	- Format : b.<br>	- Length : 4 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_INT_TRANSACTION_REF_CURRENCY_CONV	0x9F91870D		/*!< EMV - Transaction Reference Currency Conversion.<br>Factor used in the conversion from the Transaction Currency Code to the Transaction Reference Currency Code.<br>	- Format : n 8.<br>	- Length : 4 bytes.<br>	- Source : Terminal. */
#define TAG_EMV_INT_ISSUER_SCRIPT_RESULTS			0x9F91870E		/*!< EMV - Issuer Script Results.<br>Indicates the result of the terminal script processing.<br>	- Format : b.<br>	- Length : var.<br>	- Source : Terminal. */
#define TAG_EMV_INT_APPLI_SELECTION_INDICATOR		0x9F91870F		/*!< EMV - Application Selection Indicator.<br>For an application in the ICC to be supported by an application in the terminal, the Application Selection Indicator indicates whether the associated AID in the terminal must match the AID in the card exactly, including the length of the AID, or only up to the length of the AID in the terminal There is only one Application Selection Indicator per AID supported by the terminal.<br>	- Format : b.<br>	- Length : 1 byte.<br>	- Source : Terminal. */
#define TAG_EMV_INT_AUTHORISATION_RESPONSE_CRYPTOGRAM	0x9F918710	/*!< EMV - Authorisation Response Cryptogram.<br>Cryptogram generated by the issuer and used by the card to verify that the response came from the issuer.<br>	- Format : b.<br>	- Length : 4 or 8 bytes.<br>	- Source : Issuer. */

//! @}


/////////////////////////////////////////////////////////////////
//// Types //////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
//// Functions //////////////////////////////////////////////////

//! @}

#ifdef __cplusplus
}
#endif

#endif
