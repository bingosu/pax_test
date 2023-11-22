
#include "libMtiCommonApi.h"

#define MOUNT_MTI_LABEL			"Mti"
#define MOUNT_MTI				"/Mti"

#define SPEED_INOVATE

#define INIT_VAR()											\
	dpt();													\
	if (fpTemp != NULL) FS_close(fpTemp); fpTemp = NULL;	\
	if (fpNew != NULL) FS_close(fpNew);	fpNew = NULL;		\
	mtiMapClear(&tMapTemp);

static INT g_isDiskCreate = FALSE;

VOID mtiInitializeFileSystem()
{
	doubleword uiMode = 0;
	S_FS_PARAM_CREATE xCfg;
	ULONG ulSize = 0UL;
	INT iRet = 0;

	if (!g_isDiskCreate)
	{
		sprintf(xCfg.Label, "%s", MOUNT_MTI_LABEL);
		xCfg.Mode = FS_WRITEONCE;                                                // Disk on Flash
		xCfg.AccessMode	= FS_WRTMOD;                                             // R/W access
		xCfg.NbFichierMax = 16;                                                  // Max files number
		xCfg.IdentZone	= FS_WO_ZONE_DATA;                                       // Zone ID
		ulSize= xCfg.NbFichierMax * 32768;                                       // Disk size in bytes

		iRet = FS_dskcreate(&xCfg, &ulSize);
		g_isDiskCreate = 1;
	}

	iRet = FS_mount (MOUNT_MTI, &uiMode);
	dmsg("FS_mount() Result = %d", iRet);
	if (iRet != FS_OK)
	{
		buzzer(30);
		buzzer(100);
	}
}

INT mtiFileDelete(CHAR *cpFilename)
{
	CHAR caPath[MAX_FILE_PATH];
	INT iRet = 0;

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);
	iRet = FS_unlink(caPath);
	dmsg("FS_unlink Result = [%s] %d", caPath, iRet);
	return RTN_SUCCESS;
}

static S_FS_FILE *mtiFileOpen(CHAR *cpFile, CHAR *cpMode)
{
    S_FS_FILE *fpTemp;

    fpTemp = FS_open(cpFile, (char*) cpMode);
    if(fpTemp == NULL && cpMode[0] == 'w')
	{
    	fpTemp = FS_open(cpFile, "a");
    	if (fpTemp != NULL)
    	{
    		FS_close(fpTemp);
    		fpTemp = FS_open(cpFile, (char*) cpMode);
    	}
    }

    //dmsg("mtiFileOpen(%s, %s) Result = [%s] Mode [%s]", cpFile, cpMode, (fpTemp != NULL ? "SUCCESS" : "FAILED"), cpMode);
	return fpTemp;
}

INT mtiGetFileSize(CHAR *cpFilename)
{
	S_FS_FILE *fpTemp = NULL;
	INT iSize = -1;
	CHAR caPath[MAX_FILE_PATH];

	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);
	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		iSize = FS_length(fpTemp);
		FS_close(fpTemp);
	}

	return iSize;
}

INT mtiGetFileContents(CHAR *cpFilename)
{
	S_FS_FILE *fpTemp = NULL;
	INT iSize = -1;
	CHAR caPath[MAX_FILE_PATH];
	UCHAR *ucpContents = NULL;

	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);
	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		iSize = FS_length(fpTemp);
		ucpContents = (UCHAR*)mtiMalloc(iSize * sizeof(UCHAR*));

		if (FS_read(ucpContents, 1, iSize, fpTemp) == iSize)
		{
			dmsg("FILENAME [%s]", caPath);
			dbuf("FILE CONTENTS", ucpContents, iSize);
		}

		mtiFree(ucpContents);
		FS_close(fpTemp);
	}

	return iSize;
}

