/**
* \file
* \brief This module implements all the Telium Manager services.
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

#ifndef APEMV_SERVICES_MANAGER_H_INCLUDED
#define APEMV_SERVICES_MANAGER_H_INCLUDED

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//// Static function definitions ////////////////////////////////

//// Global variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

int APEMV_ServicesManager_AfterReset(NO_SEGMENT appliId, S_TRANSOUT *paramOut);
int APEMV_ServicesManager_IsName(NO_SEGMENT appliId, S_ETATOUT *paramOut);
int APEMV_ServicesManager_GiveYourDomain(NO_SEGMENT appliId, S_INITPARAMOUT *paramOut);
int APEMV_ServicesManager_GiveYourSpecificContext(NO_SEGMENT appliId, S_SPECIFIC_CONTEXT *paramOut);

int APEMV_ServicesManager_GiveMoneyExtended(NO_SEGMENT appliId, S_MONEYOUT_EXTENDED *paramOut);

int APEMV_ServicesManager_IsState(NO_SEGMENT appliId, S_ETATOUT *paramOut);

int APEMV_ServicesManager_GiveAid(NO_SEGMENT appliId, const S_TRANSIN *paramIn, _DEL_ *paramOut);
int APEMV_ServicesManager_IsCardEmvForYou(NO_SEGMENT appliId, const S_AID *paramIn, S_CARDOUT *paramOut);
int APEMV_ServicesManager_DebitEmv(NO_SEGMENT appliId, const S_TRANSIN *paramIn, S_TRANSOUT *paramOut);

int APEMV_ServicesManager_MoreFunction(NO_SEGMENT appliId);

#endif // APEMV_SERVICES_MANAGER_H_INCLUDED
