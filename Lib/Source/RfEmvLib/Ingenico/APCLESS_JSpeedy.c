/**
* \author ------------------------------------------------------------------------------\n
* \author INGENICO Technical Software Department\n
* \author ------------------------------------------------------------------------------\n
* \author Copyright (c) 2014 INGENICO.\n
* \author 28-32 boulevard de Grenelle 75015 Paris, France.\n
* \author All rights reserved.\n
* \author This source program is the property of the INGENICO Company mentioned above\n
* \author and may not be copied in any form or by any means, whether in part or in whole,\n
* \author except under license expressly granted by such INGENICO company.\n
* \author All copies of this source program, whether in part or in whole, and\n
* \author whether modified or not, must display this and all other\n
* \author embedded copyright and ownership notices in full.\n
**/

/*
 * APCLESS_JSpeedy.c
 *
 *  Created on: March 5 2021
 *      Edited by: Wilada Achmad
 *	
 *
 */

#include "APCLESS_JSpeedy.h"
void APCLESS_Explicit_DebugTag(T_SHARED_DATA_STRUCT* dataStruct);

static unsigned char gs_RetrievedInfo[500];

//! \brief Fill buffer with specific JCB requirements for transaction.
//! \param[out] pDataStruct Shared exchange structure filled with the specific JCB data.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occurred.
static int __APCLESS_JCB_AddJCBSpecificData(T_SHARED_DATA_STRUCT* dataStruct)
{
    int                     cr;
    object_info_t           ObjectInfo;
    T_KERNEL_TRANSACTION_FLOW_CUSTOM sTransactionFlowCustom;
    unsigned char           StepInterruption[KERNEL_PAYMENT_FLOW_STOP_LENGTH];// Bit field to stop payment flow,
                                                                              // if all bit set to 0 => no stop during payment process
                                                                              // if right bit set to 1 : stop after payment step number 1
    unsigned char           StepCustom[KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH];    // Bit field to custom payment flow,
                                                                              // if all bit set to 0 => no stop during payment process
                                                                              // if right bit set to 1 : stop after payment step number 1
    // Check the input data are correctly provided
    if (dataStruct == NULL)
    {
        return FALSE;
    }

    // Init parameters
    memset(StepInterruption, 0, sizeof(StepInterruption));// Default Value : not stop on process
    memset(StepCustom, 0, sizeof(StepCustom));// Default Value : not stop on process
    ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(), &ObjectInfo);

    // Add a tag indicating where the transaction has to be stopped (default value, i.e. all bytes set to 0, is strongly recommanded)
    cr = GTL_SharedExchange_AddTag(dataStruct, TAG_KERNEL_PAYMENT_FLOW_STOP, KERNEL_PAYMENT_FLOW_STOP_LENGTH, (const unsigned char *)StepInterruption);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return FALSE;
    }

    // Customize steps:
    // - to perform the User Interface Request
    ADD_STEP_CUSTOM(STEP_JCB_REMOVE_CARD, StepCustom);

	// - to deliver the Certification Authority Public Key
    ADD_STEP_CUSTOM(STEP_JCB_GET_CA_PUB_KEY, StepCustom);

#if 0
    if (ClessSample_IsBlackListPresent())
    {
        // - To check if PAN is in the blacklist
        ADD_STEP_CUSTOM(STEP_JCB_EXCEPTION_FILE_CHECK, StepCustom);
    }
#endif

	memcpy ((void*)&sTransactionFlowCustom, (void*)StepCustom, KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH);
    sTransactionFlowCustom.usApplicationType = ObjectInfo.application_type;// Kernel will call this application for customisation
    sTransactionFlowCustom.usServiceId = SERVICE_CUSTOM_KERNEL;// Kernel will call SERVICE_CUSTOM_KERNEL service id for customisation

    cr = GTL_SharedExchange_AddTag(dataStruct, TAG_KERNEL_PAYMENT_FLOW_CUSTOM, sizeof(T_KERNEL_TRANSACTION_FLOW_CUSTOM), (const unsigned char *)&sTransactionFlowCustom);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return FALSE;
    }

    return (TRUE);
}

//! \brief Get the start byte from the Outcome Parameter Set.
//! \param[in] pResultDataStruct Structure containing the C5 kernel output.
//! \param[out] pStart Retrieved start value :
//!     - \a JCB_OPS_START_A if transaction needs to start at A.
//!     - \a JCB_OPS_START_B if transaction needs to start at B.
//!     - \a JCB_OPS_START_C if transaction needs to start at C.
//!     - \a JCB_OPS_START_D if transaction needs to start at D.
//! \return
//!     - \ref TRUE if correctly retrieved.
//!     - \ref FALSE if an error occurred.
static int __APCLESS_JCB_RetrieveStart (T_SHARED_DATA_STRUCT * pResultDataStruct, int * pStart)
{
    int                     nPosition, cr;
    unsigned long           ulReadLength;
    const unsigned char *   pReadValue;


    // Init output data
    *pStart = JCB_OPS_START_NA;

    // Get the Outcome Parameter Set
    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_OUTCOME_PARAMETER_SET, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return (FALSE);
    }
	dmsg("__APCLESS_JCB_RetrieveStart");
	dbuf("TAG_JCB_OUTCOME_PARAMETER_SET", (unsigned char* )pReadValue, ulReadLength);
	
    // Get the start value
    *pStart = pReadValue[JCB_OPS_START_OFFSET];
	
    return (TRUE);
}

