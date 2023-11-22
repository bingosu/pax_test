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
/* Module Name: TSA_Menu.h                                                    */
/*                                                                            */
/* Function:    defines for TSA menu related process                          */
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*============================================================================*/
/* usr | yy-mm-dd |                                        |                  */
/*     | 09-01-07 | New for Telium Generic                 |                  */
/******************************************************************************/

#ifndef _TSA_MENU_H_
#define _TSA_MENU_H_


#define MSG_NO_SERIAL_NUMBER "NO SERIAL NUMBER"
#define MSG_TERMINAL_SERIAL_NUMBER "TERMINAL SERIAL NUMBER" 
#define MSG_DUKPT_KSN "DUKPT KSN"
#define MSG_NO_KEYS "NO KEYS"
#define MSG_SECURE_PARTS "SECURE PARTS"
#define MSG_KEY_CHECK_VALUE "KEY CHECK VALUE"
#define MSG_SPECIAL_KEYS "SPECIAL KEYS"
#define MSG_MASTER_SESSION "MASTER/SESSION"
#define MSG_DUKPT "DUKPT"
#define MSG_SECD_KEYS "SECURE DATA KEYS"
#define MSG_NO_PINPAD "PINPAD DISCONNECTED"
#define MSG_PINPAD_SERIAL_NUMBER "PINPAD SERIAL NUMBER" 
#define MSG_FAILED "PROCESS FAILED" 
#define MSG_NOT_AVAILABLE "FUNCTION NOT AVAILABLE" 
#define MSG_SCHEMES "SECURITY SCHEMES" 
#define MSG_WAIT "PLEASE WAIT" 
#define MSG_NO_SECRET "NO SECRET AREA DEFINED"


void DisplaySN(bool PPFlag);
void displayNoKeys(unsigned char *title);
void displayMessage(uchar *title, uchar * message);
void displayMessageGen(uchar *title, int thickness, uchar * message, int timeOut);
void DisplayMSKeysKCV(void);
void DisplayKSN(void);
void DisplaySpecialKeysKCV(void);
void DisplaySecDKeysKCV(void);
void DisplaySecretAreas(void);
void DisplaySchemes(void);

#endif


