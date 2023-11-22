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
/* Module Name: TSA_def.h                                                     */
/*                                                                            */
/* Function:    Application definitions used in TSA                           */
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*============================================================================*/
/* usr | yy-mm-dd |                                        |                  */
/* ewc | 09-02-03 | New for TSA                            |                  */
/******************************************************************************/

#ifndef _TSADEF_H_
#define _TSADEF_H_

/* 
 key types
 */
typedef enum 
{
  TSA_KEY_MASTER=0,
  TSA_KEY_SESSION,
  TSA_KEY_DUKPT,
  TSA_KEY_KTK,
  TSA_KEY_PEFMK,
  TSA_KEY_CEFMK,
  TSA_KEY_CDMK,
  TSA_KEY_CURMASTER,
  TSA_KEY_MAX
} eKeyType;


/* 
 key purpose
 */
typedef enum 
{
  TSA_UNKNOWN=0,
  TSA_PINKEY,
  TSA_MACKEY,
  TSA_COMKEY,
  TSA_KEYPURPOSE_MAX
} eKeyPurpose;


/* 
 application comment mode
 */
#define  TSA_APPLCMNT_READ  0       // read
#define  TSA_APPLCMNT_WRITE 1       // write


/* 
 security option keyword
 */
typedef enum 
{
    TSA_SOPT_START=0,
    TSA_SOPT_CodeMacMth,
    TSA_SOPT_CodeMacEna,
    TSA_SOPT_EraseOnIDChg,
    TSA_SOPT_PromptMACEna,
    TSA_SOPT_MacCalcMode,
    TSA_SOPT_DevMode,
    TSA_SOPT_AtaKeyEna,
    TSA_SOPT_StartVerMac,
    TSA_SOPT_VisaPEDMode,
    TSA_SOPT_FinKeyOption,
    TSA_SOPT_MAXNUM
} eSecOption;


#define  TSA_MAX_PROMPT_INDEX   63
#define  TSA_NEW_KEY_NOTIFICATION    "TSA:NEW KEY NOTIFICATION"

/* 
 return value/status
 */

typedef enum 
{
/* general */
  TSA_KO=-1,
  TSA_OK,
  TSA_APP_NOT_READY,
  TSA_KEY_REINJECTED,
  TSA_KEY_LEN_SINGLE,
  TSA_KEY_LEN_DOUBLE,
  TSA_INJ_LOCK,
  TSA_KEY_CREATED,

/* error in parsing response TLV tree > 50 */
  TSA_ERR_RESP_OLDKEYSTS=50,
  TSA_ERR_RESP_NEWKEYSTS,
  TSA_ERR_RESP_OLDKEYKCV,
  TSA_ERR_RESP_NEWKEYKCV,
  TSA_ERR_RESP_DATA,
  TSA_ERR_RESP_KEYSTRUCT,

/* specific error/status related to data encryption > 100 */
  TSA_ERR_INP_EXPPERIOD=100,
  TSA_NO_KEY,                   // secure data (new) key does not exist
  TSA_NO_OLDKEY,                // secure data old key does not exist
  TSA_EXPIRED,                  // secure data key expired
  TSA_ERR_KEYFILE,              // secure data key file error
  TSA_ERR_CREATEKEY,            // failed to create secure data key
  TSA_ERR_INP_DATA,
  TSA_ERR_DATACIPHER,
  TSA_ERR_INP_DISK,
  TSA_ERR_INP_FILE,
  TSA_ERR_USERFILE,
  TSA_ERR_INP_KEYTYPE,
  TSA_ERR_INP_KEYINDEX,
  TSA_ERR_INP_KEYUSAGE,
  TSA_ERR_INP_KEYVALUE,
  TSA_OUTBUFFER_TOO_SMALL,
  TSA_ERR_DEVICE_BUSY,          // booster is busy

/* specific error related to financial key > 200 */
  TSA_ERR_KCVCHK_FAILED=200,        // KCV verification failed
  TSA_ERR_NO_SUCH_KEY,              // invalid key
  TSA_ERR_KEY_NOT_LOADED,           // not loaded
  TSA_ERR_KEY_AREA_NOT_INIT,        // key area not initialized
  TSA_ERR_MASTERKEY_NOT_LOADED,     // master key not loaded
  TSA_ERR_LOAD_KEY,                 // fail to load key
  TSA_ERR_KEY_PURPOSE,              // session key purpose does not match with master's
  TSA_ERR_KEY_LENGTH,               // session key length longer than master's
  TSA_ERR_SET_SEC_CONFIG,           // failed to set security config

/* specific error related to other API > 300 */
  TSA_ERR_MAC_VER=300,              // MAC verification failed
  TSA_ERR_PROMPT_IDX,               // invalid prompt index
  TSA_ERR_APPLCMNT,                 // invalid application comment mode
  TSA_ERR_PP_NOT_CONFIG,            // pinpad not configured
  TSA_ERR_PP_DISCONNECTED,          // pinpad configured but disconnected 
  TSA_ERR_PP_CONFIG,                // pinpad configured: not allowed for registering prompt
  TSA_ERR_NOT_REGISTERED,           // prompt not registered
  TSA_ERR_PP_NOT_INJECTED,          // pinpad not injected
  TSA_ERR_SECOPTION,                // invalid security option
  TSA_ERR_RSASIZE,                  // RSA key size error
  TSA_ERR_RSAMODULUS,               // RSA modulus error
  TSA_ERR_RSAEXPONENT,              // RSA exponent error
  TSA_ERR_NOOFFLINERKI,                 // Offline RKI is not available
  TSA_ERR_RKI_INVALID_KEYBLOCK,     // Invalid Key Block for Offline RKI
  TSA_MAX_ERROR
} eTSAErrorCode;

#endif
