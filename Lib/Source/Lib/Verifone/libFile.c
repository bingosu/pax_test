#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SPEED_INOVATE

#include "libMtiCommonApi.h"
#include "apSubPj.h"

#define MTI_DATA_PATH	"F:15/MTI_DATA"

#define INIT_VAR()											\
	dpt();													\
	dmsg("Error [%s]\n", strerror(errno));					\
	if (fpTemp < 0) fclose(fpTemp); fpTemp = 0;				\
	if (fpNew < 0) fclose(fpNew); fpNew = 0;					\
	mtiMapClear(&tMapTemp);

/***
static VOID dirTest()
{
	int iRet;
	int len =100;
	char fontName[100]={0,};
	iRet = len = 0;

	iRet = chdir("/MTI_DATA");
	dmsg("chdir iRet[%d]\n",iRet);
	if(iRet < 0)
		dmsg("error[%s]\n", strerror(errno));

	memset(fontName, 0x00, sizeof(fontName));

	iRet = getcwd(fontName, len);
	dmsg("getcwd iret[%d]\tlen[%d]\tresult[%s]\n", iRet, len, fontName);
	if(iRet < 0)
		dmsg("error[%s]\n", strerror(errno));

	memset(fontName, 0x00, sizeof(fontName));

	dir_get_first(fontName);
	dmsg("filename[%s]\n", fontName);
	memset(fontName, 0x00, sizeof(fontName));
	while(dir_get_next(fontName) != -1)
	{
		dmsg("filename[%s]\n", fontName);
		memset(fontName, 0x00, sizeof(fontName));
	}
	return;
}
***/

VOID mtiInitializeFileSystem()
{
	INT iRet = 0;
	iRet = mkdir(MTI_DATA_PATH);
	if(iRet < 0){
		dmsg("mkdir [%s] Error [%s]\n",MTI_DATA_PATH, strerror(errno));
	}

	iRet = mkdir(TMS_UPDATEDIR);
	if(iRet < 0){
		dmsg("mkdir [%s] Error [%s]\n",TMS_UPDATEDIR, strerror(errno));
	}

	iRet = mkdir(TMS_OLDPATH);
	if(iRet < 0){
		dmsg("mkdir [%s] Error [%s]\n",TMS_OLDPATH, strerror(errno));
	}
}

INT mtiFileDelete(CHAR *cpFilename)
{
	CHAR caPath[MAX_FILE_PATH] = {0,};
	INT iRet = 0;

	if(strstr(cpFilename, MTI_DATA_PATH) == NULL)
		sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);
	else
		mtiStrcpy(caPath, cpFilename, sizeof(caPath));

	dmsg("%s File[%s]",__func__, caPath);
	iRet = remove(caPath);
	if(iRet < 0){
		dpt();
		dmsg("iRet[%d]\tError [%s]\n", iRet, strerror(errno));
		return RTN_ERROR;
	}
	return RTN_SUCCESS;
}

static FILE *mtiFileOpen(CHAR *cpFile, CHAR *cpMode)
{
    FILE* fpTemp;

    fpTemp = fopen(cpFile, (char *) cpMode);

    if(fpTemp == NULL && cpMode[0] == 'w')
	{
    	fpTemp = fopen(cpFile, "a");
    }

    if(fpTemp == NULL){
		dmsg("Filename [%s] MODE[%s]\n",cpFile, cpMode);
		dmsg("Error [%s]\n", strerror(errno));
		return RTN_ERROR;
	}

    return fpTemp;
}

