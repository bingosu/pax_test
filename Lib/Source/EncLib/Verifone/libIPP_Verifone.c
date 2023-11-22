#include "libMtiDukptApi.h"

#define DEF_TRC_TRACE

typedef struct{
	UCHAR ucaMsgId[2];
	UCHAR ucRespCd;
	UCHAR ucaZeros[5];
	UCHAR ucaKsn[20];
	UCHAR ucaEncPinBlock[16];
} stIppPINresponse, *tpIppPINresponse;


// Internal Functions
VOID mtiIPPSerialClear (INT iHandle);
INT  mtiIPPVerifyPacket (INT iHandle, UCHAR *pucMessage, INT *piLen);

/**********************************************************************************/
/* MULTI HANDLE ROUTINES                                                          */
/**********************************************************************************/
static UCHAR g_ucRecPcktStart;

/**************************************************************************
 IPP_Open
 --------------------------------------------------------------------------
 Return:  IPP Comm POrt
 **************************************************************************/
INT mtiIPPOpenPort (INT iDevice, INT iBaud, INT iFormat, INT *piHandle)
{
	INT iRet;
	open_block_t rs232_Opn_Blk;

	dmsg("[FUNC] mtiIPPOpenPort() => START");

	mtiMemset(&rs232_Opn_Blk, 0, sizeof(rs232_Opn_Blk));

	rs232_Opn_Blk.protocol  = P_char_mode;
	rs232_Opn_Blk.parameter = 0;
	rs232_Opn_Blk.rate      = iBaud;
	rs232_Opn_Blk.format    = iFormat;

	if (iDevice == IPP_COM1_DEVICE)
	{
		*piHandle = open (DEV_COM1, 0);
		if (*piHandle < 0)
		{
			dmsg("[FUNC] mtiIPPOpenPort() => open(DEV_COM1) failed iRet=%d", *piHandle);
			return RTN_ERROR;
		}
	}
	else
	{
		if (iDevice == PINPAD_INTERNAL)
		{
			*piHandle = open (DEV_COM5, 0);
			if (*piHandle < 0) 
			{
				dmsg("[FUNC] mtiIPPOpenPort() => open(DEV_COM5) failed iRet=%d", *piHandle);
				return RTN_ERROR;
			}
		}
		else
		{
			*piHandle = open (DEV_COM2, 0);
			if (*piHandle < 0) return RTN_ERROR;
		}

	}

	iRet = set_opn_blk (*piHandle, &rs232_Opn_Blk);
	if (iRet < 0)
	{
		close (*piHandle);
		return RTN_ERROR;
	}
	
	if (iDevice == PINPAD_INTERNAL) select_pinpad (iDevice);

	dmsg("[FUNC] mtiIPPOpenPort() => END iRet=[%d], piHandle=[%d]", RTN_DUKPT_OK, *piHandle);
	return RTN_DUKPT_OK;
}

/**************************************************************************
 IPP_Receive
 --------------------------------------------------------------------------
 Return: 
 **************************************************************************/
