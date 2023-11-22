
#include "libMtiCommonApi.h"
#include <Telium_crypto.h>
#include <SEC_interface.h>

VOID *mtiMalloc(INT iSize)
{
	VOID *pPtr = NULL;
	CHAR *cpPtr = NULL;
	INT i = 0;

	pPtr = umalloc(iSize);
	cpPtr = (CHAR*)pPtr;
	for (i = 0; i < iSize; i++)
	{
		cpPtr[i] = 0;
	}

	return pPtr;
}

VOID mtiFree(VOID *pPtr)
{
	if (pPtr != NULL)
	{
		ufree(pPtr);
	}
}

INT mtiStrcpy(VOID *pDest, VOID *pSrc, INT iMaxLength)
{
	INT iCnt;
	CHAR cTemp;
	CHAR* pchDest = (CHAR*)pDest;
	CHAR* pchSrc  = (CHAR*)pSrc;

	if (pSrc == NULL)
	{
		return 0;
	}

	for(iCnt = 0; iCnt < iMaxLength; iCnt++)
	{
		cTemp = *pchSrc++;

		if(cTemp == '\0')
		{
			break;
		}

		*pchDest++ = cTemp;
	}
	return iCnt;
}

INT mtiStrlen(INT iMaxLength, VOID *szpSrc)
{
	UCHAR *ucpSrc = (UCHAR*)szpSrc;
	INT iCnt = 0;

	if (szpSrc == NULL)
	{
		return 0;
	}

	while( *ucpSrc != '\0' && iCnt < iMaxLength )
	{
		iCnt++;
		ucpSrc++;
	}
	return iCnt;
}

VOID mtiMemset(VOID *pDest, INT cValue, INT iMaxLength)
{
    UCHAR *ucDest = (UCHAR*)pDest;
    INT i = 0;

    if (pDest == NULL)
    {
    	return;
    }

    if (0 < iMaxLength)
    {
        for (i = 0; i < iMaxLength; i++)
            *ucDest++ = cValue;
    }
}

INT mtiMemcpy(VOID *pDest, VOID *pSrc, INT iMaxLength)
{
    UCHAR *ucDest = (UCHAR*)pDest;
    UCHAR *ucSrc = (UCHAR*)pSrc;
    INT i = 0;

    if (pDest == NULL || pSrc == NULL)
	{
		return -1;
	}

    if (0 < iMaxLength)
    {
        for (i = 0; i < iMaxLength; i++)
            *ucDest++ = *ucSrc++;
    }
    return iMaxLength;
}

INT mtiMemcmp(VOID *pDest, VOID *pSrc, INT iMaxLength)
{
    UCHAR *ucDest = (UCHAR*)pDest;
    UCHAR *ucSrc = (UCHAR*)pSrc;
    INT i = 0;

    if (pDest == NULL || pSrc == NULL)
	{
		return -1;
	}

    if (0 < iMaxLength)
    {
        for (i = 0; i < iMaxLength; i++)
        {
            if (ucDest[i] != ucSrc[i])
            {
                if (ucDest[i] < ucSrc[i])
                    return (i + 1);
                else
                    return -(i + 1);
            }
        }
    }
    return 0;
}

VOID mtiBcdToInt( const UCHAR *ucpBcd, INT *ipOut, const UINT iLength )
{
	INT iStep;

	for( iStep = 0, *ipOut = 0;iStep < iLength;iStep++ ) {
		*ipOut *= 100;
		*ipOut += ( ( ( ( ucpBcd[iStep] >> 4 ) & 0x0F ) * 10 ) + ( ucpBcd[iStep] & 0x0F ) );
	}

	return;
}

VOID mtiIntToBcd( const INT *ipOut, UCHAR *ucpBcd, const UINT iLength )
{
	INT iStep, iTemp;

	iTemp = *ipOut;
	if( iTemp < 0 ) return;

	memset( ucpBcd, 0x00, iLength );

	for( iStep = 0;iStep < iLength;iStep++ ) {
		ucpBcd[iLength - iStep - 1] = ( (UCHAR)( iTemp % 10 ) & 0x0F );
		iTemp /= 10;
		ucpBcd[iLength - iStep] |= ( ( (UCHAR)( iTemp % 10 ) << 4 ) & 0xF0 );
		iTemp /= 10;
		if( iTemp == 0 )
			break;
	}

	return;
}

