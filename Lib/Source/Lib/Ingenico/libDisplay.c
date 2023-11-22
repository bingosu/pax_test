
#include "libMtiCommonApi.h"

#ifdef __TETRA__
#include "GL_Types.h"
#include "GL_Window.h"
#include "GL_Message.h"
#include "GL_Widget.h"
#include "GL_Barcode.h"
#endif

#define SEL_FONT					_POLICE7x8_
#define SEL_SMALL_FONT				_POLICE5x7_

/*********************************************************************************************************
 * DISPLAY DEFUALT SETTING
*********************************************************************************************************/
#define PAGE_MAX					5
#define T_SIZE						5

#define LEFT_UP_ARROW				4
#define TOP_UP_ARROW				57

#define LEFT_DOWN_ARROW				111
#define TOP_DOWN_ARROW				57

#define LCD_PIXEL_PER_LINE			11

#define SEC_INPUT_MINIMUM_LEN		4

static Telium_File_t *mtDisplay = NULL;

#define INIT_DISP_KEY()									\
	mtDisplay = Telium_Fopen("DISPLAY", "w*");			\
	mtiInitializeKeyboard();							\
	mtiClearDisp();

#define FINAL_DISP_KEY()								\
	if (mtDisplay != NULL) Telium_Fclose(mtDisplay);	\
	mtiFinalizeKeyboard();

#define INIT_DISP()										\
	mtDisplay = Telium_Fopen("DISPLAY", "w*");			\
	mtiClearDisp();

#define FINAL_DISP()									\
	if (mtDisplay != NULL) Telium_Fclose(mtDisplay);

#ifndef __TETRA__
T_GL_HGRAPHIC_LIB g_hGoal = NULL;
#endif

BOOL mtiCheckQRDispCap()
{
	UCHAR ucRange = 0;
	UCHAR ucProduct;

	ucRange = PSQ_Get_product_type(&ucProduct);

	switch(ucRange)
	{
		case TYPE_TERMINAL_IWL220:
		case TYPE_TERMINAL_ICT220:
			return FALSE;
			break;

		case TYPE_TERMINAL_ICT250:
			return TRUE;
			break;
		case TYPE_TERMINAL_M2500:
			return TRUE;
			break;
		case TYPE_TERMINAL_M3500:
			return TRUE;
			break;

		default:
			return FALSE;
	}

	return FALSE;
}

VOID mtiProcAddCtrl(tAddCtrlContents *tpAddCtrl)
{
	CHAR caBottom[25 + 1];

	MZERO(caBottom, sizeof(caBottom));
	MSPACE(caBottom, sizeof(caBottom) - 1);
	if (tpAddCtrl != NULL)
	{
		mtiStrcpy(caBottom, tpAddCtrl->tBtnLeft.caContents, 10);
		mtiStringRightAlign(tpAddCtrl->tBtnRight.caContents, &caBottom[10], 11);
		_DrawExtendedString(0, 5 * LCD_PIXEL_PER_LINE, caBottom, _OFF_, _POLICE5x7_, _NORMALE_);

		PaintGraphics();
	}
}

VOID mtiClearDisp()
{
	_clrscr();
}

INT mtiShowDialogDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl)
{
	INT i = 0, iLen = 0, iRet = 0, iTid = 0, iKey = 0, iMaxLen = 0, iSelFont = 0;
	INT nDiableTimout = 0;
	CHAR *cpCaption = NULL;
	CHAR caLine[DISP_INPUT_MAX_COULUM + 1];

	iRet = RTN_SUCCESS;

	if (tpDispCont != NULL)
	{
		if (tpAddCtrl != NULL)
		{
			INIT_DISP_KEY();
		}
		else
		{
			INIT_DISP();
		}

		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i].caContents;
			iLen = SLEN(20, cpCaption);
			if (iLen > 0)
			{
				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SMALL_FONT))
				{
					iMaxLen = DISP_SMALL_MAX_COULUM;
				}
				else
				{
					iMaxLen = DISP_MAX_COULUM;
				}

				MZERO(caLine, sizeof(caLine));
				if (tpDispCont[i].iAlignment == DISP_ALIGN_CENTER)
				{
					mtiStringCenterAlign(cpCaption, caLine, iMaxLen);
				}
				else if (tpDispCont[i].iAlignment == DISP_ALIGN_RIGHT)
				{
					mtiStringRightAlign(cpCaption, caLine, iMaxLen);
				}
				else
				{
					MSPACE(caLine, iMaxLen);
					MCPY(caLine, cpCaption, iLen);
				}

				if (iMaxLen == DISP_SMALL_MAX_COULUM)
				{
					iSelFont = SEL_SMALL_FONT;
				}
				else
				{
					iSelFont = SEL_FONT;
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_TITLE))
				{
					_DrawExtendedString(0, 0 * LCD_PIXEL_PER_LINE, caLine, _ON_, iSelFont, _NORMALE_);
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					_DrawExtendedString(0, 1 * LCD_PIXEL_PER_LINE, caLine, _OFF_, iSelFont, _NORMALE_);
				}
				else
				{
					_DrawExtendedString(0, i * LCD_PIXEL_PER_LINE, caLine, _OFF_, iSelFont, _NORMALE_);
				}

				// Disable timeout
				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NOTUSED_TIMEOUT))
				{
					nDiableTimout = 1;
				}
			}
		}
		PaintGraphics();

		if (tpAddCtrl != NULL)
		{
			mtiProcAddCtrl(tpAddCtrl);
			iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
			while (TRUE)
			{
				if (nDiableTimout)
				{

				}
				else
				{
					if (mtiIsTimeout(iTid))
					{
						iRet = RTN_TIMEOUT;
						break;
					}
				}

				iKey = mtiGetKeyPress();

				if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);

					return RTN_SELECT_LEFT;
				}
				else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);

					return RTN_SELECT_RIGHT;
				}
				else if (iKey == tpAddCtrl->ucCancelKey)
				{
					if (tpAddCtrl->iCancelKeyResultValue)
					{
						iRet = tpAddCtrl->iCancelKeyResultValue;
					}
					else
					{
						iRet = RTN_CANCEL;
					}
					break;
				}
				else if (iKey == tpAddCtrl->ucEnterKey)
				{
					if (tpAddCtrl->iEnterKeyResultValue)
					{
						iRet = tpAddCtrl->iEnterKeyResultValue;
					}
					else
					{
						iRet = RTN_SUCCESS;
					}
					break;
				}
			}
			mtiStopTimer(iTid);
		}
	}

	if (tpAddCtrl != NULL)
	{
		FINAL_DISP_KEY();
	}
	else
	{
		FINAL_DISP();
	}

	return iRet;
}