INT mtiIPPReceive (INT iHandle, UCHAR *ucpMsg, INT *piLen, INT iTimeout)
{
	INT iOffset, iReadCnt, iRequestedLen, iExpectLen;
	INT iTimer=-1, iInterTimer=-1, iTimerType, iFindStart;

	iRequestedLen = *piLen;

	*piLen = 0;
	iOffset = 0;
	iFindStart = FALSE;
	iTimer = mtiStartTimer(iTimeout);
	iTimerType = 1;
	iExpectLen = 1;
	for (;;)
	{
		if (iExpectLen <= 0)
		{
			if (iTimerType == 1)
				mtiStopTimer(iTimer);
			else if (iTimerType == 2)
				mtiStopTimer(iInterTimer);
			break;
		}

		iReadCnt = read (iHandle, (char *)&ucpMsg[iOffset], iExpectLen);
		if (iReadCnt < 0)
		{
			dmsg("[FUNC] mtiIPPReceive() => read() error (%0d)", iReadCnt);
			if (iTimerType == 1)
				mtiStopTimer(iTimer);
			else if (iTimerType == 2)
				mtiStopTimer(iInterTimer);
			return RTN_DUKPT_RECEIVE_ERROR;
		}
		else if (iReadCnt > 0)
		{
			if (!iFindStart)
			{
				if ((ucpMsg[0] == COMM_CHAR_SI) || (ucpMsg[0] == COMM_CHAR_STX) ||
					(ucpMsg[0] == COMM_CHAR_ACK) || (ucpMsg[0] == COMM_CHAR_NAK) || (ucpMsg[0] == COMM_CHAR_EOT))
				{
					iFindStart = TRUE;
					iOffset += iReadCnt;
					iExpectLen = iRequestedLen - iOffset;
				}
			}
			else
			{
				iOffset += iReadCnt;
				iExpectLen = iRequestedLen - iOffset;
			}

			if (iTimerType == 1)
				mtiStopTimer(iTimer);
			else if (iTimerType == 2)
				mtiStopTimer(iInterTimer);

			iInterTimer = mtiStartTimer(300);
			iTimerType = 2;
		}
		else
		{
			/* Exit if reception has began (*piLen > 0) and then stopped (iCount == 0) */
			if (iOffset > 0 && iReadCnt == 0) 
			{
				if (mtiIsTimeout(iInterTimer))
				{
					mtiStopTimer(iInterTimer);
					break;
				}
			}
			else
			{
				if (mtiIsTimeout(iTimer))
				{
					mtiStopTimer(iTimer);
					return RTN_TIMEOUT;
				}
			}		
		}
	}

	*piLen = iOffset;

	return RTN_DUKPT_OK;
}


/**************************************************************************
 IPP_Send
 --------------------------------------------------------------------------
 Return: 
 **************************************************************************/
INT mtiIPPSend (INT iHandle, UCHAR ucStartChar, UCHAR *ucpMsg, INT iSize, INT iTimeout)
{
	INT iIdx, iRet, iLen;
	UCHAR cBegPkt = COMM_CHAR_STX, cEndPkt = COMM_CHAR_ETX;
	UCHAR ucaBuffer[IPP_BUFFER_SIZE];
	UCHAR ucaSendBuffer[IPP_BUFFER_SIZE];
	UCHAR vbyCalcCRC = 0;

	if (ucStartChar == COMM_CHAR_STX || ucStartChar == COMM_CHAR_SI) 
		g_ucRecPcktStart = ucStartChar;
	else
		return RTN_DUKPT_SEND_ERROR;

	if (g_ucRecPcktStart == COMM_CHAR_SI) 
	{
		cBegPkt = COMM_CHAR_SI;
		cEndPkt = COMM_CHAR_SO;
	}

	//Make the Packet <Start><Data><End>
	mtiMemset (ucaSendBuffer, 0x00, sizeof (ucaSendBuffer));
	ucaSendBuffer[0] = cBegPkt;
	mtiMemcpy(&ucaSendBuffer[1], ucpMsg, iSize);
	ucaSendBuffer[1 + iSize] = cEndPkt;
	// Calculate LRC
	iLen = iSize + 2; //To include sentinel: is 2 because we are skipping the first one and including the last one.

	
	for (iIdx=1; iIdx<iLen; iIdx++) //Skip Start centinel but include End sentinel
        vbyCalcCRC ^= ucaSendBuffer[iIdx];
	//Add LRC
	ucaSendBuffer[iSize + 2] = vbyCalcCRC;
	iSize += 3; 
  
	mtiIPPSerialClear (iHandle);
	for (iIdx = 0; iIdx < 3; iIdx++)
	{
       	if (write (iHandle, (char *)ucaSendBuffer, iSize) != iSize)
   		{
			iRet = RTN_DUKPT_SEND_ERROR; 
			break;
   		}
//		dbuf("mtiIPPSend", ucaSendBuffer, iSize); 

		mtiMemset(ucaBuffer, 0x00, sizeof (ucaBuffer));

		iLen = 1;
		iRet = mtiIPPReceive (iHandle, ucaBuffer, &iLen, 500);
		if (iRet == RTN_DUKPT_OK)
		{
			if (ucaBuffer[0] == COMM_CHAR_ACK)
			{
				iRet = RTN_DUKPT_OK; 
				break;
			}
			else if (ucaBuffer[0] == COMM_CHAR_NAK)
				continue;
		}
		else
		{
			iRet = RTN_DUKPT_SEND_ERROR; 
			break;
		}
	}

	if (iIdx > 2)
		iRet = RTN_DUKPT_SEND_ERROR; 

//	dmsg("[FUNC] mtiIPPSend() => END(%0d)", iRet);

	return (iRet);   
}

