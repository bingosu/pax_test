
#include "libMtiCommonApi.h"
#include "html/prt.h"
#include "apSubDir.h"

extern "C" {
#include "printer.h"
}

using namespace vfiprt;
using namespace std;

VOID mtiInitializePrinter()
{
	INT prtVal = 0;

	prtSetPropertyInt(PRT_PROP_CONTRAST, 255);
	prtSetPropertyString(PRT_PROP_DEFAULT_FONT, "dejavu sans mono");
	prtGetPropertyInt(PRT_PROP_CONTRAST, &prtVal);
	dmsg("Current PRT CONTRAST - [%d]", prtVal);
	return;
}

INT mtiIsValildPrinter()
{
	return TRUE;
}

INT mtiGetPrintStatus()
{
	INT prtStatus = 0;

	prtGetPropertyInt(PRT_PROP_STATE, &prtStatus);

	switch(prtStatus)
	{
		case PRT_OK:
			prtStatus = 0;
			dmsg("PRINT STATUS OK");
			break;
		case PRT_PAPEREND:
			prtStatus = P_STATUS_NO_PAPER;
			dmsg("PRT ERR: NO PAPER");
			break;
		case PRT_UNDERVOLTAGE:
			prtStatus = P_STATUS_LOW_BATTERY;
			dmsg("PRT ERR: LOW BATTERY");
			break;
		default:
			prtStatus = P_STATUS_PRINT_ERROR;
			dmsg("PRT ERR: UNKNOWN ERROR");
			break;
	}
	return prtStatus;
}

typedef struct _tAsciiToHtmlCode
{
	char cAsciiLetter;
	char *caHtmlCode;
}tAsciiToHtmlCode;

tAsciiToHtmlCode tAsciiToHtmlCodeList[] =
{
		{'<',	"&lt;"},
		{'>',	"&gt;"},
		{'&',   "&amp;"}, //@@WAY BRIZZI 20200511
		{0x00,	 NULL},
};

tAsciiToHtmlCode tAsciiToHtmlCodeListQR[] =
{
		{'<',	"%3C"},
		{'>',	"%3E"},
		{' ',   "%20"},
		{0x00,	 NULL},
};


INT mtiAddPrintQueue(tMtiMap *tpPrtMap, CHAR cAttribute, CHAR *cpContents)
{
	CHAR caData[1024];
	CHAR *cpData = caData;
	INT iOffset = 0;
	INT iIdx = 0;
	INT iDestIdx = 0;
	INT iFoundFlag = 0;

	if (tpPrtMap == NULL)
	{
		dpt();
		return RTN_ERROR;
	}

	mtiMemset(caData, 0, sizeof(caData));
	*cpData++ = cAttribute;

	//Ascii To HTML Code
	if (CHK_BIT(cAttribute, P_MODE_QRCODE))
	{

		for(iIdx = 0; iIdx < 512; iIdx++)
		{
			iOffset = 0;
			iFoundFlag = 0;
			while(tAsciiToHtmlCodeListQR[iOffset].cAsciiLetter != 0x00)
			{
				if(cpContents[iIdx] == tAsciiToHtmlCodeListQR[iOffset].cAsciiLetter)
				{
					iFoundFlag = 1;
					iDestIdx += mtiStrcpy(cpData + iDestIdx, tAsciiToHtmlCodeListQR[iOffset].caHtmlCode, 10);
				}
				iOffset++;
			}
			if(iFoundFlag == 0)
			{
				cpData[iDestIdx] = cpContents[iIdx];
				iDestIdx++;
			}
		}

	}
	else
	{
		for(iIdx = 0; iIdx < P_MAX_COULUM; iIdx++)
		{
			iOffset = 0;
			iFoundFlag = 0;
			while(tAsciiToHtmlCodeList[iOffset].cAsciiLetter != 0x00)
			{
				if(cpContents[iIdx] == tAsciiToHtmlCodeList[iOffset].cAsciiLetter)
				{
					iFoundFlag = 1;
					iDestIdx += mtiStrcpy(cpData + iDestIdx, tAsciiToHtmlCodeList[iOffset].caHtmlCode, 10);
				}
				iOffset++;
			}
			if(iFoundFlag == 0)
			{
				cpData[iDestIdx] = cpContents[iIdx];
				iDestIdx++;
			}
		}

	}

	mtiMapPutString(tpPrtMap, tpPrtMap->iMapCount, caData);
	return RTN_SUCCESS;
}

