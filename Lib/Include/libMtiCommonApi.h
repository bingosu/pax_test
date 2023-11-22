#ifndef INC_LIB_MTI_COMMON_API_H_
#define INC_LIB_MTI_COMMON_API_H_

#ifndef __NOT_COMMON_INCLUDE__
#ifdef __INGENICO_SRC__
#include <sdk_tplus.h>
#include <basearm_def.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "LinkLayer.h"
#include "TlvTree.h"
#include "libDebug.h"
#include "oem_backlight_def.h"
#include "GL_GraphicLib.h" //@@WAY

extern T_GL_HGRAPHIC_LIB TGoal; //@@WAY
extern T_GL_HWIDGET TScreen, TLayout; //@@WAY

#elif (defined __VERIFONE_SRC__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <svc.h>
#include <eoslog.h>
#include <log/liblog.h>
#include <sys/stat.h>
#include "libDebug.h"
#endif

#define TRUE		1
#define FALSE		0

#define VX520		1
#define VX675		2
#define VX520C		3
#define C680		4
#define VX675_CONSOLE_FONT_FILE		"F:15/FONTS/MTI16X16.VFT"

typedef enum {
	EDC_MODEL_VX520 = 1,
	EDC_MODEL_VX675,
	EDC_MODEL_ICT220,
	EDC_MODEL_ICT250,
	EDC_MODEL_IWL220,
	EDC_MODEL_VX520C,
	EDC_MODEL_C680,
	EDC_MODEL_M2500,
	EDC_MODEL_M3500,
	EDC_MODEL_UNKNOWN,
} eEDCModelName;


#endif

/************************************************************************************************************
	APPLICATION INFO__NOT_COMMON_INCLUDE__h4
****************************************** ******************************************************************/
#define REL_NUM							"31"			//"nn" of 6 Character (YYMMnn)		// @@EB MARK

/*****************************************************************************************************
	COMMON DEFINITION
************************************************************************************************************/

#define BIG_ENDIAN_		1
#define LITTLE_ENDIAN_	2

#ifdef __INGENICO_SRC__
typedef void					VOID;
typedef unsigned char 			BOOL;

typedef int						INT;
typedef char					CHAR;
typedef long					LONG;
typedef float					FLOAT;
typedef short					SHORT;
typedef long long				LLONG;

typedef unsigned char			UCHAR;
typedef unsigned int			UINT;
typedef unsigned long			ULONG;
typedef unsigned short			USHORT;
typedef unsigned long long		ULLONG;

typedef unsigned char			BYTE;

#define __PACKED					__attribute__((packed))
#define __PACKED_S
#elif (defined __VERIFONE_SRC__)
#undef NULL
#define NULL			0

typedef void			VOID;

typedef int				INT;
typedef char			CHAR;
typedef long			LONG;
typedef long long		LLONG;
typedef float			FLOAT;
typedef double			DOUBLE;
typedef short			SHORT;
typedef unsigned char 	BOOL;

typedef unsigned char	UCHAR;
typedef unsigned int	UINT;
typedef unsigned long	ULONG;
typedef unsigned short	USHORT;
typedef unsigned long long	ULLONG;

typedef unsigned char			BYTE;

//Structure Alignement
#define __PACKED
#define __PACKED_S					__packed


typedef struct _NetworkStat{
	BOOL ethLink;
	INT gprsRssiPersent;
	INT iCommType;
	CHAR caWifiSSID[50];
} networkStat;
#endif

//HW SUPPORT
#define HW_SUPPORT_CTLS		(INT) (1 << 0)


//Macro Functions

//Get Size of array
#define DIM_SIZEOF(X)		\
		(sizeof(X) / sizeof(X[0]))

//Get Size of Structure Member Variable.
#define M_SIZEOF(structure, member) \
		sizeof(((structure *)0)->member)

//Memory Zero Set
#define	MZERO(X, Y)						mtiMemset(X, 0, Y)

//Memory Space Set
#define	MSPACE(X, Y)					mtiMemset(X, 0x20, Y)

#define MEM_ALLOC(X)					malloc(X)
#define MEM_FREE(X)						free(X)


//Definition Marco
#define	SCPY(X, Y, Z)					mtiStrcpy(X, Y, Z)
#define	MCPY(X, Y, Z)					mtiMemcpy(X, Y, Z)
#define	MCMP(X, Y, Z)					mtiMemcmp(X, Y, Z)
#define	SLEN(Z, X)						mtiStrlen(Z, X)

#define ADD_BIT(WORD, BIT)				WORD |= BIT
#define CHK_BIT(WORD, BIT)				((WORD & BIT) == BIT)
#define CLR_BIT(WORD, BIT)				WORD &= ~(1 << BIT)

#define RTN_COMM_CONNECTION_FAULT		-2000
#define RTN_COMM_LINE_BUSY				-2001
#define RTN_COMM_NO_DIALTONE			-2002
#define RTN_COMM_PSTN_CABLE_DISCONN		-2003

#define RTN_COMM_MSG_PARSE_ERROR		-2010
#define RTN_COMM_SEND_ERROR				-2011
#define RTN_COMM_TRY_RTRAN				-2012
#define RTN_COMM_MANUAL_DISCONNECT		-2013
#define RTN_COMM_ALREADY_DISCONNECT		-2014
#define RTN_COMM_MAC_COMP_ERROR			-2015
#define RTN_COMM_NET_FAIL				-2016
#define RTN_COMM_NET_ETH_UNLINK			-2017
#define RTN_COMM_REVERSAL_COMPLETE		-2018
#define RTN_COMM_USER_CANCEL			-2019
#define RTN_COMM_TIMEOUT				-2020
#define RTN_COMM_MANDATORY_ERROR		-2021

// @@EB AUTO REVERSAL
#define RTN_COMM_MAC_COMP_ERROR_REVERSAL	-2022
#define RTN_COMM_MSG_PARSE_ERROR_REVERSAL	-2023
//