INT mtiAtoh(UCHAR *ucpDest, UCHAR *ucpSrc, UINT iSize)
{
    UINT i = 0, iSCnt = 0, iDCnt = 0;
    UCHAR ucRet = 0, ucOne = 0, ucTwo = 0;

    for(i = 0; i < iSize; i++)
    {
        ucOne = ucpSrc[iSCnt + i * 2];
        ucTwo = ucpSrc[iSCnt + i * 2 + 1];

        ucRet =  ((ucOne > '9') ? (ucOne - 0x37) : (ucOne - 0x30)) << 4;
        ucRet |= (( ucTwo > '9') ? ( ucTwo - 0x37) : ( ucTwo - 0x30)) & 0x0F;

        ucpDest[iDCnt++] = ucRet;
    }

    return iDCnt;
}

INT mtiHtoa(UCHAR *ucpDest, UCHAR *ucpSrc, UINT iSize)
{
    UINT i = 0;
    UCHAR ucOne = 0, ucTwo = 0;
    UCHAR *ucpVal = NULL;

    for(i = 0; i < iSize; i++)
    {
    	ucOne = ucpSrc[i] & 0x0F;
		ucTwo = (ucpSrc[i] & 0xF0) >> 4;

		ucpVal = ucpDest + i * 2;
		ucpVal[1] = (ucOne < 10) ? (ucOne + 0x30) : (ucOne + 0x37);
		ucpVal[0] = (ucTwo < 10) ? (ucTwo + 0x30) : (ucTwo + 0x37);
    }

    return iSize;
}

ULLONG mtiAtoll(UINT iMaxLength, UCHAR *ucpString)
{
	INT	iStep, iStart, iLength, iTot;
	ULLONG ullResult = 0L;

	iTot = mtiStrlen(iMaxLength, ucpString);
	if( iMaxLength < iTot ) iTot = iMaxLength;

	for( iLength = iTot - 1;iLength >= 0;iLength-- )
	{
		if( ucpString[iLength] >= '0' && ucpString[iLength] <= '9' )
		{
			break;
		}
	}

	for( iStart = 0;iStart < iLength;iStart++ )
	{
		if( ucpString[iStart] >= '0' && ucpString[iStart] <= '9' )
		{
			break;
		}
	}

	iLength = iLength - iStart + 1;

	if( iLength < 0 )
	{
		return 0;
	}

	for( iStep = 0, ullResult = 0;iStep < iLength;iStep++ )
	{
		ullResult *= 10;
		ullResult += (INT)( ucpString[iStep + iStart] - '0' );
	}

	return ullResult;
}

INT mtiAtoi(UINT iMaxLength, UCHAR *ucpString)
{
	INT	iStep, iStart, iLength, iResult, iTot;

	iTot = mtiStrlen(iMaxLength, ucpString);
	if( iMaxLength < iTot ) iTot = iMaxLength;

	for( iLength = iTot - 1;iLength >= 0;iLength-- )
	{
		if( ucpString[iLength] >= '0' && ucpString[iLength] <= '9' )
		{
			break;
		}
	}

	for( iStart = 0;iStart < iLength;iStart++ )
	{
		if( ucpString[iStart] >= '0' && ucpString[iStart] <= '9' )
		{
			break;
		}
	}

	iLength = iLength - iStart + 1;

	if( iLength < 0 )
	{
		return 0;
	}

	for( iStep = 0, iResult = 0;iStep < iLength;iStep++ )
	{
		iResult *= 10;
		iResult += (INT)( ucpString[iStep + iStart] - '0' );
	}

	return iResult;
}

UINT mtiAtoUi(UINT iMaxLength, UCHAR *ucpString)
{
	INT iStep, iStart, iLength, iTot;
	UINT iResult;

	iTot = mtiStrlen(iMaxLength, ucpString);
	if( iMaxLength < iTot ) iTot = iMaxLength;

	for( iLength = iTot - 1;iLength >= 0;iLength-- )
	{
		if( ucpString[iLength] >= '0' && ucpString[iLength] <= '9' )
		{
			break;
		}
	}

	for( iStart = 0;iStart < iLength;iStart++ )
	{
		if( ucpString[iStart] >= '0' && ucpString[iStart] <= '9' )
		{
			break;
		}
	}

	iLength = iLength - iStart + 1;

	if( iLength < 0 )
	{
		return 0;
	}

	for( iStep = 0, iResult = 0;iStep < iLength;iStep++ )
	{
		iResult *= 10;
		iResult += (UINT)( ucpString[iStep + iStart] - '0' );
	}

	return iResult;
}

