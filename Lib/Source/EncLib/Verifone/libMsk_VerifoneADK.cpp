//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// DUKPT Library for Verifone ADK
//----------------------------------------------------------------------------
// 
// Project : MTI EDC Application
// Module  : DUKPT function implementation for PIN, MSG & MAC
//           with ADK-SEC module
// 
// @file   : libDukpt_VerifoneDK.c
// 
// @date   : 06/02/2017
// @Author : Allen Kim
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

#include "libMtiDukptApi.h"
#include <sec/libseccmd.h>
#include <html/gui.h>
#include "libAdkDisplay.h"
#include "libMtiCommonApi.h"


#define DEF_TRC_TRACE
#define DEF_DATA_TRACE

#define MTI_CLEAR_KLK   "\xAB\xCD\xEF\xAB\xCD\xEF\xAB\xCD\x12\x34\x56\x78\x90\x12\x34\x56"
#define MTI_SUBDEV   0x00

#define MTI_SET_ENC_FLAG	0x95
#define MTI_GET_ENC_FLAG	0x98

char MTI_VSS_name[] = "MTI10112.vso";   // PAN Encrypt Script

unsigned char ucInputData [512+1];
unsigned char ucOutputData [512+1];

using namespace com_verifone_seccmd;
int _mtiPanCheckEncryptKey (void);
static int panEncTest (void);

static tEnc_cb enc_cb = NULL;

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function get VD version information.
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is VS version information
//////////////////////////////////////////////////////////////////////////////
void mtivdGetVersions()
{
	int         iRET;
	uint8_t     errCode = 0;
	std::string versions;
	const char  *APIVersion;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtivdGetVersions() => START");
#endif

	iRET = Sec_GetVersions(&versions, &errCode, DUKPT_TIMEOUT);
#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtivdGetVersions() => Sec_GetVersions() [ret:%d,errcode:%ld]", iRET, errCode);
#endif

	if (iRET == RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE	
	dmsg("[FUNC] mtivdGetVersions() => Version Info: \"%s\"", versions.c_str());
#endif
	}
	else
	{
#ifdef DEF_TRC_TRACE	
		dmsg("[FUNC] mtivdGetVersions() => Sec_GetVersions() failed...(%d)", iRET);
#endif	
		return;
	}

	APIVersion = Sec_GetVersion();
#ifdef DEF_TRC_TRACE 
	dmsg("[FUNC] mtiDukptMakeRandom() => Sec_GetVersion() Lib Version: \"%s\"", APIVersion);
	dmsg("[FUNC] mtiDukptMakeRandom() => END");
#endif
	iRET = RTN_DUKPT_OK;

	return;
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function is checking that DUKPT is work or not.
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is DUKPT work, otherwise does not work
//////////////////////////////////////////////////////////////////////////////
int mtiDukptIsWork(void)
{
	int iRET;

	iRET = RTN_DUKPT_OK;
#if 0	
	T_SEC_DATAKEY_ID stDukptKeyID;
	int iStatus;

#ifdef DEF_TRC_TRACE
    dmsg("[mtiDukptIsWork()] START");
#endif  /* #ifdef DEF_TRC_TRACE */

	stDukptKeyID.iSecretArea = ID_SCR_MTI_BANK;
	stDukptKeyID.cAlgoType = TLV_TYPE_TDESDUKPT;		/* This key is a TDES Key */
	stDukptKeyID.usNumber = MASTER_SERIAL_KEY_NO_MTI;
	stDukptKeyID.uiBankId = ID_BANK_MTI;

	iRET = SEC_DukptStatus(C_SEC_PINCODE, &stDukptKeyID, &iStatus);
	if (iRET == OK)
	{
		if (iStatus == 0)
		{
#ifdef DEF_TRC_TRACE
			dmsg("[mtiDukptIsWork()] DUKPT works");
#endif  /* #ifdef DEF_TRC_TRACE */
		}
		else
		{
#ifdef DEF_TRC_TRACE
			dmsg("[mtiDukptIsWork()] DUKPT does not work");
#endif  /* #ifdef DEF_TRC_TRACE */
			iRET = iStatus;
		}
	}
#ifdef DEF_TRC_TRACE
	else
		dmsg("[mtiDukptIsWork()] SEC_DukptStatus() was failed");

	dmsg("[mtiDukptIsWork()] SEC_DukptStatus ret = %d, sta = %d", iRET, iStatus);
#endif  /* #ifdef DEF_TRC_TRACE */

#endif

	return (iRET);
}

//----------------------------------------------------------------------------
// This function load Root key of DUKPT.
//
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptLoadRootKey(void)
{	
	int iRET = -1;
	unsigned char ucpMSK[16+1];
	unsigned char ucBuffer[60];
	
#ifdef _VRXEVO
	int hCrypto;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptLoadRootKey() => START Open the terminal Crypto Device");
#endif
	hCrypto = open("/DEV/CRYPTO", O_RDWR);
	if (hCrypto < 0)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiDukptLoadRootKey() => open() Unable to open Crypto Device [ret:%d,errno:%ld]", hCrypto, errno);
#endif
		return RTN_DUKPT_DEVICE_OPEN_ERROR;
	}
#ifdef DEF_TRC_TRACE	
	dmsg("[FUNC] mtiDukptLoadRootKey() => Crypto Device opened");
#endif

#endif  /* #ifdef _VRXEVO */

	mtiMemset(ucBuffer,0,sizeof(ucBuffer));

	iRET = iPS_DeleteKeys(DEL_VSS0);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE		
		dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_DeleteKeys failed(ret:%d)", iRET);
#endif
	}
