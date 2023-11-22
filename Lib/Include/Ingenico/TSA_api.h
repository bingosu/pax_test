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
/* Module Name: TSA_api.h                                                     */
/*                                                                            */
/* Function:    API functions provided by TSA (formerly known as SDA)         */
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*============================================================================*/
/* usr | yy-mm-dd |                                        |                  */
/* ewc | 09-02-03 | New for TSA                            |                  */
/******************************************************************************/

#ifndef _TSA_API_H
#define _TSA_API_H

#include "SEC_interface.h"
#include "TSA_def.h"


#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif


/**
 * TSA_InquireKeyStatus - Inquire Key Status
 * Input:
 *  int expiredPeriod: expiry period of secure data key, in number of days
 *                     <= 0 = apply the default expire period
 *                            TSA will then apply the default expire period (90 days)
 *                     else, i.e. > 0, check for key expiry
 * Output:
 *  char* newKeyStatus: status of the new key
 *                      TSA_OK = new key OK
 *                      TSA_NO_KEY = new key does not exist
 *                      TSA_EXPIRED = new key is expired (note)
 *                      Note: starting from TSA v1.01, TSA will automatically generate
 *                            a new key if the new key is expired. So the user application
 *                            not receive TSA_EXPIRED status for new key
 *  uchar* newKCV: 3 bytes key check value of the new key
 *  char* oldKeyStatus: status of the old key
 *                      TSA_OK = old key OK
 *                      TSA_NO_KEY = old key does not exist
 *                      TSA_EXPIRED = old key is expired
 *  uchar* oldKCV: 3 bytes key check value of the old key
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_EXPPERIOD = error in processing expiry period
 *  Other possible error in formatting response:
 *  TSA_ERR_RESP_OLDKEYSTS 
 *  TSA_ERR_RESP_NEWKEYSTS
 *  TSA_ERR_RESP_OLDKEYKCV
 *  TSA_ERR_RESP_NEWKEYKCV
 *  TSA_KO = other error
 */
int TSA_InquireKeyStatus(int expiredPeriod, char* newKeyStatus, uchar* newKCV, 
                          char* oldKeyStatus, uchar* oldKCV);


/**
 * TSA_CreateNewKey - Create New Key
 * Input:
 *  NIL
 * Output:
 *  NIL
 * Return value:
 *  TSA_OK = successful
 *  TSA_KEY_CREATED = new key has been created since reboot
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_KEYFILE = error in updating key information
 *  TSA_ERR_CREATEKEY = failed to create secure data key
 *  TSA_KO = other error
 */
int TSA_CreateNewKey(void);


/**
 * TSA_EncryptData - Encrypt Data
 * Input:
 *  char* inData: input data
 *                if not in multiple of 16 bytes, TSA will pack hex zeros at the end to make
 *                up the length to a multiple of 16.
 *  uint inLen: input data length
 *  uint * outLen: maximum output data length; TSA will reject if it is too small for
 *                       output; this will be overwritten by the actual output length
 * Output:
 *  char* outData: output data, i.e. encrypted data
 *  uint * outLen: output data length
 *  uchar* KCV: 3 bytes key check value of the key used to encrypt the data
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_NO_KEY = key does not exist
 *  TSA_ERR_INP_DATA = error in processing input data
 *  TSA_ERR_DATACIPHER = failed to data ciphering
 *  TSA_OUTBUFFER_TOO_SMALL = output buffer too small
 *  TSA_KO = other error
 */
int TSA_EncryptData(char* inData, uint inLen, char* outData, 
                     uint* outLen, uchar* KCV);


/**
 * TSA_DecryptData - Decrypt Data
 * Input:
 *  char* inData: input data
 *                if not in multiple of 16 bytes, TSA will pack hex zeros at the end to make
 *                up the length to a multiple of 16.
 *  uint inLen: input data length
 *  uint * outLen: maximum output data length; TSA will reject if it is too small for
 *                       output; this will be overwritten by the actual output length
 * Output:
 *  char* outData: output data, i.e. decrypted data
 *  uint * outLen: output data length
 *  uchar* KCV: 3 bytes key check value of the key used to decrypt the data
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_NO_KEY = key does not exist
 *  TSA_ERR_INP_DATA = error in processing input data
 *  TSA_ERR_DATACIPHER = failed to data ciphering
 *  TSA_OUTBUFFER_TOO_SMALL = output buffer too small
 *  TSA_KO = other error
 */