VOID mtiItoa(INT iValue, const CHAR cDefaultChar, const INT iBufLength, UCHAR *ucpOutBuf)
{
	UCHAR ucSign = cDefaultChar;
	INT	siStep;

	if( iValue < 0 )
	{
		iValue = 0 - iValue;
		ucSign = '-';
	}

	for( siStep = iBufLength - 1; siStep >= 0; siStep-- )
	{
		if( iValue == 0 )
		{
			ucpOutBuf[siStep] = ucSign;
			ucSign = cDefaultChar;
		}
		else
		{
			ucpOutBuf[siStep] = (UCHAR)( iValue % 10 ) + '0';
			iValue = iValue / 10;
		}
	}

	return;
}

VOID mtiLtoa(ULONG lValue, const CHAR cDefaultChar, const INT iBufLength, UCHAR *ucpOutBuf)
{
	UCHAR ucSign = ' ';
	INT	siStep;

	if( lValue < 0 )
	{
		lValue = 0 - lValue;
		ucSign = '-';
	}
	else
	{
		ucSign = cDefaultChar;
	}

	for( siStep = iBufLength - 1; siStep >= 0; siStep-- )
	{
		if( lValue == 0 )
		{
			if(cDefaultChar == 0x20)
			{
				ucpOutBuf[siStep] = ucSign;
				ucSign = cDefaultChar;
			}
			else if(siStep == 0)
			{
				ucpOutBuf[siStep] = ucSign;
			}
			else
			{
				ucpOutBuf[siStep] = cDefaultChar;
			}
		}
		else
		{
			ucpOutBuf[siStep] = (UCHAR)( lValue % (LONG)10 ) + '0';
			lValue = lValue / (LONG)10;
		}
	}

	return;
}

VOID mtiLLtoa(LLONG llValue, const CHAR cDefaultChar, const INT iBufLength, UCHAR *ucpOutBuf)
{
	UCHAR ucSign = ' ';
	INT	siStep;

	if( llValue < 0 )
	{
		llValue = 0 - llValue;
		ucSign = '-';
	}
	else
	{
		ucSign = cDefaultChar;
	}

	for( siStep = iBufLength - 1; siStep >= 0; siStep-- )
	{
		if( llValue == 0 )
		{
			if(cDefaultChar == 0x20)
			{
				ucpOutBuf[siStep] = ucSign;
				ucSign = cDefaultChar;
			}
			else if(siStep == 0)
			{
				ucpOutBuf[siStep] = ucSign;
			}
			else
			{
				ucpOutBuf[siStep] = cDefaultChar;
			}
		}
		else
		{
			ucpOutBuf[siStep] = (UCHAR)(llValue % (LLONG)10 ) + '0';
			llValue = llValue / (LLONG)10;
		}
	}

	return;
}

VOID mtiStringCenterAlign(CHAR *cpIn, CHAR *cpOut, INT iMaxLen)
{
	INT iLength = mtiStrlen(65535, cpIn);
	INT iCenter = 0, iLop = 0, iOut = 0, iIn = 0;

	if (iLength < iMaxLen)
	{
		iCenter = (iMaxLen / 2) - (iLength / 2);
		for (iLop = 0; iLop < iMaxLen; iLop++)
		{
			if ((iLop >= iCenter) && (cpIn[iIn] != 0))
			{
				cpOut[iOut++] = cpIn[iIn++];
			}
			else
			{
				cpOut[iOut++] = 0x20;
			}
		}
	}
	else
	{
		mtiMemcpy(cpOut, cpIn, iMaxLen);
	}
}

VOID mtiStringRightAlign(CHAR *cpIn, CHAR *cpOut, INT iMaxLen)
{
	INT iLength = mtiStrlen(65535, cpIn);
	INT iLop = 0, iOut = 0;

	if (iLength < iMaxLen)
	{
		for (iLop = 0; iLop < (iMaxLen - iLength); iLop++)
		{
			cpOut[iOut++] = 0x20;
		}

		mtiMemcpy(&cpOut[iOut], cpIn, iLength);
	}
	else
	{
		mtiMemcpy(cpOut, cpIn, iMaxLen);
	}
}

INT mtiPaddingLeft(CHAR *cpOut, CHAR *cpIn, INT iPaddingSize, CHAR cFillChar)
{
	INT i = 0;
	INT iInSize = mtiStrlen(MAX_ALLOC_SIZE, cpIn);
	CHAR *cpStart = cpOut;

	for (i = 0; i < iPaddingSize - iInSize; i++)
	{
		*cpStart++ = cFillChar;
	}

	for (i = 0; i < iInSize; i++)
	{
		*cpStart++ = cpIn[i];
	}

	return cpStart - cpOut;
}

