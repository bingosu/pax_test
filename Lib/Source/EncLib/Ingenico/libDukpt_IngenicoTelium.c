//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// DUKPT Library for INGENICO 
//----------------------------------------------------------------------------
// 
// Project : MTI EDC Application
// Module  : DUKPT function implementation for PIN, MSG & MAC
//           with Telium2 security module
// update   : remove MSG & MAC functions
//               add using DUPKT key which is injected from SKI-9000 by TSA scheme
// 
// @file   : libDukpt_IngenicoTelium.c
// 
// @date   : 22/01/2017
// @update   : 19/04/2017
// @Author : Allen Kim
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

#include "libMtiDukptApi.h"

#include "TSA_Def.h"
#include "TSA_api.h"

#if defined(INITECH_DUKPT_FOR_INGENICO)
#include "isemMTI.h"
#include "isemMTI_client.h"
#include "isemMTI_error.h"
#include "isemMTI_internal.h"
#include "isemMTI_version.h"
#include "isemMTI_dukpt.h"

#include "apMem.h"
#endif

unsigned char ucMtiTestPan[22] = "5573380123456780008";
unsigned char ucMtiTestKSN[22] = "FA000510000002000000";
unsigned char ucMtiTestRootKey[36] = "0123456789ABCDEF0123456789ABCDEF";

extern VOID mtiProcAddCtrl(tAddCtrlContents *tpAddCtrl);

// ///////////////////////////////////////////////////////////////////////////////////////////////////////
// PAN DATA Encrypt/Decrypt Operation

static tEnc_cb enc_cb = NULL;

int mtiPanEncryptData (unsigned char *ucpMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET;
	T_SEC_DATAKEY_ID stEncryptKeyID;

#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanEncryptData() => START");
#endif

	stEncryptKeyID.iSecretArea = ID_SCR_MTI_BANK;   /* 0x0f012130                     */
	stEncryptKeyID.cAlgoType   = TLV_TYPE_KTDES_16; /* This Encrypt key is a TDES Key */
	stEncryptKeyID.usNumber    = ENCKEY_LOC*8;      /* ENCKEY_LOC(4) * 8              */
	stEncryptKeyID.uiBankId    = ID_BANK_MTI;       /* BK_SAGEM (0x800000000)         */

	iRET = SEC_ECBCipher(&stEncryptKeyID, C_SEC_CIPHER_FUNC, ucpMsg, iMsgLen, ucpOut, (unsigned int *)ipOutLen);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_DUKPT_INIT_TRACE
		dmsg("[FUNC] mtiPanEncryptData() => SEC_ECBCipher()... Encrypt Failed... [%d]", iRET);
#endif
	}
	else
	{
#ifdef DEF_DUKPT_INIT_TRACE
		dmsg("[FUNC] mtiPanEncryptData() => SEC_ECBCipher()... Encrypt OK");
#endif
	}
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanEncryptData() => END (%d)", iRET);
#endif
	return (iRET);
}

int mtiPanDecryptData (unsigned char *ucpEncMsg, int iMsgLen, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET;
	int iLength;
	T_SEC_DATAKEY_ID stEncryptKeyID;

	dmsg("[FUNC] mtiPanDecryptData() => START");

	stEncryptKeyID.iSecretArea = ID_SCR_MTI_BANK;   /* 0x0f012130                     */
	stEncryptKeyID.cAlgoType   = TLV_TYPE_KTDES_16; /* This Encrypt key is a TDES Key */
	stEncryptKeyID.usNumber    = ENCKEY_LOC*8;      /* ENCKEY_LOC(4) * 8              */
	stEncryptKeyID.uiBankId    = ID_BANK_MTI;       /* BK_SAGEM (0x800000000)         */

	iRET = SEC_ECBCipher(&stEncryptKeyID, C_SEC_DECIPHER_FUNC, ucpEncMsg, iMsgLen, ucpOut, (unsigned int *)ipOutLen);
	if (iRET != RTN_DUKPT_OK)
	{
#ifdef DEF_DUKPT_INIT_TRACE
		dmsg("[FUNC] mtiPanDecryptData() => SEC_ECBCipher()... Decrypt Failed... [%d]", iRET);
#endif
	}
	else
	{
#ifdef DEF_DUKPT_INIT_TRACE
		dmsg("[FUNC] mtiPanDecryptData() => SEC_ECBCipher()... Decrypt OK");
#endif

		iLength = *ipOutLen;
		while (iLength > 0)
		{
			if (ucpOut[iLength-1] != 0x00)
				break;
			iLength--;
		}
		*ipOutLen = iLength;
	}
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanDecryptData() => END(%d)", iRET);
#endif
	return (iRET);
}