/**************************************************************************
 IPP_SendByte
 --------------------------------------------------------------------------
 Return: 
 **************************************************************************/
INT mtiIPPSendByte (INT iHandle, UCHAR ucByte)
{
	if (write (iHandle, (char *)&ucByte, 1) != 1)
		return RTN_DUKPT_SEND_ERROR;

	return RTN_DUKPT_OK;
}

/**************************************************************************
 IPP_ReceivePacket
 --------------------------------------------------------------------------
 Return: 
 **************************************************************************/
INT mtiIPPReceivePacket (INT iHandle, UCHAR *ucpMsg, INT *piLen, INT iTimeout)
{
	INT iRet, iTries;

	for (iTries = 0; iTries < 3; iTries++)
	{
		iRet = mtiIPPReceive (iHandle, ucpMsg, piLen, (iTimeout*1000));
	
		if (iRet != RTN_DUKPT_OK)
		{
			mtiIPPSendByte(iHandle, COMM_CHAR_EOT);
			break;
		}

		if (*piLen == 1 && ucpMsg[0] == COMM_CHAR_EOT)
			return RTN_DUKPT_CANCEL;

		if (*piLen > 0)
		{
			// Verifies format and LRC/CRC
			iRet = mtiIPPVerifyPacket(iHandle, ucpMsg, piLen);

//			if (iRet == RTN_IPP_OK || iRet == RTN_IPP_OK_NO_ACK) 
			if (iRet == RTN_DUKPT_OK) 
			{
				if (*piLen > 0)
					mtiIPPSendByte(iHandle, COMM_CHAR_ACK);
				break;
			}
			else 
			{
				mtiIPPSendByte(iHandle, COMM_CHAR_NAK);
			}
		}
		else
		{
			iRet = RTN_DUKPT_RECEIVE_ERROR;
			break;
		}
	}

	if (iTries > 2)
	{
		mtiIPPSendByte(iHandle, COMM_CHAR_EOT);
		iRet = RTN_DUKPT_RECEIVE_ERROR;
	}

	return (iRet);
}

/**************************************************************************
 iPIN_Close
 --------------------------------------------------------------------------

 Return: 

 **************************************************************************/
void mtiIPPClose (INT iHandle)
{
	if (iHandle >= 0) 
	{
		close (iHandle);
	}
}

/**************************************************************************
 IPP_SerialClear
 --------------------------------------------------------------------------
 Return: 
 **************************************************************************/
void mtiIPPSerialClear (INT iHandle)
{
	char c;
	BOOL first = TRUE;

	if (iHandle >= 0)
	{
		for (;;) 
		{
			if (read (iHandle, &c, 1) <= 0) break;
			if (first) {
				first = FALSE;
			}
		}
	}
}

/**************************************************************************
 IPP_VerifyPacket
 --------------------------------------------------------------------------
 Takes a packet just received from the serial port checks the LRC and strips
 the protocol caracters.
 Return: 
 **************************************************************************/
INT mtiIPPVerifyPacket (INT iHandle, UCHAR *pucMessage, INT *piLen)
{
	INT inLoop, inGarbage;
	UCHAR vbyMsgCRC[2], vbyCalcCRC[2];
   
	// Ignore undesired protocol characteres
	inGarbage = 0;
	while (pucMessage[inGarbage] != COMM_CHAR_STX && pucMessage[inGarbage] != COMM_CHAR_SI && inGarbage < *piLen) 
		inGarbage++;

	if (inGarbage >= *piLen)
	{
		*piLen = 0;
		return RTN_DUKPT_RECEIVE_ERROR;
	}

	g_ucRecPcktStart = pucMessage[inGarbage];
	
	vbyCalcCRC[0] = 0x00;
	for (inLoop = 1; inLoop < (*piLen - inGarbage); inLoop++) 
	{
		vbyCalcCRC[0] ^= pucMessage[inLoop + inGarbage];
			
		if (g_ucRecPcktStart == COMM_CHAR_STX && pucMessage[inLoop + inGarbage] == COMM_CHAR_ETX) break;
		if (g_ucRecPcktStart == COMM_CHAR_SI  && pucMessage[inLoop + inGarbage] == COMM_CHAR_SO ) break;
	
		/* strips Protocol characters from message */
		pucMessage[inLoop - 1] = pucMessage[inLoop + inGarbage];
	}

	/* makes the clean message NULL terminated */
	pucMessage[inLoop - 1] = 0;

	vbyMsgCRC[0] = pucMessage[++inLoop + inGarbage];

    return ( (vbyMsgCRC[0] == vbyCalcCRC[0]) ? RTN_DUKPT_OK : RTN_DUKPT_ERROR);
}