INT mtiUpdatePrintQueue(tMtiMap *tpPrtMap, INT iIndex, CHAR cAttribute, CHAR *cpContents)
{
	CHAR caData[50];
	CHAR *cpData = caData;

	if (tpPrtMap == NULL)
	{
		dpt();
		return RTN_ERROR;
	}

	mtiMemset(caData, 0, sizeof(caData));
	*cpData++ = cAttribute;
	cpData += mtiStrcpy(cpData, cpContents, P_MAX_COULUM);

	mtiMapPutString(tpPrtMap, iIndex, caData);
	return RTN_SUCCESS;
}

#if 1
static CHAR *spaceToHtml(CHAR *cpData, INT iFontSize)
{
	INT iIdx, iOffset;
	static CHAR caLineBuff[1024];

	mtiMemset(caLineBuff, 0x00, sizeof(caLineBuff));
	iOffset = 0;
	for(iIdx = 0; iIdx < mtiStrlen(100, cpData) + 1; iIdx++)
	{
		if(cpData[iIdx] == ' ' && cpData[iIdx+1] == ' ')
		{
			INT iSpIdx = iIdx;
			INT iSpCnt = 0;

			while(cpData[iSpIdx++] == ' ')
				iSpCnt++;

			iOffset += sprintf(&caLineBuff[iOffset], "<span style=\"color: white;font-size: %dpx;\">", iFontSize);
			mtiMemset(&caLineBuff[iOffset], '*', iSpCnt);
			iOffset += iSpCnt;
			iOffset += sprintf(&caLineBuff[iOffset], "</span>");

			//iOffset += sprintf(&caLineBuff[iOffset], "&nbsp;");
			iIdx += (iSpCnt - 1);
		}
		else
			caLineBuff[iOffset++] = cpData[iIdx];
	}

	if(strcmp(caLineBuff, " ") == 0)
		iOffset += sprintf(caLineBuff, "<br>");

	//dmsg("caLineBuff[%s]", caLineBuff);
	return caLineBuff;
}