#define RTN_COMM_GPRS_NO_APN			-2200
#define RTN_COMM_GPRS_REQ_PIN			-2201
#define RTN_COMM_GPRS_REQ_PUK			-2202
#define RTN_COMM_GPRS_SIM_PROBLEM		-2203
#define RTN_COMM_GPRS_LOW_BATTERY		-2204
#define RTN_COMM_GPRS_INTERNAL_ERR		-2205
#define RTN_COMM_GPRS_GSM_PWR_ERR		-2206
#define RTN_COMM_GPRS_PIN_VERIFY_ERR	-2207
#define RTN_COMM_GPRS_INUSE				-2208
#define RTN_COMM_GPRS_DETATCH			-2209


#define RTN_IC_CTRL_NOT_INIT_CTRL		-10000
#define RTN_IC_CTRL_CARD_REMOVED		-10001
#define RTN_IC_CTRL_CARD_ERROR			-10002
#define RTN_IC_CTRL_CARD_NOT_RESPONSE	-10003
#define RTN_IC_CTRL_UNKNOWN_ERROR		-10099

#define RTN_CTLS_CTRL_NOT_INIT_CTRL		-15000
#define RTN_CTLS_CTRL_CARD_ERROR		-15001
#define RTN_CTLS_CTRL_CARD_NOT_RESPONSE	-15002
#define RTN_CTLS_CTRL_UNKNOWN_ERROR		-15099

#define RTN_SELECT_LEFT					-1000
#define RTN_SELECT_RIGHT				-1001

#define RTN_NOT_FOUND_RECORD			-3000
#define RTN_NOT_FOUND_PARAMETER			-3001
#define RTN_ERROR_ALREADY_DONE			-3002

#define RTN_XLS_NEED_AUTO_CANCEL		-4000
#define RTN_XLS_GETCONFIG_ERROR			-4001
#define RTN_XLS_GETENTITY_ERROR			-4002

// shkim for Auto settlement of Prepaid
#define RTN_SETTLE_NO_RECORD			0
#define RTN_SETTLE_YET_PERIOD			0
#define RTN_SETTLE_NOT_APPLICANT		-4001
#define RTN_SETTLE_UNDEFINED_ERROR		-4099

#define RTN_DENIAL						-99
#define RTN_SKIP						-98

// By Allen for preventing display "TRANSACTION CANCELED" when canceled in sub menu 
#define RTN_MENU_CANCEL				-4

#define RTN_CLEAR_BACK					-3

#define RTN_TIMEOUT						-2
#define RTN_CANCEL						-1
#define RTN_ERROR						0
#define RTN_SUCCESS						1

/************************************************************************************************************
	MEM
************************************************************************************************************/

#define MAX_ALLOC_COUNT											   	256
#define MAX_ALLOC_SIZE												65535

#define RTN_UNKNOWN_SIZE											-1
#define RTN_UNKNOWN_ID												-1

#define VTYPE_STRING												0
#define VTYPE_INT													1
#define VTYPE_LONG													2
#define VTYPE_FIXED_SIZE											3
#define VTYPE_MAP													4
#define VTYPE_LONGLONG												5

#define INVALID_HANDLER												-1

typedef struct _tMtiMem
{
	VOID *pAllocAddr;
	INT  iSize;
	INT  iType;
	INT  iID;
	INT  iIndex;
	VOID *tpNextNode;
} tMtiMem;

typedef struct _tMtiMap
{
	tMtiMem *tpFirstNode;
	INT iMapCount;
} tMtiMap;