INT mtiPaddingRight(CHAR *cpOut, CHAR *cpIn, INT iPaddingSize, CHAR cFillChar)
{
	INT i = 0;
	INT iInSize = mtiStrlen(MAX_ALLOC_SIZE, cpIn);
	CHAR *cpStart = cpOut;

	for (i = 0; i < iInSize; i++)
	{
		*cpStart++ = cpIn[i];
	}

	for (i = 0; i < iPaddingSize - iInSize; i++)
	{
		*cpStart++ = cFillChar;
	}

	return cpStart - cpOut;
}

VOID mtiMapInit(tMtiMap *tpMap)
{
	MemInit(tpMap);
}

VOID mtiMapClear(tMtiMap *tpMap)
{
	MemClear(tpMap);
}

VOID mtiMapRemove(tMtiMap *tpMap, INT iID)
{
	MemFree(tpMap, iID);
}

#if 1
UCHAR *mtiMapCreateBytes(tMtiMap *tpMap, INT iID, INT iLength)
{
	UCHAR *ucpTemp = NULL;

	if (iLength > 0)
	{
		ucpTemp = (UCHAR*)MemAlloc(tpMap, iLength, iID, VTYPE_FIXED_SIZE);
	}

	return ucpTemp;
}

VOID mtiMapPutBytes(tMtiMap *tpMap, INT iID, UCHAR *ucpValue, INT iLength)
{
	UCHAR *cpTemp = NULL;

	if (ucpValue != NULL && iLength > 0)
	{
		cpTemp = (UCHAR*)MemAlloc(tpMap, iLength, iID, VTYPE_FIXED_SIZE);
		mtiMemcpy(cpTemp, ucpValue, iLength);
	}
}
#else
VOID mtiMapPutBytes(tMtiMap *tpMap, INT iID, UCHAR *ucpValue, INT iLength)
{
	tMtiMem *pstNodeInfo = NULL;
	UCHAR *cpTemp = NULL;
	INT nLength = 0;

	if (ucpValue != NULL && iLength > 0)
	{
		pstNodeInfo = MemGetNodeInfo(tpMap, iID);

		if (NULL == pstNodeInfo)
		{
			cpTemp = (UCHAR*)MemAlloc(tpMap, iLength, iID, VTYPE_FIXED_SIZE);
			mtiMemcpy(cpTemp, ucpValue, iLength);
		}
		else
		{
			// if new data is equal to old data ..
			if (NULL != pstNodeInfo->pAllocAddr && 
					nLength == pstNodeInfo->iSize && 
					VTYPE_FIXED_SIZE == pstNodeInfo->iType &&
					0 == MCMP(pstNodeInfo->pAllocAddr, ucpValue, iLength))
			{
				return;
			}

			if(NULL!=pstNodeInfo->pAllocAddr)
				mtiFree(pstNodeInfo->pAllocAddr);

			pstNodeInfo->pAllocAddr = MEM_ALLOC(iLength);
			pstNodeInfo->iType = VTYPE_FIXED_SIZE;
			pstNodeInfo->iSize = iLength;
			mtiMemcpy(pstNodeInfo->pAllocAddr, ucpValue, iLength);
		}
	}
}
#endif

UCHAR *mtiMapGetBytes(tMtiMap *tpMap, INT iID, INT *iLength)
{
	INT iType = 0;
	INT iDataLen = 0;
	UCHAR *ucpTemp = MemReference(tpMap, iID, &iDataLen, &iType);

	if (iType == VTYPE_FIXED_SIZE)
	{
		if (ucpTemp != NULL)
		{
			if (iDataLen > 0)
			{
				*iLength = iDataLen;
				return ucpTemp;
			}
		}
	}
	
	return NULL;
}

#if 1
CHAR *mtiMapCreateString(tMtiMap *tpMap, INT iID, INT iLength)
{
	CHAR *cpTemp = NULL;

	if (iLength > 0)
	{
		cpTemp = (CHAR*)MemAlloc(tpMap, iLength + 1, iID, VTYPE_STRING);
	}
	return cpTemp;
}

VOID mtiMapPutString(tMtiMap *tpMap, INT iID, CHAR *cpValue)
{
	CHAR *cpTemp = NULL;
	INT iLength = mtiStrlen(MAX_ALLOC_SIZE, cpValue);

	if (cpValue != NULL)
	{
		cpTemp = (CHAR*)MemAlloc(tpMap, iLength + 1, iID, VTYPE_STRING);
		mtiMemset(cpTemp, 0, iLength + 1);
		mtiMemcpy(cpTemp, cpValue, iLength);
	}
}

VOID mtiMapPutStringWithLength(tMtiMap *tpMap, INT iID, CHAR *cpValue, INT iLength)
{
	CHAR *cpTemp = NULL;

	if (cpValue != NULL)
	{
		cpTemp = (CHAR*)MemAlloc(tpMap, iLength + 1, iID, VTYPE_STRING);
		mtiMemset(cpTemp, 0, iLength + 1);
		mtiMemcpy(cpTemp, cpValue, iLength);
	}
}