//! \brief Get the JCB transaction outcome from the Outcome Parameter Set.
//! \param[in] pResultDataStruct Structure containing the C5 kernel output.
//! \param[out] pTransactionOutcome Retrieved transaction outcome :
//!     - \a JCB_OPS_STATUS_APPROVED if transaction is approved.
//!     - \a JCB_OPS_STATUS_ONLINE_REQUEST if an online authorization is requested.
//!     - \a JCB_OPS_STATUS_DECLINED if the transaction is declined.
//!     - \a JCB_OPS_STATUS_TRY_ANOTHER_INTERFACE transaction should be retried on an other interface.
//!     - \a JCB_OPS_STATUS_END_APPLICATION if the transaction is terminated.
//!     - \a JCB_OPS_STATUS_SELECT_NEXT if next AID shall be selected.
//!     - \a JCB_OPS_STATUS_TRY_AGAIN transaction shall be restarted from the begining..
//! \return
//!     - \ref TRUE if correctly retrieved.
//!     - \ref FALSE if an error occurred.
static int __APCLESS_JCB_RetrieveTransactionOutcome (T_SHARED_DATA_STRUCT * pResultDataStruct, int * pTransactionOutcome)
{
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char * pReadValue;

    // Init position
    nPosition = SHARED_EXCHANGE_POSITION_NULL;

    // Init output data
    if (pTransactionOutcome != NULL)
        *pTransactionOutcome = JCB_OPS_STATUS_END_APPLICATION;

    // Get the Outcome Parameter Set
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_OUTCOME_PARAMETER_SET, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return (FALSE);
    }
	dmsg("__APCLESS_JCB_RetrieveTransactionOutcome");
	dbuf("TAG_JCB_OUTCOME_PARAMETER_SET", (unsigned char* )pReadValue, ulReadLength);
	
    // Get the transaction outcome
    if (pTransactionOutcome != NULL)
        *pTransactionOutcome = pReadValue[JCB_OPS_STATUS_OFFSET];

    return (TRUE);
}

//! \brief Get the card type.
//! \param[in] pResultDataStruct Structure containing the C5 kernel output.
//! \param[out] pCardType Retrieved card type
//!     - \a 0 If card type not found.
//!     - \a 0x8501 for MStripe card.
//!     - \a 0x8502 for MChip card.
//! \return
//!     - \ref TRUE if correctly retrieved.
//!     - \ref FALSE if an error occurred.
static int __APCLESS_JCB_RetrieveCardType (T_SHARED_DATA_STRUCT * pResultDataStruct, unsigned short * pCardType)
{
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char * pReadValue;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;

    if (pCardType != NULL)
        *pCardType = 0;// Default result

    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_KERNEL_CARD_TYPE, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return (FALSE);
    }
	dmsg("__APCLESS_JCB_RetrieveCardType");
	dbuf("TAG_KERNEL_CARD_TYPE", (unsigned char* )pReadValue, ulReadLength);

    // Get the card type
    if (pCardType != NULL)
        *pCardType = (pReadValue[0] << 8) + pReadValue[1];

    return (TRUE);
}

//! \brief Get the CVM to apply (read from the Outcome Parameter Set).
//! \param[in] pResultDataStruct Structure containing the C5 kernel output.
//! \param[out] pCvm Retrieved transaction outcome :
//!     - \a JCB_OPS_CVM_NO_CVM No CVM to be performed.
//!     - \a JCB_OPS_CVM_SIGNATURE if signature shall be performed.
//!     - \a JCB_OPS_CVM_ONLINE_PIN if online PIN shall be performed.
//!     - \a JCB_OPS_CVM_CONFIRMATION_CODE_VERIFIED if confirmation code has been verified.
//!     - \a JCB_OPS_CVM_TO_BE_PERFORMED if confirmation code has to be performed by the reader.

//!     - \a JCB_OPS_CVM_NA if CVM is not applicable to the case.
//! \return
//!     - \ref TRUE if correctly retrieved.
//!     - \ref FALSE if an error occurred.
static int __APCLESS_JCB_RetrieveCvmToApply (T_SHARED_DATA_STRUCT * pResultDataStruct, unsigned char * pCvm)
{
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char * pReadValue;

    // Init position
    nPosition = SHARED_EXCHANGE_POSITION_NULL;

    // Init output data
    if (pCvm != NULL)
        *pCvm = JCB_OPS_CVM_NA;

    // Get the Outcome Parameter Set
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_OUTCOME_PARAMETER_SET, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return (FALSE);
    }
	dmsg("__APCLESS_JCB_RetrieveCvmToApply");
	dbuf("TAG_JCB_OUTCOME_PARAMETER_SET", (unsigned char* )pReadValue, ulReadLength);
    // Get the CVM to apply
    if (pCvm != NULL)
        *pCvm = pReadValue[JCB_OPS_CVM_OFFSET];

    return (TRUE);
}