int TSA_DecryptData(char* inData, uint inLen, char* outData, 
                     uint* outLen, uchar* KCV);


/**
 * TSA_ConvertData - Convert Data
 * Input:
 *  char* inData: input data
 *                if not in multiple of 16 bytes, TSA will pack hex zeros at the end to make
 *                up the length to a multiple of 16.
 *  uint inLen: input data length
 *  uint * outLen: maximum output data length; TSA will reject if it is too small for
 *                       output; this will be overwritten by the actual output length
 * Output:
 *  char* outData: output data, i.e. converted data (decrypt with the old key, then encrypt with the new key)
 *  uint * outLen: output data length
 *  uchar* KCV: 3 bytes key check value of the key used to encrypt the data
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_NO_KEY = new key does not exist
 *  TSA_NO_OLDKEY = old key does not exist
 *  TSA_ERR_INP_DATA = error in processing input data
 *  TSA_ERR_DATACIPHER = failed to data ciphering
 *  TSA_OUTBUFFER_TOO_SMALL = output buffer too small
 *  TSA_KO = other error
 */
int TSA_ConvertData(char* inData, uint inLen, char* outData, 
                     uint* outLen, uchar* KCV);


/**
 *TSA_ConvertFile - Convert File
 * Input:
 *  char* convertDiskName: NULL terminated string for diskname of the file to be converted
 *  char* convertFileName: NULL terminated string for filename of the file to be converted
 * Output:
 *  uchar* KCV: 3 bytes key check value of the key used to encrypt the data
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_NO_KEY = new key does not exist
 *  TSA_NO_OLDKEY = old key does not exist
 *  TSA_ERR_INP_DISK = diskname error
 *  TSA_ERR_INP_FILE = filename error
 *  TSA_ERR_USERFILE = error processing user file
 *  TSA_ERR_DATACIPHER = failed to data ciphering
 *  TSA_KO = other error
 */
int TSA_ConvertFile(const char* convertDiskName, const char* convertFileName, 
                     uchar* KCV);


/**
 * TSA_GetTermSerialNumber - Get Terminal Serial Number
 * Input:
 *  NIL
 * Output:
 *  uchar* termSerialNum: max 16 bytes terminal serial number plus a NULL terminator
 * Return value:
 *  TSA_OK = successful
 *  TSA_KO = failed
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_RESP_DATA = error processing data in response 
 */
int TSA_GetTermSerialNumber (uchar* termSerialNum);


/**
 * TSA_GetExtPPSerialNumber - Get External Pinpad Serial Number
 * Input:
 *  NIL
 * Output:
 *  uchar* pinpadSerialNum: max 16 bytes pinpad serial number plus a NULL terminator
 * Return value:
 *  TSA_OK = successful
 *  TSA_KO = failed
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_PP_NOT_CONFIG = external pinpad not config
 *  TSA_ERR_PP_DISCONNECTED = external pinpad config but disconnected
 *  TSA_ERR_RESP_DATA = error processing data in response 
 */
int TSA_GetExtPPSerialNumber (uchar* pinpadSerialNum);


/**
 * TSA_GetKeyID - Get Key ID
 * Input:
 *  uchar keyType: key type : see TSA_def.h
 *                         e.g. TSA_KEY_MASTER, TSA_KEY_DUKPT, etc
 *  uchar keyIndex: key slot #
 *                          when key type = TSA_KEY_MASTER, i.e.Master
 *                              0-9 for Application Based Financial Key Emulation
 *                              0-63 for Terminal Based Financial Key Emulation
 *                          when key type = TSA_KEY_DUKPT, i.e.DUKPT
 *                              0-9 for Application Based Financial Key Emulation
 *                              0-5 for Terminal Based Financial Key Emulation
 *                          when key type = TSA_KEY_KTK, 
 *                                          TSA_KEY_PEFMK,
 *                                          TSA_KEY_CEFMK,
 *                                          TSA_KEY_CDMK
 * Output:
 *  T_SEC_DATAKEY_ID * ptKeyId: key ID in Telium format
 *                              user application can use it to access the key through Security TSA schemes
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYTYPE = error in key type
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_KEY_NOT_LOADED =  key not loaded 
 *  TSA_ERR_PP_DISCONNECTED = when external pinpad is configured, keys in pinpad should be used
 *                            but pinpad is disconnected
 *  TSA_ERR_SET_SEC_CONFIG = failed to set security part as configured in preparation for
 *                           the PA to access the required key
 *  TSA_KO = other error
 */
