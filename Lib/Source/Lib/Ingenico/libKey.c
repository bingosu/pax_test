
#include "libMtiCommonApi.h"
#include "apMem.h"

#define BEEP()						buzzer(10)

static Telium_File_t *mtKeyboard = NULL;
static INT g_iLibPosLinked = 0;

VOID mtiInitializeKeyboard()
{
	if (mtKeyboard != NULL)
	{
		mtiFinalizeKeyboard();
	}

	mtKeyboard = Telium_Fopen("KEYBOARD", "r*");
}

INT mtiIsValildKeyboard()
{
	if (mtKeyboard != NULL)
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
	switch (iKeyValue)
	{
		case T_SK10:
			return KEY_FUNC;

		case T_POINT:
			return KEY_DOT;

		case T_NUM0:
			return KEY_0;

		case T_NUM1:
			return KEY_1;

		case T_NUM2:
			return KEY_2;

		case T_NUM3:
			return KEY_3;

		case T_NUM4:
			return KEY_4;

		case T_NUM5:
			return KEY_5;

		case T_NUM6:
			return KEY_6;

		case T_NUM7:
			return KEY_7;

		case T_NUM8:
			return KEY_8;

		case T_NUM9:
			return KEY_9;

		case T_SKHAUT:
			return KEY_UP;

		case T_SKBAS:
			return KEY_DOWN;

		case T_SK1:
			return KEY_F1;

		case T_SK2:
			return KEY_F2;

		case T_SK3:
			return KEY_F3;

		case T_SK4:
			return KEY_F4;

		case T_VAL:
			return KEY_ENTER;

		case T_ANN:
			return KEY_CANCEL;

		case T_CORR:
			return KEY_BACK;

		case T_SKVAL:
			return KEY_CENTER;
	}

	return KEY_NONE;
}


INT mtiGetKeyPressWithPOS(INT nPOSUse)
{
	INT iRet = 0;
	INT nMsg = KEYBOARD;
	INT nStopMsg = 0;
	S_MESSAGE_IAM stMessage;

	if (1 == nPOSUse)
	{
		iGetAppTypeCode(NULL, NULL, &nStopMsg);
		nMsg |= MSG;
	}

	iRet = Telium_Ttestall(nMsg, 1);
	if (iRet & KEYBOARD)
	{
		//BEEP();
		return mtiSystemKeyValueToCommonKeyValue(Telium_Getchar());
	}
	else if (iRet & MSG)
	{
		dpt();
		if (1 == Read_Message(&stMessage, 0))
		{
			dpt();
			if (nStopMsg == stMessage.type)
				return KEY_CANCEL;
		}
	}

	return KEY_NONE;
}

INT mtiGetKeyPress()
{
	return mtiGetKeyPressWithPOS(g_iLibPosLinked);
}

INT mtiSetPosLinked(INT nPOSUse)
{
	dpt();
	g_iLibPosLinked = nPOSUse;
	return RTN_SUCCESS;
}

VOID mtiFinalizeKeyboard()
{
	if (mtKeyboard != NULL)
	{
		Telium_Fclose(mtKeyboard);
	}
	mtKeyboard = NULL;
}