INT mtiMapGetElementsToFile(tMtiMap *tpMap, FILE *fpSrc)
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
		if (fwrite(&iKey, 1, sizeof(INT), fpSrc) != sizeof(INT))
		{
			dpt();
			iExit = TRUE;
			break;
		}

		iType = MemType(tpMap, iKey);
		if (fwrite(&iType, 1, sizeof(INT), fpSrc) != sizeof(INT))
		{
			dpt();
			iExit = TRUE;
			break;
		}

		switch (iType)
		{
			case VTYPE_FIXED_SIZE:
				ucpValue = mtiMapGetBytes(tpMap, iKey, &iLen);
				if (fwrite(&iLen, 1, sizeof(INT), fpSrc) != sizeof(INT) )
				{
					dpt();
					iExit = TRUE;
					break;
				}

				if (fwrite(ucpValue, 1, iLen, fpSrc) != iLen)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_STRING:
				cpValue = mtiMapGetString(tpMap, iKey);
				iLen = mtiStrlen(MAX_ALLOC_SIZE, cpValue);
				if (fwrite(&iLen, 1, sizeof(INT), fpSrc) != sizeof(INT))
				{
					dpt();
					iExit = TRUE;
					break;
				}

				if (fwrite(cpValue, 1, iLen, fpSrc) != iLen)
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_INT:
				iValue = mtiMapGetInt(tpMap, iKey);
				if (fwrite(&iValue, 1, sizeof(INT), fpSrc) != sizeof(INT))
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_LONG:
				lValue = mtiMapGetLong(tpMap, iKey);
				if (fwrite(&lValue, 1, sizeof(LONG), fpSrc) != sizeof(LONG))
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;
			case VTYPE_LONGLONG:
				llValue = mtiMapGetLLong(tpMap, iKey);
				if (fwrite(&llValue, 1, sizeof(LLONG), fpSrc) != sizeof(LLONG))
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
		dmsg("Error [%s]\n", strerror(errno));
		return RTN_ERROR;
	}
	else
	{
		return RTN_SUCCESS;
	}
}

INT mtiGetFileSize(CHAR *cpFilename)
{
	FILE* fpTemp;
	INT iSize = -1;
	CHAR caPath[MAX_FILE_PATH];

	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);
	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		iSize = 0;
		while (fgetc(fpTemp) != EOF)
		{
			iSize++;
		}

		fclose(fpTemp);
	}

	return iSize;
}

INT mtiFileToCalcMapSizeMove(FILE *fpSrc, INT iCount)
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

		if (fread(&iKey, 1, sizeof(INT), fpSrc) < sizeof(INT))
		{
			dpt();
			iExit = TRUE;
			break;
		}

		if (fread(&iType, 1, sizeof(INT), fpSrc) < sizeof(INT))
		{
			dpt();
			iExit = TRUE;
			break;
		}

		switch (iType)
		{
			case VTYPE_FIXED_SIZE:
			case VTYPE_STRING:
				if (fread(&iLen, 1, sizeof(INT), fpSrc) < sizeof(INT))
				{
					dpt();
					iExit = TRUE;
					break;
				}

				cpValue = mtiMalloc(iLen + 1);
				if (fread(cpValue, 1, iLen, fpSrc) < iLen)
				{
					dpt();
					iExit = TRUE;
					mtiFree(cpValue);	// @@EB MALLOC MODIFY
					cpValue = NULL;		// @@EB MALLOC MODIFY
					break;
				}
				mtiFree(cpValue);
				cpValue = NULL;		// @@EB MALLOC MODIFY
				break;

			case VTYPE_INT:
				if (fread(&iValue, 1, sizeof(INT), fpSrc) < sizeof(INT))
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;

			case VTYPE_LONG:
				if (fread(&lValue, 1, sizeof(LONG), fpSrc) < sizeof(LONG))
				{
					dpt();
					iExit = TRUE;
					break;
				}
				break;
			case VTYPE_LONGLONG:
				if (fread(&llValue, 1, sizeof(LLONG), fpSrc) < sizeof(LLONG))
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
		dmsg("Error [%s]\n", strerror(errno));
		return RTN_ERROR;
	}
	else
	{
		return RTN_SUCCESS;
	}
}

