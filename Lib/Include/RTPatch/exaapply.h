/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXAAPPLY.H                                                         |
|                                                                           |
|                                                                           |
|  RTPatch Server Apply DLL Internal Definition Header File                 |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2011.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/

# ifndef EXAPATCH_APPLY_INTERNALS_INCLUDED
# define EXAPATCH_APPLY_INTERNALS_INCLUDED

# ifndef EXAPATCH_INTERNAL
# define EXAPATCH_INTERNAL
# endif

# include "exafeature.h"
# include "expint.h"
# include "expapply.h"

/* structures */


typedef struct _ExaPatchApplyTable {
DWORD TableType;
DWORD MinLength;
DWORD MaxLength;
DWORD EscCW;
DWORD HintLength;
DWORD NumLevels;
DWORD StartLevel;
DWORD EscSize;
DWORD * BrkPt;
DWORD * BrkCW;
DWORD * CW;
const unsigned char * CWExtra;
} ExaPatchApplyTable;

typedef struct _ExaPatchApplyState {
ExaPatchApplyFileHeaderInfo * HeaderPtr;
int (CALLBACK *CBPtr)(DWORD ID, LPVOID Ptr, PLATWORD Handle);
PLATWORD CBHandle;
DWORD LocalStart;
DWORD LocalDelta;
DWORD LocalPrev;
DWORD GlobalStart;
DWORD GlobalDelta;
DWORD GlobalPrev;
int EOFEncountered;
ExaPatchStream * PatchFile;
ExaPatchStream * OldFile;
ExaPatchStream * NewFile;
ExaPatchApplyTable * CharTable;
ExaPatchApplyTable * DispTable;
ExaPatchApplyTable * LenTable;
QWORD PatchSize;
QWORD PatchSizeLeft;
unsigned char * LocalBuffer;
DWORD LBUsed;
unsigned char * PropBuffer;
DWORD PBUsed;
DWORD PBSize;
DWORD PBOffset;
DWORD NeedsSeek;
unsigned char * Buffer;
DWORD BufSize;
unsigned char * BufferPtr;
DWORD BufSizeLeft;
DWORD Barrel;
DWORD BitsInBarrel;
DWORD PrevNegOff;
DWORD PrevPosOff;
DWORD PrevDWordMod[256];
DWORD NumPDM;
DWORD OrgPDM;
DWORD PrevWordMod[256];
DWORD NumPWM;
DWORD OrgPWM;
DWORD PrevByteMod[16];
DWORD NumPBM;
DWORD OrgPBM;
QWORD PrevModOrigin;
QWORD PrevModLength;
QWORD PrevModOffset;
QWORD NextMark;
QWORD NewFilePos;
QWORD OldFileSize;
# ifdef DUMP_ONLY
QWORD MaxOldPos;
FILETIME PrevTime;
FILETIME StartTime;
DWORD NumCW;
DWORD NumCopy;
DWORD NumWdw;
DWORD NumChar;
DWORD NumMod;
DWORD NumPropMod;
DWORD MaxBackSeek;
# endif
} ExaPatchApplyState;



/* routines to be called from exaawrap.c */
int ExaPatchApplyWorkInit( ExaPatchApplyState * TheState );
int ExaPatchApplyUsePW( ExaPatchApplyState * TheState,
	QWORD * Hash1, QWORD * Hash2 );
int ExaPatchApplyWorkRoutine( ExaPatchApplyState * TheState );

# ifndef DUMP_ONLY
# ifndef ATTOPATCH
int ExaPatchFileSearch(
  WCHAR ** FullPathPtrPtr,
	WCHAR * FileName, 
	WCHAR * Directory, 
	DWORD SearchFlags, 
	ExaPatchFileAttrib * TheAttrib,
	ExaPatchFileChecksum * TheCkSum,
	ExaPatchApplyIgnoreRegion * RegList,
	ExaPatchApplyDupListEntry ** DupList,
	ExaPatchDiscriminator * TheDisc,
	WCHAR PathDelim,
	WCHAR PathSep,
	int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
	PLATWORD CallbackHandle,
	WCHAR * SystemName,
	DWORD SystemBase );
int VerifyFile( WCHAR * FullName,
		DWORD SearchFlags,
		ExaPatchFileAttrib * TheAttrib,
		ExaPatchFileChecksum * TheCkSum,
		ExaPatchApplyIgnoreRegion * RegList,
		ExaPatchApplyDupListEntry ** DupList,
		ExaPatchDiscriminator * TheDisc,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		WCHAR * SystemName,
		DWORD SystemBase  );
int ExaPatchDoHook( WCHAR * Hook,
	WCHAR * FileName, 
	WCHAR * UpdateDir );