INT mtiDoPrinting(tMtiMap *tpPrtMap)
{
#if 0

/*
	INT icontrast = 0;
	CHAR caTemp[256] = {0,};
	for(icontrast = 150; icontrast < 180; icontrast+=5)
	{
	prtSetPropertyInt(PRT_PROP_CONTRAST, icontrast);


	sprintf(caTemp,"<p style=\"font-size: 14px;font-stretch: condensed;\">CONTRAST [%d]</p><br>",icontrast);
	prtHTML(caTemp);
	prtHTML("<div style=\"font-size: 14px;font-stretch: condensed;\"><b>HHHHHHHHHHHHHHHHHHHHHHH</b></div>\n");
*/

#if 1

	{
	//prtSetPropertyString(PRT_PROP_DEFAULT_FONT, "dejavu sans mono");
	prtSetPropertyString(PRT_PROP_DEFAULT_FONT, "dec terminal modern");

	prtHTML("<p style=\"font-size: 31px;\">TEST PRINT 31px</p><br>");
	prtHTML("<p style=\"font-size: 31px;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 29px;font-stretch: expanded;\">TEST PRINT 29px</p><br>");
	prtHTML("<p style=\"font-size: 29px;font-stretch: expanded;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 27px;font-stretch: expanded;\">TEST PRINT 27px</p><br>");
	prtHTML("<p style=\"font-size: 27px;font-stretch: expanded;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 25px;font-stretch: expanded;\">TEST PRINT 25</p><br>");
	prtHTML("<p style=\"font-size: 25px;font-stretch: expanded;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 23px;font-stretch: expanded;\">TEST PRINT 23px</p><br>");
	prtHTML("<p style=\"font-size: 23px;font-stretch: expanded;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 21px;font-stretch: expanded;\">TEST PRINT 21px</p><br>");
	prtHTML("<p style=\"font-size: 21px;font-stretch: expanded;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 19px;font-stretch: expanded;\">TEST PRINT 19px</p><br>");
	prtHTML("<p style=\"font-size: 19px;font-stretch: expanded;\">012345678901234567890123456789012345678901234567</p><br>");

	prtHTML("<br><br><br>");
#endif
	}
	return RTN_SUCCESS;

#else

	INT i = 0, iKey = 0, iOffset, iPageSize;
	CHAR *cpData = NULL;
	CHAR cAttr = 0;
	CHAR *cpPrintBuff = NULL;
	short sRet = 0;
	INT prtVal = 0;
	PrtError prtErrRet = PRT_OK;
	INT iRet = RTN_SUCCESS;
	CHAR caTemp[2048];

	//prtSetPropertyString(PRT_PROP_DEFAULT_FONT, "dejavu sans mono");
	prtSetPropertyString(PRT_PROP_DEFAULT_FONT, "dec terminal modern");

	if (tpPrtMap == NULL)
	{
		return RTN_ERROR;
	}

	iPageSize = 300 * tpPrtMap->iMapCount;
	cpPrintBuff = (CHAR *) mtiMalloc(iPageSize);
	dmsg("cpPrintBuff[%p], iPageSize[%d]",cpPrintBuff, iPageSize);

	mtiMemset(cpPrintBuff, 0x00, iPageSize);
	iOffset = 0;

	//LOGO MANDIRI
	//iOffset += sprintf(cpPrintBuff + iOffset,"<img src='F:15/PRTLGO_320.png' align='middle'>");

	for (i = 0; i < tpPrtMap->iMapCount; i++)
	{
		iKey = MemCheck(tpPrtMap, i);
		cpData = mtiMapGetString(tpPrtMap, iKey);

		if (cpData != NULL)
		{
			cAttr = *cpData++;
			if (CHK_BIT(cAttr, P_MODE_BOLD))
			{
//				iOffset += sprintf(cpPrintBuff + iOffset, "<div style=\"font-size: 46px;font-stretch: condensed;\"><b>%s</b></div>\n",spaceToHtml(cpData, 46));
				iOffset += sprintf(cpPrintBuff + iOffset, "<div style=\"font-size: 39px;\"><b>%s</b></div>\n",spaceToHtml(cpData, 39));
			}
			else if (CHK_BIT(cAttr, P_MODE_DOUBLE_BOLD))
			{
				iOffset += sprintf(cpPrintBuff + iOffset, "<div style=\"font-size: 60px;font-stretch: expanded;\">%s</div>\n",spaceToHtml(cpData, 60));
			}
			/***
			else if (CHK_BIT(cAttr, P_MODE_DEF_MANDIRI))
			{
				iOffset += sprintf(cpPrintBuff + iOffset,"<center><img src='F:15/MTI_DATA/RPTLOGO.png' align='middle'"
																"></center>");

				dmsg("sRet [%hd]", sRet);
			}
			***/
			else if (CHK_BIT(cAttr, P_MODE_QRCODE))
			{
				//<?barcode ean-13 4104640025303 384 60?>
				//iOffset += sprintf(cpPrintBuff + iOffset,"<center><?barcodevar qr-m %s 384 384?><br></center>", cpData);
				dmsg("cpData ====== [%s]", cpData);
				sprintf(caTemp, "%s", cpData);
				dpt();
				dmsg("QRCODE [%s][%d]", caTemp, strlen(caTemp));
				iOffset += sprintf(cpPrintBuff + iOffset,"<center><?barcode qr-h %s 384 384?></center>", caTemp);

				//dmsg("sRet [%hd]", sRet);
			}
			else if (CHK_BIT(cAttr, P_MODE_IMAGE))
			{
				dmsg("P_MODE_IMAGE File[%s]", cpData);
					iOffset += sprintf(cpPrintBuff + iOffset,"<center><img src='F:15/MTI_DATA/%s' align='middle'"
												"></center>", cpData);
			}
			else
			{
//				iOffset += sprintf(cpPrintBuff + iOffset, "<div style=\"font-size: 14px;font-stretch: condensed;\"><b>%s</b></div>\n",spaceToHtml(cpData, 14));
				iOffset += sprintf(cpPrintBuff + iOffset,
						"<div style=\"font-size: 20px;\"><b>%s</b></div>\n",spaceToHtml(cpData, 20));

			}
		}
	}
	//dmsg("Print Page[%s]",cpPrintBuff);
	iOffset += sprintf(cpPrintBuff + iOffset,"<br style=\"page-break-after:always\">");
	prtSetPropertyInt(PRT_PROP_CONTRAST, 150);
	prtGetPropertyInt(PRT_PROP_CONTRAST, &prtVal);
	dmsg("Current PRT CONTRAST - [%d]", prtVal);

	dmsg("**cpPrintBuff[%s]", cpPrintBuff);
	prtErrRet = prtHTML(cpPrintBuff, 0);
	dmsg("Print Result = [%d]",prtErrRet);
	mtiFree(cpPrintBuff);

	//PRINT RESULT PROCESS
	switch(prtErrRet)
		{
			case PRT_OK:
				iRet = 0;
				dmsg("PRINT STATUS OK");
				break;
			case PRT_PAPEREND:
				iRet = P_STATUS_NO_PAPER;
				dmsg("PRT ERR: NO PAPER");
				break;
			case PRT_UNDERVOLTAGE:
				iRet = P_STATUS_LOW_BATTERY;
				dmsg("PRT ERR: LOW BATTERY");
				break;
			default:
				iRet = P_STATUS_PRINT_ERROR;
				dmsg("PRT ERR: UNKNOWN ERROR");
				break;
		}

	return iRet;
#endif
}
#else
static CHAR *spaceToHtml(CHAR *cpData, INT iFontSize)
{
	INT iIdx, iOffset;
	static CHAR caLineBuff[1024];

	mtiMemset(caLineBuff, 0x00, sizeof(caLineBuff));
	iOffset = 0;
	for(iIdx = 0; iIdx < mtiStrlen(P_MAX_COULUM, cpData) + 1; iIdx++)
	{
		if(cpData[iIdx] == ' ' && cpData[iIdx+1] == ' ')
		{
			INT iSpIdx = iIdx;
			INT iSpCnt = 0;

			while(cpData[iSpIdx++] == ' ')
				iSpCnt++;
			iSpCnt--;
			iOffset += sprintf(&caLineBuff[iOffset], "<span style=\"color: white;\">");
			mtiMemset(&caLineBuff[iOffset], '*', iSpCnt);
			iOffset += iSpCnt;
			iOffset += sprintf(&caLineBuff[iOffset], "</span>");

			//iOffset += sprintf(&caLineBuff[iOffset], "&nbsp;");
			iIdx += iSpCnt;
		}
		else
			caLineBuff[iOffset++] = cpData[iIdx];
	}

	if(strcmp(caLineBuff, " ") == 0)
		iOffset += sprintf(caLineBuff, "<br>");

	//dmsg("caLineBuff[%s]", caLineBuff);
	return caLineBuff;
}