int TSA_GetKeyID (uchar keyType, uchar keyIndex,
                    T_SEC_DATAKEY_ID * ptKeyId);


/**
 * TSA_LoadMasterKey - Load Master key to secret area
 *
 * Input:
 *  uchar keyIndex:     key index of the master key
 *  uchar keyPurpose:     key usage (PIN, MAC, COM) of the master key
 *  uchar * pKeyValue:  pointer to the encrypted key value (in ascii hex), null terminated string
 *                      e.g. a single length key 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
 *                      will be passed as "0123456789abcdef" or "0123456789ABCDEF"
 *  uchar * keyCV:  pointer to 8 bytes (in ascii hex) key check value of the key
 *  uchar encKey:  encryption key type 
 *                  TSA_KEY_KTK = encryption key is KTK
 *                  TSA_KEY_CURMASTER = encryption key is current master key
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_INP_KEYUSAGE = error in key purpose
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_KEY_NOT_LOADED =  key not loaded 
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_LOAD_KEY = failed to load key
 *  TSA_ERR_KCVCHK_FAILED =  KCV verification failed
 *  TSA_KO = other error
 */
int TSA_LoadMasterKey (uchar keyIndex, uchar keyPurpose, 
                       uchar * pKeyValue,
                       uchar * keyCV, uchar encKey);


/**
 * TSA_LoadSessionKey - Load Session key to secret area
 *
 * Input:
 *  uchar keyIndex:     key index of the session key
 *  uchar keyPurpose:     key usage (PIN, MAC, COM) of the session key
 *  uchar * pKeyValue:  pointer to the encrypted key value (in ascii hex), null terminated string
 *                      e.g. a single length key 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
 *                      will be passed as "0123456789abcdef" or "0123456789ABCDEF"
 *
 * Output:
 *  uchar* keyCV: 8 bytes (in ascii hex) key check value of the key
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_INP_KEYUSAGE = error in key purpose
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_MASTERKEY_NOT_LOADED =  master key not loaded 
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_KEY_PURPOSE = key purpose does not match with the corresponding master key
 *  TSA_ERR_LOAD_KEY = failed to load key
 *  TSA_ERR_KEY_LENGTH = session key length longer than its master key's
 *  TSA_KO = other error
 */
int TSA_LoadSessionKey (uchar keyIndex, uchar keyPurpose, 
                        uchar * pKeyValue,
                        uchar * keyCV);


/**
 * TSA_EraseSessionKey - Erase Session key from secret area
 *
 * Input:
 *  uchar keyIndex:     key index of the session key
 *  uchar keyPurpose:     key usage (PIN, MAC, COM) of the session key
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_INP_KEYUSAGE = error in key purpose
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_KEY_NOT_LOADED = key not loaded
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_KEY_PURPOSE = key purpose does not match with what has been loaded
 *  TSA_KO = other error
 */
int TSA_EraseSessionKey (uchar keyIndex, uchar keyPurpose);


/**
 * TSA_LoadMasterKeyUsingKEK - Load Master key to secret area using another Master key
 *
 * Input:
 *  uchar keyIndex:     key index of the master key
 *  uchar keyPurpose:   key usage (PIN, MAC, COM) of the master key
 *  uchar * pKeyValue:  pointer to the encrypted key value (in ascii hex), null terminated string
 *                      e.g. a single length key 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
 *                      will be passed as "0123456789abcdef" or "0123456789ABCDEF"
 *  uchar* keyCV:       8 bytes (in ascii hex) key check value of the key
 *  uchar decryptKeyIndex   key index of the master key used as decrypting key
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_INP_KEYUSAGE = error in key purpose
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_KEY_NOT_LOADED =  key not loaded 
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_LOAD_KEY = failed to load key
 *  TSA_ERR_KCVCHK_FAILED =  KCV verification failed
 *  TSA_KO = other error
 */