#ifdef DEF_TRC_TRACE	
	dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_DeleteKeys........OK");
#endif

	errno = 0;
	iRET = iPS_LoadSysClearKey((unsigned char ) 0x00, (unsigned char *) MTI_CLEAR_KLK); // or any random value
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE		
		dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_LoadSysClearKey failed[ret:%d,errno:%ld]", iRET, errno);
		dmsg("[FUNC] mtiDukptLoadRootKey() => Unable to set system key ..............FAIELD");
#endif

	}
#ifdef DEF_TRC_TRACE	
		dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_LoadSysClearKey ..... OK");
#endif

	errno = 0;
	iPS_UninstallScript (MTI_SUBDEV);
	iRET = iPS_InstallScript(MTI_VSS_name); //For VOS the script installation is done by the Secure Installer. It is not needed to call this function
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE		
		dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_InstallScript [%s] failed [ret:%d,errno:%ld]", MTI_VSS_name, iRET, errno);
#endif

	}
	else
	{
#ifdef DEF_TRC_TRACE	
		dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_InstallScript [%s] .......OK(errno:%ld)", MTI_VSS_name, errno);
#endif
	}

	mtiMemset (ucpMSK, 0,16+1);
	mtiGetRandom(ucpMSK, 16);
	errno = 0;
	iRET = iPS_LoadMasterClearKey(MTI_SUBDEV, 0, ucpMSK);
	if( iRET != RTN_DUKPT_OK )
	{
#ifdef DEF_TRC_TRACE		
		dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_LoadMasterClearKey(0) failed [ret:%d,errno:%ld]", iRET, errno);
#endif
	}
	else
	{
#ifdef DEF_TRC_TRACE		
		dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_LoadMasterClearKey(0) Success");
#endif	
		errno = 0;
		iRET = iPS_LoadMasterClearKey(MTI_SUBDEV, 1, &ucpMSK[8]);	
		if( iRET != RTN_DUKPT_OK )
		{
#ifdef DEF_TRC_TRACE		
			dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_LoadMasterClearKey(1) failed [ret:%d,errno:%ld]", iRET, errno);
#endif
		}
		else
		{
#ifdef DEF_TRC_TRACE		
			dmsg("[FUNC] mtiDukptLoadRootKey() => iPS_LoadMasterClearKey(1) Success");
#endif		
		}
	}
	
