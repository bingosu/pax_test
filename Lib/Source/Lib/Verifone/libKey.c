#include <stdio.h>
#include "libMtiCommonApi.h"
#include "apMem.h"

#define BEEP()						buzzer(10)

typedef struct {
	INT iScanCode;
	INT iKeyVal;
} tVerifoneKeyMap;

extern char isActiveEVTchk;

tVerifoneKeyMap tVeri520KeyMap[] = {
		{0xB0, 		KEY_0},
		{0xB1, 		KEY_1},
		{0xB2, 		KEY_2},
		{0xB3, 		KEY_3},
		{0xB4, 		KEY_4},
		{0xB5, 		KEY_5},
		{0xB6, 		KEY_6},
		{0xB7, 		KEY_7},
		{0xB8, 		KEY_8},
		{0xB9, 		KEY_9},
		{0xFA, 		KEY_F1},
		{0xFB, 		KEY_F2},
		{0xFC, 		KEY_F3},
		{0xFD, 		KEY_F4},
		{0xA3, 		KEY_DOT},
		{0xAA, 		KEY_FUNC},
		{0xE4, 		KEY_UP},
		{0xE1, 		KEY_DOWN},
		{0x9B, 		KEY_CANCEL},
		{0x8D, 		KEY_ENTER},
		{0x88, 		KEY_BACK},
		{0x8F,		KEY_CENTER},
		{KEY_NONE,	KEY_NONE},
};
tVerifoneKeyMap tVeri675KeyMap[] = {
		{0xB0, 		KEY_0},
		{0xB1, 		KEY_1},
		{0xB2, 		KEY_2},
		{0xB3, 		KEY_3},
		{0xB4, 		KEY_4},
		{0xB5, 		KEY_5},
		{0xB6, 		KEY_6},
		{0xB7, 		KEY_7},
		{0xB8, 		KEY_8},
		{0xB9, 		KEY_9},
		{0xE1, 		KEY_F1},
		{0xE2, 		KEY_F2},
		{0xE3, 		KEY_F3},
		{0xE4, 		KEY_F4},
		{0xA3, 		KEY_DOT},
		{0xAA, 		KEY_FUNC},
		{0xDA, 		KEY_UP},
		{0xDB, 		KEY_DOWN},
		{0x9B, 		KEY_CANCEL},
		{0x8D, 		KEY_ENTER},
		{0xDE, 		KEY_CENTER},
		{0x88, 		KEY_BACK},
		{KEY_NONE,	KEY_NONE},
};

extern INT mtDisplay;

extern INT g_iPosLinked = 0;

VOID keyBufferClear()
{
	CHAR caKeyBuff[512] = { 0, };
	INT iKeyBuffLen = 0;

	iKeyBuffLen = read(mtDisplay, caKeyBuff, sizeof(caKeyBuff));

/*
	if(iKeyBuffLen > 0)
	{
		dbuf("KEY INPUT FLUSH DBG", caKeyBuff, iKeyBuffLen);
	}
*/
}

VOID mtiInitializeKeyboard()
{
	if (mtDisplay == 0)
	{
		mtDisplay = open(DEV_CONSOLE, O_RDWR);
	}

	//Before Key input, Nedd to Clear Key Device buffer.
	keyBufferClear();
}

INT mtiIsValildKeyboard()
{
	if (mtDisplay != 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

INT mtiSystemKeyValueToCommonKeyValue(INT iKeyValue)
{
	INT iIdx = 0;
	tVerifoneKeyMap *tpVeriKeyMap = NULL;

	if(mtiGetHwType() == VX520)
		tpVeriKeyMap = tVeri520KeyMap;
	else
		tpVeriKeyMap = tVeri675KeyMap;

	while(tpVeriKeyMap[iIdx].iKeyVal != KEY_NONE)
	{
		if(tpVeriKeyMap[iIdx].iScanCode == iKeyValue )
			return tpVeriKeyMap[iIdx].iKeyVal;
		else
			iIdx++;
	}

	return KEY_NONE;
}

BOOL iCheckPOSStopMessage()
{
	INT nMsg;
	INT nRecvMsg;

	iGetAppTypeCode(NULL, NULL, &nMsg);

	nRecvMsg = read_user_event();

	if (nMsg == nRecvMsg)
		return TRUE;

	return FALSE;
}

INT mtiGetKeyPressWithPOS(INT nPOSUse)
{
	CHAR caKeyBuff[512] = { 0, };
	INT iKeyBuffLen = 0;
	LONG lEvents = 0;

	if (1 == nPOSUse)
	{
		if (TRUE == iCheckPOSStopMessage())
			return KEY_CANCEL;
	}

	if (isActiveEVTchk)
		lEvents = read_evt(EVT_KBD);
	else
		lEvents = 1L;

	//if (events & EVT_KBD){
	if (lEvents)
	{
		iKeyBuffLen = read(mtDisplay, caKeyBuff, sizeof(caKeyBuff));

		//dmsg("read Key buffer size[%d]", iKeyBuffLen)
		//dbuf("mtiGetKeyPressWithPOS", caKeyBuff, iKeyBuffLen);
		return mtiSystemKeyValueToCommonKeyValue((INT)caKeyBuff[iKeyBuffLen - 1]);
	}

	return KEY_NONE;
}

INT mtiGetKeyPress()
{
	return mtiGetKeyPressWithPOS(g_iPosLinked);
}

INT mtiSetPosLinked(INT nPOSUse)
{
	g_iPosLinked = nPOSUse;
}

INT mtiForceReadKeyData()
{
	CHAR caKeyBuff[512] = { 0, };
	INT iKeyBuffLen = 0;

	iKeyBuffLen = read(mtDisplay, caKeyBuff, sizeof(caKeyBuff));

	if (0 < iKeyBuffLen)
		return mtiSystemKeyValueToCommonKeyValue((INT)caKeyBuff[iKeyBuffLen - 1]);
	
	return KEY_NONE;
}

VOID mtiFinalizeKeyboard()
{

}