int TSA_LoadMasterKeyUsingKEK (uchar keyIndex, uchar keyPurpose, 
                               uchar * pKeyValue,
                               uchar * keyCV, uchar decryptKeyIndex);


/**
 * TSA_LoadKeyFromKeyBlock - Load a key from TR31-formated key block to secret area
 *
 * Input:
 *
 *  pKeyBundleRecord - Pointer to a NULL terminated string having the following format:
 *  	<KeyBundleRecord> := <SERIAL NUMBER><FS><KEY LABEL><fs><KEY USAGE><FS><KEY_BUNDLE> where
 *  	1. <FS> is the ASCII code 0x1C,
 *  	2. <SERIAL_NUMBER> this field is not used by the function
 *  	3. <KEY LABEL> a string specifying the type and index of key to be injected:
 *  	   The accepted key types and their valid range of index are described below:
 *  	   - "CDMK", "PEFMK", "CEFMK" and "KBPK", the input key value is to be injected as a CDMK, PEFMK, CEFMK, KBPK key respectively,
 *  	   - "MK<x>", the input key value is to be injected as a Master Key with index is a <x> where x is '0' <= x <= '9',
 *  	   - "SK<x>", the input key value is to be injected as a Session key with index <x> where x is '0' <= x <= '9',
 *  	   - "IPPK<x>", the input key value is to be injected as the initial DUKPT key with index <x> where x  is '0' <= x <= '5'
 *  	   all other values of <KEY LABEL> are considered invalid.
 *  	4. <KEY USAGE>, the usage of the input key being injected. This field will be ignored when <KEY LABEL> is either
 *  	   "CDMK", "PEFMK", "CEFMK" or "KBPK".  When <KEY LABEL> is either <MK<x> or <SK<x>>, the API will accept one of the following
 *  	   three values "PIN", "MAC", "COM".  When <KEY LABEL> is <IPPK<x>>, the valid values for this field are "PIN", "DATA", "E2EE".
 *  	5. <KEY BUNDLE>, this field contains a TR-31 formatted key block, the block contains an encrypted key to be injected to
 *  	   a Telium terminal's secret area.
 *
 * Output:
 *    Upon successful execution, the encryption key embedded in <KEY BUNDLE> will be stored in the Telium's secret area whose location
 *    is specified by the <KEY LABEL> AND <KEY USAGE> fields.
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_INP_KEYUSAGE = error in key purpose
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_KEY_NOT_LOADED =  key not loaded
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_LOAD_KEY = failed to load key
 *  TSA_ERR_NOOFFLINERKI = Offline RKI is not enabled.
 *  TSA_ERR_RKI_INVALID_KEYBLOCK = The input Key Block contains invalid data
 *  TSA_KO = other error
 */
int TSA_LoadKeyFromKeyBlock (const uchar * pKeyBundleRecord );

/**
 * TSA_isOfflineRKIReady - Can Offline RKI be performed successfully
 *
 * Input:
 *
 *  None
 *
 * Output:
 *
 *  None
 *
 * Return value:
 *  TRUE - The terminal can accept Offline RKI request.
 *  FALSE - The terminal cannot accept offline RKI request.
 */
int TSA_isOfflineRKIReady( void );

/**
 * TSA_RandomKeyExpRSA - Generate random key to a Master Key slot and return RSA ciphered block
 *
 * Input:
 *  uchar iKeyIndex:    key index of the master key to be generated
 *  uchar keyPurpose:   key usage; only used when iUsage is CIPHERING_DATA - TSA_MACKEY or TSA_COMKEY
 *                          for CIPHERING_KEY key purpose will be set to TSA_UNKNOWN automatically
 *                          for CIPHERING_PIN key purpose will be set to TSA_PINKEY automatically
 *  int iKeySize        Key size in bytes
 *  int iUsage          Key usage (CIPHERING_DATA, CIPHERING_KEY or CIPHERING_PIN)
 *                          random key will be saved as Master key when usage is CIPHERING_KEY
 *                          otherwise saved as Session key
 *  int iParityAdjustment   (PARITY_ODD, PARITY_EVEN or PARITY_NONE) for the key to generate
 *  int iKeyBlockType       according to PKCS#1 v1.5 for RSA encryption
 *  R_RSA_PUBLIC_KEY * pt_stRSAPubKey   RSA Public key in the form modulus/exponent (max 2048 bits)
 *
 * Output:
 *  unsigned char * RSABlock    RSA ciphered block
 *  unsigned int * RSABlockLen  length (in bytes) of RSA ciphered block
 */