INT mtiIPPCommunicate (INT iHdl, UCHAR ucPcktStart, UCHAR *ucpMsgIn, INT iInLen, UCHAR *ucpOut, INT *ipExpLen, BOOL bEOT)
{
	INT iRet = 0, iSize;
	UCHAR ucaAux[100];

	if (ucpOut != NULL) ucpOut[0] = 0x00;

//	dmsg("[FUNC] mtiIPPCommunicate => START");
	dbuf("[FUNC] mtiIPPCommunicate => SEND  ucpMsgIn", ucpMsgIn, iInLen);
	iRet = mtiIPPSend (iHdl, ucPcktStart, ucpMsgIn, iInLen, 10);
	if (iRet == RTN_DUKPT_OK)
	{
		if (ucpOut != NULL)
		{
			iRet = mtiIPPReceivePacket (iHdl, ucpOut, ipExpLen, 20);
			if (iRet == RTN_DUKPT_OK)
			{
				dbuf("[FUNC] mtiIPPCommunicate => RECEIVE  ucpMsgIn", ucpOut, *ipExpLen);
			}
			else
			{
				dmsg("[FUNC] mtiIPPCommunicate => mtiIPPReceivePacket() failed  iRet=%d", iRet);
			}
		}
	}
	else
	{
		dmsg("[FUNC] mtiIPPCommunicate => mtiIPPSend() failed  iRet=%d", iRet);
	}
	
	if (iRet == RTN_DUKPT_OK)
	{
		if (bEOT) 
		{
			iSize = 1;
			iRet = mtiIPPReceivePacket (iHdl, ucaAux, &iSize, 4);
			if (iRet == RTN_DUKPT_CANCEL) /* CANCELLED mean EOT */
			{
				iRet = RTN_DUKPT_OK;
			}
			else
			{
				dmsg("[FUNC] mtiIPPCommunicate => mtiIPPReceivePacket() EOT receive failed  iRet=%d", iRet);
				iRet = RTN_DUKPT_ERROR;
			}
		}
	}
	return iRet ;
}


// open_ipp
INT mtiIPPOpen (int iInitFlag)
{
	INT iRet, iOutLen;
	INT iHdl;
	char caOutput[IPP_BUFFER_SIZE];

	dmsg("[FUNC] mtiIPPOpen() => START iInitFlag=[%d] ", iInitFlag);

	if (iInitFlag)
	{
		iRet = mtiIPPOpenPort (PINPAD_INTERNAL, Rt_1200, Fmt_A8N1, &iHdl);
		if (iRet != RTN_DUKPT_OK) 
		{
			dmsg("[FUNC] mtiIPPOpen() => open failed 1200 bps iRet=%d", iRet);
			return RTN_DUKPT_ERROR;
		}
		iRet = mtiIPPSend (iHdl, COMM_CHAR_STX, (byte *)"11", 2, 10);
		if (iRet == RTN_DUKPT_OK)
		{
			dmsg("[FUNC] mtiIPPOpen() => open IPP as 1200 bps");
			iOutLen = 10;		
			iRet = mtiIPPCommunicate(iHdl, COMM_CHAR_SI, "135", 3, (UCHAR*)caOutput, &iOutLen, TRUE);
			if (iRet != RTN_DUKPT_OK) 
			{
				dmsg("[FUNC] mtiIPPOpen() => baudrte change failed Rt_19200 iRet=%d", iRet);
				mtiIPPClose (iHdl);
				return RTN_DUKPT_ERROR;
			}
			else
			{
				mtiIPPClose (iHdl);
				if (mtiMemcmp(caOutput, "1419200", 7) == 0)
					iHdl = RTN_DUKPT_ERROR;
				else
					return RTN_DUKPT_ERROR;
			}
		}
		else
		{
			dmsg("[FUNC] mtiIPPOpen() => mtiIPPSend failed 1200 iRet=%d", iRet);
			mtiIPPClose (iHdl);
			iHdl = RTN_DUKPT_ERROR;
		}
	}

	iRet = mtiIPPOpenPort(PINPAD_INTERNAL, Rt_19200, Fmt_A8N1, &iHdl);
	if (iRet != RTN_DUKPT_OK) 
	{
		dmsg("[FUNC] mtiIPPOpen() => open failed Rt_19200 iRet=%d", iRet);
		return RTN_DUKPT_ERROR;
	}
	iRet = mtiIPPSend(iHdl, COMM_CHAR_STX, (byte *)"11", 2, 10);
	if (iRet != RTN_DUKPT_OK)
	{
		dmsg("[FUNC] mtiIPPOpen() => mtiIPPSend failed 19200 iRet=%d", iRet);
		mtiIPPClose (iHdl);
		return RTN_DUKPT_ERROR;
	}

	dmsg("[FUNC] mtiIPPOpen() => END 19200 bps Handle=%d", iHdl);
	return iHdl;
}