//! \brief Get the Field Off value (read from the Outcome Parameter Set).
//! \param[in] pResultDataStruct Structure containing the C5 kernel output.
//! \param[out] pFieldOff Retrieved field off value (in units of 100 ms).
//!     - \a JCB_OPS_FIELD_OFF_REQUEST_NA if Field Off is not applicable to the case.
//! \return
//!     - \ref TRUE if correctly retrieved.
//!     - \ref FALSE if an error occurred.
static int __APCLESS_JCB_RetrieveFieldOffValue (T_SHARED_DATA_STRUCT * pResultDataStruct, unsigned char * pFieldOff)
{
    int                     nPosition, cr;
    unsigned long           ulReadLength;
    const unsigned char *   pReadValue;


    // Init output data
    *pFieldOff = JCB_OPS_FIELD_OFF_REQUEST_NA;

    // Get the Outcome Parameter Set
    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_OUTCOME_PARAMETER_SET, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return (FALSE);
    }
	dmsg("__APCLESS_JCB_RetrieveFieldOffValue");
	dbuf("TAG_JCB_OUTCOME_PARAMETER_SET", (unsigned char* )pReadValue, ulReadLength);

    // Get the Field Off value
    *pFieldOff = pReadValue[JCB_OPS_FIELD_OFF_REQUEST_OFFSET];

    return (TRUE);
}

//! \brief Get the JCB message to be displayed on outcome from the UIRD.
//! \param[in] pResultDataStruct Structure containing the C5 kernel output.
//! \param[out] pUirdMessage Retrieved from transaction outcome :
//!     - \a JCB_UIRD_MESSAGE_ID_ENTER_PIN                   - Indicates that the cardholder has to enter his PIN code.
//!     - \a JCB_UIRD_MESSAGE_ID_TRY_AGAIN                   - Indicates that the card has to be presented again.
//!     - \a JCB_UIRD_MESSAGE_ID_APPROVED                    - Indicates that the transaction is approved.
//!     - \a JCB_UIRD_MESSAGE_ID_APPROVED_SIGN               - Indicates that the transaction is approved but signature required.
//!     - \a JCB_UIRD_MESSAGE_ID_DECLINED                    - Indicates that the transaction is declined.
//!     - \a JCB_UIRD_MESSAGE_ID_SEE_PHONE                   - Indicates that the cardholder shall see his phone for instructions.
//!     - \a JCB_UIRD_MESSAGE_ID_AUTHORIZING_PLEASE_WAIT     - Indicates that the cardholder the transaction is sent online for authorization.
//!     - \a JCB_UIRD_MESSAGE_ID_INSERT_CARD                 - Indicates that the cardholder has to insert his card.
//!     - \a JCB_UIRD_MESSAGE_ID_NA Message                  - Indicates that the identifier is not applicable.
//! \param[out] pUirdStatus Retrieved from transaction outcome :
//!     - \a JCB_UIRD_STATUS_NOT_READY                       - Indicates that the reader is ready.
//!     - \a JCB_UIRD_STATUS_IDLE                            - Indicates that the reader is idle.
//!     - \a JCB_UIRD_STATUS_READY_TO_READ                   - Indicates that the reader is ready to read the card.
//!     - \a JCB_UIRD_STATUS_PROCESSING                      - Indicates that the transaction is in progress.
//!     - \a JCB_UIRD_STATUS_CARD_READ_SUCCESSFULLY          - Indicates that the card has been read successfully.
//!     - \a JCB_UIRD_STATUS_PROCESSING_ERROR                - Indicates that an error occured.
//!     - \a JCB_UIRD_STATUS_NA                              - Indicates that the Status for UIRD is not applicable.
//! \return
//!     - \ref TRUE if correctly retrieved.
//!     - \ref FALSE if an error occurred.
static int __APCLESS_JCB_RetrieveUirdMessageOnOutcome (T_SHARED_DATA_STRUCT * pResultDataStruct, int * pUirdMessage, int * pUirdStatus)
{
    int                     nPosition, cr;
    unsigned long           ulReadLength;
    const unsigned char *   pReadValue;
    bool                    bUiRequestPresent;


    // Init output data
    *pUirdMessage = JCB_UIRD_MESSAGE_ID_NA;
    *pUirdStatus  = JCB_UIRD_STATUS_NA;

    // ==============================================================================================================
    // First case: the Outcome Parameter Set is present but the UI request is not present
    // Get the Outcome Parameter Set
    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_OUTCOME_PARAMETER_SET, &ulReadLength, &pReadValue);
    if (cr == STATUS_SHARED_EXCHANGE_OK)
    {
        // Check if a UI request on outcome is present
        bUiRequestPresent = ((pReadValue[JCB_OPS_DATA_PRESENCE_OFFSET] & JCB_OPS_DATA_PRESENCE_MASK_UIR_ON_OUTCOME) == JCB_OPS_DATA_PRESENCE_MASK_UIR_ON_OUTCOME);
        if (!bUiRequestPresent)
            return (TRUE);
    }
	dmsg("__APCLESS_JCB_RetrieveUirdMessageOnOutcome");
	dbuf("TAG_JCB_OUTCOME_PARAMETER_SET", (unsigned char* )pReadValue, ulReadLength);
    // ==============================================================================================================

    // ==============================================================================================================
    // Second case: the UI request should be present
    // Get the UI request on outcome
    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_UIR_ON_OUTCOME_DATA, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return (FALSE);
    }
	dmsg("__APCLESS_JCB_RetrieveUirdMessageOnOutcome");
	dbuf("TAG_JCB_UIR_ON_OUTCOME_DATA", (unsigned char* )pReadValue, ulReadLength);
    // Get the UIRD message
    *pUirdMessage = pReadValue[JCB_UIRD_MESSAGE_ID_OFFSET];
    *pUirdStatus  = pReadValue[JCB_UIRD_STATUS_OFFSET];
    // ==============================================================================================================

    return (TRUE);
}