#ifdef _VRXEVO
	close(hCrypto);
	hCrypto = -1;
#endif

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptLoadRootKey() => END(%d)", iRET);
#endif

	return (iRET);
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function initialize DUKPT - Load KSN & IPEK from secure area
// The KSN & IPEK was injected by ski-9000(FutureX)
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiMskInit(void)
{
	int iRET = -1;
	int iPanKeyChkRet = 0;
	uint8_t errCode=0;
	char ucAppName[8] = "MTI_SEC";
	unsigned char ucHosId = HID01;
	unsigned int ucKeySetId = KEYSET01;

#ifdef DEF_TRC_TRACE		
	dmsg("[FUNC] mtiMskInit() => START");
#endif
	
	iRET = Sec_Init((unsigned char*)ucAppName);
	if( iRET == RTN_DUKPT_OK )
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiMskInit() => Sec_Init API OK");
#endif
		iRET = Sec_SetKSId(ucHosId, ucKeySetId);
		if (iRET != RTN_DUKPT_OK)
		{
	#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] mtiPanEncryptData() => Sec_SetKSId() failed... (%d)", iRET);
	#endif
			return iRET;
		}

		iRET = Sec_SelectHostConf(ucHosId, &errCode, 30000);
		dmsg("Sec_SelectHostConf iRET[%d] errCode[%d]", iRET, errCode);
		if (iRET == 0)
		{
			iRET = RTN_DUKPT_OK;
			iPanKeyChkRet = _mtiPanCheckEncryptKey();
			if(iPanKeyChkRet != RTN_DUKPT_EXIST_ENCKEY)
			{
				// Inject New Key
                // checking records
				// remove records and warning.
				// Call Callback Function (iRecordConfirmAndRemove)

				if(enc_cb != NULL);
				{
					enc_cb();
				}
				mtiDukptLoadRootKey();
				iPanKeyChkRet = mtiPanLoadEncryptKey();
				if(iPanKeyChkRet != RTN_DUKPT_OK)
					iRET = RTN_DUKPT_NOT_WORK;
				if(panEncTest() != RTN_DUKPT_OK)
					iRET = RTN_DUKPT_NOT_WORK;
			}
		}
		else
			iRET = RTN_DUKPT_NOT_WORK;
	}
	else
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiMskInit() => Sec_Init API failed (RET:%d)",iRET);
#endif
		iRET = RTN_DUKPT_SCRET_AREA_FAIL;
	}

#ifdef DEF_TRC_TRACE		
	dmsg("[FUNC] mtiMskInit() => END(%d)", iRET);
#endif	
	return (iRET);
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function Load DUKPT - Load KSN & IPEK from secure area
// The KSN & IPEK was injected by ski-9000(FutureX)
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptLoad(void)
{
	int iRET = 0;
	int hCrypto = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptLoad() => START");
#endif	
	iRET = mtiDukptLoadRootKey();
	if (iRET != RTN_DUKPT_OK)
	{
		return RTN_DUKPT_ERROR;
	}		
	
	iRET = mtiMskInit();
	if (iRET != RTN_DUKPT_OK)
	{
		dmsg(" DUKPT TESTING]mtiMskInit() error --> %d", iRET);
		return RTN_DUKPT_ERROR;
	}	
	
	iRET = mtiPanLoadEncryptKey();
	if (iRET != RTN_DUKPT_OK)
	{
		dmsg(" DUKPT TESTING]mtiPanLoadEncryptKey() error --> %d", iRET);
	}

	//Sec_Close();
#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptLoad() => END (%0d)", iRET);
#endif


	return (iRET);
}