int mtiPanCheckEncryptKey (void)
{
	int iRET, iEncLen, iDecLen;
	unsigned char ucEncCardNo[30];
	unsigned char ucDecCardNo[30];

#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanCheckEncryptKey() => START");
#endif

#if 1
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
			if (mtiMemcmp(ucMtiTestPan, ucDecCardNo, iDecLen) == 0)
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
			if (mtiMemcmp(ucMtiTestPan, ucDecCardNo, iDecLen) == 0)
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

int mtiDukptLoadRootKey	(void)
{
    int iRET;
    T_SEC_DATAKEY_ID stRootKeyID;
    unsigned char ucTheRootKey[DUKPT_TDES_LENGTH+1];

#ifdef DEF_DUKPT_INIT_TRACE
    dmsg("[FUNC] mtiDukptLoadRootKey() => START");
#endif

    stRootKeyID.iSecretArea = ID_SCR_MTI_BANK;
    stRootKeyID.cAlgoType   = TLV_TYPE_KTDES_16; /* This ROOT key is a TDES Key */
    stRootKeyID.usNumber    = ROOT_KEY_NO_MTI;
    stRootKeyID.uiBankId    = ID_BANK_MTI;

    iRET = SEC_FreeSecret(C_SEC_PINCODE, &stRootKeyID);
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiDukptLoadRootKey() => SEC_FreeSecret() result is [%d]", iRET);
#endif

	mtiAtoh(ucTheRootKey, ucMtiTestRootKey, DUKPT_TDES_LENGTH);
	iRET = SEC_LoadKey(C_SEC_PINCODE, NULL, &stRootKeyID, (unsigned char*)&ucTheRootKey, CIPHERING_KEY);
#ifdef DEF_DUKPT_INIT_TRACE
	if (iRET != OK)
	{
		dmsg("[FUNC] mtiDukptLoadRootKey() => SEC_LoadKey() Loading Root Key... Failed... [%d]", iRET);
		dmsg("[FUNC] mtiDukptLoadRootKey() => Use SKMT/SKMT2 tool to load the Root Key");
	}
	else
	{
		dmsg("[FUNC] mtiDukptLoadRootKey() => Loading Root Key... OK");
	}
#endif

    return (iRET);
}

int mtiDukptCreateSecureArea(void)
{
	int iRET;

#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiDukptCreateSecureArea() => START");
#endif

    /* Check Secure Area*/
	iRET = SEC_isSecretArea(C_SEC_PINCODE, (SEG_ID)ID_SCR_MTI_BANK);
	if (iRET == OK)
	{
#ifdef DEF_DUKPT_INIT_TRACE
		dmsg("[FUNC] mtiDukptCreateSecureArea() => Secure Area is already Exist");
#endif

		iRET = OK;
		return iRET;
	}

	/* Create Secure Area */
#ifndef __TETRA__

	iRET = SEC_CreateSecretArea(C_SEC_PINCODE, (SEG_ID)ID_SCR_MTI_BANK, CARD_NUMBER_SIGN, VAR_NUMBER_SIGN);
	#ifdef DEF_DUKPT_INIT_TRACE
	if (iRET == OK)
	{
		dmsg("[FUNC] mtiDukptCreateSecureArea() => Creation... OK");
		iRET = SEC_isSecretArea(C_SEC_PINCODE, (SEG_ID)ID_SCR_MTI_BANK);
		if (iRET != OK)
		{
			dmsg("[FUNC] mtiDukptCreateSecureArea() => Creation confirm ... failed (%0d)", iRET);
		}
	}
	else
	{
		dmsg("[FUNC] mtiDukptCreateSecureArea() => Creation... failed (%0d)", iRET);
	}

	dmsg("[FUNC] mtiDukptCreateSecureArea() => END (%0d)", iRET);
	#endif

#endif


	return (iRET);
}



/* 2017-02-21, INTECH ADD START */
int mtiPanLoadEncryptKey (void)
{
	int iRET;
	T_SEC_DATAKEY_ID stEncryptKeyID;
	unsigned char ucTheEncryptKey[DUKPT_TDES_LENGTH+1];

#ifdef DEF_DUKPT_INIT_TRACE
	int iConfPartsNum, iNumId=0, iTabList[10], iRslt, iIdx;
	T_SEC_CONFIG stConfParts[DUKPT_KSN_LENGTH];

	dmsg("[FUNC] mtiLoadEncryptKey() => START");
#endif


#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiPanLoadEncryptKey() => Before making secure area...");
	iRslt = SEC_listSecureId(C_SEC_PINCODE, &iNumId, iTabList);
	if (iRslt == OK)
	{
		dmsg("[FUNC] mtiPanLoadEncryptKey() => Secret Area Id's: %d", iNumId);
		for (iIdx=0; iIdx<iNumId; iIdx++)
		{
			dmsg("                                Secret Area Id's [%d]: %x", iIdx, iTabList[iIdx]);
		}
	}
	else
		dmsg("[FUNC] mtiPanLoadEncryptKey() => SEC_listSecureId() was Failed... [%d]", iRslt);

	// Check Secure Area
	iRET = SEC_GetConfig(&iConfPartsNum, stConfParts);
	if (iRET == OK)
	{
		dmsg("[FUNC] mtiPanLoadEncryptKey() => Secret Area Configuration parts : %d", iConfPartsNum);
		for (iIdx=0; iIdx<iConfPartsNum; iIdx++)
		{
			dmsg("                         Configuration parts[%d].SecureType : %x", iIdx, stConfParts[iIdx].SecureType);
			dmsg("                         Configuration parts[%d].cBoosterType : %x", iIdx, stConfParts[iIdx].cBoosterType);
			dmsg("                         Configuration parts[%d].ptszBoosterPeripheral : %s", iIdx, stConfParts[iIdx].ptszBoosterPeripheral);
			dmsg("                         Configuration parts[%d].cbGestResid : %x", iIdx, stConfParts[iIdx].cbGestResid);
		}
	}
#endif

	iRET = mtiDukptCreateSecureArea();
	if (iRET == OK)
	{
		// Forced Root key which is used ciphering IPEK inject for test
		iRET = mtiDukptLoadRootKey();
		if (iRET != OK)
		{
			return RTN_DUKPT_SCRET_AREA_FAIL;
		}
	}
	else
	{
		return RTN_DUKPT_SCRET_AREA_FAIL;
	}

	stEncryptKeyID.iSecretArea = ID_SCR_MTI_BANK;
	stEncryptKeyID.cAlgoType   = TLV_TYPE_KTDES_16;
	stEncryptKeyID.usNumber    = ENCKEY_LOC*8;
	stEncryptKeyID.uiBankId    = ID_BANK_MTI;

	// check PAN encrypt key
	iRET = mtiPanCheckEncryptKey();
	if (iRET != RTN_DUKPT_OK)
	{
		/**
		 * it has to add on the logic for special case if has problem from PAN key for encrypt to PAN data.
		 **/

		// Call Callback Function (iRecordConfirmAndRemove)
		if(enc_cb != NULL);
		{
			enc_cb();
		}
		// Get random number
		mtiMemset(ucTheEncryptKey, 0, DUKPT_TDES_LENGTH+1);
		iRET = mtiGetRandom(ucTheEncryptKey, DUKPT_TDES_LENGTH);
		if (iRET != RTN_SUCCESS)
		{
#ifdef DEF_DUKPT_INIT_TRACE
			dmsg("[FUNC] mtiLoadEncryptKey() => mtiGetRandom() failed... [%d]", iRET);
#endif
			return RTN_DUKPT_GET_RANDOM_ERROR;
		}

		// PAN encrypt key load
		iRET = SEC_LoadKey(C_SEC_CIPHERING, NULL, &stEncryptKeyID, (unsigned char*)&ucTheEncryptKey, CIPHERING_DATA);
		if (iRET != RTN_DUKPT_OK)
		{
#ifdef DEF_DUKPT_INIT_TRACE
			dmsg("[FUNC] mtiDukptLoadRootKey() => SEC_LoadKey() Loading Root Key... Failed... [%d]", iRET);
#endif
			return RTN_DUKPT_SCRET_AREA_FAIL;
		}
#ifdef DEF_DUKPT_INIT_TRACE
		else
		{
			dmsg("[FUNC] mtiDukptLoadRootKey() => Loading Root Key... OK");
		}
#endif
	}

#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiLoadEncryptKey() => END(%d)", iRET);
#endif

	return (iRET);
}

/* 2017-02-21, INTECH ADD END */
// ///////////////////////////////////////////////////////////////////////////////////////////////////////


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
	T_SEC_DATAKEY_ID stDukptKeyID;

#ifdef DEF_TRC_TRACE
    dmsg("[mtiDukptIsWork()] START");
#endif

	// check whether PIN encryption key was injected or not
	iRET = TSA_GetKeyID(TSA_KEY_DUKPT, 0, &stDukptKeyID);
	if (iRET == TSA_OK)
	{
#ifdef DEF_DUKPT_INIT_TRACE
		dmsg("[FUNC] mtiDukptInit() => TSA_GetKeyID() is returned [%d]  : PIN key was injected ", iRET);
		dmsg("            stDukptKeyID.iSecretArea = [%08x]", stDukptKeyID.iSecretArea);
		dmsg("            stDukptKeyID.cAlgoType = [%08x]", stDukptKeyID.cAlgoType);
		dmsg("            stDukptKeyID.usNumber = [%08x]", stDukptKeyID.usNumber);
		dmsg("            stDukptKeyID.uiBankId = [%08x]", stDukptKeyID.uiBankId);
#endif
		iRET = RTN_DUKPT_OK;
	}
	else
	{
#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiDukptInit() => TSA_GetKeyID() is returned [%d]  : PIN key was not injected ", iRET);
#endif
		iRET = RTN_DUKPT_NO_KEY;
	}

	return (iRET);
}

