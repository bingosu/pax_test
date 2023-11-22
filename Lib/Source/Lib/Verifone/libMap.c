
#include "libMtiCommonApi.h"

#define MEM_ALLOC(X)					malloc(X)
#define MEM_FREE(X)						free(X)

VOID MemInit(tMtiMap *pMap)
{
	pMap->tpFirstNode = NULL;
	pMap->iMapCount = 0;
}

VOID MemClear(tMtiMap *pMap)
{
	tMtiMem *tpMapNode = NULL;

	if (pMap->tpFirstNode != NULL)
	{
		tpMapNode = pMap->tpFirstNode;
		while (tpMapNode != NULL)
		{
			MEM_FREE(tpMapNode->pAllocAddr);
			MEM_FREE(tpMapNode);

			tpMapNode = tpMapNode->tpNextNode;
		}
	}

	MemInit(pMap);
}

VOID MemFree(tMtiMap *pMap, INT iID)
{
	tMtiMem *tpMapNode = NULL, *tpMapLast = NULL;
	INT i = 0, isFound = 0;

	if (pMap->tpFirstNode != NULL)
	{
		tpMapNode = pMap->tpFirstNode;

		while (tpMapNode != NULL)
		{
			if (tpMapNode->iID == iID)
			{
				if (i > 0)
				{
					if (tpMapNode->tpNextNode != NULL)
					{
						tpMapLast->tpNextNode = tpMapNode->tpNextNode;
					}
					else
					{
						tpMapLast->tpNextNode = NULL;
					}
				}
				else
				{
					if (pMap->tpFirstNode->tpNextNode != NULL)
					{
						pMap->tpFirstNode = pMap->tpFirstNode->tpNextNode;
					}
					else
					{
						pMap->tpFirstNode = NULL;
					}
				}

				MEM_FREE(tpMapNode->pAllocAddr);
				MEM_FREE(tpMapNode);
				isFound = 1;
			}

			tpMapLast = tpMapNode;
			tpMapNode = tpMapNode->tpNextNode;
			i++;

			if (isFound)
			{
				if (pMap->iMapCount > 0)
				{
					pMap->iMapCount--;
				}

				break;
			}
		}
	}
}

VOID *MemAlloc(tMtiMap *pMap, INT iSize, INT iID, INT iType)
{
	INT iAllocSize = 0;
	UCHAR *ucpAllocPtr = NULL;
	tMtiMem *tpMapItem = NULL, *tpMapNode = NULL, *tpMapLast = NULL;

	MemFree(pMap, iID);
	tpMapItem = (tMtiMem*)MEM_ALLOC(sizeof(tMtiMem));
	tpMapItem->pAllocAddr = MEM_ALLOC(iSize);

	tpMapItem->iType = iType;
	tpMapItem->iSize = iSize;
	tpMapItem->iID = iID;
	tpMapItem->tpNextNode = NULL;

	ucpAllocPtr = (UCHAR*)tpMapItem->pAllocAddr;
	for (iAllocSize = 0; iAllocSize < iSize; iAllocSize++)
	{
		(*ucpAllocPtr++) = 0;
	}

	if (pMap != NULL && pMap->tpFirstNode != NULL)
	{
		tpMapNode = pMap->tpFirstNode;

		while (tpMapNode != NULL)
		{
			tpMapLast = tpMapNode;
			tpMapNode = tpMapNode->tpNextNode;
		}

		tpMapLast->tpNextNode = tpMapItem;
	}
	else
	{
		pMap->tpFirstNode = tpMapItem;
	}

	pMap->iMapCount++;
	return tpMapItem->pAllocAddr;
}

INT MemAllocSize(tMtiMap *pMap, INT iID)
{
	tMtiMem *tpMapNode = NULL;

	if (pMap->tpFirstNode != NULL)
	{
		tpMapNode = pMap->tpFirstNode;

		while (tpMapNode != NULL)
		{
			if (tpMapNode->iID == iID)
			{
				return tpMapNode->iSize;
			}

			tpMapNode = tpMapNode->tpNextNode;
		}
	}

	return RTN_UNKNOWN_SIZE;
}

VOID *MemReference(tMtiMap *pMap, INT iID, INT *iAllocSize, INT *iAllocType)
{
	tMtiMem *tpMapNode = NULL;

	if (pMap->tpFirstNode != NULL)
	{
		tpMapNode = pMap->tpFirstNode;

		while (tpMapNode != NULL)
		{
			if (tpMapNode->iID == iID)
			{
				if (tpMapNode->pAllocAddr != NULL)
				{
					*iAllocSize = tpMapNode->iSize;
					*iAllocType = tpMapNode->iType;
					return tpMapNode->pAllocAddr;
				}
				break;
			}

			tpMapNode = tpMapNode->tpNextNode;
		}
	}
	return NULL;
}

INT MemType(tMtiMap *pMap, INT iID)
{
	tMtiMem *tpMapNode = NULL;

	if (pMap->tpFirstNode != NULL)
	{
		tpMapNode = pMap->tpFirstNode;

		while (tpMapNode != NULL)
		{
			if (tpMapNode->iID == iID)
			{
				return tpMapNode->iType;
			}

			tpMapNode = tpMapNode->tpNextNode;
		}
	}

	return VTYPE_STRING;
}

INT MemCheck(tMtiMap *pMap, INT iIndex)
{
	tMtiMem *tpMapNode = NULL;
	INT iRelativeIndex = 0;

	if (pMap->tpFirstNode != NULL)
	{
		tpMapNode = pMap->tpFirstNode;

		while (tpMapNode != NULL)
		{
			if (iRelativeIndex == iIndex)
			{
				return tpMapNode->iID;
			}

			tpMapNode = tpMapNode->tpNextNode;
			iRelativeIndex++;
		}
	}

	return -1;
}