int mtiDukptPinEntry(tDispContents *tpDispCont, int iDispContCnt, int iSecLength, tAddCtrlContents *tpAddCtrl)
{
	int iRET = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptPinEntry() => Verifone Not Support..SKIP");
#endif	

	return (iRET);
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function increase KSN of DUKPT library
// Because Ingenico's library does not support KSN control function so that forced execute
//     SEC_DukptECBCipher() function with C_EXTDUKPT_FORCED_MODE
//----------------------------------------------------------------------------
// Parameters : none
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptIncreaseKsn(unsigned char *ucpOut)
{
	int iRET = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptIncreaseKsn() => Verifone Not Support..SKIP");
#endif	

	return (iRET);
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function encrypt message with DUKPT MSG key.
// Because this function use C_EXTDUKPT_AUTO_MODE option inside, KSN is not increased
//----------------------------------------------------------------------------
// Parameters [in] 	: ucpMsg 	: plain message buffer to encrypt
//            [in]	: iLength 	: Length in bytes of the Input plain message buffer 
// 							    (it must be multiple of 8 and max is 512) 
//            [out]	: ucpOut		: encrypted output Data buffer 
//            [out]	: ipOutLen	: encrypted output Data length
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptEncryptData(unsigned char *ucpMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptEncryptData() => Verifone Not Support..SKIP");
#endif	

	return (iRET);
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function decrypt message with DUKPT MSG key.
// Because this function use C_EXTDUKPT_AUTO_MODE option inside, KSN is not increased
//----------------------------------------------------------------------------
// Parameters [in] 	: cpKsn 	: KSN which was received from Server
//            [in] 	: ucpEncMsg : ciphered message buffer to decrypt
//            [in]	: iLength 	: Length in bytes of the ciphered message buffer
//  							   (it must be multiple of 8 and max is 512) 
//            [out]	: ucpOut	: decrypted plain output Data buffer
//            [out]	: ipOutLen	: decrypted plain output Data length
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptDecryptData(char *cpKsn, unsigned char *ucpEncMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptDecryptData() => Verifone Not Support..SKIP");
#endif	

	return (iRET);
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This function generate 4bytes MAC with DUKPT MAC key.
// Because this function use C_EXTDUKPT_AUTO_MODE option inside, KSN is not increased
//----------------------------------------------------------------------------
// Parameters [in] 	: ucpMsg 	: input message buffer to compute MAC
//            [in]	: iMsgLen 	: Length in bytes of the input message buffer
//            [out]	: ucpMac		: computed MAC Data buffer (4 bytes)
//
// return : OK is success, otherwise error
//////////////////////////////////////////////////////////////////////////////
int mtiDukptCbcMAC(unsigned char *ucpMsg, int iMsgLen, unsigned char *ucpMac)
{
	int iRET = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptCbcMAC() => Verifone Not Support..SKIP");
#endif	

	return (iRET);
}

#if 0   /* 2017-02-27 TEST */
/* 2017-02-21, INTECH ADD START */
int mtiPanLoadEncryptKey (void)
{
	int iRET = 0;
	uint8_t errCode=0;
	unsigned char ucHosId = HID01;
	unsigned char ucpEncKey [16+1];

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiPanLoadEncryptKey() => START");
#endif	

	mtiMemset(ucpEncKey, 0x00, 16+1);
	mtiGetRandom(ucpEncKey,16);
	iRET = Sec_UpdateKey(ucHosId, KEY_TYPE_TPK_FOR_ENC_DATA, (char *)ucpEncKey, (u_long)16, &errCode, DUKPT_TIMEOUT);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanLoadEncryptKey(KEY_TYPE_TPK_FOR_ENC_DATA) => Sec_UpdateKey() failed... (%d)", iRET);
#endif	
	}

	iRET = Sec_UpdateKey(ucHosId, KEY_TYPE_TPK_FOR_DEC_DATA, (char *)ucpEncKey, (u_long)16, &errCode, DUKPT_TIMEOUT);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanLoadEncryptKey() => Sec_UpdateKey(KEY_TYPE_TPK_FOR_DEC_DATA) failed... (%d)", iRET);
#endif	
	}

	return (iRET);
}

#else
int _mtiPanSetEncryptKeyFlag(void)
{
	INT iRET = 0;
	unsigned char ucBuffer[30];
	unsigned short iBufferLength = 0;

#ifdef _VRXEVO
	int hCrypto;
#endif

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] _mtiPanSetEncryptKeyFlag() => START");
#endif

#ifdef _VRXEVO
	hCrypto = open("/DEV/CRYPTO", O_RDWR);
	if (hCrypto < 0)	return RTN_DUKPT_DEVICE_OPEN_ERROR;
#endif

	mtiMemset(ucBuffer,0,sizeof(ucBuffer));
	/* Encrypt Key flag Check */
	iRET = iPS_ExecuteScript(MTI_SUBDEV, MTI_SET_ENC_FLAG, 0, ucBuffer, 30, &iBufferLength, ucBuffer);
	if ((iRET != RTN_DUKPT_OK) &&(iRET!=52))
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] _mtiPanSetEncryptKeyFlag() => iPS_ExecuteScript(MTI_SET_ENC_FLAG) failed..(%d)", iRET);
#endif
		return RTN_DUKPT_EXEC_SCRIPT_ERROR;
	}
	else
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] _mtiPanSetEncryptKeyFlag() => iPS_ExecuteScript(MTI_SET_ENC_FLAG) Success");
#endif
		iRET = RTN_DUKPT_OK;
	}