INT mtiFileToMapPutElements(FILE *fpSrc, INT iCount, tMtiMap *tpMap)
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

		if (fread(&iKey, 1, sizeof(INT), fpSrc) < sizeof(INT))
		{
			dpt();
			iExit = TRUE;
			break;
		}

		if (fread(&iType, 1, sizeof(INT), fpSrc) < sizeof(INT))
		{
			dpt();
			iExit = TRUE;
			break;
		}

		switch (iType)
		{
			case VTYPE_FIXED_SIZE:
				if (fread(&iLen, 1, sizeof(INT), fpSrc) < sizeof(INT))
				{
					dpt();
					iExit = TRUE;
					break;
				}

#ifndef SPEED_INOVATE
				ucpValue = mtiMalloc(iLen);
				if (fread(ucpValue, 1, iLen, fpSrc) < iLen)
				{
					dpt();
					iExit = TRUE;
                                        mtiFree(ucpValue);	// @@EB MALLOC MODIFY
					ucpValue = NULL;	// @@EB MALLOC MODIFY
					break;
				}


				mtiMapPutBytes(tpMap, iKey, ucpValue, iLen);
				mtiFree(ucpValue);
                                ucpValue = NULL;	// @@EB MALLOC MODIFY
#else
				ucpValue = mtiMapCreateBytes(tpMap, iKey, iLen);
				if (fread(ucpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
#endif
				break;

			case VTYPE_STRING:
				if (fread(&iLen, 1, sizeof(INT), fpSrc) < sizeof(INT))
				{
					dpt();
					iExit = TRUE;
					break;
				}

#ifndef SPEED_INOVATE
				cpValue = mtiMalloc(iLen + 1);
				if (fread(cpValue, 1, iLen, fpSrc) < iLen)
				{
					dpt();
					iExit = TRUE;
                                        mtiFree(ucpValue);	// @@EB MALLOC MODIFY
					ucpValue = NULL;	// @@EB MALLOC MODIFY
					break;
				}

				
				mtiMapPutString(tpMap, iKey, cpValue);
				mtiFree(cpValue);
				ucpValue = NULL;	// @@EB MALLOC MODIFY
#else
				cpValue = mtiMapCreateString(tpMap, iKey, iLen);
				if (fread(cpValue, 1, iLen, fpSrc) < 0)
				{
					dpt();
					iExit = TRUE;
					break;
				}
#endif
				break;

			case VTYPE_INT:
				if (fread(&iValue, 1, sizeof(INT), fpSrc) < sizeof(INT))
				{
					dpt();
					iExit = TRUE;
					break;
				}

				mtiMapPutInt(tpMap, iKey, iValue);
				break;

			case VTYPE_LONG:
				if (fread(&lValue, 1, sizeof(LONG), fpSrc) < sizeof(LONG))
				{
					dpt();
					iExit = TRUE;
					break;
				}
				mtiMapPutLong(tpMap, iKey, lValue);
				break;
			case VTYPE_LONGLONG:
				if (fread(&llValue, 1, sizeof(LLONG), fpSrc) < sizeof(LLONG))
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
		dmsg("Error [%s]\n", strerror(errno));
		return RTN_ERROR;
	}
	else
	{
		return RTN_SUCCESS;
	}
}


INT mtiLoadRecordCount(CHAR *cpFilename)
{
	FILE *fpTemp = NULL;
	INT iRecordCount = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	if(fpTemp != NULL)
	{
		if (fread(&iRecordCount, 1, sizeof(INT), fpTemp) < sizeof(INT))
		{
			dpt();
			fclose(fpTemp);
			dmsg("Error [%s]\n", strerror(errno));
			return RTN_ERROR;
		}

		fclose(fpTemp);
		return iRecordCount;
	}

	dmsg("Error [%s]\n", strerror(errno));
	return RTN_ERROR;
}

INT mtiLoadIndexRecordMapFromFile(CHAR *cpFilename, INT iUpdateIndex, tMtiMap *tpMap)
{
	FILE* fpTemp = 0;
	INT i = 0, iCount = 0, iIndex = 0, iRecordCount = 0, iRet = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		if (fread(&iRecordCount, 1, sizeof(INT), fpTemp) < sizeof(INT))
		{
			dpt();
			fclose(fpTemp);
			dmsg("Error [%s]\n", strerror(errno));
			return RTN_ERROR;
		}

		for (i = 0; i < iRecordCount; i++)
		{
			if (fread(&iIndex, 1, sizeof(INT), fpTemp) <= 0)
			{
				dpt();
				break;
			}

			if (fread(&iCount, 1, sizeof(INT), fpTemp) <= 0)
			{
				dpt();
				fclose(fpTemp);
				dmsg("Error [%s]\n", strerror(errno));
				return RTN_ERROR;
			}

			if (iIndex == (iUpdateIndex + 1))
			{
				mtiMapClear(tpMap);
				iRet = mtiFileToMapPutElements(fpTemp, iCount, tpMap);
				fclose(fpTemp);
				return iRet;
			}
			else
			{
				iRet = mtiFileToCalcMapSizeMove(fpTemp, iCount);
				if (iRet != RTN_SUCCESS)
				{
					fclose(fpTemp);
					return iRet;
				}
			}
		}

		fclose(fpTemp);
	}
	return RTN_ERROR;
}