INT mtiLoadFile(CHAR *cpFilename, UCHAR *ucpOut, INT *iOutSize)
{
	S_FS_FILE *fpTemp = NULL;
	INT iSize = -1;
	INT iRet = RTN_ERROR;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		iSize = FS_length(fpTemp);
		*iOutSize = iSize;
		if (iSize > 0)
		{
			if (FS_read(ucpOut, 1, iSize, fpTemp) == iSize)
			{
				iRet = RTN_SUCCESS;
			}
		}
		FS_close(fpTemp);
	}

	return iRet;
}

INT mtiSaveFile(CHAR *cpFilename, UCHAR *ucpIn, INT *iInSize)
{
	return RTN_ERROR;
}

INT mtiMapGetElementsToFile(tMtiMap *tpMap, S_FS_FILE *fpSrc)
{
	INT iExit = 0, i = 0, iKey = 0, iType = 0, iLen = 0, iValue = 0;
	LONG lValue = 0L;
	LLONG llValue = 0LL;
	CHAR *cpValue = NULL;
	UCHAR *ucpValue = NULL;

	for (i = 0; i < tpMap->iMapCount; i++)
	{
		if (iExit)
		{
			break;
		}

		iKey = MemCheck(tpMap, i);
		if (FS_write(&iKey, 1, sizeof(INT), fpSrc) < 0)
		{
			dpt();
			iExit = TRUE;
			break;
		}

		iType = MemType(tpMap, iKey);
		if (FS_write(&iType, 1, sizeof(INT), fpSrc) < 0)
		{
			dpt();
			iExit = TRUE;
			break;
		}

		switch (iType)
		{
			case VTYPE_FIXED_SIZE:
				ucpValue = mtiMapGetBytes(tpMap, iKey, &iLen);
				if (FS_write(&iLen, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

				if (FS_write(ucpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_STRING:
				cpValue = mtiMapGetString(tpMap, iKey);
				iLen = mtiStrlen(MAX_ALLOC_SIZE, cpValue);
				if (FS_write(&iLen, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

				if (FS_write(cpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_INT:
				iValue = mtiMapGetInt(tpMap, iKey);
				if (FS_write(&iValue, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_LONG:
				lValue = mtiMapGetLong(tpMap, iKey);
				if (FS_write(&lValue, 1, sizeof(LONG), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;
			case VTYPE_LONGLONG:
				llValue = mtiMapGetLLong(tpMap, iKey);
				if (FS_write(&llValue, 1, sizeof(LLONG), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;
		}
	}

	if (iExit == TRUE)
	{
		return RTN_ERROR;
	}
	else
	{
		return RTN_SUCCESS;
	}
}

INT mtiFileToCalcMapSizeMove(S_FS_FILE *fpSrc, INT iCount)
{
	INT iExit = 0, i = 0, iKey = 0, iType = 0, iLen = 0, iValue = 0;
	LONG lValue = 0L;
	LLONG llValue = 0LL;
	CHAR *cpValue = NULL;

	iExit = FALSE;
	for (i = 0; i < iCount; i++)
	{
		if (iExit)
		{
			break;
		}

		if (FS_read(&iKey, 1, sizeof(INT), fpSrc) < 0)
		{
			dpt();
			iExit = TRUE;
			break;
		}

		if (FS_read(&iType, 1, sizeof(INT), fpSrc) < 0)
		{
			dpt();
			iExit = TRUE;
			break;
		}

		switch (iType)
		{
			case VTYPE_FIXED_SIZE:
			case VTYPE_STRING:
				if (FS_read(&iLen, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

				cpValue = mtiMalloc(iLen + 1);
				if (FS_read(cpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					mtiFree(cpValue);	// @@EB MALLOC MODIFY
					cpValue = NULL;	// @@EB MALLOC MODIFY
					break;
				}
				mtiFree(cpValue);
				cpValue = NULL;	// @@EB MALLOC MODIFY
				break;

			case VTYPE_INT:
				if (FS_read(&iValue, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_LONG:
				if (FS_read(&lValue, 1, sizeof(LONG), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;
			case VTYPE_LONGLONG:
				if (FS_read(&llValue, 1, sizeof(LLONG), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;
		}
	}

	if (iExit == TRUE)
	{
		return RTN_ERROR;
	}
	else
	{
		return RTN_SUCCESS;
	}
}

INT mtiFileToMapPutElements(S_FS_FILE *fpSrc, INT iCount, tMtiMap *tpMap)
{
	INT iExit = 0, i = 0, iKey = 0, iType = 0, iLen = 0, iValue = 0;
	LONG lValue = 0L;
	LLONG llValue = 0LL;
	CHAR *cpValue = NULL;
	UCHAR *ucpValue = NULL;

	iExit = FALSE;
	for (i = 0; i < iCount; i++)
	{
		if (iExit)
		{
			break;
		}

		if (FS_read(&iKey, 1, sizeof(INT), fpSrc) < 0)
		{
			dpt();
			iExit = TRUE;
			break;
		}

		if (FS_read(&iType, 1, sizeof(INT), fpSrc) < 0)
		{
			dpt();
			iExit = TRUE;
			break;
		}

		switch (iType)
		{
			case VTYPE_FIXED_SIZE:
				if (FS_read(&iLen, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

#ifndef SPEED_INOVATE
				ucpValue = mtiMalloc(iLen);
				if (FS_read(ucpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					mtiFree(ucpValue);		// @@EB MALLOC MODIFY
					ucpValue = NULL;	// @@EB MALLOC MODIFY
					break;
				}

				mtiMapPutBytes(tpMap, iKey, ucpValue, iLen);
				mtiFree(ucpValue);
				ucpValue = NULL;	// @@EB MALLOC MODIFY

#else
				ucpValue = mtiMapCreateBytes(tpMap, iKey, iLen);
				if (FS_read(ucpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
#endif
				break;

			case VTYPE_STRING:
				if (FS_read(&iLen, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

#ifndef SPEED_INOVATE
				cpValue = mtiMalloc(iLen + 1);
				if (FS_read(cpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					mtiFree(cpValue);	// @@EB MALLOC MODIFY
					cpValue = NULL;	// @@EB MALLOC MODIFY
					break;
				}

				mtiMapPutString(tpMap, iKey, cpValue);
				mtiFree(cpValue);
				cpValue = NULL;	// @@EB MALLOC MODIFY
#else
				cpValue = mtiMapCreateString(tpMap, iKey, iLen);
				if (FS_read(cpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
#endif
				break;

			case VTYPE_INT:
				if (FS_read(&iValue, 1, sizeof(INT), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

				mtiMapPutInt(tpMap, iKey, iValue);
				break;

			case VTYPE_LONG:
				if (FS_read(&lValue, 1, sizeof(LONG), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

				mtiMapPutLong(tpMap, iKey, lValue);
				break;
			case VTYPE_LONGLONG:
				if (FS_read(&llValue, 1, sizeof(LLONG), fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}

				mtiMapPutLLong(tpMap, iKey, llValue);
				break;
		}
	}

	if (iExit == TRUE)
	{
		return RTN_ERROR;
	}
	else
	{
		return RTN_SUCCESS;
	}
}


INT mtiLoadRecordCount(CHAR *cpFilename)
{
	S_FS_FILE *fpTemp = NULL;
	INT iRecordCount = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		if (FS_read(&iRecordCount, 1, sizeof(INT), fpTemp) < 0)
		{
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		FS_close(fpTemp);
		return iRecordCount;
	}

	return RTN_ERROR;

}

INT mtiFileToRecordMap(CHAR *cpFilename, tMtiMap *tpAllMap)
{
	S_FS_FILE *fpTemp = NULL;
	INT iRecordCount = 0, i = 0, iIndex = 0, iCount = 0, iRet = RTN_ERROR;
	CHAR caPath[MAX_FILE_PATH];
#ifndef SPEED_INOVATE
	tMtiMap tMapTemp;
	tMtiMap *tpMapTemp = &tMapTemp;
#else
	tMtiMap *tpMapTemp = NULL;
#endif

#ifndef SPEED_INOVATE
	mtiMapInit(tpMapTemp);
#endif

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		if (FS_read(&iRecordCount, 1, sizeof(INT), fpTemp) > 0)
		{
			for (i = 0; i < iRecordCount; i++)
			{
				if (FS_read(&iIndex, 1, sizeof(INT), fpTemp) <= 0)
				{
					break;
				}

				if (FS_read(&iCount, 1, sizeof(INT), fpTemp) <= 0)
				{
					break;
				}

#ifdef SPEED_INOVATE
				tpMapTemp = mtiMapCreateMap(tpAllMap, tpAllMap->iMapCount);
				if (tpMapTemp != NULL)
				{
					iRet = mtiFileToMapPutElements(fpTemp, iCount, tpMapTemp);
					if (iRet != RTN_SUCCESS)
					{
						dpt();
						mtiMapClear(tpMapTemp);
					}
				}
#else
				iRet = mtiFileToMapPutElements(fpTemp, iCount, tpMapTemp);
				if (iRet == RTN_SUCCESS)
				{
					mtiMapPutMap(tpAllMap, tpAllMap->iMapCount, tpMapTemp);
				}

				mtiMapClear(tpMapTemp);
#endif
			}
		}

		FS_close(fpTemp);
	}

	return iRet;
}

INT mtiRecordMapToFile(tMtiMap *tpAllMap, CHAR *cpFilename)
{
	S_FS_FILE *fpRec = NULL;
	INT i = 0, iCount = 0, iIndex = 0, iRet = RTN_ERROR;
	CHAR caPath[MAX_FILE_PATH];
	tMtiMap *tpMapTemp = NULL;

	if (tpAllMap->iMapCount > 0)
	{
		mtiMemset(caPath, 0, sizeof(caPath));
		sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);

		fpRec = mtiFileOpen(caPath, "w");

		if (fpRec != NULL)
		{
			if (FS_write(&tpAllMap->iMapCount, 1, sizeof(INT), fpRec) > 0)
			{
				for (i = 0; i < tpAllMap->iMapCount; i++)
				{
					iIndex = MemCheck(tpAllMap, i);

					tpMapTemp = mtiMapGetMap(tpAllMap, iIndex);
					iCount = tpMapTemp->iMapCount;

					if (FS_write(&iIndex, 1, sizeof(INT), fpRec) < 0)
					{
						break;
					}

					if (FS_write(&iCount, 1, sizeof(INT), fpRec) < 0)
					{
						break;
					}

					iRet = mtiMapGetElementsToFile(tpMapTemp, fpRec);

					if (iRet != RTN_SUCCESS)
					{
						break;
					}
				}
			}

			FS_close(fpRec);
		}
	}

	return iRet;
}

INT mtiSaveRecordMapAddToFile(tMtiMap *tpMap, CHAR *cpFilename)
{
	S_FS_FILE *fpTemp = NULL, *fpNew = NULL;
	INT i = 0, iCount = 0, iIndex = 0, iRecordCount = 0;
	CHAR caPath[MAX_FILE_PATH];
	CHAR caNew[MAX_FILE_PATH];
	tMtiMap tMapTemp;

	mtiMemset(caNew, 0, sizeof(caNew));
	mtiMapInit(&tMapTemp);

	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);
	sprintf(caNew, "%s/%s.new", MOUNT_MTI, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	fpNew = mtiFileOpen(caNew, "w");

	if (fpTemp != NULL && fpNew != NULL)
	{
		dmsg("iRecordCount = %d", iRecordCount);
		if (FS_read(&iRecordCount, 1, sizeof(INT), fpTemp) < 0)
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		if (FS_write(&iRecordCount, 1, sizeof(INT), fpNew) <= 0)
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		for (i = 0; i < iRecordCount; i++)
		{
			if (FS_read(&iIndex, 1, sizeof(INT), fpTemp) <= 0)
			{
				break;
			}

			if (FS_write(&iIndex, 1, sizeof(INT), fpNew) < 0)
			{
				INIT_VAR();
				return RTN_ERROR;
			}

			if (FS_read(&iCount, 1, sizeof(INT), fpTemp) <= 0)
			{
				INIT_VAR();
				return RTN_ERROR;
			}

			if (FS_write(&iCount, 1, sizeof(INT), fpNew) < 0)
			{
				INIT_VAR();
				return RTN_ERROR;
			}

			mtiMapClear(&tMapTemp);
			if (mtiFileToMapPutElements(fpTemp, iCount, &tMapTemp) == RTN_SUCCESS)
			{
				if (mtiMapGetElementsToFile(&tMapTemp, fpNew) != RTN_SUCCESS)
				{
					INIT_VAR();
					return RTN_ERROR;
				}
			}
			else
			{
				INIT_VAR();
				return RTN_ERROR;
			}
		}
	}

	if (fpNew != NULL)
	{
		if (fpTemp == NULL)
		{
			iRecordCount = 1;
			if (FS_write(&iRecordCount, 1, sizeof(INT), fpNew) <= 0)
			{
				INIT_VAR();
				return RTN_ERROR;
			}
			iRecordCount = 0;
		}

		iIndex++;
		if (FS_write(&iIndex, 1, sizeof(INT), fpNew) < 0)
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		if (FS_write(&tpMap->iMapCount, 1, sizeof(INT), fpNew) < 0)
		{
			INIT_VAR();
			return RTN_ERROR;
		}
		mtiMapGetElementsToFile(tpMap, fpNew);

		iRecordCount++;
		FS_seek(fpNew, 0, SEEK_SET);
		FS_write(&iRecordCount, 1, sizeof(INT), fpNew);
		FS_close(fpNew);

		if (fpTemp != NULL)
		{
			FS_close(fpTemp);
			FS_unlink(caPath);
		}

		FS_rename(caNew, caPath);
	}
	else
	{
		mtiMapClear(&tMapTemp);
		return RTN_ERROR;
	}

	mtiMapClear(&tMapTemp);
	return RTN_SUCCESS;
}

INT mtiMapToFile(tMtiMap *tpMap, CHAR *cpFilename)
{
	S_FS_FILE *fpTemp = NULL;
	INT iRecordCount = 0, iIndex = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);

	fpTemp = mtiFileOpen(caPath, "w");
	if (fpTemp != NULL)
	{
		if (FS_write(&iRecordCount, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		if (FS_write(&iIndex, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		if (FS_write(&tpMap->iMapCount, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		mtiMapGetElementsToFile(tpMap, fpTemp);
		FS_close(fpTemp);
	}
	else
	{
		dpt();
		return RTN_ERROR;
	}

	return RTN_SUCCESS;
}

INT mtiFileToMap(CHAR *cpFilename, tMtiMap *tpMap)
{
	S_FS_FILE *fpTemp = NULL;
	INT iCount = 0, iIndex = 0, iRecordCount = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		if (FS_read(&iRecordCount, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		if (FS_read(&iIndex, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			return RTN_ERROR;
		}

		if (FS_read(&iCount, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		if (iCount > 0)
		{
			mtiMapClear(tpMap);
			mtiFileToMapPutElements(fpTemp, iCount, tpMap);
		}
		else
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		tpMap->iMapCount = iCount;
		FS_close(fpTemp);
	}
	else
	{
		dpt();
		return RTN_ERROR;
	}

	return RTN_SUCCESS;
}

INT mtiWriteFileAndClose(CHAR *cpFilename, UCHAR *uchData, INT nLength)
{
	S_FS_FILE *fpTemp = NULL;
	INT nResult = RTN_SUCCESS;
	CHAR szPath[MAX_FILE_PATH] = { '\0', };

	sprintf(szPath, "%s/%s", MOUNT_MTI, cpFilename);

	if (NULL != cpFilename && NULL != uchData && 0 != nLength)
	{
		if(FS_OK==FS_exist(szPath))
		{
			FS_unlink(szPath);
		}

		fpTemp = FS_open(szPath, "a");
		if (NULL == fpTemp)
		{
			dmsg("mtiWriteFileAndClose - %s OPEN FAIL !!", szPath);
			return RTN_ERROR;
		}

		nResult = FS_write(uchData, 1, nLength, fpTemp);

		if (nResult != nLength)
		{
			dmsg("mtiWriteFileAndClose - %s WRITE FAIL !!", szPath);
			nResult = RTN_ERROR;
		}

		FS_close(fpTemp);
	}

	return nResult;
}

INT mtiReadFileAndClose(CHAR *cpFilename, UCHAR *uchData, INT nLength)
{
	S_FS_FILE *fpTemp = NULL;
	INT nResult = RTN_SUCCESS;
	CHAR szPath[MAX_FILE_PATH];

	sprintf(szPath, "%s/%s", MOUNT_MTI, cpFilename);

	if (NULL != cpFilename && NULL != uchData)
	{
		fpTemp = FS_open(szPath, "r");
		if (NULL == fpTemp)
		{
			dmsg("mtiReadFileAndClose - %s OPEN FAIL !!", szPath);
			return RTN_ERROR;
		}

		nResult = FS_read(uchData, 1, nLength, fpTemp);

		if (nResult > nLength)
		{
			dmsg("mtiReadFileAndClose - %s WRITE FAIL !!", szPath);
			nResult = RTN_ERROR;
		}

		FS_close(fpTemp);
	}

	return nResult;
}

VOID mtiFinalizeFileSystem()
{
	FS_unmount(MOUNT_MTI);
}

VOID mtiFileDbg(CHAR *cpFileName)
{
	S_FS_FILE *file = NULL;
	INT iOffset = 0, iReadSize = 0;
	CHAR *readBuff = NULL;
	LONG lFileSize = 0;

	file = FS_open(cpFileName, "r");

	if(file == NULL)
	{
		dmsg("File [%s] Open Error", cpFileName);
		return;
	}

	lFileSize = FS_length(file);

	if(lFileSize < 1)
	{
		dmsg("File [%s] Size [%ld] Error", cpFileName, lFileSize);
		FS_close(file);
		return ;
	}

	readBuff =(CHAR *)mtiMalloc(lFileSize);

	if(readBuff == NULL)
	{
		dmsg("mtiMalloc is failed!");
		mtiFree(readBuff);		// @@EB MALLOC MODIFY
		return;
	}

	while(1)
	{
		iReadSize = FS_read(readBuff+(iOffset++), 1, 1, file);
		if(iReadSize < 1)
			break;
	}

	dmsg("FILE[%s] SIZE[%d]Contents...", cpFileName, iOffset);
	dmsg("------------------------------");
	dbuf("FILE READ BUFF",(UCHAR *)readBuff, iOffset);
	dmsg("------------------------------");
	FS_close(file);
	mtiFree(readBuff);
}

//@@WAY, 201901 slowly trans
#ifdef __TETRA__
INT mtiMapToFilePerTrans(tMtiMap *tpMap, CHAR *cpFilename, int iIndexRec)
{
	S_FS_FILE *fpTemp = NULL;
	INT iRecordCount = 0, iIndex = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	//sprintf(caPath, "%s/%s", MOUNT_MTI, cpFilename);
	sprintf(caPath, "%s/%s%03d", MOUNT_MTI, cpFilename, iIndexRec);

	fpTemp = mtiFileOpen(caPath, "w");
	if (fpTemp != NULL)
	{
		if (FS_write(&iRecordCount, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		if (FS_write(&iIndex, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		if (FS_write(&tpMap->iMapCount, 1, sizeof(INT), fpTemp) < 0)
		{
			dpt();
			FS_close(fpTemp);
			return RTN_ERROR;
		}

		mtiMapGetElementsToFile(tpMap, fpTemp);
		FS_close(fpTemp);
	}
	else
	{
		dpt();
		return RTN_ERROR;
	}

	return RTN_SUCCESS;
}

#endif