// NOT USE DISP_OPT_AFTER_CHECK
INT mtiShowSecureInput(tDispContents *tpDispCont, INT iDispContCnt, INT iSecLength, tAddCtrlContents *tpAddCtrl)
{
	INT iRet = 0, i = 0, iLen = 0, iCnt = 0, iIndex = 0, iKey = 0, iLeft = 0, iTid = 0;
	CHAR *cpCaption = NULL;
	CHAR caLine[DISP_MAX_COULUM + 1];

#if 1	//@@WAY, 20190528 DISPLAY
	CHAR caInput[16 + 1];
#else
	CHAR caInput[20 + 1];
#endif

	if (tpDispCont != NULL)
	{
		INIT_DISP_KEY();

		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i].caContents;
#if 1		// @@WAY, 20190528 DISPLAY
			iLen = SLEN(16, cpCaption);
#else
			iLen = SLEN(20, cpCaption);
#endif
			if (iLen > 0)
			{
				MZERO(caLine, sizeof(caLine));
				if (tpDispCont[i].iAlignment == DISP_ALIGN_CENTER)
				{
					mtiStringCenterAlign(cpCaption, caLine, DISP_MAX_COULUM);
				}
				else if (tpDispCont[i].iAlignment == DISP_ALIGN_RIGHT)
				{
					mtiStringRightAlign(cpCaption, caLine, DISP_MAX_COULUM);
				}
				else
				{
					MSPACE(caLine, DISP_MAX_COULUM);
					MCPY(caLine, cpCaption, iLen);
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_TITLE))
				{
					_DrawExtendedString(0, 0 * LCD_PIXEL_PER_LINE, caLine, _ON_, SEL_FONT, _NORMALE_);
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					_DrawExtendedString(0, 1 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_INPUT))
				{
					_DrawExtendedString(0, i * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
					iIndex = i + 1;
				}
				else
				{
					_DrawExtendedString(0, i * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
				}
			}
		}

		MZERO(caInput, sizeof(caInput));
		iLeft = 8 - (iSecLength / 2);
		iLen = 0;

		if (tpAddCtrl != NULL)
		{
			mtiProcAddCtrl(tpAddCtrl);
			iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
		}
		else
		{
			iTid = mtiStartTimer(TIMEOUT_30S);
		}

		if (iIndex > 0)
		{
			while(_ON_)
			{
				PaintGraphics();

				if (mtiIsTimeout(iTid))
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_TIMEOUT;
				}

				iKey = mtiGetKeyPress();
				if (iKey != KEY_NONE)
				{
					dpt()
					dmsg("mtiGetKeyPress [%d], %02x", iKey, iKey);
					mtiResetTimer(iTid);
				}

				if (tpAddCtrl != NULL)
				{
					if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
					{
						MCPY(tpDispCont[iIndex - 1].caInout, caInput, iLen);

						FINAL_DISP_KEY();
						mtiStopTimer(iTid);

						return RTN_SELECT_LEFT;
					}
					else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
					{
						// YES
						if (iLen < 1)
						{
							// Check PIN Bypass or NO INPUT
							if (CHK_BIT(tpDispCont[iIndex - 1].iAttribute, DISP_OPT_CHECK_NO_INPUT))
							{
								continue;
							}
						}
						else if (iLen < SEC_INPUT_MINIMUM_LEN)
						{
							// Check Length
							if (CHK_BIT(tpDispCont[iIndex - 1].iAttribute, DISP_OPT_CHECK_MINIMUM_LENGTH))
							{
								mtiSoundBeep(50, 2);
								continue;
							}
						}
					
						MCPY(tpDispCont[iIndex - 1].caInout, caInput, iLen);

						FINAL_DISP_KEY();
						mtiStopTimer(iTid);

						return RTN_SELECT_RIGHT;
					}
					else if (iKey == tpAddCtrl->ucCancelKey)
					{
						if (tpAddCtrl->iCancelKeyResultValue)
						{
							iRet = tpAddCtrl->iCancelKeyResultValue;
						}
						else
						{
							iRet = RTN_CANCEL;
						}
						break;
					}
					else if (iKey == tpAddCtrl->ucEnterKey)
					{
						if (tpAddCtrl->iEnterKeyResultValue)
						{
							iRet = tpAddCtrl->iEnterKeyResultValue;
						}
						else
						{
							iRet = RTN_SUCCESS;
						}
						break;
					}
				}

				switch (iKey)
				{
					case KEY_ENTER:
						if (iLen < 1)
						{
							// Check PIN Bypass or NO INPUT
							if (CHK_BIT(tpDispCont[iIndex - 1].iAttribute, DISP_OPT_CHECK_NO_INPUT))
							{
								continue;
							}
						}
						else if (iLen < SEC_INPUT_MINIMUM_LEN)
						{
							// Check Length
							if (CHK_BIT(tpDispCont[iIndex - 1].iAttribute, DISP_OPT_CHECK_MINIMUM_LENGTH))
							{
								continue;
							}
						}

						MCPY(tpDispCont[iIndex - 1].caInout, caInput, iLen);
						FINAL_DISP_KEY();
						mtiStopTimer(iTid);
						return RTN_SUCCESS;

					case KEY_CANCEL:
						FINAL_DISP_KEY();
						mtiStopTimer(iTid);
						return RTN_CANCEL;

					case KEY_0:
					case KEY_1:
					case KEY_2:
					case KEY_3:
					case KEY_4:
					case KEY_5:
					case KEY_6:
					case KEY_7:
					case KEY_8:
					case KEY_9:
						if (iLen < iSecLength)
						{
							caInput[iLen++] = (CHAR)iKey;

							if (CHK_BIT(tpDispCont[iIndex - 1].iAttribute, DISP_OPT_FORWARD_SECURE))
							{
								MZERO(caLine, sizeof(caLine));

								// @@EB DISPLAY 20190628
#if 1
								MSPACE(caLine, sizeof(caLine) - 1);
								for (iCnt = iLeft; iCnt < (iLeft + SLEN(16, caInput) - 1); iCnt++)
#else
								MSPACE(caLine, sizeof(caLine));
								for (iCnt = iLeft; iCnt < (iLeft + SLEN(20, caInput) - 1); iCnt++)
#endif
								{
									caLine[iCnt] = '*';
								}

								caLine[iCnt] = (CHAR)iKey;
							}
							else
							{
								MZERO(caLine, sizeof(caLine));
								// @@EB DISPLAY 20190628
#if 1
								MSPACE(caLine, sizeof(caLine) - 1);
								for (iCnt = iLeft; iCnt < (iLeft + SLEN(16, caInput)); iCnt++)
#else
								MSPACE(caLine, sizeof(caLine));
								for (iCnt = iLeft; iCnt < (iLeft + SLEN(20, caInput)); iCnt++)
#endif
								{
									caLine[iCnt] = '*';
								}
							}

							_DrawExtendedString(0, iIndex * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
						}
						break;

					case KEY_BACK:
						if (0 < iLen)
						{
							caInput[--iLen] = 0x00;

							MZERO(caLine, sizeof(caLine));
							// @@EB DISPLAY 20190628
#if 1
							MSPACE(caLine, sizeof(caLine) - 1);
							for (iCnt = iLeft; iCnt < (iLeft + SLEN(20, caInput)); iCnt++)
#else
							MSPACE(caLine, sizeof(caLine));
							for (iCnt = iLeft; iCnt < (iLeft + SLEN(20, caInput)); iCnt++)
#endif
							{
								caLine[iCnt] = '*';
							}

							_DrawExtendedString(0, iIndex  * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
						}
						break;
				}
			}
		}
	}

	FINAL_DISP_KEY();
	mtiStopTimer(iTid);
	return iRet;
}