#ifdef _VRXEVO
	close(hCrypto);
	hCrypto = -1;
#endif

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] _mtiPanSetEncryptKeyFlag() => END (%d)", iRET);
#endif	

	return (iRET);
}
int _mtiPanCheckEncryptKey (void)
{
	INT iRET = 0;
	uint8_t errCode=0;
	unsigned char ucBuffer[30];
	unsigned short iBufferLength = 0;
#ifdef _VRXEVO
	int hCrypto;
#endif

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] _mtiPanCheckEncryptKey() => START");
#endif

#ifdef _VRXEVO
	hCrypto = open("/DEV/CRYPTO", O_RDWR);
	if (hCrypto < 0)	return RTN_DUKPT_DEVICE_OPEN_ERROR;
#endif

	/* Check VSS Script */
	iRET = iPS_GetScriptStatus(MTI_SUBDEV, ucBuffer);
	if (iRET == RTN_DUKPT_OK)
	{
		if (mtiMemcmp(MTI_VSS_name,ucBuffer,8) != RTN_DUKPT_OK)
		{
#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] _mtiPanCheckEncryptKey() => iPS_GetScriptStatus() failed... VSS inValid");
#endif		
#ifdef _VRXEVO
			close(hCrypto);
			hCrypto = -1;
#endif

			return RTN_DUKPT_INVALID_SCRIPT;
		}
		
		mtiMemset(ucBuffer,0,sizeof(ucBuffer));
		/* Encrypt Key flag Check */
		iRET = iPS_ExecuteScript(MTI_SUBDEV, MTI_GET_ENC_FLAG, 0, ucBuffer, 30, &iBufferLength, ucBuffer);
		if (iRET == RTN_DUKPT_OK)  // 'return 52' is Not Found VSS register value (Empty)
		{
#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] _mtiPanCheckEncryptKey() => iPS_ExecuteScript(MTI_GET_ENC_FLAG) Success");
			dbuf("Encrypt Key Flag", ucBuffer, 8);
#endif
			if( (ucBuffer[0] == 0xAA) && (ucBuffer[1] == 0xAA) && (ucBuffer[7] == 0xAA))
			{
#ifdef DEF_TRC_TRACE
				dmsg("[FUNC] _mtiPanCheckEncryptKey() => ");
#endif			
#ifdef _VRXEVO
				close(hCrypto);
				hCrypto = -1;
#endif
				//Test Encryption
				if(panEncTest() == RTN_DUKPT_OK)
				{
					iRET = RTN_DUKPT_EXIST_ENCKEY;
				}
				else
				{
					iRET = RTN_DUKPT_OK;
				}

			}
			else
			{
				iRET = RTN_DUKPT_OK;
			}
		}
		else if (iRET == 52)
		{
#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] _mtiPanCheckEncryptKey() => Not found Pan Encrypt Key");
#endif		
			iRET = RTN_DUKPT_OK;
		}
		else
		{					
#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] _mtiPanCheckEncryptKey() => iPS_ExecuteScript(MTI_GET_ENC_FLAG) failed..(%d)", iRET);
#endif
			return RTN_DUKPT_EXEC_SCRIPT_ERROR;
		}
	}
	else if (iRET == E_VS_SCRIPT_NOT_LOADED)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] _mtiPanCheckEncryptKey() => iPS_GetScriptStatus() failed...Script is not loaded or is not accessible from the current GID  (%d)", iRET);