void checkKeyID(unsigned char ucKeyTypeIdx)
{
   int iRET;
   T_SEC_DATAKEY_ID stDukptKeyID;

   dmsg("=============================================================");
   iRET = TSA_GetKeyID(ucKeyTypeIdx, 0, &stDukptKeyID);
   if (ucKeyTypeIdx == TSA_KEY_PEFMK)
   {
      dmsg("[FUNC] checkKeyID() => TSA_GetKeyID(TSA_KEY_PEFMK) is returned [%d]  ", iRET);
   }
   else if (ucKeyTypeIdx == TSA_KEY_CEFMK)
   {
      dmsg("[FUNC] checkKeyID() => TSA_GetKeyID(TSA_KEY_CEFMK) is returned [%d]  ", iRET);
   }
   else if (ucKeyTypeIdx == TSA_KEY_CDMK)
   {
      dmsg("[FUNC] checkKeyID() => TSA_GetKeyID(TSA_KEY_CDMK) is returned [%d]  ", iRET);
   }
   else if (ucKeyTypeIdx == TSA_KEY_MASTER)
   {
      dmsg("[FUNC] checkKeyID() => TSA_GetKeyID(TSA_KEY_MASTER) is returned [%d]  ", iRET);
   }
   else if (ucKeyTypeIdx == TSA_KEY_SESSION)
   {
      dmsg("[FUNC] checkKeyID() => TSA_GetKeyID(TSA_KEY_SESSION) is returned [%d]  ", iRET);
   }
   else if (ucKeyTypeIdx == TSA_KEY_DUKPT)
   {
      dmsg("[FUNC] checkKeyID() => TSA_GetKeyID(TSA_KEY_DUKPT) is returned [%d]  ", iRET);
   }
   else
   {
      dmsg("[FUNC] checkKeyID() => TSA_GetKeyID(unknown-%02x) is returned [%d]  ", ucKeyTypeIdx, iRET);
   }

   if (iRET == TSA_OK)
   {
      dmsg("            stDukptKeyID.iSecretArea = [%08x]", stDukptKeyID.iSecretArea);
      dmsg("            stDukptKeyID.cAlgoType = [%08x]", stDukptKeyID.cAlgoType);
      dmsg("            stDukptKeyID.usNumber = [%08x]", stDukptKeyID.usNumber);
      dmsg("            stDukptKeyID.uiBankId = [%08x]", stDukptKeyID.uiBankId);
   }
   else
   {
      dmsg("            key was not injected ");
   }

   dmsg("-----------------------------------------------------------------");

   return;
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
int mtiDukptInit(void)
{
   int iRET = -1;

#if 0 //@@WAY, 20190626 JUST REMARK, NOT USED
	#ifdef DEF_DUKPT_INIT_TRACE
		#ifndef __TETRA__
   debugStart(COMMCHANNEL_DEV_USB);
   dmsg("[FUNC] mtiDukptInit() => START");
		#endif
	#endif
#endif

   /* Copheck Secure Area*/
#ifdef DEF_DUKPT_INIT_TRACE

   checkKeyID(TSA_KEY_PEFMK);
   checkKeyID(TSA_KEY_CEFMK);
   checkKeyID(TSA_KEY_CDMK);
   checkKeyID(TSA_KEY_MASTER);
   checkKeyID(TSA_KEY_SESSION);
   checkKeyID(TSA_KEY_DUKPT);

   iRET = SEC_isSecretArea(C_SEC_PINCODE, (SEG_ID)ID_SCR_MTI_BANK);
   if (iRET != OK)
   {
      dmsg("[FUNC] mtiDukptInit() => SEC_isSecretArea()  return [%d] : Secure Area is not Exist", iRET);
   }
   else
   {
      dmsg("[FUNC] mtiDukptInit() => SEC_isSecretArea()  return [%d] : Secure Area is Exist", iRET);
   }
#endif

   // create Secure area & inject PAN encryption key
   iRET = mtiPanLoadEncryptKey();
   if (iRET == RTN_DUKPT_OK)
   {
      iRET = mtiDukptIsWork();
   }
   else
   {
#ifdef DEF_DUKPT_INIT_TRACE
      dmsg("[FUNC] mtiDukptInit() => PAN key was not injected [%d] ", iRET);
#endif
      iRET = RTN_DUKPT_SCRET_AREA_FAIL;
   }
#if 0 //@@WAY, 20190626 JUST REMARK, NOT USED
	#ifdef DEF_DUKPT_INIT_TRACE
   dmsg("[FUNC] mtiDukptInit() => END(%d)", iRET);
   debugStop();
	#endif
#endif
   return (iRET);
}

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
int mtiDukptPinEntry(tDispContents *tpDispCont, int iDispContCnt, int iSecLength, tAddCtrlContents *tpAddCtrl)
{
	T_SEC_ENTRYCONF stEntryConfig;
	tDispContents *tpKeyEnt = NULL;
	int iRET = 0, iEndEnterPIN = 0, iIdx;
	unsigned int uiEventToWait = 0;
	int iToContinue = 0;
	int iInputOffset=0;
	int iAfterShow = FALSE;
	int iUpdateFlag = FALSE;
	int iPinLength = FALSE;
	char *cpEchoStr = NULL;
	unsigned char ucPinOut;
	char caEcho[DISP_MAX_COULUM + 1];

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptPinEntry() => START");
#endif

	mtiMemset(caEcho, 0, sizeof(caEcho));

    // Check Pinpad
	iRET = DLLExist("SECURITY");
#ifdef DEF_TRC_TRACE
	if (iRET != TRUE)
	{
		dmsg("[FUNC] mtiDukptPinEntry() => No Security Dll loaded");
	}
#endif

	if ((iSecLength < PIN_ENTRY_MIN) || (iSecLength > PIN_ENTRY_MAX))
		return (RTN_DUKPT_PIN_SIZE_ERR);

	for (iIdx = 0; iIdx < iDispContCnt; iIdx++)
	{
		iPinLength = SLEN(20, tpDispCont[iIdx].caContents);
		if (iPinLength > 0)
		{
			if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_INPUT))
			{
				iInputOffset = iIdx;
				dmsg("**** iInputOffset = %d", iInputOffset);
				tpKeyEnt = tpDispCont + iIdx +1;
				if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_AFTER_CHECK))
				{
					iAfterShow = TRUE;
				}
				caEcho[0] = '[';
				mtiMemset(&caEcho[1], ' ', iSecLength);
				caEcho[1+iSecLength] = ']';
				cpEchoStr = &caEcho[1];
				SCPY(tpKeyEnt->caContents, caEcho, (iSecLength+3));
			}
		}
	}
	if (cpEchoStr == NULL)
		return (RTN_DUKPT_PIN_ENTRY_ERR);

	//PIN Entry Initialize
	stEntryConfig.ucEchoChar = ECHO_PIN;
	if (CHK_BIT(tpDispCont[iInputOffset].iAttribute, DISP_OPT_CHECK_NO_INPUT))
		stEntryConfig.ucMinDigits = PIN_ENTRY_MIN;
	else
		stEntryConfig.ucMinDigits = 0;
	stEntryConfig.ucMaxDigits = iSecLength;
	stEntryConfig.iFirstCharTimeOut = (PIN_ENTRY_TIMEOUT);
	stEntryConfig.iInterCharTimeOut = (PIN_ENTRY_INTERVAL);

