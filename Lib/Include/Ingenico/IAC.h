/******************************************************************************/
/*                                                                            */
/*            Copyright (C) 2009.  Ingenico Canada Ltd.                       */
/*      All rights reserved.  No part of this program or listing may          */
/*      be reproduced transmitted, transcribed, stored in a retrieval         */
/*      system, or translated into any language, in any form or by any        */
/*      means, electronic, mechanical, magnetic, optical, chemical,           */
/*      manual, or otherwise, without the prior written permission of         */
/*                                                                            */
/*      Ingenico Canada Ltd., 79 Torbarrie Rd., Toronto, Ontario,             */
/*                   Canada,  M3L 1G5,  (416) 245-6700                        */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/* Module Name: iac.h                                                         */
/*                                                                            */
/* Function:    function prototypes for IAC services                          */
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*============================================================================*/
/* usr | yy-mm-dd |                                        |                  */
/*     | 09-01-07 | New for Telium Generic                 |                  */
/******************************************************************************/

#ifndef _IACINC_H_
#define _IACINC_H_

int IAC_AppInit (void);
int IAC_InqKeyStatus (unsigned int size, void *data);
int IAC_CreateNewKey (void);
int IAC_EncryptData (unsigned int size, void *data);
int IAC_DecryptData (unsigned int size, void *data);
int IAC_ConvertData (unsigned int size, void *data);
int IAC_ConvertFile (unsigned int size, void *data);
int IAC_GetTermNumber (unsigned int size, void *data);
int IAC_GetPPNumber (unsigned int size, void *data);
int IAC_GetKeyID (unsigned int size, void *data);
int IAC_LoadMasterKey (unsigned int size, void *data);
int IAC_LoadSessionKey (unsigned int size, void *data);
int IAC_GetTSAVersion (unsigned int size, void *data);
int IAC_AboutToReboot (void);
int IAC_EraseSessionKey (unsigned int size, void *data);
int IAC_Is_64KeyMode (unsigned int size, void *data);
int IAC_GetKCV (unsigned int size, void *data);
int IAC_GetKeyLength (unsigned int size, void *data);
int IAC_RegisterPrompt (unsigned int size, void *data);
int IAC_VerifyPrompt (unsigned int size, void *data);
int IAC_ApplComment (unsigned int size, void *data);
int IAC_DeleteSecDKey (void);
int IAC_DeletePromptMAC (unsigned int size, void *data);
int IAC_ReadInjData (unsigned int size, void *data);
int IAC_UpdateInjData (unsigned int size, void *data);
int IAC_GetSecOption (unsigned int size, void *data);
int IAC_LoadMasterKeyUsingKEK (unsigned int size, void *data);
int IAC_RandomKeyExpRSA (unsigned int size, void *data);
int IAC_RegisterLockdownApp (unsigned int size, void *data);
int IAC_LoadKeyFromKeyBlock (unsigned int size, void *data);
int IAC_IsOfflineRKIReady( unsigned int size, void *data );

#endif