//! \brief manage an 'end application' outcome.
//! \param[in] pDataStruct Data buffer to be filled and used for JCB transaction.
//! \return
//!     - JCB kernel result.
int __APCLESS_JCB_ManageEndApplication (T_SHARED_DATA_STRUCT * pKernelDataStruct)
{
    unsigned char           ucFieldOff;
    unsigned int            nEndApplicationType;
    int                     nResult;
    int                     nTransactionStart;
    int                     nUirdMessageOnOutcome, nUirdStatus;

    // get field off information
    __APCLESS_JCB_RetrieveFieldOffValue (pKernelDataStruct, &ucFieldOff);

    // Retrieve the start value in the OPS
    __APCLESS_JCB_RetrieveStart (pKernelDataStruct, &nTransactionStart);

    // Get the UIRD message on outcome to be displayed
    __APCLESS_JCB_RetrieveUirdMessageOnOutcome (pKernelDataStruct, &nUirdMessageOnOutcome, &nUirdStatus);

    // identify the end application type
    if (nTransactionStart == JCB_OPS_START_NA)
        // end application without restart
        nEndApplicationType = CLESS_SAMPLE_JCB_END_APPLI;
    else
    {
        // end application with restart. Get the reason of the restart.
        if(nUirdMessageOnOutcome == JCB_UIRD_MESSAGE_ID_TRY_AGAIN)
            // restart is due to  a communication error with the card
            nEndApplicationType = CLESS_SAMPLE_JCB_END_APPLI_RESTART_COMM_ERROR;
        else
            // restart is due to an on-device CVM
            nEndApplicationType = CLESS_SAMPLE_JCB_END_APPLI_RESTART_ON_DEVICE_CVM;
    }

    // manage  end application
    switch (nEndApplicationType)
    {
        case CLESS_SAMPLE_JCB_END_APPLI_RESTART_COMM_ERROR:
#ifndef __SIMU_ICC__
            ClessEmv_CloseDriver();
#endif
            // Transaction shall be restarted from the beginning
            nResult = CLESS_CR_MANAGER_RESTART;
            break;

        case CLESS_SAMPLE_JCB_END_APPLI_RESTART_ON_DEVICE_CVM:
            // manage field off
#ifndef __SIMU_ICC__
            ClessEmv_CloseDriver();
#endif
            if(ucFieldOff != 0)
                Telium_Ttestall(0, ucFieldOff * 10);
            // Transaction shall be restarted
            nResult = CLESS_CR_MANAGER_RESTART_DOUBLE_TAP;
            break;

        case CLESS_SAMPLE_JCB_END_APPLI:
        default:
            nResult = CLESS_CR_MANAGER_END;
		break;
    }

    // Transaction is completed, clear JCB kernel transaction data
    JCB_Clear ();

    // Return result
    return (nResult);
}

void __APCLESS_JCB_GetTrascationTag(T_SHARED_DATA_STRUCT * pResultDataStruct, tMtiMap *tpMap)
{
	int iRet = 0, i = 0;
	unsigned long int ilTag = 0L;
	unsigned long int ilLen = 0L;
	const unsigned char *ucppValue = NULL;

	dmsg("=========================================================================");

	while (1)
	{
		iRet = GTL_SharedExchange_GetNext(pResultDataStruct, &i, &ilTag, &ilLen, &ucppValue);
		if (iRet == STATUS_SHARED_EXCHANGE_END)
		{
			break;
		}
		else if (iRet == STATUS_SHARED_EXCHANGE_OK)
		{
			dmsg("TAG[%X] LEN[%ld]", (UINT)ilTag, ilLen);
			dbuf("TAGDATA", (UCHAR*)ucppValue, (INT)ilLen);

			mtiMapPutBytes(tpMap, (INT)ilTag, (UCHAR*)ucppValue, (INT)ilLen);
		}
	}

	dmsg("=========================================================================");
}