#else

VOID mtiMapPutString(tMtiMap *tpMap, INT iID, CHAR *cpValue)
{
	tMtiMem *pstNodeInfo = NULL;
	CHAR *cpTemp = NULL;
	INT iLength = mtiStrlen(MAX_ALLOC_SIZE, cpValue);

	if (cpValue != NULL)
	{
		pstNodeInfo = MemGetNodeInfo(tpMap, iID);

		if (NULL == pstNodeInfo)
		{
			cpTemp = (CHAR*)MemAlloc(tpMap, iLength + 1, iID, VTYPE_STRING);
			mtiMemset(cpTemp, 0, iLength + 1);
			mtiMemcpy(cpTemp, cpValue, iLength);
		}
		else
		{
			dpt_n_dmsg("mtiMapPutString [ALREADY EXIST] : [%d] [%s]", iID, (CHAR *)pstNodeInfo->pAllocAddr);

			// if new data is equal to old data ..
			if(NULL != pstNodeInfo->pAllocAddr &&
				(iLength + 1) == pstNodeInfo->iSize &&
				VTYPE_STRING == pstNodeInfo->iType &&
				0 == MCMP(pstNodeInfo->pAllocAddr, cpValue, iLength + 1))
			{
				

				return;
			}

			if (NULL != pstNodeInfo->pAllocAddr)
				mtiFree(pstNodeInfo->pAllocAddr);

			pstNodeInfo->pAllocAddr = MEM_ALLOC(iLength + 1);
			pstNodeInfo->iType = VTYPE_STRING;
			pstNodeInfo->iSize = iLength + 1;

			cpTemp = (CHAR *)pstNodeInfo->pAllocAddr;

			mtiMemset(cpTemp, 0, iLength + 1);
			mtiMemcpy(cpTemp, cpValue, iLength);
		}
	}
}
#endif

CHAR *mtiMapGetString(tMtiMap *tpMap, INT iID)
{
	INT iValueSize = 0;
	INT iType = 0;
	CHAR *cpTemp = MemReference(tpMap, iID, &iValueSize, &iType);

	if (iType == VTYPE_STRING)
	{
		if (cpTemp != NULL)
		{
			if (iValueSize > 0)
			{
				return cpTemp;
			}
		}
	}

	return NULL;
}

VOID mtiMapPutInt(tMtiMap *tpMap, INT iID, INT iValue)
{
	INT *iTemp = NULL;

	iTemp = (INT*)MemAlloc(tpMap, sizeof(INT), iID, VTYPE_INT);
	if (iTemp != NULL)
	{
		*iTemp = iValue;
	}
}

INT mtiMapGetInt(tMtiMap *tpMap, INT iID)
{
	INT iValueSize = 0;
	INT iType = 0;
	INT *iTemp = MemReference(tpMap, iID, &iValueSize, &iType);

	if (iType == VTYPE_INT)
	{
		return *iTemp;
	}
	return 0;
}

VOID mtiMapPutLong(tMtiMap *tpMap, INT iID, LONG lValue)
{
	LONG *lTemp = NULL;

	lTemp = (LONG*)MemAlloc(tpMap, sizeof(LONG), iID, VTYPE_LONG);
	if (lTemp != NULL)
	{
		*lTemp = lValue;
	}
}

LONG mtiMapGetLong(tMtiMap *tpMap, INT iID)
{
	INT iValueSize = 0;
	INT iType = 0;
	INT *iTemp = MemReference(tpMap, iID, &iValueSize, &iType);

	if (iType == VTYPE_LONG)
	{
		return *iTemp;
	}
	return 0;
}

VOID mtiMapPutLLong(tMtiMap *tpMap, INT iID, LLONG llValue)
{
	LLONG *llTemp = NULL;

	llTemp = (LLONG*)MemAlloc(tpMap, sizeof(LLONG), iID, VTYPE_LONGLONG);
	if (llTemp != NULL)
	{
		*llTemp = llValue;
	}
}

LLONG mtiMapGetLLong(tMtiMap *tpMap, INT iID)
{
	INT iValueSize = 0;
	INT iType = 0;
	LLONG *llTemp = MemReference(tpMap, iID, &iValueSize, &iType);

	if (iType == VTYPE_LONGLONG)
	{
		return *llTemp;
	}
	
	return 0;
}