int TSA_RandomKeyExpRSA (uchar iKeyIndex, uchar keyPurpose, int iKeySize, 
                         int iUsage, int iParityAdjustment, int iKeyBlockType, 
                         R_RSA_PUBLIC_KEY * pt_stRSAPubKey, 
                         unsigned char * RSABlock, unsigned int * RSABlockLen);


/**
 * TSA_GetTSAVersion - get TSA version (security and diagnostic app)
 * Input:
 *  NIL
 * Output:
 *  char * version:     4 bytes version number of the TSA in ASCII printable format, plus a NULL terminator
 * Return value:
 *  TSA_OK = successful
 *  TSA_KO = failed
 *  TSA_ERR_RESP_DATA = error processing data in response 
 */
int TSA_GetTSAVersion (char * version);


/**
 * TSA_is64KeyMode - check if 64 key mode is being used
 * Input:
 *  NIL
 * Output:
 *  bool * is64Key:     true = it is in 64 key mode
 *                      else = it is not
 * Return value:
 *  TSA_OK = successful
 *  TSA_KO = failed
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_RESP_DATA = error processing data in response 
 */
int TSA_is64KeyMode (bool * is64Key);


/**
 * TSA_GetKCV - Get key check value
 *
 * Input:
 *  uchar keyType: key type : see TSA_def.h
 *                         e.g. TSA_KEY_MASTER or TSA_KEY_SESSION
 *  uchar keyIndex:     key index of the key
 *
 * Output:
 *  uchar* KCV: 8 bytes (in ascii hex) key check value of the key
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYTYPE = error in key type
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_KEY_NOT_LOADED = key not loaded
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_KO = other error
 */
int TSA_GetKCV (uchar keyType, uchar keyIndex, uchar * KCV);


/**
 * TSA_GetKeyLength - Get key length of a master/session key
 *
 * Input:
 *  uchar keyType: key type : see TSA_def.h
 *                         e.g. TSA_KEY_MASTER or TSA_KEY_SESSION
 *  uchar keyIndex:     key index of the key
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TSA_KEY_LEN_SINGLE = successful, key length SINGLE
 *  TSA_KEY_LEN_DOUBLE = successful, key length DOUBLE
 *
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_KEYTYPE = error in key type
 *  TSA_ERR_INP_KEYINDEX = error in key index
 *  TSA_ERR_NO_SUCH_KEY = no such key
 *  TSA_ERR_KEY_NOT_LOADED = key not loaded
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_KO = other error
 */
int TSA_GetKeyLength (uchar keyType, uchar keyIndex);


/**
 * TSA_RegisterPromptMAC - register a prompt after MAC verification is passed
 *
 * Input:
 *  bool isSecure       true means it is a prompt secure data entry
 *                      else it is for clear text data entry
 *  uchar promptIndex   from 0 to 63
 *  int promptSize      length of the prompt text to be used in MAC calculation
 *  char* promptText    prompt text
 *  uchar* promptMAC    8 byte MAC in ASCII-hex
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_ERR_MAC_VER = MAC verification failed
 *
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_KEY_NOT_LOADED = key required for the MAC is not loaded
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_PROMPT_IDX = invalid prompt index
 *  TSA_ERR_PP_CONFIG = register prompt is rejected if external pinpad is configured
 *  TSA_KO = other error
 */
int TSA_RegisterPromptMAC (bool isSecure, uchar promptIndex, int promptSize,
                           char* promptText, uchar* promptMAC);