int __APCLESS_JCB_RetrieveUirdMessageOnRestart (T_SHARED_DATA_STRUCT * pResultDataStruct, int * pUirdMessage, int * pUirdStatus)
{
    int                     nPosition, cr;
    unsigned long           ulReadLength;
    const unsigned char *   pReadValue;
    bool                    bUiRequestPresent;


    // Init output data
    *pUirdMessage = JCB_UIRD_MESSAGE_ID_NA;
    *pUirdStatus  = JCB_UIRD_STATUS_NA;

    // ==============================================================================================================
    // First case: the Outcome Parameter Set is present but the UI request is not present
    // Get the Restart Parameter Set
    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_OUTCOME_PARAMETER_SET, &ulReadLength, &pReadValue);
    if (cr == STATUS_SHARED_EXCHANGE_OK)
    {
        // Check if a UI request on restart is present
        bUiRequestPresent = ((pReadValue[JCB_OPS_DATA_PRESENCE_OFFSET] & JCB_OPS_DATA_PRESENCE_MASK_UIR_ON_RESTART) == JCB_OPS_DATA_PRESENCE_MASK_UIR_ON_RESTART);
        if (!bUiRequestPresent)
            return (TRUE);
    }
    // ==============================================================================================================


    // ==============================================================================================================
    // Second case: the UI request should be present
    // Get the UI request on restart
    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_JCB_UIR_ON_RESTART_DATA, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return (FALSE);
    }
    // Get the UIRD message
    *pUirdMessage = pReadValue[JCB_UIRD_MESSAGE_ID_OFFSET];
    *pUirdStatus  = pReadValue[JCB_UIRD_STATUS_OFFSET];
    // ==============================================================================================================

    return (TRUE);
}

//! \brief Get a specific information in a shared buffer.
//! \param[in] pDataStruct
//! \param[out] ulTag
//! \param[out] pInfo
//! \return
//!     - \ref TRUE if correctly retrieved.
//!     - \ref FALSE if an error occurred.

int APCLESS_JCB_Common_RetrieveInfo (T_SHARED_DATA_STRUCT * pDataStruct, unsigned long ulTag, unsigned char ** pInfo)
{
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char * pReadValue;

    if ((pDataStruct == NULL) || (ulTag == 0) || (pInfo == NULL))
    {
        return(FALSE);
    }

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    *pInfo = NULL; // Default result : no information

    cr = GTL_SharedExchange_FindNext(pDataStruct, &nPosition, ulTag, &ulReadLength, &pReadValue);
    if (cr != STATUS_SHARED_EXCHANGE_OK)
    {
        return(FALSE);
    }

    // copy data
    if (ulReadLength>sizeof(gs_RetrievedInfo))
    {
        return(FALSE);
    }
    memcpy(gs_RetrievedInfo, pReadValue, ulReadLength);

    // Get the value
    *pInfo = gs_RetrievedInfo;

    return (TRUE);
}

//! \brief Set the application language with the cardholder language if present, else with the merchant language.
//! \param[in] pPreferredLanguage Indicates the preferred language if already extracted.

void APCLESS_JCB_GuiState_SetCardholderLanguage (unsigned char * pPreferredLanguage)
{
    int                     ret;
    T_SHARED_DATA_STRUCT *  pStruct = NULL;
    unsigned char *         pInfo;
    int                     nKernelIdentifier;

    // Get the used payment scheme
    nKernelIdentifier = APCLESS_ParamTrn_GetCurrentPaymentScheme();

    if (nKernelIdentifier != APCLESS_SCHEME_JSPEEDY)
        return;

    if (pPreferredLanguage == NULL)
    {
        // Init a shared buffer to get the prefered selected language
        pStruct = GTL_SharedExchange_InitShared (128);
        if (pStruct == NULL)
        {
            // An error occurred when creating the shared buffer
            goto End;
        }

        // Add tag in the shared buffer to request it
        ret = GTL_SharedExchange_AddTag (pStruct, TAG_KERNEL_SELECTED_PREFERED_LANGUAGE, 0, NULL);
        if (ret != STATUS_SHARED_EXCHANGE_OK)
        {
            // An error occurred when adding the TAG_KERNEL_SELECTED_PREFERED_LANGUAGE tag in the structure
            goto End;
        }

        // Request data to the kernel
        switch (nKernelIdentifier)
        {
            case APCLESS_SCHEME_JSPEEDY :
                ret = JCB_GetData(pStruct);
                break;
            default:
                ret = KERNEL_STATUS_UNKNOWN;
                //GTL_Traces_TraceDebug("%s : Unknown value for kernel identifier parameter : %x", pcFuncName, nKernelIdentifier);
                goto End;
        }
        if (ret != KERNEL_STATUS_OK)
        {
            // An error occurred when getting data from the kernel
            //GTL_Traces_TraceDebug ("%s : An error occurred when getting data from the kernel (ret = 0x%02X)", pcFuncName, ret);
            goto End;
        }
    }
    else
    {
        pInfo = pPreferredLanguage;
    }

    // Search the tag in the kernel response structure
    if (APCLESS_JCB_Common_RetrieveInfo (pStruct, TAG_KERNEL_SELECTED_PREFERED_LANGUAGE, &pInfo))
    {
/*
    	gs_tTransactionLanguage.nCardholderLanguage = ClessSample_Term_GiveLangNumber(pInfo);
        if (gs_tTransactionLanguage.nCardholderLanguage == -1)
            gs_tTransactionLanguage.nCardholderLanguage = gs_tTransactionLanguage.nMerchantLanguage;
*/
    }

End:
    // Destroy the shared buffer if created
    if (pStruct != NULL)
        GTL_SharedExchange_DestroyShare (pStruct);
}

