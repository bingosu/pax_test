/**
* \file
* \brief This module implements the EMV API functionalities related to the end of the transaction.
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

#ifndef APEMV_SERVICES_EMV_STOP_H_INCLUDED
#define APEMV_SERVICES_EMV_STOP_H_INCLUDED

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

void APEMV_ServicesEmv_StepStop(void);
EMV_Status_t APEMV_ServicesEmv_End(EMV_Object_t EMV_object, TLV_TREE_NODE inputTlvTree,
		TLV_TREE_NODE *tagsKernelToAppli, TLV_TREE_NODE *tagsRequestedByKernel);

#endif // APEMV_SERVICES_EMV_STOP_H_INCLUDED