LLONG mtiMapGetLLongAndExist(tMtiMap *tpMap, INT iID, BOOL *pbIsExist)
{
	INT iValueSize = 0;
	INT iType = 0;
	LLONG *llTemp = MemReference(tpMap, iID, &iValueSize, &iType);

	if (iType == VTYPE_LONGLONG)
	{
		if(NULL!= pbIsExist)
			*pbIsExist = TRUE;

		return *llTemp;
	}

	if (NULL != pbIsExist)
		*pbIsExist = FALSE;

	return 0;
}

// batch data list, record number, transaction data
VOID mtiMapPutMap(tMtiMap *tpMap, INT iID, tMtiMap *pDestMap)
{
	tMtiMap *tpMapTemp = NULL;

	tpMapTemp = (tMtiMap*)MemAlloc(tpMap, sizeof(tMtiMap), iID, VTYPE_MAP);
	if (tpMapTemp != NULL)
	{
		mtiMapClone(tpMapTemp, pDestMap);
	}
}

tMtiMap *mtiMapCreateMap(tMtiMap *tpMap, INT iID)
{
	tMtiMap *tpMapTemp = NULL;

	tpMapTemp = (tMtiMap*)MemAlloc(tpMap, sizeof(tMtiMap), iID, VTYPE_MAP);
	if (tpMapTemp != NULL)
	{
		mtiMapInit(tpMapTemp);
	}

	return tpMapTemp;
}

tMtiMap *mtiMapGetMap(tMtiMap *tpMap, INT iID)
{
	INT iValueSize = 0;
	INT iType = 0;
	tMtiMap *tpMapTemp = MemReference(tpMap, iID, &iValueSize, &iType);

	if (iType == VTYPE_MAP)
	{
		return tpMapTemp;
	}
	return NULL;
}

INT mtiMapClone(tMtiMap *pDestMap, tMtiMap *  pSrcMap)
{
	INT i = 0, iID = 0, iSize = 0;
	CHAR *cpMem = NULL;
	UCHAR *ucpMem = NULL;
	INT iMem = 0;
	LONG lMem = 0;
	LLONG llMem = 0LL;
	tMtiMap *tpMap = NULL;

	for (i = 0; i < pSrcMap->iMapCount; i++)
	{
		iID = MemCheck(pSrcMap, i);

		switch (MemType(pSrcMap, iID))
		{
			case VTYPE_INT:
				iMem = mtiMapGetInt(pSrcMap, iID);
				mtiMapPutInt(pDestMap, iID, iMem);
				break;

			case VTYPE_LONG:
				lMem = mtiMapGetLong(pSrcMap, iID);
				mtiMapPutLong(pDestMap, iID, lMem);
				break;

			case VTYPE_FIXED_SIZE:
				ucpMem = mtiMapGetBytes(pSrcMap, iID, &iSize);
				mtiMapPutBytes(pDestMap, iID, ucpMem, iSize);
				break;

			case VTYPE_STRING:
				cpMem = mtiMapGetString(pSrcMap, iID);
				mtiMapPutString(pDestMap, iID, cpMem);
				break;

			case VTYPE_MAP:
				tpMap = mtiMapGetMap(pSrcMap, iID);
				mtiMapPutMap(pDestMap, iID, tpMap);
				break;
			case VTYPE_LONGLONG:
				llMem = mtiMapGetLLong(pSrcMap, iID);
				mtiMapPutLLong(pDestMap, iID, llMem);
				break;
		}
	}

	return i;
}

VOID mtiIntArraySortQuick(INT *piaArray, INT iStart, INT iEnd)
{
	INT iMid = 0, iPivot = 0, iSwap = 0, iP = 0, iQ = 0;

#define SWAP(X, Y)						\
	iSwap = X;							\
	X = Y;								\
	Y = iSwap;

	if (iStart >= iEnd)
	{
		return;
	}

	iMid = (iStart + iEnd) / 2;
	iPivot = piaArray[iMid];

	SWAP(piaArray[iStart], piaArray[iMid]);
	iP = iStart + 1;
	iQ = iEnd;

	while (1)
	{
		while (piaArray[iP] <= iPivot)
		{
			iP++;
		}

		while (piaArray[iQ] > iPivot)
		{
			iQ--;
		}

		if (iP > iQ)
		{
			break;
		}

		SWAP(piaArray[iP], piaArray[iQ]);
	}

	SWAP(piaArray[iStart], piaArray[iQ]);

	mtiIntArraySortQuick(piaArray, iStart, iQ - 1);
	mtiIntArraySortQuick(piaArray, iQ + 1, iEnd);
}