INT mtiDukptInit (void)
{
	INT hCrypto = -1;
	INT iRET, iOutLen;
	char caOutput[IPP_BUFFER_SIZE];

#ifdef DEF_DUKPT_INIT_TRACE
	dmsg("[FUNC] mtiDukptInit() => START");
#endif

	//Disable Card PAN Encryption for Test 22/04/17 - Alan under only VERIFONE
//#ifdef __VERIFONE_SRC__
#if 0
#else
	iRET = mtiMskInit();
	if(iRET != RTN_DUKPT_OK)
		return RTN_DUKPT_SCRET_AREA_FAIL;
#endif

	// Open IPP
	hCrypto = mtiIPPOpen(TRUE);
	if (hCrypto < RTN_DUKPT_OK)
	{
		dmsg("[FUNC] mtiDukptInit() => IPP open error	hCrypto=[%d]", hCrypto);
		hCrypto = -1;
		return (RTN_DUKPT_ERROR);
	}

// Gets IPP info
	mtiMemset(caOutput, 0x00, sizeof(caOutput));
	iOutLen = 21;
	iRET = mtiIPPCommunicate(hCrypto, COMM_CHAR_SI, (UCHAR *)"06", 2, ( UCHAR *)caOutput, &iOutLen, TRUE);
	if (iRET != RTN_DUKPT_OK)  /* 02-19 INITECH */
	{
		dmsg("[FUNC] mtiDukptInit() => mtiIPPCommunicate error	ret=[%d]", iRET);
		mtiIPPClose(hCrypto);
		return (RTN_DUKPT_ERROR);
	}
	dbuf("[FUNC] mtiDukptInit() => Gets IPP info result", caOutput, iOutLen);
	mtiSleep(100);
	
// Set DUKPT Engine DES
	mtiMemset(caOutput, 0x00, sizeof(caOutput));
	iOutLen = 8;
	iRET = mtiIPPCommunicate (hCrypto, COMM_CHAR_SI, (UCHAR *)"17083", 5, ( UCHAR *)caOutput, &iOutLen, TRUE);
	if (iRET != RTN_DUKPT_OK)
	{
		dmsg("[FUNC] mtiDukptInit() => Sets IPP key management mode error	iRET=[%d]", iRET);
		mtiIPPClose(hCrypto);
		return (RTN_DUKPT_ERROR);
	}
	dbuf("[FUNC] mtiDukptInit() => Sets IPP key management mode result", caOutput, iOutLen);
	mtiSleep(100);

// gets IPP mode
	mtiMemset(caOutput, 0x00, sizeof(caOutput));
	iOutLen = 8;
	iRET = mtiIPPCommunicate (hCrypto, COMM_CHAR_SI, (UCHAR *)"18", 2, ( UCHAR *)caOutput, &iOutLen, TRUE);
	if (iRET != RTN_DUKPT_OK)
	{
		dmsg("[FUNC] mtiDukptInit() => Gets IPP key management mode error	iRET=[%d]", iRET);
		mtiIPPClose(hCrypto);
		return (RTN_DUKPT_ERROR);
	}
	dbuf("[FUNC] mtiDukptInit() => Gets IPP key management mode result", caOutput, iOutLen);
	mtiSleep(100);

// Sets Dukpt Engine 2
	mtiMemset(caOutput, 0x00, sizeof(caOutput));
	iOutLen = 6;
	iRET = mtiIPPCommunicate (hCrypto, COMM_CHAR_SI, (UCHAR *)"192", 3, ( UCHAR *)caOutput, &iOutLen, TRUE);
	if (iRET != RTN_DUKPT_OK)
	{
		dmsg("[FUNC] mtiDukptInit() =>Select Dukpt Engine 0 error	iRET=[%d]", iRET);
		mtiIPPClose(hCrypto);
		return (RTN_DUKPT_ERROR);
	}
	dbuf("[FUNC] mtiDukptInit() => Select Dukpt Engine 0 result", caOutput, iOutLen);
	mtiSleep(100);

// Checks Dukpt Engine
	mtiMemset(caOutput, 0x00, sizeof(caOutput));
	iOutLen = 6;
	iRET = mtiIPPCommunicate (hCrypto, COMM_CHAR_SI, (UCHAR *)"25", 2, ( UCHAR *)caOutput, &iOutLen, TRUE);
	if (iRET != RTN_DUKPT_OK)
	{
		dmsg("[FUNC] mtiDukptInit() =>Checks Dukpt Engine error	iRET=[%d]", iRET);
		mtiIPPClose(hCrypto);
		return (RTN_DUKPT_ERROR);
	}
	dbuf("[FUNC] mtiDukptInit() => Checks Dukpt Engine result", caOutput, mtiStrlen(100, caOutput));
	mtiSleep(100);

	iRET = RTN_DUKPT_OK;

end:
	if( hCrypto >= 0 )		mtiIPPClose(hCrypto);

	//iRET = mtiMskInit();

	dmsg("[FUNC] mtiDukptInit() => END(%0d)", iRET);


	return (iRET);
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
INT mtiDukptPinEntryEncrypt(tDispContents *tpDispCont, int iDispContCnt, int iSecLength, tAddCtrlContents *tpAddCtrl, char *cpPan, unsigned char *ucpOut, int *ipOutLen)
{
	INT iCurX, iCurY;
	INT hCrypto = -1;
	tDispContents *tpKeyEnt = NULL;
	tpIppPINresponse tpPinResp;
	INT iRET = 0, iIdx;
	INT iInputOffset=0;
	INT iAfterShow = FALSE;
	INT iPinLength = FALSE;
	char *cpEcho = NULL;
	char *cpEchoSt = NULL;
	char caEcho[16 + 1];
	char caSend[128], caRecv[IPP_BUFFER_SIZE];
	/* INIDEV , leading F's processing START */
	INT iOffset = 0;
	INT iLoop = 0;
	/* INIDEV , leading F's processing END */

	dmsg("=======================================");
	dmsg("[FUNC] mtiIPPPinEntryEncrypt() => START");
	dmsg("=======================================");

	(*ipOutLen) = 0;

	if ((iSecLength < PIN_ENTRY_MIN) || (iSecLength > PIN_ENTRY_MAX))
		return (RTN_DUKPT_PIN_SIZE_ERR);

	if (cpPan == NULL)
	{
		dmsg("[FUNC] mtiIPPPinEntryEncrypt() => PAN string is NULL");
		return (RTN_DUKPT_IPP_ACCOUNT_ERR);
	}

	// Open IPP
	hCrypto = mtiIPPOpen(FALSE);
	if (hCrypto < RTN_DUKPT_OK)
	{
#ifdef DEF_TRC_TRACE
		dmsg("[FUNC] mtiIPPPinEntryEncrypt() => IPP open error	hCrypto=[%d]", hCrypto);
#endif
		return (RTN_DUKPT_PIN_ENTRY_ERR);
	}
	
	for (iIdx = 0; iIdx < iDispContCnt; iIdx++)
	{
		iPinLength = SLEN(20, tpDispCont[iIdx].caContents);
		if (iPinLength > 0)
		{
			if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_INPUT))
			{
				iInputOffset = iIdx;
#ifdef DEF_TRC_TRACE				
				dmsg("**** iInputOffset = %d", iInputOffset);
#endif
				tpKeyEnt = tpDispCont + iIdx +1;
				cpEchoSt = cpEcho = tpKeyEnt->caContents;
				if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_AFTER_CHECK))
				{
					iAfterShow = TRUE;
				}
				caEcho[0] = '[';
				mtiMemset(&caEcho[1], ' ', iSecLength);
				caEcho[1+iSecLength] = ']';
				caEcho[2+iSecLength] = 0x00; /* INIDEV , 2017-03-05 */
				//caEcho[1+iSecLength+1] = ']'; /* 2017-02-23 INITECH Disable */
				cpEcho = &caEcho[1];
				cpEchoSt = cpEcho;
				SCPY(tpKeyEnt->caContents, caEcho, (iSecLength+4));
			}
		}
	}
	if ((cpEcho == NULL) || (cpEchoSt == NULL))
	{
		if( hCrypto >= 0 )		mtiIPPClose(hCrypto);
		return (RTN_DUKPT_PIN_ENTRY_ERR);
	}

	mtiShowDialogDisp(tpDispCont, iDispContCnt, NULL);
	if (tpAddCtrl != NULL)
	{
		mtiProcAddCtrl(tpAddCtrl);
	}

	// Set Cursor position
	iCurX = (DISP_MAX_COULUM/2) - ((iSecLength+2)/2) + 1;
	iCurY = iInputOffset + 2;
