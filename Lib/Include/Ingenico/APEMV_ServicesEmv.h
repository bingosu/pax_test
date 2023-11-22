/**
* \file
* \brief This module implements main the EMV API functionalities.
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

#ifndef APEMV_SERVICES_EMV_H_INCLUDED
#define APEMV_SERVICES_EMV_H_INCLUDED

#include "libMtiEmvApi.h"

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//! \brief Status of the transaction.
typedef enum
{
	APEMV_TR_STATUS_UNKNOWN,				//!< Unknown status.

	APEMV_TR_STATUS_APPROVED,				//!< The transaction is approved.
	APEMV_TR_STATUS_DECLINED,				//!< The transaction is declined.

	APEMV_TR_STATUS_SERVICE_NOT_ALLOWED,	//!< The service is not allowed.
	APEMV_TR_STATUS_CANCELLED,				//!< The transaction is cancelled.

	APEMV_TR_STATUS_CARD_BLOCKED,			//!< The card is blocked.
	APEMV_TR_STATUS_CARD_REMOVED,			//!< The transaction is removed.

	APEMV_TR_STATUS_TERMINAL_ERROR,			//!< There is an error that comes from the terminal.
	APEMV_TR_STATUS_CARD_ERROR,				//!< There is an error that comes from the card.

	APEMV_TR_STATUS_END
} APEMV_ServicesEmv_TransactionStatus_e;

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

EMV_Status_t APEMV_ServicesEmv_DoTransaction(const char *cardReader, unsigned int cardReaderBufferSize, int doEmvAppliSelection, TLV_TREE_NODE inputTlvTree);
int APEMV_ServicesEmv_GetTransactionData(TLV_TREE_NODE outputTlvTree);

int APEMV_ServicesEmv_AmountIsSet(void);
int APEMV_ServicesEmv_AmountSet(unsigned long long amount, TLV_TREE_NODE outputTlvTree);

APEMV_ServicesEmv_TransactionStatus_e APEMV_ServicesEmv_TransacStatusGet(void);
void APEMV_ServicesEmv_TransacStatusChange(APEMV_ServicesEmv_TransactionStatus_e status);
void APEMV_ServicesEmv_TransacStatusSet(EMV_Status_t emvStatus, TLV_TREE_NODE transactionData);
void APEMV_ServicesEmv_GetGlobalParam(TLV_TREE_NODE outputTlvTree);
int __APEMV_InputSecurePinEntry(TLV_TREE_NODE outputTlvTree, int bypass);


#endif // APEMV_SERVICES_EMV_H_INCLUDED