int __APCLESS_JCB_EndTransaction(void)
{
#ifndef __SIMU_ICC_
    // TODO: demander le retrait carte si carte pr�sente
    // deselect the card and close the driver
    ClessEmv_DeselectCard(0, TRUE, FALSE);
    ClessEmv_CloseDriver();
#endif

    // Transaction is completed, clear JCB kernel transaction data
    JCB_Clear ();

    // Return result
    return (CLESS_CR_MANAGER_END);
}

int __APCLESS_JCB_ManageSelectNext(void)
{
	// Transaction is completed, clear JCB kernel transaction data
    JCB_Clear ();

    // Return result
    return (CLESS_CR_MANAGER_REMOVE_AID);
}

//! \brief manage an approved outcome.
//! \param[in] pDataStruct Data buffer to be filled and used for JCB transaction.
//! \return
//!     - JCB kernel result.
int __APCLESS_JCB_ManageApprovedOutcome (T_SHARED_DATA_STRUCT * pKernelDataStruct)
{
    unsigned char           ucCvm;
    unsigned short          usCardType;

	APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_OFFLINE_APPROVED);

    // Retrieve the card type
    if (!__APCLESS_JCB_RetrieveCardType (pKernelDataStruct, &usCardType))
    {
        usCardType = 0;
    }

    // Get the CVM to perform
    if (!__APCLESS_JCB_RetrieveCvmToApply (pKernelDataStruct, &ucCvm))
    {
        ucCvm = JCB_OPS_CVM_NA;
    }

    // user interface sequence according to the CVM
    switch (ucCvm)
    {
        case JCB_OPS_CVM_SIGNATURE:
			mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_SIGN, NULL, NULL);
            break;
        default:
			break;
	}

    // end transaction
    return (__APCLESS_JCB_EndTransaction());
}

//! \brief manage a 'declined' outcome.
//! \param[in] pDataStruct Data buffer to be filled and used for JCB transaction.
//! \return
//!     - JCB kernel result.
int __APCLESS_JCB_ManageDeclined (void)
{
	APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_OFFLINE_DECLINED);
	
    // Return result
    return (__APCLESS_JCB_EndTransaction());
}

//! \brief manage a 'try another interface' outcome.
//! \param[in] pDataStruct Data buffer to be filled and used for JCB transaction.
//! \return
//!     - JCB kernel result.
int __APCLESS_JCB_ManageTryAnotherInterface (void)
{
	APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_USE_CONTACT);
	
	// Return result
	return (__APCLESS_JCB_EndTransaction());
}

//! \brief manage an 'unknown' outcome.
//! \param[in] pKernelDataStruct Data buffer to be filled and used for JCB transaction.
//! \param[in] nTransactionOutcome.
//! \return
//!     - JCB kernel result.
int __APCLESS_JCB_ManageUnknownOutcome (void)
{	
	APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_ERROR);
	
    // Indicate double tap is not in progress
    APCLESS_ParamTrn_SetDoubleTapInProgress(FALSE);

    return (__APCLESS_JCB_EndTransaction());
}

