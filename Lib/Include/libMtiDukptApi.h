#ifndef INC_LIB_MTI_DUKPT_API_H_
#define INC_LIB_MTI_DUKPT_API_H_

#include "libMtiCommonApi.h"

#ifdef __INGENICO_SRC__

#include "sdk_tplus.h"
#include "schVar_def.h"
#include "tlvVar_def.h"

#include "SEC_interface.h"
#ifndef __TETRA__
#include "SEC_AESHMacinterface.h"
#include "SEC_DataCipherinterface.h"
#include "SEC_ExtDukptinterface.h"
#endif

#define VAR_NUMBER_SIGN 			0x010D
#define CARD_NUMBER_SIGN 			0x0003C00E
//#define ID_SCR_MTI_BANK             0x023F2030 /* SECRET AREA ID TO USE       , 0x0f012130 backup  */
//#define ID_SCR_MTI_BANK             0x00042133 /* SECRET AREA ID TO USE       , 0x0f012130 backup  */
#define ID_SCR_MTI_BANK             0x00042133 /* SECRET AREA ID TO USE       , 0x0f012130 backup  */
#define ID_BANK_MTI                 0x80000606 /* BANK ID TO USE                  , BK_SAGEM backup   */
#define ROOT_KEY_NO_MTI             1008       /* ROOT KEY NUMBER */          
#define MASTER_SERIAL_KEY_NO_MTI	24         /* MASTER KEY SERIAL NUMBER , 8  backup */

#define ENCKEY_LOC			        4          /* data fc */

//#ifndef INITECH_DUKPT_FOR_INGENICO
//	#define INITECH_DUKPT_FOR_INGENICO
//#endif

#elif  __VERIFONE_SRC__
#include <log/liblog.h>             /* ADK LOGGING */
#include <svc_sec.h>
#include <errno.h>

#ifdef _VRXEVO
#include <stdlib.h>
/* #include <stdint.h> */
#else
#include <vficom.h>
#endif


#define IPP_BUFFER_SIZE			300
#define IPP_COM1_DEVICE			2 /* It is an alternative for PINPAD_INTERNAL & PINPAD_EXTERNAL */

#define IPP_SEND_TIMEOUT		3
#define IPP_RECV_TIMEOUT		3

#define CRC_MASK        0x1021   /* x^16 + x^12 + x^5 + x^0 */

/* Protocol Characters */
#define COMM_CHAR_SI			(unsigned char)0x0F	/* shift in */
#define COMM_CHAR_SO			(unsigned char)0x0E	/* shift out */
#define COMM_CHAR_STX			(unsigned char)0x02
#define COMM_CHAR_ETX			(unsigned char)0x03
#define COMM_CHAR_EOT			(unsigned char)0x04	/* end of transaction */
#define COMM_CHAR_ACK			(unsigned char)0x06	/* acknowledge */
#define COMM_CHAR_NAK			(unsigned char)0x15	/* not acknowledge */

#define HID01                   1   /* MSK DUKPT */
#define HID03                   3   /* MSK DUKPT */

/* KEYSET IDs  */
#define KEYSET01                1

#endif


#define ECHO_PIN '*'

#define PIN_ENTRY_MIN		4
#define PIN_ENTRY_MAX		12
#define PIN_BLOCK_LENGTH	8
#define PIN_ENTRY_TIMEOUT    (30 * 1000)
#define PIN_ENTRY_INTERVAL   (10 * 1000)

#define DUKPT_KSN_LENGTH	10
#define DUKPT_KSN_ID_LENGTH	7
#define DUKPT_TDES_LENGTH	16
#define DEF_DUKPY_MAC_SIZE	4
#define DUKPT_TIMEOUT		5000

#define RTN_DUKPT_OK		                 0
#define RTN_DUKPT_CANCEL	                -1
#define RTN_DUKPT_TIMEOUT	                -2
#define RTN_DUKPT_ERROR		                -3
#define RTN_DUKPT_SEND_ERROR		        -4
#define RTN_DUKPT_RECEIVE_ERROR             -5
#define RTN_DUKPT_PIN_SIZE_ERR	            -6
#define RTN_DUKPT_PIN_ENTRY_ERR	            -7
#define RTN_DUKPT_KSN_ID_ERROR	            -8
#define RTN_DUKPT_KSN_SEQ_ERROR	            -9
#define RTN_DUKPT_NO_KEY				        -10
#define RTN_DUKPT_PIN_ENCRYPT_ERR	        -11
#define RTN_DUKPT_SCRET_AREA_FAIL	        -18
#define RTN_DUKPT_NOT_WORK			        -19
#define RTN_DUKPT_IPP_NO_KEY_ERR	        -21
#define RTN_DUKPT_IPP_ACCOUNT_ERR	        -22
#define RTN_DUKPT_IPP_PIN_TOO_LONG	        -23
#define RTN_DUKPT_IPP_PIN_TOO_SHORT	        -24
#define RTN_DUKPT_IPP_USE_MS		        -25
#define RTN_DUKPT_IPP_MAX_CNT_ERR	        -26
#define RTN_DUKPT_IPP_UNKNOWN_ERR	        -27