typedef struct _tFmtEle {
	CHAR cpVal;
	INT iRelPos;
	INT iIsInput;
} tFmtEle;

enum eInType {
	IN_TYPE_NUM = 100,
	IN_TYPE_CURRENCY,
	IN_TYPE_ANS
};

typedef struct _tAlphaKeyMap
{
	INT iKeyVal;
	CHAR *caAlphaVal;
} tAlphaKeyMap;

tAlphaKeyMap tAlphaKeyMapList[10] ={
		{KEY_1, "QZ.qz."},
		{KEY_2, "ABCabc"},
		{KEY_3, "DEFdef"},
		{KEY_4, "GHIghi"},
		{KEY_5, "JKLjkl"},
		{KEY_6, "MNOmno"},
		{KEY_7, "PRSprs"},
		{KEY_8, "TUVtuv"},
		{KEY_9, "WXYwxy"},
		{KEY_0, "- "},
};

INT mtiShowInputDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl)
{
	INT iRet = 0, i = 0, iLen = 0, iCnt = 0, iIndex = 0, iKey = 0, iRow = 0, iTid = 0, iAfterShow = 0;
	INT iSelIndex = 0, ii = 0, iInPos = 0, iMaxInput = 0, iLastIndex = 0;
	INT iIsUpdate = FALSE, iIsShown = FALSE;
	INT iLineIndex[2] = { 0 }, iLineOrg[2] = { 0 };
	INT iSkipFlag = FALSE, iDefaultFlag = FALSE, iFreeInput = FALSE, iCurrencyFlag = FALSE;

	INT iAddLine = 0;

	INT iAlphaInput = FALSE;
	INT iAlphaIdx = 0;
	INT iAlphaUpdate = FALSE;
	INT iInputTimer = 0;
	INT nDiableTimout = 0;

	CHAR *cpCaption = NULL;
	CHAR caLine[DISP_MAX_COULUM + 1];
	CHAR caInput[DISP_INPUT_MAX_COULUM + 1];
	CHAR caOutput[DISP_INPUT_MAX_COULUM + 1];
	CHAR cFmt = 0;
	tFmtEle taFrmEle[DISP_INPUT_MAX_COULUM + 1];
	tFmtEle taCurEle[DISP_INPUT_MAX_COULUM + 1];
	tFmtEle taNumEle[DISP_INPUT_MAX_COULUM + 1];
	tFmtEle taAnsEle[DISP_INPUT_MAX_COULUM + 1];
	INT iNumCnt = 0, iCurCnt = 0, iANSCnt = 0;

	if (tpDispCont != NULL)
	{
		INIT_DISP_KEY();

		//INPUT 입력이 2개 이상인지 확인한다.
		for (i = 0; i < iDispContCnt; i++)
		{		
			cpCaption = tpDispCont[i].caContents;
			iLen = SLEN(DISP_INPUT_MAX_COULUM, cpCaption);
			if (iLen > 0)
			{
				MZERO(caLine, sizeof(caLine));
				if (tpDispCont[i].iAlignment == DISP_ALIGN_CENTER)
				{
					mtiStringCenterAlign(cpCaption, caLine, DISP_MAX_COULUM);
				}
				else if (tpDispCont[i].iAlignment == DISP_ALIGN_RIGHT)
				{
					mtiStringRightAlign(cpCaption, caLine, DISP_MAX_COULUM);
				}
				else
				{
					MSPACE(caLine, DISP_MAX_COULUM);
					MCPY(caLine, cpCaption, iLen);
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_TITLE))
				{				
					_DrawExtendedString(0, 0 * LCD_PIXEL_PER_LINE, caLine, _ON_, SEL_FONT, _NORMALE_);
					iRow = 1;
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					_DrawExtendedString(0, 1 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
					iRow = 2;
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_INPUT))
				{
					iLineIndex[0] = i + 1;
					iLineOrg[0] = i;

					if (tpDispCont[i].caDefValue[0] > 0)
					{
						if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SMALL_FONT))
						{
							_DrawExtendedString(0, (i + iAddLine + 1) * LCD_PIXEL_PER_LINE, tpDispCont[i].caDefValue, _OFF_, SEL_SMALL_FONT, _NORMALE_);
						}
						else
						{
							_DrawExtendedString(0, (i + iAddLine + 1) * LCD_PIXEL_PER_LINE, tpDispCont[i].caDefValue, _OFF_, SEL_FONT, _NORMALE_);
						}
					}
					iAddLine++;
					
					iCnt++;
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NORMAL))
				{
					if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SMALL_FONT))
					{
						_DrawExtendedString(0, (i+iAddLine) * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_SMALL_FONT, _NORMALE_);
					}
					else
					{
						_DrawExtendedString(0, (i+iAddLine) * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
					}

					// Check Max  
					if (2 > i)
					{
						iRow = i + 1;
					}
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_AFTER_CHECK))
				{
					iAfterShow = 1;
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NOTUSED_TIMEOUT))
				{
					nDiableTimout = 1;
				}
				
			}
			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_ALPHA))
			{
				iAlphaInput = TRUE;
			}
		}

		if (iCnt > 1)
		{
			for (i = 0; i < iDispContCnt; i++)
			{
				cpCaption = tpDispCont[i].caContents;
				iLen = SLEN(20, cpCaption);
				if (iLen > 0)
				{
					MZERO(caLine, sizeof(caLine));
					if (tpDispCont[i].iAlignment == DISP_ALIGN_CENTER)
					{
						mtiStringCenterAlign(cpCaption, caLine, DISP_MAX_COULUM);
					}
					else if (tpDispCont[i].iAlignment == DISP_ALIGN_RIGHT)
					{
						mtiStringRightAlign(cpCaption, caLine, DISP_MAX_COULUM);
					}
					else
					{
						MSPACE(caLine, DISP_MAX_COULUM);
						MCPY(caLine, cpCaption, iLen);
					}

					if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_INPUT))
					{
						if (iRow == 1)
						{
							if (i == 1)
							{
								if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SMALL_FONT))
								{
									_DrawExtendedString(0, 1 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_SMALL_FONT, _NORMALE_);
								}
								else
								{
									_DrawExtendedString(0, 1 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
								}

								iLineOrg[0] = i;
								iLineIndex[0] = 2;
							}
							else if (i == 2)
							{
								if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SMALL_FONT))
								{
									_DrawExtendedString(0, 3 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_SMALL_FONT, _NORMALE_);
								}
								else
								{
									_DrawExtendedString(0, 3 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
								}

								iLineOrg[1] = i;
								iLineIndex[1] = 4;
							}
						}
						else if (iRow == 2)
						{
							if (i == 2)
							{
								if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SMALL_FONT))
								{
									_DrawExtendedString(0, 2 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_SMALL_FONT, _NORMALE_);
								}
								else
								{
									_DrawExtendedString(0, 2 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
								}

								iLineOrg[0] = i;
								iLineIndex[0] = 3;
							}
							else if (i == 3)
							{
								if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SMALL_FONT))
								{
									_DrawExtendedString(0, 4 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_SMALL_FONT, _NORMALE_);
								}
								else
								{
									_DrawExtendedString(0, 4 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
								}

								iLineOrg[1] = i;
								iLineIndex[1] = 5;
							}
						}
					}
				}
			}
		}
		else
		{
			_DrawExtendedString(0, iLineOrg[0] * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
		}

		MZERO(caInput, sizeof(caInput));
		iIndex = 0;
		iLen = 0;

		if (tpAddCtrl != NULL)
		{
			if (!iAfterShow)
			{
				iIsShown = TRUE;
				mtiProcAddCtrl(tpAddCtrl);
			}
			else
			{
				iIsShown = FALSE;
			}

			iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
		}
		else
		{
			iTid = mtiStartTimer(TIMEOUT_30S);
		}

		iSelIndex = -1;
		iIsUpdate = TRUE;
		while(_ON_)
		{
			PaintGraphics();

			if (nDiableTimout)
			{

			}
			else
			{
				if (mtiIsTimeout(iTid))
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_TIMEOUT;
				}
			}

			iKey = mtiGetKeyPress();
			if (iKey != KEY_NONE)
			{
				mtiResetTimer(iTid);
			}

			if (iSelIndex != iIndex)
			{
				mtiMemset(taFrmEle, 0, sizeof(taFrmEle));
				mtiMemset(taCurEle, 0, sizeof(taCurEle));
				mtiMemset(taNumEle, 0, sizeof(taNumEle));
				mtiMemset(taAnsEle, 0, sizeof(taAnsEle));
				iInPos = 0;
				iCurCnt = 0;
				iNumCnt = 0;
				iANSCnt = 0;
				iMaxInput = 0;

				if (tpDispCont[iLineOrg[iIndex]].caFormat[0] != 0x00)
				{
					for (ii = 0; ii < DISP_INPUT_MAX_COULUM; ii++)
					{
						cFmt = tpDispCont[iLineOrg[iIndex]].caFormat[ii];
						if (cFmt == '@')
						{
							taFrmEle[ii].cpVal = ' ';
							taFrmEle[ii].iIsInput = IN_TYPE_CURRENCY;
							taFrmEle[ii].iRelPos = ii;

							taCurEle[iCurCnt].iIsInput = IN_TYPE_CURRENCY;
							taCurEle[iCurCnt].iRelPos = ii;
							iCurCnt++;

							iCurrencyFlag = TRUE;
							iMaxInput++;
						}
						else if (cFmt == '#')
						{
							taFrmEle[ii].cpVal = ' ';
							taFrmEle[ii].iIsInput = IN_TYPE_NUM;
							taFrmEle[ii].iRelPos = ii;

							taNumEle[iNumCnt].iIsInput = IN_TYPE_NUM;
							taNumEle[iNumCnt].iRelPos = ii;
							iNumCnt++;

							iMaxInput++;
						}
						else if (cFmt == '*')
						{
							taFrmEle[ii].cpVal = '_';
							taFrmEle[ii].iIsInput = IN_TYPE_ANS;
							taFrmEle[ii].iRelPos = ii;

							taAnsEle[iANSCnt].iIsInput = IN_TYPE_ANS;
							taAnsEle[iANSCnt].iRelPos = ii;
							iANSCnt++;

							iMaxInput++;
						}
						else if (cFmt == ',' || cFmt == '-' || cFmt == '.' || cFmt == '/')
						{
							taFrmEle[ii].cpVal = cFmt;
							taFrmEle[ii].iRelPos = -1;
						}
						else
						{
							taFrmEle[ii].cpVal = cFmt;
							taFrmEle[ii].iRelPos = -2;
						}
					}
				}
				else
				{
					iFreeInput = TRUE;
					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_SMALL_FONT))
					{
						iMaxInput = 20;
					}
					else
					{
						iMaxInput = 16;
					}
				}

				iSelIndex = iIndex;

				if (tpDispCont[iLineOrg[iIndex]].caDefValue[0] > 0)
				{
					iLen = mtiStrcpy(caInput, tpDispCont[iLineOrg[iIndex]].caDefValue, DISP_INPUT_MAX_COULUM);
					iDefaultFlag = TRUE;
				}
				else
				{
					iDefaultFlag = FALSE;
				}

				iIsUpdate = TRUE;
				dmsg("iMaxInput = %d", iMaxInput);
			}

			if (tpAddCtrl != NULL && iIsShown)
			{
				if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
				{
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_SELECT_LEFT;
				}
				else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
				{
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_SELECT_RIGHT;
				}
				else if (iKey == tpAddCtrl->ucCancelKey)
				{
					if (tpAddCtrl->iCancelKeyResultValue)
					{
						iRet = tpAddCtrl->iCancelKeyResultValue;
					}
					else
					{
						iRet = RTN_CANCEL;
					}
					break;
				}
				else if (iKey == tpAddCtrl->ucEnterKey)
				{
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					if (tpAddCtrl->iEnterKeyResultValue)
					{
						iRet = tpAddCtrl->iEnterKeyResultValue;
					}
					else
					{
						iRet = RTN_SUCCESS;
					}
					break;
				}
			}

			if (iIsShown)
			{
				continue;
			}

			switch (iKey)
			{
				case KEY_ENTER:
					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_CORRECT_MAX_LENGTH))
					{
						if (iLen != iMaxInput)
						{
							continue;
						}
					}

					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_CHECK_NO_INPUT))
					{
						dmsg("************* DISP_OPT_CHECK_NO_INPUT : [%d]", iLen);
					
						if (0 < iLen)
						{							
						}
						else 
						{
							// NO INPUT
							break;
						}
					}
					
					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_ENABLE_SKIP))
					{
						iSkipFlag = TRUE;
					}
					else
					{
						iSkipFlag = FALSE;
					}

					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_AFTER_CHECK) && iAfterShow)
					{
						if (!iIsShown)
						{
							mtiProcAddCtrl(tpAddCtrl);

							if (iTid > 0)
							{
								mtiStopTimer(iTid);
							}

							iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
							iIsShown = TRUE;
							break;
						}
					}

					dmsg("*** iLen : [%d]", iLen);

					if (0 < iLen)
					{
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

						if (iCnt == (iIndex + 1))
						{
							FINAL_DISP_KEY();
							mtiStopTimer(iTid);
							return RTN_SUCCESS;
						}
						else
						{
							MZERO(caInput, sizeof(caInput));
							iLen = 0;
							iIndex++;
						}
					}
					else
					{
						if (iSkipFlag)
						{
							FINAL_DISP_KEY();
							mtiStopTimer(iTid);
							/** return RTN_CANCEL; **/
							return RTN_SKIP;
						}

						if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_NO_CHECK_NO_INPUT))
						{
							MZERO(caInput, sizeof(caInput));
							iLen = 0;
							iIndex++;
							break;
						}

						if (tpDispCont[iLineOrg[iIndex]].caDefValue[0] > 0)
						{
							SCPY(tpDispCont[iLineOrg[iIndex]].caInout, tpDispCont[iLineOrg[iIndex]].caDefValue, DISP_INPUT_MAX_COULUM);

							if (iCnt == (iIndex + 1))
							{
								FINAL_DISP_KEY();
								mtiStopTimer(iTid);
								return RTN_SUCCESS;
							}
							else
							{
								MZERO(caInput, sizeof(caInput));
								iLen = 0;
								iIndex++;
							}
						}
					}
					break;

				case KEY_CANCEL:
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_CANCEL;

				case KEY_0:
				case KEY_1:
				case KEY_2:
				case KEY_3:
				case KEY_4:
				case KEY_5:
				case KEY_6:
				case KEY_7:
				case KEY_8:
				case KEY_9:
					if (iDefaultFlag)
					{
						mtiMemset(caInput, 0, sizeof(caInput));
						iLen = 0;
						iDefaultFlag = FALSE;
					}

					if (iLen < iMaxInput)
					{
						if (iLen == 0 && iKey == KEY_0)
						{
							if (iCurrencyFlag != TRUE)
							{
								caInput[iLen++] = (CHAR)iKey;
								iIsUpdate = TRUE;
							}
						}
						else
						{
							caInput[iLen++] = (CHAR)iKey;
							iIsUpdate = TRUE;
						}
					}

					if(iAlphaInput == TRUE)
					{
						iInputTimer = mtiStartTimer(TIMEOUT_1S);
						iAlphaUpdate = TRUE;
					}
					break;

				case KEY_DOT:
					if (iANSCnt > 0 || iFreeInput == TRUE)
					{
						caInput[iLen++] = '.';
						iIsUpdate = TRUE;
					}
					break;

				case KEY_BACK:
					if (0 < iLen)
					{
						/** caInput[--iLen] = 0x00; **/
						caInput[iLen--] = 0x00;
						dmsg("caInput[%s]",caInput);
					}
					iIsUpdate = TRUE;
					break;
			}

			while (iIsUpdate)
			{
				mtiMemset(caOutput, 0, sizeof(caOutput));
				if (iFreeInput == FALSE)
				{
					iInPos = 0;

					for (i = iInPos; i < DISP_INPUT_MAX_COULUM; i++)
					{
						if (taFrmEle[i].iRelPos == -2)
						{
							caOutput[i] = taFrmEle[i].cpVal;
						}
						else
						{
							caOutput[i] = ' ';
						}
					}

					if (0 < iNumCnt)
					{
						iInPos = 0;
						iLastIndex = 0;
						for (i = 0; i < iNumCnt; i++)
						{
							if (iInPos < iLen)
							{
								caOutput[taNumEle[i].iRelPos] = caInput[iInPos++];
								iLastIndex = taNumEle[i].iRelPos;
							}
						}

						if (iLen > 0)
						{
							for (i = 0; i < iLastIndex ; i++)
							{
								if (taFrmEle[i].iRelPos == -1)
								{
									caOutput[i] = taFrmEle[i].cpVal;
								}
							}
						}
					}

					if (0 < iCurCnt)
					{
						iInPos = iLen;
						iLastIndex = 0;
						for (i = iCurCnt - 1; 0 <= i; i--)
						{
							if (0 < iInPos)
							{
								caOutput[taCurEle[i].iRelPos] = caInput[--iInPos];
								iLastIndex = taCurEle[i].iRelPos;
							}
						}

						if (iLen > 0)
						{
							for (i = DISP_INPUT_MAX_COULUM; iLastIndex < i; i--)
							{
								if (taFrmEle[i].iRelPos == -1)
								{
									caOutput[i] = taFrmEle[i].cpVal;
								}
							}
						}
						else
						{
							caOutput[taCurEle[iCurCnt - 1].iRelPos] = '0';
						}
					}
				}
				else
				{
					mtiMemset(caOutput, 0, sizeof(caOutput));
					mtiMemset(caOutput, 0x20, DISP_INPUT_MAX_COULUM);
					mtiMemcpy(caOutput, caInput, iLen);
				}

				if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_SMALL_FONT))
				{
					_DrawExtendedString(0, iLineIndex[iIndex] * LCD_PIXEL_PER_LINE, caOutput, _OFF_, SEL_SMALL_FONT, _NORMALE_);
				}
				else
				{
					_DrawExtendedString(0, iLineIndex[iIndex] * LCD_PIXEL_PER_LINE, caOutput, _OFF_, SEL_FONT, _NORMALE_);
				}

				if (iAlphaUpdate)
				{
					PaintGraphics();
				}

				iIsUpdate = FALSE;
				iAlphaUpdate = FALSE;

				//ALPHA INPUT
				if(iAlphaInput == TRUE && iKey >= KEY_0 && iKey <= KEY_9)
				{
					INT iAfterKey = KEY_NONE;

					while(1)
					{
						iAfterKey = mtiGetKeyPress();

						if(mtiIsTimeout(iInputTimer) == TRUE)
						{
							dmsg("Next Key...");
							mtiStopTimer(iInputTimer);
							iAlphaIdx = 0;
							break;
						}

						if(iAfterKey != KEY_NONE)
							break;
					}

					if(iKey == iAfterKey)
					{
						INT iIdx = 0;
						dmsg("ALPHA INPUT key[%c]", iAfterKey);
						for(iIdx = 0; iIdx < 10; iIdx++)
						{
							if(tAlphaKeyMapList[iIdx].iKeyVal == iAfterKey)
							{
								if(iAlphaIdx >= mtiStrlen(DISP_MAX_COULUM, tAlphaKeyMapList[iIdx].caAlphaVal))
								{
									iAlphaIdx = 0;
									caInput[iLen - 1] = iAfterKey;
								}
								else
								{
									dmsg("iLen[%d], iAlphaIdx[%d]", iLen, iAlphaIdx);
									caInput[iLen - 1] = tAlphaKeyMapList[iIdx].caAlphaVal[iAlphaIdx];
									iAlphaIdx++;
								}
							}
						}

						mtiResetTimer(iInputTimer);
						iIsUpdate = TRUE;
						iAlphaUpdate = TRUE;
					}
					else
					{
						iIsUpdate = FALSE;
						iAlphaUpdate = FALSE;
					}
				}
			}

			if(iAlphaInput == TRUE && iInputTimer != -1)
			{
				mtiStopTimer(iInputTimer);
			}
		}
	}

	FINAL_DISP_KEY();
	mtiStopTimer(iTid);
	return iRet;
}