#endif	
		iRET = RTN_DUKPT_SCRIPT_LOAD_ERROR;
	}
	else
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] _mtiPanCheckEncryptKey() => iPS_GetScriptStatus() failed... VSS Mising (%d)", iRET);
#endif	
		iRET = RTN_DUKPT_SCRIPT_STATUS_ERROR;
	}
	
#ifdef _VRXEVO
	close(hCrypto);
	hCrypto = -1;
#endif

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] _mtiPanCheckEncryptKey() => END (%d)", iRET);
#endif	

	return (iRET);
}

int mtiPanLoadEncryptKey (void)
{
	INT iRET = RTN_DUKPT_OK;
	uint8_t errCode=0;
	unsigned char ucHosId = HID01;
	unsigned int ucKeySetId = KEYSET01;
	unsigned char ucpEncKey [16+1];
	
#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiPanLoadEncryptKey() => START");
#endif

	/* If return code RTN_DUKPT_OK, PAN Encrypt Key Insert */
	iRET = _mtiPanCheckEncryptKey();
	if (iRET == RTN_DUKPT_OK)
	{
		iRET = Sec_SetKSId(ucHosId, ucKeySetId);
		if (iRET != RTN_DUKPT_OK)
		{
	#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] mtiPanEncryptData() => Sec_SetKSId() failed... (%d)", iRET);
	#endif
			return iRET;
		}

		mtiMemset(ucpEncKey, 0x00, 16+1);
		mtiGetRandom(ucpEncKey,16);
		iRET = Sec_UpdateKey(ucHosId, KEY_TYPE_TPK_FOR_ENC_DATA, (char *)ucpEncKey, (u_long)16, &errCode, DUKPT_TIMEOUT);
		if ((iRET != RTN_DUKPT_OK) && (iRET != RTN_DUKPT_PAN_ENCKEY_VERIFY_ERROR))
		{
#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] mtiPanLoadEncryptKey() => Sec_UpdateKey(KEY_TYPE_TPK_FOR_ENC_DATA) failed... (%d)", iRET);
#endif	
		}
		else
		{
#ifdef DEF_TRC_TRACE
			dmsg("[FUNC] mtiPanLoadEncryptKey() => Sec_UpdateKey(KEY_TYPE_TPK_FOR_ENC_DATA) Success errCode[%d]",errCode);
#endif		
			iRET = Sec_UpdateKey(ucHosId, KEY_TYPE_TPK_FOR_DEC_DATA, (char *)ucpEncKey, (u_long)16, &errCode, DUKPT_TIMEOUT);
			if (iRET != RTN_DUKPT_OK)
			{
#ifdef DEF_TRC_TRACE
				dmsg("[FUNC] mtiPanLoadEncryptKey() => Sec_UpdateKey(KEY_TYPE_TPK_FOR_DEC_DATA) failed... (%d)", iRET);
#endif	
			}
			else
			{
#ifdef DEF_TRC_TRACE
				dmsg("[FUNC] mtiPanLoadEncryptKey() => Sec_UpdateKey(KEY_TYPE_TPK_FOR_DEC_DATA) Success errCode[%d]",errCode);
#endif			
			}

			_mtiPanSetEncryptKeyFlag();
		}
	}
	else if (iRET == RTN_DUKPT_EXIST_ENCKEY)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanLoadEncryptKey() => PAN Encrypt Key exist...SKIP");
#endif	
		iRET = RTN_DUKPT_OK;
	}
	else
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanLoadEncryptKey() => _mtiPanCheckEncryptKey() faile... (%d)", iRET);
#endif		
	}
	