#ifdef __TETRA__
	if (!mtiIsValildKeyboard())
	{
		mtiInitializeKeyboard();
	}
#endif

	iRET = SEC_PinEntryInit (&stEntryConfig, C_SEC_PINCODE);
	if (iRET == OK)
	{
		mtiShowDialogDisp(tpDispCont, iDispContCnt, NULL);
		if (tpAddCtrl != NULL)
		{
			mtiProcAddCtrl(tpAddCtrl);
		}

		// Initialization
		iToContinue = TRUE;
		iEndEnterPIN = FALSE;
		iPinLength = 0;
		iRET = RTN_DUKPT_CANCEL;
		uiEventToWait = KEYBOARD;

		dmsg("[SEC_PinEntry] ucMinDigits=[%d]", stEntryConfig.ucMinDigits);
		
		while (!iEndEnterPIN)
		{
			iUpdateFlag = FALSE;
			iRET = SEC_PinEntry(&uiEventToWait,&ucPinOut, &iToContinue);
			dmsg("[SEC_PinEntryInit result] iRET=[%d]  uiEventToWait=[%d]  ucPinOut=[%02x]  iToContinue=[%d]", iRET, uiEventToWait, ucPinOut, iToContinue);
			if (iRET == OK)
			{
				switch (ucPinOut)
				{
					case ECHO_PIN:
						*cpEchoStr++ = ECHO_PIN;
						iUpdateFlag = TRUE;
						break;
#ifdef __TETRA__
					case 0x03: //PIN TOO SHORT
						iPinLength = cpEchoStr - &caEcho[1];

						if (stEntryConfig.ucMinDigits != 0 || iPinLength != 0) 
						{
							dmsg("[SEC_PinEntry] PIN TOO SHORT=[%d]", iPinLength);
							break;
						}
						else
						{
							iToContinue = FALSE;
							iRET = SEC_PinEntry(&uiEventToWait,&ucPinOut, &iToContinue);
							dmsg("[SEC_PinEntry] PIN TOO SHORT=[%d] but bypass", iPinLength);
						}
#endif
						
					case T_VAL:
						dpt();
						iPinLength = cpEchoStr - &caEcho[1];
						if (!CHK_BIT(tpDispCont[iInputOffset].iAttribute, DISP_OPT_CHECK_NO_INPUT))
						{
							if ((iPinLength > 0) && (iPinLength<PIN_ENTRY_MIN))	// insufficient PIN 
							{
								iRET = SEC_PinEntryInit (&stEntryConfig, C_SEC_PINCODE);
								if (iRET != OK)
								{
									iEndEnterPIN = TRUE;
									iRET = RTN_DUKPT_ERROR;
									break;
								}
								iToContinue = TRUE;
								mtiMemset(&caEcho[1], ' ', iSecLength);
								cpEchoStr = &caEcho[1];
								iPinLength = 0;
								iUpdateFlag = TRUE;
								break;
							}
							else
							if (iPinLength == 0)
							{
								iEndEnterPIN = TRUE;
								dmsg("**** iPinLength = %d", iPinLength);
								iRET = RTN_DUKPT_PIN_BYPASSED;
								break;
							}
						}
						iEndEnterPIN = TRUE;
						MCPY(tpDispCont[iInputOffset].caInout, caEcho, iPinLength);
						dmsg("**** iPinLength = %d", iPinLength);
						iRET = RTN_SELECT_RIGHT;
						break;

					case T_ANN:
						iEndEnterPIN = TRUE;
						iRET = RTN_DUKPT_CANCEL;
						break;

					case T_CORR:
						if ((cpEchoStr - &caEcho[1]) > 0)
						{
							*--cpEchoStr = ' ';
							iUpdateFlag = TRUE;
						}
						break;

					case 0x00:
						iEndEnterPIN = TRUE;
						iRET = RTN_DUKPT_TIMEOUT;
						break;

					default:
						break;
				}

				if (iUpdateFlag)
				{
					SCPY(tpKeyEnt->caContents, caEcho, (iSecLength+3));
					mtiShowDialogDisp(tpDispCont, iDispContCnt, NULL);
					if (tpAddCtrl != NULL)
					{
						mtiProcAddCtrl(tpAddCtrl);
					}
					dmsg("caEcho = [%s]", caEcho);
					dmsg("tpKeyEnt->caContents = [%s]", tpKeyEnt->caContents);
				}
			}
			else if (iRET == ERR_TIMEOUT)
			{
				iEndEnterPIN = TRUE;
				iRET = RTN_DUKPT_TIMEOUT;
			}
			else
			{
				iEndEnterPIN = TRUE;
				iRET = RTN_DUKPT_ERROR;
			}
		}
	}
	else
	{
		dmsg("SEC_PinEntryInit() Result = %d", iRET);
		iRET = RTN_DUKPT_ERROR;
	}