//	write_at(caEcho, 1, iCurX, iCurY);
	gotoxy(iCurX+1, iCurY);

// PIN Entry & encrypt
	mtiMemset(caSend, 0x00, sizeof(caSend));
	mtiMemset(caRecv, 0x00, sizeof(caRecv));
	// Check PIN Bypass or NO INPUT
	if (CHK_BIT(tpDispCont[iInputOffset].iAttribute, DISP_OPT_CHECK_NO_INPUT))
		sprintf (caSend, "Z63.%s%cDUKPT ENCRYPTION04%02dN*", cpPan, 0x1C, iSecLength);
	else
		sprintf (caSend, "Z63.%s%cDUKPT ENCRYPTION04%02dY*", cpPan, 0x1C, iSecLength);

	dbuf("[FUNC] mtiIPPPinEntryEncrypt() => IPP send =>", caSend, mtiStrlen(100, caSend));
	iPinLength = 47;
	tpPinResp = (tpIppPINresponse)caRecv;
	iRET = mtiIPPCommunicate (hCrypto, COMM_CHAR_STX, (UCHAR *)caSend, mtiStrlen(256, caSend), ( UCHAR *)caRecv, &iPinLength, FALSE);
	if (iRET == RTN_DUKPT_OK)
	{
		if (mtiMemcmp(tpPinResp->ucaMsgId, (UCHAR*)"73", 2) == 0)
		{
			if (tpPinResp->ucRespCd == '.')
			{
				if ((mtiMemcmp(tpPinResp->ucaZeros, (UCHAR*)"00000", 5) == 0) && (iPinLength == 47))
				{
					mtiMemcpy(tpDispCont[iInputOffset].caInout, "********************", iSecLength);
					mtiAtoh(ucpOut,tpPinResp->ucaKsn, DUKPT_KSN_LENGTH);
					mtiAtoh(ucpOut+DUKPT_KSN_LENGTH,tpPinResp->ucaEncPinBlock, PIN_BLOCK_LENGTH);
					(*ipOutLen) = DUKPT_KSN_LENGTH + PIN_BLOCK_LENGTH;
					iRET = RTN_SELECT_RIGHT;
				}
				else if ((mtiMemcmp(tpPinResp->ucaZeros, (UCHAR*)"00000", 5) == 0) && (iPinLength < 47) && (iPinLength > 42))
				{
					/* INIDEV , 2017-03-03 Leading F's Processing , START */
					mtiMemcpy(tpDispCont[iInputOffset].caInout, "********************", iSecLength);
					mtiMemset(caSend, 0x00, sizeof(caSend));
					iOffset = 47 - iPinLength;   // iPinLength Max size 47
					for (iLoop = 0 ; iLoop < iOffset ; iLoop++)
					{
						caSend[iLoop] = 'F';
					}

					if (iLoop != 0)
					{
						mtiMemcpy(&caSend[iLoop], tpPinResp->ucaKsn, (DUKPT_KSN_LENGTH*2) - iOffset);
						mtiAtoh(ucpOut,caSend, DUKPT_KSN_LENGTH);
						mtiMemset(caSend, 0x00, sizeof(caSend));
						caSend[0] = tpPinResp->ucaKsn[(DUKPT_KSN_LENGTH*2) - iOffset];
						mtiMemcpy(&caSend[1], tpPinResp->ucaEncPinBlock, 15);
						mtiAtoh(ucpOut+DUKPT_KSN_LENGTH,caSend, PIN_BLOCK_LENGTH);
						(*ipOutLen) = DUKPT_KSN_LENGTH + PIN_BLOCK_LENGTH;
						iRET = RTN_SELECT_RIGHT;
						dbuf("[FUNC] mtiDukptPinEntryEncrypt() => PIN Entry & encrypt result", ucpOut, (*ipOutLen));
					}
					else
					{
						dmsg("[FUNC] mtiDukptPinEntryEncrypt() => iPinLength=[%d]", iPinLength);
						iRET = RTN_DUKPT_IPP_UNKNOWN_ERR;
					}
					/* INIDEV , 2017-03-03 Leading F's Processing , END */
				}
				else if ((mtiMemcmp(tpPinResp->ucaZeros, (UCHAR*)"00000", 5) == 0) && (iPinLength == 11))
				{
					if (CHK_BIT(tpDispCont[iInputOffset].iAttribute, DISP_OPT_CHECK_NO_INPUT))
						iRET = RTN_DUKPT_IPP_UNKNOWN_ERR;
					else
						iRET = RTN_SELECT_RIGHT;
					dbuf("[FUNC] mtiDukptPinEntryEncrypt() => PIN Entry & encrypt result", ucpOut, (*ipOutLen));
				}
				else
				{
					dmsg("[FUNC] mtiDukptPinEntryEncrypt() => IPP receive error tpPinResp->ucaZeros=[%5.5s]  iPinLength=[%d]", tpPinResp->ucaZeros, iPinLength);
					iRET = RTN_DUKPT_IPP_UNKNOWN_ERR;
				}
			}
			else if (tpPinResp->ucRespCd == '1')
				iRET = RTN_DUKPT_IPP_NO_KEY_ERR;
			else if (tpPinResp->ucRespCd == '2')
				iRET = RTN_DUKPT_IPP_ACCOUNT_ERR;
			else if (tpPinResp->ucRespCd == '3')
				iRET = RTN_DUKPT_IPP_PIN_TOO_LONG;
			else if (tpPinResp->ucRespCd == '4')
				iRET = RTN_DUKPT_IPP_PIN_TOO_SHORT;
			else if (tpPinResp->ucRespCd == '5')
				iRET = RTN_DUKPT_IPP_USE_MS;
			else if (tpPinResp->ucRespCd == '6')
				iRET = RTN_DUKPT_IPP_MAX_CNT_ERR;
			else
				iRET = RTN_DUKPT_IPP_UNKNOWN_ERR;
		}
		else
			iRET = RTN_DUKPT_IPP_UNKNOWN_ERR;
	}
	else if (iRET == RTN_DUKPT_CANCEL)
		iRET = RTN_DUKPT_CANCEL;
	else if (iRET == RTN_DUKPT_TIMEOUT)
	{
		iRET = mtiIPPSend (hCrypto, COMM_CHAR_STX, (UCHAR *)"72", 2, 10);
		iRET = RTN_DUKPT_TIMEOUT;
	}
	else
		iRET = RTN_DUKPT_IPP_UNKNOWN_ERR;
	dbuf("[FUNC] mtiDukptPinEntryEncrypt() => PIN Entry & encrypt result", caRecv, mtiStrlen(100, caRecv));

	if( hCrypto >= 0 )		mtiIPPClose(hCrypto);

#ifdef DEF_TRC_TRACE
	dmsg("[FUNC] mtiDukptPinEntryEncrypt() => END(%0d)", iRET);
#endif
	return (iRET);
}