#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiPanLoadEncryptKey() => END (%d)", iRET);
#endif
	return (iRET);
}

#endif  /* 2017-02-27 TEST */

int mtiPanDecryptData (unsigned char *ucpEncMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET = 0;
	EncDecDataIV Data;
	uint8_t errCode=0;
	unsigned char ucHosId = HID01;
	unsigned int ucKeySetId = KEYSET01;

	/*
	unsigned char ucInputData [512+1];
	unsigned char ucOutputData [512+1];

	*/
	Ksn tKsn;
	INT iLength = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiPanDecryptData() => START");
#endif	

	iRET = Sec_SetKSId(ucHosId, ucKeySetId);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanDecryptData() => Sec_SetKSId() failed... (%d)", iRET);
#endif	
		return iRET;
	}

	mtiMemset(ucInputData, 0x00, 512+1);
	mtiMemset(ucOutputData, 0x00, 512+1);
	mtiMemset(&Data, 0x00, sizeof(Data));
	mtiMemset(&tKsn, 0x00, sizeof(tKsn));
	
	Data.pInData = ucInputData;
	memcpy(Data.pInData, ucpEncMsg, iMsgLen);
	Data.pInData = ucpEncMsg;
	
	Data.uiInLen = iMsgLen;
	Data.pOutData= ucOutputData;
	Data.uiOutLen = 512+1;
	
	Data.IV = NULL;
	Data.uiIVLen  = 0;
	  
	errCode=0;

	iRET = Sec_DecryptData(ucHosId, &Data, &tKsn, &errCode, DUKPT_TIMEOUT);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanDecryptData() => Sec_DecryptData() failed... (%d)", iRET);
#endif	
	}
	else
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanDecryptData() => Sec_DecryptData() Success");
		dbuf("Decrypt", Data.pOutData, Data.uiOutLen);
		mtiMemcpy(ucpOut, Data.pOutData,Data.uiOutLen);
		*ipOutLen = Data.uiOutLen;
#endif	
	}
	iLength = *ipOutLen;
	while (iLength > 0)
	{
		if (ucpOut[iLength-1] != 0x00)
			break;
		iLength--;
	}
	*ipOutLen = iLength;

	return (iRET);
}

int mtiPanEncryptData (unsigned char *ucpMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET = 0;
	EncDecDataIV Data;
	uint8_t errCode=0;
	unsigned char ucHosId = HID01;
	unsigned int ucKeySetId = KEYSET01;
	unsigned char ucInputData [512+1];
	unsigned char ucOutputData [512+1];
	Ksn tKsn;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiPanEncryptData() => START");
#endif	
	iRET = Sec_SetKSId(ucHosId, ucKeySetId);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanEncryptData() => Sec_SetKSId() failed... (%d)", iRET);
#endif	
		return iRET;
	}
	mtiMemset(ucInputData, 0x00, 512+1);
	mtiMemset(ucOutputData, 0x00, 512+1);
	mtiMemset(&Data, 0x00, sizeof(Data));
	mtiMemset(&tKsn, 0x00, sizeof(tKsn));
	
	Data.pInData = ucInputData;
	memcpy(Data.pInData, ucpMsg, iMsgLen);
	Data.pInData = ucpMsg;
	
	Data.uiInLen = iMsgLen;
	Data.pOutData= ucOutputData;
	Data.uiOutLen = 512+1;
	
	Data.IV = NULL;
	Data.uiIVLen  = 0;
	  
	errCode=0;
	//iRET = Sec_EncryptData(ucHosId, &Data, &errCode, DUKPT_TIMEOUT);
	iRET = Sec_EncryptData(ucHosId, &Data, &tKsn, &errCode, DUKPT_TIMEOUT);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanEncryptData() => Sec_EncryptData() failed... (%d) errCode[%d]", iRET, errCode);