#ifdef __cplusplus
extern "C" {
#endif

VOID MemInit(tMtiMap *pMap);
VOID MemClear(tMtiMap *pMap);
VOID MemDeinit(tMtiMap *pMap);
VOID *MemAlloc(tMtiMap *pMap, INT iSize, INT iID, INT iType);
VOID *MemReference(tMtiMap *pMap, INT iID, INT *iAllocSize, INT *iAllocType);
VOID MemFree(tMtiMap *pMap, INT iID);
INT MemAllocSize(tMtiMap *pMap, INT iID);
INT MemType(tMtiMap *pMap, INT iID);
INT MemCheck(tMtiMap *pMap, INT iIndex);
tMtiMem *MemGetNodeInfo(tMtiMap *pMap, INT iID);

#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	COMMON
************************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***
 *
 * @param iSize
 * @return
 */
VOID *mtiMalloc(INT iSize);

/***
 *
 * @param pPtr
 * @return
 */
VOID mtiFree(VOID *pPtr);

/***
 *
 * @param pDest
 * @param pSrc
 * @param iMaxLength
 * @return
 */
INT mtiStrcpy(VOID *pDest, VOID *pSrc, INT iMaxLength);

/***
 *
 * @param iMaxLength
 * @param szpSrc
 * @return
 */
INT mtiStrlen(INT iMaxLength, VOID *szpSrc);

/***
 *
 * @param szpDest
 * @param cValue
 * @param iMaxLength
 * @return
 */
VOID mtiMemset(VOID *szpDest, INT cValue, INT iMaxLength);

/***
 *
 * @param szpDest
 * @param szpSrc
 * @param iMaxLength
 * @return
 */
INT mtiMemcpy(VOID *szpDest, VOID *szpSrc, INT iMaxLength);

/***
 *
 * @param szpDest
 * @param szpSrc
 * @param iMaxLength
 * @return
 */
INT mtiMemcmp(VOID *szpDest, VOID *szpSrc, INT iMaxLength);

/***
 *
 * @param ucpBcd
 * @param ipOut
 * @param iLength
 * @return
 */
VOID mtiBcdToInt( const UCHAR *ucpBcd, INT *ipOut, const UINT iLength);

/***
 *
 * @param ipOut
 * @param ucpBcd
 * @param iLength
 * @return
 */
VOID mtiIntToBcd( const INT *ipOut, UCHAR *ucpBcd, const UINT iLength);

/***
 *
 * @param ucpDest
 * @param ucpSrc
 * @param iSize
 * @return
 */
INT mtiAtoh(UCHAR *ucpDest, UCHAR *ucpSrc, UINT iSize);

/***
 *
 * @param ucpDest
 * @param ucpSrc
 * @param iSize
 * @return
 */
INT mtiHtoa(UCHAR *ucpDest, UCHAR *ucpSrc, UINT iSize);

/***
 *
 * @param iMaxLength
 * @param ucpString
 * @return
 */
ULLONG mtiAtoll(UINT iMaxLength, UCHAR *ucpString);

/***
 *
 * @param iMaxLength
 * @param ucpString
 * @return
 */
INT mtiAtoi( UINT iMaxLength, UCHAR *ucpString);

/***
 *
 * @param iMaxLength
 * @param ucpString
 * @return
 */
UINT mtiAtoUi(UINT iMaxLength, UCHAR *ucpString);

/***
 *
 * @param iValue
 * @param cDefaultChar
 * @param iBufLength
 * @param ucpOutBuf
 * @return
 */
VOID mtiItoa(INT iValue, const CHAR cDefaultChar, const INT iBufLength, UCHAR *ucpOutBuf);

/***
 *
 * @param lValue
 * @param cDefaultChar
 * @param iBufLength
 * @param ucpOutBuf
 * @return
 */
VOID mtiLtoa(ULONG lValue, const CHAR cDefaultChar, const INT iBufLength, UCHAR *ucpOutBuf);

/***
 *
 *
 * @param llValue
 * @param cDefaultChar
 * @param iBufLength
 * @param ucpOutBuf
 * @return
 */
VOID mtiLLtoa(LLONG llValue, const CHAR cDefaultChar, const INT iBufLength, UCHAR *ucpOutBuf);

/***
 *
 * @param cpIn
 * @param cpOut
 * @param iMaxLen
 * @return
 */
VOID mtiStringCenterAlign(CHAR *cpIn, CHAR *cpOut, INT iMaxLen);

/***
 *
 * @param cpIn
 * @param cpOut
 * @param iMaxLen
 * @return
 */
VOID mtiStringRightAlign(CHAR *cpIn, CHAR *cpOut, INT iMaxLen);

/***
 *
 * @param cpOut
 * @param cpIn
 * @param iPaddingSize
 * @return
 */
INT mtiPaddingLeft(CHAR *cpOut, CHAR *cpIn, INT iPaddingSize, CHAR cFillChar);

/***
 *
 * @param cpOut
 * @param cpIn
 * @param iPaddingSize
 * @param cFillChar
 * @return
 */
INT mtiPaddingRight(CHAR *cpOut, CHAR *cpIn, INT iPaddingSize, CHAR cFillChar);


//SECURITY
/***
 *
 * @param ucpSrc
 * @param iSrcLen
 * @return MD5 String
 */
CHAR* mtiMD5Gen(UCHAR *ucpSrc, INT iSrcLen);

/***
 *
 * @param caFilePath
 */
CHAR* mtiMD5GenFile(CHAR *cpFilePath);

/***
 *
 * @return VX520, VX675
 */



/***
 *
 * @return VX520, VX675
 */
INT mtiSHA256(UCHAR *ucpSrcData, INT iSrcLen, UCHAR *ucpRamdomData);


/***
 *
 * @param tpMap
 * @return
 */
VOID mtiMapInit(tMtiMap *tpMap);

/***
 *
 * @param tpMap
 * @return
 */
VOID mtiMapClear(tMtiMap *tpMap);

/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
VOID mtiMapRemove(tMtiMap *tpMap, INT iID);

/***
 *
 * @param tpMap
 * @param iID
 * @param iLength
 * @return
 */
UCHAR *mtiMapCreateBytes(tMtiMap *tpMap, INT iID, INT iLength);

/***
 *
 * @param tpMap
 * @param iID
 * @param ucpValue
 * @param iLength
 * @return
 */
VOID mtiMapPutBytes(tMtiMap *tpMap, INT iID, UCHAR *ucpValue, INT iLength);

/***
 *
 * @param tpMap
 * @param iID
 * @param iLength
 * @return
 */
UCHAR *mtiMapGetBytes(tMtiMap *tpMap, INT iID, INT *iLength);

/***
 *
 * @param tpMap
 * @param iID
 * @param iLengn
 * @return
 */
CHAR *mtiMapCreateString(tMtiMap *tpMap, INT iID, INT iLength);

/***
 *
 * @param tpMap
 * @param iID
 * @param cpValue
 * @return
 */
VOID mtiMapPutString(tMtiMap *tpMap, INT iID, CHAR *cpValue);
VOID mtiMapPutStringWithLength(tMtiMap *tpMap, INT iID, CHAR *cpValue, INT iLength);

/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
CHAR *mtiMapGetString(tMtiMap *tpMap, INT iID);

/***
 *
 * @param tpMap
 * @param iID
 * @param iValue
 * @return
 */
VOID mtiMapPutInt(tMtiMap *tpMap, INT iID, INT iValue);

/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
INT mtiMapGetInt(tMtiMap *tpMap, INT iID);

/***
 *
 * @param tpMap
 * @param iID
 * @param lValue
 * @return
 */
VOID mtiMapPutLong(tMtiMap *tpMap, INT iID, LONG lValue);

/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
LONG mtiMapGetLong(tMtiMap *tpMap, INT iID);


/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
VOID mtiMapPutLLong(tMtiMap *tpMap, INT iID, LLONG llValue);


/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
LLONG mtiMapGetLLong(tMtiMap *tpMap, INT iID);

LLONG mtiMapGetLLongAndExist(tMtiMap *tpMap, INT iID, BOOL *pbIsExist);

/***
 *
 * @param tpMap
 * @param iID
 * @param pDestMap
 * @return
 */
VOID mtiMapPutMap(tMtiMap *tpMap, INT iID, tMtiMap *pDestMap);

/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
tMtiMap *mtiMapCreateMap(tMtiMap *tpMap, INT iID);

/***
 *
 * @param tpMap
 * @param iID
 * @return
 */
tMtiMap *mtiMapGetMap(tMtiMap *tpMap, INT iID);

/***
 *
 * @param pSrcMap
 * @param pDestMap
 * @return
 */
INT mtiMapClone(tMtiMap *pDestMap, tMtiMap *  pSrcMap);

/***
 *
 * @param piaArray
 * @param iStart
 * @param iEnd
 * @return
 */
VOID mtiIntArraySortQuick(INT *piaArray, INT iStart, INT iEnd);

VOID quickSort(INT *arr, INT elements);

/***
 *
 * @param tpMap
 * @return
 */
VOID mtiMapSort(tMtiMap *tpMap);

#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	DISPLAY
************************************************************************************************************/
#ifdef __INGENICO_SRC__
#define DISP_INPUT_MAX_COULUM			20
#define DISP_MAX_COULUM					16
#define DISP_SMALL_MAX_COULUM			20
#define DISP_MAX_ROW					6
#endif

#ifdef __VERIFONE_SRC__
#if 0
#if VX520
#define DISP_INPUT_MAX_COULUM			21
#define DISP_MAX_COULUM					21
#define DISP_MAX_ROW					8
#elif VX675
#define DISP_INPUT_MAX_COULUM			40
#define DISP_MAX_COULUM					40
#define DISP_MAX_ROW					15
#endif
#endif
#ifdef SET_DISPLAY_INFO
INT g_DISP_INPUT_MAX_COULUM;
INT g_DISP_MAX_COULUM;
INT g_DIS_MAX_ROW;
#else
extern INT g_DISP_INPUT_MAX_COULUM;
extern INT g_DISP_MAX_COULUM;
extern INT g_DIS_MAX_ROW;
#endif

#define DISP_INPUT_MAX_COULUM			g_DISP_INPUT_MAX_COULUM
#define DISP_MAX_COULUM					g_DISP_MAX_COULUM
#define DISP_MAX_ROW					g_DIS_MAX_ROW
#define DISP_SMALL_MAX_COULUM			g_DISP_MAX_COULUM
#endif



#define DISP_OPT_TITLE					0x00000001
#define DISP_OPT_SUBTITLE				0x00000002
#define DISP_OPT_INPUT					0x00000004
#define DISP_OPT_NORMAL					0x00000008
#define DISP_OPT_SMALL_FONT				0x00000010
#define DISP_OPT_ENABLE_SKIP			0x00000020
#define DISP_OPT_AFTER_CHECK			0x00000040
#define DISP_OPT_FORWARD_SECURE			0x00000080
#define DISP_OPT_CHECK_MINIMUM_LENGTH	0x00000100
#define DISP_OPT_ALPHA					0x00000200
#define DISP_OPT_EXPIRE_DATE			0x00000400
#define DISP_OPT_CHECK_NO_INPUT			0x00000800
#define DISP_OPT_USE_CLEAR_KEY			0x00001000
#define DISP_OPT_NOTUSED_TIMEOUT		0x00002000
#define DISP_OPT_TIMEOUT_3S				0x00004000
#define DISP_OPT_CORRECT_MAX_LENGTH		0x00008000
#define DISP_OPT_TIMEOUT_10S			0x00010000
#define DISP_OPT_NO_CHECK_NO_INPUT		0x00020000

#define DISP_ALIGN_LEFT					100
#define DISP_ALIGN_CENTER				101
#define DISP_ALIGN_RIGHT				102

#define DISP_WIDGET_BARCODE_QR			0x00000001
#define DISP_WIDGET_BARCODE_CODE39		0x00000002
#define DISP_WIDGET_BARCODE_CODE128		0x00000004

#ifdef __INGENICO_SRC__
typedef struct _tDspCont {
	CHAR caContents[DISP_MAX_COULUM + 1];
	INT iAlignment;
	INT iAttribute;
	CHAR caInout[DISP_SMALL_MAX_COULUM + 1];
	CHAR caDefValue[DISP_SMALL_MAX_COULUM + 1];
	CHAR caFormat[DISP_SMALL_MAX_COULUM + 1];
} tDispContents;

typedef struct _tBtnCont {
	CHAR caContents[DISP_MAX_COULUM + 1];
	INT iAlignment;
	UCHAR cRegKey;
} tButtonContents;
#endif

#ifdef __VERIFONE_SRC__
typedef struct _tDspCont {
	CHAR caContents[40 + 1];
	INT iAlignment;
	INT iAttribute;
	CHAR caInout[40 + 1];
	CHAR caDefValue[40 + 1];
	CHAR caFormat[40 + 1];
} tDispContents;

typedef struct _tBtnCont {
	CHAR caContents[40 + 1];
	INT iAlignment;
	UCHAR cRegKey;
} tButtonContents;
#endif

typedef struct _tAddCtlCont {
	tButtonContents tBtnLeft;
	tButtonContents tBtnRight;
	ULONG ulTimeout;
	UCHAR ucCancelKey;
	INT iCancelKeyResultValue;
	UCHAR ucEnterKey;
	INT iEnterKeyResultValue;
} tAddCtrlContents;


/***
 *
 * @return
 */
VOID mtiSetDisplayInfo();

/***
 *
 * @return
 */
VOID mtiClearDisp();

/***
 *
 * @param tpDispCont
 * @param iDispContCnt
 * @param tpAddCtrl
 * @return
 */
INT mtiShowDialogDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl);