VOID mtiMapSort(tMtiMap *tpMap)
{
	tMtiMap tSortMap;
	tMtiMap *pSrcMap = &tSortMap;
	INT *piID = NULL;
	INT i = 0, iCount = 0, iID = 0, iSize = 0;
	UCHAR *ucpMem = NULL;
	INT iMem = 0;
	LONG lMem = 0;
	LLONG llMem = 0LL;
	CHAR *cpMem = NULL;

	mtiMapInit(pSrcMap);
	mtiMapClone(pSrcMap, tpMap);

	if (tpMap != NULL && tpMap->iMapCount > 0)
	{
		iCount = tpMap->iMapCount;
		piID = (INT*)mtiMalloc(sizeof(INT) * iCount);

		for (i = 0; i < iCount; i++)
		{
			piID[i] = MemCheck(tpMap, i);
		}

		mtiIntArraySortQuick(piID, 0, iCount - 1);

		MemClear(tpMap);
		for (i = 0; i < iCount; i++)
		{
			iID = piID[i];

			switch (MemType(pSrcMap, iID))
			{
				case VTYPE_INT:
					iMem = mtiMapGetInt(pSrcMap, iID);
					mtiMapPutInt(tpMap, iID, iMem);
					break;

				case VTYPE_LONG:
					lMem = mtiMapGetLong(pSrcMap, iID);
					mtiMapPutLong(tpMap, iID, lMem);
					break;

				case VTYPE_FIXED_SIZE:
					ucpMem = mtiMapGetBytes(pSrcMap, iID, &iSize);
					mtiMapPutBytes(tpMap, iID, ucpMem, iSize);
					break;

				case VTYPE_STRING:
					cpMem = mtiMapGetString(pSrcMap, iID);
					mtiMapPutString(tpMap, iID, cpMem);
					break;
				case VTYPE_LONGLONG:
					llMem = mtiMapGetLLong(pSrcMap, iID);
					mtiMapPutLLong(tpMap, iID, llMem);
					break;
			}
		}

		mtiFree(piID);
		MemClear(pSrcMap);
	}
}

CHAR* mtiMD5Gen(UCHAR *ucpSrc, INT iSrcLen)
{
	static CHAR caMD5Hash[50] = {0,};
	INT iIdx = 0;
	UCHAR digest[DIGEST_SIZE] = {0,};
	MD5_CTX ctx;

	dmsg("iSrcLen[%d]", iSrcLen);

	memset(caMD5Hash, 0x00, sizeof(caMD5Hash));

	MD5Init(&ctx);
	MD5Update(&ctx, ucpSrc, iSrcLen);
	MD5Final(digest, &ctx);

	for(iIdx = 0; iIdx < DIGEST_SIZE; iIdx++)
	{
		sprintf(caMD5Hash + (iIdx *2), "%02x", digest[iIdx]);
	}

	return caMD5Hash;
}

CHAR* mtiMD5GenFile(CHAR *cpFilePath)
{
	S_FS_FILE *fp = NULL;
	INT iFileReadCnt, iFileReadRemind, iLoop, iReadCnt;
	CHAR readBuf[1024] = {0,};

	static CHAR caMD5HashFile[50] = {0,};
	INT iFileSize = 0;
	INT iIdx = 0;
	LONG lOrgFileSize = 0L;
	UCHAR digest[DIGEST_SIZE] = {0,};
	MD5_CTX ctx;

	if(cpFilePath == NULL)
	{
		dmsg("cpFilePath is NULL!");
		return NULL;
	}

	fp = FS_open(cpFilePath, "r");
	if(fp == NULL)
	{
		dmsg("File [%s] Open Error", cpFilePath);
		return NULL;
	}

	lOrgFileSize = FS_length(fp);

	if(lOrgFileSize < 1)
	{
		dmsg("File [%s] Size [%ld] Error", cpFilePath, lOrgFileSize);
		FS_close(fp);
		return NULL;
	}

	mtiMemset(caMD5HashFile, 0x00, sizeof(caMD5HashFile));

	MD5Init(&ctx);

	iFileReadCnt = lOrgFileSize / sizeof(readBuf);
	iFileReadRemind = lOrgFileSize % sizeof(readBuf);

	dmsg("FILE [%s], iFileReadCnt [%d], iFileReadRemind[%d]", cpFilePath, iFileReadCnt, iFileReadRemind);

	for(iLoop = 0; iLoop < iFileReadCnt ; iLoop++)
	{
		memset(readBuf, 0x00, sizeof(readBuf));
		iReadCnt = FS_read(readBuf, 1, sizeof(readBuf), fp);

		if(iReadCnt > 0)
		{
			iFileSize += iReadCnt;
			MD5Update(&ctx, (UCHAR *)readBuf, iReadCnt);
		}
	}

	memset(readBuf, 0x00, sizeof(readBuf));
	iReadCnt = FS_read(readBuf, 1, iFileReadRemind, fp);
	if(iReadCnt > 0)
	{
		iFileSize += iReadCnt;
		MD5Update(&ctx, (UCHAR *)readBuf, iReadCnt);
	}

	MD5Final(digest, &ctx);

	for(iIdx = 0; iIdx < DIGEST_SIZE; iIdx++)
	{
		sprintf(caMD5HashFile + (iIdx *2), "%02x", digest[iIdx]);
	}

	FS_close(fp);

	dmsg("FILENAME [%s], FILESIZE[%d]", cpFilePath, iFileSize);
	dmsg("MD5[%s]", caMD5HashFile);
	return caMD5HashFile;
}