int APCLESS_JCB_PerformTransaction(T_SHARED_DATA_STRUCT * dataStruct)
{
    int                     nResult = CLESS_CR_MANAGER_END;
    int                     cr, nTransactionOutcome;
    unsigned char           ucCvm;
    int                     nTransactionStart;


    // Indicate JCB kernel is going to be used (for customisation purposes)
	APCLESS_ParamTrn_SetCurrentPaymentScheme(APCLESS_SCHEME_JSPEEDY);

    // Get the JCB
    if (!__APCLESS_JCB_AddJCBSpecificData(dataStruct))
    {
		// clear JCB kernel transaction data
		JCB_Clear ();
		return nResult;
    }

	APCLESS_Explicit_DebugTag(dataStruct);
	
	// Call the JCB kernel to perform the transaction
	cr = JCB_DoTransaction(dataStruct);
	dmsg("JCB_DoTransaction() Result = %04X", cr);
	if ((cr != KERNEL_STATUS_OK)&&
		(cr != KERNEL_STATUS_CANCELLED)&&
		(cr != (KERNEL_STATUS_CANCELLED|KERNEL_STATUS_STOPPED_BY_APPLICATION_MASK)))
	{
		// clear JCB kernel transaction data
		return __APCLESS_JCB_EndTransaction();
	}
	APCLESS_JCB_GuiState_SetCardholderLanguage(NULL);
		
	// ====== JCB Contactless IC Terminal Specification V1.2 ===========================
	// Requirements � Outcome with Restart
	// 3.4.1.3 Requirements 3.4.1.4 through 3.4.1.8 shall be processed if Outcome parameter �Start� has a value other than �N/A�.
	// 3.4.1.4 The Core Reader shall set the Restart flag.
	// ================================================================
	// Retrieve the start value in the OPS
	if (!__APCLESS_JCB_RetrieveStart (dataStruct, &nTransactionStart))
	{
		// clear JCB kernel transaction data
		return __APCLESS_JCB_EndTransaction();
	}
	dmsg("__APCLESS_JCB_RetrieveStart() result = %02X", nTransactionStart);

	// Get the transaction outcome
	// - JCB_OPS_STATUS_APPROVED if transaction is approved.
	// - JCB_OPS_STATUS_ONLINE_REQUEST if an online authorization is requested.
	// - JCB_OPS_STATUS_DECLINED if the transaction is declined.
	// - JCB_OPS_STATUS_TRY_ANOTHER_INTERFACE transaction should be retried on an other interface.
	// - JCB_OPS_STATUS_END_APPLICATION if the transaction is terminated.
	// - JCB_OPS_STATUS_SELECT_NEXT if next AID shall be selected.
	if (!__APCLESS_JCB_RetrieveTransactionOutcome (dataStruct, &nTransactionOutcome))
	{
		nTransactionOutcome = JCB_OPS_STATUS_END_APPLICATION;
	}
	dmsg("__APCLESS_JCB_RetrieveTransactionOutcome() result = %02X", nTransactionOutcome);
	
	switch (nTransactionOutcome)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case JCB_OPS_STATUS_APPROVED:
			// Get all the kernel data
			JCB_GetAllData(dataStruct);
	
			__APCLESS_JCB_GetTrascationTag(dataStruct, &mtiGetRfEmvParmInfo()->tEmvTagMap);
	
			// manage the approved outcome
			nResult = __APCLESS_JCB_ManageApprovedOutcome(dataStruct);
			break;
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case JCB_OPS_STATUS_DECLINED:
			nResult = __APCLESS_JCB_ManageDeclined();
			break;
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case JCB_OPS_STATUS_TRY_ANOTHER_INTERFACE:
			nResult = __APCLESS_JCB_ManageTryAnotherInterface();
			break;
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case JCB_OPS_STATUS_ONLINE_REQUEST:
			// Get all the kernel data to manage the issuer update processing
			JCB_GetAllData(dataStruct);
			__APCLESS_JCB_GetTrascationTag(dataStruct, &mtiGetRfEmvParmInfo()->tEmvTagMap);

			// Get the CVM to be performed
			if (!__APCLESS_JCB_RetrieveCvmToApply (dataStruct, &ucCvm))
			{
				nResult = CLESS_CR_MANAGER_REMOVE_AID;
				break;
			}

			dmsg("OPS CVM = %X", ucCvm);
			
			// Retrieve the start value in the OPS
			if (!__APCLESS_JCB_RetrieveStart (dataStruct, &nTransactionStart))
			{
				nResult = CLESS_CR_MANAGER_REMOVE_AID;
				break;
			}
	
			// Check if the required CVM is Online PIN
			if (ucCvm == JCB_OPS_CVM_ONLINE_PIN)
			{
				mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_PIN_FLAG, NULL, NULL);
			}
			else if(ucCvm == JCB_OPS_CVM_SIGNATURE)
			{
				mtiGetRfEmvParmInfo()->FuncUICallback(MTI_EMV_STEP_NEED_SIGN, NULL, NULL);
				//mtiMapPutInt(mtiGetRfEmvParmInfo()->tpMainProcMap, KEY_CLESS_CVM_LIMIT_NOCVM, FALSE);
			}
			nResult = __APCLESS_JCB_EndTransaction();
			break;
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case JCB_OPS_STATUS_SELECT_NEXT:
			nResult = __APCLESS_JCB_ManageSelectNext();
			break;
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case JCB_OPS_STATUS_END_APPLICATION:
			nResult = __APCLESS_JCB_ManageEndApplication(dataStruct);
			break;
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		default:
			nResult = __APCLESS_JCB_ManageUnknownOutcome();
			break;
	}


    // Return result
    return (nResult);
}