INT mtiShowMenuDisp(tDispContents *tpDispCont, INT iDispContCnt, INT iDefaultSelect, tAddCtrlContents *tpAddCtrl)
{
	INT i = 0, iLen = 0, iPos = 0, iCur = 0, iUpdate = 0, iRow = 0, iKey = 0, iPageMaxCount = PAGE_MAX, iTid = 0, iNum = 0;
	INT iX = 0, iY = 0, iCnt = 0, iLeft = 0, iExitFlag = 0, iExitValue = 0, iRet = 0;

	INT iUseClearKey = 0;
	
	CHAR *cpCaption = NULL;
	CHAR caLine[(DISP_MAX_COULUM * 2) + 1];

	if (tpDispCont != NULL)
	{
		INIT_DISP_KEY();

		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i + iCur].caContents;
			iLen = SLEN(20, cpCaption);
			if (iLen > 0)
			{
				MZERO(caLine, sizeof(caLine));
				if (tpDispCont[i + iCur].iAlignment == DISP_ALIGN_CENTER)
				{
					mtiStringCenterAlign(cpCaption, caLine, DISP_MAX_COULUM);
				}
				else if (tpDispCont[i + iCur].iAlignment == DISP_ALIGN_RIGHT)
				{
					mtiStringRightAlign(cpCaption, caLine, DISP_MAX_COULUM);
				}
				else
				{
					MSPACE(caLine, DISP_MAX_COULUM);
					MCPY(caLine, cpCaption, iLen);
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_TITLE))
				{
					_DrawExtendedString(0, 0 * LCD_PIXEL_PER_LINE, caLine, _ON_, SEL_FONT, _NORMALE_);
					iRow = 1;
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					_DrawExtendedString(0, 1 * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
					iRow = 2;
				}
			}

			// Use Clear Key
			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_USE_CLEAR_KEY))
			{
				iUseClearKey = 1;
			}
		}

		iPos = 0;
		iDispContCnt -= iRow;
		if (iDispContCnt >= PAGE_MAX)
		{
			iPageMaxCount = PAGE_MAX - iRow;
		}
		else
		{
			iPageMaxCount = iDispContCnt;
		}

		for (i = 0; i < iDefaultSelect; i++)
		{
			if (iPos != (iPageMaxCount - 1))
			{
				iPos++;
			}
			else
			{
				if (iCur < iDispContCnt - iPageMaxCount)
				{
					iCur++;
					iPos = iPageMaxCount - 1;
				}
			}
		}
		iUpdate = TRUE;

		if (tpAddCtrl != NULL)
		{
			mtiProcAddCtrl(tpAddCtrl);
			iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
		}
		else
		{
			iTid = mtiStartTimer(TIMEOUT_30S);
		}

		while (_ON_)
		{
			// To receive the key input after displaying the selection menu first, Move Down.
			// PaintGraphics();			

			if (mtiIsTimeout(iTid))
			{
				FINAL_DISP_KEY();
				mtiStopTimer(iTid);
				return RTN_TIMEOUT;
			}

			if (iUpdate == TRUE)
			{
				for (i = iRow; i < iPageMaxCount + iRow; i++)
				{
					cpCaption = tpDispCont[i + iCur].caContents;
					iLen = SLEN(20, cpCaption);
					if (iLen > 0)
					{
						MZERO(caLine, sizeof(caLine));
						switch (iRow)
						{
							case 2:
								iNum = i + iCur - 1;
								break;

							case 1:
								iNum = i + iCur;
								break;

							case 0:
								iNum = i + iCur + 1;
								break;
						}

						if (10 <= iNum)
						{
							iNum -= (iNum / 10) * 10;
						}

						sprintf(caLine, "%d.%-14.14s", iNum, cpCaption);

						switch (tpDispCont[i + iCur].iAttribute)
						{
							case DISP_OPT_TITLE:
							case DISP_OPT_SUBTITLE:
								break;

							default:
								if (iPos + iRow == i)
								{
									_DrawExtendedString(0, i * LCD_PIXEL_PER_LINE, caLine, _ON_, SEL_FONT, _NORMALE_);
								}
								else
								{
									_DrawExtendedString(0, i * LCD_PIXEL_PER_LINE, caLine, _OFF_, SEL_FONT, _NORMALE_);
								}
								break;
						}
					}
				}

				dmsg("DISP_MAX_ROW[%d], iDispContCnt[%d]", DISP_MAX_ROW, iDispContCnt);
				
				// Arrows are required only when there are more than the number of screen lines
				//if (tpAddCtrl == NULL)
				if ((tpAddCtrl == NULL) && (DISP_MAX_ROW < (iDispContCnt + iRow)))
				{
					if (iCur > 0)
					{
						iY = 0;
						for (iX = 0; iX < T_SIZE; iX++)
						{
							iLeft = 0;
							for (iCnt = iX; iCnt < T_SIZE; iCnt++)
							{
								iLeft++;
							}

							for (iCnt = 0; iCnt <= iX * 2; iCnt++)
							{
								_SetPixel(LEFT_UP_ARROW + iLeft + iCnt, TOP_UP_ARROW + iY, 1);
							}

							iY++;
						}
						//PaintGraphics();
					}
					else
					{
						iY = 0;
						for (iX = 0; iX < T_SIZE; iX++)
						{
							iLeft = 0;
							for (iCnt = iX; iCnt < T_SIZE; iCnt++)
							{
								iLeft++;
							}

							for (iCnt = 0; iCnt <= iX * 2; iCnt++)
							{
								_SetPixel(LEFT_UP_ARROW + iLeft + iCnt, TOP_UP_ARROW + iY, 0);
							}

							iY++;
						}
						//PaintGraphics();
					}

					if (iCur < iDispContCnt - iPageMaxCount)
					{
						iY = 0;
						for (iX = T_SIZE; iX > 0; iX--)
						{
							iLeft = 0;
							for(iCnt = (T_SIZE + 1) - iX; iCnt > 0; iCnt--)
								iLeft++;

							for(iCnt = iX * 2; iCnt > 1; iCnt--)
							{
								_SetPixel(LEFT_DOWN_ARROW + iLeft + iCnt, TOP_DOWN_ARROW + iY, 1);
							}

							iY++;
						}
						//PaintGraphics();
					}
					else
					{
						iY = 0;
						for (iX = T_SIZE; iX > 0; iX--)
						{
							iLeft = 0;
							for(iCnt = (T_SIZE + 1) - iX; iCnt > 0; iCnt--)
								iLeft++;

							for(iCnt = iX * 2; iCnt > 1; iCnt--)
							{
								_SetPixel(LEFT_DOWN_ARROW + iLeft + iCnt, TOP_DOWN_ARROW + iY, 0);
							}

							iY++;
						}
						//PaintGraphics();
					}
				}

				iUpdate = FALSE;
			}

			// To receive the key input after displaying the selection menu first
			PaintGraphics();

			if (iExitFlag)
			{
				mtiSleep(200);
				FINAL_DISP_KEY();
				mtiStopTimer(iTid);
				return iExitValue;
			}

			iKey = mtiGetKeyPress();
			if (iKey != KEY_NONE)
			{
				mtiResetTimer(iTid);
			}

			if (tpAddCtrl != NULL)
			{
				if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_SELECT_LEFT;
				}
				else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_SELECT_RIGHT;
				}
				else if (iKey == tpAddCtrl->ucCancelKey)
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);

					if (tpAddCtrl->iCancelKeyResultValue)
					{
						iRet = tpAddCtrl->iCancelKeyResultValue;
					}
					else
					{
						iRet = RTN_CANCEL;
					}
					return iRet;
				}
				else if (iKey == tpAddCtrl->ucEnterKey)
				{
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);

					if (tpAddCtrl->iEnterKeyResultValue)
					{
						iRet = tpAddCtrl->iEnterKeyResultValue;
					}
					else
					{
						iRet = RTN_SUCCESS;
					}
					return iRet;
				}
			}

			switch (iKey)
			{
				case KEY_BACK:
					if (iUseClearKey)
					{
						FINAL_DISP_KEY();
						mtiStopTimer(iTid);
						return RTN_CLEAR_BACK;
					}
					break;
				case KEY_1:
				case KEY_2:
				case KEY_3:
				case KEY_4:
				case KEY_5:
				case KEY_6:
				case KEY_7:
				case KEY_8:
				case KEY_9:
					// Key Check
					dmsg("iDispContCnt[%d], iPageMaxCount[%d], iKey[%x], iKey - KEY_1[%d] iCur = %d", iDispContCnt, iPageMaxCount, iKey, iKey - KEY_1, iCur);
					//iDispContCnt[8], iKey[57], iKey - KEY_1[8]
					if ((iKey - KEY_1) < iDispContCnt)
					{
						if (iKey - KEY_1 < iPageMaxCount)
						{
							iPos = iKey - KEY_1;
							iCur = 0;
						}
						else
						{
							iPos = iPageMaxCount - 1;
							if ((iKey - KEY_0) < iPageMaxCount)
							{
								dpt();
								iCur = 0;
							}
							else
							{
								dpt();
								iCur = (iKey - KEY_0) - iPageMaxCount;
							}
						}

						dmsg("**** iDispContCnt[%d], iPageMaxCount[%d], iKey[%x], iKey - KEY_1[%d] iCur = %d", iDispContCnt, iPageMaxCount, iKey, iKey - KEY_1, iCur);
						iUpdate = TRUE;
						iExitFlag = TRUE;
						iExitValue = iKey - KEY_1;
						continue;
						/**
						FINAL_DISP_KEY();
						mtiStopTimer(iTid);
						return (iKey - KEY_1);
						**/
					}
					break;
				case KEY_0:
					dmsg("iDispContCnt[%d]", iDispContCnt);
					if (9 < iDispContCnt)
					{
						iPos = iPageMaxCount - 1;
						iCur = 10 - iPageMaxCount;

						iUpdate = TRUE;
						iExitFlag = TRUE;
						iExitValue = 9;
						continue;
						/*
						FINAL_DISP_KEY();
						mtiStopTimer(iTid);
						return 9;
						*/
					}
					break;
				case KEY_DOWN:
					dmsg("iDispContCnt[%d], iKey[%x], iKey - KEY_1[%d] iCur = %d", iDispContCnt, iKey, iKey - KEY_1, iCur);
					if (iPos != (iPageMaxCount - 1))
					{
						iPos++;
						iUpdate = TRUE;
					}
					else
					{
						if (iCur < iDispContCnt - iPageMaxCount)
						{
							iCur++;
							iPos = iPageMaxCount - 1;
							iUpdate = TRUE;
						}
					}
					break;

				case KEY_UP:
					if (0 < iPos)
					{
						iPos--;
						iUpdate = TRUE;
					}
					else
					{
						if (iCur > 0)
						{
							iCur--;
							iUpdate = TRUE;
						}
					}
					break;

				case KEY_ENTER:
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return iPos + iCur;

				case KEY_CANCEL:
					FINAL_DISP_KEY();
					mtiStopTimer(iTid);
					return RTN_CANCEL;
			}
		}
	}

	FINAL_DISP_KEY();
	mtiStopTimer(iTid);
	return RTN_ERROR;
}