INT mtiGetRandom(UCHAR *ucpRamdomData, INT iLen)
{
	INT iRet = 0;

	iRet = SEC_RandomData(iLen, ucpRamdomData);
	dmsg("SEC_RandomData() Result = %d [OK] = %d", iRet, OK);
	if(iRet != OK)
		return RTN_ERROR;

	return RTN_SUCCESS;
}

INT mtiSHA256(UCHAR *ucpSrcData, INT iSrcLen, UCHAR *ucpRamdomData)
{
	if(sha256(ucpSrcData, iSrcLen, ucpRamdomData) != SHA_SUCCESS)
		return RTN_ERROR;

	return RTN_SUCCESS;
}

void deviceFunctionTesting()
{
	// TIME ZONE TEST
	/*
	CHAR szCurrentTime[16] = { '\0', };

	mtiGetDateTime((UCHAR *)szCurrentTime, 14);

	dmsg("CURR TIME : %s", szCurrentTime);

	mtiMemcpy(szCurrentTime, "20171231234040", 14);
	dmsg("FOR YEAR TEST : %s", szCurrentTime);

	mtiSetDateTimeForInni((UCHAR *)szCurrentTime, 14, 2);

	mtiGetDateTime((UCHAR *)szCurrentTime, 14);
	dmsg("CHANGE TIME [1]: %s", szCurrentTime);
	*/

	// ICC SLOT TEST
	/*
	INT nIndex;
	INT i;
	LONG lEvent;

	for (i = 0; i < 10; i++)
	{
		nIndex = mtiInitializeICController(SLOT_MAIN_ICC);

		dmsg("SLOT OPEN : %d", nIndex);

		lEvent = wait_evt(EVT_ICC1_INS);

		dmsg("EVNET [%08X]", lEvent);

		mtiFinalizeICController(nIndex);
	}
	*/

	// PORT OPEN / CLOSE TEST

	tCommInfo *pstCommonInfo = NULL;
	INT nResult;
	UCHAR byBuffer[128];
	INT nReadLength = 0;
	INT nPorts[] = { COMMCHANNEL_COM1, COMMCHANNEL_DEV_USB };
	INT i;

	for (i = 0; i < sizeof(nPorts) / sizeof(INT); i++)
	{

		dmsg("[INDEX : %d] PORT OPEN INIT !!", i);

		pstCommonInfo = mtiInitializeComm();

		pstCommonInfo->iCommType = COMMTYPE_SERIAL;
		pstCommonInfo->iCommChannel = nPorts[i];
		pstCommonInfo->iCommBaudrate = COMMBAUD_115200;

		dmsg("[INDEX : %d] PORT OPEN OPEN !!", i);
		nResult = mtiOpenComm(pstCommonInfo, TIMEOUT_5S);

		if (nResult != RTN_SUCCESS)
		{
			mtiFinalizeComm(pstCommonInfo);
			pstCommonInfo = NULL;

			dmsg("[INDEX : %d] PORT OPEN FAIL !!", i);

			return;
		}

		dmsg("[INDEX : %d] PORT WRITE  !!", i);
		nResult = mtiWriteComm(pstCommonInfo, (UCHAR *)"1234567890", 10);
		dmsg("[INDEX : %d] PORT WRITE RESULT [%d] !!", i, nResult);

		dmsg("[INDEX : %d] PORT RECV WAIT  !!", i);
		nResult = mtiReadComm_With_Timeout(pstCommonInfo, byBuffer, sizeof(byBuffer), &nReadLength, 0);

		dmsg("[INDEX : %d] RECV RESULT : [%d], [%d]", i, nResult, nReadLength);
		if (0 > nReadLength)
		{
			dbuf("RECV DATA", byBuffer, nReadLength);
		}


		dmsg("[INDEX : %d] PORT RECV CLOSE  !!", i);
		mtiCloseComm(pstCommonInfo);
	}
}