#endif	
	}
	else
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiPanEncryptData() => Sec_EncryptData() Success, errCode[%d]",errCode);
		dbuf("Encrypt", Data.pOutData, Data.uiOutLen);
		mtiMemcpy(ucpOut, Data.pOutData,Data.uiOutLen);
		*ipOutLen = Data.uiOutLen;
#endif	
	}

	return (iRET);

}

int panEncTest (void)
{
	int iRET, iEncLen, iDecLen;
	unsigned char ucEncCardNo[30];
	unsigned char ucDecCardNo[30];
	unsigned char ucMtiTestPan[30] = {0,};

#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => START");
#endif

#if 1
	memcpy(ucMtiTestPan, "1111222233334444555", strlen("1111222233334444555"));
	mtiMemset(ucEncCardNo, 0x00, sizeof(ucEncCardNo));
	mtiMemset(ucDecCardNo, 0x00, sizeof(ucDecCardNo));
	iEncLen = iDecLen = 0;
	iRET = mtiPanEncryptData(ucMtiTestPan, 16, ucEncCardNo, &iEncLen);
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => mtiPanEncryptData() has return [%d]", iRET);
	dbuf("            Plain card no:", ucMtiTestPan, 16);
	dbuf("            Cipher card no:", ucEncCardNo, iEncLen);
#endif
	if (iRET == RTN_DUKPT_OK)
	{
		iRET = mtiPanDecryptData(ucEncCardNo, iEncLen, ucDecCardNo, &iDecLen);
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => mtiPanDecryptData() has return [%d]", iRET);
	dbuf("            Cipher card no:", ucEncCardNo, iEncLen);
	dbuf("            Decrypt card no:", ucDecCardNo, iDecLen);
#endif
		if (iRET == RTN_DUKPT_OK)
		{
			if (mtiMemcmp(ucMtiTestPan, ucDecCardNo, iDecLen) == 0 && iDecLen == 16)
			{
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => END(%d)", RTN_DUKPT_OK);
#endif
			}
		}
	}
#endif

	mtiMemset(ucEncCardNo, 0x00, sizeof(ucEncCardNo));
	mtiMemset(ucDecCardNo, 0x00, sizeof(ucDecCardNo));
	iEncLen = iDecLen = 0;
	iRET = mtiPanEncryptData(ucMtiTestPan, mtiStrlen(19, ucMtiTestPan), ucEncCardNo, &iEncLen);
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => mtiPanEncryptData() has return [%d]", iRET);
	dbuf("            Plain card no:", ucMtiTestPan, mtiStrlen(19, ucMtiTestPan));
	dbuf("            Cipher card no:", ucEncCardNo, iEncLen);
#endif
	if (iRET == RTN_DUKPT_OK)
	{
		iRET = mtiPanDecryptData(ucEncCardNo, iEncLen, ucDecCardNo, &iDecLen);
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => mtiPanDecryptData() has return [%d]", iRET);
	dbuf("            Cipher card no:", ucEncCardNo, iEncLen);
	dbuf("            Decrypt card no:", ucDecCardNo, iDecLen);
#endif
		if (iRET == RTN_DUKPT_OK)
		{
			if (mtiMemcmp(ucMtiTestPan, ucDecCardNo, iDecLen) == 0 && iDecLen == 19)
			{
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => END(%d)", RTN_DUKPT_OK);
#endif
				return (RTN_DUKPT_OK);
			}
		}
	}

#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => END(%d)", RTN_DUKPT_PAN_ENCKEY_VERIFY_ERROR);
#endif

	return (RTN_DUKPT_PAN_ENCKEY_VERIFY_ERROR);
}
/* 2017-02-21, INTECH ADD END */

int mtiEncCallbackReg(tEnc_cb cb)
{
	enc_cb = cb;
}

