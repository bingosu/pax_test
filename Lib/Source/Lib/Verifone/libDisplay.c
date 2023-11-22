
#include <stdio.h>

#define SET_DISPLAY_INFO	1

#include "libMtiCommonApi.h"

/*********************************************************************************************************
 * DISPLAY DEFUALT SETTING
*********************************************************************************************************/
#define PAGE_MAX					DISP_MAX_ROW - 1

#define CD_PIXEL_PER_LINE			128

const char UP_ARROW = 0x14;
const char DOWN_ARROW = 0x16;

INT mtDisplay = 0;

#define	EVT_TOUCH	(1<<1)
unsigned long  lOSEvent;
BOOL IsTouchSupport;

#define KEY_TOUCH_LEFT_BUTTON	100
#define KEY_TOUCH_RIGHT_BUTTON 	101

#define BMP_LOCATE_LEFT_X	25
#define BMP_LOCATE_LEFT_Y	400
#define BMP_LOCATE_RIGHT_X	230
#define BMP_LOCATE_RIGHT_Y	400



static VOID dbgDispCts(tDispContents *tDispCts, INT iCtsCnt);


/*
#define INIT_DISP_KEY()									\
	mtDisplay = open(DEV_CONSOLE, O_WRONLY);			\
	mtiInitializeKeyboard();							\
	mtiClearDisp();
*/

/*
#define FINAL_DISP_KEY()								\
	if (mtDisplay != NULL) close(mtDisplay);			\
	mtiFinalizeKeyboard();
*/

#define INIT_DISP_KEY()

#define FINAL_DISP_KEY()

#define INIT_DISP()										\
	do{													\
		if(mtDisplay == 0)								\
			mtDisplay = open(DEV_CONSOLE, O_RDWR);	\
		mtiClearDisp();									\
	}while(0);

#define FINAL_DISP()

#define SEC_INPUT_MINIMUM_LEN		4

BOOL mtiCheckQRDispCap()
{
	CHAR caModelNum[12+1] = {0,};
	INT iRet;

	iRet = SVC_INFO_MODELNO(caModelNum);
	if (iRet < 0) return FALSE;

	if(mtiMemcmp(caModelNum + 2, (CHAR *)"52", 2) == 0)
	{
		if(mtiMemcmp(caModelNum + 2, (CHAR *)"520C", 4) == 0) return TRUE;
		else return FALSE;
	}
	else if(mtiMemcmp(caModelNum + 2, (CHAR *)"67", 2) == 0) return TRUE;
	else if(mtiMemcmp(caModelNum + 1, (CHAR *)"680", 3) == 0) return TRUE;
	else return FALSE;
}


VOID mtiSetDisplayInfo()
{
	INT iHwType = mtiGetHwType();

	if(iHwType == VX520)
	{
		g_DISP_INPUT_MAX_COULUM = 21;
		g_DISP_MAX_COULUM = 21;
		g_DIS_MAX_ROW = 8;
	}
	else //VX675 and ETC??
	{
		g_DISP_INPUT_MAX_COULUM = 20;
		g_DISP_MAX_COULUM = 20;
		g_DIS_MAX_ROW = 15;
	}

	return;
}

static VOID mtiSetTextInverse(INT iOnOff)
{
	if(iOnOff > 0)	//ON
	{
		set_display_color(FOREGROUND_COLOR, FOREGROUND_COLOR_WHITE);
		set_display_color(BACKGROUND_COLOR, BACKGROUND_COLOR_BLACK);
	}
	else
	{
		set_display_color(FOREGROUND_COLOR, BACKGROUND_COLOR_BLACK);
		set_display_color(BACKGROUND_COLOR, FOREGROUND_COLOR_WHITE);
	}
}

static VOID mtiDisplayTextInverse(CHAR *cpStr, INT iLen, INT iX, INT iY)
{
	mtiSetTextInverse(TRUE);
	write_at(cpStr, iLen, iX, iY);
	mtiSetTextInverse(FALSE);
}

static CHAR *staticCpBothSideAlign(CHAR *cpLeftStr, CHAR *cpRightStr)
{
	static CHAR caAlignedStr[100];

	INT iOffset  = 0;
	CHAR caSpNum[10] = {0,};
	CHAR caCpyFmt[50] = {0,};
	memset(caAlignedStr, 0x00 , sizeof(caAlignedStr));

	iOffset = 0;
	sprintf(caSpNum, "%d", DISP_MAX_COULUM - mtiStrlen(DISP_MAX_COULUM, cpLeftStr));
	iOffset += mtiStrcpy(caCpyFmt + iOffset, "%s%", DISP_MAX_COULUM);
	iOffset += mtiStrcpy(caCpyFmt + iOffset, caSpNum, DISP_MAX_COULUM);
	iOffset += mtiStrcpy(caCpyFmt + iOffset, "s", DISP_MAX_COULUM);

	//dmsg("caCpyFmt:[%s]", caCpyFmt);
	sprintf(caAlignedStr, caCpyFmt, cpLeftStr, cpRightStr);

	return caAlignedStr;
}

VOID mtiDisplayMenuArrowBtn()
{
	if(mtiGetHwType() == C680)
	{
		
		set_display_coordinate_mode (PIXEL_MODE);
        put_BMP_at (BMP_LOCATE_LEFT_X, BMP_LOCATE_LEFT_Y, "I:1/WWW/320X480C0T/images/menuup.BMP");
		put_BMP_at (BMP_LOCATE_RIGHT_X, BMP_LOCATE_RIGHT_Y, "I:1/WWW/320X480C0T/images/menudown.BMP");
        set_display_coordinate_mode (CHARACTER_MODE);  
        dmsg("set_display_coordinate_mode - CHARACTER_MODE");

		set_event_bit (mtDisplay, EVT_TOUCH);

		IsTouchSupport = TRUE;
	}
}