/***
 *
 * @param tpDispCont
 * @param iDispContCnt
 * @param iDefaultSelect
 * @param tpAddCtrl
 * @return
 */
INT mtiShowMenuDisp(tDispContents *tpDispCont, INT iDispContCnt, INT iDefaultSelect, tAddCtrlContents *tpAddCtrl);

/***
 *
 * @param tpDispCont
 * @param iDispContCnt
 * @param cpFormat
 * @param cpDefaultValue
 * @param iInputAlign
 * @param tpAddCtrl
 * @return
 */
INT mtiShowInputDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl);

/***
 *
 * @param tpDispCont
 * @param iDispContCnt
 * @param iSecLength
 * @param tpAddCtrl
 * @return
 */
INT mtiShowSecureInput(tDispContents *tpDispCont, INT iDispContCnt, INT iSecLength, tAddCtrlContents *tpAddCtrl);


/***
 *
 * @param iBarCodeType
 * @param ucpBarCode
 * @param iBarCodeLen
 * @param tpAddCtrl
 * @return
 */
INT mtiShowBarCodeOnScreen(INT iBarCodeType, UCHAR *ucpBarCode, INT iBarCodeLen, tAddCtrlContents *tpAddCtrl);
INT mtiShowBarCodeOnPrint(INT iBarCodeType, UCHAR *ucpBarCode, INT iBarCodeLen);
INT mtiShowBarCodeOnScreenTest();