#ifndef __TETRA__
static bool OnKeyPressed(T_GL_HMESSAGE message)
{
	GL_Message_SetResult(message, 0);
	return true;
}

static bool OnTimeout(T_GL_HMESSAGE message)
{
	GL_Message_SetResult(message, 0);
	GL_Timer_Stop(GL_Message_GetWidget(message));
	return true;
}

INT mtiShowBarCodeOnScreen(INT iBarCodeType, UCHAR *ucpBarCode, INT iBarCodeLen, tAddCtrlContents *tpAddCtrl)
{
	Telium_File_t *hKey = NULL, *hDisp = NULL;
	const CHAR *cpBarCodeTypeOfGoal = GL_MIME_BARCODE_QR;

	// Lazy alloc the global variant at "g_hGoal" when application want use the GOAL library.
	if (g_hGoal == NULL)
	{
		g_hGoal = GL_GraphicLib_Create();
	}

	// Open display
	hDisp = Telium_Fopen("DISPLAY", "w*");

	// Open keyboard
	hKey = Telium_Fopen("KEYBOARD", "r*");

	//hGoal = GL_GraphicLib_Create();

	// sample function to display a barcode ean 8
	{
		T_GL_HWIDGET hWnd;

		// Creating a blank window
		hWnd = GL_Window_Create(g_hGoal);

		// Reduces the size of window
		GL_Widget_SetSize(hWnd, 100, 100, GL_UNIT_PERCENT);

		// Create a barcode on the screen
		{
			T_GL_HWIDGET hBarCode;

			hBarCode = GL_Barcode_Create(hWnd);

			if (iBarCodeType == DISP_WIDGET_BARCODE_CODE39)
			{
				cpBarCodeTypeOfGoal = GL_MIME_BARCODE_CODE39;
			}
			else if (iBarCodeType == DISP_WIDGET_BARCODE_CODE39)
			{
				cpBarCodeTypeOfGoal = GL_MIME_BARCODE_CODE128;
			}

			// Set the mime type of barcode
			GL_Barcode_SetMimeType(hBarCode, cpBarCodeTypeOfGoal);

			// Set the value of barcode
			GL_Barcode_SetValue(hBarCode, ucpBarCode, iBarCodeLen+1);

			// Configure the margin of QR
			GL_Barcode_SetParamInteger(hBarCode, GL_BARCODE_QR_MARGIN, 2);

			// Fit the barcode to the screen
			GL_Widget_SetTransformation(hBarCode, GL_TRANSFORMATION_FIT_ALL);
		}

		// Create a timer
		{
			T_GL_HWIDGET hTimer;

			hTimer = GL_Timer_Create(hWnd);

			// Sets the timer interval
			//GL_Timer_SetInterval(hTimer, tpAddCtrl->ulTimeout);
			// @@EB QR 20190318
#if 1
			GL_Timer_SetInterval(hTimer, TIMEOUT_10S);
#else
			GL_Timer_SetInterval(hTimer, TIMEOUT_240S);
#endif

			// Start the timer
			GL_Timer_Start(hTimer);

			// Registering a callback on time change
			GL_Widget_RegisterCallback(hTimer, GL_EVENT_TIMER_OUT, OnTimeout);
		}

		// Registering a callback. This callback will be called every keypress on the keyboard
		GL_Widget_RegisterCallback(hWnd, GL_EVENT_KEY_DOWN, OnKeyPressed);

		// Display window and wait clic on button
		GL_Window_MainLoop(hWnd);

		// Destruction of the window. Destruction frees all allocated memory,
		// all the widgets attached to the window and suppress the window on the screen.
		// This call is crucial, if it forgotten you get a memory leak.
		GL_Widget_Destroy(hWnd);
	}

	// close keyboard
	if (hKey)
	{
		Telium_Fclose(hKey);
		hKey = NULL;
	}

	// close display
	if (hDisp)
	{
		Telium_Fclose(hDisp);
		hDisp = NULL;
	}

	return RTN_SUCCESS;
}