VOID mtiProcAddCtrl(tAddCtrlContents *tpAddCtrl)
{
	CHAR caBottom[25 + 1];
	INT inRet = 0;

	MZERO(caBottom, sizeof(caBottom));
	MSPACE(caBottom, sizeof(caBottom) - 1);
	if (tpAddCtrl != NULL)
	{
		if(mtiGetHwType() == VX520)
		{
			if(tpAddCtrl->ucCancelKey == KEY_F1)
				tpAddCtrl->ucCancelKey = KEY_DOWN;
			if(tpAddCtrl->ucEnterKey == KEY_F4)
				tpAddCtrl->ucEnterKey = KEY_UP;
			if(tpAddCtrl->tBtnLeft.cRegKey == KEY_F1)
				tpAddCtrl->tBtnLeft.cRegKey = KEY_DOWN;
			if(tpAddCtrl->tBtnRight.cRegKey == KEY_F4)
				tpAddCtrl->tBtnRight.cRegKey = KEY_UP;
		}


		if(mtiGetHwType() == C680)
		{
			dmsg("C680  - mtiProcAddCtrl");
			dmsg("LEFT KEY=[%s] RIGHT KEY=[%s]", tpAddCtrl->tBtnLeft.caContents, tpAddCtrl->tBtnRight.caContents);

			if (mtiMemcmp(tpAddCtrl->tBtnLeft.caContents, "NO", 2) == 0)
			{
				set_display_coordinate_mode (PIXEL_MODE);
            	put_BMP_at (BMP_LOCATE_LEFT_X, BMP_LOCATE_LEFT_Y, "I:1/WWW/320X480C0T/images/no.BMP");
				put_BMP_at (BMP_LOCATE_RIGHT_X, BMP_LOCATE_RIGHT_Y, "I:1/WWW/320X480C0T/images/yes.BMP");
            	set_display_coordinate_mode (CHARACTER_MODE);  
            	dmsg("set_display_coordinate_mode - CHARACTER_MODE");

				set_event_bit (mtDisplay, EVT_TOUCH);

				IsTouchSupport = TRUE;
			}
			else if (mtiMemcmp(tpAddCtrl->tBtnLeft.caContents, "EXIT", 4) == 0)
			{
				set_display_coordinate_mode (PIXEL_MODE);
            	put_BMP_at (BMP_LOCATE_LEFT_X, BMP_LOCATE_LEFT_Y, "I:1/WWW/320X480C0T/images/no.BMP");
				put_BMP_at (BMP_LOCATE_RIGHT_X, BMP_LOCATE_RIGHT_Y, "I:1/WWW/320X480C0T/images/next.BMP");
            	set_display_coordinate_mode (CHARACTER_MODE);  
            	dmsg("set_display_coordinate_mode - CHARACTER_MODE");

				set_event_bit (mtDisplay, EVT_TOUCH);

				IsTouchSupport = TRUE;
			}
			else if (mtiMemcmp(tpAddCtrl->tBtnLeft.caContents, "PREV", 4) == 0)
			{
				set_display_coordinate_mode (PIXEL_MODE);
            	put_BMP_at (BMP_LOCATE_LEFT_X, BMP_LOCATE_LEFT_Y, "I:1/WWW/320X480C0T/images/prev.BMP");
				put_BMP_at (BMP_LOCATE_RIGHT_X, BMP_LOCATE_RIGHT_Y, "I:1/WWW/320X480C0T/images/next.BMP");
            	set_display_coordinate_mode (CHARACTER_MODE);  
            	dmsg("set_display_coordinate_mode - CHARACTER_MODE");

				set_event_bit (mtDisplay, EVT_TOUCH);

				IsTouchSupport = TRUE;
			}
			else if (mtiMemcmp(tpAddCtrl->tBtnLeft.caContents, "SIGN", 4) == 0)
			{
				set_display_coordinate_mode (PIXEL_MODE);
            	put_BMP_at (BMP_LOCATE_LEFT_X, BMP_LOCATE_LEFT_Y, "I:1/WWW/320X480C0T/images/SIGN.BMP");
				put_BMP_at (BMP_LOCATE_RIGHT_X, BMP_LOCATE_RIGHT_Y, "I:1/WWW/320X480C0T/images/PIN.BMP");
            	set_display_coordinate_mode (CHARACTER_MODE);  
            	dmsg("set_display_coordinate_mode - CHARACTER_MODE");

				set_event_bit (mtDisplay, EVT_TOUCH);

				IsTouchSupport = TRUE;
			}
			else if (mtiMemcmp(tpAddCtrl->tBtnLeft.caContents, "IDR", 3) == 0)
			{
				set_display_coordinate_mode (PIXEL_MODE);
            	put_BMP_at (BMP_LOCATE_LEFT_X, BMP_LOCATE_LEFT_Y, "I:1/WWW/320X480C0T/images/IDR.BMP");
				put_BMP_at (BMP_LOCATE_RIGHT_X, BMP_LOCATE_RIGHT_Y, "I:1/WWW/320X480C0T/images/OFFER.BMP");
            	set_display_coordinate_mode (CHARACTER_MODE);  
            	dmsg("set_display_coordinate_mode - CHARACTER_MODE");

				set_event_bit (mtDisplay, EVT_TOUCH);

				IsTouchSupport = TRUE;
			}
			else if (mtiMemcmp(tpAddCtrl->tBtnLeft.caContents, "CANCEL", 6) == 0)
			{
				set_display_coordinate_mode (PIXEL_MODE);
            	put_BMP_at (BMP_LOCATE_LEFT_X, BMP_LOCATE_LEFT_Y, "I:1/WWW/320X480C0T/images/no.BMP");
				put_BMP_at (BMP_LOCATE_RIGHT_X, BMP_LOCATE_RIGHT_Y, "I:1/WWW/320X480C0T/images/yes.BMP");
            	set_display_coordinate_mode (CHARACTER_MODE);  
            	dmsg("set_display_coordinate_mode - CHARACTER_MODE");

				set_event_bit (mtDisplay, EVT_TOUCH);

				IsTouchSupport = TRUE;
			}
			
		}
		else
		{

			write_at(staticCpBothSideAlign(tpAddCtrl->tBtnLeft.caContents, tpAddCtrl->tBtnRight.caContents), DISP_MAX_COULUM, 1, DISP_MAX_ROW);
		}

		
	}
}

INT mtiGetTouchKey(INT currX, INT currY)
{
	dmsg("mtiGetTouchKey X=[%d] Y=[%d]", currX, currY);

	if (currX >= 30 && currX <=100)
	{
		
		if (currY >= 400 && currX <=470) return KEY_TOUCH_LEFT_BUTTON;
		else KEY_NONE;
	}


	if (currX >= 235 && currX <=290)
	{
		
		if (currY >= 400 && currX <=470) return KEY_TOUCH_RIGHT_BUTTON;
		else KEY_NONE;
	}

	
	return KEY_NONE;

}