/************************************************************************************************************
	KEYBOARD
************************************************************************************************************/

#define KEY_NONE						0xFF

#define KEY_0							0x30
#define KEY_1							0x31
#define KEY_2							0x32
#define KEY_3							0x33
#define KEY_4							0x34
#define KEY_5							0x35
#define KEY_6							0x36
#define KEY_7							0x37
#define KEY_8							0x38
#define KEY_9							0x39
#define KEY_DOT							0x2E
#define KEY_FUNC						0x28

#define KEY_F1							0xF1
#define KEY_F2							0xF2
#define KEY_F3							0xF3
#define KEY_F4							0xF4

#define KEY_UP							0xA1
#define KEY_DOWN						0xA2
#define KEY_CENTER						0xA3

#define KEY_CANCEL						0x1B
#define KEY_ENTER						0x0D
#define KEY_BACK						0x08

#ifdef __cplusplus
extern "C" {
#endif

/***
 *
 * @return
 */
VOID mtiInitializeKeyboard();

/***
 *
 * @return
 */
INT mtiIsValildKeyboard();

/***
 *
 * @return
 */
// FOR POS
INT mtiGetKeyPressWithPOS(INT nPOSUse);


INT mtiGetKeyPress();


INT mtiForceReadKeyData();


/***
 *
 * @param iKeyValue
 * @return
 */
INT mtiSystemKeyValueToCommonKeyValue(INT iKeyValue);

/***
 *
 * @return
 */
VOID mtiFinalizeKeyboard();

/***
 *
 * @return
 */
VOID keyBufferClear();

/***
 *
 * @param nPOSUse
 * @return
 */
INT mtiSetPosLinked(INT nPOSUse);

#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	TIMER
************************************************************************************************************/

#define TIMEOUT_240S                    240 * 1000
#define TIMEOUT_120S					120 * 1000	// @@WAY, 20190506 NETWORK
#define TIMEOUT_30S						30 * 1000
#define TIMEOUT_15S						15 * 1000
#define TIMEOUT_10S						10 * 1000
#define TIMEOUT_5S						5 * 1000
#define TIMEOUT_3S						3 * 1000
#define TIMEOUT_2S						2 * 1000
#define TIMEOUT_1S						1 * 1000

#ifdef __cplusplus
extern "C" {
#endif

/***
 *
 * @param ulTimeout
 * @return
 */
INT mtiStartTimer(ULONG ulTimeout);

/***
 *
 * @param iTimerID
 * @return
 */
VOID mtiResetTimer(INT iTimerID);

/***
 *
 * @param iTimerID
 * @return
 */
INT mtiIsTimeout(INT iTimerID);

/***
 *
 * @param iTimerID
 * @return
 */
VOID mtiStopTimer(INT iTimerID);

/***
 *
 * @param ulTimeout
 * @return
 */
VOID mtiSleep(ULONG ulTimeout);

/***
 *
 * @param iTimerID
 * @return
 */
VOID mtiResetTimer(INT iTimerID);

/***
 *
 * @return
 */
ULONG mtiGetTickCount();

#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	MAGETIC STRIPE
************************************************************************************************************/

#define TRACK_MAX_LENGTH			128

#define READ_TRACK1_SUCCESS			0x01
#define READ_TRACK2_SUCCESS			0x02
#define READ_TRACK3_SUCCESS			0x04

#define READ_TRACK_ERROR			0x80

typedef struct _tMagneticStripeContents
{
	CHAR caTrack1[TRACK_MAX_LENGTH];
	UCHAR ucTrack1Length;
	CHAR caTrack2[TRACK_MAX_LENGTH];
	UCHAR ucTrack2Length;
	CHAR caTrack3[TRACK_MAX_LENGTH];
	UCHAR ucTrack3Length;
} tMagneticStripeContents;

#ifdef __cplusplus
extern "C" {
#endif

/***
 *
 * @param iAttribute
 * @return
 */
VOID mtiInitializeMagneticStripe(INT iAttribute);

/***
 *
 * @param tpMSContents
 * @return
 */
INT mtiReadMagneticStripe(tMagneticStripeContents *tpMSContents);

/***
 *
 * @return
 */
VOID mtiFinalizeMagneticStripe();

#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	FILE SYSTEM
************************************************************************************************************/

#define MAX_FILE_PATH				255

#define FILE_WRITE_ALL				1		//Write to all record.
#define FILE_WRITE_SOME				2		//Write to modification record.

/***
 *
 * @return
 */
VOID mtiInitializeFileSystem();

/***
 *
 * @param cpFilename
 * @return
 */
INT mtiGetFileSize(CHAR *cpFilename);

/***
 *
 * @param cpFilename
 * @param ucpOut
 * @param iOutSize
 * @return
 */
INT mtiLoadFile(CHAR *cpFilename, UCHAR *ucpOut, INT *iOutSize);

/***
 *
 * @param cpFilename
 * @param ucpIn
 * @param iInSize
 * @return
 */
INT mtiSaveFile(CHAR *cpFilename, UCHAR *ucpIn, INT *iInSize);

/***
 *
 * @param cpFilename
 * @return
 */
INT mtiFileDelete(CHAR *cpFilename);

/***
 *
 * @param cpFilename
 * @return
 */
INT mtiLoadRecordCount(CHAR *cpFilename);

/***
 *
 * @param tpAllMap
 * @param cpFilename
 * @return
 */
INT mtiFileToRecordMap(CHAR *cpFilename, tMtiMap *tpAllMap);


/***
 *
 * @param tpAllMap
 * @param cpFilename
 * @return
 */
INT mtiSaveRecordMapAddToFile(tMtiMap *tpMap, CHAR *cpFilename);


/***
 *
 * @param tpAllMap
 * @param cpFilename
 * @return
 */
INT mtiRecordMapToFile(tMtiMap *tpAllMap, CHAR *cpFilename);

/***
 *
 * @param tpMap
 * @param cpFilename
 * @return
 */
INT mtiMapToFile(tMtiMap *tpMap, CHAR *cpFilename);

/***
 *
 * @param cpFilename
 * @param tpMap
 * @return
 */
INT mtiFileToMap(CHAR *cpFilename, tMtiMap *tpMap);

/***
*
* @param cpFilename
* @param tpMap
* @return
*/
INT mtiWriteFileAndClose(CHAR *cpFilename, UCHAR *uchData, INT nLength);


/***
*
* @param cpFilename
* @param tpMap
* @return
*/
INT mtiReadFileAndClose(CHAR *cpFilename, UCHAR *uchData, INT nLength);


/***
 *
 * @return
 */
VOID mtiFinalizeFileSystem();

/***
 *
 * @return
 */
VOID mtiFileDbg(CHAR *cpFileName);

INT mtiMapToFilePerTrans(tMtiMap *tpMap, CHAR *cpFilename, int iIndexRec); //@@WAY, 201901 slowly trans
/***
 *
 * @return
 */
BOOL mtiCheckQRDispCap();

/***
 *
 * @return
 */
INT MtiGetEDCModelName();


/************************************************************************************************************
	PRINTER
************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


#define P_MAX_COULUM				48

#define P_BLANK_CONTENTS			" "

#define P_ALIGN_LEFT				100
#define P_ALIGN_CENTER				101
#define P_ALIGN_RIGHT				102

#define P_MODE_NORMAL				0x0001
#define P_MODE_BOLD					0x0002
#define P_MODE_DOUBLE_BOLD			0x0004
#define P_MODE_IMAGE				0x0008
#define P_MODE_QRCODE				0x0010

#define P_STATUS_NO_PAPER			-1
#define P_STATUS_LOW_BATTERY		-2
#define P_STATUS_PRINT_ERROR		-3

/***
 *
 * @return
 */
VOID mtiInitializePrinter();

/***
 *
 * @return
 */
INT mtiIsValildPrinter();

/***
 *
 * @param tpPrtMap
 * @param iIndex
 * @param cAttribute
 * @param cpContents
 * @return
 */
INT mtiUpdatePrintQueue(tMtiMap *tpPrtMap, INT iIndex, CHAR cAttribute, CHAR *cpContents);
/***
 *
 *
 * @param tpPrtCont
 * @param iAttribute
 * @param cpContents
 * @return
 */
INT mtiAddPrintQueue(tMtiMap *tpPrtMap, CHAR cAttribute, CHAR *cpContents);

/****
 *
 *
 * @return
 */
INT mtiGetPrintStatus();

/***
 *
 * @param tpPrtCont
 * @return
 */
INT mtiDoPrinting(tMtiMap *tpPrtMap);

/***
 *
 * @return
 */
VOID mtiFinalizePrinter();


#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	COMMUNICATION
************************************************************************************************************/
#define IPV4_ADDR_LENGTH 		15 + 1
#define SSID_MAX_LENGTH 		255
#define GPRS_APN_LENGTH			20
#define GPRS_USERNAME_LENGTH	20
#define GPRS_PASSWORD_LENGTH	20

typedef struct _tCommInfo {
   INT iCommID;
   INT iCommType;
   INT iCommOption;
   INT iCommChannel;
   INT iCommStatus;
   INT iCommBaudrate;
   INT iCommTrySequence;
   CHAR *cpRemoteTelNo;
   CHAR *cpRemoteIP;
   INT iRemotePort;
   ULONG ulTimeoutMsec;
} tCommInfo;

typedef struct _tNetInfo {
	CHAR caAddr[IPV4_ADDR_LENGTH];
	CHAR caNetmask[IPV4_ADDR_LENGTH];
	CHAR caGateway[IPV4_ADDR_LENGTH];
	INT iDHCP_Enabled;
	INT iNetStatus;
	INT iEthLink;
	INT iMNC;
	INT iCellID;
	INT iLAC;
} tNetInfo;

typedef struct _tWifiScanAP {
	CHAR caSSID[100];
	INT iAuthType;
	INT iKeyMgmt;
	INT iRssiPersentage;
	INT iAuthAlg;
} tWifiScanAP;

#define COMM_NET_STAT_ERROR				0
#define COMM_NET_STAT_ONLINE			1
#define COMM_NET_STAT_OFFLINE			2

#define COMM_NET_STAT_ETH_UNLINK		0
#define COMM_NET_STAT_ETH_LINK			1

#define COMM_NET_STAT_STATIC			0
#define COMM_NET_STAT_DHCP				1

#define COMMTYPE_FAXMODEM				0
#define COMMTYPE_GPRS					1
#define COMMTYPE_WIFI					2
#define COMMTYPE_ETHERNET				3
#define COMMTYPE_SERIAL					4

#define COMMCHANNEL_COM0				100
#define COMMCHANNEL_COM1				101
#define COMMCHANNEL_COM2				102
#define COMMCHANNEL_COM3				103
#define COMMCHANNEL_COM4				104
#define COMMCHANNEL_COM5				105
#define COMMCHANNEL_COM6				106
#define COMMCHANNEL_COM7				107
#define COMMCHANNEL_COM8				108
#define COMMCHANNEL_COM9				109
#define COMMCHANNEL_DEV_USB				110

#define COMMBAUD_1200					0
#define COMMBAUD_2400					1
#define COMMBAUD_9600					2
#define COMMBAUD_19200					3
#define COMMBAUD_38400					4
#define COMMBAUD_57600					5
#define COMMBAUD_115200					6

#define COMMOPT_PRECONNECTION					0x0001
#define COMMOPT_BLOCKING_MODE					0x0002
#define COMMOPT_FAST_CONNECTION					0x0004
#define COMMOPT_HDSL_MODE						0x0008
#define COMMOPT_DATA_COMPRESSION				0x0010
#define COMMOPT_HANGUP_DTR						0x0020
#define COMMOPT_CORRECTION						0x0040
#define COMMOPT_DIALMODE_TONE					0x0080
#define COMMOPT_DIALMODE_PLUSE					0x0100
#define COMMOPT_BLIND_DIAL						0x0200
#define COMMOPT_CMD_TERMINATOR					0x0400
#define COMMOPT_ASYNC_MODEM						0x0800

#define COMMSTAT_BACKUP_DIALUP			95

#define COMMSTAT_PRECONNECTION			100
#define COMMSTAT_DIALUP					101
#define COMMSTAT_CONNECTING				102
#define COMMSTAT_CMMERR_CONN_FAULT		103
#define COMMSTAT_CONNECTION				110
#define COMMSTAT_SEND					111
#define COMMSTAT_RECV					112
#define COMMSTAT_DISCONNECT				114
#define COMMSTAT_REVERSAL_COMPLETED		115
#define COMMSTAT_CMMERR_SEND_FAULT		116
#define COMMSTAT_CMMERR_RECV_TIMEOUT	117
#define COMMSTAT_MAC_FAULT				118
#define COMMSTAT_CMMERR_RECV_INVALID	119

#define COMM_CELLULAR_AUTH_NONE			0
#define COMM_CELLULAR_AUTH_CHAP			1
#define COMM_CELLULAR_AUTH_PAP			2
#define COMM_CELLULAR_AUTH_CHAP_PAP		3

#define COMM_WIFI_PROTO_WPA_WPA2		0	//WPA + WPA2
#define COMM_WIFI_PROTO_WPA				1
#define COMM_WIFI_PROTO_WPA2			2
#define COMM_WIFI_PROTO_WEP				3

#define COMM_WIFI_KEY_MGMT_PSK			0
#define COMM_WIFI_KEY_MGMT_EAP			1
#define COMM_WIFI_KEY_MGMT_NONE			2

#define COMM_WIFI_CONN_AUTO				0
#define COMM_WIFI_CONN_MANUAL			1

#define COMM_WIFI_EAP_TYPE_PEAP			0
#define COMM_WIFI_EAP_TYPE_FAST			1

#define COMM_WIFI_AUTH_OPEN				0
#define COMM_WIFI_AUTH_SHARED			1

/***
 *
 * @return
 */
tCommInfo *mtiInitializeComm();

/***
 *
 * @param tpCommInfo
 * @param ulTimeout
 * @return
 */
INT mtiFlushComm(tCommInfo *tpCommInfo);

/***
 *
 * @param tpCommInfo
 * @param ulTimeout
 * @return
 */
INT mtiPreOpenComm(tCommInfo *tpCommInfo, ULONG ulTimeout);

/***
 *
 * @param tpCommInfo
 * @param ulTimeout
 * @return
 */
INT mtiOpenCommWithPOS(tCommInfo *tpCommInfo, ULONG ulTimeout, BOOL bPOSUse);

INT mtiOpenComm(tCommInfo *tpCommInfo, ULONG ulTimeout);

/***
*
* @param tpCommInfo
* @param ulTimeout
* @return
*/
INT mtiGetCommHandle(tCommInfo *tpCommInfo);


/***
 *
 * @param tpCommInfo
 * @param ucpWrite
 * @param iLength
 * @return
 */
INT mtiWriteComm(tCommInfo *tpCommInfo, UCHAR *ucpWrite, INT iLength);
INT mtiWriteCommWithTimeOut(tCommInfo *tpCommInfo, UCHAR *ucpWrite, INT iLength, INT nTimeOut);

/***
 *
 * @param tpCommInfo
 * @param ucpRead
 * @param iReadLength
 * @return
 */
INT mtiReadComm_With_Timeout(tCommInfo *tpCommInfo, UCHAR *ucpRead, INT iLength, INT *iReadLength, INT nMillisecond);
INT mtiReadComm(tCommInfo *tpCommInfo, UCHAR *ucpRead, INT iLength, INT *iReadLength);

/***
 *
 * @param tpCommInfo
 * @return
 */
INT mtiCloseComm(tCommInfo *tpCommInfo);

/***
 *
 * @param tpCommInfo
 * @return
 */
VOID mtiFinalizeComm(tCommInfo *tpCommInfo);

INT mtiActiveNetwork();

/***
 *
 * @param iDhcp
 * @param cpIpAddr
 * @param cpNetMask
 * @param cpGw
 * @return
 */
INT mtiSetEthIF(INT iDhcp, CHAR *cpIpAddr, CHAR *cpNetMask, CHAR *cpGw);

/***
 *
 * @param cpAPN
 * @param iAuthType
 * @param cpUserName
 * @param cpPasswd
 * @return
 */
INT mtiSetCelluarIF(CHAR *cpAPN, INT iAuthType, CHAR *cpUserName, CHAR *cpPasswd);

/***
 *
 * @param cpSSID
 * @param iAuthType
 * @param iKeyMgmt
 * @param cpUsername
 * @param cpPasswd
 * @param iWifiEapType
 * @param iAuthAlg
 * @param iDhcp
 * @param cpIpAddr
 * @param cpNetMask
 * @param cpGw
 * @return
 */
INT mtiSetWifiIF(CHAR *cpSSID, INT iAuthType, INT iKeyMgmt, CHAR *cpUsername, CHAR *cpPasswd,
		INT iWifiEapType, INT iAuthAlg, INT iDhcp, CHAR *cpIpAddr, CHAR *cpNetMask, CHAR *cpGw);

/***
 *
 * @param tpWifiScanList
 * @param iScanCnt
 * @param iCurCommType
 * @return
 */
INT mtiScanWiFi(tWifiScanAP *tpWifiScanList, INT *iScanCnt, INT iCurCommType);

/***
 *
 * @param iNetType
 * @param tpNetInfo
 * @return
 */
INT mtiGetNetInfo(INT iNetType, tNetInfo *tpNetInfo);

/***
 *
 * @param iNetType
 * @param tpNetInfo
 * @return
 */
BOOL mtiGetNetAttachStatus(INT iNetType, tNetInfo *tpNetInfo);
/************************************************************************************************************
	TIME
************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/***
 *
 * @param ucCurTime : out, Filled with Current Time Data (14 Bytes)
 * 		  iTimeBuffLen : in, Size of "ucCurTime". Should be 14.
 * @return RTN_SUCCESS | RTN_ERROR
 */
INT mtiGetDateTime(UCHAR *ucCurTime, INT iTimeBuffLen);

/***
 *
 * @param tpCommInfoucCurTime : in, be set System Time with This Time Data.
 * 		  iTimeBuffLen : in, Size of "ucCurTime". Should be 14.
 * @return RTN_SUCCESS | RTN_ERROR
 */
INT mtiSetDateTime(UCHAR *ucSetTime, INT iTimeBuffLen);

/*** */
INT mtiSetDateTimeForInni(UCHAR *ucSetTime, INT iTimeBuffLen, INT iHour);

#ifdef __cplusplus
}
#endif
/************************************************************************************************************
	MISC
************************************************************************************************************/

#define MISC_DEVICE_CTLS						1

#define MISC_DEVICE_TRUN_OFF					0x0000
#define MISC_DEVICE_TRUN_ON						0x0001

#ifdef __INGENICO_SRC__
#define MAX_SERIAL_SIZE						15
#elif __VERIFONE_SRC__
#define MAX_SERIAL_SIZE						11
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***
 *
 * @param uiMaxBytes
 * @param uiFreeBytes
 * @return
 */
VOID mtiGetRamStatus(UINT *uiMaxBytes, UINT *uiFreeBytes);
/***
 *
 * @param cpOut
 * @return
 */
INT mtiGetTerminalSerial(CHAR *cpOut);

/***
 *
 * @param cpOut
 * @return
 */
INT mtiGetTerminalVersion(CHAR *cpOut);

/***
 *
 * @param cpOut
 * @return
 */
INT mtiSysSleep(INT iOnOff);

#ifdef __VERIFONE_SRC__
/***
 *
 * @return VX520, VX675
 */
INT mtiGetHwType();
#endif

/***
 *
 * @return
 */
INT mtiGetHwSupport();

/***
 *
 * @param iType
 * @param iValue
 * @return
 */
INT mtiSetHwConfig(INT iType, INT iValue);

/***
 *
 * @param iType
 * @return
 */
INT mtiGetHwConfig(INT iType);

/***
 *
 * @return VX520, VX675
 */
INT mtiGetHwSupport();

/***
 *
 * @return VX520, VX675
 */
VOID mtiSysReboot();

//WIFI Scan Data Parser
/***
 *
 * @return RTN_SUCCESS, RTN_ERROR
 */
INT iWiFiScanRawDataParser(CHAR *cpWiFiScanRawData, tWifiScanAP *tpWifiScanList, INT *iWifiScanCnt);

#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	IC CONTROLLER
************************************************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#define SLOT_MAIN_ICC			1000
#define SLOT_SAM1_ICC			1011
#define SLOT_SAM2_ICC			1012
#define SLOT_SAM3_ICC			1013

/***
 *
 * @return Handler
 */
INT mtiInitializeICController(INT iCtrlType);

/***
 *
 * @param iHandler
 * @return
 */
INT mtiIsCardPresent(INT iHandler);

/***
 *
 * @param iHandler
 * @param ucpCardATR
 * @param iCardUidLen
 * @return
 */
INT mtiICPowerOn(INT iHandler, UCHAR *ucpCardATR, INT *iCardATRLen);

/***
 *
 * @param iHandler
 * @return
 */
INT mtiICPowerOff(INT iHandler);

/***
 *
 * @param ucpApduCmd
 * @param iApduCmdLen
 * @param ucpResData
 * @param iResDatalen
 * @return
 */
INT mtiICSendApdu(INT iHandler, UCHAR *ucpApduCmd, INT iApduCmdLen, UCHAR *ucpResData, INT *iResDatalen);

/***
 *
 * @return
 */
VOID mtiFinalizeICController(INT iHandler);

#ifdef __cplusplus
}
#endif
/************************************************************************************************************
	SOUND
************************************************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/***
 *
 * @param iDuration sustain time
 * @param iCnt repeat time
 * @return
 */
VOID mtiSoundBeep(INT iDuration, INT iCnt);

/***
 *
 * @return
 */
VOID mtiSoundDoremifa();

/***
 *
 * @return
 */
VOID mtiSoundWarning(INT iDuration, INT iCnt);

#ifdef __cplusplus
}
#endif