#else

static bool OnKeyPressed(T_GL_HMESSAGE message)
{
	GL_Message_SetResult(message, 0);
	return true;
}

static bool OnTimeout(T_GL_HMESSAGE message)
{
	GL_Message_SetResult(message, 0);
	GL_Timer_Stop(GL_Message_GetWidget(message));
	return true;
}

static T_GL_HWIDGET CreateBarcode(T_GL_HWIDGET parent)
{

    T_GL_HWIDGET barcode;


    barcode = GL_Barcode_Create(parent);

	if (barcode == NULL) 
	{
		dmsg(" **** GL_Barcode_Create Error=======");
		return barcode;
	}

	dmsg(" **** GL_Barcode_Create OK=======");

    // Set the mime type of barcode

    GL_Barcode_SetMimeType(barcode, GL_MIME_BARCODE_QR);

    // Set the value of barcode
    GL_Barcode_SetValue(barcode, "Hello world from qr", strlen("Hello world from qr") + 1);


    // Configure the margin of QR
    GL_Barcode_SetParamInteger(barcode, GL_BARCODE_QR_MARGIN, 2);


    // Fit the barcode to the screen
    GL_Widget_SetTransformation(barcode, GL_TRANSFORMATION_FIT_ALL);

    return barcode;

}


INT mtiShowBarCodeOnScreenTest()
{
	T_GL_HWIDGET window;
	//TGoal  = GL_GraphicLib_CreateWithParam(0, "STON");

	dmsg(" **** mtiShowBarCodeOnScreenTest TETRA  =====================");

	// Lazy alloc the global variant at "g_hGoal" when application want use the GOAL library.
	/*if (TGoal == NULL)
	{
		TGoal = GL_GraphicLib_Create();
		dmsg(" **** GL_GraphicLib_Create()  =====================");
	}*/

	// Creating a blank window
	window = GL_Window_Create(TGoal);

	if (window == NULL) 
	{
		dmsg(" **** GL_Window_Create Error=======");
		return RTN_ERROR;
	}

	dmsg(" **** GL_Window_Create OK=======");

	// Reduces the size of window
	//GL_Widget_SetSize(window, 100, 100, GL_UNIT_PERCENT);
	GL_Widget_SetSize(window, 200, 200, GL_UNIT_PIXEL);

	CreateBarcode(window);

	// Registering a callback. This callback will be called every keypress on the keyboard
	GL_Widget_RegisterCallback(window, GL_EVENT_KEY_DOWN, OnKeyPressed);

	// Display window and wait clic on button
	GL_Window_MainLoop(window);

	// Destruction of the window. Destruction frees all allocated memory,
	// all the widgets attached to the window and suppress the window on the screen.
		// This call is crucial, if it forgotten you get a memory leak.
	GL_Widget_Destroy(window);


	return RTN_SUCCESS;
}