#define RTN_DUKPT_GET_RANDOM_ERROR          -40

#define RTN_DUKPT_NEED_KEYINJECTION         -50
#define RTN_DUKPT_EXIST_ENCKEY              -51
#define RTN_DUKPT_PAN_ENCKEY_VERIFY_ERROR   -52
#define RTN_DUKPT_DEVICE_OPEN_ERROR         -55
#define RTN_DUKPT_SCRIPT_STATUS_ERROR       -56
#define RTN_DUKPT_EXEC_SCRIPT_ERROR         -57
#define RTN_DUKPT_SCRIPT_LOAD_ERROR         -58
#define RTN_DUKPT_INVALID_SCRIPT            -59


#define RTN_DUKPT_PIN_BYPASSED		        -99


//Enc callback function
typedef int (*tEnc_cb)();

#ifdef __cplusplus
extern "C" {
#endif


int mtiMskInit(void);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function load Encrypt key of PAN.
//
//----------------------------------------------------------------------------
// Parameters : none
//
// return : 
//////////////////////////////////////////////////////////////////////////////
int mtiPanLoadEncryptKey (void);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function decrypt PAN with ENC key.
//----------------------------------------------------------------------------
// Parameters 
//            [in] 	: ucpEncMsg 	: cipher message buffer to decrypt
//            [in]	: iMsgLen 	: Length in bytes of the Input cipher message buffer 
// 							    (it must be multiple of 8 and max is 512) 
//            [out]	: ucpOut		: decrypted plain output Data buffer 
//            [out]	: ipOutLen	: decrypted plain output Data length
//
// return : OK is success, otherwise error

// return : OK is VS version information
//////////////////////////////////////////////////////////////////////////////
int mtiPanDecryptData (unsigned char *ucpEncMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function encrypt PAN with ENC key.
//----------------------------------------------------------------------------
// Parameters 
//            [in] 	: ucpMsg 	: plain message buffer to encrypt
//            [in]	: iLength 	: Length in bytes of the Input plain message buffer 
// 							    (it must be multiple of 8 and max is 512) 
//            [out]	: ucpOut		: encrypted output Data buffer 
//            [out]	: ipOutLen	: encrypted output Data length
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiPanEncryptData (unsigned char *ucpMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen);
/* 2017-02-21, INTECH ADD END */

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function get VD version information.
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is VS version information
//////////////////////////////////////////////////////////////////////////////
void mtivdGetVersions();

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function load Root key of DUKPT.
//
//----------------------------------------------------------------------------
// Parameters : none
//
// return : 
//////////////////////////////////////////////////////////////////////////////
int mtiDukptLoadRootKey(void);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function is checking that DUKPT is work or not.
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is DUKPT work, otherwise does not work
//////////////////////////////////////////////////////////////////////////////
int mtiDukptIsWork(void);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function initialize DUKPT - Load KSN & IPEK from secure area
// The KSN & IPEK was injected by ski-9000(FutureX)
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptInit(void);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function is used to manage the secure Pincode entry. 
// Inputed PIN's are stored in secure area
//----------------------------------------------------------------------------
// Parameters [in/out] 	: tpDispCont 	: tDispContents list contains sereen design
//            [in]	: iDispContCnt 	:  tDispContents count
//            [in]	: iSecLength	: Max PIN length 
//            [in]	: tpAddCtrl	: after input, display control to confirm
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptPinEntry(tDispContents *tpDispCont, int iDispContCnt, int iSecLength, tAddCtrlContents *tpAddCtrl);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function is used to encrypt the PIN with DES/TDES DUKPT algorithm; The supported 
//    PIN block formats are formats 0.
// The PIN has been entered previously on the secure Pincode part by mtiDukptPinEntry()
//----------------------------------------------------------------------------
// Parameters [in] 	: cpPan 	: PAN number buffer (charactor type)
//            [out]	: ucpOut 	: ciphered Pin Block (format 0) 
//            [out]	: ipOutLen	: ciphered Pin Block length 
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptPinEncrypt(char *cpPan, unsigned char *ucpOut, int *ipOutLen);

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function is used to input PIN and encrypt the PIN with DES/TDES DUKPT algorithm; 
// The supported PIN block formats are formats 0.
//----------------------------------------------------------------------------
// Parameters [in/out] 	: tpDispCont 	: tDispContents list contains sereen design
//            [in]	: iDispContCnt 	:  tDispContents count
//            [in]	: iSecLength	: Max PIN length 
//            [in]	: tpAddCtrl	: after input, display control to confirm
//            [in] 	: cpPan 	: PAN number buffer (charactor type)
//            [out]	: ucpOut 	: ciphered Pin Block (format 0) 
//            [out]	: ipOutLen	: ciphered Pin Block length 
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptPinEntryEncrypt(tDispContents *tpDispCont, int iDispContCnt, int iSecLength, tAddCtrlContents *tpAddCtrl, char *cpPan, unsigned char *ucpOut, int *ipOutLen);

int mtiEncCallbackReg(tEnc_cb cb);

#ifdef __cplusplus
}
#endif

#endif