INT mtiDoPrinting(tMtiMap *tpPrtMap)
{
/*
	prtHTML("<p style=\"font-size: 14px;font-stretch: condensed;\">TEST PRINT 14px</p><br>");
	prtHTML("<p style=\"font-size: 14px;font-stretch: condensed;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 15px;font-stretch: condensed;\">TEST PRINT 15px</p><br>");
	prtHTML("<p style=\"font-size: 15px;font-stretch: condensed;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 18px;font-stretch: condensed;\">TEST PRINT 18px</p><br>");
	prtHTML("<p style=\"font-size: 18px;font-stretch: condensed;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 20px;\">TEST PRINT 20px</p><br>");
	prtHTML("<p style=\"font-size: 20px;\">012345678901234567890123456789012345678901234567</p><br>");
	prtHTML("<p style=\"font-size: 25px;\">TEST PRINT 25px</p><br>");
	prtHTML("<p style=\"font-size: 25px;\">012345678901234567890123456789012345678901234567</p><br>");

	prtHTML("<br><br><br>");
	return RTN_SUCCESS;
*/

	INT i = 0, iKey = 0, iOffset, iPageSize;
	CHAR *cpData = NULL;
	CHAR cAttr = 0;
	CHAR *cpPrintBuff = NULL;
	CHAR buf[500] = {0,};
	INT iSize = 0;
	CHAR *CTS="<b>PRINT_TEST1</b><br><strong>PRINT_TEST2</strong><br>HELLO?ANYBODY THERE?<br>";
 	short sRet = 0;

	prtSetPropertyInt(PRT_PROP_CONTRAST, 100);
	//prtSetPropertyString(PRT_PROP_DEFAULT_FONT, "neo");
	//prtSetPropertyString(PRT_PROP_RESOURCE_PATH, "F:15/FONTS");

	prtGetPropertyString(PRT_PROP_DEFAULT_FONT, buf, sizeof(buf));
	dmsg("PRT_PROP_DEFAULT_FONT [%s]", buf);

	prtGetPropertyInt(PRT_PROP_DEFAULT_FONT_SIZE, &iSize);
	dmsg("PRT_PROP_DEFAULT_FONT_SIZE [%d]",iSize);


	memset(buf, 0x00, sizeof(buf));
	sprintf(buf,"%s",CTS );
	dmsg("PRTMSG [%s]",buf);
	prtHTML(buf, 0);
	for(i = 0 ; i<60 ; i++)
	{

		memset(buf, 0x00, sizeof(buf));
		sprintf(buf, "<div style=\"font-size: %dpx;\">%d-%s</div>", i, i, CTS);
		dmsg("PRTMSG [%s]",buf);
		prtHTML(buf, 0);

	}


	return RTN_SUCCESS;
}
#endif

VOID mtiFinalizePrinter()
{
	return;
}