VOID mtiClearDisp()
{
	//Only for Vx675 CONSOLE FONT SETTING
	if(mtiGetHwType() == VX675 || mtiGetHwType() == VX520C || mtiGetHwType() == C680)
	{
		setfont(VX675_CONSOLE_FONT_FILE);
		set_display_color(CURRENT_PALETTE, 2);
		set_display_color(FOREGROUND_COLOR, FOREGROUND_COLOR_BLACK);
		set_display_color(BACKGROUND_COLOR, BACKGROUND_COLOR_WHITE);
	}
	//setfont("asc4x8.vft");
	keyBufferClear();
	clrscr();
}

INT mtiShowDialogDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl)
{
	INT i = 0, iLen = 0, iRet = RTN_SUCCESS, iTid = 0, iKey = 0;
	INT nDiableTimout = 0;
	CHAR *cpCaption = NULL;
	CHAR *caLine = NULL;

	/////////////////////////////////////////////
	int            state = 0;
	int            currX = 0, currY = 0;
    char           buff[100];
    char           PenState[50];
	/////////////////////////////////////////////

	if (getIsADKGUI() == TRUE)
	{
		dmsg("GUI is Working...Launch GUI Dialog");
		iRet = iAdkGuiShowDialogDisp(tpDispCont, iDispContCnt, tpAddCtrl);
		return iRet;
	}
	else
	{
		dmsg("!!!GUI is Not Working!!!");
	}

	caLine = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caLine, 0x00, DISP_MAX_COULUM + 1);

	if (tpDispCont != NULL)
	{
		INIT_DISP();

		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i].caContents;
			iLen = SLEN(DISP_MAX_COULUM, cpCaption);
			if (iLen > 0)
			{
				MZERO(caLine, DISP_MAX_COULUM + 1);
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
					mtiDisplayTextInverse(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,1);
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,2);
				}
				else
				{
					write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,i + 1);
				}

				// Disable timeout
				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NOTUSED_TIMEOUT))
				{
					nDiableTimout = 1;
				}

			}
		}

		if (tpAddCtrl != NULL)
		{
			IsTouchSupport = FALSE;
			
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

				if (IsTouchSupport == TRUE)
				{
					lOSEvent = 0;
					lOSEvent = read_evt(EVT_TOUCH);

					if (lOSEvent)
					{
		   				state = get_touchscreen (&currX, &currY);
		   				//strncpy(PenState, state ? "Pen Down X=%-3d Y=%-3d   \n" : "Pen =Up   X=%-3d Y=%-3d   \n", 22);
		   				//sprintf(buff,PenState,currX,currY);
           				//dmsg("[%d]get_touchscreen XY[%s]", state, buff);

						if (state)
						{
							switch (mtiGetTouchKey(currX, currY))
							{
								case KEY_TOUCH_LEFT_BUTTON :
									dpt(); mtiSoundBeep(50, 1);
									dmsg("LEFT KEY TOUCH");
									iKey = tpAddCtrl->tBtnLeft.cRegKey;
									break;
								case KEY_TOUCH_RIGHT_BUTTON :
									dpt(); mtiSoundBeep(70, 1);
									dmsg("RIGHT KEY TOUCH");
									iKey = tpAddCtrl->tBtnRight.cRegKey;
									break;
								default :
									iKey = KEY_NONE;
									break;
							}
						}

					}
				}

				if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
				{
					FINAL_DISP();
					mtiStopTimer(iTid);

					if(caLine)
						mtiFree(caLine);
					return RTN_SELECT_LEFT;
				}
				else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
				{
					FINAL_DISP();
					mtiStopTimer(iTid);
					if(caLine)
						mtiFree(caLine);
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

	FINAL_DISP();

	if(caLine)
		mtiFree(caLine);

	return iRet;
}


INT mtiShowSecureInput(tDispContents *tpDispCont, INT iDispContCnt, INT iSecLength, tAddCtrlContents *tpAddCtrl)
{
	INT iRet = 0, i = 0, iLen = 0, iCnt = 0, iIndex = 0, iKey = 0, iLeft = 0, iTid = 0;
	CHAR *cpCaption = NULL, *cpIn = NULL;
	CHAR *caLine = NULL;
	CHAR *caInput = NULL;

	/////////////////////////////////////////////
	int            state = 0;
	int            currX = 0, currY = 0;
    char           buff[100];
    char           PenState[50];
	/////////////////////////////////////////////

	if (getIsADKGUI() == TRUE)
	{
		dmsg("GUI is Working...Not Allowed Console Access!");
		return RTN_ERROR;
	}

	caLine = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caLine, 0x00, DISP_MAX_COULUM + 1);

	caInput = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caInput, 0x00, DISP_MAX_COULUM + 1);

	if (tpDispCont != NULL)
	{
		INIT_DISP();

		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i].caContents;
			iLen = SLEN(DISP_MAX_COULUM, cpCaption);
			if (iLen > 0)
			{
				MZERO(caLine, DISP_MAX_COULUM + 1);
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
					mtiDisplayTextInverse(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,1);
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,2);				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_INPUT))
				{
					write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,i + 1);
					iIndex = i + 1;
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NORMAL))
				{
					write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,i + 1);
				}
			}
		}

		MZERO(caInput, DISP_MAX_COULUM + 1);
		cpIn = caInput;
		iLeft = (DISP_MAX_COULUM - iSecLength) / 2;
		iLen = 0;

		IsTouchSupport = FALSE;

		if (tpAddCtrl != NULL)
		{
			mtiProcAddCtrl(tpAddCtrl);
			iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
		}
		else
		{
			iTid = mtiStartTimer(TIMEOUT_10S);
		}

		if (iIndex > 0)
		{
			while(TRUE)
			{
				if (mtiIsTimeout(iTid))
				{
					FINAL_DISP();
					mtiStopTimer(iTid);
					if(caLine)
						mtiFree(caLine);
					if(caInput)
						mtiFree(caInput);
					return RTN_TIMEOUT;
				}

				iKey = mtiGetKeyPress();


				if (IsTouchSupport == TRUE)
				{
	
					lOSEvent = 0;
					lOSEvent = read_evt(EVT_TOUCH);

					if (lOSEvent)
					{
		   				state = get_touchscreen (&currX, &currY);
		   				strncpy(PenState, state ? "Pen Down X=%-3d Y=%-3d   \n" : "Pen =Up   X=%-3d Y=%-3d   \n", 22);
		   				sprintf(buff,PenState,currX,currY);
           				dmsg("[%d]get_touchscreen XY[%s]", state, buff);

						if (state)
						{
							switch (mtiGetTouchKey(currX, currY))
							{
								case KEY_TOUCH_LEFT_BUTTON :
									dpt(); mtiSoundBeep(50, 1);
									dmsg("LEFT KEY TOUCH");
									iKey = tpAddCtrl->tBtnLeft.cRegKey;
									break;
								case KEY_TOUCH_RIGHT_BUTTON :
									dpt(); mtiSoundBeep(70, 1);
									dmsg("RIGHT KEY TOUCH");
									iKey = tpAddCtrl->tBtnRight.cRegKey;
									break;
								default :
									iKey = KEY_NONE;
									break;
							}
						}

					}
				}
				
				if (iKey != KEY_NONE)
				{
					mtiResetTimer(iTid);
				}

				if (tpAddCtrl != NULL)
				{
					if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
					{
						FINAL_DISP();
						mtiStopTimer(iTid);
						if(caLine)
							mtiFree(caLine);
						if(caInput)
							mtiFree(caInput);
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
								continue;
							}
						}

						FINAL_DISP();
						mtiStopTimer(iTid);
						MCPY(tpDispCont[iIndex - 1].caInout, caInput, iLen);
						if(caLine)
							mtiFree(caLine);
						if(caInput)
							mtiFree(caInput);
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
						FINAL_DISP();
						mtiStopTimer(iTid);
						if(caLine)
							mtiFree(caLine);
						if(caInput)
							mtiFree(caInput);
						return RTN_SUCCESS;

					case KEY_CANCEL:
						FINAL_DISP();
						mtiStopTimer(iTid);
						if(caLine)
							mtiFree(caLine);
						if(caInput)
							mtiFree(caInput);
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
							cpIn[iLen++] = (CHAR)iKey;

							MZERO(caLine, DISP_MAX_COULUM + 1);
							MSPACE(caLine, DISP_MAX_COULUM + 1);
							for (iCnt = iLeft; iCnt < (iLeft + SLEN(DISP_MAX_COULUM, caInput)); iCnt++)
							{
								caLine[iCnt] = '*';
							}

							write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,iIndex + 1);
						}
						break;

					case KEY_BACK:
						if (0 < iLen)
						{
							cpIn[--iLen] = 0x00;

							MZERO(caLine, DISP_MAX_COULUM + 1);
							MSPACE(caLine, DISP_MAX_COULUM + 1);
							for (iCnt = iLeft; iCnt < (iLeft + SLEN(DISP_MAX_COULUM, caInput)); iCnt++)
							{
								caLine[iCnt] = '*';
							}

							write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,iIndex + 1);
						}
						break;
				}
			}
		}
	}

	FINAL_DISP();
	mtiStopTimer(iTid);

	if(caLine)
		mtiFree(caLine);
	if(caInput)
		mtiFree(caInput);

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