/************************************************************************************************************
	CTLS (ContactLess)
************************************************************************************************************/
#define CTLS_LED_1			0x01
#define CTLS_LED_2			0x02
#define CTLS_LED_3			0x04
#define CTLS_LED_4			0x08
#define CTLS_LED_ALL		0x0F

#define CTLS_LED_OFF		0x00
#define CTLS_LED_ON			0x01
/***
 *
 * @return
 */
INT mtiInitializeCtls();

/***
 *
 * @return
 */
INT mtiFinalizeCtls();

/***
 *
 * @return
 */
INT mtiCtlsDetectCard();

/***
 *
 * @param ucpCardUid
 * @param iCardUidLen
 * @return
 */
INT mtiCtlsGetCardUid(UCHAR *ucpCardUid, INT *iCardUidLen);

/***
 *
 * @param ucpApduCmd
 * @param iApduCmdLen
 * @param ucpResData
 * @param iResDatalen
 * @return
 */
INT mtiCtlsSendApdu(UCHAR *ucpApduCmd, INT iApduCmdLen, UCHAR *ucpResData, INT *iResDatalen);

/***
 *
 * @param ucLed
 * @param ucOnOff
 * @return
 */
INT mtiCtlsLEDCtrl(UCHAR ucLed, UCHAR ucOnOff);


#ifdef __cplusplus
extern "C"
{
#endif
/***
 *
 * @return
 */
INT mtiGetRandom(UCHAR *ucpRamdomData, INT iLen);


#ifdef __cplusplus
}
#endif
/************************************************************************************************************
 function test 
************************************************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

void deviceFunctionTesting();

INT mtiGetBatteryLevel();

INT mtiRfEmvPreparing(BOOL doCTLS);
INT mtiRfEmvParamConfToReader(void); // @@WAY PAYWAVE C680 20200320
/***
 *
 * @param pszIP
 * @param nPort
 * @return
 */
VOID mtiDebugStart(CHAR *pszIP, INT nPort);

/***
 *
 * @return
 */
VOID mitDebugStop();
void hex_dump_char(const char *pzTitle, const byte *buff, long len);
void hexdump(const char *pzTitle, const byte *buff, long len);
#ifdef __cplusplus
}
#endif

#endif