INT mtiShowBarCodeOnScreen(INT iBarCodeType, UCHAR *ucpBarCode, INT iBarCodeLen, tAddCtrlContents *tpAddCtrl)
{
	Telium_File_t *hKey = NULL, *hDisp = NULL;
	T_GL_HWIDGET window;

	// Open display
	hDisp = Telium_Fopen("DISPLAY", "w*");

	// Open keyboard
	hKey = Telium_Fopen("KEYBOARD", "r*");

	// Creating a blank window
	window = GL_Window_Create(TGoal);

	GL_Widget_SetSize(window, 200, 200, GL_UNIT_PIXEL);

	// Create a barcode on the screen
	{
		T_GL_HWIDGET hBarCode;

		hBarCode = GL_Barcode_Create(window);

		// Set the mime type of barcode
		GL_Barcode_SetMimeType(hBarCode, GL_MIME_BARCODE_QR);

		// Set the value of barcode
		GL_Barcode_SetValue(hBarCode, ucpBarCode, iBarCodeLen+1);

		// Configure the margin of QR
		GL_Barcode_SetParamInteger(hBarCode, GL_BARCODE_QR_MARGIN, 2);

		// Fit the barcode to the screen
		GL_Widget_SetTransformation(hBarCode, GL_TRANSFORMATION_FIT_ALL);
	}

	// Create a timer
	{
		T_GL_HWIDGET hTimer;

		hTimer = GL_Timer_Create(window);

		// Sets the timer interval
		//GL_Timer_SetInterval(hTimer, tpAddCtrl->ulTimeout);
		// @@EB QR 20190304
#if 1
		GL_Timer_SetInterval(hTimer, TIMEOUT_10S);
#else
		GL_Timer_SetInterval(hTimer, TIMEOUT_240S);
#endif
		//

		// Start the timer
		GL_Timer_Start(hTimer);

		// Registering a callback on time change
		GL_Widget_RegisterCallback(hTimer, GL_EVENT_TIMER_OUT, OnTimeout);
	}

	// Registering a callback. This callback will be called every keypress on the keyboard
	GL_Widget_RegisterCallback(window, GL_EVENT_KEY_DOWN, OnKeyPressed);

	// Display window and wait clic on button
	GL_Window_MainLoop(window);

	// Destruction of the window. Destruction frees all allocated memory,
	// all the widgets attached to the window and suppress the window on the screen.
		// This call is crucial, if it forgotten you get a memory leak.
	GL_Widget_Destroy(window);

	// close keyboard
	if (hKey)
	{
		Telium_Fclose(hKey);
		hKey = NULL;
	}

	// close display
	if (hDisp)
	{
		Telium_Fclose(hDisp);
		hDisp = NULL;
	}

	return RTN_SUCCESS;
}