tAlphaKeyMap tAlphaKeyMapList[12] ={
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
		{KEY_DOT, "#!@#$%^&()_=+"},
		{KEY_FUNC, ",'\"?"},
};

INT mtiShowInputDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl)
{
	INT iRet = 0, i = 0, iLen = 0, iCnt = 0, iIndex = 0, iKey = 0, iRow = 0, iTid = 0, iAfterShow = 0;
	INT iSelIndex = 0, ii = 0, iInPos = 0, iMaxInput = 0, iLastIndex = 0;
	INT iIsUpdate = FALSE, iIsShown = FALSE;
	INT iLineIndex[2] = { 0 }, iLineOrg[2] = { 0 };
	INT iSkipFlag = FALSE, iDefaultFlag = FALSE, iFreeInput = FALSE, iCurrencyFlag = FALSE;
	INT iAlphaInput = FALSE;

	INT iAddLine = 0;
	
	CHAR *cpCaption = NULL;
	CHAR *caLine = NULL;
	CHAR *caInput = NULL;
	CHAR *caOutput = NULL;

	CHAR cFmt = 0;
	tFmtEle *taFrmEle = NULL;
	tFmtEle *taCurEle = NULL;
	tFmtEle *taNumEle = NULL;
	tFmtEle *taAnsEle = NULL;
	INT iNumCnt = 0, iCurCnt = 0, iANSCnt = 0;

	INT iInputTimer = -1;
	INT iAlphaIdx = 0;
	INT nDiableTimout = 0;

	/////////////////////////////////////////////
	int            state = 0;
	int            currX = 0, currY = 0;
    char           buff[100];
    char           PenState[50];
	/////////////////////////////////////////////

	if (getIsADKGUI() == TRUE)
	{
		dmsg("GUI is Working...Not Allowed Console Access!");
		return RTN_ERROR;
	}

	caLine = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caLine, 0x00, DISP_MAX_COULUM + 1);

	caInput = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caInput, 0x00, DISP_MAX_COULUM + 1);

	caOutput = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caOutput, 0x00, DISP_MAX_COULUM + 1);

	taFrmEle = (tFmtEle *)mtiMalloc((DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
	mtiMemset(taFrmEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
	taCurEle = (tFmtEle *)mtiMalloc((DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
	mtiMemset(taCurEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
	taNumEle = (tFmtEle *)mtiMalloc((DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
	mtiMemset(taNumEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
	taAnsEle = (tFmtEle *)mtiMalloc((DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
	mtiMemset(taAnsEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));

	if (tpDispCont != NULL)
	{
		INIT_DISP();

		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i].caContents;
			iLen = SLEN(DISP_MAX_COULUM, cpCaption);
			if (iLen > 0)
			{
				MZERO(caLine, DISP_MAX_COULUM + 1);
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
					dmsg("tpDispCont[%d].iAttribute = DISP_OPT_TITLE[%s]", i, caLine);
					mtiDisplayTextInverse(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,1);
					iRow = 1;
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					dmsg("tpDispCont[%d].iAttribute = DISP_OPT_SUBTITLE[%s]", i, caLine);
					write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,2);
					iRow = 2;
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_INPUT))
				{

					iLineIndex[0] = i + 1;
					iLineOrg[0] = i;
					if (tpDispCont[i].caDefValue[0] > 0)
					{
						dmsg("tpDispCont[%d].iAttribute = DISP_OPT_INPUT[%s]", i, tpDispCont[i].caDefValue);
						write_at(tpDispCont[i].caDefValue, mtiStrlen(DISP_MAX_COULUM, tpDispCont[i].caDefValue),1 , i + iAddLine + 2);
					}
					iAddLine++;
					
					iCnt++;
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NORMAL))
				{
					dmsg("tpDispCont[%d].iAttribute = DISP_OPT_NORMAL[%s]", i, caLine);
					write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , i + iAddLine+ 1);

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
				iLen = SLEN(DISP_MAX_COULUM, cpCaption);
				if (iLen > 0)
				{
					MZERO(caLine, DISP_MAX_COULUM + 1);
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
								//dmsg("caLine AMT[%s]", caLine);
								dmsg("iRow == 1 tpDispCont[%d].iAttribute = DISP_OPT_INPUT[%s]", i, caLine);
								write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,1 + 1 );
								iLineOrg[0] = i;
								iLineIndex[0] = 2;
							}
							else if (i == 2)
							{
								//dmsg("caLine TIP[%s]",caLine);
								dmsg("iRow == 1 2 tpDispCont[%d].iAttribute = DISP_OPT_INPUT[%s]", i, caLine);
								write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , 3 + 1 );
								iLineOrg[1] = i;
								iLineIndex[1] = 4;
							}
						}
						else if (iRow == 2)
						{
							if (i == 2)
							{
								dmsg("iRow == 2 tpDispCont[%d].iAttribute = DISP_OPT_INPUT[%s]", i, caLine);
								write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , 2 + 1 );
								iLineOrg[0] = i;
								iLineIndex[0] = 3;
							}
							else if (i == 3)
							{
								dmsg("iRow == 2 2 tpDispCont[%d].iAttribute = DISP_OPT_INPUT[%s]", i, caLine);
								write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , 4 + 1 );
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
			dmsg("iCnt= 0 caLine[%s]", caLine);
			write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , iLineOrg[0] + 1 );
		}


		MZERO(caInput, DISP_MAX_COULUM + 1);
		iIndex = 0;
		iLen = 0;

		IsTouchSupport = FALSE;

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
		while(TRUE)
		{
			if (nDiableTimout)
			{
			}
			else
			{
				if (mtiIsTimeout(iTid))
				{
					FINAL_DISP();
					mtiStopTimer(iTid);
					if(iAlphaInput == TRUE && iInputTimer != -1)
						mtiStopTimer(iInputTimer);
					if(caLine)
						mtiFree(caLine);
					if(caInput)
						mtiFree(caInput);
					if(caOutput)
						mtiFree(caOutput);
					if(taFrmEle)
						mtiFree(taFrmEle);
					if(taCurEle)
						mtiFree(taCurEle);
					if(taNumEle)
						mtiFree(taNumEle);
					if(taAnsEle)
						mtiFree(taAnsEle);
					return RTN_TIMEOUT;
				}
			}

			iKey = mtiGetKeyPress();

			if (IsTouchSupport == TRUE)
			{
				lOSEvent = 0;
				lOSEvent = read_evt(EVT_TOUCH);

				if (lOSEvent)
				{
		   			state = get_touchscreen (&currX, &currY);
		   			strncpy(PenState, state ? "Pen Down X=%-3d Y=%-3d   \n" : "Pen =Up   X=%-3d Y=%-3d   \n", 22);
		   			sprintf(buff,PenState,currX,currY);
           			dmsg("[%d]get_touchscreen XY[%s]", state, buff);

					if (state)
					{
						switch (mtiGetTouchKey(currX, currY))
						{
							case KEY_TOUCH_LEFT_BUTTON :
								dpt(); mtiSoundBeep(50, 1);
								dmsg("LEFT KEY TOUCH");
								iKey = tpAddCtrl->tBtnLeft.cRegKey;
								break;
							case KEY_TOUCH_RIGHT_BUTTON :
								dpt(); mtiSoundBeep(70, 1);
								dmsg("RIGHT KEY TOUCH");
								iKey = tpAddCtrl->tBtnRight.cRegKey;
								break;
							default :
								iKey = KEY_NONE;
								break;
						}
					}

				}
			}

			if (iKey != 255) dmsg("mtiGetKeyPress KEY[%d]", iKey);

			if (iKey != KEY_NONE)
			{
				mtiResetTimer(iTid);
			}

			if (iSelIndex != iIndex)
			{
				mtiMemset(taFrmEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
				mtiMemset(taCurEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
				mtiMemset(taNumEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
				mtiMemset(taAnsEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));

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
						iMaxInput = g_DISP_INPUT_MAX_COULUM;
					}
					else
					{
						iMaxInput = g_DISP_INPUT_MAX_COULUM;
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
				//dmsg("iMaxInput = %d", iMaxInput);
			}

			if (tpAddCtrl != NULL && iIsShown)
			{
				INT iRtn = 0;
				//dmsg("iKey[%d], tpAddCtrl->tBtnRight.cRegKey[%d]", iKey, tpAddCtrl->tBtnRight.cRegKey);
				if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
				{
					dmsg("iKey[%d], tpAddCtrl->tBtnLeft.cRegKey[%d]", iKey, tpAddCtrl->tBtnLeft.cRegKey);
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					FINAL_DISP();
					mtiStopTimer(iTid);
					iRtn = RTN_SELECT_LEFT;
				}
				else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
				{
					//dmsg("iIndex [%d], iLineOrg[iIndex] [%d], caInput[%s] iLen[%d]",iIndex, iLineOrg[iIndex], caInput, iLen );
					dmsg("iKey[%d], tpAddCtrl->tBtnRight.cRegKey[%d]", iKey, tpAddCtrl->tBtnRight.cRegKey);
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					FINAL_DISP();
					mtiStopTimer(iTid);
					iRtn = RTN_SELECT_RIGHT;
				}
				else if (iKey == tpAddCtrl->ucCancelKey)
				{
					dmsg("iKey[%d], tpAddCtrl->ucCancelKey[%d]", iKey, tpAddCtrl->ucCancelKey);
					if (tpAddCtrl->iCancelKeyResultValue)
					{
						dmsg("2 tpAddCtrl->iCancelKeyResultValue[%d]", tpAddCtrl->iCancelKeyResultValue);
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
					if(iLen > 0) MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					dmsg("iKey[%d], tpAddCtrl->ucEnterKey[%d]", iKey, tpAddCtrl->ucEnterKey);
					if (tpAddCtrl->iEnterKeyResultValue)
					{
						dmsg("3 tpAddCtrl->iEnterKeyResultValue[%d]", tpAddCtrl->iEnterKeyResultValue);
						iRet = tpAddCtrl->iEnterKeyResultValue;
					}
					else
					{
						iRet = RTN_SUCCESS;
					}
					break;
				}
				if(iAlphaInput == TRUE && iInputTimer != -1)
					mtiStopTimer(iInputTimer);

				if(iRtn != 0)
				{
					if(caLine)
						mtiFree(caLine);
					if(caInput)
						mtiFree(caInput);
					if(caOutput)
						mtiFree(caOutput);
					if(taFrmEle)
						mtiFree(taFrmEle);
					if(taCurEle)
						mtiFree(taCurEle);
					if(taNumEle)
						mtiFree(taNumEle);
					if(taAnsEle)
						mtiFree(taAnsEle);
					return iRtn;
				}
			}
			if(iKey != KEY_NONE)
			{
				dmsg("iKey[%d]",iKey);
				dmsg("iIsShown[%d]",iIsShown);
			}

			if (iIsShown)
			{
				continue;
			}
			switch (iKey)
			{
				case KEY_ENTER:
					dmsg("iKey = KEY_ENTER[%d]",iKey);
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
					if (0 < iLen)
					{
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

						if (iCnt == (iIndex + 1))
						{
							FINAL_DISP();
							mtiStopTimer(iTid);

							if(iAlphaInput == TRUE && iInputTimer != -1)
								mtiStopTimer(iInputTimer);

							if(caLine)
								mtiFree(caLine);
							if(caInput)
								mtiFree(caInput);
							if(caOutput)
								mtiFree(caOutput);
							if(taFrmEle)
								mtiFree(taFrmEle);
							if(taCurEle)
								mtiFree(taCurEle);
							if(taNumEle)
								mtiFree(taNumEle);
							if(taAnsEle)
								mtiFree(taAnsEle);
							return RTN_SUCCESS;
						}
						else
						{
							MZERO(caInput, DISP_MAX_COULUM + 1);
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

							if(iAlphaInput == TRUE && iInputTimer != -1)
								mtiStopTimer(iInputTimer);

							if(caLine)
								mtiFree(caLine);
							if(caInput)
								mtiFree(caInput);
							if(caOutput)
								mtiFree(caOutput);
							if(taFrmEle)
								mtiFree(taFrmEle);
							if(taCurEle)
								mtiFree(taCurEle);
							if(taNumEle)
								mtiFree(taNumEle);
							if(taAnsEle)
								mtiFree(taAnsEle);
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
							//SCPY(tpDispCont[iLineOrg[iIndex]].caInout, tpDispCont[iLineOrg[iIndex]].caDefValue, DISP_INPUT_MAX_COULUM);

							if (iCnt == (iIndex + 1))
							{
								FINAL_DISP_KEY();
								mtiStopTimer(iTid);

								if(iAlphaInput == TRUE && iInputTimer != -1)
									mtiStopTimer(iInputTimer);

								if(caLine)
									mtiFree(caLine);
								if(caInput)
									mtiFree(caInput);
								if(caOutput)
									mtiFree(caOutput);
								if(taFrmEle)
									mtiFree(taFrmEle);
								if(taCurEle)
									mtiFree(taCurEle);
								if(taNumEle)
									mtiFree(taNumEle);
								if(taAnsEle)
									mtiFree(taAnsEle);
								return RTN_SUCCESS;
							}
							else
							{
								MZERO(caInput, DISP_MAX_COULUM + 1);
								iLen = 0;
								iIndex++;
							}
						}
					}
					break;

				case KEY_CANCEL:
					dmsg("iKey = KEY_CANCEL[%d]",iKey);
					FINAL_DISP();
					mtiStopTimer(iTid);

					if(iAlphaInput == TRUE && iInputTimer != -1)
						mtiStopTimer(iInputTimer);

					if(caLine)
						mtiFree(caLine);
					if(caInput)
						mtiFree(caInput);
					if(caOutput)
						mtiFree(caOutput);
					if(taFrmEle)
						mtiFree(taFrmEle);
					if(taCurEle)
						mtiFree(taCurEle);
					if(taNumEle)
						mtiFree(taNumEle);
					if(taAnsEle)
						mtiFree(taAnsEle);
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
					dmsg("iKey = KEY_digit[%d]",iKey);
					if (iDefaultFlag)
					{
						mtiMemset(caInput, 0, DISP_MAX_COULUM + 1);
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
						iInputTimer = mtiStartTimer(TIMEOUT_1S);

					break;

				case KEY_DOT:
					dmsg("iKey = KEY_DOT[%d]",iKey);
					if (iANSCnt > 0 || iFreeInput == TRUE)
					{
						caInput[iLen++] = '.';
						iIsUpdate = TRUE;
					}
					if(iAlphaInput == TRUE)
						iInputTimer = mtiStartTimer(TIMEOUT_1S);
					break;
				case KEY_FUNC:
					dmsg("iKey = KEY_FUNC[%d]",iKey);
					if (iANSCnt > 0 || iFreeInput == TRUE)
					{
						caInput[iLen++] = '*';
						iIsUpdate = TRUE;
					}
					if(iAlphaInput == TRUE)
						iInputTimer = mtiStartTimer(TIMEOUT_1S);
					break;
				case KEY_BACK:
					dmsg("iKey = KEY_BACK[%d]",iKey);
					if (0 < iLen)
					{
						caInput[iLen--] = 0x00;
						dmsg("caInput[%s]",caInput);
					}
					iIsUpdate = TRUE;
					break;
			}

			while(1)
			{
				if (iIsUpdate)
				{
					mtiMemset(caOutput, 0, DISP_MAX_COULUM + 1);
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
						mtiMemset(caOutput, 0, DISP_INPUT_MAX_COULUM);
						mtiMemset(caOutput, 0x20, DISP_INPUT_MAX_COULUM);
						mtiMemcpy(caOutput, caInput, iLen);
					}
					dmsg("caOutput[%s]",caOutput);
					write_at(caOutput, mtiStrlen(DISP_MAX_COULUM, caOutput),1 , iLineIndex[iIndex] + 1 );
					iIsUpdate = FALSE;

					//ALPHA INPUT
					if(iAlphaInput == TRUE && ((iKey >= KEY_0 && iKey <= KEY_9)
							|| iKey == KEY_FUNC || iKey == KEY_DOT ))
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
							for(iIdx = 0; iIdx < 12; iIdx++)
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
										//dmsg("iLen[%d], iAlphaIdx[%d]", iLen, iAlphaIdx);
										caInput[iLen - 1] = tAlphaKeyMapList[iIdx].caAlphaVal[iAlphaIdx];
										iAlphaIdx++;
									}
								}
							}
							mtiResetTimer(iInputTimer);
							iIsUpdate = TRUE;
							continue;
						}


					}
				}
				break;
			}
			if(iAlphaInput == TRUE && iInputTimer != -1)
				mtiStopTimer(iInputTimer);
		}
	}

	FINAL_DISP();
	mtiStopTimer(iTid);

	if(iAlphaInput == TRUE && iInputTimer != -1)
		mtiStopTimer(iInputTimer);

	if(caLine)
		mtiFree(caLine);
	if(caInput)
		mtiFree(caInput);
	if(caOutput)
		mtiFree(caOutput);
	if(taFrmEle)
		mtiFree(taFrmEle);
	if(taCurEle)
		mtiFree(taCurEle);
	if(taNumEle)
		mtiFree(taNumEle);
	if(taAnsEle)
		mtiFree(taAnsEle);

	return iRet;
}