//! \brief Perform the C5 kernel customisation.
//! \param[in,out] pSharedData Shared buffer used for customisation.
//! \param[in] ucCustomisationStep Step to be customized.
//! \return
//!     - \a KERNEL_STATUS_CONTINUE always.
int APCLESS_JCB_CustomizeStep (T_SHARED_DATA_STRUCT * pSharedData, const unsigned char ucCustomizationStep)
{
    unsigned char           ucCapkIndex;
    unsigned char           ucRid[5];
    unsigned long           ulReadLength;
    int                     nPosition;
    const unsigned char *   pReadValue;
    const unsigned char *   pPan;
    unsigned long           ulPanLength;
    const unsigned char *   pPanSeqNumber;
    //unsigned char           ucVoidPanSeqNumber = C_CLESS_VOID_PAN_SEQ_NUMBER;// Unused value for PanSeqNumber
    unsigned long           ulPanSeqNbLength;
    unsigned char           bPanInExceptionFile = FALSE;
    int                     cr;
	unsigned char *			ucpKeyData = NULL, *ucpExponent = NULL;
	int 					iKeyLen = 0, iExpLen = 0, iRet = 0;
	tMtiMap 				tMapCAPK;
	tMtiMap *				tpMapCAPK = &tMapCAPK;

    switch (ucCustomizationStep) // Steps to customize
    {
        case (STEP_JCB_REMOVE_CARD):
            // get UIR and ....
            nPosition = SHARED_EXCHANGE_POSITION_NULL;
            cr = GTL_SharedExchange_FindNext(pSharedData, &nPosition, TAG_JCB_UIR_ON_OUTCOME_DATA, &ulReadLength, &pReadValue);
            if (cr != STATUS_SHARED_EXCHANGE_OK)
            {
                break;
            }
			APCLESS_Gui_DisplayScreen(APCLESS_SCREEN_REMOVE_CARD);
            GTL_SharedExchange_ClearEx (pSharedData, FALSE);
            break;

        case (STEP_JCB_GET_CA_PUB_KEY):
			mtiMapInit(tpMapCAPK);
            // Init RID value
            memset (ucRid, 0, sizeof(ucRid));

            // Get the CA public key index (card)
            nPosition = SHARED_EXCHANGE_POSITION_NULL;
            if (GTL_SharedExchange_FindNext (pSharedData, &nPosition, TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD, &ulReadLength, (const unsigned char **)&pReadValue) == STATUS_SHARED_EXCHANGE_OK)
                ucCapkIndex = pReadValue[0];
            else
                ucCapkIndex = 0;
			dbuf("TAG_EMV_CA_PUBLIC_KEY_INDEX_CARD",(UCHAR*) pReadValue, ulReadLength);
			mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_CA_PUBLIC_KEY_INDEX_CARD, (UCHAR*) pReadValue, ulReadLength);

            // Get the DF Name returned by the card
            nPosition = SHARED_EXCHANGE_POSITION_NULL;
            if (GTL_SharedExchange_FindNext (pSharedData, &nPosition, TAG_EMV_DF_NAME, &ulReadLength, (const unsigned char **)&pReadValue) == STATUS_SHARED_EXCHANGE_OK)
                memcpy (ucRid, pReadValue, 5);

			dbuf("TAG_EMV_DF_NAME",(UCHAR*) pReadValue, ulReadLength);
			mtiMapPutBytes(tpMapCAPK, MTI_EMV_TAG_AID_TERMINAL, (UCHAR*) pReadValue, ulReadLength);

            // Clear the output structure
            GTL_SharedExchange_ClearEx (pSharedData, FALSE);

            // Get the CA public key data (Modulus, exponent, etc) in the parameters
			if(ucCapkIndex != 0)
			{
				iRet = initEMVCAPK(tpMapCAPK);
				if(iRet)
				{
					ucpExponent = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_EXPONENT, &iExpLen);
					ucpKeyData = mtiMapGetBytes(tpMapCAPK, MTI_EMV_TAG_INT_CAPK_MODULUS, &iKeyLen);
					
					dbuf("MTI_EMV_TAG_INT_CAPK_MODULUS", ucpKeyData, iKeyLen);
					if(GTL_SharedExchange_AddTag(pSharedData, TAG_EMV_INT_CAPK_MODULUS, iKeyLen, ucpKeyData) != STATUS_SHARED_EXCHANGE_OK)
					{
						mtiMapClear(tpMapCAPK);
						GTL_SharedExchange_ClearEx(pSharedData, FALSE);
						return KERNEL_STATUS_CONTINUE;
					}
					
					dbuf("MTI_EMV_TAG_INT_CAPK_EXPONENT", ucpExponent, iExpLen);
					if(GTL_SharedExchange_AddTag(pSharedData, TAG_EMV_INT_CAPK_EXPONENT, iExpLen, ucpExponent) != STATUS_SHARED_EXCHANGE_OK)
					{
						mtiMapClear(tpMapCAPK);
						GTL_SharedExchange_ClearEx(pSharedData, FALSE);
						return KERNEL_STATUS_CONTINUE;
					}
				}
			}
			mtiMapClear(tpMapCAPK);

            break;

        case (STEP_JCB_EXCEPTION_FILE_CHECK):
            // Get the PAN
            nPosition = SHARED_EXCHANGE_POSITION_NULL;
            if (GTL_SharedExchange_FindNext (pSharedData, &nPosition, TAG_EMV_APPLI_PAN, &ulPanLength, &pPan) != STATUS_SHARED_EXCHANGE_OK)
            {
                break;
            }

            // Get the PAN Sequence Number
            nPosition = SHARED_EXCHANGE_POSITION_NULL;
            if (GTL_SharedExchange_FindNext (pSharedData, &nPosition, TAG_EMV_APPLI_PAN_SEQUENCE_NUMBER, &ulPanSeqNbLength, &pPanSeqNumber) != STATUS_SHARED_EXCHANGE_OK)
            {
                // Pan Sequence Number is missing, we will check BlackList without PanSeqNumber
                //pPanSeqNumber = &ucVoidPanSeqNumber;
            }

            // Check if PAN is in the exception file
            //bPanInExceptionFile = ClessSample_BlackListIsPan((int)ulPanLength, pPan, (int)(pPanSeqNumber[0]));

            GTL_SharedExchange_ClearEx (pSharedData, FALSE);

            if (bPanInExceptionFile)
            {
                // Add TAG_KERNEL_PAN_IN_BLACK_LIST tag in the exchange buffer to indicate to JCB kernel that the PAN is in the black list
                if (GTL_SharedExchange_AddTag (pSharedData, TAG_KERNEL_PAN_IN_BLACK_LIST, 1, &bPanInExceptionFile) != STATUS_SHARED_EXCHANGE_OK)
                {
                    GTL_SharedExchange_ClearEx (pSharedData, FALSE);
                }
            }
            break;

        default:
            break;
    }

    return (KERNEL_STATUS_CONTINUE);
}

