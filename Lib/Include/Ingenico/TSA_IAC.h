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
/* Module Name: TSA_IAC.h                                                     */
/*                                                                            */
/* Function:    TSA IAC defines                                               */
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*============================================================================*/
/* usr | yy-mm-dd |                                        |                  */
/* ewc | 09-02-03 | New for TSA                            |                  */
/******************************************************************************/

#ifndef _TSA_IAC_H_
#define _TSA_IAC_H_

//#include "appel.h"

#define     TSA_APPL_TYPE           0x733d

// IAC services
#define     TSA_SERVICE_START       101

typedef enum 
{
  IAC_APP_INIT=TSA_SERVICE_START,       // Key injection is not performed
  IAC_INQ_KEYSTATUS,
  IAC_CREATE_NEWKEY,
  IAC_ENCRYPT_DATA,
  IAC_DECRYPT_DATA,
  IAC_CONVERT_DATA,
  IAC_CONVERT_FILE,
  IAC_GET_TERMNUMBER,
  IAC_GET_KEYID,
  IAC_LOAD_MASTERKEY,
  IAC_LOAD_SESSIONKEY,
  IAC_GET_TSA_VERSION,
  IAC_ABOUT_TO_REBOOT,
  IAC_ERASE_SESSIONKEY,
  IAC_IS_64KEY_MODE,
  IAC_GET_KCV,
  IAC_GET_KEYLENGTH,
  IAC_REGISTER_PROMPT,
  IAC_VERIFY_PROMPT,
  IAC_APPL_COMMENT,
  IAC_DELETE_SECDATKEY,
  IAC_DELETE_PROMPTMAC,
  IAC_GET_PPNUMBER,
  IAC_READ_INJDATA,
  IAC_WRITE_INJDATA,
  IAC_GET_SECOPTION,
  IAC_LOAD_MASTERKEYKEK,
  IAC_GEN_KEYEXPRSA,
  IAC_REGISTER_lOCKDOWN_APP,
  IAC_LOAD_KEY_FROM_KEY_BLOCK,
  IAC_OFFLINE_RKI_STATUS,
  IAC_SERVICE_MAX
} eIAC_Service;


// tag for IAC
#define TAG_IAC_TOP_NODE                0x9FA550        // IAC top node
#define TAG_DSSKEY_EXPIRY_PERIOD        0x9FA551        // PCI DSS key expiry period
#define TAG_ENCDATA_OLDKEY              0x9FA552        // data encrypted under old key
#define TAG_DISKNAME                    0x9FA553        // disk name
#define TAG_FILENAME                    0x9FA554        // file name
#define TAG_KEY_TYPE                    0x9FA555        // key type
#define TAG_KEY_INDEX                   0x9FA556        // key index
#define TAG_ENCRYPTED_KEYVALUE          0x9FA557        // encrypted key value
#define TAG_STATUS_OLDKEY               0x9FA558        // sec data old key status
#define TAG_STATUS_NEWKEY               0x9FA559        // sec data new key status
#define TAG_KCV_OLDKEY                  0x9FA55A        // sec data old key check value
#define TAG_KCV_NEWKEY                  0x9FA55B        // sec data new key check value
#define TAG_ENCRYPTED_DATA              0x9FA55C        // encrypted data
#define TAG_CLEAR_DATA                  0x9FA55D        // clear data
#define TAG_KEY_STRUCT                  0x9FA55E        // Telium key structure (for access key in secret area)
#define TAG_GENERAL_TEXT                0x9FA55F        // text for general purpose
#define TAG_KEY_PURPOSE                 0x9FA560        // key purpose
#define TAG_GENERAL_BOOL                0x9FA561        // bool for general purpose
#define TAG_GENERAL_UCHAR               0x9FA562        // uchar for general purpose
#define TAG_PROMPT_INDEX                0x9FA563        // prompt index
#define TAG_PROMPT_SIZE                 0x9FA564        // prompt text size
#define TAG_PROMPT_TEXT                 0x9FA565        // prompt text
#define TAG_PROMPT_MAC                  0x9FA566        // prompt MAC
#define TAG_KCV_ASCII                   0x9FA567        // KCV in ascii hex
#define TAG_INJECTDATA                  0x9FA568        // injected data
#define TAG_KEY_SIZE                    0x9FA569        // key size
#define TAG_USAGE                       0x9FA56A        // usage
#define TAG_PARITY                      0x9FA56B        // parity adjustment
#define TAG_KEYBLOCKTYPE                0x9FA56C        // key block type
#define TAG_RSAPK_BITS                  0x9FA56D        // RSA public key # of bits
#define TAG_RSAPK_MOD                   0x9FA56E        // RSA public key modulus
#define TAG_RSAPK_EXP                   0x9FA56F        // RSA public key exponent
#define TAG_RSABLOCK                    0x9FA570        // RSA output block
#define TAG_OUTPUTSIZE                  0x9FA571        // output size
#define TAG_APP_FAMILY                  0x9FA572        // Application family name
#define TAG_LOCKDOWN_NAME               0x9FA573        // Lockdown Name


#define  TSA_AES_BLOCK_SIZE    16

#endif