INT mtiShowMenuDisp(tDispContents *tpDispCont, INT iDispContCnt, INT iDefaultSelect, tAddCtrlContents *tpAddCtrl)
{
	INT i = 0, iLen = 0, iPos = 0, iCur = 0, iUpdate = 0, iRow = 0, iKey = 0, iPageCount = 0, iTid = 0, iNum = 0;

	INT iUseClearKey = 0;
	
	CHAR *cpCaption = NULL;
	CHAR caLine[100] = {0,};
	INT iMaxCoulum = DISP_MAX_COULUM;
	INT iMaxPageRow = PAGE_MAX;

	/////////////////////////////////////////////
	int            state = 0;
	int            currX = 0, currY = 0;
    char           buff[100];
    char           PenState[50];
	/////////////////////////////////////////////


	if (getIsADKGUI() == TRUE)
	{
		dmsg("GUI is Working...Not Allowed Console Access!");
		return RTN_ERROR;
		//return iAdkGuiShowMenuDisp(tpDispCont, iDispContCnt, iDefaultSelect, NULL);
	}

	iPageCount = iMaxPageRow;

	if (tpDispCont != NULL)
	{
		INIT_DISP();

		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i + iCur].caContents;
			iLen = SLEN(20, cpCaption);
			if (iLen > 0)
			{
				MZERO(caLine, sizeof(caLine));
				if (tpDispCont[i + iCur].iAlignment == DISP_ALIGN_CENTER)
				{
					mtiStringCenterAlign(cpCaption, caLine, iMaxCoulum);
				}
				else if (tpDispCont[i + iCur].iAlignment == DISP_ALIGN_RIGHT)
				{
					mtiStringRightAlign(cpCaption, caLine, iMaxCoulum);
				}
				else
				{
					MSPACE(caLine, iMaxCoulum);
					MCPY(caLine, cpCaption, iLen);
				}

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_TITLE))
				{
					mtiDisplayTextInverse(caLine, mtiStrlen(iMaxCoulum, caLine),1 ,1);
					iRow = 2;
				}
				else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
				{
					write_at(caLine, mtiStrlen(iMaxCoulum, caLine),1 ,2 );
					iRow = 3;
				}
			}

			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_USE_CLEAR_KEY))
			{
				iUseClearKey = 1;
			}	
		}

		iDispContCnt -= (iRow - 1);
		iPos = 0;
		if (iDispContCnt >= iMaxPageRow - 1)
		{
			iPageCount = iMaxPageRow - iRow;
		}
		else
		{
			iPageCount = iDispContCnt;
		}

		//dmsg("iDispContCnt[%d] iPageCount[%d] iRow[%d]", iDispContCnt, iPageCount, iRow);
		for (i = 0; i < iDefaultSelect; i++)
		{
			if (iPos != (iPageCount - 1))
			{
				iPos++;
			}
			else
			{
				if (iCur < iDispContCnt - iPageCount)
				{
					iCur++;
					iPos = iPageCount - 1;
				}
			}
		}
		iUpdate = TRUE;

		IsTouchSupport = FALSE;

		mtiDisplayMenuArrowBtn();

		if (tpAddCtrl != NULL)
		{
			mtiProcAddCtrl(tpAddCtrl);
			iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
		}
		else
		{
			iTid = mtiStartTimer(TIMEOUT_30S);
		}

		while (TRUE)
		{
			if (mtiIsTimeout(iTid))
			{
				FINAL_DISP();
				mtiStopTimer(iTid);
				return RTN_TIMEOUT;
			}

			if (iUpdate == TRUE)
			{
				for (i = iRow ; i < iPageCount + iRow; i++)
				{
					//dmsg("iRow[%d] iPageCount[%d] i[%d] iCur[%d]", iRow, iPageCount, i, iCur);
					cpCaption = tpDispCont[i + iCur - 1].caContents;
					iLen = SLEN(20, cpCaption);
					if (iLen > 0)
					{
						MZERO(caLine, sizeof(caLine));

						switch (iRow)
						{
							case 3:
								iNum = i+ iCur - 2;
								break;
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
						if(mtiGetHwType() == VX520)
							sprintf(caLine, "%d.%-19.19s", iNum, cpCaption);
						else
							sprintf(caLine, "%d.%-39.39s", iNum, cpCaption);

						switch (tpDispCont[i + iCur - 1].iAttribute)
						{
							case DISP_OPT_TITLE:
							case DISP_OPT_SUBTITLE:
								break;

							default:
								if (iPos + iRow == i)
								{
									//dmsg("1-i[%d]\tiPos[%d]\tiRow[%d]\tiCur[%d]",i,iPos,iRow,iCur);
									//dmsg("1-caLine[%s]",caLine);
									mtiDisplayTextInverse(caLine, mtiStrlen(iMaxCoulum, caLine),1 ,i + 1);
								}
								else
								{
									//dmsg("2-i[%d]\tiPos[%d]\tiRow[%d]\tiCur[%d]",i,iPos,iRow,iCur);
									//dmsg("2-caLine[%s]",caLine);
									write_at(caLine, mtiStrlen(iMaxCoulum, caLine),1 ,i + 1 );
								}
								break;
						}
					}
				}

				// Arrows are required only when there are more than the number of screen lines
				//if (tpAddCtrl == NULL)
				//dmsg("DISP_MAX_ROW[%d], iDispContCnt[%d], iRow[%d]", DISP_MAX_ROW, iDispContCnt, iRow);
				if ((tpAddCtrl == NULL) && (DISP_MAX_ROW <= (iDispContCnt + iRow)))
				{
					if (iCur > 0)
					{
						write_at(&UP_ARROW, 1, DISP_MAX_COULUM, DISP_MAX_ROW);
					}
					else
					{
						write_at(" ", 1, DISP_MAX_COULUM, DISP_MAX_ROW);
					}
					//dmsg("iCur[%d], iDispContCnt[%d], iPageCount[%d]", iCur, iDispContCnt, iPageCount);
					if (iCur < iDispContCnt - iPageCount)
					{
						write_at(&DOWN_ARROW, 1, 0, DISP_MAX_ROW);

					}
					else
					{
						write_at(" ", 1, 0, DISP_MAX_ROW);
					}
				}

				iUpdate = FALSE;
			}

			iKey = mtiGetKeyPress();
			if (iKey != KEY_NONE)
			{
				mtiResetTimer(iTid);
			}


			if (IsTouchSupport == TRUE)
			{
				lOSEvent = 0;
				lOSEvent = read_evt(EVT_TOUCH);

				if (lOSEvent)
				{
		   			state = get_touchscreen (&currX, &currY);
		   			//strncpy(PenState, state ? "Pen Down X=%-3d Y=%-3d   \n" : "Pen =Up   X=%-3d Y=%-3d   \n", 22);
		   			//sprintf(buff,PenState,currX,currY);
           			//dmsg("[%d]get_touchscreen XY[%s]", state, buff);

					if (state)
					{
						switch (mtiGetTouchKey(currX, currY))
						{
							case KEY_TOUCH_LEFT_BUTTON :
								dpt(); mtiSoundBeep(50, 1);
								dmsg("LEFT KEY TOUCH");
								iKey = KEY_UP;
								break;
							case KEY_TOUCH_RIGHT_BUTTON :
								dpt(); mtiSoundBeep(70, 1);
								dmsg("RIGHT KEY TOUCH");
								iKey = KEY_DOWN;
								break;
							default :
								iKey = KEY_NONE;
								break;
						}
					}

				}
			}
			

			if (tpAddCtrl != NULL)
			{
				if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
				{
					FINAL_DISP();
					mtiStopTimer(iTid);
					return RTN_SELECT_LEFT;
				}
				else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
				{
					FINAL_DISP();
					mtiStopTimer(iTid);
					return RTN_SELECT_RIGHT;
				}
				else if (iKey == tpAddCtrl->ucCancelKey)
				{
					INT iRet = 0;

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
					INT iRet = 0;
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
					dmsg("iDispContCnt[%d], iKey[%x], iKey - KEY_1[%d]", iDispContCnt, iKey, iKey - KEY_1);
					if ((iKey - KEY_1) < iDispContCnt)
					{
						FINAL_DISP();
						mtiStopTimer(iTid);
						return (iKey - KEY_1);
					}
					break;
				case KEY_0:
					if (9 < iDispContCnt)
					{
						FINAL_DISP();
						mtiStopTimer(iTid);
						return 9;
					}
					break;
				case KEY_DOWN:
					if (iPos != (iPageCount - 1))
					{
						iPos++;
						iUpdate = TRUE;
					}
					else
					{
						if (iCur < iDispContCnt - iPageCount)
						{
							iCur++;
							iPos = iPageCount - 1;
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
					FINAL_DISP();
					mtiStopTimer(iTid);
					return iPos + iCur;

				case KEY_CANCEL:
					FINAL_DISP();
					mtiStopTimer(iTid);
					return RTN_CANCEL;
			}
		}
	}

	FINAL_DISP();
	mtiStopTimer(iTid);
	return RTN_ERROR;
}

INT mtiShowBarCodeOnScreen(INT iBarCodeType, UCHAR *ucpBarCode, INT iBarCodeLen, tAddCtrlContents *tpAddCtrl)
{
	INT iRet = RTN_ERROR;

	if (iBarCodeType == DISP_WIDGET_BARCODE_QR)
	{
		//iRet = dispQRC((CHAR*)ucpBarCode, (INT)tpAddCtrl->ulTimeout);
		// @@EB QR 20190314
#if 1
		iRet = dispQRC((CHAR*)ucpBarCode, 1000);
#else
		iRet = dispQRC((CHAR*)ucpBarCode, TIMEOUT_240S);
#endif
		//
	}

	return iRet;
}
/****
static VOID dbgDispCts(tDispContents *tDispCts, INT iCtsCnt)
{
	INT iIdx;

	dmsg("----------DISP CTS DEBUG---------");
	for(iIdx = 0; iIdx < iCtsCnt; iIdx++)
	{
		dmsg("[%d] - caContents[%s]", iIdx, tDispCts[iIdx].caContents);
		dmsg("[%d] - iAlignment[%d]", iIdx, tDispCts[iIdx].iAlignment);
		dmsg("[%d] - iAttribute[%d]", iIdx, tDispCts[iIdx].iAttribute);
		dmsg("[%d] - caInout[%s]", iIdx, tDispCts[iIdx].caInout);
		dmsg("[%d] - caDefValue[%s]", iIdx, tDispCts[iIdx].caDefValue);
		dmsg("[%d] - caFormat[%s]", iIdx, tDispCts[iIdx].caFormat);
	}
	dmsg("---------------------------------");
}
****/