INT mtiShowBarCodeOnPrint(INT iBarCodeType, UCHAR *ucpBarCode, INT iBarCodeLen)
{
	T_GL_HWIDGET document;
	T_GL_HWIDGET layout;
	T_GL_HWIDGET hBarCode;

	// Creating a blank window
		
	document = GL_Document_Create(TGoal);		
	layout = GL_Layout_Create(document);

	// Create a barcode on the screen
	
	hBarCode = GL_Barcode_Create(layout);

	// Set the mime type of barcode
	GL_Barcode_SetMimeType(hBarCode, GL_MIME_BARCODE_QR);

	// Set the value of barcode

	dmsg(" **** In mtiShowBarCodeOnPrint [%s][%d]", ucpBarCode, iBarCodeLen);
	GL_Barcode_SetValue(hBarCode, ucpBarCode, iBarCodeLen+1);

	// Configure the margin of QR
	GL_Barcode_SetParamInteger(hBarCode, GL_BARCODE_QR_MARGIN, 2);

	// Fit the barcode to the screen
	GL_Widget_SetTransformation(hBarCode, GL_TRANSFORMATION_FIT_ALL);

// @@EB QR 20190122
#if 1
	GL_Widget_SetMinSize(hBarCode, 375, 375, GL_UNIT_PIXEL);
#else
	GL_Widget_SetMinSize(hBarCode, 300, 300, GL_UNIT_PIXEL);
#endif
	
	GL_Document_Print(document, 0);
		
	GL_Widget_Destroy(document);
		

	return RTN_SUCCESS;
}

#endif