#ifdef __TETRA__
	mtiFinalizeKeyboard();
#endif
	return (iRET);
}

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
int mtiDukptPinEncrypt(char *cpPan, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET;
	int iPanLen;
	T_SEC_DATAKEY_ID stDukptKeyID;
	unsigned char ucKsn[DUKPT_KSN_LENGTH+1];  // SNKey return at pin = KET SET ID + TRMS ID + TRANSACTION COUNTER
	unsigned char ucHexPan[PIN_BLOCK_LENGTH+1];
	unsigned char ucEncPinBlock[PIN_BLOCK_LENGTH+1];
	unsigned char ucAlignPan[16+1];

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptPinEncrypt() => START");
#endif

	// make pan info for Format0 pinblock
	mtiMemset(ucHexPan, 0, sizeof(ucHexPan));
	mtiMemset(ucAlignPan, 0, sizeof(ucAlignPan));

	//iPanLen = mtiStrlen(16, cpPan);
	iPanLen = mtiStrlen(19, cpPan);
	mtiMemset(ucAlignPan, '0', 4);
	mtiMemcpy(ucAlignPan + 4, (cpPan+(iPanLen-13)), 12);
	mtiAtoh(ucHexPan, ucAlignPan, PIN_BLOCK_LENGTH);
	dbuf("***** PAN Alignment", ucHexPan, PIN_BLOCK_LENGTH);

	// PIN encryption
//	stDukptKeyID.iSecretArea = ID_SCR_MTI_BANK;
//	stDukptKeyID.cAlgoType = TLV_TYPE_TDESDUKPT; /* This key is a DES Key */
//	stDukptKeyID.usNumber = MASTER_SERIAL_KEY_NO_MTI;
//	stDukptKeyID.uiBankId = ID_BANK_MTI;

	iRET = TSA_GetKeyID(TSA_KEY_DUKPT, 0, &stDukptKeyID);
	dmsg("[Debug] TSA_GetKeyID() is returned [%d] ", iRET);
	if (iRET != TSA_OK)
	{
		if (iRET == TSA_APP_NOT_READY)
		{
			dmsg(" ######################################  TSA_APP_NOT_READY");
		}
		else
		{
			dmsg(" ######################################  see  TSA_def.h");
		}
	}
	else
	{
		dmsg("            stDukptKeyID.iSecretArea = [%08x]", stDukptKeyID.iSecretArea);
		dmsg("            stDukptKeyID.cAlgoType = [%08x]", stDukptKeyID.cAlgoType);
		dmsg("            stDukptKeyID.usNumber = [%08x]", stDukptKeyID.usNumber);
		dmsg("            stDukptKeyID.uiBankId = [%08x]", stDukptKeyID.uiBankId);
	}


	mtiMemset(ucKsn, 0, sizeof(ucKsn));
	iRET = SEC_DukptEncryptPin(C_SEC_PINCODE, &stDukptKeyID, DUKPT_ENCRYPT_PIN, ucHexPan, ucEncPinBlock, ucKsn);
	if (iRET == OK)
	{
#ifdef DEF_DATA_TRACE
		dbuf("Ciphered Pin Block", ucEncPinBlock, PIN_BLOCK_LENGTH);
		dbuf("Key Serial Number", ucKsn, DUKPT_KSN_LENGTH);
#endif
		memcpy(ucpOut, ucKsn, DUKPT_KSN_LENGTH);
		memcpy(ucpOut+DUKPT_KSN_LENGTH, ucEncPinBlock, PIN_BLOCK_LENGTH);
		(*ipOutLen) = DUKPT_KSN_LENGTH + PIN_BLOCK_LENGTH;

#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiDukptPinEncrypt() => SEC_DukptEncryptPin() success...");
#endif
	}
#ifdef DEF_TRC_TRACE
  	else
  		dmsg("[FUNC] mtiDukptPinEncrypt() => SEC_DukptEncryptPin() failed...");

	dmsg("[FUNC] mtiDukptPinEncrypt() => END(%0d)", iRET);
#endif

	return iRET;
}

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
#if defined(INITECH_DUKPT_FOR_INGENICO)
int mtiDukptPinEntryEncrypt(tDispContents *tpDispCont, INT iDispContCnt, INT iSecLength, tAddCtrlContents *tpAddCtrl, char *cpPan, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET = RTN_SELECT_RIGHT;
	INT iIdx, iInputPos=-1, iPinLength=0;
	INT iOffset = 0 ,iMaxLen = 0;
	CHAR caPANAlign[16 + 1];
	CHAR caPINBlock[16 + 1];
	UCHAR ucaPINBlock[8];
	UCHAR ucaPAN[8];
	UCHAR ucaOut[8];
	ISEM_MTI_CTX *ptCtx = CTX_GLOBAL;

	dmsg("[FUNC] mtiDukptPinEntryEncrypt() => START");

	for (iIdx = 0; iIdx < iDispContCnt; iIdx++)
	{
			if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_INPUT))
			{
				iInputPos = iIdx;
				break;
			}
	}
	if (iInputPos < 0)
		return RTN_DUKPT_ERROR;

	iRET = mtiShowSecureInput(tpDispCont, iDispContCnt, iSecLength, tpAddCtrl);
	if (iRET == RTN_SELECT_RIGHT)
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was success");
		
		iPinLength = mtiStrlen(12,tpDispCont[iInputPos].caInout);
		if (iPinLength == 0)		// PIN bypass
		{
			*ipOutLen = 0;
			iRET = RTN_SELECT_RIGHT;
		}
		else if (iPinLength < PIN_ENTRY_MIN)	// error
		{
			iRET = RTN_DUKPT_ERROR;
			dmsg("[FUNC] mtiDukptPinEntryEncrypt() => tpDispCont->caInOut Length(%d) failed...", iPinLength);
		}
		else								// Encryption
		{
			mtiMemset(caPINBlock,0,sizeof(caPINBlock));			
			mtiMemset(ucaPINBlock,0,sizeof(ucaPINBlock));
			mtiMemset(ucaPAN,0,sizeof(ucaPAN));
			mtiMemset(ucaOut,0,sizeof(ucaOut));

			mtiMemset(caPANAlign,0,sizeof(caPANAlign));
			mtiMemset(caPANAlign, 'F', 16);
			mtiItoa(iPinLength, '0', 2, (UCHAR *)caPANAlign);
			mtiMemcpy(&caPANAlign[2],tpDispCont[iInputPos].caInout,iPinLength);
			mtiAtoh(ucaPINBlock, (UCHAR*)caPANAlign, 8);
			dbuf("PINBLOCK", ucaPINBlock, 8);

			mtiMemset(caPANAlign,0,sizeof(caPANAlign));
			iMaxLen = mtiStrlen(20, cpPan);
			mtiMemset(caPANAlign, '0', 16);
			mtiMemcpy(caPANAlign + 4, cpPan + (iMaxLen - 1 - 12), 12);
			mtiAtoh(ucaPAN, (UCHAR*)caPANAlign, 8);
			dbuf("PAN Alignment", ucaPAN, 8);		
			for (iOffset = 0; iOffset < 8; iOffset++)
			{
				ucaOut[iOffset] = ucaPINBlock[iOffset] ^ ucaPAN[iOffset];
			}
			dbuf("PIN Block Output", ucaOut, 8);	

			iRET = ISEM_Dukpt_Encrypt_GenKey(ptCtx);
			if (iRET == RTN_DUKPT_OK)
			{
				iRET = ISEM_Encrypt_Data_PIN(ptCtx, ucaOut, 8, ucpOut, ipOutLen);
				if (iRET != RTN_DUKPT_OK)
				{
					dmsg("[FUNC] mtiDukptPinEntryEncrypt() => ISEM_Encrypt_Data_PIN() failed... (%d)", iRET);
					iRET = RTN_DUKPT_ERROR;
				}
				else
				{
					dbuf("ENC Output", ucpOut, *ipOutLen);
					iRET = RTN_SELECT_RIGHT;
				}
			}
			else
			{
				dmsg("[FUNC] mtiDukptPinEntryEncrypt() => ISEM_Dukpt_Encrypt_GenKey() failed... (%d)", iRET);
				iRET = RTN_DUKPT_ERROR;			
			}
		}
		
	}
	else if (iRET == RTN_DUKPT_PIN_BYPASSED)
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was bypassed");
		*ipOutLen = 0;
		iRET = RTN_SELECT_RIGHT;
	}
	else if (iRET == RTN_DUKPT_CANCEL)
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was canceled");
	}
	else if (iRET == RTN_DUKPT_TIMEOUT)
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was timeout");
	}
	else
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was occurred unknown error");
	}

	dmsg("[FUNC] mtiDukptPinEntryEncrypt() => END(%0d)", iRET);

	return (iRET);
}
#else
int mtiDukptPinEntryEncrypt(tDispContents *tpDispCont, INT iDispContCnt, INT iSecLength, tAddCtrlContents *tpAddCtrl, char *cpPan, unsigned char *ucpOut, int *ipOutLen)
{
	int iRET;

	dmsg("[FUNC] mtiDukptPinEntryEncrypt() => START");

	iRET = mtiDukptPinEntry(tpDispCont, iDispContCnt, iSecLength, tpAddCtrl);
	if ((iRET == RTN_DUKPT_OK) || (iRET == RTN_SELECT_RIGHT))
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was success");
		iRET = mtiDukptPinEncrypt(cpPan, ucpOut, ipOutLen);
		if (iRET == RTN_DUKPT_OK)
		{
			dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin encryption was success");
			iRET = RTN_SELECT_RIGHT;
		}
		else
		{
			dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin encryption was failed");
		}

		//return (RTN_DUKPT_PIN_ENCRYPT_ERR);
	}
	else if (iRET == RTN_DUKPT_PIN_BYPASSED)
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was bypassed");
		*ipOutLen = 0;
		iRET = RTN_SELECT_RIGHT;
	}
	else if (iRET == RTN_DUKPT_CANCEL)
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was canceled");
	}
	else if (iRET == RTN_DUKPT_TIMEOUT)
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was timeout");
	}
	else
	{
		dmsg("[FUNC] mtiDukptPinEntryEncrypt() => Pin entry was occurred unknown error");
	}

	dmsg("[FUNC] mtiDukptPinEntryEncrypt() => END(%0d)", iRET);

	return (iRET);
}
#endif

int mtiMskInit(void)
{
	int iRET = 0;

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiMskInit() => mtiMskInit Not Support....SKIP");
#endif

	return (iRET);
}

int mtiEncCallbackReg(tEnc_cb cb)
{
	enc_cb = cb;
	return RTN_ERROR;
}