# endif
# endif
void ExaPatchError( 
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * Entry,
		DWORD ErrorNumber,
		... );
int ExaPatchWarning(
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * Entry,
		DWORD WarningNumber,
		... );
int ExaPatchInfo(
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		DWORD StringNumber,
		... );
# ifdef DUMP_ONLY
int ExaPatchDump( ExaPatchApplyState * StatePtr,
		int FmtFlag,
    WCHAR * FmtString,
    ... );
# endif
WCHAR * ExaGetString( DWORD StringNum );
int ExaGetNarrowString( DWORD StringNum, char ** ppString );
int ExaEZTemp( WCHAR * pDirectory, WCHAR ** ppFileName );
int ExaPrettify( QWORD NumBytes, DWORD * pNum, char * pPrefix, DWORD UpOrDown );
# ifndef DUMP_ONLY
# ifndef ATTOPATCH
/* these are in exareg.c */
int ExaPatchDoFileManip( ExaPatchApplyFileManipulation * FileManip,
	ExaPatchApplyFileHeaderInfo * HeaderPtr, 
	int DelayFlag,
	int (CALLBACK * CBPtr)(DWORD, LPVOID, PLATWORD),
	PLATWORD CBHandle );
	/* DelayFlag = 0 => Do the indicated operation with no delay allowed */
	/* DelayFlag = 1 => Attempt the indicated operation.  If it fails, add to delay list */
	/* DelayFlag = 2 => Just add to delay list (don't attempt) */
	/* DelayFlag = -1 => process the delay list */
	/* in all cases, if CBPtr is non-NULL, attempt user file manipulation first, */
	/*	with fall-through to default if EXP_CALLBACK_CONTINUE is returned */
int ExaPatchDoRegScript( unsigned char * Script,
	DWORD Size, 
	DWORD NumEntries, 
	ExaPatchApplyFileHeaderInfo * HeaderPtr );
	/* Script = NULL does an Undo */
int ExaPatchFindKeyDir( WCHAR ** RetPtr,
	int KeyType,
	int KeyBase,
	WCHAR * lpParm1, 
	WCHAR * lpParm2, 
	WCHAR * lpParm3 );
# endif
# endif

/* Defines */
/*SearchFlags */
#define EXP_SEARCH_RECURSE 0x1
#define EXP_SEARCH_VERIFY_ATTRIB 0x2
#define EXP_SEARCH_VERIFY_CKSUM 0x4
#define EXP_SEARCH_IGNORE_DUPS 0x8
#define EXP_SEARCH_ADD_TO_DUP 0x10
#define EXP_SEARCH_CHECK_OLD_DISC 0x20
#define EXP_SEARCH_CHECK_NEW_DISC 0x40
#define EXP_SEARCH_IGNORE_CASE 0x80
#define EXP_SEARCH_IGNORE_TIMEZONE 0x100
#define EXP_SEARCH_SPECIAL_RECURSE 0x200
#define EXP_SEARCH_CALLBACK 0x400
#define EXP_SEARCH_CONFIRMATION 0x800
#define EXP_SEARCH_FULL_RECURSE 0x1000
#define EXP_SEARCH_IGNORELINKS 0x2000
#define EXP_SEARCH_PROCESSLINKS 0x4000
#define EXP_SEARCH_ALLOWPAD 0x8000
/*
NOTE: EXP_SEARCH_RECURSE has recursion on ONLY the first directory in the list
EXP_SEARCH_SPECIAL_RECURSE moves this to the second directory in the list
EXP_SEARCH_FULL_RECURSE recurses every directory in the list
*/

/* BW defines */
#define EXP_BW_BACKUP_USED 0x1
#define EXP_BW_ERRORFILE_USED 0x2

/* Stat index defines */
#define EXP_FILES_MODIFIED 0x0
#define EXP_FILES_ADDED 0x1
#define EXP_FILES_RENAMED 0x2
#define EXP_FILES_DELETED 0x3
#define EXP_FILES_SKIPPED 0x4
#define EXP_FILES_MISSING 0x5
#define EXP_FILES_INVALID 0x6

/* Platform-specific file names */
# ifndef ATTOPATCH
# ifdef _WIN32
#  define BACKUP_CMD_FILE L"dfcundo.bak"
#  define BACKUP2_CMD_FILE L"dfcundo2.bak"
# else
#  define BACKUP_CMD_FILE L"dfcundo"
# endif
# endif /* ATTOPATCH */

# ifdef _WIN32
extern HANDLE hExaApplyModule;
# endif
/* Error and Warning string numbers */
# include "exaapstr.h"


# endif /* !EXAPATCH_APPLY_INTERNALS_INCLUDED */