/**
 * TSA_VerifyPromptMAC   - verify a prompt
 *                         user application can either pass the actual MAC in promptMAC
 *                         or a registered prompt index #
 *                         if the promptMAC pointer is not NULL
 *                         {
 *                              data pointed will be used for verification, 
 *                              in such case the promptIndex will be ignored
 *                         }
 *                         else     // promptMAC == NULL
 *                         {
 *                              prompt MAC registed which is pointed by promptIndex
 *                              will be used for verification
 *                         }
 *
 * Input:
 *  bool isSecure       true means it is a prompt secure data entry
 *                      else it is for clear text data entry
 *  uchar promptIndex   from 0 to 63
 *  int promptSize      length of the prompt text to be used in MAC calculation
 *  char* promptText    prompt text
 *  uchar* promptMAC    8 byte MAC in ASCII-hex; == NULL means use the registered prompt MAC for verification
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_ERR_MAC_VER = MAC verification failed
 *
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_KEY_NOT_LOADED = key required for the MAC is not loaded
 *  TSA_ERR_KEY_AREA_NOT_INIT = secret area not init
 *  TSA_ERR_PROMPT_IDX = invalid prompt index
 *  TSA_ERR_NOT_REGISTERED = prompt has not been registered
 *  TSA_KO = other error
 */
int TSA_VerifyPromptMAC (bool isSecure, uchar promptIndex, int promptSize,
                         char* promptText, uchar* promptMAC);


/**
 * TSA_ApplComment - read or write application comment
 *
 * Input:
 *  uchar mode          TSA_APPLCMNT_READ = read
 *                      TSA_APPLCMNT_WRITE = write
 *  char* applComment   null terminated string, maximum string length of 16
 *
 * Output:
 *  char* applComment   application comment read when mode is read
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_APPLCMNT = invalid application comment mode
 *  TSA_KO = other error
 */
int TSA_ApplComment (uchar mode, char* applComment);


/**
 * TSA_GetSecOption - get security option setting
 *
 * Input:
 *  uchar option       TSA_SOPT_CodeMacMth,
 *                     TSA_SOPT_CodeMacEna,
 *                     TSA_SOPT_EraseOnIDChg,
 *                     TSA_SOPT_PromptMACEna,
 *                     TSA_SOPT_MacCalcMode,
 *                     TSA_SOPT_DevMode,
 *                     TSA_SOPT_AtaKeyEna,
 *                     TSA_SOPT_StartVerMac,
 *                     TSA_SOPT_VisaPEDMode,
 *                     TSA_SOPT_FinKeyOption
 *
 * Output:
 *  uchar * setting    returns 0 or 1 for the option being inquired
 *                     0 or 1 for each option is set according to 
 *                     the key injection spec
 *
 * Return value:
 *  TSA_OK = successful
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_OPTION = invalid security option
 *  TSA_KO = other error
 */
int TSA_GetSecOption (uchar option, uchar * setting);


/**
 * TSA_NewKeyNotification - check if the IAM message is a New Key Notification
 *
 * Input:
 *  S_MESSAGE_IAM *param_in     IAM input parameter from message_received
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TRUE: it is a New Key Generated Notification
 *  FALSE: it is NOT a New Key Generated Notification
 */
bool TSA_NewKeyNotification(S_MESSAGE_IAM *param_in);

/**
 * TSA_RegisterLockdownApp - registers an application type with TSA, so that it will not be deleted by TSA as
 *                           result of Lockdown enforcement.  Note that an application must register itself with
 *                           the TSA within a minute of terminal reset for the registration to take effect.  If the
 *                           registration is not done within the time limit, TSA will automatically delete it from
 *                           the terminal.
 *
 * Input:
 *  appFamily    - The Application Family name assigned to the application registering for the Lockdown function.
 *  lockdownInfo - The name of the Lockdown Info being enforced by TSA.
 *
 * Output:
 *  NIL
 *
 * Return value:
 *  TSA_OK - successful registration.
 *  TSA_APP_NOT_READY = TSA is not ready yet, retry later
 *  TSA_ERR_INP_DATA = error in processing input data
 *  TSA_KO = other error
 */
int TSA_RegisterLockdownApp( const char* appFamily, const char* lockdownInfo );

#endif
