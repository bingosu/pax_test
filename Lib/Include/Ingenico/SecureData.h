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
/* Module Name: SecureData.h                                                  */
/*                                                                            */
/* Function:    Prompt MAC processing                                         */
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*============================================================================*/
/* usr | yy-mm-dd |                                        |                  */
/*     | 09-01-07 | New for Telium Generic                 |                  */
/******************************************************************************/

#ifndef _SECUREDATA_H_
#define _SECUREDATA_H_

int read_SECD_keyinfo_file(void);
int SECD_CreateNewKey(void);
void SECD_checkKeyFile(void);
void SECD_procFormatKeyInfo (int expiredPeriod, 
                             char* newKeyStatus, unsigned char* newKCV, 
                             char* oldKeyStatus, unsigned char* oldKCV);
int SECD_DeleteSecDKey (void);

#endif