INT mtiUpdateIndexRecordMapFromFile(tMtiMap *tpMap,  INT iUpdateIndex, CHAR *cpFilename)
{
	FILE *fpTemp = NULL, *fpNew = NULL;
	INT i = 0, iCount = 0, iIndex = 0, iRecordCount = 0, iRet = 0;
	CHAR caPath[MAX_FILE_PATH];
	CHAR caNew[MAX_FILE_PATH];
	tMtiMap tMapTemp;

	mtiMemset(caPath, 0, sizeof(caPath));
	mtiMemset(caNew, 0, sizeof(caNew));
	mtiMapInit(&tMapTemp);

	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);
	sprintf(caNew, "%s/%s.new", MTI_DATA_PATH, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	fpNew = mtiFileOpen(caNew, "w");
	if (fpTemp != NULL && fpNew != NULL)
	{
		if (fread(&iRecordCount, 1, sizeof(INT), fpTemp) < sizeof(INT))
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		if (fwrite(&iRecordCount, 1, sizeof(INT), fpNew) <= 0)
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		for (i = 0; i < iRecordCount; i++)
		{
			if (fread(&iIndex, 1, sizeof(INT), fpTemp) <= 0)
			{
				dmsg("Error [%s]\n", strerror(errno));
				break;
			}

			if (fwrite(&iIndex, 1, sizeof(INT), fpNew) < sizeof(INT))
			{
				INIT_VAR();
				return RTN_ERROR;
			}

			if (fread(&iCount, 1, sizeof(INT), fpTemp) <= 0)
			{
				INIT_VAR();
				return RTN_ERROR;
			}

			if (iIndex == (iUpdateIndex + 1))
			{
				if (fwrite(&tpMap->iMapCount, 1, sizeof(INT), fpNew) < sizeof(INT))
				{
					INIT_VAR();
					return RTN_ERROR;
				}

				if (mtiMapGetElementsToFile(tpMap, fpNew) != RTN_SUCCESS)
				{
					INIT_VAR();
					return RTN_ERROR;
				}

				iRet = mtiFileToCalcMapSizeMove(fpTemp, iCount);
				if (iRet != RTN_SUCCESS)
				{
					fclose(fpTemp);
					return iRet;
				}
			}
			else
			{
				if (fwrite(&iCount, 1, sizeof(INT), fpNew) < sizeof(INT))
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
	}

	if (fpNew != NULL)
	{
		fclose(fpNew);

		if (fpTemp != NULL)
		{
			fclose(fpTemp);
			mtiFileDelete(caPath);
		}

		_rename(caNew, caPath);
	}
	else
	{
		mtiMapClear(&tMapTemp);
		return RTN_ERROR;
	}

	mtiMapClear(&tMapTemp);
	return RTN_SUCCESS;
}

INT mtiFileToRecordMap(CHAR *cpFilename, tMtiMap *tpAllMap)
{
	FILE *fpTemp = NULL;
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
	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		if (fread(&iRecordCount, 1, sizeof(INT), fpTemp) > 0)
		{
			for (i = 0; i < iRecordCount; i++)
			{
				if (fread(&iIndex, 1, sizeof(INT), fpTemp) <= 0)
				{
					break;
				}

				if (fread(&iCount, 1, sizeof(INT), fpTemp) <= 0)
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

		fclose(fpTemp);
	}

	return iRet;
}

INT mtiRecordMapToFile(tMtiMap *tpAllMap, CHAR *cpFilename)
{
	FILE *fpRec = NULL;
	INT i = 0, iCount = 0, iIndex = 0, iRet = RTN_ERROR;
	CHAR caPath[MAX_FILE_PATH];
	tMtiMap *tpMapTemp = NULL;

	if (tpAllMap->iMapCount > 0)
	{
		mtiMemset(caPath, 0, sizeof(caPath));
		sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);

		fpRec = mtiFileOpen(caPath, "w");
		if (fpRec != NULL)
		{
			if (fwrite(&tpAllMap->iMapCount, 1, sizeof(INT), fpRec) > 0)
			{
				for (i = 0; i < tpAllMap->iMapCount; i++)
				{
					iIndex = MemCheck(tpAllMap, i);

					tpMapTemp = mtiMapGetMap(tpAllMap, iIndex);
					iCount = tpMapTemp->iMapCount;

					if (fwrite(&iIndex, 1, sizeof(INT), fpRec) != sizeof(INT))
					{
						break;
					}

					if (fwrite(&iCount, 1, sizeof(INT), fpRec) != sizeof(INT))
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

			fclose(fpRec);
		}
#warning DEBUG REC BATCH		
#if 0
		//FILE SIZE CHECK
		{
			struct stat fileInfo;

			SVC_WAIT(100);
			stat(caPath, &fileInfo);
			dmsg("FILE[%s] SIZE[%u] Bytes", caPath, fileInfo.st_size);
		}
#endif
	}

	return iRet;
}

INT mtiSaveRecordMapAddToFile(tMtiMap *tpMap, CHAR *cpFilename)
{
	FILE *fpTemp = NULL;
	FILE *fpNew = NULL;
	INT i = 0, iCount = 0, iIndex = 0, iRecordCount = 0;
	CHAR caPath[MAX_FILE_PATH];
	CHAR caNew[MAX_FILE_PATH];
	tMtiMap tMapTemp;

	mtiMemset(caNew, 0, sizeof(caNew));
	mtiMapInit(&tMapTemp);

	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);
	sprintf(caNew, "%s/%s.new", MTI_DATA_PATH, cpFilename);

	fpTemp = mtiFileOpen(caPath, "r");
	fpNew = mtiFileOpen(caNew, "w");

	if (fpTemp != NULL && fpNew != NULL)
	{
		if (fread(&iRecordCount, 1, sizeof(INT), fpTemp) < sizeof(INT))
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		if (fwrite(&iRecordCount, 1, sizeof(INT), fpNew) <= 0)
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		for (i = 0; i < iRecordCount; i++)
		{
			if (fread(&iIndex, 1, sizeof(INT), fpTemp) <= 0)
			{
				break;
			}

			if (fwrite(&iIndex, 1, sizeof(INT), fpNew) < sizeof(INT))
			{
				INIT_VAR();
				return RTN_ERROR;
			}

			if (fread(&iCount, 1, sizeof(INT), fpTemp) <= 0)
			{
				INIT_VAR();
				return RTN_ERROR;
			}

			if (fwrite(&iCount, 1, sizeof(INT), fpNew) < sizeof(INT))
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
			if (fwrite(&iRecordCount, 1, sizeof(INT), fpNew) <= 0)
			{
				INIT_VAR();
				return RTN_ERROR;
			}
		}

		iIndex++;
		if (fwrite(&iIndex, 1, sizeof(INT), fpNew) < sizeof(INT))
		{
			INIT_VAR();
			return RTN_ERROR;
		}

		if (fwrite(&tpMap->iMapCount, 1, sizeof(INT), fpNew) < sizeof(INT))
		{
			INIT_VAR();
			return RTN_ERROR;
		}
		mtiMapGetElementsToFile(tpMap, fpNew);

		iRecordCount++;
		fseek(fpNew, 0, SEEK_SET);
		fwrite(&iRecordCount, 1, sizeof(INT), fpNew);
		fclose(fpNew);
		if (fpTemp != NULL)
		{
			fclose(fpTemp);
			mtiFileDelete(caPath);
		}

		if(_rename(caNew, caPath) != 0)
		{
			dpt();
			dmsg("Error [%s]\n", strerror(errno));
		}
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
	FILE *fpTemp = NULL;
	INT iRecordCount = 0, iIndex = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);

	fpTemp = mtiFileOpen(caPath, "w");
	if (fpTemp != NULL)
	{
		if (fwrite(&iRecordCount, 1, sizeof(INT), fpTemp) != sizeof(INT))
		{
			dpt();
			dmsg("Error [%s]\n", strerror(errno));
			fclose(fpTemp);
			return RTN_ERROR;
		}

		if (fwrite(&iIndex, 1, sizeof(INT), fpTemp) != sizeof(INT))
		{
			dpt();
			dmsg("Error [%s]\n", strerror(errno));
			fclose(fpTemp);
			return RTN_ERROR;
		}

		if (fwrite(&tpMap->iMapCount, 1, sizeof(INT), fpTemp) != sizeof(INT))
		{
			dpt();
			dmsg("Error [%s]\n", strerror(errno));
			fclose(fpTemp);
			return RTN_ERROR;
		}

		mtiMapGetElementsToFile(tpMap, fpTemp);
		fclose(fpTemp);
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
	FILE *fpTemp = NULL;
	INT iCount = 0, iIndex = 0, iRecordCount = 0;
	CHAR caPath[MAX_FILE_PATH];

	mtiMemset(caPath, 0, sizeof(caPath));
	sprintf(caPath, "%s/%s", MTI_DATA_PATH, cpFilename);


	fpTemp = mtiFileOpen(caPath, "r");
	if (fpTemp != NULL)
	{
		if (fread(&iRecordCount, 1, sizeof(INT), fpTemp) < sizeof(INT))
		{
			dpt();
			dmsg("Error [%s]\n", strerror(errno));
			fclose(fpTemp);
			return RTN_ERROR;
		}


		if (fread(&iIndex, 1, sizeof(INT), fpTemp) < sizeof(INT))
		{
			dpt();
			dmsg("Error [%s]\n", strerror(errno));
			return RTN_ERROR;
		}


		if (fread(&iCount, 1, sizeof(INT), fpTemp) < sizeof(INT))
		{
			dpt();
			dmsg("Error [%s]\n", strerror(errno));
			fclose(fpTemp);
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
			fclose(fpTemp);
			return RTN_ERROR;
		}

		tpMap->iMapCount = iCount;
		fclose(fpTemp);
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
	FILE *fpTemp = NULL;
	CHAR szPath[MAX_FILE_PATH];
	INT nResult = RTN_SUCCESS;

	sprintf(szPath, "%s/%s", MTI_DATA_PATH, cpFilename);

	if (NULL != cpFilename && NULL != uchData && 0 != nLength)
	{
		fpTemp = fopen(szPath, "w");
		if (NULL == fpTemp)
		{
			dmsg("mtiWriteFileAndClose - %s OPEN FAIL !!", cpFilename);
			return RTN_ERROR;
		}

		nResult = fwrite(uchData, 1, nLength, fpTemp);

		if (nResult != nLength)
		{
			dmsg("mtiWriteFileAndClose - %s WRITE FAIL !!", cpFilename);
			nResult = RTN_ERROR;
		}
		fclose(fpTemp);
	}

	return nResult;
}

INT mtiReadFileAndClose(CHAR *cpFilename, UCHAR *uchData, INT nLength)
{
	FILE *fpTemp = NULL;
	CHAR szPath[MAX_FILE_PATH];
	INT nResult = RTN_SUCCESS;

	sprintf(szPath, "%s/%s", MTI_DATA_PATH, cpFilename);

	if (NULL != cpFilename && NULL != uchData)
	{
		fpTemp = fopen(szPath, "r");
		if (NULL == fpTemp)
		{
			dmsg("mtiReadFileAndClose - %s OPEN FAIL [%s]!!", szPath, strerror(errno));
			return RTN_ERROR;
		}

		nResult = fread(uchData, 1, nLength, fpTemp);

		if (nResult > nLength)
		{
			dmsg("mtiReadFileAndClose - %s WRITE FAIL [%s]!!", szPath, strerror(errno));
			nResult = RTN_ERROR;
		}

		fclose(fpTemp);
	}

	return nResult;
}

VOID mtiFinalizeFileSystem()
{
	//DO NOT NEED THIS FUNCTION AT THE VERIFONE DEVICE
}

VOID mtiFileDbg(CHAR *cpFileName)
{
	FILE *file;
	INT iOffset = 0, iReadSize = 0;
	CHAR *readBuff = NULL;
	struct stat fileInfo;

	memset(&fileInfo, 0x00, sizeof(fileInfo));

	if(cpFileName == NULL)
	{
		dmsg("cpFilePath is NULL!");
		return;
	}

	stat(cpFileName, &fileInfo);

	readBuff = (CHAR *)mtiMalloc(fileInfo.st_size);

	if(readBuff == NULL)
	{
		dmsg("mtiMalloc is failed!");
		return;
	}

	file = fopen(cpFileName, "r");
	if(file == NULL)
	{
		dmsg("File [%s] Open Error", cpFileName);
		mtiFree(readBuff);
		return;
	}

	while(1)
	{
		iReadSize = fread(readBuff+(iOffset++), 1, 1, file);
		if(iReadSize < 1)
			break;
	}

	dmsg("FILE[%s] SIZE[%d]Contents...", cpFileName, iOffset);
	dmsg("------------------------------");
	dbuf("FILE READ BUFF",readBuff, iOffset);
	dmsg("------------------------------");
	fclose(file);
	mtiFree(readBuff);
}
