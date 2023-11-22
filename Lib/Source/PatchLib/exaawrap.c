

/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXAAWRAP.C                                                         |
|                                                                           |
|                                                                           |
|  RTPatch Server Apply Interface Wrapper Routines                          |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2014.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/
/*lint --e{950, 9026, 9024, 621, 715, 818, 9087, 9078, 923, 904, 527, 801, 9042, 766} */
/* Suppress non-ANSI, function-type macro, ## operator, identifier clash, unused parameter, const pointer, 
	suspicious cast, early return, unreachable code, goto, switch syntax, 
	unused header warnings in this file (all deviations are marked) */

# define EXAPATCH_INTERNAL_CODE
# include "eval.h"
// MISRA C 2012 [general; 20.5]: unnecessary (but harmless) header included (eval.h) to enable code re-use
// in other products

# ifdef EVAL
# ifdef UNIX_SUPPORT
# include <ctype.h>
void  PrintEvalMessage( void );
# else
#  include <time.h>
# endif
# endif

# if defined(EA_SUPPORT) || defined(ADS_SUPPORT)
#  ifdef UNIX_SUPPORT
# include <stdint.h>
# include <ctype.h>
typedef uint32_t UINT32;
typedef uint16_t UINT16;
typedef unsigned char BYTE;
#  endif
# endif
# ifdef EA_SUPPORT
# include <eautils.h>
# endif
# ifdef ADS_SUPPORT
# include <adsutils.h>
# endif

# include "expachan.h"
# include "expfmt.h"
# ifndef ATTOPATCH
# include "expunique.h"
# include <stdio.h>
# endif

/* suppress some spurious warning messages for gcc - this is (of necessity) overly broad if the version is between 4.2 and 4.6 */
# ifndef _WIN32
# if (GCC_VERSION >= 40200) && (GCC_VERSION < 40600)
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
# endif
# endif

# ifndef ATTOPATCH
# define Validate(a,b) if((a) >= (QWORD)(b)){Code=EXAPATCH_CORRUPT_PATCH_FILE;goto exit;}
# define Validate2(a,b) if((a) >= (QWORD)(b)){Code=EXAPATCH_INVALID_PARAMETER;goto exit;}
# endif

# if defined(EVAL) || !defined( ATTOPATCH )
# ifndef UNIX_SUPPORT  
/* First 11 chars stay. Second 11 chars replaced by hammer */
static unsigned char SerialNumber[] = {
	(unsigned char)('$'+0x31U),	(unsigned char)('$'+0x32U),	(unsigned char)('$'+0x33U),	(unsigned char)('$'+0x34U),
	(unsigned char)('$'+0x35U),	(unsigned char)('$'+0x36U),	(unsigned char)('$'+0x37U),	(unsigned char)('$'+0x38U),
	(unsigned char)('$'+0x39U),	(unsigned char)('$'+0x3aU),	(unsigned char)(0x3bU),
	(unsigned char)('$'+0x31U),	(unsigned char)('$'+0x32U),	(unsigned char)('$'+0x33U),	(unsigned char)('$'+0x34U),
	(unsigned char)('$'+0x35U),	(unsigned char)('$'+0x36U),	(unsigned char)('$'+0x37U),	(unsigned char)('$'+0x38U),
	(unsigned char)('$'+0x39U),	(unsigned char)('$'+0x3aU),	(unsigned char)(0x3bU),
  0U
};
# endif
# endif
# ifdef EVAL
# ifndef UNIX_SUPPORT
static int PrintBanner ( void );
# endif
# endif
# ifdef ADS_SUPPORT
static char ADSDefaultString[]="\0\0\0\0\3\0\0\0\1\0*\0\0\0\0";
static ExaPatchExtendedHeader ADSDefaultEH={ADS_BLOCKID_GLOBAL,sizeof(ADSDefaultString),(void *) ADSDefaultString};
static int DoADS( ExaPatchApplyFileManipulation *, ExaPatchExtendedHeader *);
# endif
# ifdef EA_SUPPORT
static char EADefaultString[]="\0\0\0\0\2\0\0\0\1*\0\0\0\0";
static ExaPatchExtendedHeader EADefaultEH={EA_BLOCKID_GLOBAL,sizeof(EADefaultString),(void *) EADefaultString};
static int DoEA( ExaPatchApplyFileManipulation *, ExaPatchExtendedHeader *);
# endif
# ifdef EXAPATCH_OWNERGROUP_SUPPORT
static int DoOG( ExaPatchApplyFileManipulation *, ExaPatchExtendedHeader *);
# endif
static int MarkProgress( ExaPatchApplyFileHeaderInfo * HeaderPtr,
# ifdef ATTOPATCH
		const ExaPatchApplyFileEntryInfo * EntryPtr,
# else
		ExaPatchApplyFileEntryInfo * EntryPtr,
# endif
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		DWORD dwLocalStart,
		DWORD dwLocalDelta
 );
# if defined( EXAPATCH_REGISTRY_SUPPORT ) && defined( EXAPATCH_EH_SUPPORT )
#define DESTKEY_EH_SIG 0x6b44
static int DirRecurse( DWORD dwFlags,
											wchar_t * Directory, 
											int (*ProcessFile)(void *, wchar_t *), 
											int (*PreProcessDir)(void *,wchar_t *), 
											int (*PostProcessDir)(void *,wchar_t *), 
											void * Object);
static int rrm( wchar_t * Directory );
static int rcp( wchar_t * SrcDir, wchar_t * DestDir, 
							int (CALLBACK *ProgressCallBack)(DWORD ID, 
										LPVOID Ptr, PLATWORD Handle),
							PLATWORD CallbackHandle,
							wchar_t ** pTopDir);
static int DestKeyHandling( ExaPatchApplyFileHeaderInfo * HeaderPtr, 
						  wchar_t ** pApplyDir,
							int (CALLBACK *ProgressCallBack)(DWORD ID, 
										LPVOID Ptr, PLATWORD Handle),
							PLATWORD CallbackHandle);
static int DestKeyHandling2( ExaPatchApplyFileHeaderInfo * HeaderPtr,
							wchar_t ** pApplyDir,
							int (CALLBACK *ProgressCallBack) (DWORD ID,
								LPVOID Ptr, PLATWORD Handle),
							PLATWORD CallbackHandle );
# else
# ifdef EXAPATCH_ARCHIVE_SUPPORT
static int DirRecurse( DWORD dwFlags,
											wchar_t * Directory, 
											int (*ProcessFile)(void *, wchar_t *), 
											int (*PreProcessDir)(void *,wchar_t *), 
											int (*PostProcessDir)(void *,wchar_t *), 
											void * Object);
static int rrm( wchar_t * Directory );
# endif
# endif
# if defined( EXAPATCH_ARCHIVE_SUPPORT ) || defined( EXAPATCH_LOCKDIR_SUPPORT )
static DWORD GetLEDword( unsigned char * Ptr );
static SQWORD GetLESQword( unsigned char * Ptr );
static int GetLEwchar( unsigned char * Ptr, wchar_t ** ppWc );
# endif
# ifdef EXAPATCH_ARCHIVE_SUPPORT
# ifdef EXAPATCH_EH_SUPPORT
typedef struct _ArchiveStack {
	struct _ArchiveStack * pPrev;
	wchar_t * PrevApplyDir;
	wchar_t * PrevSubDir;
	wchar_t * NewApplyRoot;
	wchar_t * OldArchName;
	wchar_t * TempArchName;
	wchar_t * NewArchName;
	ExaPatchApplyFileManipulation * pManip;
	DWORD dwPrevGlobalFlags;
	DWORD dwPrevFileFlags;
	DWORD dwOwner;
	DWORD dwGroup;
	DWORD dwPermissions;
	DWORD dwMagic;
	DWORD dwFlags;
	int nArchType;
} ArchiveStack;
typedef struct _ArchiveQueueEntry 
{
	struct _ArchiveQueueEntry * pNext;
	struct _ArchiveQueueEntry * pPrev;
	DWORD dwType; // 0 => Start, 1 => Subdir
	DWORD dwMagic;
	SQWORD nSubdir;	// Start only
	SQWORD nSystem;	// Start only
	DWORD dwOwner;	// Start only
	DWORD dwGroup;	// Start only
	DWORD dwPermissions;	// Start only
	DWORD dwFlags;	// Start only
	wchar_t * pName;	// filename/dirname
} ArchiveQueueEntry, * PArchiveQueueEntry;
typedef struct _ArchiveInfo
{
	ArchiveStack * pStack;
	ArchiveQueueEntry * pAQHead;
	ArchiveQueueEntry * pAQTail;
	wchar_t * pCurSubdir;
} ArchiveInfo, * PArchiveInfo;
static int ArchiveStart(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, ExaPatchExtendedHeader * EHBlockPtr);
static int ArchiveSubdir(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, ExaPatchExtendedHeader * EHBlockPtr);
static int ArchiveEnd(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, ExaPatchExtendedHeader * EHBlockPtr,
	int (CALLBACK *ProgressCallBack)(DWORD ID, LPVOID Ptr, PLATWORD Handle), PLATWORD CallbackHandle );
static int ArchiveProcess(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, PLATWORD Handle);
# endif
# endif
# ifndef ATTOPATCH
static int SetSearchParms( 
	ExaPatchApplyFileHeaderInfo * HeaderPtr,
	ExaPatchApplyFileEntryInfo * EntryPtr, 
	DWORD * pSearchFlags, 
	DWORD * pAttribFlags,
	wchar_t ** pSearchPath, 
	wchar_t ** pHintSubDir,
	DWORD HistoryFlag );
static int AllocNameArrays(
	DWORD dwNumOldFiles,
	DWORD dwNumNewFiles,
	wchar_t *** pOldFileNameArray,
	wchar_t *** pIntFileNameArray,
	wchar_t *** pNewFileNameArray,
	QWORD ** pSizeArray
	);
static int SetFileLoc(
	wchar_t ** OldFileNameArray,
	wchar_t ** IntFileNameArray,
	wchar_t ** NewFileNameArray,
	QWORD * SizeArray,
 	ExaPatchApplyFileHeaderInfo * HeaderPtr,
	ExaPatchApplyFileEntryInfo * OldEntryPtr,
	ExaPatchApplyFileEntryInfo * NewEntryPtr,
	DWORD dwNumOldFiles,
	DWORD dwNumNewFiles,
	DWORD *pCNCFlag,
	DWORD dwNumManipEntries,
	unsigned short wModFlags,
	DWORD dwFileFlags,
	DWORD dwAttribRetain,
	DWORD * pNumOldMoved,
	DWORD * pNumIntMoved,
	wchar_t * HintSubDir,
	PLATWORD Handle
	);
# endif /* ATTOPATCH */

# ifdef _WIN32
BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved );
HANDLE hExaApplyModule;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
  switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hExaApplyModule = hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
		default:
			/* no initialization needed */
			break;
    }
    return TRUE;
}
/* MISRA C 2012 [2.7, 8.13]: correct DllMain usage should not reference lpReserved */
# else
# ifndef ATTOPATCH
static void Slashify( wchar_t * Ptr );
# endif
# endif
static int AcquireChannel( PLATWORD Handle )
{
# ifdef ATTOPATCH
  /* NOTE: non-reentrant */
  /* MISRA C 2012 [2.7]: Handle is intentionally not referenced (used to enable code re-use for other products) */
  return(EXAPATCH_SUCCESS);
# else
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle;
	int Code;

# ifdef _WIN32
	if (IsBadReadPtr( TheChannel, 4) || TheChannel->Signature != CHANNEL_SIGNATURE)
	{
		return(EXAPATCH_INVALID_HANDLE);
	}
	EnterCriticalSection( &TheChannel->CritSect );
	if (TheChannel->BusyFlag)
	{
		Code = EXAPATCH_BUSY;
	}
	else
	{
		TheChannel->BusyFlag = TRUE;
    Code = EXAPATCH_SUCCESS;
	}
	LeaveCriticalSection( &TheChannel->CritSect );
# else
	if (TheChannel->Signature != CHANNEL_SIGNATURE)
	{
		return(EXAPATCH_INVALID_HANDLE);
	}
	if (pthread_mutex_lock( &TheChannel->Mutex ))
		return(EXAPATCH_INVALID_HANDLE);
		
	if (TheChannel->BusyFlag)
	{
		Code = EXAPATCH_BUSY;
	}
	else
	{
		TheChannel->BusyFlag = TRUE;
		Code = EXAPATCH_SUCCESS;
	}
	pthread_mutex_unlock( &TheChannel->Mutex );
# endif
	return(Code);
# endif /* ATTOPATCH */
}

static int ReleaseChannel( PLATWORD Handle )
{
# ifndef ATTOPATCH
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle;

# ifdef _WIN32
	EnterCriticalSection( &TheChannel->CritSect );
	TheChannel->BusyFlag = FALSE;
	LeaveCriticalSection( &TheChannel->CritSect );
# else
	pthread_mutex_lock( &TheChannel->Mutex );
	TheChannel->BusyFlag = FALSE;
	pthread_mutex_unlock( &TheChannel->Mutex );
# endif
# endif
  /* MISRA C 2012 [2.7]: Handle is intentionally not referenced (used to enable code re-use for other products) */
	return(EXAPATCH_SUCCESS);
}

# ifndef ATTOPATCH
/* 	Non-handle-based routines (user must synchronize) */
int EXP_DLLIMPORT
	ExaPatchApplyOpenFileArrayAsStreamA(
	ExaPatchStream ** FileStreamPtr,
	unsigned char ** FileNameArray,
	DWORD NumFiles,
	DWORD OpenFlags,
	QWORD * SizeArray	/* may be NULL for read-only files */
	)
{
	wchar_t ** WideNames;
	int Code;
	DWORD i;
	DWORD BufSize;

	Code = ExaMemAlloc(NULL, NumFiles*sizeof(void *), (void **) &WideNames); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (Code == EXAPATCH_SUCCESS)
	{
		for (i=0; i<NumFiles && Code == EXAPATCH_SUCCESS; i++ )
		{
# ifdef _WIN32
			BufSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, FileNameArray[i], -1, NULL, 0 );
# else
			BufSize = strlen( (char *) FileNameArray[i] ) + 1;
# endif
			Code = ExaMemAlloc( NULL, BufSize*sizeof(wchar_t), (void **) WideNames+i ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code == EXAPATCH_SUCCESS)
			{
# ifdef _WIN32
				if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, FileNameArray[i], -1, WideNames[i], BufSize ))
				{
					Code = EXAPATCH_CHARSET_ERROR;
				}
# else
				if (-1 == PSmbstowcs(WideNames[i], (char *)FileNameArray[i], BufSize))
				{
					Code = EXAPATCH_CHARSET_ERROR;
				}
# endif
			}
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = 	ExaPatchApplyOpenFileArrayAsStreamW(
				FileStreamPtr,
				WideNames,
				NumFiles,
				OpenFlags,
				SizeArray);
		}
		NumFiles = i;
		for (i=0; i<NumFiles ; i++ )
		{
			ExaMemFree( NULL, WideNames[i] );
		}
		ExaMemFree( NULL, WideNames );
	}
	return(Code);
}


int EXP_DLLIMPORT
	ExaPatchApplyOpenFileArrayAsStreamW(
	ExaPatchStream ** FileStreamPtr,
	wchar_t ** FileNameArray,
	DWORD NumFiles,
	DWORD OpenFlags,
	QWORD * SizeArray /* may be NULL for read-only and append files */
	)
{
	HANDLE * HandlePtr=NULL;
	QWORD * SizePtr=SizeArray;
# ifdef _WIN32
	LARGE_INTEGER ll;
# endif
	int Code;
	DWORD i;

	Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) FileStreamPtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (Code == EXAPATCH_SUCCESS)
	{
		memset( *FileStreamPtr, 0, sizeof( ExaPatchStream ) );
		(*FileStreamPtr)->dwSize = sizeof( ExaPatchStream );
		Code = ExaMemAlloc( NULL, NumFiles*sizeof( HANDLE ), (void **) &HandlePtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			if (SizePtr == NULL && (OpenFlags & (EXP_OPEN_READONLY | EXP_OPEN_APPEND)) )
			{
				Code = ExaMemAlloc( NULL, NumFiles*sizeof( QWORD ), (void **) &SizePtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			}
			if (Code == EXAPATCH_SUCCESS)
			{
				for (i=0; i<NumFiles && Code == EXAPATCH_SUCCESS ; i++ )
				{
					HandlePtr[i] = ExaOpen( FileNameArray[i], (OpenFlags & EXP_OPEN_READONLY)?TRUE:FALSE, (OpenFlags & EXP_OPEN_APPEND)?TRUE:FALSE );
					if (HandlePtr[i] != INVALID_HANDLE_VALUE)
					{
						if (OpenFlags & (EXP_OPEN_READONLY | EXP_OPEN_APPEND) )
						{
# ifdef _WIN32
							ll.LowPart = GetFileSize( HandlePtr[i], &ll.HighPart );
							SizePtr[i] = ll.QuadPart;
# else
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
#  ifdef SEEK64
							/* NOT Portable */
							struct stat64 SS;

							fstat64( (int) HandlePtr[i], &SS );

#  else
							struct stat SS;
							fstat( (int) HandlePtr[i], &SS );
#  endif
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
							SizePtr[i] = (QWORD) SS.st_size;
# endif
						}
					}
					else
					{
						Code = EXAPATCH_OPEN_FAILED;
					}
				}
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = MakeExaStreamFromFileArray( *FileStreamPtr, NumFiles, HandlePtr, SizePtr );
				}
				if (Code != EXAPATCH_SUCCESS)
				{
					NumFiles = i;
					for (i=0; i<NumFiles ; i++ )
					{
						ExaClose( HandlePtr[i] );
					}
				}
			}
			ExaMemFree( NULL, HandlePtr );
			if (SizePtr && (SizeArray == NULL))
			{
				ExaMemFree( NULL, SizePtr );
			}
		}
	}
	else
	{
		*FileStreamPtr = NULL;
	}
	if (Code != EXAPATCH_SUCCESS && *FileStreamPtr)
	{
		ExaMemFree( NULL, *FileStreamPtr );
		*FileStreamPtr = NULL;
	}
	return(Code);
}


int EXP_DLLIMPORT
	ExaPatchApplyCloseStream(
	ExaPatchStream * FileStream
	)
{
	int Code;

	if (FileStream == NULL)
	{
		return(EXAPATCH_SUCCESS);
	}
	if (FileStream->Type == EXP_STREAM_USER)
	{
		Code = CloseExaStream( FileStream );
	}
	else
	{
		Code = CloseExaStream( FileStream );
		ExaMemFree( NULL, FileStream );
	}
	return(Code);
}

int EXP_DLLIMPORT
	ExaPatchApplyFreeParsedEntry(
	ExaPatchApplyFileEntryInfo * Entry
	)
{
	DWORD i;
	void * Ptr;

	if (Entry->OldRegionListArray)
	{
		for (i=0; i<Entry->Comm.Files.dwNumOldFiles ; i++ )
		{
			while (Entry->OldRegionListArray[i])
			{
				Ptr = Entry->OldRegionListArray[i];
				Entry->OldRegionListArray[i]
					= Entry->OldRegionListArray[i]->Next;
				ExaMemFree( NULL, Ptr );
			}
		}
		ExaMemFree( NULL, Entry->OldRegionListArray );
		Entry->OldRegionListArray = NULL;
	}
	if (Entry->NewRegionListArray)
	{
		for (i=0; i<Entry->Comm.Files.dwNumNewFiles ; i++ )
		{
			while (Entry->NewRegionListArray[i])
			{
				Ptr = Entry->NewRegionListArray[i];
				Entry->NewRegionListArray[i]
					= Entry->NewRegionListArray[i]->Next;
				ExaMemFree( NULL, Ptr );
			}
		}
		ExaMemFree( NULL, Entry->NewRegionListArray );
		Entry->NewRegionListArray = NULL;
	}
	return(ExaPatchCommFreeParsedEntry(&Entry->Comm));
}

static void ExaPatchFreeManipList( ExaPatchApplyFileManipulation ** ppHead,
		ExaPatchApplyFileManipulation ** ppTail )
{
	ExaPatchApplyFileManipulation * pManip;

	while (*ppHead)
	{
		pManip = *ppHead;
		if (pManip->OldFile)
			ExaMemFree( NULL, pManip->OldFile );
		if (pManip->IntFile)
			ExaMemFree( NULL, pManip->IntFile );
		if (pManip->IntFile2)
			ExaMemFree( NULL, pManip->IntFile2 );
		if (pManip->NewFile)
			ExaMemFree( NULL, pManip->NewFile );
		if (pManip->BackupFile)
			ExaMemFree( NULL, pManip->BackupFile );
		if (pManip->BackupFile2)
			ExaMemFree( NULL, pManip->BackupFile2 );
		*ppHead = pManip->NextManip;
		ExaMemFree( NULL, pManip );
	}
	*ppTail = NULL;

}
int EXP_DLLIMPORT
	ExaPatchApplyFreeParsedHeader(
	ExaPatchApplyFileHeaderInfo * HeaderPtr
	)
{
	DWORD i;
  void * Ptr;

	/* 
	this part needs the common portion intact 
	in order to be freed correctly
	*/
	if (HeaderPtr->SystemDirs)
	{
		for (i=0; i<HeaderPtr->Comm.Systems->dwNumSystems ; i++ )
		{
			if (HeaderPtr->SystemDirs[i])
				ExaMemFree( NULL, HeaderPtr->SystemDirs[i] );
		}
		ExaMemFree( NULL, HeaderPtr->SystemDirs );
		HeaderPtr->SystemDirs = NULL;
	}
	if (HeaderPtr->Comm.dwReserved & 1)
	{
		if (HeaderPtr->dwExtensionSize >= DESTKEY_SIZE)
		{
			if (HeaderPtr->DKApplyDir)
			{
				ExaMemFree( NULL, HeaderPtr->DKApplyDir );
				HeaderPtr->DKApplyDir = NULL;
			}
			if (HeaderPtr->DKRmDir)
			{
				ExaMemFree( NULL, HeaderPtr->DKRmDir );
				HeaderPtr->DKRmDir = NULL;
			}
		}
	}
	/* free common portion */
	ExaPatchCommFreeParsedHeader( &HeaderPtr->Comm );
	/* free apply-specific portion */
	ExaPatchFreeManipList( &HeaderPtr->ManipList, &HeaderPtr->ManipTail );
	ExaPatchFreeManipList( &HeaderPtr->LocalManipList, &HeaderPtr->LocalManipTail );
	ExaPatchFreeManipList( &HeaderPtr->RegList, &HeaderPtr->RegTail );
	while ( HeaderPtr->DupList )
	{
		if (HeaderPtr->DupList->FileName)
		{
			ExaMemFree( NULL, HeaderPtr->DupList->FileName );
		}
		Ptr = (void *) HeaderPtr->DupList;
		HeaderPtr->DupList = HeaderPtr->DupList->NextEntry;
		ExaMemFree( NULL, Ptr );
	}
# ifdef EXAPATCH_TEMPFILE_SUPPORT
	while ( HeaderPtr->TempList )
	{
		if (HeaderPtr->TempList->FileName)
		{
			ExaMemFree( NULL, HeaderPtr->TempList->FileName );
		}
		if (HeaderPtr->TempList->OldFile)
		{
			ExaMemFree( NULL, HeaderPtr->TempList->OldFile );
		}
		Ptr = (void *) HeaderPtr->TempList;
		HeaderPtr->TempList = HeaderPtr->TempList->NextEntry;
		ExaMemFree( NULL, Ptr );
	}
# endif
	if (HeaderPtr->ActualApplyDir)
	{
		ExaMemFree( NULL, HeaderPtr->ActualApplyDir );
		HeaderPtr->ActualApplyDir = NULL;
	}
	if (HeaderPtr->ErrorFileName)
	{
		ExaMemFree( NULL, HeaderPtr->ErrorFileName );
		HeaderPtr->ErrorFileName = NULL;
	}
	if (HeaderPtr->ScratchDir)
	{
		ExaMemFree( NULL, HeaderPtr->ScratchDir );
		HeaderPtr->ScratchDir = NULL;
	}
	if (HeaderPtr->BackupCFName)
	{
		ExaMemFree( NULL, HeaderPtr->BackupCFName );
		HeaderPtr->BackupCFName = NULL;
	}
	if (HeaderPtr->ErrorBuffer)
	{
		ExaMemFree( NULL, HeaderPtr->ErrorBuffer );
		HeaderPtr->ErrorBuffer = NULL;
	}
# ifdef EXAPATCH_BACKUP_SUPPORT
	if (HeaderPtr->BackupCmdFile)
	{
		CloseExaStream( HeaderPtr->BackupCmdFile );
		ExaMemFree( NULL, HeaderPtr->BackupCmdFile );
		HeaderPtr->BackupCmdFile = NULL;
	}
# endif
	if (HeaderPtr->ErrorFile)
	{
		CloseExaStream( HeaderPtr->ErrorFile );
		ExaMemFree( NULL, HeaderPtr->ErrorFile );
		HeaderPtr->ErrorFile = NULL;
	}
	return(EXAPATCH_SUCCESS);
}

int EXP_DLLIMPORT
	ExaPatchApplyVerifyFile(
	wchar_t * FullPath,
	ExaPatchFileInfo * InfoPtr,
	DWORD dwIndex,
	DWORD dwOldOrNew,
	DWORD dwNumBH,
	ExaPatchByteHandling * BHPtr,
	DWORD dwWhatToCheck
	)
{
	int Code;
	DWORD SearchFlags;
	ExaPatchApplyIgnoreRegion * IgnoreHead = NULL;
	ExaPatchApplyIgnoreRegion * ThisIgnore;
	ExaPatchDiscriminator * TheDiscrim = NULL;
	ExaPatchFileAttrib * TheAttrib;
	DWORD i;
	QWORD Origin = 0;
	QWORD Size;

	Size = ((dwOldOrNew)?InfoPtr->lpNewAttribArray:InfoPtr->lpOldAttribArray)[dwIndex].Size;
	for (i=0; i<dwIndex ; i++ )
	{
		Origin += ((dwOldOrNew)?InfoPtr->lpNewAttribArray:InfoPtr->lpOldAttribArray)[i].Size;
	}
# ifdef EXAPATCH_BYTE_SUPPORT
	/* fixup byte handling */
	if (dwOldOrNew)
	{
		/* new file */
		for (i=0; i<dwNumBH ; i++ )
		{
			if (BHPtr[i].dwType != EXP_BYTE_IGNORE && BHPtr[i].dwType != EXP_BYTE_RETAIN)
			{
				if ((BHPtr[i].qwNewPos < (Origin + Size))&& ((BHPtr[i].qwNewPos + BHPtr[i].qwLength) >= Origin) )
				{
					Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyIgnoreRegion ), (void **) &ThisIgnore ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					if (Code) goto exit;
					ThisIgnore->Next = IgnoreHead;
					IgnoreHead = ThisIgnore;
					if (BHPtr[i].qwNewPos > Origin)
					{
						ThisIgnore->Offset = BHPtr[i].qwNewPos - Origin;
					}
					else
					{
						ThisIgnore->Offset = 0;
					}
					ThisIgnore->Length = (BHPtr[i].qwNewPos + BHPtr[i].qwLength) - (Origin + ThisIgnore->Offset);
					if ((ThisIgnore->Offset + ThisIgnore->Length) > Size)
					{
						ThisIgnore->Length = Size - ThisIgnore->Offset;
					}
				}
			}
		}
	}
	else
	{
		/* old file */
		for (i=0; i<dwNumBH ; i++ )
		{
			if (BHPtr[i].dwType == EXP_BYTE_IGNORE || BHPtr[i].dwType == EXP_BYTE_RETAIN)
			{
				if ((BHPtr[i].qwOldPos < (Origin + Size))&& ((BHPtr[i].qwOldPos + BHPtr[i].qwLength) >= Origin) )
				{
					Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyIgnoreRegion ), (void **) &ThisIgnore ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					if (Code) goto exit;
					ThisIgnore->Next = IgnoreHead;
					IgnoreHead = ThisIgnore;
					if (BHPtr[i].qwOldPos > Origin)
					{
						ThisIgnore->Offset = BHPtr[i].qwOldPos - Origin;
					}
					else
					{
						ThisIgnore->Offset = 0;
					}
					ThisIgnore->Length = (BHPtr[i].qwOldPos + BHPtr[i].qwLength) - (Origin + ThisIgnore->Offset);
					if ((ThisIgnore->Offset + ThisIgnore->Length) > Size)
					{
						ThisIgnore->Length = Size - ThisIgnore->Offset;
					}
				}
			}
		}
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (dwNumBH)
	{
		return(EXAPATCH_NOT_SUPPORTED);
	}
#  endif
# endif
	/* calculate TheDiscrim */
	for (i=0; i<InfoPtr->dwNumDiscrims ; i++ )
	{
		if (dwIndex == ((dwOldOrNew)?InfoPtr->lpDiscArray[i].NewIndex:InfoPtr->lpDiscArray[i].OldIndex))
		{
			TheDiscrim = &InfoPtr->lpDiscArray[i];
			i = InfoPtr->dwNumDiscrims;
		}
	}
	/* calculate SearchFlags */
	TheAttrib = &((dwOldOrNew)?InfoPtr->lpNewAttribArray:InfoPtr->lpOldAttribArray)[dwIndex];
	TheAttrib->Flags = 0;
	SearchFlags = EXP_SEARCH_VERIFY_ATTRIB;
	if (dwWhatToCheck & EXP_APPLY_CHECK_TIME)
	{
		/* Probably shouldn't check the Create date */
		/* TheAttrib->Flags |= EXP_ATTRIB_CHANGED_DATE | EXP_ATTRIB_CREATE_DATE; */
		TheAttrib->Flags |= EXP_ATTRIB_CHANGED_DATE;
	}
	if (dwWhatToCheck & EXP_APPLY_CHECK_ATTRIB)
	{
		TheAttrib->Flags |= EXP_ATTRIB_ATTRIBUTE;
	}
	if (dwWhatToCheck & EXP_APPLY_CHECK_SIZE)
	{
		TheAttrib->Flags |= EXP_ATTRIB_SIZE;
	}
	if (dwWhatToCheck & EXP_APPLY_CHECK_IGNORETIMEZONE)
	{
		SearchFlags |= EXP_SEARCH_IGNORE_TIMEZONE;
	}
	if (TheDiscrim)
	{
		SearchFlags |= dwOldOrNew?EXP_SEARCH_CHECK_NEW_DISC:EXP_SEARCH_CHECK_OLD_DISC;
	}
	if (dwWhatToCheck & EXP_APPLY_CHECK_CHECKSUM)
	{
		SearchFlags |= EXP_SEARCH_VERIFY_CKSUM;
	}
# ifdef EXAPATCH_UPDATELINK_SUPPORT
	if (dwWhatToCheck & EXP_APPLY_CHECK_PROCESSLINKS)
	{
		SearchFlags |= EXP_SEARCH_PROCESSLINKS;
	}
# endif
	/* call VerifyFile */
	Code = VerifyFile( FullPath, 
		SearchFlags, 
		TheAttrib,
		(dwOldOrNew)?&InfoPtr->lpNewCksumArray[dwIndex]:&InfoPtr->lpOldCksumArray[dwIndex],
		IgnoreHead,
		NULL,
		TheDiscrim,
		NULL, 0, NULL, 0 );

exit:
	while (IgnoreHead)
	{
		ThisIgnore = IgnoreHead;
		IgnoreHead = IgnoreHead->Next;
		ExaMemFree( NULL, ThisIgnore );
	}
	return(Code);
}
# endif /* ATTOPATCH */

/* 	Handle-based routines (synchronized) */
PLATWORD EXP_DLLIMPORT
	ExaPatchApplyOpen( void )
{
	ExaPatchApplyChannel * TheChannel = NULL;
	int Code;

	# ifdef EVAL
#  ifdef UNIX_SUPPORT
        PrintEvalMessage();
#  else
	if (PrintBanner ())
	{

		// Expired. Exit! (with error message)
    MessageBox( 0, 
  		"The evaluation period has expired!\n"
			"Please contact Pocket Soft to purchase software\n"
			"or to obtain an evaluation period extension.\n\n"
			"(C) Copyright 2003-2016 Pocket Soft Inc.\n"
			"All Rights Reserved\n\n"
			"+1 713-460-5600  FAX +1 713-460-2651\n"
			"Pocket Soft Inc.\n"
			"P.O. Box 821049\nHouston, TX 77282\nUSA",
			"RTPatch Server Evaluation Software",
			MB_TASKMODAL | MB_OK | MB_ICONINFORMATION );
	  return (EXAPATCH_UNSPECIFIED_ERROR);
	}

  MessageBox( 0, 
  	"This fully functional evaluation copy may be used\n"
	  "SOLELY to determine if RTPatch Server is suitable\n"
	  "for your needs.  Any other use is prohibited\n\n"
    "(C) Copyright 2003-2016 Pocket Soft Inc.\n"
	  "All Rights Reserved\n\n"
	  "+1 713-460-5600  FAX +1 713-460-2651\n"
	  "Pocket Soft Inc.\n"
	  "P.O. Box 821049\nHouston, TX 77282\nUSA",
	  "RTPatch Server Evaluation Software",
	  MB_TASKMODAL | MB_OK | MB_ICONINFORMATION );
#  endif
# endif

	Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyChannel ), (void **) &TheChannel ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (Code == EXAPATCH_SUCCESS)
	{
		TheChannel->Signature = CHANNEL_SIGNATURE;
# ifdef _WIN32
		InitializeCriticalSection( &TheChannel->CritSect );
# else
# ifndef ATTOPATCH
		pthread_mutex_init( &TheChannel->Mutex, NULL );
# endif
# endif
		TheChannel->BusyFlag = 0;
		TheChannel->BufferBreak = 0;
# ifdef EXAPATCH_CONTAINER_SUPPORT
		TheChannel->dwPatchIndex = 0xffffffffU;
# endif
# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
		TheChannel->pChannelBuffer = NULL;
		TheChannel->dwCBSize = 0;
# endif
# ifdef EXAPATCH_TEMPDIR_SUPPORT
		TheChannel->pTempDir = NULL;
# endif
	}
	if ((Code != EXAPATCH_SUCCESS) && (NULL != TheChannel))
	{
		(void)ExaMemFree( NULL, TheChannel );
		TheChannel = NULL;
	}
	return((PLATWORD)TheChannel); /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
}


int EXP_DLLIMPORT
	ExaPatchApplyClose( PLATWORD Handle )
{
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle; /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
	int Code;

	Code = AcquireChannel( Handle );
	if (0 != Code)
	{
		return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
		
	TheChannel->Signature = 0;
# ifdef _WIN32
	DeleteCriticalSection( &TheChannel->CritSect );
# else
# ifndef ATTOPATCH
	pthread_mutex_destroy( &TheChannel->Mutex );
# endif
# endif
# ifdef EXAPATCH_TEMPDIR_SUPPORT
	if (TheChannel->pTempDir)
	{
		(void)ExaMemFree( NULL, TheChannel->pTempDir );
		TheChannel->pTempDir = NULL;
	}
# endif
	(void)ExaMemFree( NULL, TheChannel );
	return(EXAPATCH_SUCCESS);
}
# ifndef ATTOPATCH
# ifndef _WIN32
static QWORD StaticMemSize = 0;
# endif
int /* ErrCode */
	EXP_DLLIMPORT
	ExaPatchApplySetBuffering( PLATWORD Handle, QWORD qwBreak, BOOL bAmt )
{
	int Code;

# ifndef _WIN32
		char * DirNames[]={"","/compat/linux","/usr/compat/linux"};
# endif
	/* if bAmt is nonzero, qwBreak is interpreted as an amount, if bAmt is zero, it is taken to be an integral percentage of system memory */

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);

	if (bAmt)
	{
		((ExaPatchApplyChannel *)Handle)->BufferBreak = qwBreak;
	}
	else
	{
		QWORD MemSize;
# ifdef _WIN32
		MEMORYSTATUS MS;

		GlobalMemoryStatus( &MS );
		MemSize = (QWORD) MS.dwTotalPhys;
# else
		if (StaticMemSize)
		{
			MemSize = StaticMemSize;
		}
		else
		{
			/* 
			This APPEARS to be (unbelievably) the MOST portable way of finding out
			the memory size under *nix...
			
			parse ~/proc/meminfo (where ~ is either root, or /compat/linux or /usr/compat/linux)
			looking for a line
			"MemTotal:" 
			then read the following number, and interpret it as a "KB"
			
			*/
			int inx;
			int Found=0;
			int Finished=0;
			char Buffer[100];
			char Line[1000];
			FILE * ProcMeminfo;
			
			for (inx=0;(inx<3)&&!Found;inx++)
			{
				//@@CACING PENTEST strcpy( Buffer, DirNames[inx] );
				strncpy( Buffer, DirNames[inx],(int)strlen(DirNames[inx]) );
				//@@CACING PENTEST strcat( Buffer, "/proc/meminfo");
				strncat( Buffer, "/proc/meminfo",13);
				ProcMeminfo=fopen( Buffer, "r" );
				if (ProcMeminfo)
				{
					Finished=0;
					while (!Finished)
					{
						if(fgets( Line, 1000, ProcMeminfo ))
						{
							if (!memcmp(Line,"MemTotal:",9))
							{
								Found = Finished = 1;
								sscanf( &Line[9], " %lld", (long long *) &MemSize );
								MemSize = MemSize << 10;
								StaticMemSize = MemSize;
							}
						}
						else
						{
							Finished = 1;
						}
					}
					fclose( ProcMeminfo );
				}
			}
			if (!Found)
			{
				/* 
				Oh, well, ... do SOMETHING even if it's wrong! 
				If no proc/meminfo file can be located, set memory size to 1 GB
				*/
				StaticMemSize = MemSize = 1 << 30;
			}
		}
# endif
		MemSize = (MemSize + 0x3ff) >> 10;
		MemSize *= qwBreak;
		MemSize = MemSize / 100;
		MemSize <<= 10;
		((ExaPatchApplyChannel *)Handle)->BufferBreak = MemSize;
	}
	if (((ExaPatchApplyChannel *)Handle)->BufferBreak > 0x40000000)
	{
		((ExaPatchApplyChannel *)Handle)->BufferBreak = 0x40000000;
	}
	ReleaseChannel( Handle );
	return(Code);

		
}
static DWORD DoFlags[]=
{
	EXP_PATCH_MANIP_PHASE1,
	EXP_PATCH_MANIP_PHASE2,
	EXP_PATCH_MANIP_PHASE3,
	EXP_PATCH_MANIP_PHASE4,
};
static DWORD UndoFlags[]=
{
	EXP_PATCH_MANIP_UNDO_PHASE1,
	EXP_PATCH_MANIP_UNDO_PHASE2,
	EXP_PATCH_MANIP_UNDO_PHASE3
};
static int ExaPatchDoManipList( ExaPatchApplyFileHeaderInfo * HeaderPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		ExaPatchApplyFileManipulation ** ppHead,
		ExaPatchApplyFileManipulation ** ppTail )
{
	int Code = EXAPATCH_SUCCESS;
	int i;
	int BadPhase = 0;
	ExaPatchApplyFileManipulation * pManip;
	ExaPatchApplyFileManipulation * pBadManip = NULL;
	DWORD DelayFlag;

# ifdef EXAPATCH_ALLOWDELAY_SUPPORT
	DelayFlag = (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_FORCEDELAY)?2
		:((HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_ALLOWDELAY)?1
		:0);
# else
	DelayFlag = 0;
# endif
	for (i=0; (i<4) && (Code == EXAPATCH_SUCCESS) ; i+= (DelayFlag)?2:1 )
	{
		for (pManip = *ppHead; (Code == EXAPATCH_SUCCESS) && pManip ; pManip = pManip->NextManip )
		{
			pManip->FileManipFlags &= ~(EXP_PATCH_MANIP_PHASE1 
				| EXP_PATCH_MANIP_PHASE2
				| EXP_PATCH_MANIP_PHASE3
				| EXP_PATCH_MANIP_PHASE4
				| EXP_PATCH_MANIP_UNDO_PHASE1
				| EXP_PATCH_MANIP_UNDO_PHASE2
				| EXP_PATCH_MANIP_UNDO_PHASE3
				);
			pManip->FileManipFlags |= DoFlags[i];
			Code = ExaPatchDoFileManip( pManip,
					HeaderPtr,
					DelayFlag,
					//(HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_USERFILEMANIP)?ProgressCallBack:NULL,
					ProgressCallBack,
					CallbackHandle );
			if (Code)
			{
				pBadManip = pManip;
				BadPhase = i;
			}
		}
		if (Code)
		{
			for (pManip = pBadManip; pManip ; pManip = pManip->PrevManip )
			{
				pManip->FileManipFlags &= ~(EXP_PATCH_MANIP_PHASE1 
					| EXP_PATCH_MANIP_PHASE2
					| EXP_PATCH_MANIP_PHASE3
					| EXP_PATCH_MANIP_PHASE4
					| EXP_PATCH_MANIP_UNDO_PHASE1
					| EXP_PATCH_MANIP_UNDO_PHASE2
					| EXP_PATCH_MANIP_UNDO_PHASE3
					);
				pManip->FileManipFlags |= UndoFlags[i];
				ExaPatchDoFileManip( pManip,
					HeaderPtr,
					DelayFlag,
					//(HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_USERFILEMANIP)?ProgressCallBack:NULL,
					ProgressCallBack,
					CallbackHandle );
			}
		}
	}
	if (Code)
	{
		for (i = BadPhase - (DelayFlag?2:1); i>0 ; i -= (DelayFlag?2:1) )
		{
			for (pManip = *ppTail; pManip ; pManip = pManip->PrevManip )
			{
				pManip->FileManipFlags &= ~(EXP_PATCH_MANIP_PHASE1 
					| EXP_PATCH_MANIP_PHASE2
					| EXP_PATCH_MANIP_PHASE3
					| EXP_PATCH_MANIP_PHASE4
					| EXP_PATCH_MANIP_UNDO_PHASE1
					| EXP_PATCH_MANIP_UNDO_PHASE2
					| EXP_PATCH_MANIP_UNDO_PHASE3
					);
				pManip->FileManipFlags |= UndoFlags[i];
			  ExaPatchDoFileManip( pManip,
					HeaderPtr,
					DelayFlag,
					//(HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_USERFILEMANIP)?ProgressCallBack:NULL,
					ProgressCallBack,
					CallbackHandle );

			}
		}
	}
	else
	{
		ExaPatchFreeManipList( ppHead, ppTail );
	}
	return(Code);
}

static int ExaPatchDoLocal( ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		 )
{
  int Code = EXAPATCH_SUCCESS;

# ifdef EXAPATCH_UNDO_SUPPORT
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_UNDO)
	{
		/* move to global list */
    Code = EXAPATCH_SUCCESS;
		if (HeaderPtr->LocalManipList)
		{
			HeaderPtr->LocalManipList->PrevManip = HeaderPtr->ManipTail;
			if (HeaderPtr->ManipTail)
			{
				HeaderPtr->ManipTail->NextManip = HeaderPtr->LocalManipList;
				HeaderPtr->ManipTail = HeaderPtr->LocalManipTail;
			}
			else
			{
				HeaderPtr->ManipList = HeaderPtr->LocalManipList;
				HeaderPtr->ManipTail = HeaderPtr->LocalManipTail;
			}
			HeaderPtr->LocalManipList = 
				HeaderPtr->LocalManipTail = NULL;

		}
		else
		{
			HeaderPtr->LocalManipTail = NULL;
		}
	}
	else
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_UNDO)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
	}
	else
#  endif
# endif
	{
		/* perform the indicated file manipulations */
		Code = ExaPatchDoManipList( HeaderPtr, ProgressCallBack, CallbackHandle,
				&HeaderPtr->LocalManipList, &HeaderPtr->LocalManipTail );
	}
	return(Code);
}

static int ExaPatchUndoLocal( ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		 )
{
	ExaPatchApplyFileManipulation * pManip;
	int Code = EXAPATCH_SUCCESS;
	/* delete all the temp files mentioned in the LocalManipList */
	/*  (that is, all the files that have a non-NULL IntFile entry) */
	for (pManip = HeaderPtr->LocalManipList; pManip; pManip = pManip->NextManip)
	{
		if (pManip->IntFile)
		{
			if (0 == (EXP_PATCH_MANIP_SPECIAL_HANDLING & pManip->FileManipFlags))
			{
				Code = ExaDelete( pManip->IntFile );
				if (Code) goto exit;
			}
		}
	}
	ExaPatchFreeManipList( &HeaderPtr->LocalManipList, &HeaderPtr->LocalManipTail );
exit:
	return(Code);
}

# ifdef EXAPATCH_UNDO_SUPPORT
static int ExaPatchDoGlobal( ExaPatchApplyFileHeaderInfo * HeaderPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Pandle),
		PLATWORD CallbackHandle
	 )
{
	int Code = EXAPATCH_SUCCESS;
	/* do all the file manipulations in the Global List */
	/* check the ApplyFlags to see if Delay is allowed */
	Code = ExaPatchDoManipList( HeaderPtr, ProgressCallBack, CallbackHandle,
			&HeaderPtr->ManipList, &HeaderPtr->ManipTail );
	return(Code);
}

static int ExaPatchUndoGlobal( ExaPatchApplyFileHeaderInfo * HeaderPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
 )
{
	ExaPatchApplyFileManipulation * pManip;
	int Code = EXAPATCH_SUCCESS;
	/* delete all the temp files mentioned in the ManipList */
	/*  (that is, all the files that have a non-NULL IntFile entry) */
	for (pManip = HeaderPtr->ManipList; pManip; pManip = pManip->NextManip)
	{
		if (pManip->IntFile)
		{
			if (0 == (EXP_PATCH_MANIP_SPECIAL_HANDLING & pManip->FileManipFlags))
			{
				Code = ExaDelete( pManip->IntFile );
				if (Code) goto exit;
			}
		}
	}
	ExaPatchFreeManipList( &HeaderPtr->ManipList, &HeaderPtr->ManipTail );
exit:
	return(Code);
}
# endif
static int ExaPatchKillTemp( ExaPatchApplyFileHeaderInfo * HeaderPtr )
{
# ifdef EXAPATCH_TEMPFILE_SUPPORT
	void * Ptr;
	/* delete all the files in the temp file list (and perform any */
	/*  necessary renaming) */
	while (HeaderPtr->TempList)
	{
		ExaDelete( HeaderPtr->TempList->FileName );
		if (HeaderPtr->TempList->OldFile)
		{
			ExaRename( HeaderPtr->TempList->OldFile,
					HeaderPtr->TempList->FileName );
			ExaMemFree( NULL, HeaderPtr->TempList->OldFile );
		}
		ExaMemFree( NULL, HeaderPtr->TempList->FileName );
		Ptr = (wchar_t *) HeaderPtr->TempList;
		HeaderPtr->TempList = HeaderPtr->TempList->NextEntry;
		ExaMemFree( NULL, Ptr );
	}
# endif
	return(EXAPATCH_SUCCESS);
}

static DWORD ExaPatchUserLevelMask[4] = {
	/*USER_NONE: */
	EXP_PATCH_APPLY_USERFILEMANIP | EXP_PATCH_APPLY_FULLPATHDISPLAY,
	/*USER_BACKUP */
	EXP_PATCH_APPLY_BACKUP | EXP_PATCH_APPLY_KEEPTEMP 
		| EXP_PATCH_APPLY_USERFILEMANIP | EXP_PATCH_APPLY_FULLPATHDISPLAY,
	/*USER_BASIC: */
	EXP_PATCH_APPLY_BACKUP | EXP_PATCH_APPLY_KEEPTEMP
		| EXP_PATCH_APPLY_ONEFILEONLY | EXP_PATCH_APPLY_VERIFYONLY 
		| EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_QUIET 
		| EXP_PATCH_APPLY_UNDO | EXP_PATCH_APPLY_VERBOSE
		| EXP_PATCH_APPLY_USERFILEMANIP | EXP_PATCH_APPLY_FULLPATHDISPLAY,
	/*USER_ADVANCED */
	EXP_PATCH_APPLY_BACKUP | EXP_PATCH_APPLY_ERRORFILE 
		| EXP_PATCH_APPLY_ONEFILEONLY	| EXP_PATCH_APPLY_VERIFYONLY 
		| EXP_PATCH_APPLY_CONFIRMSYSTEMS | EXP_PATCH_APPLY_IGNOREMISSING
		| EXP_PATCH_APPLY_KEEPTEMP | EXP_PATCH_APPLY_LISTONLY
		| EXP_PATCH_APPLY_MESSAGES | EXP_PATCH_APPLY_PATHSEARCH 
		| EXP_PATCH_APPLY_SUBDIRSEARCH | EXP_PATCH_APPLY_TIMEZONEIGNORE
		| EXP_PATCH_APPLY_UNDO | EXP_PATCH_APPLY_VERBOSE
		| EXP_PATCH_APPLY_USERFILEMANIP | EXP_PATCH_APPLY_FULLPATHDISPLAY
};

static int ExaPatchEntryParseWork(
		ExaPatchStream * InputStream,	/*  assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		DWORD HistoryFlag
	)
{
	int Code;
	unsigned short HeaderWord;
# ifdef EXAPATCH_EH_SUPPORT
	QWORD HeaderPos;
# endif
	DWORD i;
# ifdef EXAPATCH_BYTE_SUPPORT
	DWORD j;
# endif
	DWORD Count;
	unsigned short WBuf;
	DWORD DWBuf;
	QWORD QWBuf;
# ifdef EXAPATCH_BYTE_SUPPORT
	DWORD BHFlags=0;
	QWORD Origin, FileEnd, Start, End;
	ExaPatchApplyIgnoreRegion * TheRegion;
# endif

# ifndef EXAPATCH_HISTORY_SUPPORT
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (HistoryFlag)
	{
		return(EXAPATCH_NOT_SUPPORTED);
	}
#  endif
# endif

	/*HistoryFlag == TRUE is used when parsing history entries */
	memset( EntryPtr, 0, sizeof( ExaPatchApplyFileEntryInfo ) );
	EntryPtr->qwEntryPos = InputStream->CurPos;
	EntryPtr->Comm.dwSize = sizeof( ExaPatchCommFileEntryInfo );
	EntryPtr->Comm.nSubDirIndex = -1;
	EntryPtr->Comm.nSystemIndex = -1;
	if (Code = GetWord(InputStream, &HeaderWord) ) goto exit;
	EntryPtr->wPatchType = HeaderWord;
	if (HeaderWord & 0x10)
	{
		/*set FileFlags */
		if (Code = GetDword( InputStream, &DWBuf) ) goto exit;
		EntryPtr->Comm.dwFileFlags = (DWORD) DWBuf;
		EntryPtr->Comm.dwWhichFileFlags = (HeaderPtr->Comm.dwFileFlags) ^ (EntryPtr->Comm.dwFileFlags);
		EntryPtr->Comm.dwFileFlags ^= EntryPtr->Comm.dwWhichFileFlags & HeaderPtr->IndivFlagsOverridden;
	}
	else
	{
		EntryPtr->Comm.dwWhichFileFlags = 0;
		EntryPtr->Comm.dwFileFlags = HeaderPtr->Comm.dwFileFlags;
	}

	if (!HistoryFlag && (HeaderWord & 0x20) )
	{
		/* set EntryName */
		if (Code = GetVarStr( InputStream, &EntryPtr->Comm.lpEntryName) ) goto exit;
# ifndef _WIN32
		Slashify( EntryPtr->Comm.lpEntryName );
# endif
	}
	else
	{
		EntryPtr->Comm.lpEntryName = NULL;
	}
	if (!HistoryFlag && (HeaderWord & 0x40) )
	{
		/* set SubDirIndex */
		if (Code = GetVarIndex( InputStream, &QWBuf )	) goto exit;
		Validate(QWBuf, HeaderPtr->Comm.SubDirs->dwNumSubDirs);
# ifdef EXAPATCH_SUBDIR_SUPPORT
#  ifdef _WIN32
		EntryPtr->Comm.nSubDirIndex = (__int64) QWBuf;
#  else
		EntryPtr->Comm.nSubDirIndex = (long long) QWBuf;
#  endif
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
#  else
		EntryPtr->Comm.nSubDirIndex = -1;
#  endif
# endif
	}
	else
	{
		EntryPtr->Comm.nSubDirIndex = -1;
	}

	if (!HistoryFlag && (HeaderWord & 0x80) )
	{
		/* set SysIndex */
		if (Code = GetVarIndex( InputStream, &QWBuf )	) goto exit;
		Validate(QWBuf, HeaderPtr->Comm.Systems->dwNumSystems);
# ifdef _WIN32
		EntryPtr->Comm.nSystemIndex = (__int64) QWBuf;
# else
		EntryPtr->Comm.nSystemIndex = (long long) QWBuf;
# endif
	}
	else
	{
		EntryPtr->Comm.nSystemIndex = -1;
	}

	if (HeaderWord & 0x100)
	{
		/* set AttribRetain */
		if (Code = GetWord( InputStream, &WBuf )) goto exit;
		EntryPtr->Comm.dwAttribRetain = (DWORD) WBuf;
		EntryPtr->Comm.dwAttribWhich = (HeaderPtr->Comm.dwAttribRetain)^(EntryPtr->Comm.dwAttribRetain);
	}
	else
	{
		EntryPtr->Comm.dwAttribWhich = 0;
		EntryPtr->Comm.dwAttribRetain = HeaderPtr->Comm.dwAttribRetain;
	}
	if (HeaderWord & 0x200)
	{
		/* set Hooks */
		if (Code = GetVarStr( InputStream, &EntryPtr->Comm.lpDoBefore) ) goto exit;
		if (Code = GetVarStr( InputStream, &EntryPtr->Comm.lpDoAfter) ) goto exit;
	}
	else
	{
		EntryPtr->Comm.lpDoBefore = EntryPtr->Comm.lpDoAfter = NULL;
	}
	if (HeaderWord & 0x400)
	{
		/* get StatusFlags */
		if (Code = GetWord( InputStream, &WBuf )) goto exit;
		EntryPtr->wModFlags = WBuf;
	}
# ifdef EXAPATCH_BYTE_SUPPORT
	if (HeaderWord & 0x800)
	{
		/* parse ByteHandling */
		/* first, the OLD */
		if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
			Validate(QWBuf, MAX_NUM_BH);
		EntryPtr->Comm.dwNumBH = 
			Count = (DWORD) QWBuf;
		if (Count)
		{
			Code = ExaMemAlloc( NULL, Count*sizeof( ExaPatchByteHandling ), (void **) &EntryPtr->Comm.BHPtr); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			memset( EntryPtr->Comm.BHPtr, 0, Count*sizeof( ExaPatchByteHandling ) );
			BHFlags |= 1;
		}
		for (i=0; i<Count ; i++ )
		{
			EntryPtr->Comm.BHPtr[i].dwType = EXP_BYTE_IGNORE;
			if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.BHPtr[i].qwOldPos ) ) goto exit;
			if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.BHPtr[i].qwLength ) ) goto exit;
		}
		/* now, resize for the NEW */
		if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
			Validate(QWBuf, MAX_NUM_BH);
		Count = (DWORD) QWBuf;
		if (Count)
		{
      unsigned char * Buffer;

			Code = ExaMemAlloc( NULL, (Count + EntryPtr->Comm.dwNumBH)*sizeof( ExaPatchByteHandling ),(void **) &Buffer); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			memset( Buffer, 0, (Count + EntryPtr->Comm.dwNumBH)*sizeof(ExaPatchByteHandling) );
			BHFlags |= 2;
			if (EntryPtr->Comm.dwNumBH)
			{
				memcpy( Buffer, EntryPtr->Comm.BHPtr, (EntryPtr->Comm.dwNumBH)*sizeof( ExaPatchByteHandling ) );
				ExaMemFree( NULL, EntryPtr->Comm.BHPtr );
			}
			EntryPtr->Comm.BHPtr = (ExaPatchByteHandling *) Buffer;
			Buffer = NULL;
			for (i=0, j=EntryPtr->Comm.dwNumBH, EntryPtr->Comm.dwNumBH += Count; 
				i < Count; 
				i++, j++ )
			{
				if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.BHPtr[j].qwNewPos ) ) goto exit;
				if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.BHPtr[j].qwLength ) ) goto exit;
				if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
				if (QWBuf)
				{
					EntryPtr->Comm.BHPtr[j].dwType = EXP_BYTE_READ;
					if (Code = GetVarStr( InputStream, &EntryPtr->Comm.BHPtr[j].lpFileName) ) goto exit;
# ifndef _WIN32
					Slashify( EntryPtr->Comm.BHPtr[j].lpFileName );
# endif
					if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.BHPtr[j].qwOldPos) ) goto exit;
				}
				else
				{
					EntryPtr->Comm.BHPtr[j].dwType = EXP_BYTE_NEWIGNORE;
				}
			}
		}
	}
	else
# else
#  ifdef EXAPATCH_SILENT_UNSUPPORT
	if (HeaderWord & 0x800)
	{
		/* parse ByteHandling */
		/* first, the OLD */
		if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
			Validate(QWBuf, MAX_NUM_BH);
		Count = (DWORD) QWBuf;
		for (i=0; i<Count ; i++ )
		{
			if (Code = GetVarIndex( InputStream, &QWBuf ) ) goto exit;
			if (Code = GetVarIndex( InputStream, &QWBuf ) ) goto exit;
		}
		/* now, resize for the NEW */
		if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
			Validate(QWBuf, MAX_NUM_BH);
		Count = (DWORD) QWBuf;
		if (Count)
		{
      wchar_t * Buffer;

			for (i=0; i < Count; i++ )
			{
				if (Code = GetVarIndex( InputStream, &QWBuf ) ) goto exit;
				if (Code = GetVarIndex( InputStream, &QWBuf ) ) goto exit;
				if (Code = GetVarIndex( InputStream, &QWBuf ) ) goto exit;
				if (QWBuf)
				{
					if (Code = GetVarStr( InputStream, &Buffer) ) goto exit;
					if (Buffer)
						ExaMemFree( NULL, Buffer );
					if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
				}
			}
		}
	}
	// NOTE: the lack of an "else" here is intentional
#  else
	if (HeaderWord & 0x800)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
	else
#  endif
# endif
	{
		EntryPtr->Comm.dwNumBH = 0;
		EntryPtr->Comm.BHPtr = NULL;
	}

	if (HeaderWord & 0x1000)
	{
		/* parse File Block */
# ifdef EXAPATCH_TEMPFILE_SUPPORT
		if (EXAPATCH_TEMP_TYPE == (HeaderWord & 0xf))
		{
			/* special TEMP format (varstr, varindex) */
			EntryPtr->Comm.Files.dwNumOldFiles = 0;
			EntryPtr->Comm.Files.dwNumNewFiles = 1;
			EntryPtr->Comm.Files.lpOldAttribArray = NULL;
			EntryPtr->Comm.Files.lpOldCksumArray = NULL;
			EntryPtr->Comm.Files.lpOldPathArray = NULL;
			Code = ExaMemAlloc( NULL, sizeof( ExaPatchFileChecksum ), (void **) &EntryPtr->Comm.Files.lpNewCksumArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			Code = ExaMemAlloc( NULL, sizeof( ExaPatchFileAttrib ), (void **) &EntryPtr->Comm.Files.lpNewAttribArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			Code = ExaMemAlloc( NULL, sizeof( void * ), (void **) &EntryPtr->Comm.Files.lpNewPathArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			memset( EntryPtr->Comm.Files.lpNewCksumArray, 0, sizeof( ExaPatchFileChecksum ) );
			memset( EntryPtr->Comm.Files.lpNewAttribArray, 0, sizeof( ExaPatchFileAttrib ) );
			if (Code = GetVarStr( InputStream, EntryPtr->Comm.Files.lpNewPathArray) ) goto exit;
# ifndef _WIN32
			Slashify( *EntryPtr->Comm.Files.lpNewPathArray );
# endif
			if (Code = GetVarIndex(InputStream, &EntryPtr->Comm.Files.lpNewAttribArray->Size)) goto exit;
		}
		else
# else
		if (EXAPATCH_TEMP_TYPE == (HeaderWord & 0xf))
		{
			Code = EXAPATCH_NOT_SUPPORTED;
			goto exit;
		}
# endif		
		{
			/* Old files first... */
			if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
			Validate(QWBuf, MAX_GRP_FILES);
			Count = EntryPtr->Comm.Files.dwNumOldFiles = (DWORD) QWBuf;
			if (Count)
			{
# ifdef EXAPATCH_HISTORY_SUPPORT
				if (HistoryFlag && (0 == (HeaderWord & 0x20)))
				{
					/* special OLD History version format */
					EntryPtr->Comm.Files.lpOldAttribArray = NULL;
					EntryPtr->Comm.Files.lpOldPathArray = NULL;
					if (HeaderWord & 0x40)
					{
						/* checksum only */
						Code = ExaMemAlloc( NULL, Count*sizeof(ExaPatchFileChecksum), (void **) &EntryPtr->Comm.Files.lpOldCksumArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						if (Code) goto exit;
						for (i=0; i<Count ; i++ )
						{
							if (Code = GetBytes( InputStream, &EntryPtr->Comm.Files.lpOldCksumArray[i], 10) ) goto exit;
						}
					}
					else
					{
						EntryPtr->Comm.Files.lpOldCksumArray = NULL;
					}
				}
				else
# endif				
				{
					Code = ExaMemAlloc( NULL, Count*sizeof(void *), (void **) &EntryPtr->Comm.Files.lpOldPathArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					if (Code) goto exit;
					Code = ExaMemAlloc( NULL, Count*sizeof(ExaPatchFileAttrib), (void **) &EntryPtr->Comm.Files.lpOldAttribArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					if (Code) goto exit;
					Code = ExaMemAlloc( NULL, Count*sizeof(ExaPatchFileChecksum), (void **) &EntryPtr->Comm.Files.lpOldCksumArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					if (Code) goto exit;
					for (i=0; i<Count ; i++ )
					{
						if (Code = GetVarStr( InputStream, &EntryPtr->Comm.Files.lpOldPathArray[i]) ) goto exit;
# ifndef _WIN32
						Slashify( EntryPtr->Comm.Files.lpOldPathArray[i] );
# endif
						if (Code = GetVarIndex( InputStream, &QWBuf ) ) goto exit;
# ifndef QWORD_IS_DWORD
						Validate(QWBuf, 0x100000000);
# endif
						EntryPtr->Comm.Files.lpOldAttribArray[i].Attributes = (DWORD) QWBuf;
						if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.Files.lpOldAttribArray[i].Size) ) goto exit;
						if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
						memcpy( &EntryPtr->Comm.Files.lpOldAttribArray[i].ChangedTime, &QWBuf, sizeof( FILETIME ) );
						if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
						memcpy( &EntryPtr->Comm.Files.lpOldAttribArray[i].CreateTime, &QWBuf, sizeof( FILETIME ) );
						if (Code = GetBytes( InputStream, &EntryPtr->Comm.Files.lpOldCksumArray[i], 10) ) goto exit;
					}
				}
			}
# ifdef EXAPATCH_ADDFILE_SUPPORT
			else
			{
				EntryPtr->Comm.Files.lpOldAttribArray = NULL;
				EntryPtr->Comm.Files.lpOldPathArray = NULL;
				EntryPtr->Comm.Files.lpOldCksumArray = NULL;
			}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
			else
			{
				Code = EXAPATCH_NOT_SUPPORTED;
				goto exit;
			}
#  endif
# endif
			/* Next, the new files... */
			if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
			Validate(QWBuf, MAX_GRP_FILES);
			Count = EntryPtr->Comm.Files.dwNumNewFiles = (DWORD) QWBuf;
			if (Count)
			{
				Code = ExaMemAlloc( NULL, Count*sizeof(void *), (void **) &EntryPtr->Comm.Files.lpNewPathArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				if (Code) goto exit;
				Code = ExaMemAlloc( NULL, Count*sizeof(ExaPatchFileAttrib), (void **) &EntryPtr->Comm.Files.lpNewAttribArray);  /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				if (Code) goto exit;
				Code = ExaMemAlloc( NULL, Count*sizeof(ExaPatchFileChecksum), (void **) &EntryPtr->Comm.Files.lpNewCksumArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				if (Code) goto exit;
				for (i=0; i<Count ; i++ )
				{
					if (Code = GetVarStr( InputStream, &EntryPtr->Comm.Files.lpNewPathArray[i]) ) goto exit;
# ifndef _WIN32
					Slashify( EntryPtr->Comm.Files.lpNewPathArray[i] );
# endif
					if (Code = GetVarIndex( InputStream, &QWBuf ) ) goto exit;
# ifndef QWORD_IS_DWORD
						Validate(QWBuf, 0x100000000);
# endif
					EntryPtr->Comm.Files.lpNewAttribArray[i].Attributes = (DWORD) QWBuf;
					if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.Files.lpNewAttribArray[i].Size) ) goto exit;
					if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
					memcpy( &EntryPtr->Comm.Files.lpNewAttribArray[i].ChangedTime, &QWBuf, sizeof( FILETIME ) );
					if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
					memcpy( &EntryPtr->Comm.Files.lpNewAttribArray[i].CreateTime, &QWBuf, sizeof( FILETIME ) );
					if (Code = GetBytes( InputStream, &EntryPtr->Comm.Files.lpNewCksumArray[i], 10) ) goto exit;
				}
			}
			else
			{
				EntryPtr->Comm.Files.lpNewAttribArray = NULL;
				EntryPtr->Comm.Files.lpNewPathArray = NULL;
				EntryPtr->Comm.Files.lpNewCksumArray = NULL;
			}
			/* finally, the discriminators */
			if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
			Validate(QWBuf, MAX_GRP_FILES);
			Count = EntryPtr->Comm.Files.dwNumDiscrims = (DWORD) QWBuf;
			if (Count)
			{
				Code = ExaMemAlloc( NULL, Count*sizeof(ExaPatchDiscriminator), (void **) &EntryPtr->Comm.Files.lpDiscArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				if (Code) goto exit;
				for (i=0; i<Count ; i++ )
				{
					if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
					Validate(QWBuf, EntryPtr->Comm.Files.dwNumOldFiles);
					EntryPtr->Comm.Files.lpDiscArray[i].OldIndex = (DWORD) QWBuf;
					if (Code = GetVarIndex( InputStream, &QWBuf) ) goto exit;
					Validate(QWBuf, EntryPtr->Comm.Files.dwNumOldFiles);
					EntryPtr->Comm.Files.lpDiscArray[i].NewIndex = (DWORD) QWBuf;
					if (Code = GetVarIndex( InputStream, &EntryPtr->Comm.Files.lpDiscArray[i].Pos) ) goto exit;
					if (Code = GetBytes( InputStream, &EntryPtr->Comm.Files.lpDiscArray[i].OldByte, 1) ) goto exit;
					if (Code = GetBytes( InputStream, &EntryPtr->Comm.Files.lpDiscArray[i].NewByte, 1) ) goto exit;
				}
			}
			else
			{
				EntryPtr->Comm.Files.lpDiscArray = NULL;
			}
		}
	}
	else
	{
		EntryPtr->Comm.Files.dwNumOldFiles = EntryPtr->Comm.Files.dwNumNewFiles = EntryPtr->Comm.Files.dwNumDiscrims = 0;
		EntryPtr->Comm.Files.lpOldAttribArray = EntryPtr->Comm.Files.lpNewAttribArray = NULL;
		EntryPtr->Comm.Files.lpOldPathArray = EntryPtr->Comm.Files.lpNewPathArray = NULL;
		EntryPtr->Comm.Files.lpOldCksumArray = EntryPtr->Comm.Files.lpNewCksumArray = NULL;
		EntryPtr->Comm.Files.lpDiscArray = NULL;
	}
# ifdef EXAPATCH_BYTE_SUPPORT
	/* build region ignore lists */
	if (BHFlags & 1)
	{
		Code = ExaMemAlloc(NULL, EntryPtr->Comm.Files.dwNumOldFiles*sizeof(char *), (void **) &EntryPtr->OldRegionListArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		memset( EntryPtr->OldRegionListArray, 0, EntryPtr->Comm.Files.dwNumOldFiles*sizeof(char *) );
		for (i=0; i<EntryPtr->Comm.dwNumBH ; i++ )
		{
			if (EntryPtr->Comm.BHPtr[i].dwType == EXP_BYTE_IGNORE)
			{
				Origin = 0;
				Start = EntryPtr->Comm.BHPtr[i].qwOldPos;
				End = Start + EntryPtr->Comm.BHPtr[i].qwLength;
				for (j=0; 
					j<EntryPtr->Comm.Files.dwNumOldFiles && Origin < End; 
					j++ )
				{
					FileEnd = Origin + EntryPtr->Comm.Files.lpOldAttribArray[j].Size;
					if (Start < FileEnd)
					{
						Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyIgnoreRegion), (void **) &TheRegion ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						if (Code) goto exit;
						TheRegion->Next = EntryPtr->OldRegionListArray[j];
						EntryPtr->OldRegionListArray[j] = TheRegion;
						if (Start > Origin)
						{
							TheRegion->Offset = Start - Origin;
						}
						else
						{
							TheRegion->Offset = 0;
						}
						if (End < FileEnd)
						{
							TheRegion->Length = End - (Origin + TheRegion->Offset);
						}
						else
						{
							TheRegion->Length = FileEnd - (Origin + TheRegion->Offset);
						}
					}
					Origin = FileEnd;
				}
			}
		}
	}
	if (BHFlags & 2)
	{
		Code = ExaMemAlloc(NULL, EntryPtr->Comm.Files.dwNumNewFiles*sizeof(char *), (void **) &EntryPtr->NewRegionListArray); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		memset( EntryPtr->NewRegionListArray, 0, EntryPtr->Comm.Files.dwNumNewFiles*sizeof(char *) );
		for (i=0; i<EntryPtr->Comm.dwNumBH ; i++ )
		{
			if (EntryPtr->Comm.BHPtr[i].dwType != EXP_BYTE_IGNORE)
			{
				Origin = 0;
				Start = EntryPtr->Comm.BHPtr[i].qwNewPos;
				End = Start + EntryPtr->Comm.BHPtr[i].qwLength;
				for (j=0; 
					j<EntryPtr->Comm.Files.dwNumNewFiles && Origin < End; 
					j++ )
				{
					FileEnd = Origin + EntryPtr->Comm.Files.lpNewAttribArray[j].Size;
					if (Start < FileEnd)
					{
						Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyIgnoreRegion), (void **) &TheRegion ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						if (Code) goto exit;
						TheRegion->Next = EntryPtr->NewRegionListArray[j];
						EntryPtr->NewRegionListArray[j] = TheRegion;
						if (Start > Origin)
						{
							TheRegion->Offset = Start - Origin;
						}
						else
						{
							TheRegion->Offset = 0;
						}
						if (End < FileEnd)
						{
							TheRegion->Length = End - (Origin + TheRegion->Offset);
						}
						else
						{
							TheRegion->Length = FileEnd - (Origin + TheRegion->Offset);
						}
					}
					Origin = FileEnd;
				}
			}
		}
	}
# endif


# ifdef EXAPATCH_HISTORY_SUPPORT
	if (HeaderWord & 0x2000)
	{
		/* set History Info */
		if (Code = GetQword( InputStream, &EntryPtr->Comm.qwHistorySize )) goto exit;
		if (Code = GetDword( InputStream, &EntryPtr->Comm.dwNumHistoryVersions )) goto exit;
	}
	else
# endif
	{
		EntryPtr->Comm.qwHistorySize = 0;
		EntryPtr->Comm.dwNumHistoryVersions = 0;
	}
	if (HeaderWord & 0x4000)
	{
		/* set Size Info */
		if (Code = GetQword( InputStream, &EntryPtr->qwEntrySize )) goto exit;
	}
	else
	{
		EntryPtr->qwEntrySize = 0;
	}

	if (HeaderWord & 0x8000)
# ifdef EXAPATCH_EH_SUPPORT
	{
		ExaPatchCommLink * StartLink = NULL;
		ExaPatchCommLink * EndLink = NULL;
		ExaPatchCommLink * ThisLink;
		ExaPatchExtendedHeader * ThisEH;

		/* build a list of the EHBlocks */
		if (Code = GetQword( InputStream, &QWBuf )) goto exit;
		HeaderPos = InputStream->CurPos + QWBuf;
		EntryPtr->Comm.dwNumEHBlocks = 0;
		while ( InputStream->CurPos < HeaderPos )
		{
			EntryPtr->Comm.dwNumEHBlocks++;
			Code = ExaMemAlloc( NULL, sizeof( ExaPatchCommLink ), (void **) &ThisLink); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			Code = ExaMemAlloc( NULL, sizeof( ExaPatchExtendedHeader ), &ThisLink->Node); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			ThisEH = (ExaPatchExtendedHeader *) ThisLink->Node;
			ThisLink->Next = NULL;
			if (EndLink)
			{
				EndLink->Next = ThisLink;
				EndLink = ThisLink;
			}
			else
			{
				StartLink = EndLink = ThisLink;
			}
			if (Code = GetWord( InputStream, &ThisEH->ID ) ) goto exit;
			if (Code = GetVarIndex( InputStream, &ThisEH->qwSize ) ) goto exit;
			Validate(ThisEH->qwSize, MAX_EH_SIZE);
			Code = ExaMemAlloc( NULL, (DWORD)ThisEH->qwSize, &ThisEH->HeaderBlock ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			if (Code = GetBytes( InputStream, ThisEH->HeaderBlock, (DWORD) ThisEH->qwSize) ) goto exit;
		}
		/* make sure nothing went horribly wrong */
		if (InputStream->CurPos != HeaderPos)
		{
			Code = EXAPATCH_CORRUPT_PATCH_FILE;
			goto exit;
		}
		/* roll the list up into an array */
		Code = ExaMemAlloc(NULL, EntryPtr->Comm.dwNumEHBlocks * sizeof( ExaPatchExtendedHeader ), (void **) &EntryPtr->Comm.EHBlockPtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		ThisLink = StartLink;
		for (i=0, ThisLink = StartLink; i<EntryPtr->Comm.dwNumEHBlocks ; i++ )
		{
			memcpy( &EntryPtr->Comm.EHBlockPtr[i], ThisLink->Node, sizeof( ExaPatchExtendedHeader ) );
			StartLink = ThisLink;
			ThisLink = StartLink->Next;
			ExaMemFree( NULL, StartLink->Node );
			ExaMemFree( NULL, StartLink );
		}
	}
	else
# else
#  ifdef EXAPATCH_SILENT_UNSUPPORT
	{
		EntryPtr->Comm.dwNumEHBlocks = 0;
		EntryPtr->Comm.EHBlockPtr = NULL;
		if (Code = GetQword( InputStream, &QWBuf )) goto exit;
		Code = SeekExaStream( InputStream, QWBuf, EXAPATCH_SEEK_CUR, NULL );
		if (Code) goto exit;
	}
	else
#  else
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
	else
#  endif
# endif
	{
		EntryPtr->Comm.dwNumEHBlocks = 0;
		EntryPtr->Comm.EHBlockPtr = NULL;
	}
	if ((EntryPtr->wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)
	{
		// for HISTORY entry, include header in qwEntrySize (which would otherwise be 0)
		EntryPtr->qwEntrySize = EntryPtr->Comm.qwHistorySize + (InputStream->CurPos - EntryPtr->qwEntryPos);
	}
exit:
	if (Code)
	{
		ExaPatchApplyFreeParsedEntry( EntryPtr );
	}
	return(Code);
}

static int ExaPatchStartWork(
		ExaPatchStream * FileToParse,	/* assumed to be positioned at header */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		DWORD ApplyFlags,
		DWORD ApplyFlagsToForce,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		PLATWORD Handle
		)
{
	int Code = EXAPATCH_SUCCESS;
# if defined( EXAPATCH_CONTAINER_SUPPORT ) || defined( EXAPATCH_TEMPDIR_SUPPORT )
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle;
# endif
# ifdef EXAPATCH_TEMPDIR_SUPPORT
	DWORD TempDirUsed = FALSE;
	WCHAR * TDPtr = NULL;
	DWORD Resilient = FALSE;
# endif
# if defined (_WIN32) && !defined (WINCE)
	DWORD Remember;
# endif
	DWORD i;
	DWORD Size;
  wchar_t * Ptr;
	unsigned short PatchEntryHeader;
	size_t TheLen;

# ifdef EXAPATCH_TEMPDIR_SUPPORT
	if (TheChannel->pTempDir)
	{
		TempDirUsed = TRUE;
		TDPtr = TheChannel->pTempDir;
	}
# endif
	/* initialize */
	HeaderPtr->DupList = NULL;
	HeaderPtr->TempList = NULL;
	HeaderPtr->ManipList = HeaderPtr->ManipTail = NULL;
	HeaderPtr->LocalManipList = HeaderPtr->LocalManipTail = NULL;
	HeaderPtr->RegList = HeaderPtr->RegTail = NULL;
	HeaderPtr->BackupCmdFile = HeaderPtr->ErrorFile = NULL;
	HeaderPtr->ErrorBuffer = NULL;
	HeaderPtr->BackupCFName = HeaderPtr->ActualApplyDir = HeaderPtr->ErrorFileName
		= HeaderPtr->ScratchDir = NULL;
	HeaderPtr->SystemDirs = NULL;
	HeaderPtr->RegUndoID = -1;
	HeaderPtr->dwBW = 0;
	memset( &HeaderPtr->Stats, 0, 7*sizeof(QWORD) );
	/* parse header */
# ifdef EXAPATCH_CONTAINER_SUPPORT
	if (TheChannel->dwPatchIndex != 0xffffffffU)
	{
		Code = ExaPatchCommParseHeader( FileToParse, &HeaderPtr->Comm, EXP_PARSE_ALT, TheChannel->dwPatchIndex );
	}
	else
# endif
	{
		Code = ExaPatchCommParseHeader( FileToParse, &HeaderPtr->Comm, 0, 0 );
	}
# ifdef EXAPATCH_TEMPDIR_SUPPORT
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_RESILIENT)
	{
		Resilient = TRUE;
	}
# endif
# ifndef _WIN32
	/* Slashify any paths that got read */
	Slashify( HeaderPtr->Comm.lpBackupDirectory );
	if (HeaderPtr->Comm.SubDirs)
	{
		for (i=0; i<HeaderPtr->Comm.SubDirs->dwNumSubDirs ; i++ )
		{
			Slashify( HeaderPtr->Comm.SubDirs->lpPathArray[i] );
		}
	}
# endif
	if (Code) goto exit;
	HeaderPtr->ApplyFlags = ApplyFlags;
	HeaderPtr->ApplyForceFlags = ApplyFlagsToForce;
	/* fix up the apply flags based on the user level */
	HeaderPtr->ApplyFlags &= ExaPatchUserLevelMask[(HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_USER_ADVANCED) >> 2];
	HeaderPtr->ApplyForceFlags &= ExaPatchUserLevelMask[(HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_USER_ADVANCED) >> 2];
	/* process the apply flags */
	/* the following flags need no particular handling at this point: */
	/*	ONEFILEONLY, VERIFYONLY, KEEPTEMP, LISTONLY, QUIET, */
	/*	VERBOSE, USERFILEMANIP, FULLPATHDISPLAY */
	HeaderPtr->ApplyFlags &= ~EXP_PATCH_APPLY_REMOTE_TMPDIR;
# ifdef EXAPATCH_BACKUP_SUPPORT
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_BACKUP)
	{
		HeaderPtr->IndivFlagsOverridden |= EXP_FILE_BACKUP;
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_BACKUP)
		{
			HeaderPtr->Comm.dwFileFlags |= 	EXP_FILE_BACKUP;
		}
		else
		{
			HeaderPtr->Comm.dwFileFlags &= ~ EXP_FILE_BACKUP;
		}
	}
# endif
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_ERRORFILE)
	{
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ERRORFILE)
		{
			HeaderPtr->Comm.dwGlobalFlags |= EXP_GLOBAL_ERRORFILE;
		}
		else
		{
			HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_ERRORFILE;
		}
	}
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_CONFIRMSYSTEMS)
	{
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_CONFIRMSYSTEMS)
		{
			HeaderPtr->Comm.dwGlobalFlags |= EXP_GLOBAL_CONFIRM;
		}
		else
		{
			HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_CONFIRM;
		}
	}
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_IGNOREMISSING)
	{
		HeaderPtr->IndivFlagsOverridden |= EXP_FILE_IGNOREMISSING;
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_IGNOREMISSING)
		{
			HeaderPtr->Comm.dwFileFlags |= 	EXP_FILE_IGNOREMISSING;
		}
		else
		{
			HeaderPtr->Comm.dwFileFlags &= ~ EXP_FILE_IGNOREMISSING;
		}
	}
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_MESSAGES)
	{
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_MESSAGES)
		{
			HeaderPtr->Comm.dwGlobalFlags |= EXP_GLOBAL_MESSAGE;
		}
		else
		{
			HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_MESSAGE;
		}
	}
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_PATHSEARCH)
	{
		HeaderPtr->IndivFlagsOverridden |= EXP_FILE_PATHSEARCH;
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_PATHSEARCH)
		{
			HeaderPtr->Comm.dwFileFlags |= 	EXP_FILE_PATHSEARCH;
		}
		else
		{
			HeaderPtr->Comm.dwFileFlags &= ~ EXP_FILE_PATHSEARCH;
		}
	}
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_SUBDIRSEARCH)
	{
		HeaderPtr->IndivFlagsOverridden |= EXP_FILE_SUBDIRSEARCH;
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_SUBDIRSEARCH)
		{
			HeaderPtr->Comm.dwFileFlags |= 	EXP_FILE_SUBDIRSEARCH;
		}
		else
		{
			HeaderPtr->Comm.dwFileFlags &= ~ EXP_FILE_SUBDIRSEARCH;
		}
	}
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_TIMEZONEIGNORE)
	{
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_TIMEZONEIGNORE)
		{
			HeaderPtr->Comm.dwGlobalFlags |= EXP_GLOBAL_NOTZCHECK;
		}
		else
		{
			HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_NOTZCHECK;
		}
	}
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_UNDO)
	{
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_UNDO)
		{
			HeaderPtr->Comm.dwGlobalFlags |= EXP_GLOBAL_UNDO;
		}
		else
		{
			HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_UNDO;
		}
	}
# ifdef EXAPATCH_ALLOWDELAY_SUPPORT
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_ALLOWDELAY)
	{
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ALLOWDELAY)
		{
			HeaderPtr->Comm.dwGlobalFlags |= EXP_GLOBAL_ALLOWDELAY;
		}
		else
		{
			HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_ALLOWDELAY;
		}
	}  
	if (HeaderPtr->ApplyForceFlags & EXP_PATCH_APPLY_FORCEDELAY)
	{
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_FORCEDELAY)
		{
			HeaderPtr->Comm.dwGlobalFlags |= EXP_GLOBAL_FORCEDELAY;
		}
		else
		{
			HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_FORCEDELAY;
		}
	}  
# endif
# ifdef EXAPATCH_LINK_SUPPORT
	/*  initialize Unique Tree if necessary	*/
	if ((HeaderPtr->Comm.dwReserved & 1)
		&& (HeaderPtr->dwExtensionSize >= HUNIQUE_SIZE))
	{
		if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_LINKUNIQUE)
		{
			HeaderPtr->hUnique = ExaUniqueOpen();
		}
		else
		{
			HeaderPtr->hUnique = 0;
		}
	}
	else
	{
		// drop LINKUNIQUE flag since the structure wasn't provided for it
		HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_LINKUNIQUE;
	}
# else
#  ifndef EXAPATCH_SILENT_SUPPORT
	if (HeaderPtr->Comm.dwGlobalFlags & (EXP_GLOBAL_LINKUNIQUE | EXP_GLOBAL_IGNORELINKS | EXP_GLOBAL_PRESERVELINKS))
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
#  endif
# endif
# ifdef EXAPATCH_EH_SUPPORT
#  ifdef EXAPATCH_ARCHIVE_SUPPORT
	if ((HeaderPtr->Comm.dwReserved & 1)
		&& (HeaderPtr->dwExtensionSize >= ARCHIVE_SIZE))
	{
		Code = ExaMemAlloc( NULL, sizeof(ArchiveInfo), &HeaderPtr->pArchiveInfo ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			memset( HeaderPtr->pArchiveInfo, 0, sizeof(ArchiveInfo) );
		}
	}
#  endif
# endif
# ifdef EXAPATCH_PASSWORD_SUPPORT
	/* get password if necessary */
	if (HeaderPtr->Comm.PWHash)
	{
		QWORD Hash1=0;

		if ((ProgressCallBack ==  NULL) || (0 == (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE)))
		{
			Code = EXAPATCH_INVALID_PASSWORD;
			goto exit;
		}
		Code = ExaMemAlloc( NULL, 512*sizeof(wchar_t), (void **) &HeaderPtr->Comm.lpPassword ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		while (Hash1 != HeaderPtr->Comm.PWHash)
		{
			/* get password */
			Code = (*ProgressCallBack)(EXP_PATCH_PASSWORD, (void *) HeaderPtr->Comm.lpPassword, CallbackHandle );
			if (Code)
			{
				Code = EXAPATCH_USER_CANCEL;
				goto exit;
			}
			/* crunch it */
			Code = CrunchPW( HeaderPtr->Comm.lpPassword, &Hash1, &HeaderPtr->Comm.PWHash2 );
			if (Code) goto exit;
			/* check it */
			if (Hash1 != HeaderPtr->Comm.PWHash)
			{
				/* wrong password */
				if (Code = (*ProgressCallBack)(EXP_PATCH_INVALID_PASSWORD, NULL, CallbackHandle))
				{
					Code = EXAPATCH_USER_CANCEL;
					goto exit;
				}
			}
		}
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (HeaderPtr->Comm.PWHash)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
#  endif
# endif
	/* initialize backup directory/file manipulation list/list file, etc. */
	/* get apply directory first */
	if (HeaderPtr->ApplyDirectory && *HeaderPtr->ApplyDirectory)
	{
		TheLen = PSwcslen( HeaderPtr->ApplyDirectory );
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen +1 )),(void **) &Ptr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code) goto exit;
		wcscpy_s( Ptr, TheLen+1, HeaderPtr->ApplyDirectory );
	}
	else
	{
# ifdef EXAPATCH_REGISTRY_SUPPORT
		if (HeaderPtr->Comm.nRootType != -1)
		{
			Code = ExaPatchFindKeyDir( &Ptr,
				HeaderPtr->Comm.nRootType,
				HeaderPtr->Comm.dwRootBase,
				HeaderPtr->Comm.lpRootParm1,
				HeaderPtr->Comm.lpRootParm2,
				HeaderPtr->Comm.lpRootParm3 );
			if (Code) goto exit;
		}
		else
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
		if (HeaderPtr->Comm.nRootType != -1)
		{
			Code = EXAPATCH_NOT_SUPPORTED;
			goto exit;
		}
		else
#  endif
# endif		
		{
			Code = ExaMemAlloc( NULL, 2*sizeof(wchar_t), (void **) &Ptr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			wcscpy_s( Ptr, 2, L"." );
		}
	}
# if defined( EXAPATCH_EH_SUPPORT ) && defined( EXAPATCH_REGISTRY_SUPPORT )
	Code = DestKeyHandling( HeaderPtr, &Ptr, ProgressCallBack, CallbackHandle );
	if (Code) goto exit;
# endif
	/* fully qualify it */
	Code = ExaFullyQualify( Ptr, &HeaderPtr->ActualApplyDir );
	ExaMemFree( NULL, Ptr );
	if (Code) goto exit;
	/* attempt to create a file in the apply directory */
	Code = ExaDirMerge( HeaderPtr->ActualApplyDir, L"aaaaaa", &Ptr, PATH_DELIMITER );
	if (Code) goto exit;
	Ptr[PSwcslen(Ptr)-5] = L'\0';
	Code = ExaBuildTempName( Ptr, 4, PATH_DELIMITER );
	if (Code) 
	{
		ExaMemFree( NULL, Ptr );
		goto exit;
	}

# ifdef _WIN32
#  ifndef WINCE
	Remember = SetErrorMode( SEM_FAILCRITICALERRORS );
#  endif /* WINCE */
# endif /* _WIN32 */

	Code = ExaPatchApplyOpenFileArrayAsStreamW( &HeaderPtr->ErrorFile,
			&Ptr,
			1,
			0,
			NULL );

# ifdef _WIN32
#  ifndef WINCE
	SetErrorMode( Remember );
#  endif /* WINCE */
# endif /* _WIN32 */

	if (Code)
	{
# ifdef _WIN32
		/* couldn't create it...use TEMP directory for Scratch Dir */
# ifdef WINCE
    Size = GetTempPathW( 0, NULL ) + 1;
		Code = ExaMemAlloc( NULL, sizeof(wchar_t)*Size, (void **) &HeaderPtr->ScratchDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
    GetTempPathW( Size, HeaderPtr->ScratchDir );
# else
		unsigned char NBuf[MAX_PATH];

		ExaMemFree( NULL, Ptr );
		GetTempPathA( MAX_PATH, NBuf );
		Size = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
			NBuf, -1, NULL, 0 );
		Code = ExaMemAlloc( NULL, sizeof(wchar_t)*Size, (void **) &HeaderPtr->ScratchDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
			NBuf, -1, HeaderPtr->ScratchDir, Size );
# endif /* WINCE */
# else
		char * NBuf;
		
		NBuf = getenv( "TMPDIR" );
		if (NBuf)
		{
			Size = strlen( NBuf ) + 2;
			Code = ExaMemAlloc( NULL, sizeof(wchar_t)*Size, (void **) &HeaderPtr->ScratchDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) 
			{
				//free( NBuf ); this appears to be wrong
				goto exit;
			}
			if (-1 == PSmbstowcs( HeaderPtr->ScratchDir, NBuf, Size ))
			{
				Code = EXAPATCH_CHARSET_ERROR;
				//free( NBuf ); this appears to be wrong
				goto exit;
			}
			if (NBuf[strlen(NBuf)-1] != '/')
				PSwcscat( HeaderPtr->ScratchDir, L"/" );
			//free( NBuf ); this appears to be wrong
		}
		else
		{
			Size = 10;
			Code = ExaMemAlloc( NULL, sizeof(wchar_t)*Size, (void **) &HeaderPtr->ScratchDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			PSwcscpy( HeaderPtr->ScratchDir, L"/usr/tmp/" );
		}
# endif
	}
	else
	{
		/* create worked...all is well */
		CloseExaStream( HeaderPtr->ErrorFile );
		ExaDelete( Ptr );
		ExaMemFree( NULL, Ptr );
		TheLen = PSwcslen( HeaderPtr->ActualApplyDir );
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen ) ), (void **) &HeaderPtr->ScratchDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code) goto exit;
		wcscpy_s( HeaderPtr->ScratchDir, TheLen+1, HeaderPtr->ActualApplyDir );
	}


	/* open the errorfile (in scratch dir) */
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_ERRORFILE)
	{
		Code = ExaDirMerge( HeaderPtr->ScratchDir, L"dfcapp.err", &HeaderPtr->ErrorFileName, PATH_DELIMITER );
		if (Code) goto exit;
		Code = ExaPatchApplyOpenFileArrayAsStreamW( &HeaderPtr->ErrorFile,
			&HeaderPtr->ErrorFileName,
			1,
			EXP_OPEN_APPEND,
			NULL );
		if (Code) goto exit;
		Code = SeekExaStream( HeaderPtr->ErrorFile, 0, EXAPATCH_SEEK_END ,NULL );
	}
# ifdef EXAPATCH_BACKUP_SUPPORT
	/* setup the Backup Directory */
	if (HeaderPtr->BackupDirectory && *HeaderPtr->BackupDirectory)
	{
		if (HeaderPtr->Comm.lpBackupDirectory)
		{
			ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
		}
		TheLen = PSwcslen( HeaderPtr->BackupDirectory );
		if (TheLen > 0x7ffffffe )
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)),(void **) &HeaderPtr->Comm.lpBackupDirectory ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code) goto exit;
		wcscpy_s( HeaderPtr->Comm.lpBackupDirectory, TheLen+1, HeaderPtr->BackupDirectory );
	}
	else
	{
		if (HeaderPtr->Comm.lpBackupDirectory == NULL || *HeaderPtr->Comm.lpBackupDirectory == L'\0')
		{
			Code = ExaMemAlloc( NULL, sizeof(wchar_t)*7, (void **) &HeaderPtr->Comm.lpBackupDirectory ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			wcscpy_s( HeaderPtr->Comm.lpBackupDirectory, 7, L"Backup" );
		}
	}
# ifndef WINCE
# ifdef _WIN32
	if ((HeaderPtr->Comm.lpBackupDirectory[1] == L':')
		|| ((HeaderPtr->Comm.lpBackupDirectory[0] == L'\\') && (HeaderPtr->Comm.lpBackupDirectory[1] == L'\\'))
		|| (HeaderPtr->Comm.lpBackupDirectory[0] == L'*')
		|| (HeaderPtr->Comm.lpBackupDirectory[0] == L'&')	)
# else
	if ((HeaderPtr->Comm.lpBackupDirectory[0] == PATH_DELIMITER)
		|| (HeaderPtr->Comm.lpBackupDirectory[0] == L'&') )
# endif
	{
		/* Fully-qualified (or equivalent) */
		if (HeaderPtr->Comm.lpBackupDirectory[0] == L'&')
		{
			/* Find environment variable */
# ifdef _WIN32
			if (0x80000000U & GetVersion())
			{
				char * NBuf1;
				char * NBuf2;
				DWORD BufSize;
				DWORD OK = FALSE;

				TheLen = PSwcslen(HeaderPtr->Comm.lpBackupDirectory);
				if (TheLen > 0x7ffffffe)
				{
					Code = EXAPATCH_OUT_OF_MEMORY;
				}
				else
				{
					BufSize = (DWORD)(sizeof(wchar_t)*(TheLen+1));
					if (0 == ExaMemAlloc(NULL,BufSize, &NBuf1))
					{
						WideCharToMultiByte( CP_ACP, 0, 1 + HeaderPtr->Comm.lpBackupDirectory, -1, NBuf1, BufSize, NULL, NULL );
						ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
						HeaderPtr->Comm.lpBackupDirectory = NULL;
						BufSize = GetEnvironmentVariableA( NBuf1, NULL, 0 );
						if (BufSize)
						{
							if (0 == ExaMemAlloc( NULL, BufSize, (void **) &NBuf2)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
							{
								GetEnvironmentVariableA( NBuf1, NBuf2, BufSize );
								BufSize = 1+lstrlenA( NBuf2 );
								if (0 == ExaMemAlloc( NULL, sizeof(wchar_t)*BufSize, (void **) &HeaderPtr->Comm.lpBackupDirectory)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
								{
									MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, NBuf2, -1, HeaderPtr->Comm.lpBackupDirectory, BufSize );
									OK = TRUE;
								}
								ExaMemFree( NULL, NBuf2 );
							}
						}
						ExaMemFree( NULL, NBuf1 );
					}
					if (HeaderPtr->Comm.lpBackupDirectory && !OK)
					{
						ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
						HeaderPtr->Comm.lpBackupDirectory = NULL;
					}
				}
			}
			else
			{
				DWORD BufSize;
				wchar_t * Ptr;
				DWORD OK = FALSE;

				BufSize = GetEnvironmentVariableW( 1 + HeaderPtr->Comm.lpBackupDirectory, NULL, 0 );
				if (BufSize)
				{
					if (0 == ExaMemAlloc( NULL, sizeof(wchar_t)*BufSize, (void **) &Ptr)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					{
						GetEnvironmentVariableW( 1 + HeaderPtr->Comm.lpBackupDirectory, Ptr, BufSize );
					}
				}
				if (HeaderPtr->Comm.lpBackupDirectory && !OK)
				{
					ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
					HeaderPtr->Comm.lpBackupDirectory = NULL;
				}

			}

# else /* _WIN32 */
			char * NBuf1;
			char * NBuf2;
			DWORD BufSize;
			DWORD OK = FALSE;

			BufSize = sizeof(wchar_t)*PSwcslen(HeaderPtr->Comm.lpBackupDirectory) + 1;
			if (0 == ExaMemAlloc(NULL,BufSize, (void **) &NBuf1)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			{
				if (-1 != PSwcstombs( NBuf1, 1 + HeaderPtr->Comm.lpBackupDirectory, BufSize ) )
				{
					ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
					HeaderPtr->Comm.lpBackupDirectory = NULL;
					NBuf2 = getenv( NBuf1 );
					if (NBuf2)
					{
						if (0 == ExaMemAlloc( NULL, sizeof(wchar_t)*(1+strlen(NBuf2)), (void **) &HeaderPtr->Comm.lpBackupDirectory)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						{
							if (-1 != PSmbstowcs( HeaderPtr->Comm.lpBackupDirectory, NBuf2, sizeof(wchar_t)*(1+strlen(NBuf2))))
							{
								OK = TRUE;
							}
						}
					}
				}
				ExaMemFree( NULL, NBuf1 );
			}
			if (HeaderPtr->Comm.lpBackupDirectory && !OK)
			{
				ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
				HeaderPtr->Comm.lpBackupDirectory = NULL;
			}
# endif
		}
# ifdef EXAPATCH_DISKSPACE_SUPPORT
#  ifdef _WIN32
		if (HeaderPtr->Comm.lpBackupDirectory[0] == L'*')
		{
			/* Find drive with most space */
			/* cheap, semi-gross re-use of code...*/
			if (0 == ExaDevInit( FALSE ))
			{
				PMOUNTPOINT pThisMP;
				PPHYSDRIVE pThisPD, pMaxPD;

				for (pThisMP = ExaDevGetMP(); pThisMP ; pThisMP = pThisMP->pNext )
				{
					ExaDevRegNeeds( pThisMP->szWideName, 0, 1 );
				}
				ExaDevCheckNeeds();

				pMaxPD = NULL;
				for (pThisPD = ExaDevGetPD() ; pThisPD ; pThisPD = pThisPD->pNext )
				{
					if (pMaxPD)
					{
						if (pThisPD->qwBytesPresent > pMaxPD->qwBytesPresent)
						{
							pMaxPD = pThisPD;
						}
					}
					else
					{
						pMaxPD = pThisPD;
					}
				}
				ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
				TheLen = 8+PSwcslen( pMaxPD->szDevName );
				if (TheLen > 0x7ffffffe)
				{
					Code = EXAPATCH_OUT_OF_MEMORY;
				}
				else
				{
					if (0 == ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1 + TheLen ) ), (void **) &HeaderPtr->Comm.lpBackupDirectory )) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					{
						wcscpy_s( HeaderPtr->Comm.lpBackupDirectory, TheLen+1, pMaxPD->szDevName );
						wcscat_s( HeaderPtr->Comm.lpBackupDirectory, TheLen+1, L"\\DFCBACK" );
					}
				}
				ExaDevClose();
			}
		}
#  endif
# endif
		if (NULL == HeaderPtr->Comm.lpBackupDirectory)
		{
			TheLen = PSwcslen(HeaderPtr->ScratchDir);
			if (TheLen > 0x7ffffffe)
			{
				Code = EXAPATCH_OUT_OF_MEMORY;
			}
			else
			{
				if (0 == ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) &HeaderPtr->Comm.lpBackupDirectory)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				{
					if (HeaderPtr->Comm.lpBackupDirectory)
					{
						wcscpy_s( HeaderPtr->Comm.lpBackupDirectory, TheLen+1, HeaderPtr->ScratchDir );
					}
				}
			}
		}
		HeaderPtr->ApplyFlags |= EXP_PATCH_APPLY_REMOTE_TMPDIR;
	}
	else
# endif /* !WINCE */
	{
		Code = ExaDirMerge( HeaderPtr->ScratchDir, HeaderPtr->Comm.lpBackupDirectory, &Ptr, PATH_DELIMITER );
		if (Code) goto exit;
		ExaMemFree( NULL, HeaderPtr->Comm.lpBackupDirectory );
		HeaderPtr->Comm.lpBackupDirectory = Ptr;
	}
	/* set up backup command file, list file, dup list, temp list, */
	if (HeaderPtr->Comm.dwFileFlags & EXP_FILE_BACKUP)
	{
		if (EXAPATCH_SUCCESS == (Code = ExaMakeDir(HeaderPtr->Comm.lpBackupDirectory)))
		{
			if (EXAPATCH_SUCCESS == (Code = ExaDirMerge(HeaderPtr->Comm.lpBackupDirectory,BACKUP_CMD_FILE,&HeaderPtr->BackupCFName, PATH_DELIMITER)))
			{
				if (EXAPATCH_SUCCESS == ExaExists(HeaderPtr->BackupCFName))
				{
					/* rename it */
					wchar_t * Ptr;
					if (EXAPATCH_SUCCESS == (Code = ExaEZTemp(HeaderPtr->Comm.lpBackupDirectory, &Ptr)))
					{
						Code = ExaRename( HeaderPtr->BackupCFName, Ptr );
						ExaMemFree( NULL, Ptr );
					}
				}
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = ExaPatchApplyOpenFileArrayAsStreamW( &HeaderPtr->BackupCmdFile,
							&HeaderPtr->BackupCFName,
							1,
							0,
							NULL );
				}
			}
		}
	}
	if (Code) goto exit;
# else
#  ifdef EXAPATCH_SILENT_UNSUPPORT
	HeaderPtr->Comm.dwFileFlags &= ~EXP_FILE_BACKUP;
#  else
	if (HeaderPtr->Comm.dwFileFlags & EXP_FILE_BACKUP)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
#  endif
# endif
# ifdef EXAPATCH_LIST_SUPPORT
	if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY)
	{
		/* TODO: LIST*/
		/* Print File List Header */
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
#  endif
# endif
# ifndef EXAPATCH_VERIFYONLY_SUPPORT
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
#  endif
# endif
# ifndef EXAPATCH_ONEFILE_SUPPORT
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
#  endif
# endif
# ifdef EXAPATCH_REGISTRYSCRIPT_SUPPORT
#  ifdef EXAPATCH_LIST_SUPPORT
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#   else
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   endif
#  else
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#   endif
#  endif
	{
		/* do first reg script */
		if (HeaderPtr->Comm.lpRegScript1)
		{
			Code = ExaPatchDoRegScript( HeaderPtr->Comm.lpRegScript1,
				HeaderPtr->Comm.dwRegScript1Size,
				HeaderPtr->Comm.dwRegScript1Num,
				HeaderPtr );
			if (Code) goto exit;
		}
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
# 	ifdef EXAPATCH_LIST_SUPPORT
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#    else
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#    endif
#   else
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#    endif
#   endif
	{
		/* do first reg script */
		if (HeaderPtr->Comm.lpRegScript1)
		{
			Code = EXAPATCH_NOT_SUPPORTED;
			goto exit;
		}
	}

#  endif
# endif
# ifdef EXAPATCH_SYSTEM_SUPPORT
#  ifdef EXAPATCH_LIST_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#  endif
	{
		/* lookup system base dirs */
		if (HeaderPtr->Comm.Systems && HeaderPtr->Comm.Systems->dwNumSystems)
		{
# if defined (_WIN32) && !defined (WINCE)
			wchar_t * SearchDirectoryList = NULL;
			
			{
				DWORD NetToo = FALSE;
				wchar_t * RealDriveList = HeaderPtr->DrivesToCheck;
				wchar_t WBuffer[27];
				wchar_t * Ptr;
				wchar_t * Ptr2;
				char DriveSpec[]="X:\\";
				DWORD DriveType;

				if (RealDriveList == NULL)
				{
					RealDriveList = L"";
				}
				if (RealDriveList[0] == L'*')
				{
					if (RealDriveList[1] == L'*')
					{
						NetToo = TRUE;
					}
					Ptr = RealDriveList = WBuffer;
					for (i=0; i<26 ; i++ )
					{
						DriveSpec[0] = 'A' + ((char) i);
						DriveType = GetDriveTypeA( DriveSpec );
						if ((DriveType == DRIVE_FIXED) || (NetToo && (DriveType == DRIVE_REMOTE)))
						{
							*(Ptr++) = L'A' + ((wchar_t) i);
						}
					}
					*Ptr = L'\0';
				}
				TheLen = 4*PSwcslen(RealDriveList)+PSwcslen(HeaderPtr->ActualApplyDir);
				if (TheLen > 0x7ffffffe)
				{
					Code = EXAPATCH_OUT_OF_MEMORY;
					goto exit;
				}
				else
				{
					Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), &SearchDirectoryList ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					if (Code == 0)
					{
						wcscpy_s( SearchDirectoryList, TheLen+1, HeaderPtr->ActualApplyDir );
						Ptr2 = SearchDirectoryList + PSwcslen( SearchDirectoryList );
						for (Ptr = RealDriveList; *Ptr ; Ptr++ )
						{
							*(Ptr2++) = PATH_SEPARATOR;
							*(Ptr2++) = *Ptr;
							*(Ptr2++) = L':';
							*(Ptr2++) = PATH_DELIMITER;
						}
						*Ptr2 = L'\0';
					}
					else
					{
						goto exit;
					}
				}
			}
# else
			wchar_t * SearchDirectoryList = HeaderPtr->DrivesToCheck;
# endif
			Code = ExaMemAlloc( NULL, HeaderPtr->Comm.Systems->dwNumSystems*sizeof(wchar_t *), (void **) &HeaderPtr->SystemDirs ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			for (i=0; i<HeaderPtr->Comm.Systems->dwNumSystems ; i++ )
			{
				if ((NULL == HeaderPtr->Comm.Systems->lpKeyTypeArray)
# ifdef EXAPATCH_REGISTRY_SUPPORT
					|| (ExaPatchFindKeyDir( &HeaderPtr->SystemDirs[i],
							HeaderPtr->Comm.Systems->lpKeyTypeArray[i],
							HeaderPtr->Comm.Systems->lpKeyBaseArray[i],
							HeaderPtr->Comm.Systems->lpParm1Array[i],
							HeaderPtr->Comm.Systems->lpParm2Array[i],
							HeaderPtr->Comm.Systems->lpParm3Array[i]))
# endif
							)
				{
					if (HeaderPtr->Comm.Systems->lpTagArray[i] 
						&& L'%'== *(HeaderPtr->Comm.Systems->lpTagArray[i]))
					{
						// tag beginning with % is a literal absolute path
						size_t TheLen;
						TheLen = PSwcslen( HeaderPtr->Comm.Systems->lpTagArray[i] );
						// note that this is actually correct since it should include
						// the trailing null, but NOT the initial %
						Code = ExaMemAlloc( NULL,sizeof(wchar_t)* TheLen ,(void **) &HeaderPtr->SystemDirs[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						if (Code) goto exit;
						wcscpy_s( HeaderPtr->SystemDirs[i], TheLen,
							1 + HeaderPtr->Comm.Systems->lpTagArray[i] );
					}
					else
					{
						if (ExaPatchFileSearch( &HeaderPtr->SystemDirs[i],
						HeaderPtr->Comm.Systems->lpTagArray[i],
						SearchDirectoryList,
						(EXP_SEARCH_FULL_RECURSE | EXP_SEARCH_CALLBACK) 
					 		+ ((HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_CONFIRM)?EXP_SEARCH_CONFIRMATION:0)
# ifdef EXAPATCH_LINK_SUPPORT
					 		+ ((HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_IGNORELINKS)?EXP_SEARCH_IGNORELINKS:0)
# endif
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					 		+ ((HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_PROCESSLINKS)?EXP_SEARCH_PROCESSLINKS:0)
# endif
					 	,NULL, NULL, NULL, NULL, NULL, PATH_DELIMITER, PATH_SEPARATOR,
						ProgressCallBack, CallbackHandle, HeaderPtr->Comm.Systems->lpNameArray[i], HeaderPtr->Comm.Systems->lpBaseArray[i] ))
						{
							if (ProgressCallBack)
							{
								wchar_t * Ptrs[3];
								wchar_t RetBuffer[512];
								DWORD Flag = FALSE;

								Ptrs[0] = HeaderPtr->Comm.Systems->lpNameArray[i];
								Ptrs[1] = HeaderPtr->Comm.Systems->lpTagArray[i];
								Ptrs[2] = RetBuffer;
								while (!Flag)
								{
									Code = (*ProgressCallBack)(EXP_PATCH_SYSTEM_PROMPT, Ptrs, CallbackHandle );
									if (Code)
									{
										Code = EXAPATCH_USER_CANCEL;
										goto exit;
									}
									else
									{

										/* Check RetBuffer - use it if OK */
										if (RetBuffer[PSwcslen(RetBuffer)-1] == PATH_DELIMITER)
										{
											RetBuffer[PSwcslen(RetBuffer)-1] = L'\0';
										}
										if (ExaDirExists(RetBuffer))
										{
											TheLen = PSwcslen(RetBuffer);
											if (TheLen > 0x7ffffffe)
											{
												Code = EXAPATCH_OUT_OF_MEMORY;
											}
											else
											{
												Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) &HeaderPtr->SystemDirs[i]); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
											}
											if (Code) goto exit;
											wcscpy_s( HeaderPtr->SystemDirs[i], TheLen+1, RetBuffer );
											Flag = TRUE;
										}

									}
								}
							}
							else
							{
								/* use Apply Directory */
								TheLen = PSwcslen( HeaderPtr->ActualApplyDir );
								if (TheLen > 0x7ffffffe)
								{
									Code = EXAPATCH_OUT_OF_MEMORY;
								}
								else
								{
									Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) &HeaderPtr->SystemDirs[i]); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
								}
								if (Code) goto exit;
								wcscpy_s( HeaderPtr->SystemDirs[i], TheLen+1, HeaderPtr->ActualApplyDir );
							}
						}
						else
						{
							DWORD Count;
							wchar_t * DelimPtr;


							Count = HeaderPtr->Comm.Systems->lpBaseArray[i] + 1;
							while (Count--)
							{
								DelimPtr = PSwcsrchr( HeaderPtr->SystemDirs[i], PATH_DELIMITER );
								if (DelimPtr)
								{
									*DelimPtr = L'\0';
								}
							}
						}
					}
				}
# ifndef EXAPATCH_REGISTRY_SUPPORT
#  ifndef EXAPATCH_SILENT_UNSUPPORT
				else
				{
					Code = EXAPATCH_NOT_SUPPORTED;
					goto exit;
				}
#  endif
# endif
			}
# ifdef _WIN32
			if (SearchDirectoryList)
			{
				ExaMemFree( NULL, SearchDirectoryList );
			}
# endif
		}
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
#   ifdef EXAPATCH_LIST_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   endif
	{
		if (HeaderPtr->Comm.Systems && HeaderPtr->Comm.Systems->dwNumSystems)
		{
			Code = EXAPATCH_NOT_SUPPORTED;
			goto exit;
		}
	}
#  endif
# endif
# ifdef EXAPATCH_DISKSPACE_SUPPORT
#  ifdef EXAPATCH_LIST_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#  endif
	{
		/* check disk space */
		QWORD * OtherApplyCost;
		QWORD ArchiveCost = 0;
		SDWORD nTotalNumSys;
		SDWORD Index;
		wchar_t * DevPtr;
		DWORD BackupUsed;
		QWORD qwO,qwN,qwM,qwB,qwONum,qwNNum;
		BOOL bLimited = FALSE;

# ifdef EXAPATCH_ARCHIVE_SUPPORT
		if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_ARCHIVE)
		{
			DWORD i;
			DWORD dwFound = 0;
			ExaPatchExtendedHeader * ACEH = NULL;

			for (i=0; !(dwFound) && i<HeaderPtr->Comm.dwNumEHBlocks ; i++ )
			{
				if (HeaderPtr->Comm.EHBlockPtr[i].ID == ARCH_COST_BLOCKID_GLOBAL)
				{
					dwFound = 1;
					ACEH = &HeaderPtr->Comm.EHBlockPtr[i];
				}
			}
			if (NULL != ACEH)
			{
				DWORD * dwPtr = (DWORD *) ACEH->HeaderBlock;
				DWORD DW;

				DW = dwPtr[0];
				ArchiveCost = (QWORD)(DWORD_SWAP(DW));
				DW = dwPtr[1];
				ArchiveCost += ((QWORD)(DWORD_SWAP(DW))) << 32;
			}
		}
# endif
		BackupUsed = FALSE;
#  ifdef EXAPATCH_BACKUP_SUPPORT
		if (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_REMOTE_TMPDIR | EXP_PATCH_APPLY_BACKUP))
		{
			BackupUsed = TRUE;
		}
#  endif
#  ifdef EXAPATCH_SYSTEM_SUPPORT
		if (HeaderPtr->Comm.Systems)
		{
			nTotalNumSys = (SDWORD)(HeaderPtr->Comm.Systems->dwNumSystems);
		}
		else
#  endif		
		{
			nTotalNumSys = 0;
		}
		Code = ExaMemAlloc( NULL, sizeof(QWORD)*(2+nTotalNumSys), (void **) &OtherApplyCost ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		memset( OtherApplyCost, 0, sizeof(QWORD)*(2+nTotalNumSys) );
		if ((nTotalNumSys == 0) 
			&& (HeaderPtr->ActualApplyDir[1]==L':') 
			&& ((!BackupUsed) || ((HeaderPtr->Comm.lpBackupDirectory[1]==L':') && (HeaderPtr->Comm.lpBackupDirectory[0]==HeaderPtr->ActualApplyDir[0]))))
		{
			// can avoid extensive device checking
			bLimited = TRUE;
		}
		Code = ExaDevInit( bLimited );
		if (Code) 
		{
			ExaMemFree( NULL, OtherApplyCost );
			goto exit;
		}
		for (Index=-1; (Code == 0) && (Index < nTotalNumSys) ; Index++ )
		{
#  ifdef EXAPATCH_SYSTEM_SUPPORT
			if (Index == -1)
			{
				DevPtr = HeaderPtr->ActualApplyDir;
				qwO = HeaderPtr->Comm.qwOldDisk;
				qwN = HeaderPtr->Comm.qwNewDisk;
				qwM = HeaderPtr->Comm.qwMaxDisk;
				qwB = HeaderPtr->Comm.qwBigDisk;
				qwONum = HeaderPtr->Comm.qwOldNum;
				qwNNum = HeaderPtr->Comm.qwNewNum;
			}
			else
			{
				DevPtr = HeaderPtr->SystemDirs[Index];
				qwO = HeaderPtr->Comm.Systems->lpSizeArray[(6*Index)+EXAPATCH_SYSTEM_OLD_DISK];
				qwN = HeaderPtr->Comm.Systems->lpSizeArray[(6*Index)+EXAPATCH_SYSTEM_NEW_DISK];
				qwM = HeaderPtr->Comm.Systems->lpSizeArray[(6*Index)+EXAPATCH_SYSTEM_MAX_DISK];
				qwB = HeaderPtr->Comm.Systems->lpSizeArray[(6*Index)+EXAPATCH_SYSTEM_BIG_DISK];
				qwONum = HeaderPtr->Comm.Systems->lpSizeArray[(6*Index)+EXAPATCH_SYSTEM_OLD_NUM];
				qwNNum = HeaderPtr->Comm.Systems->lpSizeArray[(6*Index)+EXAPATCH_SYSTEM_NEW_NUM];
			}
#  else
			{
				DevPtr = HeaderPtr->ActualApplyDir;
				qwO = HeaderPtr->Comm.qwOldDisk;
				qwN = HeaderPtr->Comm.qwNewDisk;
				qwM = HeaderPtr->Comm.qwMaxDisk;
				qwB = HeaderPtr->Comm.qwBigDisk;
				qwONum = HeaderPtr->Comm.qwOldNum;
				qwNNum = HeaderPtr->Comm.qwNewNum;
			}
#  endif
#  ifdef EXAPATCH_UNDO_SUPPORT
			if (0 == (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_UNDO))
			{
				if (OtherApplyCost[2+Index] < qwB)
				{
					OtherApplyCost[2+Index] = qwB;
				}
				if (BackupUsed)
				{
					if (OtherApplyCost[0] < qwB)
					{
						OtherApplyCost[0] = qwB;
					}
				}
			}
#  endif
#  ifdef EXAPATCH_ONEFILE_SUPPORT
			if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY))
#  endif
			{
#  ifdef EXAPATCH_BACKUP_SUPPORT
				if (BackupUsed && ExaDevCompare(DevPtr, HeaderPtr->Comm.lpBackupDirectory))
				{
					/* System != Backup*/
#   ifdef EXAPATCH_ALLOWDELAY_SUPPORT
					if (EXP_GLOBAL_ALLOWDELAY & HeaderPtr->Comm.dwGlobalFlags)
					{
						/* open-file patching */
#    ifdef EXAPATCH_UNDO_SUPPORT
						if (EXP_GLOBAL_UNDO & HeaderPtr->Comm.dwGlobalFlags)
						{
							/* Undo */
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
									Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, qwO, qwONum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
									Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, qwO, qwONum );
								}
							}
							else
							{
								/*No Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
								 Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
								}
							}
						}
						else
#    endif
						{
							/* No Undo */
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
									Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, qwO, qwONum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
									Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, qwO, qwONum );
								}
							}
							else
							{
								/*No Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
								}
							}
						}
					}
					else
#   endif					
					{
						/* no open-file patching */
#   ifdef EXAPATCH_UNDO_SUPPORT
						if (EXP_GLOBAL_UNDO & HeaderPtr->Comm.dwGlobalFlags)
						{
							/* Undo */
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
									Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, qwO, qwONum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
									Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, qwO, qwONum );
								}
							}
							else
							{
								/*No Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
								}
							}
						}
#   endif
						else
						{
							/* No Undo */
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
								Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
								Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, qwO, qwONum );
							}
							else
							{
								/*No Backup */
								Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
							}
						}
					}
				}
				else
#  endif				
				{
					/* System == Backup*/
#  ifdef EXAPATCH_ALLOWDELAY_SUPPORT
					if (EXP_GLOBAL_ALLOWDELAY & HeaderPtr->Comm.dwGlobalFlags)
					{
						/* open-file patching */
#   ifdef EXAPATCH_UNDO_SUPPORT
						if (EXP_GLOBAL_UNDO & HeaderPtr->Comm.dwGlobalFlags)
						{
							/* Undo */
#     ifdef EXAPATCH_BACKUP_SUPPORT
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM + qwO, qwNNum + qwONum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN + qwO, qwNNum + qwONum );
								}
							}
							else
#     endif							
							{
								/*No Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
								}
							}
						}
						else
#   endif
						{
							/* No Undo */
#   ifdef EXAPATCH_BACKUP_SUPPORT
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM + qwO, qwNNum + qwONum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN + qwO, qwNNum + qwONum );
								}
							}
							else
#   endif						
							{
								/*No Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
								}
							}
						}
					}
					else
#  endif					
					{
						/* no open-file patching */
#  ifdef EXAPATCH_UNDO_SUPPORT
						if (EXP_GLOBAL_UNDO & HeaderPtr->Comm.dwGlobalFlags)
						{
							/* Undo */
#   ifdef EXAPATCH_BACKUP_SUPPORT
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM + qwO, qwNNum + qwONum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN + qwO, qwNNum + qwONum );
								}
							}
							else
#   endif							
							{
								/*No Backup */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
								if (TempDirUsed)
								{
									Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
									Code = ExaDevRegNeeds( TDPtr, qwN, qwNNum );
								}
								else
# endif
								{
									Code = ExaDevRegNeeds( DevPtr, qwN, qwNNum );
								}
							}
						}
						else
#  endif
						{
							/* No Undo */
#  ifdef EXAPATCH_BACKUP_SUPPORT
							if (EXP_FILE_BACKUP & HeaderPtr->Comm.dwFileFlags)
							{
								/*Backup */
								Code = ExaDevRegNeeds( DevPtr, qwM + qwO, qwNNum + qwONum );
							}
							else
#  endif							
							{
								/*No Backup */
								Code = ExaDevRegNeeds( DevPtr, qwM, qwNNum );
							}
						}
					}
				}
			}
		}
		for (Index = 0; (Code == 0) && (Index < 2+nTotalNumSys); Index++ )
		{
/*
#  ifdef EXAPATCH_BACKUP_SUPPORT
			if ((Index == 0) && (BackupUsed))
			{
				Code = ExaDevRegNeeds( HeaderPtr->Comm.lpBackupDirectory, OtherApplyCost[0], 1 );
			}
			else if (Index == 1)
#  else
			if (Index == 1)
#  endif			
*/
			if (Index == 1)
			{
# ifdef EXAPATCH_TEMPDIR_SUPPORT
				if (TempDirUsed)
				{
					if (Resilient)
					{
						Code = ExaDevRegNeeds( TDPtr, ArchiveCost + OtherApplyCost[1], 1 );
						if (Code == EXAPATCH_SUCCESS)
						{
							QWORD TheMax = ArchiveCost >> 1;

							if (TheMax < OtherApplyCost[1])
							{
								TheMax = OtherApplyCost[1];
							}
							Code = ExaDevRegNeeds( HeaderPtr->ActualApplyDir, TheMax, 1 );
						}
					}
					else
					{
						Code = ExaDevRegNeeds( TDPtr, ArchiveCost + 2*OtherApplyCost[1], 2 );
					}
				}
				else
# endif
				{
					Code = ExaDevRegNeeds( HeaderPtr->ActualApplyDir, ArchiveCost + OtherApplyCost[1], 1 );
				}
			}
#  ifdef EXAPATCH_SYSTEM_SUPPORT
			else if (Index > 1)
			{
# ifdef EXAPATCH_TEMPDIR_SUPPORT
				if (TempDirUsed)
				{
					if (Resilient)
					{
						Code = ExaDevRegNeeds( TDPtr, OtherApplyCost[Index], 1 );
						if (Code == EXAPATCH_SUCCESS)
						{
							Code = ExaDevRegNeeds( HeaderPtr->SystemDirs[Index-2], OtherApplyCost[Index], 1 );
						}
					}
					else
					{
						Code = ExaDevRegNeeds( TDPtr, 2*OtherApplyCost[Index], 2 );
					}
				}
				else
# endif
				{
					Code = ExaDevRegNeeds( HeaderPtr->SystemDirs[Index-2], OtherApplyCost[Index], 1 );
				}
			}
#  endif
		}
		ExaMemFree( NULL, OtherApplyCost );
		if (Code) goto exit;
		Code = ExaDevCheckNeeds( );
		if (Code)
		{
			/* build appropriate message about insufficient disk space */
			char * Ptr;
			char * Ptr2;
			PPHYSDRIVE pPD;
			DWORD dwNum1, dwNum2;
			char c1, c2;
			DWORD BufSize = 0;
			size_t LenLeft;



			if (EXAPATCH_SUCCESS == (Code = ExaGetNarrowString( EXP_APPLY_STRING_DISK_MSG, &Ptr )))
			{

				for (pPD = ExaDevGetPD(); pPD ; pPD = pPD->pNext )
				{
					if (pPD->qwBytesNeeded > pPD->qwBytesPresent)
					{
						BufSize += (DWORD)(strlen( Ptr ) + (sizeof(wchar_t)*PSwcslen( pPD->szDevName )) + 20);
					}
				}
				Code = ExaMemAlloc(NULL, BufSize, (void **) &HeaderPtr->ErrorBuffer); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				if (EXAPATCH_SUCCESS == Code)
				{
					Ptr2 = HeaderPtr->ErrorBuffer;
					LenLeft = BufSize;
					for (pPD = ExaDevGetPD(); pPD ; pPD = pPD->pNext )
					{
						if (pPD->qwBytesNeeded > pPD->qwBytesPresent)
						{
							ExaPrettify( pPD->qwBytesPresent, &dwNum1, &c1, 0 );
							ExaPrettify( pPD->qwBytesNeeded, &dwNum2, &c2, 1 );
							sprintf5( Ptr2, LenLeft, Ptr, dwNum1, c1, dwNum2, c2, pPD->szDevName );
							LenLeft -= strlen( Ptr2 );
							Ptr2 += strlen( Ptr2 );
						}
					}
				}
				ExaMemFree( NULL, Ptr );
				Code = EXAPATCH_INSUFFICIENT_DISK_SPACE;
			}
		}
		ExaDevClose();
		if (Code) goto exit;
		
	}
# endif
# ifdef EXAPATCH_COMMENT_SUPPORT
	/* display comments */
	if (HeaderPtr->Comm.Comments && ProgressCallBack && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE) )
	{
		DWORD i;
		for (i=0; (Code == EXAPATCH_SUCCESS) 
			&& HeaderPtr->Comm.Comments[i] ; i++ )
		{
			if ((*ProgressCallBack)(EXP_PATCH_COMMENT,HeaderPtr->Comm.Comments[i],CallbackHandle))
			{
				Code = EXAPATCH_USER_CANCEL;
			}
		}
	}
# endif
	/* initialize progress indicators */
	HeaderPtr->qwNextEntryNum = 0;

	/* install TEMP files */
	if (Code = GetWord( FileToParse, &PatchEntryHeader) ) goto exit;
	Code = SeekExaStream( FileToParse, 
		FileToParse->CurPos - 2, 
		EXAPATCH_SEEK_BEGIN, 
		NULL );
	if (Code) goto exit;
# ifdef EXAPATCH_TEMPFILE_SUPPORT
	while (EXAPATCH_TEMP_TYPE == (PatchEntryHeader & 0xf) )
	{
		ExaPatchApplyFileEntryInfo TempEntry;
		ExaPatchApplyTempListEntry * TheTempListEntry;
		ExaPatchStream * TempStream;
		ExaPatchApplyState TempState;

		if (PatchEntryHeader & 0xaff0)
		{
			Code = EXAPATCH_CORRUPT_PATCH_FILE;
			goto exit;
		}

		Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyTempListEntry ),(void **) &TheTempListEntry ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		TheTempListEntry->FileName = TheTempListEntry->OldFile = NULL;
		/* parse temp entry */
		Code = ExaPatchEntryParseWork( FileToParse,
			HeaderPtr,
			&TempEntry,
			FALSE );
		if (Code)
		{
			ExaMemFree( NULL, TheTempListEntry );
			goto exit;
		}

# ifdef EXAPATCH_LIST_SUPPORT
#  ifdef EXAPATCH_VERIFYONLY_SUPPORT
		if (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY))
#  else
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY)
#  endif
		{
			if (ProgressCallBack && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE))
			{
				/* LIST TempFile*/
				if ((*ProgressCallBack)(EXP_PATCH_FILE_START,TheTempListEntry->FileName,CallbackHandle))
				{
					Code = EXAPATCH_USER_CANCEL;
					goto exit;
				}
				if ((*ProgressCallBack)(EXP_PATCH_FILE_FINISH,NULL,CallbackHandle))
				{
					Code = EXAPATCH_USER_CANCEL;
					goto exit;
				}
			}
		}
		else
# else
#  ifdef EXAPATCH_VERIFYONLY_SUPPORT
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY)
		{
			if (ProgressCallBack && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE))
			{
				/* LIST TempFile*/
				if ((*ProgressCallBack)(EXP_PATCH_FILE_START,TheTempListEntry->FileName,CallbackHandle))
				{
					Code = EXAPATCH_USER_CANCEL;
					goto exit;
				}
				if ((*ProgressCallBack)(EXP_PATCH_FILE_FINISH,NULL,CallbackHandle))
				{
					Code = EXAPATCH_USER_CANCEL;
					goto exit;
				}
			}
		}
		else
#  endif
# endif
		{
			/* build full temp name */
			Code = ExaDirMerge( HeaderPtr->ScratchDir, 
				TempEntry.Comm.Files.lpNewPathArray[0], 
				&TheTempListEntry->FileName,
				PATH_DELIMITER );
			if (Code) 
			{
				ExaMemFree( NULL, TheTempListEntry );
				ExaPatchApplyFreeParsedEntry( &TempEntry );
				goto exit;
			}
			/* notify caller */
			if (ProgressCallBack && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE))
			{
			  Code = (*ProgressCallBack)( EXP_PATCH_FILE_START,
						(void *)((HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_FULLPATHDISPLAY)?
							TheTempListEntry->FileName :
							TempEntry.Comm.Files.lpNewPathArray[0]),
						CallbackHandle );
				if (Code == EXP_CALLBACK_ABORT)
				{
					Code = EXAPATCH_USER_CANCEL;
					ExaMemFree( NULL, TheTempListEntry->FileName );
					ExaMemFree( NULL, TheTempListEntry );
					ExaPatchApplyFreeParsedEntry( &TempEntry );
					goto exit;
				}
				Code = EXAPATCH_SUCCESS;
			}

			/* if already exists, rename it */
			if (EXAPATCH_SUCCESS == ExaExists(TheTempListEntry->FileName))
			{
				Code = ExaEZTemp( HeaderPtr->ScratchDir, &TheTempListEntry->OldFile );
				if (Code) 
				{
					ExaMemFree( NULL, TheTempListEntry->FileName );
					ExaMemFree( NULL, TheTempListEntry );
					ExaPatchApplyFreeParsedEntry( &TempEntry );
					goto exit;
				}

				Code = ExaRename( TheTempListEntry->FileName,
						TheTempListEntry->OldFile );
				if (Code)
				{
					ExaMemFree( NULL, TheTempListEntry->FileName );
					ExaMemFree( NULL, TheTempListEntry->OldFile );
					ExaMemFree( NULL, TheTempListEntry );
					ExaPatchApplyFreeParsedEntry( &TempEntry );
					goto exit;
				}

			}
			/* create file */
			Code = ExaPatchApplyOpenFileArrayAsStreamW(
				&TempStream,
				&TheTempListEntry->FileName,
				1,
				0,
				NULL );
			if (Code) 
			{
				ExaMemFree( NULL, TheTempListEntry->FileName );
				if (TheTempListEntry->OldFile)
					ExaMemFree( NULL, TheTempListEntry->OldFile );
				ExaMemFree( NULL, TheTempListEntry );
				ExaPatchApplyFreeParsedEntry( &TempEntry );
				goto exit;
			}
			/* install it */
			TempState.PatchFile = FileToParse;
			TempState.OldFile = NULL;
			TempState.NewFile = TempStream;
			TempState.PatchSize = TempEntry.qwEntrySize;
			TempState.CBPtr = (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE)?ProgressCallBack:NULL;
			TempState.CBHandle = CallbackHandle;
			TempState.LocalStart = 	0;
			TempState.LocalDelta = 	0x8000;
			TempState.GlobalStart = (DWORD)((TempEntry.qwEntryNum << 15) / HeaderPtr->Comm.qwNumEntries);
			TempState.GlobalDelta = (DWORD)(((QWORD)0x8000) / HeaderPtr->Comm.qwNumEntries );
			TempState.OldFileSize=0;
			Code = ExaPatchApplyWorkInit( &TempState );
			if (Code)
			{
				CloseExaStream( TempStream );
				ExaMemFree( NULL, TheTempListEntry->FileName );
				if (TheTempListEntry->OldFile)
					ExaMemFree( NULL, TheTempListEntry->OldFile );
				ExaMemFree( NULL, TheTempListEntry );
				ExaPatchApplyFreeParsedEntry( &TempEntry );
				goto exit;
			}
# ifdef EXAPATCH_PASSWORD_SUPPORT
			if (HeaderPtr->Comm.lpPassword)
			{
				Code = ExaPatchApplyUsePW( &TempState,
					&HeaderPtr->Comm.PWHash,
					&HeaderPtr->Comm.PWHash2 );
				if (Code) 
				{
					CloseExaStream( TempStream );
					ExaMemFree( NULL, TheTempListEntry->FileName );
					if (TheTempListEntry->OldFile)
						ExaMemFree( NULL, TheTempListEntry->OldFile );
					ExaMemFree( NULL, TheTempListEntry );
					ExaPatchApplyFreeParsedEntry( &TempEntry );
					goto exit;
				}
			}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
			if (HeaderPtr->Comm.lpPassword)
			{
				Code = EXAPATCH_NOT_SUPPORTED;
				CloseExaStream( TempStream );
				ExaMemFree( NULL, TheTempListEntry->FileName );
				if (TheTempListEntry->OldFile)
					ExaMemFree( NULL, TheTempListEntry->OldFile );
				ExaMemFree( NULL, TheTempListEntry );
				ExaPatchApplyFreeParsedEntry( &TempEntry );
				goto exit;
			}
#  endif
# endif
			Code = ExaPatchApplyWorkRoutine( &TempState );
			CloseExaStream( TempStream );
# ifndef _WIN32
			if (!Code)
			{
				ExaPatchFileAttrib myAttrib;
				memset (&myAttrib, 0, sizeof (ExaPatchFileAttrib));
				myAttrib.Flags = EXP_ATTRIB_ATTRIBUTE;
				myAttrib.Attributes = EXP_ATTRIBUTE_OWNER_X|EXP_ATTRIBUTE_OWNER_W|EXP_ATTRIBUTE_OWNER_R;
				ExaSetFileAttrib (TheTempListEntry->FileName, &myAttrib, NULL, FALSE);
			}
# endif
			if (Code)
			{
				ExaMemFree( NULL, TheTempListEntry->FileName );
				if (TheTempListEntry->OldFile)
					ExaMemFree( NULL, TheTempListEntry->OldFile );
				ExaMemFree( NULL, TheTempListEntry );
				ExaPatchApplyFreeParsedEntry( &TempEntry );
				goto exit;
			}
			/* add it to list */
			TheTempListEntry->NextEntry = HeaderPtr->TempList;
			HeaderPtr->TempList = TheTempListEntry;
			/* notify caller */
			if (ProgressCallBack && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE))
			{
			  Code = (*ProgressCallBack)( EXP_PATCH_FILE_FINISH,
						NULL,
						CallbackHandle );
				if (Code == EXP_CALLBACK_ABORT)
				{
					Code = EXAPATCH_USER_CANCEL;
					ExaMemFree( NULL, TheTempListEntry->FileName );
					if (TheTempListEntry->OldFile)
						ExaMemFree( NULL, TheTempListEntry->OldFile );
					ExaMemFree( NULL, TheTempListEntry );
					ExaPatchApplyFreeParsedEntry( &TempEntry );
					goto exit;
				}
			}
		}

		/* get next header word */
		if (Code = GetWord( FileToParse, &PatchEntryHeader) ) goto exit;
		Code = SeekExaStream( FileToParse, 
			FileToParse->CurPos - 2, 
			EXAPATCH_SEEK_BEGIN, 
			NULL );
		if (Code) goto exit;
	}
# else
	if (EXAPATCH_TEMP_TYPE == (PatchEntryHeader & 0xf) )
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
# endif
# ifdef EXAPATCH_HOOK_SUPPORT
#  ifdef EXAPATCH_LIST_SUPPORT
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#   else
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   endif
#  else
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#   endif
#  endif
	{
		/* do global pre-hook */
		Code = ExaPatchDoHook( HeaderPtr->Comm.lpGlobalPreHook, NULL, HeaderPtr->ActualApplyDir );
		if (Code) goto exit;
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
#   ifdef EXAPATCH_LIST_SUPPORT
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#    else
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#    endif
#   else
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#    endif
#   endif
	{
		if (HeaderPtr->Comm.lpGlobalPreHook && *HeaderPtr->Comm.lpGlobalPreHook)
		{
			Code = EXAPATCH_NOT_SUPPORTED;
			goto exit;
		}
	}
#  endif
# endif
# if defined( EXAPATCH_EH_SUPPORT ) && defined( EXAPATCH_REGISTRY_SUPPORT )
	if ((Code == EXAPATCH_SUCCESS) && (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY)))
	{
		Code = DestKeyHandling2( HeaderPtr, &HeaderPtr->ActualApplyDir, ProgressCallBack, CallbackHandle );
		if (Code) goto exit;
	}
# endif
# ifdef EXAPATCH_LIST_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
# endif
	{
# ifdef EXAPATCH_EH_SUPPORT
		/* process extended header blocks */
		void * Ptrs[3];
		DWORD Processed;
		Ptrs[1] = (void *) HeaderPtr;
		Ptrs[2] = NULL;
		for (i=0; (Code == EXAPATCH_SUCCESS) && (i< HeaderPtr->Comm.dwNumEHBlocks); i++ )
		{
			Processed = FALSE;

			if (ProgressCallBack && (HeaderPtr->Comm.EHBlockPtr[i].ID & 0x8000))
			{
				Ptrs[0] = (void *) (i + HeaderPtr->Comm.EHBlockPtr);
				Code = (*ProgressCallBack)(EXP_PATCH_EXTENDED_HEADER, Ptrs, CallbackHandle);
				if (Code == EXP_CALLBACK_ABORT)
				{
					Code = EXAPATCH_USER_CANCEL;
				}
				else if (Code == EXP_CALLBACK_OK)
				{
					Processed = TRUE;
					Code = EXAPATCH_SUCCESS;
				}
				else
				{
					Code = EXAPATCH_SUCCESS;
				}
			}
			if ((Code == EXAPATCH_SUCCESS) && (!Processed))
			{
				switch (HeaderPtr->Comm.EHBlockPtr[i].ID)
				{
					case ARCH_COST_BLOCKID_GLOBAL:
# ifdef EXAPATCH_LOCKDIR_SUPPORT
					case LOCKDIR_BLOCKID_GLOBAL:
					case LOCKDIRPERM_BLOCKID_GLOBAL:
# endif
# ifdef ADS_SUPPORT
					case ADS_BLOCKID_GLOBAL:
# endif
# ifdef EA_SUPPORT
					case EA_BLOCKID_GLOBAL:
# endif
# ifdef EXAPATCH_REGISTRY_SUPPORT
					case DESTKEY_EH_SIG:
# endif
						// will be handled later (in ExaPatchEntryFinishWork, ExaPatchFinishWork or ExaPatchStartWork)
						break;

					default:
						/* Default EH processing - none defined yet */
						if (HeaderPtr->Comm.EHBlockPtr[i].ID & 0x4000)
						{
							Code = EXAPATCH_MANDATORY_EH_NOT_HANDLED;
						}
						break;

				}
			}
		}
# endif
	}
exit:
	if (Code)
	{
		/* clean up */
		ExaPatchKillTemp( HeaderPtr );
	}

	return(Code);
}
static int ExaPatchFinishWork(
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		DWORD SuccessFlag,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
	)
{
	int Code = EXAPATCH_SUCCESS;

# ifdef EXAPATCH_LINK_SUPPORT
	if ((HeaderPtr->Comm.dwReserved & 1) 
		&& (HeaderPtr->dwExtensionSize >= HUNIQUE_SIZE) 
		&& HeaderPtr->hUnique)
	{
		ExaUniqueClose( HeaderPtr->hUnique );
		HeaderPtr->hUnique = 0;
	}
# endif
# ifdef EXAPATCH_EH_SUPPORT
#  ifdef EXAPATCH_ARCHIVE_SUPPORT
	if ((HeaderPtr->Comm.dwReserved & 1)
		&& (HeaderPtr->dwExtensionSize >= ARCHIVE_SIZE))
	{
		PArchiveInfo pAI = HeaderPtr->pArchiveInfo;

		if (pAI)
		{
			while (pAI->pStack)
			{
				ArchiveStack * pStack = pAI->pStack;

				// this should only occur when an error happens with a pending
				// archive on the stack
				pAI->pStack = pStack->pPrev;
				if (pStack->NewApplyRoot && *pStack->NewApplyRoot)
				{
					// remove temp directory
					rrm( pStack->NewApplyRoot );
				}
				if (pStack->TempArchName && *pStack->TempArchName)
				{
					// delete temporary archive
					ExaDelete( pStack->TempArchName );
				}
				// cleanup stack entry
				if (pStack->PrevApplyDir)
				{
					if (HeaderPtr->ActualApplyDir)
					{
						ExaMemFree( NULL, HeaderPtr->ActualApplyDir );
						pStack->NewApplyRoot = NULL;
					}
					HeaderPtr->ActualApplyDir = pStack->PrevApplyDir;
				}
				if (pStack->PrevSubDir)
				{
					ExaMemFree( NULL, pStack->PrevSubDir );
					pStack->PrevSubDir = NULL;
				}
				if (pStack->pManip)
				{
					ExaPatchFreeManipList( &pStack->pManip, &pStack->pManip );
					pStack->OldArchName = pStack->TempArchName = pStack->NewArchName = NULL;
				}
				else
				{
					if (pStack->NewArchName)
					{
						ExaMemFree( NULL, pStack->NewArchName );
					}
					if (pStack->OldArchName)
					{
						ExaMemFree( NULL, pStack->OldArchName );
					}
					if (pStack->TempArchName)
					{
						ExaMemFree( NULL, pStack->TempArchName );
					}
				}
				ExaMemFree( NULL, pStack );
			}
			while (pAI->pAQHead)
			{
				PArchiveQueueEntry pQE = pAI->pAQHead;
				pAI->pAQHead = pQE->pNext;
				if (pQE->pName)
				{
					ExaMemFree( NULL, pQE->pName );
				}
				ExaMemFree( NULL, pQE );
			}
			if (pAI->pCurSubdir)
			{
				ExaMemFree( NULL, pAI->pCurSubdir );
			}
			ExaMemFree( NULL, pAI );
			HeaderPtr->pArchiveInfo = NULL;
		}
	}
#  endif
# endif
	if (SuccessFlag)
	{
# ifdef EXAPATCH_UNDO_SUPPORT
		/* do pending file manipulations */
		Code = ExaPatchDoGlobal( HeaderPtr, ProgressCallBack, CallbackHandle );
    if (Code)
    {
      if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_KEEPTEMP))
      {
        ExaPatchUndoGlobal( HeaderPtr, ProgressCallBack, CallbackHandle );
				HeaderPtr->dwBW &= ~EXP_BW_BACKUP_USED;
				goto exit2;
      }
      goto exit;
    }
# endif
		/* do global post-hook */
# ifdef EXAPATCH_HOOK_SUPPORT
		Code = ExaPatchDoHook( HeaderPtr->Comm.lpGlobalPostHook, NULL, HeaderPtr->ActualApplyDir );
		if (Code) goto exit;
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
		if (HeaderPtr->Comm.lpGlobalPostHook && *HeaderPtr->Comm.lpGlobalPostHook)
		{
			Code = EXAPATCH_NOT_SUPPORTED;
			goto exit;
		}
#  endif
# endif

# ifdef EXAPATCH_REGISTRYSCRIPT_SUPPORT
		/* do second registry script */
#  ifdef EXAPATCH_LIST_SUPPORT
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
		if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#   else
		if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   endif
#  else
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
		if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#   endif
#  endif
		{
			/* do first reg script */
			if (HeaderPtr->Comm.lpRegScript2)
			{
				Code = ExaPatchDoRegScript( HeaderPtr->Comm.lpRegScript2,
					HeaderPtr->Comm.dwRegScript2Size,
					HeaderPtr->Comm.dwRegScript2Num,
					HeaderPtr );
				if (Code) goto exit;
			}
		}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
#   ifdef EXAPATCH_LIST_SUPPORT
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
		if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#    else
		if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#    endif
#   else
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
		if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#    endif
#   endif
		{
			/* do first reg script */
			if (HeaderPtr->Comm.lpRegScript2)
			{
				Code = EXAPATCH_NOT_SUPPORTED;
				goto exit;
			}
		}
#  endif
# endif
		/* remove temp files */
		ExaPatchKillTemp( HeaderPtr );
# ifdef EXAPATCH_VERBOSE_SUPPORT
		if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERBOSE)
		{
			/* Dump Stats Info*/
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_0 );
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_1, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_MODIFIED] 
						+ HeaderPtr->Stats[EXP_FILES_ADDED]
						+ HeaderPtr->Stats[EXP_FILES_RENAMED]
						+ HeaderPtr->Stats[EXP_FILES_DELETED]
						) );
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_2, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_MODIFIED]));
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_3, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_ADDED]));
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_4, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_RENAMED]));
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_5, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_DELETED]));
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_6, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_SKIPPED]));
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_7, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_MISSING]));
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_8, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_INVALID]));
			ExaPatchInfo( ProgressCallBack, CallbackHandle, EXP_APPLY_STRING_INFO_9, 
				(DWORD) (HeaderPtr->Stats[EXP_FILES_MODIFIED] 
						+ HeaderPtr->Stats[EXP_FILES_ADDED]
						+ HeaderPtr->Stats[EXP_FILES_RENAMED]
						+ HeaderPtr->Stats[EXP_FILES_DELETED]
						+ HeaderPtr->Stats[EXP_FILES_SKIPPED]
						+ HeaderPtr->Stats[EXP_FILES_MISSING]
						+ HeaderPtr->Stats[EXP_FILES_INVALID]
						) );
		}
# endif
		if (0 == (HeaderPtr->dwBW & EXP_BW_ERRORFILE_USED))
		{
			if (HeaderPtr->ErrorFile)
			{
				CloseExaStream( HeaderPtr->ErrorFile );
				ExaMemFree( NULL, HeaderPtr->ErrorFile );
				HeaderPtr->ErrorFile = NULL;
				if (HeaderPtr->ErrorFileName)
				{
					ExaDelete( HeaderPtr->ErrorFileName );
					ExaMemFree( NULL, HeaderPtr->ErrorFileName );
					HeaderPtr->ErrorFileName = NULL;
				}
			}
		}
# ifdef EXAPATCH_LOCKDIR_SUPPORT
		{
			DWORD i;
			DWORD dwFound = 0;
			DWORD dwPerm = 0;
			ExaPatchExtendedHeader * LDEH = NULL;

			for (i=0; !(dwFound) && i<HeaderPtr->Comm.dwNumEHBlocks ; i++ )
			{
				if (HeaderPtr->Comm.EHBlockPtr[i].ID == LOCKDIR_BLOCKID_GLOBAL)
				{
					dwFound = 1;
					LDEH = &HeaderPtr->Comm.EHBlockPtr[i];
				}
				if (HeaderPtr->Comm.EHBlockPtr[i].ID == LOCKDIRPERM_BLOCKID_GLOBAL)
				{
					// mark that we're doing a LOCKDIRPERM header, in which
					// adds have owner/group/permissions attached and there is
					// a third list of directories that should just have their
					// owner/group/permissions altered
					dwFound = 1;
					dwPerm = 1;
					LDEH = &HeaderPtr->Comm.EHBlockPtr[i];
				}
			}
			if (NULL != LDEH)
			{
				unsigned char * Ptr = LDEH->HeaderBlock;
				DWORD dwNumAdds=0, dwNumDels=0, dwNumSets = 0;
				DWORD i;
				wchar_t * pName;
				wchar_t * pFullName;
				int Code2 = EXAPATCH_SUCCESS; /* note: fail silently here */

				dwNumAdds = GetLEDword( Ptr );
				Ptr += 4;
				dwNumDels = GetLEDword( Ptr );
				Ptr += 4;
				if (dwPerm)
				{
					dwNumSets = GetLEDword( Ptr );
					Ptr += 4;
				}
				for (i=0; (Code2 == EXAPATCH_SUCCESS) && (i<dwNumAdds); i++ )
				{
					Code2 = GetLEwchar( Ptr, &pName );
					if (Code2 == EXAPATCH_SUCCESS)
					{
						Ptr += 2*(1+PSwcslen( pName ));
						Code2 = ExaDirMerge( HeaderPtr->ActualApplyDir, pName, &pFullName, PATH_DELIMITER );
						if (Code2 == EXAPATCH_SUCCESS)
						{
							Code2 = ExaMakeDir( pFullName );
							if (dwPerm && (Code2 == EXAPATCH_SUCCESS))
							{
								DWORD dwOwner, dwGroup, dwPermissions;
								ExaPatchFileAttrib Attrib;

								Attrib.Flags = EXP_ATTRIB_ATTRIBUTE;
								dwOwner = GetLEDword( Ptr );
								Ptr += 4;
								dwGroup = GetLEDword( Ptr );
								Ptr += 4;
								dwPermissions = GetLEDword( Ptr );
								Ptr += 4;
								Code2 = ExaSetOwnerGroupNum( pFullName, dwOwner, dwGroup );
								if (Code2 == EXAPATCH_SUCCESS)
								{
									Attrib.Attributes = dwPermissions;
									Code2 = ExaSetFileAttrib( pFullName, &Attrib, NULL, 0 );
								}

							}
							ExaMemFree( NULL, pFullName );
						}
						ExaMemFree( NULL, pName );
					}
				}
				for (i=0; (Code2 == EXAPATCH_SUCCESS) && (i<dwNumDels); i++ )
				{
					Code2 = GetLEwchar( Ptr, &pName );
					if (Code2 == EXAPATCH_SUCCESS)
					{
						Ptr += 2*(1+PSwcslen( pName ));
						Code2 = ExaDirMerge( HeaderPtr->ActualApplyDir, pName, &pFullName, PATH_DELIMITER );
						if (Code2 == EXAPATCH_SUCCESS)
						{
							Code2 = ExaRmDir( pFullName );
							ExaMemFree( NULL, pFullName );
						}
						ExaMemFree( NULL, pName );
					}
				}
				if (dwPerm)
				{
					for (i=0; (Code2 == EXAPATCH_SUCCESS) && (i<dwNumSets) ; i++)
					{
						Code2 = GetLEwchar( Ptr, &pName );
						if (Code2 == EXAPATCH_SUCCESS)
						{
							Ptr += 2*(1+PSwcslen( pName ));
							Code2 = ExaDirMerge( HeaderPtr->ActualApplyDir, pName, &pFullName, PATH_DELIMITER );
							if (Code2 == EXAPATCH_SUCCESS)
							{
								DWORD dwOwner, dwGroup, dwPermissions;
								ExaPatchFileAttrib Attrib;

								Attrib.Flags = EXP_ATTRIB_ATTRIBUTE;
								dwOwner = GetLEDword( Ptr );
								Ptr += 4;
								dwGroup = GetLEDword( Ptr );
								Ptr += 4;
								dwPermissions = GetLEDword( Ptr );
								Ptr += 4;
								Code2 = ExaSetOwnerGroupNum( pFullName, dwOwner, dwGroup );
								if (Code2 == EXAPATCH_SUCCESS)
								{
									Attrib.Attributes = dwPermissions;
									Code2 = ExaSetFileAttrib( pFullName, &Attrib, NULL, 0 );
								}
								ExaMemFree( NULL, pFullName );
							}
							ExaMemFree( NULL, pName );
						}
					}
				}
			}
		}
# endif
	}
	else
	{

# ifdef EXAPATCH_UNDO_SUPPORT
		/* remove global temps */
		if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_KEEPTEMP))
			ExaPatchUndoGlobal( HeaderPtr, ProgressCallBack, CallbackHandle );
# endif
		/* remove temp files */
		ExaPatchKillTemp( HeaderPtr );
		/* undo any reg file changes */
# ifdef EXAPATCH_REGISTRYSCRIPT_SUPPORT
		if (HeaderPtr->RegUndoID != -1)
		{
			ExaPatchDoRegScript( NULL, 0, 0, HeaderPtr );
		}
# endif
# if defined( EXAPATCH_REGISTRY_SUPPORT) && defined( EXAPATCH_EH_SUPPORT )
		if ((HeaderPtr->Comm.dwFileFlags & 0x4000) && (HeaderPtr->DKRmDir))
		{
			rrm( HeaderPtr->DKRmDir );
		}
# endif
	}

exit2:
	/* finish up backup batch file */
# ifdef EXAPATCH_BACKUP_SUPPORT
	if (HeaderPtr->BackupCmdFile)
	{
		/* TODO: possible command file trailer,
		possible Win32/ANSI fiddling about in the Successful case */
		CloseExaStream( HeaderPtr->BackupCmdFile );
		ExaMemFree( NULL, HeaderPtr->BackupCmdFile );
		HeaderPtr->BackupCmdFile = NULL;
		if (0 == (HeaderPtr->dwBW & EXP_BW_BACKUP_USED))
		{
			wchar_t * Ptr;

			ExaDelete( HeaderPtr->BackupCFName );
			Ptr = ExaBaseName( HeaderPtr->BackupCFName, PATH_DELIMITER );
			if (Ptr != HeaderPtr->BackupCFName)
			{
				Ptr[-1] = L'\0';
				ExaRmDir( HeaderPtr->BackupCFName );
			}
		}
	}
# endif
exit:
	return(Code);
}

static int ExaPatchEntryStartWork(
		ExaPatchStream * FileToApply, /* assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		PLATWORD Handle
	)
{
	int Code;
	wchar_t * Ptr = NULL;
	wchar_t * Ptr2 = NULL;
# ifdef EXAPATCH_EH_SUPPORT
	DWORD i;
# endif
	size_t TheLen;

	Code = ExaPatchEntryParseWork(
		FileToApply,
		HeaderPtr,
		EntryPtr,
		FALSE );
	if (Code) goto exit;
# ifdef EXAPATCH_ARCHIVE_SUPPORT
	if ((HeaderPtr->Comm.dwReserved & 1)
		&& (HeaderPtr->dwExtensionSize >= ARCHIVE_SIZE)
		&& ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack)
	{
		// we're in an archive - fudge the file flags
		EntryPtr->Comm.dwFileFlags &= ~(EXP_FILE_BACKUP | EXP_FILE_PATHSEARCH | EXP_FILE_SUBDIRSEARCH | EXP_FILE_IGNOREMISSING);
	}
# endif
  if (EntryPtr->wPatchType == EXAPATCH_EOF_TYPE) goto exit;
	EntryPtr->qwEntryNum = HeaderPtr->qwNextEntryNum++;
	/*do pre-hook */
# ifdef EXAPATCH_HOOK_SUPPORT
#  ifdef EXAPATCH_LIST_SUPPORT
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#   else
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   endif
#  else
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#   endif
#  endif
	{
		Code = ExaPatchDoHook( EntryPtr->Comm.lpDoBefore, EntryPtr->Comm.lpEntryName, 
#   ifdef EXAPATCH_SYSTEM_SUPPORT
			(EntryPtr->Comm.nSystemIndex == -1)?HeaderPtr->ActualApplyDir:HeaderPtr->SystemDirs[EntryPtr->Comm.nSystemIndex] 
#   else
			HeaderPtr->ActualApplyDir 
#   endif
			);
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
#   ifdef EXAPATCH_LIST_SUPPORT
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#    else
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#    endif
#   else
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#    endif
#   endif
	{
		if (EntryPtr->Comm.lpDoBefore && *EntryPtr->Comm.lpDoBefore)
		{
			Code = EXAPATCH_NOT_SUPPORTED;
			goto exit;
		}
	}
#  endif
# endif
	/* process extended header blocks */
# ifdef EXAPATCH_LIST_SUPPORT
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
# endif
	{
# ifdef EXAPATCH_EH_SUPPORT
		void * Ptrs[3];
		DWORD Processed;
		Ptrs[1] = (void *) HeaderPtr;
		Ptrs[2] = (void *) EntryPtr;
		for (i=0; (Code == EXAPATCH_SUCCESS) && (i< EntryPtr->Comm.dwNumEHBlocks); i++ )
		{
			Processed = FALSE;

			if (ProgressCallBack && (EntryPtr->Comm.EHBlockPtr[i].ID & 0x8000))
			{
				Ptrs[0] = (void *) (i + EntryPtr->Comm.EHBlockPtr);
				Code = (*ProgressCallBack)(EXP_PATCH_EXTENDED_HEADER, Ptrs, CallbackHandle);
				if (Code == EXP_CALLBACK_ABORT)
				{
					Code = EXAPATCH_USER_CANCEL;
				}
				else if (Code == EXP_CALLBACK_OK)
				{
					Processed = TRUE;
					Code = EXAPATCH_SUCCESS;
				}
				else
				{
					Code = EXAPATCH_SUCCESS;
				}
			}
			if ((Code == EXAPATCH_SUCCESS) && (!Processed))
			{
				/* Default EH processing - none defined yet */
				switch (EntryPtr->Comm.EHBlockPtr[i].ID)
				{
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					case UPDATE_LINK_TARGET_BLOCKID_LOCAL:
					case UPDATE_DLINK_TARGET_BLOCKID_LOCAL:
						// will be handled later (in ExaPatchEntryFinishWork)
						// assuming EXP_FILE_UPDATE_LINK is set in EntryPtr->Comm.dwFileFlags
						break;
# endif
# ifdef EXAPATCH_OWNERGROUP_SUPPORT
					case OWNERGROUP_BLOCKID_LOCAL:
						// will be handled later (in ExaPatchEntryFinishWork)
						// for now, just mark it
						EntryPtr->wModFlags |= EXP_STATUS_RESERVED2;
						break;
# endif
# ifdef EXAPATCH_ARCHIVE_SUPPORT
					case ARCH_START_BLOCKID_LOCAL:
					case ARCH_START2_BLOCKID_LOCAL:
					case ARCH_START3_BLOCKID_LOCAL:
						Code = ArchiveStart(HeaderPtr, EntryPtr, &EntryPtr->Comm.EHBlockPtr[i] );
						break;
					case ARCH_SUBDIR_BLOCKID_LOCAL:
						Code = ArchiveSubdir(HeaderPtr, EntryPtr, &EntryPtr->Comm.EHBlockPtr[i] );
					case ARCH_END_BLOCKID_LOCAL:
						// will be handled later (in ExaPatchEntryFinishWork)
						break;
# endif
# ifdef ADS_SUPPORT
					case ADS_BLOCKID_LOCAL:
# endif
# ifdef EA_SUPPORT
					case EA_BLOCKID_LOCAL:
# endif
# if defined( ADS_SUPPORT ) || defined( EA_SUPPORT )
						// will be handled later (in ExaPatchEntryFinishWork)
						break;
# endif
					default:
						if (EntryPtr->Comm.EHBlockPtr[i].ID & 0x4000)
						{
							Code = EXAPATCH_MANDATORY_EH_NOT_HANDLED;
						}
						break;
				}
			}
		}
		if (Code) goto exit;
# endif
	}
# ifdef EXAPATCH_ARCHIVE_SUPPORT
# ifdef EXAPATCH_EH_SUPPORT
	Code = ArchiveProcess( HeaderPtr, EntryPtr, Handle );
	if (Code) goto exit;
# endif
# endif

	/* set default entry name */
	if (EntryPtr->Comm.lpEntryName == NULL)
	{
		if (EntryPtr->Comm.Files.dwNumNewFiles)
		{
			TheLen = PSwcslen(EntryPtr->Comm.Files.lpNewPathArray[0]);
			if (TheLen > 0x7ffffffe)
			{
				Code = EXAPATCH_OUT_OF_MEMORY;
			}
			else
			{
				Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) &EntryPtr->Comm.lpEntryName ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			}
			if (Code) goto exit;
			wcscpy_s( EntryPtr->Comm.lpEntryName, TheLen+1, EntryPtr->Comm.Files.lpNewPathArray[0] );
		}
		else if (EntryPtr->Comm.Files.dwNumOldFiles)
		{
			TheLen = PSwcslen(EntryPtr->Comm.Files.lpOldPathArray[0]);
			if (TheLen > 0x7ffffffe)
			{
				Code = EXAPATCH_OUT_OF_MEMORY;
			}
			else
			{
				Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) &EntryPtr->Comm.lpEntryName ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			}
			if (Code) goto exit;
			wcscpy_s( EntryPtr->Comm.lpEntryName, TheLen+1, EntryPtr->Comm.Files.lpOldPathArray[0] );
		}
		else
		{
			// this can happen if a history entry doesn't have an entryname somehow
			Code = ExaMemAlloc(NULL, (DWORD)(sizeof(wchar_t)),(void **) &EntryPtr->Comm.lpEntryName ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			EntryPtr->Comm.lpEntryName[0] = L'\0';
		}
	}
	/*do Entry Start callback */
# ifdef EXAPATCH_LIST_SUPPORT
	/* Do this even if LIST mode...
	if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))*/
# endif
	{
		if (ProgressCallBack && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE))
		{
# ifdef EXAPATCH_ONEFILE_SUPPORT
			if ((0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY)) || (0 == PSFNwcscmp(EntryPtr->Comm.lpEntryName,HeaderPtr->SingleFileName)))
# endif
			{
				if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_FULLPATHDISPLAY)
				{
					/* note that this isn't quite right...the OLD files */
					/*  could actually exist in a subdirectory (or on the */
					/*  path) and this would be wrong! */
					/* the correct way to do this is to wait until */
					/*  after the files are found to do this callback */
					/* TODO: LATER */
# ifdef EXAPATCH_SYSTEM_SUPPORT
					if (EntryPtr->Comm.nSystemIndex == -1)
# endif
					{
						TheLen = PSwcslen(HeaderPtr->ActualApplyDir);
						if (TheLen > 0x7ffffffe)
						{
							Code = EXAPATCH_OUT_OF_MEMORY;
						}
						else
						{
							Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) &Ptr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						}
						if (Code) goto exit;
						wcscpy_s( Ptr, TheLen+1, HeaderPtr->ActualApplyDir );
					}
# ifdef EXAPATCH_SYSTEM_SUPPORT
					else
					{
						TheLen = PSwcslen(HeaderPtr->SystemDirs[(DWORD)EntryPtr->Comm.nSystemIndex]);
						if(TheLen > 0x7ffffffe)
						{
							Code = EXAPATCH_OUT_OF_MEMORY;
						}
						else
						{
							Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) &Ptr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						}
						if (Code) goto exit;
						wcscpy_s( Ptr, TheLen+1, HeaderPtr->SystemDirs[(DWORD)EntryPtr->Comm.nSystemIndex] );
					}
# endif
# ifdef EXAPATCH_SUBDIR_SUPPORT
					if (EntryPtr->Comm.nSubDirIndex == -1)
# endif
					{
						Ptr2 = Ptr;
						Ptr = NULL;
					}
# ifdef EXAPATCH_SUBDIR_SUPPORT
					else
					{
						Code = ExaDirMerge( Ptr, 
							HeaderPtr->Comm.SubDirs->lpPathArray[(DWORD)EntryPtr->Comm.nSubDirIndex],
							&Ptr2,
							PATH_DELIMITER );
						ExaMemFree( NULL, Ptr );
						Ptr = NULL;
						if (Code) goto exit;
					}
# endif
					Code = ExaDirMerge( Ptr2, EntryPtr->Comm.lpEntryName,
						&Ptr, PATH_DELIMITER );
					ExaMemFree( NULL, Ptr2 );
					Ptr2 = NULL;
					if (Code) goto exit;
					Code = (*ProgressCallBack)(EXP_PATCH_FILE_START,
							Ptr,
							CallbackHandle );
					ExaMemFree( NULL, Ptr );
					Ptr = NULL;
				}
				else
				{
# ifdef EXAPATCH_ARCHIVE_SUPPORT
					/* improve user feedback in the exact archive case */
					if ((0 == PSwcscmp(EntryPtr->Comm.lpEntryName,L".arch.intermediate"))
						&& (HeaderPtr->Comm.dwReserved & 1)
						&& (HeaderPtr->dwExtensionSize >= ARCHIVE_SIZE)
						&& ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack) 
					{
						ArchiveStack * pStack = ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack;
						Code = (*ProgressCallBack)( EXP_PATCH_FILE_START,
								ExaBaseName(pStack->OldArchName, PATH_DELIMITER),
								CallbackHandle );
					}
					else
# endif
					{
						Code = (*ProgressCallBack)( EXP_PATCH_FILE_START,
								EntryPtr->Comm.lpEntryName,
								CallbackHandle );
					}
				}
				if (Code == EXP_CALLBACK_ABORT)
				{
					Code = EXAPATCH_USER_CANCEL;
					goto exit;
				}
				else
				{
					Code = EXAPATCH_SUCCESS;
				}
			}
		}
	}
# ifdef EXAPATCH_LIST_SUPPORT
	/*else*/
	{
		/* TODO: LIST*/
		/* Print File List entry info */
	}
# endif
exit:
	if (Ptr)
		ExaMemFree( NULL, Ptr );
	if (Ptr2)
		ExaMemFree( NULL, Ptr2 );
	return(Code);
}

static int ExaPatchEntryFinishWork(
		ExaPatchStream * FileToApply, /* assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		DWORD SuccessFlag
	)
{
	int Code = EXAPATCH_SUCCESS;

  if (EntryPtr->wPatchType == EXAPATCH_EOF_TYPE)
    return EXAPATCH_SUCCESS;

	if (SuccessFlag)
	{
# ifdef EXAPATCH_EH_SUPPORT
#  ifdef EXAPATCH_UPDATELINK_SUPPORT
		if ((Code == EXAPATCH_SUCCESS)
			&& (0U != (EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK)))
		{
			DWORD i;
			DWORD dwFound = 0;
			ExaPatchExtendedHeader * ULEH = NULL;
			DWORD dwDirFlag = FALSE;

			for (i=0; !(dwFound) && i<EntryPtr->Comm.dwNumEHBlocks ; i++ )
			{
				if ((EntryPtr->Comm.EHBlockPtr[i].ID == UPDATE_LINK_TARGET_BLOCKID_LOCAL) || (EntryPtr->Comm.EHBlockPtr[i].ID == UPDATE_DLINK_TARGET_BLOCKID_LOCAL))
				{
					dwFound = 1;
					ULEH = &EntryPtr->Comm.EHBlockPtr[i];
					dwDirFlag = (ULEH->ID == UPDATE_DLINK_TARGET_BLOCKID_LOCAL)? TRUE : FALSE;
				}
			}
			if ((NULL != HeaderPtr->LocalManipList) && (NULL != ULEH))
			{
				Code = ExaSetLinkTarget(HeaderPtr->LocalManipList->IntFile, ULEH->HeaderBlock, dwDirFlag);
			}

		}
#  endif
#  ifdef ADS_SUPPORT
		/* do ADS manipulation */
		if ( (Code == EXAPATCH_SUCCESS)
#   ifdef EXAPATCH_LIST_SUPPORT
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#    else
			&& (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY))
#    endif
#   else
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#    endif
#   endif
			&& ((EntryPtr->wPatchType & 0xf)== EXAPATCH_ADC_TYPE)
			&& EntryPtr->Comm.Files.dwNumNewFiles)
		{
			// only for ADD or CHANGED when actually applying patches
			DWORD i;
			int dwFound = 0;
			ExaPatchExtendedHeader * ADSEH = &ADSDefaultEH;

			for (i=0; !(dwFound) && i< EntryPtr->Comm.dwNumEHBlocks; i++ )
			{
				if (EntryPtr->Comm.EHBlockPtr[i].ID == ADS_BLOCKID_LOCAL)
				{
					dwFound = 1;
					ADSEH = &EntryPtr->Comm.EHBlockPtr[i];
				}
			}
			for (i=0; !(dwFound) && i<HeaderPtr->Comm.dwNumEHBlocks ; i++ )
			{
				if (HeaderPtr->Comm.EHBlockPtr[i].ID == ADS_BLOCKID_GLOBAL)
				{
					dwFound = 1;
					ADSEH = &HeaderPtr->Comm.EHBlockPtr[i];
				}
			}
			Code = DoADS( HeaderPtr->LocalManipList, ADSEH );
		}
#  endif /* ADS_SUPPORT */
#  ifdef EA_SUPPORT
		/* do EA manipulation */
		if ((Code == EXAPATCH_SUCCESS)
#   ifdef EXAPATCH_LIST_SUPPORT
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#    else
			&& (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY))
#    endif
#   else
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT 
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#    endif
#   endif
			&& ((EntryPtr->wPatchType & 0xf)== EXAPATCH_ADC_TYPE)
			&& EntryPtr->Comm.Files.dwNumNewFiles)
		{
			// only for ADD or CHANGED when actually applying patches
			DWORD i;
			int dwFound = 0;
			ExaPatchExtendedHeader * EAEH = &EADefaultEH;

			for (i=0; !(dwFound) && i< EntryPtr->Comm.dwNumEHBlocks; i++ )
			{
				if (EntryPtr->Comm.EHBlockPtr[i].ID == EA_BLOCKID_LOCAL)
				{
					dwFound = 1;
					EAEH = &EntryPtr->Comm.EHBlockPtr[i];
				}
			}
			for (i=0; !(dwFound) && i<HeaderPtr->Comm.dwNumEHBlocks ; i++ )
			{
				if (HeaderPtr->Comm.EHBlockPtr[i].ID == EA_BLOCKID_GLOBAL)
				{
					dwFound = 1;
					EAEH = &HeaderPtr->Comm.EHBlockPtr[i];
				}
			}
			Code = DoEA( HeaderPtr->LocalManipList, EAEH );
		}
#  endif /* EA_SUPPORT */
#  ifdef EXAPATCH_OWNERGROUP_SUPPORT
		if (0U != (EntryPtr->wModFlags & EXP_STATUS_RESERVED2)
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0U == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#   endif
			&& ((EntryPtr->wPatchType & 0xf) == EXAPATCH_ADC_TYPE)
			&& (0U != EntryPtr->Comm.Files.dwNumNewFiles)
			)
		{
			// only for ADD or CHANGED when actually applying patches
			DWORD i;
			int dwFound = 0;
			ExaPatchExtendedHeader * OGEH = NULL;
			
			for (i=0; !(dwFound) && i< EntryPtr->Comm.dwNumEHBlocks; i++ )
			{
				if (EntryPtr->Comm.EHBlockPtr[i].ID == OWNERGROUP_BLOCKID_LOCAL)
				{
					dwFound = 1;
					OGEH = &EntryPtr->Comm.EHBlockPtr[i];
				}
			}
			if (0 != dwFound)
			{
				Code = DoOG( HeaderPtr->LocalManipList, OGEH );
			}
		}
#  endif
# endif /* EXAPATCH_EH_SUPPORT */
		/* do post-hook */

# ifdef EXAPATCH_HOOK_SUPPORT
		if ((Code == EXAPATCH_SUCCESS)
#  ifdef EXAPATCH_LIST_SUPPORT
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#   else
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   endif
#  else
#   ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#   endif
#  endif
			)
		{
			wchar_t * Ptr = EntryPtr->Comm.lpEntryName;

			if (HeaderPtr->LocalManipList && HeaderPtr->LocalManipList->IntFile)
			{
				Ptr = HeaderPtr->LocalManipList->IntFile;
			}
			Code = ExaPatchDoHook( EntryPtr->Comm.lpDoAfter, Ptr, 
#  ifdef EXAPATCH_SYSTEM_SUPPORT
					(EntryPtr->Comm.nSystemIndex == -1)?HeaderPtr->ActualApplyDir:HeaderPtr->SystemDirs[EntryPtr->Comm.nSystemIndex] 
#  else
					HeaderPtr->ActualApplyDir
#  endif
					);
		}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
		if ((Code == EXAPATCH_SUCCESS) 
#   ifdef EXAPATCH_LIST_SUPPORT
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & (EXP_PATCH_APPLY_LISTONLY | EXP_PATCH_APPLY_VERIFYONLY)))
#    else
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#    endif
#   else
#    ifdef EXAPATCH_VERIFYONLY_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))
#    endif
#   endif
			)
		{
			if (EntryPtr->Comm.lpDoAfter && *EntryPtr->Comm.lpDoAfter)
			{
				Code = EXAPATCH_NOT_SUPPORTED;
			}
		}
#  endif
# endif
		if (Code) 
		{
      if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_KEEPTEMP))
			  ExaPatchUndoLocal( HeaderPtr, EntryPtr, ProgressCallBack, CallbackHandle );
		}
		else
		{
			Code = ExaPatchDoLocal( HeaderPtr, EntryPtr, ProgressCallBack, CallbackHandle );
      if (Code)
      {
        if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_KEEPTEMP))
          ExaPatchUndoLocal( HeaderPtr, EntryPtr, ProgressCallBack, CallbackHandle );
      }
		}
		if (Code) goto exit;
	}
	else
	{
    if (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_KEEPTEMP))
		  ExaPatchUndoLocal( HeaderPtr, EntryPtr, ProgressCallBack, CallbackHandle );
	}
# ifdef EXAPATCH_EH_SUPPORT
# ifdef EXAPATCH_ARCHIVE_SUPPORT
		/* handle archive end */
		if ((Code == EXAPATCH_SUCCESS)
#   ifdef EXAPATCH_LIST_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   endif
			)
		{
			DWORD i;

			for (i=0; (Code == EXAPATCH_SUCCESS) && i< EntryPtr->Comm.dwNumEHBlocks; i++ )
			{
				if (EntryPtr->Comm.EHBlockPtr[i].ID == ARCH_END_BLOCKID_LOCAL)
				{
					Code = ArchiveEnd( HeaderPtr, EntryPtr, &EntryPtr->Comm.EHBlockPtr[i], ProgressCallBack, CallbackHandle);
				}
			}
		}
# endif	/* EXAPATCH_ARCHIVE_SUPPORT */
# endif /* EXAPATCH_EH_SUPPORT */
	/* do End Entry Callback */
	if (ProgressCallBack && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE) && (Code == EXAPATCH_SUCCESS))
	{
# ifdef EXAPATCH_ONEFILE_SUPPORT
		if ((0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY)) || (0 == PSFNwcscmp(EntryPtr->Comm.lpEntryName,HeaderPtr->SingleFileName)))
# endif
		{
		  Code = (*ProgressCallBack)( EXP_PATCH_FILE_FINISH,
					NULL,
					CallbackHandle );
			if (Code == EXP_CALLBACK_ABORT)
			{
				Code = EXAPATCH_USER_CANCEL;
			}
			else
			{
				Code = EXAPATCH_SUCCESS;
			}
		}
	}
exit:
	return(Code);
}

static int SetFileLoc(
	wchar_t ** OldFileNameArray,
	wchar_t ** IntFileNameArray,
	wchar_t ** NewFileNameArray,
	QWORD * SizeArray,
 	ExaPatchApplyFileHeaderInfo * HeaderPtr,
	ExaPatchApplyFileEntryInfo * OldEntryPtr,
	ExaPatchApplyFileEntryInfo * NewEntryPtr,
	DWORD dwNumOldFiles,
	DWORD dwNumNewFiles,
	DWORD *pCNCFlag,
	DWORD dwNumManipEntries,
	unsigned short wModFlags,
	DWORD dwFileFlags,
	DWORD dwAttribRetain,
	DWORD * pNumOldMoved,
	DWORD * pNumIntMoved,
	wchar_t * HintSubDir,
	PLATWORD Handle
	)
{
	int Code = EXAPATCH_SUCCESS;
	wchar_t * AuxPtr = NULL;
	wchar_t * AuxPtr2 = NULL;
	wchar_t * Ptr = NULL;
	wchar_t * Ptr2 = NULL;
	size_t TheLen;
	DWORD i;
	ExaPatchApplyFileManipulation * TheManip;
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle; /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
	DWORD TempDirUsed = FALSE;
	DWORD TempDirReallyUsed = FALSE;

	if (TheChannel->pTempDir)
	{
# ifdef EXAPATCH_ARCHIVE_SUPPORT
		if ((0 == (HeaderPtr->Comm.dwReserved & 1))
			|| (HeaderPtr->dwExtensionSize < ARCHIVE_SIZE)
			|| (NULL == (((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack)))
# endif
		{
			TempDirUsed = TRUE;
		}
	}
# endif

	if (dwNumNewFiles
		&& ((0 == dwNumOldFiles) 
			|| (wModFlags & (EXP_STATUS_CONTENTS | EXP_STATUS_ATTRIB | EXP_STATUS_TIMESTAMP 
				| EXP_STATUS_EA | EXP_STATUS_ADS | EXP_STATUS_RESERVED2))))
	{
		/* Add or Changed/contents+attrib+timestamp */
		/* decide on Intermediate file location(s) */
		/* use Ptr for File[0], Ptr2 for File[1-n] */
# ifdef EXAPATCH_ADDFILE_SUPPORT
		if (0 == dwNumOldFiles)
		{ 
			/* ADD */
			Ptr = Ptr2 = HintSubDir;
			Code = ExaMakeDir( HintSubDir );
			if (Code) goto exit;
		}
		else
# endif
		{
			if (dwNumNewFiles > 1)
			{
				if (dwNumOldFiles > 1)
				{
					/* multiples of each */
					TheLen = PSwcslen(OldFileNameArray[0]);
					if (TheLen > 0x7ffffffe)
					{
						Code = EXAPATCH_OUT_OF_MEMORY;
					}
					else
					{
						Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) &AuxPtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					}
					if (Code) goto exit;
					wcscpy_s( AuxPtr, TheLen+1, OldFileNameArray[0] );
					Ptr = ExaBaseName( AuxPtr, PATH_DELIMITER );
					if (Ptr) *Ptr = L'\0';
					Ptr = AuxPtr;
					TheLen = PSwcslen(OldFileNameArray[1]);
					if (TheLen > 0x7ffffffe)
					{
						Code = EXAPATCH_OUT_OF_MEMORY;
					}
					else
					{
						Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) &AuxPtr2 ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					}
					if (Code) goto exit;
					wcscpy_s( AuxPtr2, TheLen+1, OldFileNameArray[1] );
					Ptr2 = ExaBaseName( AuxPtr2, PATH_DELIMITER );
					if (Ptr2) *Ptr2 = L'\0';
					Ptr2 = AuxPtr2;
				}
				else
				{
					/* multiple new, single old */
					TheLen = PSwcslen(OldFileNameArray[0]);
					if (TheLen > 0x7ffffffe)
					{
						Code = EXAPATCH_OUT_OF_MEMORY;
					}
					else
					{
						Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) &AuxPtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					}
					if (Code) goto exit;
					wcscpy_s( AuxPtr, TheLen+1, OldFileNameArray[0] );
					Ptr = ExaBaseName( AuxPtr, PATH_DELIMITER );
					if (Ptr) *Ptr = L'\0';
					Ptr2 = Ptr = AuxPtr;
				}
			}
			else
			{
				/* single new */
				TheLen = PSwcslen(OldFileNameArray[0]);
				if (TheLen > 0x7ffffffe)
				{
					Code = EXAPATCH_OUT_OF_MEMORY;
				}
				else
				{
					Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) &AuxPtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				}
				if (Code) goto exit;
				wcscpy_s( AuxPtr, TheLen+1, OldFileNameArray[0] );
				Ptr = ExaBaseName( AuxPtr, PATH_DELIMITER );
				if (Ptr) *Ptr = L'\0';
				Ptr = AuxPtr;
			}
		}
		/* build Int Names/New Names */
		for (i=0; i<dwNumNewFiles ; i++ )
		{
			if (NewFileNameArray[i])
			{
				ExaMemFree( NULL, NewFileNameArray[i] );
				NewFileNameArray[i] = NULL;
			}
			Code = ExaDirMerge( i?Ptr2:Ptr, 
				NewEntryPtr->Comm.Files.lpNewPathArray[i], 
				NewFileNameArray + i, PATH_DELIMITER );
			if (Code) goto exit;
			// if (TRUE)
			if (NewEntryPtr->Comm.Files.dwNumOldFiles 
				|| EXAPATCH_SUCCESS == ExaExists( NewFileNameArray[i] ))
			{
				/* CHANGED or ADD/Conflict */
# ifdef EXAPATCH_TEMPDIR_SUPPORT
				if ((0U != TempDirUsed) && (0 == (dwFileFlags & EXP_FILE_UPDATE_LINK)))
				{
					dwFileFlags |= EXP_FILE_FORCECOPY;
					TempDirReallyUsed = TRUE;
					Code = ExaEZTemp( TheChannel->pTempDir, IntFileNameArray + i );
				}
				else
# endif
				{
					Code = ExaEZTemp( i?Ptr2:Ptr, IntFileNameArray + i );
				}
				if (Code) goto exit;
				/* create the temp file so that the suffixes will increment correctly */
				{
					HANDLE Handle;

					Handle = ExaOpen( IntFileNameArray[i], FALSE, FALSE );
					if (Handle != INVALID_HANDLE_VALUE)
						ExaClose( Handle );
				}
			}
			else
			{
				/* ADD/no conflict */
				TheLen = PSwcslen(NewFileNameArray[i]);
				if (TheLen > 0x7ffffffe)
				{
					Code = EXAPATCH_OUT_OF_MEMORY;
				}
				else
				{
					Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) (IntFileNameArray + i) );	/* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				}
				if (Code) goto exit;
				wcscpy_s( IntFileNameArray[i], TheLen+1, NewFileNameArray[i] );
			}
		}
	}
	else if (dwNumNewFiles)
	{
		/* Changed/no contents(or attrib+ts) */
		*pCNCFlag = 1;
		if (dwNumOldFiles != dwNumNewFiles)
		{
			Code = EXAPATCH_INVALID_PATCH_FILE;
			goto exit;
		}

		if (wModFlags & EXP_STATUS_NAME)
		{
			size_t SizeDiff;

			for (i=0; i<dwNumNewFiles ; i++ )
			{
				SizeDiff = PSwcslen( NewEntryPtr->Comm.Files.lpNewPathArray[i] )
					- PSwcslen( OldEntryPtr->Comm.Files.lpOldPathArray[i] );
				if (NewFileNameArray[i])
				{
					ExaMemFree( NULL, NewFileNameArray[i] );
					NewFileNameArray[i] = NULL;
				}
				TheLen = SizeDiff + PSwcslen(OldFileNameArray[i]);
				if (TheLen > 0x7ffffffe)
				{
					Code = EXAPATCH_OUT_OF_MEMORY;
				}
				else
				{
					Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) (NewFileNameArray + i) ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				}
				if (Code) goto exit;
				wcsncpy_s( NewFileNameArray[i], TheLen+1, OldFileNameArray[i], PSwcslen( OldFileNameArray[i] ) + SizeDiff );
				Ptr = ExaBaseName( NewFileNameArray[i], PATH_DELIMITER );
				wcscpy_s( Ptr, TheLen+1-(Ptr-NewFileNameArray[i]),NewEntryPtr->Comm.Files.lpNewPathArray[i] );
			}
		}
		else
		{
			for (i=0; i<dwNumNewFiles ; i++ )
			{
				if (NewFileNameArray[i])
				{
					ExaMemFree( NULL, NewFileNameArray[i] );
					NewFileNameArray[i] = NULL;
				}
				TheLen = PSwcslen(OldFileNameArray[i]);
				if (TheLen > 0x7ffffffe)
				{
					Code = EXAPATCH_OUT_OF_MEMORY;
				}
				else
				{
					Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) (NewFileNameArray + i) ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				}
				if (Code) goto exit;
				wcscpy_s( NewFileNameArray[i], TheLen+1, OldFileNameArray[i] );
			}
		}
	}

	/* set up LocalManip */
	for (i=0; i<dwNumManipEntries ; i++ )
	{
		Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyFileManipulation ), (void **) &TheManip ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
    memset( TheManip, 0, sizeof( ExaPatchApplyFileManipulation ) );
		if (Code) goto exit;
		TheManip->PrevManip = HeaderPtr->LocalManipTail;
		if (HeaderPtr->LocalManipTail)
		{
			HeaderPtr->LocalManipTail->NextManip = TheManip;
			HeaderPtr->LocalManipTail = TheManip;
		}
		else
		{
			HeaderPtr->LocalManipList =
				HeaderPtr->LocalManipTail = TheManip;
		}
		if (*pCNCFlag)
		{
			(*pNumOldMoved)++;
			(*pNumIntMoved)++;
			TheManip->IntFile = OldFileNameArray[i];
			TheManip->NewFile = NewFileNameArray[i];
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_SPECIAL_HANDLING;
		}
		else
		{
			if (i < dwNumOldFiles)
			{
				TheManip->OldFile = OldFileNameArray[i];
				(*pNumOldMoved)++;
			}
			if (i < dwNumNewFiles)
			{
				TheManip->IntFile = IntFileNameArray[i];
				TheManip->NewFile = NewFileNameArray[i];
				(*pNumIntMoved)++;
			}
		}
		TheManip->FileManipFlags |= EXP_PATCH_MANIP_DELETERENAME;
		if (dwFileFlags & EXP_FILE_SELFREG)
		{
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_SELFREG;
		}
		if (dwFileFlags & EXP_FILE_SHAREDFILE)
		{
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_SHARED;
		}
# ifdef EXAPATCH_BACKUP_SUPPORT
		if (dwFileFlags & EXP_FILE_BACKUP)
		{
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_BACKUP;
		}
# endif
# ifdef EXAPATCH_UPDATELINK_SUPPORT
		if (dwFileFlags &  EXP_FILE_UPDATE_LINK)
		{
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_TREAT_AS_FILE;
		}
		else
		{
			if ((OldEntryPtr->Comm.Files.dwNumOldFiles == 1)
				&& (OldEntryPtr->Comm.Files.lpOldAttribArray[0].Size == 0)
				&& (EXAPATCH_SUCCESS == ExaIsSymlink(OldFileNameArray[0])))
			{
				TheManip->FileManipFlags |= EXP_PATCH_MANIP_OLDISLINK;
			}
		}
# endif
# if defined( EXAPATCH_FORCECOPY_SUPPORT ) || defined( EXAPATCH_TEMPDIR_SUPPORT )
		if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_RESILIENT)
		{
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_RESILIENT;
		}
		if ((dwFileFlags & EXP_FILE_FORCECOPY) || (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_FORCECOPY))
		{
# ifdef EXAPATCH_ARCHIVE_SUPPORT
			if ((0 == (HeaderPtr->Comm.dwReserved & 1))
				|| (HeaderPtr->dwExtensionSize < ARCHIVE_SIZE)
				|| (NULL == (((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack)))
# endif
			{
				if (0 == (dwFileFlags & EXP_FILE_UPDATE_LINK))
				{
					TheManip->FileManipFlags |= EXP_PATCH_MANIP_PRESERVELINKS_SEMANTICS | EXP_PATCH_MANIP_SEMANTICS_SET;
				}
			}
		}
#  ifdef EXAPATCH_TEMPDIR_SUPPORT
		if (TempDirReallyUsed)
		{
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_USING_TEMPDIR;
		}
#  endif
# endif
		if (dwAttribRetain)
		{
			TheManip->FileManipFlags |= EXP_PATCH_MANIP_RETAIN;
			TheManip->WhichRetain = dwAttribRetain;
		}
	}
exit:
	if (AuxPtr)
		ExaMemFree( NULL, AuxPtr );
	if (AuxPtr2)
		ExaMemFree( NULL, AuxPtr2 );
	return(Code);
	
}
static int AllocNameArrays(
	DWORD dwNumOldFiles,
	DWORD dwNumNewFiles,
	wchar_t *** pOldFileNameArray,
	wchar_t *** pIntFileNameArray,
	wchar_t *** pNewFileNameArray,
	QWORD ** pSizeArray
	)
{
	int Code = EXAPATCH_SUCCESS;
	if (dwNumOldFiles)
	{
		Code = ExaMemAlloc( NULL, dwNumOldFiles*sizeof(char *),(void **) pOldFileNameArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		memset( *pOldFileNameArray, 0, dwNumOldFiles*sizeof(char *) );
	}
	if (dwNumNewFiles)
	{
		Code = ExaMemAlloc( NULL, dwNumNewFiles*sizeof(char *), (void **) pIntFileNameArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		memset( *pIntFileNameArray, 0, dwNumNewFiles*sizeof(char *) );
		Code = ExaMemAlloc( NULL, dwNumNewFiles*sizeof(char *), (void **) pNewFileNameArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code) goto exit;
		memset( *pNewFileNameArray, 0, dwNumNewFiles*sizeof(char *) );
		Code = ExaMemAlloc( NULL, dwNumNewFiles*sizeof(QWORD), (void **) pSizeArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	  if (Code) goto exit;
	}
exit:
	return(Code);
}
static int SetSearchParms(
	ExaPatchApplyFileHeaderInfo * HeaderPtr,
	ExaPatchApplyFileEntryInfo * EntryPtr, 
	DWORD * pSearchFlags, 
	DWORD * pAttribFlags,
	wchar_t ** pSearchPath, 
	wchar_t ** pHintSubDir,
	DWORD HistoryFlag )
{
	int Code = EXAPATCH_SUCCESS;
	wchar_t * Ptr;
	wchar_t * Ptr2;
	size_t TheLen;
	DWORD i;

# ifdef EXAPATCH_ARCHIVE_SUPPORT
	if ((HeaderPtr->Comm.dwReserved & 1)
		&& (HeaderPtr->dwExtensionSize >= ARCHIVE_SIZE)
		&& ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack)
	{
		// we have an active archive
		// set (Ptr == HintSubDir == Ptr2) to subdirectory
		Ptr2 = HeaderPtr->ActualApplyDir;
		if (((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir)
		{
			Code = ExaDirMerge( Ptr2, ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir, 
					pHintSubDir, PATH_DELIMITER );
			if ((*pHintSubDir)[PSwcslen(*pHintSubDir)-1] == PATH_DELIMITER)
			{
				(*pHintSubDir)[PSwcslen(*pHintSubDir)-1] = L'\0';
			}
			Ptr = Ptr2 = *pHintSubDir;
		}
		else
		{
			Ptr = Ptr2;
		}
	}
	else
# endif
	{
# ifdef EXAPATCH_SYSTEM_SUPPORT
		/* set SearchPath, SearchFlags */
		if (EntryPtr->Comm.nSystemIndex == -1)
# endif
		{
			if (HistoryFlag || EntryPtr->Comm.Files.dwNumOldFiles)
			{
				/* Changed/Delete */
				Ptr2 = HeaderPtr->ActualApplyDir;
			}
# ifdef EXAPATCH_ADDFILE_SUPPORT
			else
			{
				/* Add */
				Ptr2 = HeaderPtr->ScratchDir;
			}
# endif
		}
# ifdef EXAPATCH_SYSTEM_SUPPORT
		else
		{
			if (EntryPtr->Comm.nSystemIndex < HeaderPtr->Comm.Systems->dwNumSystems)
			{
				Ptr2 = HeaderPtr->SystemDirs[EntryPtr->Comm.nSystemIndex];
			}
			else
			{
				Code = EXAPATCH_INVALID_PATCH_FILE;
				goto exit;
			}
		}
# endif
		/* Ptr2 == expected base directory */
		/* set Ptr to expected actual directory */
		Ptr = Ptr2;
# ifdef EXAPATCH_SUBDIR_SUPPORT
		if (EntryPtr->Comm.nSubDirIndex != -1)
		{
			if (EntryPtr->Comm.nSubDirIndex < HeaderPtr->Comm.SubDirs->dwNumSubDirs)
			{
#  ifdef EXAPATCH_ADDWITHFILE_SUPPORT
				if ((!HistoryFlag) && (EntryPtr->Comm.Files.dwNumOldFiles == 0) && (EntryPtr->Comm.dwFileFlags & EXP_FILE_ADDWITHFILE))
				{
					/* do ADDWITHFILE processing */
					/* Build (HintSubDir == Ptr) by locating SubDir [file] under Ptr2 */
					Code = ExaPatchFileSearch( pHintSubDir,
							ExaBaseName(HeaderPtr->Comm.SubDirs->lpPathArray[EntryPtr->Comm.nSubDirIndex],PATH_DELIMITER),
							Ptr2,
#   ifdef _WIN32
							EXP_SEARCH_FULL_RECURSE | EXP_SEARCH_IGNORE_CASE
#   else
							EXP_SEARCH_FULL_RECURSE
#   endif
#   ifdef EXAPATCH_LINK_SUPPORT
								+ ((HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_IGNORELINKS)?EXP_SEARCH_IGNORELINKS:0)
#   endif
# ifdef EXAPATCH_UPDATELINK_SUPPORT
						 		+ ((HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_PROCESSLINKS)?EXP_SEARCH_PROCESSLINKS:0)
# endif
							,NULL,NULL,NULL,NULL,NULL,PATH_DELIMITER, PATH_SEPARATOR, NULL, 0, NULL, 0);
					if (Code)
					{
						/* couldn't find file - use Ptr2 itself */
						TheLen = PSwcslen(Ptr2);
						if (TheLen > 0x7ffffffe)
						{
							Code = EXAPATCH_OUT_OF_MEMORY;
						}
						else
						{
							Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) pHintSubDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						}
						if (Code) goto exit;
						wcscpy_s( *pHintSubDir, TheLen+1, Ptr2 );
					}
					else
					{
						*ExaBaseName( *pHintSubDir, PATH_DELIMITER ) = L'\0';
					}
				}
				else
#  else
#   ifndef EXAPATCH_SILENT_UNSUPPORT
				if ((!HistoryFlag) && (EntryPtr->Comm.Files.dwNumOldFiles == 0) && (EntryPtr->Comm.dwFileFlags & EXP_FILE_ADDWITHFILE))
				{
					Code = EXAPATCH_NOT_SUPPORTED;
					goto exit;
				}
				else
#   endif
#  endif			
				{
					/* Build (HintSubDir == Ptr) from Ptr2 and SubDir */
					Code = ExaDirMerge( Ptr2, HeaderPtr->Comm.SubDirs->lpPathArray[EntryPtr->Comm.nSubDirIndex], pHintSubDir, PATH_DELIMITER );
					if (Code) goto exit;
				}
				if ((*pHintSubDir)[PSwcslen(*pHintSubDir)-1] == PATH_DELIMITER)
				{
					(*pHintSubDir)[PSwcslen(*pHintSubDir)-1] = L'\0';
				}
				Ptr = *pHintSubDir;
			}
			else
			{
				Code = EXAPATCH_INVALID_PATCH_FILE;
				goto exit;
			}
		}
# endif
	}
# ifdef EXAPATCH_PATHSEARCH_SUPPORT
	if (EntryPtr->Comm.dwFileFlags & EXP_FILE_PATHSEARCH)
	{
		/* Set SearchPath to <Ptr2>;<Path>*/
# ifdef WINCE
   /* WINCE TODO: HKLM\Loader\SystemPath contains the PATH info */
# else /* WINCE */
#  ifdef _WIN32
		i = GetEnvironmentVariableW( L"PATH", NULL, 0 );
		TheLen = PSwcslen(Ptr2)+i;
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) pSearchPath ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code) goto exit;
		wcscpy_s( *pSearchPath, TheLen+1, Ptr2 );
		Ptr2 = (*pSearchPath) + PSwcslen( *pSearchPath );
		*Ptr2++ = L';';
		GetEnvironmentVariableW( L"PATH", Ptr2, i );
#  else /* _WIN32 */
		if (getenv("PATH"))
		{
			i = strlen( getenv("PATH") ) + 1;
			Code = ExaMemAlloc( NULL, sizeof(wchar_t)*(PSwcslen(Ptr2)+1+i), (void **) pSearchPath ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			PSwcscpy( *pSearchPath, Ptr2 );
			Ptr2 = (*pSearchPath) + PSwcslen( *pSearchPath );
			*Ptr2++ = L':';
			if (-1 == PSmbstowcs( Ptr2, getenv("PATH"), i ))
			{
				Code = EXAPATCH_CHARSET_ERROR;
				goto exit;
			}
		}
		else
		{
			Code = ExaMemAlloc( NULL, sizeof(wchar_t)*(PSwcslen(Ptr2)+1), (void **) pSearchPath ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code) goto exit;
			PSwcscpy( *pSearchPath, Ptr2 );
		}
#  endif /* _WIN32 */
# endif /* WINCE */
	}
	else
# else
	if (EntryPtr->Comm.dwFileFlags & EXP_FILE_PATHSEARCH)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
	else
# endif			
	{
		/* Set SearchPath to Ptr2 */
		TheLen = PSwcslen(Ptr2);
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(1+TheLen)), (void **) pSearchPath ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code) goto exit;
		wcscpy_s( *pSearchPath, TheLen+1, Ptr2 );
	}
	/* Ptr2 is invalid now */
	*pSearchFlags = EXP_SEARCH_VERIFY_ATTRIB | EXP_SEARCH_IGNORE_CASE | EXP_SEARCH_VERIFY_CKSUM;
	*pAttribFlags = EXP_ATTRIB_SIZE;
# ifdef EXAPATCH_SUBDIRSEARCH_SUPPORT
	if (EntryPtr->Comm.dwFileFlags & EXP_FILE_SUBDIRSEARCH)
	{
		*pSearchFlags |= EXP_SEARCH_RECURSE;
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if (EntryPtr->Comm.dwFileFlags & EXP_FILE_SUBDIRSEARCH)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit;
	}
#  endif
# endif
	if (EntryPtr->Comm.dwFileFlags & EXP_FILE_CHECKATTRIB)
	{
		*pAttribFlags |= EXP_ATTRIB_ATTRIBUTE;
	}
	if (EntryPtr->Comm.dwFileFlags & EXP_FILE_CHECKTIMEDATE)
	{
		/* Probably shouldn't check Create date */
		/* AttribFlags |= EXP_ATTRIB_CHANGED_DATE | EXP_ATTRIB_CREATE_DATE; */
		*pAttribFlags |= EXP_ATTRIB_CHANGED_DATE;
	}
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_NOTZCHECK)
	{
		*pSearchFlags |= EXP_SEARCH_IGNORE_TIMEZONE;
	}
# ifdef EXAPATCH_LINK_SUPPORT
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_IGNORELINKS)
	{
		*pSearchFlags |= EXP_SEARCH_IGNORELINKS;
	}
# endif
# ifdef EXAPATCH_UPDATELINK_SUPPORT
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_PROCESSLINKS)
	{
		*pSearchFlags |= EXP_SEARCH_PROCESSLINKS;
	}
# endif
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_ALLOWPAD)
	{
		*pSearchFlags |= EXP_SEARCH_ALLOWPAD;
	}
	if (!*pHintSubDir)
	{
		/* copy Ptr to HintSubDir */
		TheLen=PSwcslen(Ptr);
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen + 1)), (void **) pHintSubDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code) goto exit;
		wcscpy_s( *pHintSubDir, TheLen+1, Ptr );
		if ((*pHintSubDir)[PSwcslen(*pHintSubDir)-1] == PATH_DELIMITER)
		{
			(*pHintSubDir)[PSwcslen(*pHintSubDir)-1] = L'\0';
		}
	}
	if (*pHintSubDir)
	{
		/* prepend HintSubDir to SearchPath */
		if (*pSearchFlags & EXP_SEARCH_RECURSE)
		{
			*pSearchFlags |= EXP_SEARCH_SPECIAL_RECURSE;
		}
		if (*pSearchPath)
			TheLen = PSwcslen( *pHintSubDir ) +  PSwcslen( *pSearchPath ) + 1;
		else
			TheLen = PSwcslen( *pHintSubDir );
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			 Code = ExaMemAlloc( NULL, (DWORD)(sizeof(wchar_t)*(TheLen+1)), (void **) &Ptr2 ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code) goto exit;

		wcscpy_s( Ptr2, TheLen+1, *pHintSubDir );
		if (*pSearchPath)
		{
			Ptr = Ptr2 + PSwcslen( Ptr2 );
			TheLen -= PSwcslen( Ptr2 ) + 1;
			*Ptr++ = PATH_SEPARATOR;
			wcscpy_s( Ptr, TheLen+1, *pSearchPath );
			ExaMemFree( NULL, *pSearchPath );
		}
		*pSearchPath = Ptr2;
	}

	/* Ptr is invalid now */
exit:
	return(Code);
}
# if defined( EA_SUPPORT ) || defined( ADS_SUPPORT )
static int WildMatchList( DWORD NumSpecs, char ** SpecList, char * Name );
static int WildMatch( char * Spec, char * Name );
# endif
static int ExaPatchEntryFindFiles(
		ExaPatchStream * FileToApply,
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		ExaPatchStream ** OldStreamPtr,
		ExaPatchStream ** NewStreamPtr,
		PLATWORD Handle
	)
{
	/* QUESTION: where to store temp names??? */
	/* ANSWER: in LocalManipList */
	/* NOTE: this also needs to do the version checking */
	/*	and advancing for history patches */
	int Code = EXAPATCH_SUCCESS;
	wchar_t * SearchPath = NULL;
	wchar_t * HintSubDir = NULL;
	wchar_t ** OldFileNameArray = NULL;
	wchar_t ** IntFileNameArray = NULL;
	wchar_t ** NewFileNameArray = NULL;
	DWORD MaxOldFiles = 0;
	ExaPatchApplyFileEntryInfo ** HistoryEntryArray = NULL;
	ExaPatchApplyFileEntryInfo * LastHistoryEntry = NULL;
	ExaPatchApplyFileEntryInfo * FirstHistoryEntry = NULL;
	unsigned short HistoryModFlags;
	DWORD SearchFlags;
	DWORD AttribFlags;
	QWORD * SizeArray = NULL;
	DWORD dwNumManipEntries;
	DWORD i;
	DWORD Count;
	DWORD NumIntMoved = 0;
	DWORD NumOldMoved = 0;
	DWORD CNCFlag = 0;
	DWORD InvalidFlag = FALSE;
	QWORD qwRestorePos = FileToApply->CurPos;
# ifdef EXAPATCH_ARCHIVE_SUPPORT
	DWORD dwInArchive = 0;
# endif

# ifdef EXAPATCH_ARCHIVE_SUPPORT
	if ((HeaderPtr->Comm.dwReserved & 1)
		&& (HeaderPtr->dwExtensionSize >= ARCHIVE_SIZE)
		&& ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack)
	{
		dwInArchive = 1;
	}
# endif
	switch (EntryPtr->wPatchType & 0xf)
	{
		case EXAPATCH_HISTORY_TYPE:
			/* handle history entries */
# ifdef EXAPATCH_HISTORY_SUPPORT
			// set search parameters
			Code = SetSearchParms( HeaderPtr, EntryPtr, 
				&SearchFlags, &AttribFlags, &SearchPath, &HintSubDir, TRUE );
			if (Code) goto exit;
			// parse patch file forward, remembering positions of subpatches
			Code = ExaMemAlloc( NULL, EntryPtr->Comm.dwNumHistoryVersions * sizeof( void *), (void **) &HistoryEntryArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code == EXAPATCH_SUCCESS)
			{
				memset( HistoryEntryArray, 0, EntryPtr->Comm.dwNumHistoryVersions * sizeof( void * ) );
				for (i=0; (!Code) && (i<EntryPtr->Comm.dwNumHistoryVersions) ; i++ )
				{
					Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyFileEntryInfo ), (void **) &HistoryEntryArray[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				}
			}
			if (Code) goto exit;
			for (i=0; (!Code) && (i<EntryPtr->Comm.dwNumHistoryVersions) ; i++ )
			{
				Code = ExaPatchEntryParseWork( FileToApply, HeaderPtr, HistoryEntryArray[i], TRUE );
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = SeekExaStream( FileToApply, HistoryEntryArray[i]->qwEntrySize,
							EXAPATCH_SEEK_CUR, NULL );
					LastHistoryEntry = HistoryEntryArray[i];
					if (MaxOldFiles < LastHistoryEntry->Comm.Files.dwNumOldFiles)
					{
						MaxOldFiles = LastHistoryEntry->Comm.Files.dwNumOldFiles;
					}
				}
			}
			if (Code) goto exit;
			// see if last NEW files exist (unless we're allowing dup's)
			{
				wchar_t * TempName = NULL;


				if (LastHistoryEntry->Comm.Files.dwNumNewFiles && 0 == (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_ALLOWDUP))
				{
					for (i=0, Count=0 ; i<LastHistoryEntry->Comm.Files.dwNumNewFiles; i++ )
					{
						LastHistoryEntry->Comm.Files.lpNewAttribArray[i].Flags = AttribFlags;
						Code = ExaPatchFileSearch(
							&TempName,
							LastHistoryEntry->Comm.Files.lpNewPathArray[i],
							SearchPath,
							SearchFlags | EXP_SEARCH_CHECK_NEW_DISC,
							LastHistoryEntry->Comm.Files.lpNewAttribArray + i,
							LastHistoryEntry->Comm.Files.lpNewCksumArray + i,
							LastHistoryEntry->NewRegionListArray?LastHistoryEntry->NewRegionListArray[i]:NULL,
							NULL,
							LastHistoryEntry->Comm.Files.lpDiscArray,
							PATH_DELIMITER,
							PATH_SEPARATOR,
							NULL, 0, NULL, 0 );
						if (TempName)
						{
							ExaMemFree( NULL, TempName );
							TempName = NULL;
						}
						if (Code == EXAPATCH_SUCCESS)
						{
							Count++;
						}
						else if (Code != EXAPATCH_FILE_NOT_FOUND)
						{
							goto exit;
						}
					}
					if ((Count == LastHistoryEntry->Comm.Files.dwNumNewFiles) 
						&& (0 == (LastHistoryEntry->wModFlags & (EXP_STATUS_EA | EXP_STATUS_ADS))))
					{
						Code = EXAPATCH_ALREADY_PATCHED;
						goto exit;
					}
				}

			}
			// alloc name arrays
			Code = AllocNameArrays( MaxOldFiles, 
					LastHistoryEntry->Comm.Files.dwNumNewFiles,
					&OldFileNameArray,
					&IntFileNameArray,
					&NewFileNameArray,
					&SizeArray );
			// mark "final delete" for optimization in ExaPatchEntryDoWork
			if (LastHistoryEntry->Comm.Files.dwNumNewFiles == 0)
			{
				EntryPtr->wModFlags |= EXP_STATUS_RESERVED;
			}
			//walk patches backwards, looking for OLD file(s)
			{
				DWORD Found = FALSE;
				DWORD j;
				wchar_t * ThisFileName;
				ExaPatchFileAttrib * pThisAttrib;
				ExaPatchFileChecksum * pThisCkSum;


				HistoryModFlags = 0;

				for (j=EntryPtr->Comm.dwNumHistoryVersions; (!Found) && j ; j-- )
				{
					HistoryModFlags |= HistoryEntryArray[j-1]->wModFlags;
					for (i=0, Count = 0; i < HistoryEntryArray[j-1]->Comm.Files.dwNumOldFiles ; i++ )
					{
						ThisFileName = (HistoryEntryArray[j-1]->Comm.Files.lpOldPathArray)?
							HistoryEntryArray[j-1]->Comm.Files.lpOldPathArray[i]:
							HistoryEntryArray[j-2]->Comm.Files.lpNewPathArray[i];
						pThisAttrib = (HistoryEntryArray[j-1]->Comm.Files.lpOldAttribArray)?
							HistoryEntryArray[j-1]->Comm.Files.lpOldAttribArray + i:
							HistoryEntryArray[j-2]->Comm.Files.lpNewAttribArray + i;
						pThisCkSum = (HistoryEntryArray[j-1]->Comm.Files.lpOldCksumArray)?
							HistoryEntryArray[j-1]->Comm.Files.lpOldCksumArray + i:
							HistoryEntryArray[j-2]->Comm.Files.lpNewCksumArray + i;
						pThisAttrib->Flags = AttribFlags;
						Code = ExaPatchFileSearch(
							OldFileNameArray + i,
							ThisFileName,
							SearchPath,
							SearchFlags | EXP_SEARCH_CHECK_OLD_DISC,
							pThisAttrib,
							pThisCkSum,
							HistoryEntryArray[j-1]->OldRegionListArray?HistoryEntryArray[j-1]->OldRegionListArray[i]:NULL,
							&HeaderPtr->DupList,
							HistoryEntryArray[j-1]->Comm.Files.lpDiscArray,
							PATH_DELIMITER,
							PATH_SEPARATOR,
							NULL,0,NULL,0);
# ifdef EXAPATCH_LINK_SUPPORT
						if ((Code == EXAPATCH_SUCCESS) && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_LINKUNIQUE))
						{
							UniqueFileID UFI;
							int UCode;

							if (0 == ExaGetFileIDW( &UFI, OldFileNameArray[i]))
							{
								UCode = ExaUnique( HeaderPtr->hUnique, &UFI );
								if (UCode > 0)
								{
									// let it go if an error occurred
									Code = EXAPATCH_ALREADY_PATCHED;
								}
							}
						}
# endif
						if (Code == EXAPATCH_SUCCESS)
						{
							Count++;
						}
						else if (Code != EXAPATCH_FILE_NOT_FOUND)
						{
							goto exit;
						}
						else
						{
							if (OldFileNameArray[i])
							{
								InvalidFlag = TRUE;
							}
						}
					}
					if (Count != HistoryEntryArray[j-1]->Comm.Files.dwNumOldFiles)
					{
						for (i=0; i<MaxOldFiles ; i++ )
						{
							if (OldFileNameArray[i])
							{
								ExaMemFree( NULL, OldFileNameArray[i] );
								OldFileNameArray[i] = NULL;
							}
						}
					}
					else
					{
						Found = TRUE;
						EntryPtr->dwHistoryCurrentVersion = j-1;
						FirstHistoryEntry = HistoryEntryArray[j-1];
					}
				}
				if (!Found)
				{
					Code = InvalidFlag?EXAPATCH_INVALID_FILE_FOUND:EXAPATCH_FILE_NOT_FOUND;
					if (LastHistoryEntry->Comm.Files.dwNumNewFiles == 0)
					{
						Code = EXAPATCH_ALREADY_PATCHED;
					}
					goto exit;
				}
			}
			//set file locations
			dwNumManipEntries = LastHistoryEntry->Comm.Files.dwNumNewFiles;
			if (dwNumManipEntries < FirstHistoryEntry->Comm.Files.dwNumOldFiles)
				dwNumManipEntries = FirstHistoryEntry->Comm.Files.dwNumOldFiles;
			if (0 == dwNumManipEntries)
			{
				// Kerry - I am not sure what the right thing to do is here.
				// In a history patch that includes a DELETE, this is causing an
				// error, whereas ignoring the condition permits the patch to run
				// completion. The history patch entry is not optimal (ADD/DELETE),
				// but I'm not sure the intent of this check...
				//Code = EXAPATCH_INVALID_PATCH_FILE;
				//goto exit;
			}
			Code = SetFileLoc( OldFileNameArray, 
					IntFileNameArray, 
					NewFileNameArray,
					SizeArray, 
					HeaderPtr, 
					FirstHistoryEntry, 
					LastHistoryEntry, 
					FirstHistoryEntry->Comm.Files.dwNumOldFiles,
					LastHistoryEntry->Comm.Files.dwNumNewFiles, 
					&CNCFlag, 
					dwNumManipEntries, 
					HistoryModFlags, 
					EntryPtr->Comm.dwFileFlags, 
					EntryPtr->Comm.dwAttribRetain,
					&NumOldMoved, 
					&NumIntMoved,
					HintSubDir,
					Handle );
			//position patchfile stream
			Code = SeekExaStream( FileToApply, 
				FirstHistoryEntry->qwEntryPos, 
				EXAPATCH_SEEK_BEGIN,
				NULL );
			//open OLD file stream
			*NewStreamPtr = *OldStreamPtr = NULL;
			if (FirstHistoryEntry->Comm.Files.dwNumOldFiles)
			{
				Code = ExaPatchApplyOpenFileArrayAsStreamW(
					OldStreamPtr,
					OldFileNameArray,
					FirstHistoryEntry->Comm.Files.dwNumOldFiles,
					EXP_OPEN_READONLY,
					NULL );
				if (Code) goto exit;
			}
			//open final NEW file stream (int file)
			if (LastHistoryEntry->Comm.Files.dwNumNewFiles)
			{
				for (i=0; i<LastHistoryEntry->Comm.Files.dwNumNewFiles ; i++ )
				{
					SizeArray[i] = LastHistoryEntry->Comm.Files.lpNewAttribArray[i].Size;
				}
				Code = ExaPatchApplyOpenFileArrayAsStreamW(
					NewStreamPtr,
					IntFileNameArray,
					LastHistoryEntry->Comm.Files.dwNumNewFiles,
					0,
					SizeArray );
			}


# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
			return(EXAPATCH_NOT_SUPPORTED);
			
#  endif
# endif
			break;
		case EXAPATCH_ADC_TYPE:
			/* handle Add/Delete/Change entries */
			dwNumManipEntries = EntryPtr->Comm.Files.dwNumOldFiles;
			if (dwNumManipEntries < EntryPtr->Comm.Files.dwNumNewFiles)
				dwNumManipEntries = EntryPtr->Comm.Files.dwNumNewFiles;
			if (0 == dwNumManipEntries)
			{
				return(EXAPATCH_INVALID_PATCH_FILE);
			}
			Code = SetSearchParms( HeaderPtr, EntryPtr, 
				&SearchFlags, &AttribFlags, &SearchPath, &HintSubDir, FALSE );
			if (Code) goto exit;
			/*
			At this point: SearchFlags is set correctly
			AttribFlags is set correctly
			SearchPath is set correctly
			HintSubDir is set correctly
			*/
			/* NOTE: dup handling is perhaps not quite right - */
			/*  Do we need a separate call (after success) */
			/*  to add OLD names to dup list? */

			/* allocate the various arrays */
			Code = AllocNameArrays( EntryPtr->Comm.Files.dwNumOldFiles,
					EntryPtr->Comm.Files.dwNumNewFiles,
					&OldFileNameArray,
					&IntFileNameArray,
					&NewFileNameArray,
					&SizeArray );
			if (Code) goto exit;
			/* first, see if the New files already exist (unless we're allowing duplicates) */
			if (EntryPtr->Comm.Files.dwNumNewFiles && 0 == (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_ALLOWDUP))
			{
				for (i=0, Count=0; i<EntryPtr->Comm.Files.dwNumNewFiles ; i++ )
				{
          EntryPtr->Comm.Files.lpNewAttribArray[i].Flags = AttribFlags;
					Code = ExaPatchFileSearch(
						NewFileNameArray + i,
						EntryPtr->Comm.Files.lpNewPathArray[i],
            SearchPath,
						SearchFlags | EXP_SEARCH_CHECK_NEW_DISC,
						EntryPtr->Comm.Files.lpNewAttribArray + i,
						EntryPtr->Comm.Files.lpNewCksumArray + i,
            EntryPtr->NewRegionListArray?EntryPtr->NewRegionListArray[i]:NULL,
            NULL,
						EntryPtr->Comm.Files.lpDiscArray,
						PATH_DELIMITER,
						PATH_SEPARATOR,
						NULL, 0, NULL, 0 );
					if (Code == EXAPATCH_SUCCESS)
					{
						Count++;
					}
					else if (Code != EXAPATCH_FILE_NOT_FOUND)
					{
						goto exit;
					}
				}

				/* if they ALL do, then exit with EXAPATCH_ALREADY_PATCHED */
				if ((Count == EntryPtr->Comm.Files.dwNumNewFiles) && (0 == (EntryPtr->wModFlags & EXP_STATUS_ADS))
# ifdef EA_SUPPORT
					//&& (0U == (EntryPtr->wModFlags & EXP_STATUS_EA))
# endif
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					//&& (0U == (EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK))
# endif
# ifdef EXAPATCH_OWNERGROUP_SUPPORT
					//&& (0U == (EntryPtr->wModFlags & EXP_STATUS_RESERVED2))
# endif
					)
				{
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					DWORD dwLinkOK = TRUE;

					if ((0U != (EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK))
						&& (EXAPATCH_SUCCESS == ExaIsSymlink(NewFileNameArray[0])))
					{
						// check link target
						char * LinkTarget;
						DWORD LinkTargetSize;
						DWORD dwFound = 0;
						ExaPatchExtendedHeader * ULEH = NULL;
						DWORD i;

						ExaGetLinkTarget( NewFileNameArray[0], NULL, 0, &LinkTargetSize );
						if (EXAPATCH_SUCCESS == ExaMemAlloc(NULL, LinkTargetSize, (void **) &LinkTarget))
						{
							if (EXAPATCH_SUCCESS == ExaGetLinkTarget(NewFileNameArray[0], LinkTarget, LinkTargetSize, NULL))
							{
								for (i=0; !(dwFound) && i<EntryPtr->Comm.dwNumEHBlocks ; i++ )
								{
									if ((EntryPtr->Comm.EHBlockPtr[i].ID == UPDATE_LINK_TARGET_BLOCKID_LOCAL) || (EntryPtr->Comm.EHBlockPtr[i].ID == UPDATE_DLINK_TARGET_BLOCKID_LOCAL))
									{
										dwFound = 1;
										ULEH = &EntryPtr->Comm.EHBlockPtr[i];
									}
								}
								if (dwFound)
								{
									if (0 != strcmp(LinkTarget,ULEH->HeaderBlock))
									{
										dwLinkOK = FALSE;
									}
								}
							}
							ExaMemFree( NULL, LinkTarget );
						}

					}
					if (0U != dwLinkOK)
# endif
					{
# ifdef EXAPATCH_OWNERGROUP_SUPPORT
						DWORD dwOGOK = TRUE;
						if (0U != (EntryPtr->wModFlags & EXP_STATUS_RESERVED2))
						{
							// check owner/group
							DWORD i;
							int dwFound = 0;
							ExaPatchExtendedHeader * OGEH = NULL;
							DWORD dwOwner, dwGroup;
							unsigned char OwnerType;
							unsigned char GroupType;
							void * OwnerDatum;
							void * GroupDatum;
							unsigned char * Ptr;
							DWORD dwDesiredOwner = 0xffffffff;
							DWORD dwDesiredGroup = 0xffffffff;

							if (EXAPATCH_SUCCESS == ExaGetOwnerGroupNum(NewFileNameArray[0], &dwOwner, &dwGroup))
							{
								for (i=0; !(dwFound) && i< EntryPtr->Comm.dwNumEHBlocks; i++ )
								{
									if (EntryPtr->Comm.EHBlockPtr[i].ID == OWNERGROUP_BLOCKID_LOCAL)
									{
										dwFound = 1;
										OGEH = &EntryPtr->Comm.EHBlockPtr[i];
									}
								}
								if (0 != dwFound)
								{
									Ptr = OGEH->HeaderBlock;
									OwnerType = Ptr[0] & 0xf;
									GroupType = (Ptr[0] & 0xf0) >> 4;
									Ptr++;
									OwnerDatum = (void *)&Ptr[1];
									GroupDatum = (void *)&Ptr[1+(Ptr[0])];
									Ptr = (unsigned char *) OwnerDatum;
									if (OwnerType == 2)
									{
										dwDesiredOwner = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8);
									}
									else if (OwnerType == 3)
									{
										dwOwner = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8)
											+ (((DWORD)(Ptr[2] & 0xff)) << 16) + (((DWORD)(Ptr[3] & 0xff)) << 24);
									}
									Ptr = (unsigned char *) GroupDatum;
									if (GroupType == 2)
									{
										dwDesiredGroup = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8);
									}
									else if (GroupType == 3)
									{
										dwDesiredGroup = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8)
											+ (((DWORD)(Ptr[2] & 0xff)) << 16) + (((DWORD)(Ptr[3] & 0xff)) << 24);
									}
									if (((dwDesiredOwner != 0xffffffff)&&(dwDesiredOwner != dwOwner))
										|| ((dwDesiredGroup != 0xffffffff)&&(dwDesiredGroup != dwGroup)))
									{
										dwOGOK = FALSE;
									}
								}
							}
						}
						if (0U != dwOGOK)
# endif
						{
# ifdef EA_SUPPORT
							DWORD dwEAOK = TRUE;

							if (0U != (EntryPtr->wModFlags & EXP_STATUS_EA))
							{
								DWORD i;
								int dwFound = 0;
								ExaPatchExtendedHeader * EAEH = NULL;

								for (i=0; !(dwFound) && i < EntryPtr->Comm.dwNumEHBlocks; i++ )
								{
									if (EntryPtr->Comm.EHBlockPtr[i].ID == EA_BLOCKID_LOCAL)
									{
										dwFound = 1;
										EAEH = &EntryPtr->Comm.EHBlockPtr[i];
									}
								}
								for (i=0; !(dwFound) && i<HeaderPtr->Comm.dwNumEHBlocks; i++ )
								{
									if (HeaderPtr->Comm.EHBlockPtr[i].ID == EA_BLOCKID_GLOBAL)
									{
										dwFound = 1;
										EAEH = &HeaderPtr->Comm.EHBlockPtr[i];
									}
								}
								if (dwFound)
								{
									// check EAs
									char * Ptr;
									char * BufPtr;
									DWORD Section;
									unsigned NumSpecs[3];
									DWORD Sizes[3];
									DWORD Origins[3];
									DWORD Ends[3];
									EAData EAD;
									char * NewFile = NULL;
									DWORD NameSize;
									DWORD ThisSpecSize;
									char ** Specs[3]={NULL,NULL,NULL};
									char * NameBuffers[3]={NULL,NULL,NULL};
									void ** Vals = NULL;
									char * OldNameBuffer;
									void * ValBuffer;
									unsigned ValBufferSize;
									unsigned TheValSize;
									int WildLevel1;
									int WildLevel2;
									int WildLevel3;
									DWORD ValueSizeBackout = 0;
									DWORD SpecNum;
									DWORD j;

									eaUtilsInit();
									Ptr = EAEH->HeaderBlock;
									Section = i = 0;
									while ((i < EAEH->qwSize) && (Section < 3))
									{
										// first pass to count things
										Sizes[Section] = *((DWORD *)(Ptr+i));
										Origins[Section] = i;
										Ends[Section] = i + Sizes[Section] + 4;
										if (Ends[Section] > EAEH->qwSize)
										{
											Ends[Section] = (DWORD) EAEH->qwSize;
										}
										NumSpecs[Section]=0;
										i += 4;
										while (i < Ends[Section])
										{
											NumSpecs[Section]++;
											i += 1 + Ptr[i];
											if (Section == 2)
											{
												ValueSizeBackout += 2 + *((UINT16 *)(Ptr + i));
												i += 2 + *((UINT16 *)(Ptr + i));
											}
										}
										i = Ends[Section];
										Section++;
									}
									// allocate space to remember things
									for (i=0; (i<3) && (Code == EXAPATCH_SUCCESS) ; i++ )
									{
										Code = ExaMemAlloc( NULL, NumSpecs[i]*sizeof(void *),(void **) &Specs[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
										if (Code == EXAPATCH_SUCCESS)
										{
											memset( Specs[i], 0, NumSpecs[i]*sizeof(void *) );
											Code = ExaMemAlloc( NULL, (i==2)?(Sizes[i]-ValueSizeBackout):Sizes[i], (void **) &NameBuffers[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
										}
									}
									Code = ExaMemAlloc( NULL, NumSpecs[2]*sizeof(void *), (void **) &Vals ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
									if (Code == EXAPATCH_SUCCESS)
									{
										for (Section = 0; Section < 3 ; Section++ )
										{
											// second pass to remember things
											i = Origins[Section]+4;
											SpecNum = 0;
											BufPtr = NameBuffers[Section];
											while (i < Ends[Section])
											{
												Specs[Section][SpecNum] = BufPtr;
												ThisSpecSize = Ptr[i];
												memcpy( BufPtr, Ptr+i+1, ThisSpecSize);
												BufPtr[ThisSpecSize] = '\0';
												BufPtr += ThisSpecSize+1;
												i += 1+ThisSpecSize;
												if (Section == 2)
												{
													Vals[SpecNum] = (void *)(Ptr+i+2);
													i += 2+*((UINT16 *)(Ptr+i));
												}
												SpecNum++;
											}
										}
									}
									NameSize = (DWORD)PSwcstombs( NULL, NewFileNameArray[0], 0 );
									if (NameSize != -1)
									{
										Code = ExaMemAlloc(NULL, NameSize + 1, (void **) &NewFile);
										if (Code == EXAPATCH_SUCCESS)
										{
											if (-1 == PSwcstombs(NewFile, NewFileNameArray[0], NameSize+1))
											{
												ExaMemFree( NULL, NewFile );
												Code = EXAPATCH_CHARSET_ERROR;
											}
										}
									}
									else
									{
										Code = EXAPATCH_CHARSET_ERROR;
									}
									if (Code == EXAPATCH_SUCCESS)
									{
										if (eaGetAll(NewFile, &EAD))
										{
											Code = EXAPATCH_EA_ERROR;
										}
										else
										{
											for (i=0; (i<EAD.EACount) && (0U != dwEAOK) && (Code == EXAPATCH_SUCCESS) ; i++ )
											{
												eaLookupName( &EAD, i, &OldNameBuffer );
												if (*OldNameBuffer)
												{
													WildLevel1 = WildMatchList( NumSpecs[0], Specs[0], OldNameBuffer );
													WildLevel2 = WildMatchList( NumSpecs[1], Specs[1], OldNameBuffer );
													WildLevel3 = WildMatchList( NumSpecs[2], Specs[2], OldNameBuffer );
													if ((WildLevel1 > WildLevel2) && (WildLevel1 > WildLevel3))
													{
														// should have been deleted, but it wasn't
														dwEAOK = 0U;
													}
													else if (WildLevel3 > WildLevel2)
													{
														// should have been added - see if it was
														eaLookupValueSize( &EAD, i, &ValBufferSize );
														Code = ExaMemAlloc( NULL, (DWORD) ValBufferSize, (void **) &ValBuffer );
														if (Code == EXAPATCH_SUCCESS)
														{
															if (eaCopyValue(&EAD, i, ValBuffer, (unsigned) ValBufferSize))
															{
																Code = EXAPATCH_EA_ERROR;
															}
															else
															{
																for (j=0; (j<NumSpecs[2]) && (0U != dwEAOK) ; j++ )
																{
																	if (WildLevel3 == WildMatch(OldNameBuffer, Specs[2][j]))
																	{
																		dwEAOK = 0U;
																		TheValSize = (unsigned) (*((UINT16 *)(((char *)Vals[j])-2)));
																		if (ValBufferSize == TheValSize)
																		{
																			if (0 == memcmp(ValBuffer, Vals[j],ValBufferSize))
																			{
																				dwEAOK = 1U;
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
									for (i=0; i<3 ; i++ )
									{
										if (Specs[i])
										{
											ExaMemFree( NULL, Specs[i] );
										}
										if (NameBuffers[i])
										{
											ExaMemFree( NULL, NameBuffers[i] );
										}
									}
									if (Vals)
									{
										ExaMemFree( NULL, Vals );
									}
									if (NewFile)
									{
										ExaMemFree( NULL, NewFile );
										eaGetAllCleanup( &EAD );
									}
								}
							}

							if (0U != dwEAOK)
# endif 
							{
								Code = EXAPATCH_ALREADY_PATCHED;
								goto exit;
							}
						}
					}
				}
			}

			/* then, see if the Old files exist */
			InvalidFlag = FALSE;
			if (EntryPtr->Comm.Files.dwNumOldFiles)
			{
				for (i=0, Count=0; i<EntryPtr->Comm.Files.dwNumOldFiles ; i++ )
				{
          EntryPtr->Comm.Files.lpOldAttribArray[i].Flags = AttribFlags;
					Code = ExaPatchFileSearch(
						OldFileNameArray + i,
						EntryPtr->Comm.Files.lpOldPathArray[i],
            SearchPath,
# ifdef EXAPATCH_ARCHIVE_SUPPORT
						SearchFlags | EXP_SEARCH_CHECK_OLD_DISC | 
						((dwInArchive)?0:(EXP_SEARCH_IGNORE_DUPS | EXP_SEARCH_ADD_TO_DUP)),
# else
						SearchFlags | EXP_SEARCH_CHECK_OLD_DISC | EXP_SEARCH_IGNORE_DUPS | EXP_SEARCH_ADD_TO_DUP,
# endif
						EntryPtr->Comm.Files.lpOldAttribArray + i,
						EntryPtr->Comm.Files.lpOldCksumArray + i,
            EntryPtr->OldRegionListArray?EntryPtr->OldRegionListArray[i]:NULL,
            &HeaderPtr->DupList,
						EntryPtr->Comm.Files.lpDiscArray,
						PATH_DELIMITER,
						PATH_SEPARATOR,
						NULL, 0, NULL, 0 );
# ifdef EXAPATCH_LINK_SUPPORT
					if ((Code == EXAPATCH_SUCCESS) && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_LINKUNIQUE))
					{
						UniqueFileID UFI;
						int UCode;

						if (0 == ExaGetFileIDW( &UFI, OldFileNameArray[i]))
						{
							UCode = ExaUnique( HeaderPtr->hUnique, &UFI );
							if (UCode > 0)
							{
								// let it go if an error occurred
								Code = EXAPATCH_ALREADY_PATCHED;
							}
						}
					}
# endif
					if (Code == EXAPATCH_SUCCESS)
					{
						Count++;
					}
					else if (Code != EXAPATCH_FILE_NOT_FOUND)
					{
						goto exit;
					}
					else
					{
						if (OldFileNameArray[i])
						{
							InvalidFlag = TRUE;
						}
					}
				}
				if (Count != EntryPtr->Comm.Files.dwNumOldFiles)
				{
					if ((Count == 0) && (EntryPtr->Comm.Files.dwNumNewFiles == 0))
					{
						//TLO -- no old files found, and it's a DELETE, return EXAPATCH_ALREADY_PATCHED
						Code = EXAPATCH_ALREADY_PATCHED;
						goto exit;
					}
					if ((EntryPtr->Comm.Files.dwNumNewFiles) && Count == 0 && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_ALLOWDUP) )
					{
						/* if we are allowing duplicates AND */
						/* if ALL the old files don't exist and ALL the */
						/*	new files do exist, then return EXAPATCH_ALREADY_PATCHED */

						for (i=0, Count=0; i<EntryPtr->Comm.Files.dwNumNewFiles ; i++ )
						{
              EntryPtr->Comm.Files.lpNewAttribArray[i].Flags = AttribFlags;
							Code = ExaPatchFileSearch(
								NewFileNameArray + i,
								EntryPtr->Comm.Files.lpNewPathArray[i],
                SearchPath,
								SearchFlags | EXP_SEARCH_CHECK_NEW_DISC,
								EntryPtr->Comm.Files.lpNewAttribArray + i,
								EntryPtr->Comm.Files.lpNewCksumArray + i,
                EntryPtr->NewRegionListArray?EntryPtr->NewRegionListArray[i]:NULL,
                NULL,
								EntryPtr->Comm.Files.lpDiscArray,
								PATH_DELIMITER,
								PATH_SEPARATOR,
								NULL, 0, NULL, 0 );
							if (Code == EXAPATCH_SUCCESS)
							{
								Count++;
							}
							else if (Code != EXAPATCH_FILE_NOT_FOUND)
							{
								goto exit;
							}
						}
						if (Count == EntryPtr->Comm.Files.dwNumNewFiles)
						{
							Code = EXAPATCH_ALREADY_PATCHED;
							goto exit;
						}
            else
            {
              Code = InvalidFlag?EXAPATCH_INVALID_FILE_FOUND:EXAPATCH_FILE_NOT_FOUND;
              goto exit;
            }
					}
          else
          {
            Code = InvalidFlag?EXAPATCH_INVALID_FILE_FOUND:EXAPATCH_FILE_NOT_FOUND;
            goto exit;
          }
				}
			}
# ifdef EXAPATCH_VERIFYONLY_SUPPORT
			if (EXP_PATCH_APPLY_VERIFYONLY & HeaderPtr->ApplyFlags)
				goto exit;
# endif
			Code = SetFileLoc( OldFileNameArray, IntFileNameArray, NewFileNameArray,
					SizeArray, HeaderPtr, EntryPtr, EntryPtr, EntryPtr->Comm.Files.dwNumOldFiles,
					EntryPtr->Comm.Files.dwNumNewFiles, &CNCFlag, dwNumManipEntries, 
					EntryPtr->wModFlags, EntryPtr->Comm.dwFileFlags, EntryPtr->Comm.dwAttribRetain,
					&NumOldMoved, &NumIntMoved, HintSubDir, Handle );
			if (Code) goto exit;
			/* open streams */
			if (CNCFlag 
				|| (EntryPtr->Comm.Files.dwNumNewFiles == 0) 
				|| ((EntryPtr->Comm.Files.dwNumOldFiles !=0)&&(0 ==(EntryPtr->wModFlags & EXP_STATUS_CONTENTS))))
			{
				*NewStreamPtr = *OldStreamPtr = NULL;
			}
			else
			{
				for (i=0; i<EntryPtr->Comm.Files.dwNumNewFiles ; i++ )
				{
					SizeArray[i] = EntryPtr->Comm.Files.lpNewAttribArray[i].Size;
				}
				if (EntryPtr->Comm.Files.dwNumOldFiles)
				{
					Code = ExaPatchApplyOpenFileArrayAsStreamW(
						OldStreamPtr,
						OldFileNameArray,
						EntryPtr->Comm.Files.dwNumOldFiles,
						EXP_OPEN_READONLY,
						NULL );
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					if (Code 
						// && (0U != (EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK))
						&& (EntryPtr->Comm.Files.dwNumOldFiles == 1)
						&& (EntryPtr->Comm.Files.lpOldAttribArray[0].Size == 0))
					{
						Code = EXAPATCH_SUCCESS;
						*OldStreamPtr = NULL;
					}
# endif
					if (Code) goto exit;
				}
				else
				{
					*OldStreamPtr = NULL;
				}
				if (EntryPtr->Comm.Files.dwNumNewFiles)
				{
					Code = ExaPatchApplyOpenFileArrayAsStreamW(
						NewStreamPtr,
						IntFileNameArray,
						EntryPtr->Comm.Files.dwNumNewFiles,
						0,
						SizeArray );
					if (Code) goto exit;
				}
				else
				{
					*NewStreamPtr = NULL;
				}
			}
			break;
		default:
			/* EOF and TEMP are handled elsewhere */
			return(EXAPATCH_INTERNAL_ERROR);
	}

exit:

	if (SearchPath)
	  ExaMemFree( NULL, SearchPath );
	if (HintSubDir)
		ExaMemFree( NULL, HintSubDir );
	if (SizeArray)
		ExaMemFree( NULL, SizeArray );
	if (HistoryEntryArray)
	{
		for (i=0; i<EntryPtr->Comm.dwNumHistoryVersions ; i++ )
		{
			if (HistoryEntryArray[i])
			{
				ExaPatchApplyFreeParsedEntry( HistoryEntryArray[i] );
				ExaMemFree( NULL, HistoryEntryArray[i] );
				HistoryEntryArray[i] = NULL;
			}
		}
		ExaMemFree( NULL, HistoryEntryArray );
		HistoryEntryArray = NULL;
	}
	if (OldFileNameArray)
	{
		DWORD dwNumOld;

		dwNumOld = MaxOldFiles?MaxOldFiles:EntryPtr->Comm.Files.dwNumOldFiles;
		for (i=0; i<dwNumOld ; i++ )
		{
			if (OldFileNameArray[i] && i >= NumOldMoved)
				ExaMemFree( NULL, OldFileNameArray[i] );
		}
		ExaMemFree( NULL, OldFileNameArray );
	}
	if (IntFileNameArray)
	{
		for (i=0; i<EntryPtr->Comm.Files.dwNumNewFiles ; i++ )
		{
			if (IntFileNameArray[i] && i >= NumIntMoved)
			{
				if (EXAPATCH_SUCCESS == ExaExists( IntFileNameArray[i]) )
					ExaDelete( IntFileNameArray[i] );
				ExaMemFree( NULL, IntFileNameArray[i] );
			}
		}
		ExaMemFree( NULL, IntFileNameArray );
	}
	if (NewFileNameArray)
	{
		for (i=0; i<EntryPtr->Comm.Files.dwNumNewFiles ; i++ )
		{
			if (NewFileNameArray[i] && i >= NumIntMoved)
				ExaMemFree( NULL, NewFileNameArray[i] );
		}
		ExaMemFree( NULL, NewFileNameArray );
	}
	// reposition file (just in case)
	SeekExaStream( FileToApply, 
				qwRestorePos, 
				EXAPATCH_SEEK_BEGIN,
				NULL );
	return(Code);
	
}
# endif /* ATTOPATCH */

static int ExaPatchEntryDoWork(
		ExaPatchStream * FileToApply, /* assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
# ifdef ATTOPATCH
		const ExaPatchApplyFileEntryInfo * EntryPtr,
# else
		ExaPatchApplyFileEntryInfo * EntryPtr,
# endif
		ExaPatchStream * OldStream,
		ExaPatchStream * NewStream,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		QWORD BufferBreak,
		DWORD dwCurHistory,
		DWORD dwTotHistory,
		PLATWORD Handle
	)
{
	int Code = EXAPATCH_SUCCESS;
	ExaPatchApplyState TheState;
# ifndef ATTOPATCH
	DWORD i;
	ExaPatchApplyFileManipulation * pTheManip;
# endif
	DWORD dwProgressOrigin = (dwCurHistory * 0x8000U)/dwTotHistory;
# ifdef EXAPATCH_BYTE_SUPPORT
 	ExaPatchStream * pReadStream;
	wchar_t * ReadFileFullPath;
	wchar_t * ReadFileBaseDir;
	DWORD ReadFileSpecialFlag;
# endif

	(void)BufferBreak; /* MISRA C 2012 [2.7]: BufferBreak is not needed with some sets of compile-time switches */
	/*This routine must handle Add/Delete/Change/History/EOF */
	/*Temp is done elsewhere */
	/*NOTE: EOF must set qwNextEntryNum to 0xffffffffffffffff */
	switch ( EntryPtr->wPatchType & 0xfU )
	{
		default:
			return (EXAPATCH_CORRUPT_PATCH_FILE); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			break; /* MISRA C 2012 [2.1]: unreachable code (alternative is MISRA switch syntax departure) */
		case EXAPATCH_HISTORY_TYPE:
# ifdef EXAPATCH_HISTORY_SUPPORT
			/*
			NOTE: OldStream is original old file(s), NewStream is final int file(s)

			procedure: 
			DoingFirstVersion = TRUE
			PrevTemp = NULL
			CurTemp = NULL
			if (OLD stream exists)
				set "OLD" to OLD
			if (int stream exists)
				if doing last version
					set "NEW" to INT
				else
					build temp file (CurTemp)
					open temp file as "NEW"
			loop through remaining versions 
				[perform current ADC flow, *including* search for NEW files] use "OLD" and "NEW" streams
				if (SUCCESS)
					DoingFirstVersion = FALSE
					close "OLD" and "NEW" streams
					if (PrevTemp)
						delete PrevTemp; PrevTemp = NULL
					PrevTemp = CurTemp; CurTemp = NULL
					if (! last version)
						read next entry header
						if (PrevTemp)
							open PrevTemp as "OLD" stream
						[are there conditions on this next block???]
						if (next-to-last version)
							open INT as "NEW" stream
						else
							build temp file (CurTemp)
							open temp file as "NEW" stream
				else
					quit version loop - delete CurTemp/PrevTemp [other cleanup will be done later]
			*/
			{
				BOOL DoingFirstVersion = TRUE;
				BOOL StreamDupFlag = FALSE;
				ExaPatchStream * FromStream;
				ExaPatchStream * ToStream;
				wchar_t * PrevTemp = NULL;
				wchar_t * CurTemp = NULL;
				DWORD dwThisVersion;
				ExaPatchApplyFileEntryInfo ThisEntry;
				DWORD dwActualVersion;
				DWORD dwNumActualVersions;
				QWORD qwSize;
				QWORD qwEndPos = EntryPtr->qwEntryPos + EntryPtr->qwEntrySize;

				if (EntryPtr->wModFlags & EXP_STATUS_RESERVED)
				{
					// delete optimization - do nothing but mark progress and skip
					SeekExaStream( FileToApply, qwEndPos, EXAPATCH_SEEK_BEGIN, NULL );
					Code = MarkProgress( HeaderPtr, EntryPtr, ProgressCallBack, CallbackHandle, dwProgressOrigin, 0x8000U/dwTotHistory );
					goto exit;
				}
				// skip unused versions
				{
					DWORD i;
					for (i=0; i<EntryPtr->dwHistoryCurrentVersion; i++)
					{
						Code = ExaPatchEntryParseWork( FileToApply, HeaderPtr, &ThisEntry, TRUE );
						if (Code) goto HistoryExit;
						Code = SeekExaStream( FileToApply, ThisEntry.qwEntrySize,
							EXAPATCH_SEEK_CUR, NULL );
						if (Code) goto HistoryExit;
						Code = ExaPatchApplyFreeParsedEntry( &ThisEntry );
						if (Code) goto HistoryExit;
					}
				}
				// start first version
				dwNumActualVersions = EntryPtr->Comm.dwNumHistoryVersions - EntryPtr->dwHistoryCurrentVersion;
				Code = ExaPatchEntryParseWork( FileToApply, HeaderPtr, &ThisEntry, TRUE );
				if (Code) goto HistoryExit;
				FromStream = OldStream;
				if (dwNumActualVersions > 1)
				{
					Code = ExaEZTemp( HeaderPtr->ActualApplyDir, &CurTemp );
					if (Code) goto HistoryExit;
					qwSize = 0;
					for (i=0; i<ThisEntry.Comm.Files.dwNumNewFiles ; i++ )
					{
						qwSize += ThisEntry.Comm.Files.lpNewAttribArray[i].Size;
					}
					Code = ExaPatchApplyOpenFileArrayAsStreamW( &ToStream,
							&CurTemp, 1, 0, &qwSize );
				}
				else
				{
					ToStream = NewStream;
				}
				for (dwThisVersion = EntryPtr->dwHistoryCurrentVersion, dwActualVersion = 0;
					(!Code) && (dwActualVersion < dwNumActualVersions) ; dwThisVersion++, dwActualVersion++ )
				{
					StreamDupFlag = FALSE;
					if (((ThisEntry.Comm.Files.dwNumOldFiles == 0)&&(0U == (ThisEntry.Comm.dwFileFlags & EXP_FILE_UPDATE_LINK)))
						|| (ThisEntry.wModFlags & EXP_STATUS_CONTENTS) 
						|| (dwActualVersion == (dwNumActualVersions - 1)))
					{
						Code = ExaPatchEntryDoWork( FileToApply, HeaderPtr, &ThisEntry,
								FromStream, ToStream, ProgressCallBack, CallbackHandle,
								BufferBreak, dwActualVersion, dwNumActualVersions, Handle );
					}
					else
					{
						// we need to copy FromStream to ToStream
						if (dwActualVersion == 0)
						{
							// real stream copy is needed
							QWORD qwPos;
							DWORD dwThisSize;
							DWORD dwActualIO;
							QWORD qwFileSize;
							char *Buffer;

							qwFileSize = FromStream->Size;
							qwPos = 0;
							SeekExaStream( FromStream, 0, EXAPATCH_SEEK_BEGIN, NULL);
							SeekExaStream( ToStream, 0, EXAPATCH_SEEK_BEGIN, NULL );
							Code = ExaMemAlloc( NULL, 0x100000, (void **) &Buffer ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
							while ((Code == EXAPATCH_SUCCESS) && (qwPos < qwFileSize))
							{
								dwThisSize = 0x100000;
								if (dwThisSize > (qwFileSize - qwPos))
								{
									dwThisSize = (DWORD) (qwFileSize - qwPos);
								}
								if (EXAPATCH_SUCCESS == (Code = ReadExaStream( FromStream, Buffer, dwThisSize, &dwActualIO )))
								{
									if (EXAPATCH_SUCCESS == (Code = WriteExaStream( ToStream, Buffer, dwThisSize, &dwActualIO )))
									{
										qwPos += dwThisSize;
									}
								}
							}
						}
						else
						{
							// just change pointers around
							StreamDupFlag = TRUE;
							CloseExaStream( ToStream );
							ExaDelete( CurTemp );
							ExaMemFree( NULL, CurTemp );
							CurTemp = PrevTemp;
							PrevTemp = NULL;
							ToStream = FromStream;
						}
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						EntryPtr->dwHistoryCurrentVersion++;
						if ((!DoingFirstVersion) && (!StreamDupFlag))
						{
							CloseExaStream(FromStream);
						}
						if (dwActualVersion != (dwNumActualVersions - 1))
						{
							CloseExaStream(ToStream);
						}
						if (PrevTemp)
						{
							ExaDelete( PrevTemp );
							ExaMemFree( NULL, PrevTemp );
							PrevTemp = NULL;
						}
						PrevTemp = CurTemp;
						CurTemp = NULL;
						if (dwActualVersion != (dwNumActualVersions - 1))
						{
							Code = ExaPatchApplyFreeParsedEntry( &ThisEntry );
							if (Code) goto HistoryExit;
							Code = ExaPatchEntryParseWork( FileToApply, 
								HeaderPtr, 
								&ThisEntry, 
								TRUE );
							if (PrevTemp)
							{
								Code = ExaPatchApplyOpenFileArrayAsStreamW( &FromStream,
										&PrevTemp, 1, EXP_OPEN_READONLY, NULL );
							}
							if (Code) goto HistoryExit;
							if ((dwActualVersion + 1) == (dwNumActualVersions - 1))
							{
								ToStream = NewStream;
							}
							else
							{
								Code = ExaEZTemp( HeaderPtr->ActualApplyDir, &CurTemp );
								if (Code) goto HistoryExit;
								qwSize = 0;
								for (i=0; i<ThisEntry.Comm.Files.dwNumNewFiles ; i++ )
								{
									qwSize += ThisEntry.Comm.Files.lpNewAttribArray[i].Size;
								}
								Code = ExaPatchApplyOpenFileArrayAsStreamW( &ToStream,
										&CurTemp, 1, 0, &qwSize );

							}
						}
					}
				}
HistoryExit:
				if (Code)
				{
					// skip over rest of patch
					SeekExaStream( FileToApply, qwEndPos, EXAPATCH_SEEK_BEGIN, NULL );
				}
				ExaPatchApplyFreeParsedEntry( &ThisEntry );
				if (CurTemp)
				{
					ExaDelete( CurTemp );
					ExaMemFree( NULL, CurTemp );
					CurTemp = NULL;
				}
				if (PrevTemp)
				{
					ExaDelete( CurTemp );
					ExaMemFree( NULL, PrevTemp );
					PrevTemp = NULL;
				}
				if (Code) goto exit;

			}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
			return(EXAPATCH_NOT_SUPPORTED); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
#  endif
# endif
			break; /* MISRA C 2012 [2.1]: unreachable code (alternative is MISRA switch syntax departure) */

		case EXAPATCH_EOF_TYPE:
# ifdef QWORD_IS_DWORD
			HeaderPtr->qwNextEntryNum = 0xffffffffU;
# else
			HeaderPtr->qwNextEntryNum = 0xffffffffffffffffU;
# endif
			return(EXAPATCH_SUCCESS); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			break; /* MISRA C 2012 [2.1]: unreachable code (alternative is MISRA switch syntax departure) */

		case EXAPATCH_ADC_TYPE:
			if (((EntryPtr->Comm.Files.dwNumOldFiles == 0U)&&(0U == (EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK))) 
				|| (0U != (EntryPtr->wModFlags & EXP_STATUS_CONTENTS)))
			{
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
				/* there is an actual patch to apply */
				/* (not a Delete or a Change/No Contents) */
				ForceExaStreamBuffering( NewStream, 0 );
				if (OldStream && (OldStream->Size >= BufferBreak))
				{
					// file too big - use two *small* buffers
					ForceExaStreamBuffering( OldStream, 0 );
				}
				else
				{
					// read it all in to memory - one *big* buffer
					ForceExaStreamBuffering( OldStream, 0xffffffffU );
				}
# endif
# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
				if ((0U != Handle) && (0U != ((ExaPatchApplyChannel *)Handle)->dwCBSize)) /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
				{
					const ExaPatchApplyChannel * pChan = (ExaPatchApplyChannel *)Handle; /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
					(void)ExaStreamSupplyBuffer( NewStream, pChan->pChannelBuffer, pChan->dwCBSize );
				}
# endif
# ifdef EXAPATCH_HISTORY_SUPPORT
				if ((dwCurHistory + 1U) == dwTotHistory)
# endif
				{
# ifdef EXAPATCH_ADDFILE_SUPPORT
					if (EntryPtr->Comm.Files.dwNumOldFiles == 0)
					{
						HeaderPtr->Stats[EXP_FILES_ADDED]++;
					}
					else
# endif
					{
						HeaderPtr->Stats[EXP_FILES_MODIFIED]++;
					}
				}
				TheState.PatchFile = FileToApply;
				TheState.OldFile = OldStream;
				TheState.NewFile = NewStream;
				TheState.PatchSize = EntryPtr->qwEntrySize;
				/*TheState.CBPtr = (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE)?ProgressCallBack:NULL;*/
				TheState.CBPtr = ProgressCallBack;
				TheState.CBHandle = CallbackHandle;
				TheState.LocalStart = dwProgressOrigin;
				TheState.LocalDelta = 0x8000U/dwTotHistory;
				TheState.GlobalStart = (DWORD)((EntryPtr->qwEntryNum << 15) / HeaderPtr->Comm.qwNumEntries);
        TheState.GlobalDelta = (DWORD)(0x8000U / HeaderPtr->Comm.qwNumEntries);
        TheState.HeaderPtr = HeaderPtr;
				if (NULL != OldStream)
				{
					TheState.OldFileSize = OldStream->Size;
				}
				else
				{
					TheState.OldFileSize = 0;
				}
				Code = ExaPatchApplyWorkInit( &TheState );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# ifdef EXAPATCH_PASSWORD_SUPPORT
				if (HeaderPtr->Comm.lpPassword)
				{
					Code = ExaPatchApplyUsePW( &TheState,
						&HeaderPtr->Comm.PWHash,
						&HeaderPtr->Comm.PWHash2 );
					if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
				if (NULL != HeaderPtr->Comm.lpPassword)
				{
					Code = EXAPATCH_NOT_SUPPORTED;
					goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				}
#  endif
# endif
				Code = ExaPatchApplyWorkRoutine( &TheState );
# ifdef EXAPATCH_BYTE_SUPPORT
        /* do "Read" type byte-handling */
				for (i=0; (Code == EXAPATCH_SUCCESS) && (i < EntryPtr->Comm.dwNumBH) ; i++)
				{
					ReadFileSpecialFlag = FALSE;
					pReadStream = NULL;
					if (EntryPtr->Comm.BHPtr[i].dwType == EXP_BYTE_READ)
					{
						/* The file location semantics here are slightly
						different from RTPatch.  RTPatch locates all 'read' files
						relative to the directory containing the patch file.  
						However, we don't know that directory since we are 
						passed a handle to the patch file, rather than a name.
						So, we will use the apply directory or system directory
						as the base for location of 'read' files. 
						*/
# ifdef _WIN32
						if ((EntryPtr->Comm.Files.dwNumNewFiles == 1) 
							&& (0 == _wcsicmp(EntryPtr->Comm.BHPtr[i].lpFileName,ExaBaseName(EntryPtr->Comm.Files.lpNewPathArray[0],PATH_DELIMITER))))
# else
						if ((EntryPtr->Comm.Files.dwNumNewFiles == 1) 
							&& (0 == PSwcscmp(EntryPtr->Comm.BHPtr[i].lpFileName,ExaBaseName(EntryPtr->Comm.Files.lpNewPathArray[0],PATH_DELIMITER))))
# endif						
						{
							/* Special case - reading from New file */
							pReadStream = NewStream;
							ReadFileSpecialFlag = TRUE;
						}
						else
						{
# ifdef EXAPATCH_SYSTEM_SUPPORT
							if ((EntryPtr->Comm.nSystemIndex == -1) || (EntryPtr->Comm.nSystemIndex >= (SQWORD)HeaderPtr->Comm.Systems->dwNumSystems))
# endif
							{
								ReadFileBaseDir = HeaderPtr->ActualApplyDir;
							}
# ifdef EXAPATCH_SYSTEM_SUPPORT
							else
							{
								ReadFileBaseDir = HeaderPtr->SystemDirs[i];
							}
# endif
							if (EXAPATCH_SUCCESS == (Code = ExaDirMerge( ReadFileBaseDir, EntryPtr->Comm.BHPtr[i].lpFileName, &ReadFileFullPath, PATH_DELIMITER )))
							{
								Code = ExaPatchApplyOpenFileArrayAsStreamW( &pReadStream, &ReadFileFullPath, 1, EXP_OPEN_READONLY, NULL );
								ExaMemFree( NULL, ReadFileFullPath );
							}
						}
						if (Code == EXAPATCH_SUCCESS)
						{
							/* We have opened the 'read' file */
							char * Buffer;
							DWORD BufLength;
							DWORD ThisRead;
							DWORD ActualRead;
							QWORD ReadLeft;
							QWORD OffsetIntoRead = 0;

							ReadLeft = EntryPtr->Comm.BHPtr[i].qwLength;
							BufLength = 0x100000;
							if (ReadLeft < (QWORD) BufLength)
							{
								BufLength = (DWORD) ReadLeft;
							}
							Code = ExaMemAlloc( NULL, BufLength, (void **) &Buffer ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
							if (EXAPATCH_SUCCESS == Code)
							{
								while (ReadLeft && (Code == EXAPATCH_SUCCESS))
								{
									ThisRead = BufLength;
									if (ReadLeft < (QWORD) ThisRead)
									{
										ThisRead = (DWORD) ReadLeft;
									}
									/* NOTE: this repeated-seek stuff is necessary for the
									case where the Read and New streams are identical - SeekExaStream
									is optimized for this case anyway... */
									if (EXAPATCH_SUCCESS == (Code = SeekExaStream( pReadStream, 
											EntryPtr->Comm.BHPtr[i].qwOldPos + OffsetIntoRead, 
											EXAPATCH_SEEK_BEGIN, NULL)))
									{
										if (EXAPATCH_SUCCESS == (Code = ReadExaStream( pReadStream,
												Buffer, ThisRead, &ActualRead)))
										{
											if (EXAPATCH_SUCCESS == (Code = SeekExaStream( NewStream,
													EntryPtr->Comm.BHPtr[i].qwNewPos + OffsetIntoRead,
													EXAPATCH_SEEK_BEGIN, NULL)))
											{
												if (EXAPATCH_SUCCESS == (Code = WriteExaStream( NewStream,
														Buffer, ThisRead, &ActualRead)))
												{
													ReadLeft -= (QWORD) ThisRead;
													OffsetIntoRead += (QWORD) ThisRead;
												}
											}
										}
									}
								}
								ExaMemFree( NULL, Buffer );
							}
						}
						if (pReadStream && !ReadFileSpecialFlag)
						{
							ExaPatchApplyCloseStream( pReadStream );
						}
					}
				}
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
				/* verify Int file contents and set timestamps/attribs */
# ifndef ATTOPATCH
# ifdef EXAPATCH_HISTORY_SUPPORT
				if ((dwCurHistory + 1) == dwTotHistory)
# endif
				{
					// single patch OR final stage of HISTORY
					pTheManip = HeaderPtr->LocalManipList;
					for (i=0; (Code == EXAPATCH_SUCCESS) && (pTheManip) && (i<EntryPtr->Comm.Files.dwNumNewFiles) ; i++ )
					{

						if (NewStream)
						{
			        CloseExaStream( NewStream );
							NewStream = NULL;
						}
						EntryPtr->Comm.Files.lpNewAttribArray[i].Flags = EXP_ATTRIB_ATTRIBUTE | EXP_ATTRIB_CHANGED_DATE | EXP_ATTRIB_CREATE_DATE;
						if (EXAPATCH_SUCCESS == (Code = ExaSetFileAttrib( pTheManip->IntFile,
								&EntryPtr->Comm.Files.lpNewAttribArray[i],NULL, (pTheManip->FileManipFlags & EXP_PATCH_MANIP_TREAT_AS_FILE)?TRUE:FALSE)))
						{
							Code = ExaPatchApplyVerifyFile( pTheManip->IntFile, 
									&EntryPtr->Comm.Files,
									i,
									TRUE,
									EntryPtr->Comm.dwNumBH,
									EntryPtr->Comm.BHPtr,
									EXP_APPLY_CHECK_CHECKSUM | EXP_APPLY_CHECK_SIZE
# ifdef EXAPATCH_UPDATELINK_SUPPORT
										| ((EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK)?EXP_APPLY_CHECK_PROCESSLINKS:0)
# endif
									 );
							if (Code == EXAPATCH_FILE_NOT_FOUND) 
							{
								Code = EXAPATCH_POST_PATCH_VERIFY_FAILED;
							}
						}
						pTheManip = pTheManip->NextManip;
					}
				}
# endif /* ATTOPATCH */
			}
			else
			{
				if (0U != (EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK))
				{
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					/* make sure there is a single file */
					if (1 == EntryPtr->Comm.Files.dwNumNewFiles)
					{
						Code = EXAPATCH_SUCCESS;
					}
					else
					{
						Code = EXAPATCH_INVALID_PATCH_FILE;
					}
# else
					Code = EXAPATCH_NOT_SUPPORTED;
# endif
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = MarkProgress(HeaderPtr, EntryPtr, ProgressCallBack, CallbackHandle, dwProgressOrigin, 0x8000U/dwTotHistory );
					}
				}
# ifndef ATTOPATCH
				else if (EntryPtr->wModFlags & (EXP_STATUS_TIMESTAMP | EXP_STATUS_ATTRIB | EXP_STATUS_EA | EXP_STATUS_ADS
# ifdef EXAPATCH_OWNERGROUP_SUPPORT
					| EXP_STATUS_RESERVED2
# endif
					))
				{
					// NOTE: for History patches, this should only occur
					// on the LAST patch
					// copy OLD files to INT files and set attrib/ts
					pTheManip=HeaderPtr->LocalManipList;
					for (i=0; (Code == EXAPATCH_SUCCESS)&&(i<EntryPtr->Comm.Files.dwNumNewFiles) ; i++)
					{
						Code = ExaDelete( pTheManip->IntFile );
						if (Code == EXAPATCH_SUCCESS)
						{
							Code = ExaCopyFile(pTheManip->OldFile, pTheManip->IntFile, FALSE, FALSE, FALSE );
							if (Code == EXAPATCH_SUCCESS)
							{
								EntryPtr->Comm.Files.lpNewAttribArray[i].Flags = EXP_ATTRIB_ATTRIBUTE | EXP_ATTRIB_CHANGED_DATE | EXP_ATTRIB_CREATE_DATE;
								Code = ExaSetFileAttrib( pTheManip->IntFile,
									&EntryPtr->Comm.Files.lpNewAttribArray[i],NULL, FALSE);
							}
						}
						pTheManip = pTheManip->NextManip;
					}
					if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				}
# endif /* ATTOPATCH */
# ifdef EXAPATCH_HISTORY_SUPPORT
				if ((dwCurHistory + 1) == dwTotHistory)
# endif				
				{
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					if (0U != (EntryPtr->Comm.dwFileFlags & EXP_FILE_UPDATE_LINK))
					{
						HeaderPtr->Stats[EXP_FILES_MODIFIED]++;
					}
					else
# endif
					{
						if (0U != EntryPtr->Comm.Files.dwNumNewFiles)
						{
							HeaderPtr->Stats[EXP_FILES_RENAMED]++;
						}
						else
						{
							HeaderPtr->Stats[EXP_FILES_DELETED]++;
						}
					}
				}
			}
			break;

		case EXAPATCH_TEMP_TYPE:
			/* should be handled elsewhere */
			return(EXAPATCH_INTERNAL_ERROR); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			break; /* MISRA C 2012 [2.1]: unreachable code (alternative is MISRA switch syntax departure) */ 
	}

exit:
	if (NULL != OldStream)
	{
		(void)CloseExaStream( OldStream );
	}
	if (NULL != NewStream)
	{
		(void)CloseExaStream( NewStream );
	}
	return(Code);
}

# ifndef ATTOPATCH
# ifdef EXAPATCH_DETAILED_APPLY_API_SUPPORT
int EXP_DLLIMPORT
	ExaPatchApplyStartFile( 
		PLATWORD Handle,
		ExaPatchStream * FileToParse,	/* assumed to be positioned at header */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		DWORD ApplyFlags,
		DWORD ApplyFlagsToForce,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		)
{
	int Code;

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	Code = ExaPatchStartWork(
		FileToParse,
		HeaderPtr,
		ApplyFlags,
		ApplyFlagsToForce,
		ProgressCallBack,
		CallbackHandle,
		Handle
		);
	if (Code)
	{
		ExaPatchError( ProgressCallBack, CallbackHandle, HeaderPtr, NULL, (DWORD) Code );
		/* free the header structure */
		ExaPatchApplyFreeParsedHeader( HeaderPtr );
	}
	ReleaseChannel( Handle );
	return(Code);
	
}

int EXP_DLLIMPORT
	ExaPatchApplyFinishFile( 
		PLATWORD Handle,
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		DWORD SuccessFlag,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		)
{
	int Code;

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	Code = ExaPatchFinishWork(
		HeaderPtr,
		SuccessFlag,
		ProgressCallBack,
		CallbackHandle
		);
	if (Code)
	{
		ExaPatchError( ProgressCallBack, CallbackHandle, HeaderPtr, NULL, (DWORD) Code );
	}
	/* free the header structure */
	ExaPatchApplyFreeParsedHeader( HeaderPtr );
	ReleaseChannel( Handle );
	return(Code);
	
}
# endif

int EXP_DLLIMPORT
	ExaPatchApplyDoEntireFile( 
		PLATWORD Handle,
		ExaPatchStream * FileToApply, /* assumed to be positioned at header */
		wchar_t * ApplyDirectory,
		wchar_t * BackupDirectory,
		wchar_t * SingleFile,
		wchar_t * DrivesToCheck,
		DWORD ApplyFlags,
		DWORD ApplyFlagsToForce,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		)
{
	int Code;
	ExaPatchApplyFileHeaderInfo TheHeader;
	ExaPatchApplyFileEntryInfo TheEntry;
	ExaPatchStream * OldStream = NULL;
	ExaPatchStream * NewStream = NULL;

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	memset( &TheHeader, 0, sizeof( ExaPatchApplyFileHeaderInfo ) );
	TheHeader.Comm.dwReserved = 1;
	TheHeader.dwExtensionSize = CUR_EXTENSION_SIZE;
	TheHeader.DrivesToCheck = DrivesToCheck;
	TheHeader.SingleFileName = SingleFile;
# ifdef EXAPATCH_BACKUP_SUPPORT
	TheHeader.BackupDirectory = BackupDirectory;
# endif
	TheHeader.ApplyDirectory = ApplyDirectory;
	Code = ExaPatchStartWork(
		FileToApply,
		&TheHeader,
		ApplyFlags,
		ApplyFlagsToForce,
		ProgressCallBack,
		CallbackHandle,
		Handle
		);
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# ifdef QWORD_IS_DWORD
  while (TheHeader.qwNextEntryNum != 0xffffffffU)
# else
  while (TheHeader.qwNextEntryNum != 0xffffffffffffffffU)
# endif
  {
		Code = ExaPatchEntryStartWork(
			FileToApply,
			&TheHeader,
			&TheEntry,
			ProgressCallBack,
			CallbackHandle,
			Handle
			);
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
    if (TheEntry.wPatchType == EXAPATCH_EOF_TYPE)
    {
# ifdef QWORD_IS_DWORD
      TheHeader.qwNextEntryNum = 0xffffffffU;
# else
      TheHeader.qwNextEntryNum = 0xffffffffffffffffU;
# endif
    }
    else
    {
# ifdef EXAPATCH_LIST_SUPPORT
      if ((TheHeader.ApplyFlags & EXP_PATCH_APPLY_LISTONLY)||
#  ifdef EXAPATCH_ONEFILE_SUPPORT
      	((TheHeader.ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY) &&
			  	PSFNwcscmp(TheEntry.Comm.lpEntryName, TheHeader.SingleFileName))
#  endif
			  	)
		  {
			  /* seek past patch info */
			  Code = SeekExaStream( FileToApply,
					((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
					  EXAPATCH_SEEK_CUR,
					  NULL );
			  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */

		  }
		  else
# else
#  ifdef EXAPATCH_ONEFILE_SUPPORT
      if ((TheHeader.ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY) &&
			  	PSFNwcscmp(TheEntry.Comm.lpEntryName, TheHeader.SingleFileName))
		  {
			  /* seek past patch info */
			  Code = SeekExaStream( FileToApply,
					  ((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
					  EXAPATCH_SEEK_CUR,
					  NULL );
			  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */

		  }
		  else
#  endif
# endif
		  {
			  Code = ExaPatchEntryFindFiles(
				  FileToApply,
				  &TheHeader,
				  &TheEntry,
          ProgressCallBack,
          CallbackHandle,
				  &OldStream,
				  &NewStream,
					Handle
				  );
			  if ((TheEntry.Comm.dwFileFlags & EXP_FILE_IGNOREMISSING)
				  && ((Code == EXAPATCH_FILE_NOT_FOUND) || (Code == EXAPATCH_INVALID_FILE_FOUND)))
			  {
					TheHeader.Stats[(Code == EXAPATCH_FILE_NOT_FOUND)?EXP_FILES_MISSING:EXP_FILES_INVALID]++;
				  /* issue a warning */
					Code = ExaPatchWarning( ProgressCallBack, CallbackHandle, &TheHeader, &TheEntry, (DWORD) EXAPATCH_WARNING_FILE_NOT_FOUND );
					if (Code)
					{
						Code = EXAPATCH_USER_CANCEL;
						goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
					}
				  /* seek past patch info */
				  Code = SeekExaStream( FileToApply,
				  	((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
				  	EXAPATCH_SEEK_CUR,
				  	NULL );
					if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			  }
			  else if (Code == EXAPATCH_ALREADY_PATCHED)
			  {
# ifdef EXAPATCH_UPDATELINK_SUPPORT
					if (0U == (TheEntry.Comm.dwFileFlags & EXP_FILE_UPDATE_LINK))
# endif
					{
						/* issue warning */
						TheHeader.Stats[EXP_FILES_SKIPPED]++;
						Code = ExaPatchWarning( ProgressCallBack, CallbackHandle, &TheHeader, &TheEntry, (DWORD) EXAPATCH_WARNING_FILE_ALREADY_PATCHED );
						if (Code)
						{
							Code = EXAPATCH_USER_CANCEL;
							goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
						}

						/* seek past patch info */
						Code = SeekExaStream( FileToApply,
								((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
								EXAPATCH_SEEK_CUR,
								NULL );
						if (Code) goto exit;
						Code = MarkProgress(&TheHeader, &TheEntry, ProgressCallBack, CallbackHandle, 0, 0x8000U );
						if (Code) goto exit;
					}
			  }
# ifdef EXAPATCH_VERIFYONLY_SUPPORT 
			  else if ((Code == EXAPATCH_SUCCESS && (TheHeader.ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY)) ||
				  ((Code == EXAPATCH_FILE_NOT_FOUND && (TheEntry.Comm.Files.dwNumOldFiles == 0) && (TheHeader.ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY))))
			  {
				  /* seek past patch info (assume ADD entries always succeed) */
				  Code = SeekExaStream( FileToApply,
						  ((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
						  EXAPATCH_SEEK_CUR,
						  NULL );
				  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			  }
# endif
			  else if (Code != EXAPATCH_SUCCESS)
        {
          goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
        }
        else
			  {
				  /* do the patch */
				  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				  Code = ExaPatchEntryDoWork(
					  FileToApply,
					  &TheHeader,
					  &TheEntry,
					  OldStream,
					  NewStream,
					  ProgressCallBack,
					  CallbackHandle,
						((ExaPatchApplyChannel *)Handle)->BufferBreak,
						0,1,Handle
					  );
				  if (Code) 
				  {
					  ExaPatchEntryFinishWork(
						  FileToApply,
						  &TheHeader,
						  &TheEntry,
						  ProgressCallBack,
						  CallbackHandle,
						  FALSE);
					  goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				  }
					if (OldStream)
					{
					  Code = CloseExaStream( OldStream );
					  OldStream = NULL;
					}
				  if (Code)
				  {
					  ExaPatchEntryFinishWork(
						  FileToApply,
						  &TheHeader,
						  &TheEntry,
						  ProgressCallBack,
						  CallbackHandle,
						  FALSE);
					  goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */;
				  }
					if (NewStream)
					{
					  Code = CloseExaStream( NewStream );
					  NewStream = NULL;
					}
				  if (Code)
				  {
					  ExaPatchEntryFinishWork(
						  FileToApply,
						  &TheHeader,
						  &TheEntry,
						  ProgressCallBack,
						  CallbackHandle,
						  FALSE);
					  goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				  }
			  }
		  }
    }

		Code = ExaPatchEntryFinishWork(
		  FileToApply,
			&TheHeader,
			&TheEntry,
			ProgressCallBack,
			CallbackHandle,
			TRUE
			);
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
  }
	Code = ExaPatchFinishWork(
		&TheHeader,
		TRUE,
    ProgressCallBack,
    CallbackHandle
		);
	if (0 != Code) {goto exit2;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
exit:
	if (OldStream)
	{
		CloseExaStream( OldStream );
	}
	if (NewStream)
	{
		CloseExaStream( NewStream );
	}

	if (Code)
	{
		ExaPatchFinishWork(
			&TheHeader,
			FALSE,
      ProgressCallBack,
      CallbackHandle
			);
	}

exit2:
	if (Code)
	{
		ExaPatchError( ProgressCallBack, CallbackHandle, &TheHeader, &TheEntry, (DWORD) Code );
	}
	/* free the header structure */
	ExaPatchApplyFreeParsedHeader( &TheHeader );
	ReleaseChannel( Handle );
	return(Code);
}

# ifdef EXAPATCH_CONTAINER_SUPPORT
int /* ErrCode */
	EXP_DLLIMPORT
	ExaPatchApplySelectPatch(
		PLATWORD Handle,
		DWORD dwIndex )
{
	int Code;
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle;

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	TheChannel->dwPatchIndex = dwIndex;
	ReleaseChannel( Handle );
	return(EXAPATCH_SUCCESS);
		
}
# endif

# ifdef EXAPATCH_DETAILED_APPLY_API_SUPPORT
int EXP_DLLIMPORT
	ExaPatchApplyParseEntryInfo( 
		PLATWORD Handle,
		ExaPatchStream * FileToParse,	/*  assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr
		)
{
	int Code;

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	Code = ExaPatchEntryParseWork(
		FileToParse,
		HeaderPtr,
		EntryPtr,
		FALSE
		);
	ReleaseChannel( Handle );
	return(Code);
}

int EXP_DLLIMPORT
	ExaPatchApplyDoEntry( 
		PLATWORD Handle,
		ExaPatchStream * FileToApply, /* assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		)
{
	int Code;
	ExaPatchStream * OldStream = NULL;
	ExaPatchStream * NewStream = NULL;
	ExaPatchApplyFileEntryInfo TheEntry;

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	Code = ExaPatchEntryStartWork(
		FileToApply,
		HeaderPtr,
		&TheEntry,
		ProgressCallBack,
		CallbackHandle,
		Handle
		);
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	if (TheEntry.wPatchType != EXAPATCH_EOF_TYPE)
	{
# ifdef EXAPATCH_LIST_SUPPORT
    if ((HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY) ||
#  ifdef EXAPATCH_ONEFILE_SUPPORT
    	((HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY) &&
		  	PSFNwcscmp(TheEntry.Comm.lpEntryName, HeaderPtr->SingleFileName))
#  endif
		  	)
		{
		  /* seek past patch info */
		  Code = SeekExaStream( FileToApply,
				  ((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
				  EXAPATCH_SEEK_CUR,
				  NULL );
		  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
		}
		else
# else
#  ifdef EXAPATCH_ONEFILE_SUPPORT
    if ((HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY) &&
		  	PSFNwcscmp(TheEntry.Comm.lpEntryName, HeaderPtr->SingleFileName))
		{
		  /* seek past patch info */
		  Code = SeekExaStream( FileToApply,
				  ((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
				  EXAPATCH_SEEK_CUR,
				  NULL );
		  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
		}
		else
#  endif
# endif
		{
			Code = ExaPatchEntryFindFiles(
				FileToApply,
				HeaderPtr,
				&TheEntry,
		    ProgressCallBack,
		    CallbackHandle,
				&OldStream,
				&NewStream,
				Handle
				);

			if ((0 != Code) && (Code != EXAPATCH_ALREADY_PATCHED)) 
			{
				goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			}
			if (Code == EXAPATCH_ALREADY_PATCHED)
			{
				/* issue warning */
					HeaderPtr->Stats[EXP_FILES_SKIPPED]++;
					Code = ExaPatchWarning( ProgressCallBack, CallbackHandle, HeaderPtr, &TheEntry, (DWORD) EXAPATCH_WARNING_FILE_ALREADY_PATCHED );
					if (Code)
					{
						Code = EXAPATCH_USER_CANCEL;
						goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
					}

				  /* seek past patch info */
				  Code = SeekExaStream( FileToApply,
						  ((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
						  EXAPATCH_SEEK_CUR,
						  NULL );				  
			  } else

# ifdef EXAPATCH_VERIFYONLY_SUPPORT
			if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY)
			{
			  /* seek past patch info */
			  Code = SeekExaStream( FileToApply,
					  ((TheEntry.wPatchType & 0xf) == EXAPATCH_HISTORY_TYPE)?TheEntry.Comm.qwHistorySize:TheEntry.qwEntrySize,
					  EXAPATCH_SEEK_CUR,
					  NULL );
			  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			}
			else
# endif
			{
				Code = ExaPatchEntryDoWork(
					FileToApply,
					HeaderPtr,
					&TheEntry,
					OldStream,
					NewStream,
					ProgressCallBack,
					CallbackHandle,
					((ExaPatchApplyChannel *)Handle)->BufferBreak,
					0, 1, Handle
					);
				if (Code) 
				{
					ExaPatchEntryFinishWork(
						FileToApply,
						HeaderPtr,
						&TheEntry,
						ProgressCallBack,
						CallbackHandle,
						FALSE);
					goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				}
				if (OldStream)
				{
					Code = CloseExaStream( OldStream );
					OldStream = NULL;
				}
				if (Code)
				{
					ExaPatchEntryFinishWork(
						FileToApply,
						HeaderPtr,
						&TheEntry,
						ProgressCallBack,
						CallbackHandle,
						FALSE);
					goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				}
				if (NewStream)
				{
					Code = CloseExaStream( NewStream );
					NewStream = NULL;
				}
				if (Code) 
				{
					ExaPatchEntryFinishWork(
						FileToApply,
						HeaderPtr,
						&TheEntry,
						ProgressCallBack,
						CallbackHandle,
						FALSE);
					goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				}
			}
		}
		Code = ExaPatchEntryFinishWork(
		  FileToApply,
			HeaderPtr,
			&TheEntry,
			ProgressCallBack,
			CallbackHandle,
			TRUE
			);
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	}
exit:
	if (OldStream)
	{
		CloseExaStream( OldStream );
	}
	if (NewStream)
	{
		CloseExaStream( NewStream );
	}
	if (Code)
	{
		ExaPatchError( ProgressCallBack, CallbackHandle, HeaderPtr, &TheEntry, (DWORD) Code );
	}
	ReleaseChannel( Handle );
	return(Code);
}

int EXP_DLLIMPORT
	ExaPatchApplyStartEntry( 
		PLATWORD Handle,
		ExaPatchStream * FileToApply,	/*  assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		)
{
  int Code;

	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	Code = ExaPatchEntryStartWork(
		FileToApply,
				HeaderPtr,
				EntryPtr,
				ProgressCallBack,
				CallbackHandle,
				Handle );
	if (Code)
	{
		ExaPatchError( ProgressCallBack, CallbackHandle, HeaderPtr, EntryPtr, (DWORD) Code );
	}
	ReleaseChannel( Handle );
	return(Code);
}

int EXP_DLLIMPORT
	ExaPatchApplyFinishEntry( 
		PLATWORD Handle,
		ExaPatchStream * FileToApply,	/*  assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * EntryPtr,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		DWORD SuccessFlag
		)
{
  int Code;
	Code = AcquireChannel( Handle );
	if (Code)
		return(Code);
	Code = ExaPatchEntryFinishWork(
			FileToApply,
			HeaderPtr,
			EntryPtr,
			ProgressCallBack,
			CallbackHandle,
			SuccessFlag );
	if (Code)
	{
		ExaPatchError( ProgressCallBack, CallbackHandle, HeaderPtr, EntryPtr, (DWORD) Code );
	}
	ReleaseChannel( Handle );
	return(Code);
}
# endif
# endif /* ATTOPATCH */

int EXP_DLLIMPORT
	ExaPatchApplyDoEntryRaw( 
		PLATWORD Handle,
		ExaPatchStream * FileToApply, /* assumed to be positioned correctly */
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
# ifdef ATTOPATCH
		const ExaPatchApplyFileEntryInfo * EntryPtr,
# else
		ExaPatchApplyFileEntryInfo * EntryPtr,
# endif
		ExaPatchStream * OldStream,
		ExaPatchStream * NewStream,
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle
		)
{
  int Code;
	ExaPatchApplyFileHeaderInfo * DummyHeaderPtr = NULL;
  ExaPatchApplyFileEntryInfo * DummyEntryPtr = NULL;

	if (!FileToApply)
	{
		return (EXAPATCH_INVALID_PARAMETER); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	if (!HeaderPtr)
	{
		Code = ExaMemAlloc(NULL, sizeof(ExaPatchApplyFileHeaderInfo), (void **) &DummyHeaderPtr); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {return (Code);} /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
		memset (DummyHeaderPtr, 0, sizeof (ExaPatchApplyFileHeaderInfo));
		DummyHeaderPtr->Comm.qwNumEntries = 1;
		DummyHeaderPtr->Comm.dwReserved = 1;
		DummyHeaderPtr->dwExtensionSize = (DWORD)CUR_EXTENSION_SIZE;
	}
	if (!EntryPtr)
	{
		Code = ExaMemAlloc(NULL, sizeof(ExaPatchApplyFileEntryInfo), (void **) &DummyEntryPtr); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		memset (DummyEntryPtr, 0, sizeof (ExaPatchApplyFileEntryInfo));

		DummyEntryPtr->wPatchType = EXAPATCH_ADC_TYPE;
		DummyEntryPtr->Comm.Files.dwNumOldFiles = 1;
		DummyEntryPtr->Comm.Files.dwNumNewFiles = 1;
		DummyEntryPtr->wModFlags = EXP_STATUS_CONTENTS;
		DummyEntryPtr->qwEntryNum = 0;
		DummyEntryPtr->qwEntrySize = FileToApply->Size;
	}

	Code = AcquireChannel( Handle );
	if (0 != Code)
	{
		// @@EB MALLOC MODIFY
		if (NULL != DummyHeaderPtr)
		{
			(void)ExaMemFree (NULL, DummyHeaderPtr);
		}
		if (NULL != DummyEntryPtr)
		{
			(void)ExaMemFree (NULL, DummyEntryPtr);
		}
		//

		return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	Code = ExaPatchEntryDoWork(
			FileToApply,
			(NULL != HeaderPtr)?HeaderPtr:DummyHeaderPtr,
			(NULL != EntryPtr)?EntryPtr:DummyEntryPtr,
			OldStream,
			NewStream,
			ProgressCallBack,
			CallbackHandle,
			((ExaPatchApplyChannel *)Handle)->BufferBreak, /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
			0,1,Handle
			 );
# ifndef ATTOPATCH
	if (Code)
	{
		ExaPatchError( ProgressCallBack, CallbackHandle, (NULL != HeaderPtr)?HeaderPtr:DummyHeaderPtr, EntryPtr, (DWORD) Code );
	}
# endif

	(void)ReleaseChannel( Handle ); /* MISRA C 2012 [2.2]: function lacks side-effects in this compilation configuration */

exit:
	if (NULL != DummyHeaderPtr)
	{
		(void)ExaMemFree (NULL, DummyHeaderPtr);
	}
	if (NULL != DummyEntryPtr)
	{
    (void)ExaMemFree (NULL, DummyEntryPtr);
	}
	return(Code);
}

# if !defined(_WIN32) && !defined(ATTOPATCH)
static void Slashify( wchar_t * Ptr )
{
	if (!Ptr)
		return;
	while (*Ptr)
	{
		if (*Ptr == L'\\')
		{
			*Ptr = L'/';
		}
		Ptr++;
	}
}
# endif

# ifdef EVAL
static void	Decrypt ( const unsigned char * Source, UCHAR * Dest )
{
  USHRT		Adjust = 0x31;
  int			i;
  for ( i=0 ; i<11 ; i++)
  {
    *Dest = (unsigned char) (((USHRT)*Source) - Adjust);
    if (0U == (*Dest & 0xffU) )
    {
      break;
    }
    Source++;
    Dest++;
    Adjust++;
  }
}

# ifndef UNIX_SUPPORT
static int PrintBanner (void)
{
	unsigned char FormattedSerNum[15];
	const unsigned char * Ptr = &SerialNumber[11];
  Decrypt (Ptr, FormattedSerNum);
# ifdef EVAL
	{
		// check system clock and verify against cut-off time
		time_t TheTime;
		struct tm * CurrentTime;
		int Expired = FALSE;
		int month, day, year;
		
		FormattedSerNum[10]='\0';
		time (&TheTime);
		CurrentTime = localtime (&TheTime);
		year = atol (FormattedSerNum+6);
		FormattedSerNum[6]='\0';
		day = atol (FormattedSerNum+4);
		FormattedSerNum[3]='\0';
		month = atol (FormattedSerNum);

		if (year < (CurrentTime->tm_year + 1900))
		{
			Expired = TRUE;
		} else if ( (year == (CurrentTime->tm_year + 1900)) && ((month-1) < CurrentTime->tm_mon) )
		{
			Expired = TRUE;
		} else if ( (year == (CurrentTime->tm_year + 1900)) && ((month-1) == CurrentTime->tm_mon)  && (day < CurrentTime->tm_mday) )
		{
			Expired = TRUE;
		}

    // simple check for clock rollback
    // check the SYSDIR\config\software file's timestamp (NT only)
    if (!(0x80000000U & GetVersion()))
		{
			char SysPath[MAX_PATH];
			GetSystemDirectory (SysPath, MAX_PATH);
      //@@CACING PENTEST strcat (SysPath, "\\config\\software");
      strncat (SysPath, "\\config\\software",18);
			if (GetFileAttributes (SysPath) != 0xffffffffU)
			{
				HANDLE hFile;
				SYSTEMTIME SysTime;
				hFile = CreateFile (SysPath, 0,FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
				{
					BY_HANDLE_FILE_INFORMATION FileInfo;
					memset (&FileInfo, 0, sizeof (BY_HANDLE_FILE_INFORMATION));
					GetFileInformationByHandle (hFile, &FileInfo);
					memset (&SysTime, 0, sizeof (SYSTEMTIME));
					FileTimeToSystemTime (&FileInfo.ftLastAccessTime, &SysTime);
		      if (year < SysTime.wYear)
					{
						Expired = TRUE;
					} else if ( (year == SysTime.wYear) && (month < SysTime.wMonth) )
					{
						Expired = TRUE;
					} else if ( (year == SysTime.wYear) && (month == SysTime.wMonth) && (day < SysTime.wDay) )
					{
						Expired = TRUE;
					}
					memset (&SysTime, 0, sizeof (SYSTEMTIME));
					FileTimeToSystemTime (&FileInfo.ftLastWriteTime, &SysTime);
		      if (year < SysTime.wYear)
					{
						Expired = TRUE;
					} else if ( (year == SysTime.wYear) && (month < SysTime.wMonth) )
					{
						Expired = TRUE;
					} else if ( (year == SysTime.wYear) && (month == SysTime.wMonth) && (day < SysTime.wDay) )
					{
						Expired = TRUE;
					}
					CloseHandle (hFile);
				}
			}
		}
		if (Expired)
		{
			return (1);
		}
	}
# else
  FormattedSerNum[11]=FormattedSerNum[9];
  FormattedSerNum[10]=FormattedSerNum[8];
  FormattedSerNum[9]=FormattedSerNum[7];
  FormattedSerNum[8]=FormattedSerNum[6];
  FormattedSerNum[6]=FormattedSerNum[5];
  FormattedSerNum[5]=FormattedSerNum[4];
  FormattedSerNum[4]=FormattedSerNum[3];
  FormattedSerNum[3]=(unsigned char)'-';
  FormattedSerNum[7]=(unsigned char)'-';
  FormattedSerNum[12]=(unsigned char)'\0';
# endif /* EVAL */
	return (0);
}
# else

# ifdef EVAL
/*-----------------------------------------------------------------------*\
    WaitForKeyStroke
\*-----------------------------------------------------------------------*/
int  WaitForKeyStroke( char AbortKey )
{
   char         ScratchChar;
   char         TempBuf[MAX_PATH];

   fgets( TempBuf, MAX_PATH, stdin );
   ScratchChar = TempBuf[0];
  /* Here, ScratchChar is the (first) key pressed
  if it's equal to the AbortKey, return -1,
  else return 0 */

  if ( AbortKey && (toupper(ScratchChar) == toupper(AbortKey) ) )
  {
    return(-1);
  }
  else
  {
    return( 0 );
  }
}

void  PrintEvalMessage( void )
{
   /*
      This is an evaluation copy, so print the "evaluation-only"
      message and wait for a key press
   */
   fputs("\n"
          "This fully functional evaluation copy may be used\n"
          "SOLELY to determine if RTPatch Server is suitable\n"
          "for your needs.  Any other use is prohibited.\n\n"
          "(C) Copyright 2003-2016 Pocket Soft, Inc.\n"
          "All Rights Reserved.\n\n"
          "+1 713-460-5600   FAX +1 713-460-2651\n"
          "Pocket Soft, Inc.\n"
          "P.O. Box 821049\nHouston, TX 77282\nUSA\n\n"
          "Press any key to continue (Q to quit): \n", stderr );
   if (WaitForKeyStroke('Q')) exit(1);
   return;
}

# endif
# endif
# endif
# if defined(ADS_SUPPORT) || defined( EA_SUPPORT )
static int WildMatch( char * Spec, char * Name )
{
	/*
	returns 0 if no match, 1 if wild match, 2 if exact match
	*/
	char * Ptr = Spec;
	char * Ptr2 = Name;
	int Code = 2;

	while (*Ptr || *Ptr2)
	{
		if (toupper(*Ptr) == toupper(*Ptr2))
		{
			// ordinary match
			Ptr++;
			Ptr2++;
		}
		else if (*Ptr == '?')
		{
			// single-character wild match
			Ptr++;
			Ptr2++;
			Code = 1;
		}
		else if (*Ptr == '*')
		{
			// star
			Ptr++;
			if (*Ptr)
			{
				// here, there's something after the star
				while (*Ptr2)
				{
					// look for a match for "after the star"
					while (*Ptr2 && (toupper(*Ptr2) != toupper(*Ptr)))
					{
						Ptr2++;
					}
					if (*Ptr2)
					{
						// we matched one char - see if the whole thing matches
						if (WildMatch(Ptr,Ptr2))
						{
							return(1);
						}
						// look for the next match for "after the star"
						Ptr2++;
					}
				}
				// here, we ran out of the Name before we found a match
				return(0);
			}
			else
			{
				// here, there is nothing after the star
				return(1);
			}
		}
		else
		{
			// a non-wild mismatch
			return(0);
		}
	}
	// we got to the end without a mismatch (or a star)
	return(Code);
}
static int WildMatchList( DWORD NumSpecs, char ** SpecList, char * Name )
{
	/*
	returns 0 if no match, 1 if wild match, 2 if exact match
	*/
	int Code = 0;
	DWORD i;
	int ThisMatch;

	for (i=0; (Code < 2) && i<NumSpecs ; i++ )
	{
		ThisMatch = WildMatch( SpecList[i], Name );
		if (ThisMatch > Code)
		{
			Code = ThisMatch;
		}
	}
	return(Code);
}
# endif
# ifdef EXAPATCH_EH_SUPPORT
#  ifdef ADS_SUPPORT
static int DoADS( ExaPatchApplyFileManipulation *pManip, ExaPatchExtendedHeader *pEH)
{
	int Code = EXAPATCH_SUCCESS;
	char * OldFile = NULL;
	char * NewFile = NULL; 
	unsigned NumSpecs[3];
	DWORD Sizes[3];
	DWORD Origins[3];
	DWORD Ends[3];
	DWORD ValueSizeBackout=0;
	char ** Specs[3]=	{NULL,NULL,NULL};
	char * NameBuffers[3]={NULL,NULL,NULL};
	void ** Vals = NULL;
	DWORD i;
	DWORD Section;
	DWORD SpecNum;
	char * Ptr;
	char * BufPtr;
	DWORD ThisSpecSize;
	ADSData ADSD;
	size_t NameSize;
	char OldNameBuffer[MAX_PATH];
	int WildLevel1;
	int WildLevel2;
	void * ValBuffer;
	UINT64 ValBufferSize;

	if (pManip == NULL) return(Code);
	
	adsUtilsInit();
	Ptr = pEH->HeaderBlock;
	Section=i=0;
	while ((i < pEH->qwSize) && (Section < 3))
	{
		// first pass to count things
		Sizes[Section] = *((DWORD *)(Ptr+i));
		Origins[Section] = i;
		Ends[Section] = i + Sizes[Section] + 4;
		if (Ends[Section] > pEH->qwSize)
		{
			Ends[Section] = (DWORD) pEH->qwSize;
		}
		NumSpecs[Section]=0;
		i += 4;
		while (i < Ends[Section])
		{
			NumSpecs[Section]++;
			i += 2 + *((UINT16 *)(Ptr + i));
			if (Section == 2)
			{
				ValueSizeBackout += 4 + *((DWORD *)(Ptr + i));
				i += 4 + *((DWORD *)(Ptr + i));
			}
		}
		i = Ends[Section];
		Section++;
	}
	// allocate space to remember things
	for (i=0; (i<3) && (Code == EXAPATCH_SUCCESS) ; i++ )
	{
		Code = ExaMemAlloc( NULL, NumSpecs[i]*sizeof(void *),(void **) &Specs[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			memset( Specs[i], 0, NumSpecs[i]*sizeof(void *) );
			Code = ExaMemAlloc( NULL, (i==2)?(Sizes[i]-ValueSizeBackout):Sizes[i], (void **) &NameBuffers[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
	}
	Code = ExaMemAlloc( NULL, NumSpecs[2]*sizeof(void *), (void **) &Vals ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (Code == EXAPATCH_SUCCESS)
	{
		for (Section = 0; Section < 3 ; Section++ )
		{
			// second pass to remember things
			i = Origins[Section]+4;
			SpecNum = 0;
			BufPtr = NameBuffers[Section];
			while (i < Ends[Section])
			{
				Specs[Section][SpecNum] = BufPtr;
				ThisSpecSize = *((UINT16 *) (Ptr+i));
				memcpy( BufPtr, Ptr+i+2, ThisSpecSize);
				BufPtr[ThisSpecSize] = '\0';
				BufPtr += ThisSpecSize+1;
				i += 2+ThisSpecSize;
				if (Section == 2)
				{
					Vals[SpecNum] = (void *)(Ptr+i+4);
					i += 4+*((DWORD *)(Ptr+i));
				}
				SpecNum++;
			}
		}
		if (pManip && pManip->IntFile)
		{
			NameSize = PSwcstombs( NULL, pManip->IntFile, 0 );
			Code = ExaMemAlloc(NULL, NameSize+1, (void **) &NewFile); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (EXAPATCH_SUCCESS == Code)
			{
				PSwcstombs( NewFile, pManip->IntFile, NameSize+1 );
			}
		}
		if (pManip && pManip->OldFile)
		{
			NameSize = PSwcstombs( NULL, pManip->OldFile, 0 );
			Code = ExaMemAlloc(NULL, NameSize+1, (void **) &OldFile);
			if (EXAPATCH_SUCCESS == Code)
			{
				PSwcstombs( OldFile, pManip->OldFile, NameSize+1 );
			}
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			if (OldFile)
			{
				// do DELETE/RETAIN processing
				if (adsGetAll(OldFile, &ADSD))
				{
					Code = EXAPATCH_ADS_ERROR;
				}
				else
				{
					for (i=0; i<ADSD.ADSCount ; i++ )
					{
						adsCopyNameAZ( &ADSD, i, OldNameBuffer, MAX_PATH );
						if (*OldNameBuffer)
						{
							WildLevel1 = WildMatchList( NumSpecs[0], Specs[0], OldNameBuffer );
							WildLevel2 = WildMatchList( NumSpecs[1], Specs[1], OldNameBuffer );
							if (WildLevel2 > WildLevel1)
							{
								adsLookupValueSize( &ADSD, i, &ValBufferSize );
								if (ValBufferSize > 0x80000000U)
								{
									Code = EXAPATCH_ADS_ERROR;
								}
								else
								{
									Code = ExaMemAlloc( NULL, (DWORD) ValBufferSize, (void **) &ValBuffer ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
								}
								if (Code == EXAPATCH_SUCCESS)
								{
									if (adsGetValue(OldFile, OldNameBuffer, ValBuffer, (UINT32) ValBufferSize))
									{
										Code = EXAPATCH_ADS_ERROR;
									}
									else
									{
										if (adsSetValue(NewFile, OldNameBuffer, ValBuffer, (UINT32)ValBufferSize, 0))
										{
											Code = EXAPATCH_ADS_ERROR;
										}
									}
									ExaMemFree( NULL, ValBuffer );
								}
							}
						}
					}
				}
			}
			// now do the NEW specs
			for (i=0; (i<NumSpecs[2])&&(Code == EXAPATCH_SUCCESS) ; i++ )
			{
				if (adsSetValue( NewFile, Specs[2][i], Vals[i], *((DWORD *)(((char *)Vals[i])-4)),0))
				{
					Code = EXAPATCH_ADS_ERROR;
				}
			}
		}
	}
	for (i=0; i<3 ; i++ )
	{
		if (Specs[i])
		{
			ExaMemFree( NULL, Specs[i] );
		}
		if (NameBuffers[i])
		{
			ExaMemFree( NULL, NameBuffers[i] );
		}
	}
	if (Vals)
	{
		ExaMemFree( NULL, Vals );
	}
	if (OldFile)
	{
		ExaMemFree( NULL, OldFile );
		adsGetAllCleanup( &ADSD );
	}
	if (NewFile)
	{
		ExaMemFree( NULL, NewFile );
	}
	return(Code);
}
#  endif
#  ifdef EA_SUPPORT
static int DoEA( ExaPatchApplyFileManipulation *pManip, ExaPatchExtendedHeader *pEH)
{
	int Code = EXAPATCH_SUCCESS;
	char * OldFile = NULL;
	char * NewFile = NULL; 
	unsigned NumSpecs[3];
	DWORD Sizes[3];
	DWORD Origins[3];
	DWORD Ends[3];
	char ** Specs[3]=	{NULL,NULL,NULL};
	char * NameBuffers[3]={NULL,NULL,NULL};
	void ** Vals = NULL;
	DWORD i;
	DWORD Section;
	DWORD SpecNum;
	char * Ptr;
	char * BufPtr;
	DWORD ThisSpecSize;
	EAData EAD;
	size_t NameSize;
	char * OldNameBuffer;
	int WildLevel1;
	int WildLevel2;
	void * ValBuffer;
	unsigned ValBufferSize;
	DWORD ValueSizeBackout = 0;

	if (pManip == NULL) return(Code);
	
	eaUtilsInit();
	Ptr = pEH->HeaderBlock;
	Section=i=0;
	while ((i < pEH->qwSize) && (Section < 3))
	{
		// first pass to count things
		Sizes[Section] = *((DWORD *)(Ptr+i));
		Origins[Section] = i;
		Ends[Section] = i + Sizes[Section] + 4;
		if (Ends[Section] > pEH->qwSize)
		{
			Ends[Section] = (DWORD) pEH->qwSize;
		}
		NumSpecs[Section]=0;
		i += 4;
		while (i < Ends[Section])
		{
			NumSpecs[Section]++;
			i += 1 + Ptr[i];
			if (Section == 2)
			{
				ValueSizeBackout += 2 + *((UINT16 *)(Ptr + i));
				i += 2 + *((UINT16 *)(Ptr + i));
			}
		}
		i = Ends[Section];
		Section++;
	}
	// allocate space to remember things
	for (i=0; (i<3) && (Code == EXAPATCH_SUCCESS) ; i++ )
	{
		Code = ExaMemAlloc( NULL, NumSpecs[i]*sizeof(void *),(void **) &Specs[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			memset( Specs[i], 0, NumSpecs[i]*sizeof(void *) );
			Code = ExaMemAlloc( NULL, (i==2)?(Sizes[i]-ValueSizeBackout):Sizes[i], (void **) &NameBuffers[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
	}
	Code = ExaMemAlloc( NULL, NumSpecs[2]*sizeof(void *), (void **) &Vals ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (Code == EXAPATCH_SUCCESS)
	{
		for (Section = 0; Section < 3 ; Section++ )
		{
			// second pass to remember things
			i = Origins[Section]+4;
			SpecNum = 0;
			BufPtr = NameBuffers[Section];
			while (i < Ends[Section])
			{
				Specs[Section][SpecNum] = BufPtr;
				ThisSpecSize = Ptr[i];
				memcpy( BufPtr, Ptr+i+1, ThisSpecSize);
				BufPtr[ThisSpecSize] = '\0';
				BufPtr += ThisSpecSize+1;
				i += 1+ThisSpecSize;
				if (Section == 2)
				{
					Vals[SpecNum] = (void *)(Ptr+i+2);
					i += 2+*((UINT16 *)(Ptr+i));
				}
				SpecNum++;
			}
		}
		if (pManip && pManip->IntFile)
		{
			NameSize = PSwcstombs( NULL, pManip->IntFile, 0 );
			Code = ExaMemAlloc(NULL, NameSize+1, (void **) &NewFile); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (EXAPATCH_SUCCESS == Code)
			{
				PSwcstombs( NewFile, pManip->IntFile, NameSize+1 );
			}
		}
		if (pManip && pManip->OldFile)
		{
			NameSize = PSwcstombs( NULL, pManip->OldFile, 0 );
			Code = ExaMemAlloc(NULL, NameSize+1, (void **) &OldFile); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (EXAPATCH_SUCCESS == Code)
			{
				PSwcstombs( OldFile, pManip->OldFile, NameSize+1 );
			}
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			if (OldFile)
			{
				// do DELETE/RETAIN processing
				if (eaGetAll(OldFile, &EAD))
				{
					Code = EXAPATCH_EA_ERROR;
				}
				else
				{
					for (i=0; i<EAD.EACount ; i++ )
					{
						eaLookupName( &EAD, i, &OldNameBuffer);
						if (*OldNameBuffer)
						{
							WildLevel1 = WildMatchList( NumSpecs[0], Specs[0], OldNameBuffer );
							WildLevel2 = WildMatchList( NumSpecs[1], Specs[1], OldNameBuffer );
							if (WildLevel2 > WildLevel1)
							{
								eaLookupValueSize( &EAD, i, &ValBufferSize );
								Code = ExaMemAlloc( NULL, (DWORD) ValBufferSize, (void **) &ValBuffer ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
								if (Code == EXAPATCH_SUCCESS)
								{
									if (eaCopyValue(&EAD, i, ValBuffer, (unsigned) ValBufferSize))
									{
										Code = EXAPATCH_EA_ERROR;
									}
									else
									{
										if (eaSetValue(NewFile, OldNameBuffer, ValBuffer, (UINT32)ValBufferSize, 0))
										{
											Code = EXAPATCH_EA_ERROR;
										}
									}
									ExaMemFree( NULL, ValBuffer );
								}
							}
						}
					}
				}
			}
			// now do the NEW specs
			for (i=0; (i<NumSpecs[2])&&(Code == EXAPATCH_SUCCESS) ; i++ )
			{
				if (eaSetValue( NewFile, Specs[2][i], Vals[i], *((UINT16 *)(((char *)Vals[i])-2)),0))
				{
					Code = EXAPATCH_EA_ERROR;
				}
			}
		}
	}
	for (i=0; i<3 ; i++ )
	{
		if (Specs[i])
		{
			ExaMemFree( NULL, Specs[i] );
		}
		if (NameBuffers[i])
		{
			ExaMemFree( NULL, NameBuffers[i] );
		}
	}
	if (Vals)
	{
		ExaMemFree( NULL, Vals );
	}
	if (OldFile)
	{
		ExaMemFree( NULL, OldFile );
		eaGetAllCleanup( &EAD );
	}
	if (NewFile)
	{
		ExaMemFree( NULL, NewFile );
	}
	return(Code);
}
#  endif
# ifdef EXAPATCH_OWNERGROUP_SUPPORT
static int DoOG( ExaPatchApplyFileManipulation * pManip, ExaPatchExtendedHeader * pEH)
{
	ExaPatchApplyFileManipulation * pTheManip = pManip;
	unsigned char * Ptr = pEH->HeaderBlock;
	unsigned char OwnerType;
	void * OwnerDatum;
	DWORD dwOwner = 0xffffffff;
	unsigned char GroupType;
	void * GroupDatum;
	DWORD dwGroup = 0xffffffff;
	DWORD dwRetrievedGroup = 0xffffffff;
	int Code = EXAPATCH_SUCCESS;

	OwnerType = Ptr[0] & 0xf;
	GroupType = (Ptr[0] & 0xf0) >> 4;
	Ptr++;
	OwnerDatum = (void *)&Ptr[1];
	GroupDatum = (void *)&Ptr[1+(Ptr[0])];

	while (pTheManip && (Code == EXAPATCH_SUCCESS))
	{
		dwOwner = 0xffffffff;
		dwGroup = 0xffffffff;
		dwRetrievedGroup = 0xffffffff;
		switch (OwnerType)
		{
			case 0:
				//default;
				break;
			case 1:
				//retain;
				if (pTheManip->OldFile)
				{
					Code = ExaGetOwnerGroupNum( pTheManip->OldFile, &dwOwner, &dwRetrievedGroup );
				}
				break;
			case 2:
				//set numeric WORD
				Ptr = (unsigned char *) OwnerDatum;
				dwOwner = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8);
				break;
			case 3:
				//set numeric DWORD
				Ptr = (unsigned char *) OwnerDatum;
				dwOwner = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8)
					+ (((DWORD)(Ptr[2] & 0xff)) << 16) + (((DWORD)(Ptr[3] & 0xff)) << 24);
				break;
			default:
				Code = EXAPATCH_NOT_SUPPORTED;
				break;
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			switch (GroupType)
			{
				case 0:
					//default;
					break;
				case 1:
					//retain;
					if (pTheManip->OldFile)
					{
						if (OwnerType == 1)
						{
							dwGroup = dwRetrievedGroup;
						}
						else
						{
							Code = ExaGetOwnerGroupNum( pTheManip->OldFile, NULL, &dwGroup );
						}
					}
					break;
				case 2:
					//set numeric WORD
					Ptr = (unsigned char *) GroupDatum;
					dwGroup = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8);
					break;
				case 3:
					//set numeric DWORD
					Ptr = (unsigned char *) GroupDatum;
					dwGroup = ((DWORD) (Ptr[0] & 0xff)) + (((DWORD) (Ptr[1] & 0xff)) << 8)
						+ (((DWORD)(Ptr[2] & 0xff)) << 16) + (((DWORD)(Ptr[3] & 0xff)) << 24);
					break;
				default:
					Code = EXAPATCH_NOT_SUPPORTED;
					break;
			}
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = ExaSetOwnerGroupNum( pTheManip->IntFile, dwOwner, dwGroup );
		}
		pTheManip = pTheManip->NextManip;
	}
	return(Code);
}
# endif
# if defined( EXAPATCH_REGISTRY_SUPPORT ) || defined( EXAPATCH_ARCHIVE_SUPPORT )
// 0 indicates success - any nonzero return code from a callback
//	is passed back up the line and returned.  local errors (I/O or memory) result in
//	a return code of EXAPATCH_UNSPECIFIED_ERROR (directory read error), EXAPATCH_FILE_NOT_FOUND 
//  or EXAPATCH_OUT_OF_MEMORY
// flags: 0x1 -> traverse symlinks (including junctions/mount points/HSM/SIS)
//				0x2 -> I/O or memory error only aborts current directory (not entire walk)
static int DirRecurse( DWORD dwFlags,
											wchar_t * Directory, 
											int (*ProcessFile)(void *, wchar_t *),
											int (*PreProcessDir)(void *,wchar_t *),
											int (*PostProcessDir)(void *,wchar_t *),
											void * Object)
{
# ifdef _WIN32
	wchar_t * NameBuf;
	wchar_t * Ptr;
	HANDLE hFind;
	int RetCode = 0;
	WIN32_FIND_DATAW Find;
	size_t BufferSize = 0;
	size_t SizeNeeded, Size0;
	int Code;

	Size0 = PSwcslen(Directory);
	BufferSize = Size0 + 3;
	if (BufferSize < 260)
	{
		BufferSize = 260;
	}
	NameBuf = malloc(sizeof(wchar_t)*BufferSize);
	if (NameBuf)
	{
		wcscpy_s(NameBuf,BufferSize,Directory);
		Ptr = NameBuf + Size0;
		if (Ptr[-1] == L'\\')
		{
			Ptr--;
			*Ptr = L'\0';
			Size0--;
		}
		wcscat_s(NameBuf,BufferSize,L"\\*");
		Ptr++;
		Size0++;
		hFind = FindFirstFileW( NameBuf, &Find );
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (PreProcessDir)
			{
				RetCode = (*PreProcessDir)(Object,Directory);
				if (RetCode)
				{
					FindClose( hFind );
					free( NameBuf );
					return RetCode;
				}
			}
			Code = -1;
			while (Code)
			{
				if (PSwcscmp(Find.cFileName,L".") && PSwcscmp(Find.cFileName,L".."))
				{
					SizeNeeded = Size0 + PSwcslen(Find.cFileName) + 1;
					if (BufferSize < SizeNeeded)
					{
						wchar_t * OldNB = NameBuf;

						BufferSize = SizeNeeded;
						NameBuf = realloc( NameBuf, sizeof(wchar_t)*BufferSize );
						if (NameBuf == NULL)
						{
							FindClose( hFind );
							free( OldNB );
							return((dwFlags & 0x2)?0:EXAPATCH_OUT_OF_MEMORY);
						}
						Ptr = NameBuf + Size0;
					}
					wcscpy_s( Ptr, BufferSize - Size0, Find.cFileName );
					if (Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if ((dwFlags & 0x1) || (0 == (Find.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)))
						{
							RetCode = DirRecurse( dwFlags, NameBuf, ProcessFile, PreProcessDir, PostProcessDir, Object );
							if (RetCode != 0)
							{
								Code = 0;
							}
						}
					}
					else
					{
						if (ProcessFile)
						{
							RetCode = (*ProcessFile)(Object, NameBuf);
							if (RetCode != 0)
							{
								Code = 0;
							}
						}
					}
				}
				if (Code)
				{
					Code = FindNextFileW( hFind, &Find );
					if (!Code)
					{
						RetCode = (dwFlags & 0x2)?0:EXAPATCH_UNSPECIFIED_ERROR;
						if (GetLastError() == ERROR_NO_MORE_FILES)
						{
							FindClose( hFind );
							free( NameBuf );
							if (PostProcessDir)
							{
								RetCode = (*PostProcessDir)(Object, Directory);
								return RetCode;
							}
							return 0;
						}
					}
				}
			}
			FindClose( hFind );
			free( NameBuf );
			return RetCode;
		}
		else
		{
			free( NameBuf );
			return((dwFlags & 0x2)?0:EXAPATCH_FILE_NOT_FOUND);
		}
	}
	return((dwFlags & 0x2)?0:EXAPATCH_OUT_OF_MEMORY);
# else
	char * NameBuf;
	wchar_t * WNameBuf;
	char * Ptr;
	DIR *d;
	int RetCode = 0;
	struct dirent *p;
	struct stat statbuf;
	size_t BufferSize = 0;
	size_t WBufferSize = 0;
	size_t SizeNeeded, Size0;
	size_t WSizeNeeded;
	int Code;

	Size0 = PSwcslen(Directory);
	BufferSize = 4 + sizeof(wchar_t)*Size0;
	WBufferSize = 300 + Size0;
	NameBuf = malloc(BufferSize);
	WNameBuf = malloc(sizeof(wchar_t)*WBufferSize);
	if (NameBuf && WNameBuf)
	{
		PSwcstombs(NameBuf,Directory,BufferSize);
		d = opendir(NameBuf);
		Ptr = NameBuf + Size0;
		Size0++;
		*Ptr++ = L'/';
		*Ptr = L'\0';
		if (d)
		{
			if (PreProcessDir)
			{
				RetCode = (*PreProcessDir)(Object,Directory);
				if (RetCode)
				{
					closedir(d);
					free( NameBuf );
					free( WNameBuf );
					return RetCode;
				}
			}
			Code = -1;
			while (Code && (p=readdir(d)))
			{
				if (strcmp(p->d_name,".") && strcmp(p->d_name,".."))
				{
					SizeNeeded = Size0 + strlen(p->d_name) + 1;
					if (BufferSize < SizeNeeded)
					{
						char * OldNB = NameBuf;

						BufferSize = SizeNeeded;
						NameBuf = realloc( NameBuf, BufferSize );
						if (NameBuf == NULL)
						{
							closedir(d);
							free( OldNB );
							free( WNameBuf );
							return((dwFlags & 0x2)?0:EXAPATCH_OUT_OF_MEMORY);
						}
						Ptr = NameBuf + Size0;
					}
					//@@CACING PENTEST strcpy( Ptr, p->d_name );
					strncpy( Ptr, p->d_name,(int)strlen(p->d_name) );
					if (!lstat(NameBuf,&statbuf))
					{
						WSizeNeeded = SizeNeeded;
						if (WBufferSize < WSizeNeeded)
						{
							wchar_t * OldWNB = WNameBuf;

							WBufferSize = WSizeNeeded;
							WNameBuf = realloc( WNameBuf, WBufferSize*sizeof(wchar_t) );
							if (WNameBuf == NULL)
							{
								closedir(d);
								free( OldWNB );
								free( NameBuf );
								return((dwFlags & 0x2)?0:EXAPATCH_OUT_OF_MEMORY);
							}
						}
						PSmbstowcs( WNameBuf, NameBuf, WBufferSize );
						if (S_IFDIR & statbuf.st_mode)
						{
							if ((dwFlags & 0x1) || (0 == (S_IFLNK & statbuf.st_mode)))
							{
								RetCode = DirRecurse( dwFlags, WNameBuf, ProcessFile, PreProcessDir, PostProcessDir, Object );
								if (RetCode != 0)
								{
									Code = 0;
								}
							}
						}
						else
						{
							if (ProcessFile)
							{
								RetCode = (*ProcessFile)(Object, WNameBuf);
								if (RetCode != 0)
								{
									Code = 0;
								}
							}
						}
					}
					else
					{
						// lstat failed
						closedir(d);
						free( NameBuf );
						free( WNameBuf );
						return((dwFlags & 0x2)?0:EXAPATCH_UNSPECIFIED_ERROR);
					}
				}
			}
			closedir( d );
			free( NameBuf );
			free( WNameBuf );
			if (Code)
			{
				if (PostProcessDir)
				{
					RetCode = (*PostProcessDir)(Object, Directory);
					return RetCode;
				}
				return 0;
			}
			return RetCode;
		}
		else
		{
			free( NameBuf );
			free( WNameBuf );
			return((dwFlags & 0x2)?0:EXAPATCH_FILE_NOT_FOUND);
		}
	}
	if (NameBuf)
	{
		free( NameBuf );
	}
	if (WNameBuf)
	{
		free( WNameBuf );
	}
	return((dwFlags & 0x2)?0:EXAPATCH_OUT_OF_MEMORY);
# endif
}
static int RmFile( void * Obj, wchar_t * FileName )
{
	return ExaDelete( FileName );
}
static int RmDir( void * Obj, wchar_t * DirName )
{
	return ExaRmDir( DirName );
}
static int rrm( wchar_t * Directory )
{
	return DirRecurse(0,Directory,&RmFile,NULL,&RmDir,NULL);
}
# ifdef EXAPATCH_REGISTRY_SUPPORT
typedef struct _CpObj {
	wchar_t * DestDir;
	size_t DestDirSize;
	size_t SrcDirSize;
	wchar_t * Buffer;
	size_t BufferSize;
	int (CALLBACK *ProgressCallBack)(DWORD ID,
										LPVOID Ptr, PLATWORD Handle);
	PLATWORD CallbackHandle;
	wchar_t ** pTopDir;
} CpObj;
static int CpFile( void * Obj, wchar_t * FileName )
{
	CpObj * pObj;
	size_t DestSize;

	pObj = (CpObj *) Obj;
	DestSize = PSwcslen(FileName) + 1 + pObj->DestDirSize - pObj->SrcDirSize;
	if (DestSize > pObj->BufferSize)
	{
		pObj->BufferSize = DestSize;
		pObj->Buffer = realloc( pObj->Buffer, DestSize * sizeof(wchar_t) );
		if (pObj->Buffer == NULL)
		{
			return EXAPATCH_OUT_OF_MEMORY;
		}
	}
	wcscpy_s(pObj->Buffer, pObj->BufferSize, pObj->DestDir);
	wcscpy_s(pObj->Buffer+pObj->DestDirSize, pObj->BufferSize - pObj->DestDirSize, FileName + pObj->SrcDirSize);
	return ExaCopyFile( FileName, pObj->Buffer, TRUE, FALSE, TRUE );
}
static int CpDir( void * Obj, wchar_t * DirName )
{
	CpObj * pObj;
	size_t DestSize;

	pObj = (CpObj *) Obj;
	DestSize = PSwcslen(DirName) + 1 + pObj->DestDirSize - pObj->SrcDirSize;
	if (DestSize > pObj->BufferSize)
	{
		pObj->BufferSize = DestSize;
		pObj->Buffer = realloc( pObj->Buffer, DestSize * sizeof(wchar_t) );
		if (pObj->Buffer == NULL)
		{
			return EXAPATCH_OUT_OF_MEMORY;
		}
	}
	wcscpy_s(pObj->Buffer, pObj->BufferSize, pObj->DestDir);
	wcscpy_s(pObj->Buffer+pObj->DestDirSize, pObj->BufferSize - pObj->DestDirSize, DirName + pObj->SrcDirSize);
	if (pObj->ProgressCallBack)
	{
		int Code;

		Code = (*pObj->ProgressCallBack)(EXP_PATCH_SEARCHING, L"DESTKEY", pObj->CallbackHandle);
		if (Code)
		{
			return(EXAPATCH_USER_CANCEL);
		}
	}
	return ExaMakeDirEx( pObj->Buffer, pObj->pTopDir );
}
static int rcp( wchar_t * SrcDir, wchar_t * DestDir,
							int (CALLBACK *ProgressCallBack)(DWORD ID,
										LPVOID Ptr, PLATWORD Handle),
							PLATWORD CallbackHandle,
							wchar_t ** pTopDir)
{
	CpObj Obj;
	int Code;

	Obj.DestDir = DestDir;
	Obj.DestDirSize = PSwcslen(DestDir);
	Obj.SrcDirSize = PSwcslen(SrcDir);
	Obj.BufferSize = 260;
	Obj.Buffer = malloc(Obj.BufferSize * sizeof(wchar_t) );
	Obj.ProgressCallBack = ProgressCallBack;
	Obj.CallbackHandle = CallbackHandle;
	Obj.pTopDir = pTopDir;
	if (Obj.Buffer)
	{
		Code = DirRecurse(0,SrcDir, &CpFile,&CpDir,NULL,(void *)&Obj);
		free( Obj.Buffer );
		return Code;
	}
	return EXAPATCH_OUT_OF_MEMORY;
}

static int DestKeyHandling( ExaPatchApplyFileHeaderInfo * HeaderPtr,
						  wchar_t ** pApplyDir,
							int (CALLBACK *ProgressCallBack)(DWORD ID,
										LPVOID Ptr, PLATWORD Handle),
							PLATWORD CallbackHandle)
{
	DWORD i,j;
	int Found = 0;
	int HdrPresent = -1;
	int Code = EXAPATCH_SUCCESS;
	wchar_t * FullSource = NULL;
	wchar_t * FullDest = NULL;
	DWORD dwSize;
	ExaPatchExtendedHeader * pEH;
	int nKeyType;
	DWORD dwKeyBase;
	wchar_t * lpKeyParm1;
	wchar_t * lpKeyParm2;
	wchar_t * lpKeyParm3;
	wchar_t * DestDir;
	wchar_t ** KPArray[3] = {&lpKeyParm1,&lpKeyParm2,&lpKeyParm3};
	char * EHPtr;

	if ((HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_UNDO) || (HeaderPtr->Comm.dwFileFlags & EXP_FILE_DESTKEY_LATE_COPY))
	{
		if ((0 == (HeaderPtr->Comm.dwReserved & 1)) || (HeaderPtr->dwExtensionSize < DESTKEY_SIZE))
		{
			// header extension needed but not supplied
			HdrPresent = 0;
		}
		else
		{
			HeaderPtr->DKApplyDir = HeaderPtr->DKRmDir = NULL;
		}
	}
	for (i=0; i<HeaderPtr->Comm.dwNumEHBlocks; i++ && !Found)
	{
		if (HeaderPtr->Comm.EHBlockPtr[i].ID == DESTKEY_EH_SIG)
		{
			pEH = HeaderPtr->Comm.EHBlockPtr + i;
			Found = -1;
		}
	}
	if (!Found)
		return EXAPATCH_SUCCESS;
	if (!HdrPresent)
	{
		return(EXAPATCH_NOT_SUPPORTED);
	}
	HeaderPtr->Comm.dwFileFlags |= 0x2000; // mark that DESTKEY is active
	if (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_UNDO)
	{
		HeaderPtr->Comm.dwFileFlags |= 0x4000; // mark that destination should be removed on error
	}
	if (HeaderPtr->Comm.dwFileFlags & EXP_FILE_DESTKEY_LATE_COPY)
	{
		Code = ExaMemAlloc( NULL, sizeof(wchar_t)*(1+PSwcslen(*pApplyDir)),&HeaderPtr->DKApplyDir ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			PSwcscpy( HeaderPtr->DKApplyDir, *pApplyDir );
		}
	}
	HeaderPtr->Comm.dwGlobalFlags &= ~EXP_GLOBAL_UNDO;
	HeaderPtr->Comm.dwFileFlags &= ~EXP_FILE_BACKUP;
	// parse EH
	EHPtr = (char *)pEH->HeaderBlock;
	nKeyType = *EHPtr++;
	EHPtr++;
	dwKeyBase = (((DWORD)EHPtr[1]) << 8) + ((DWORD) EHPtr[0]);
	EHPtr += 2;
	for (i=0; i<3; i++ && (Code == EXAPATCH_SUCCESS))
	{
		dwSize = (((DWORD)EHPtr[1]) << 8) + ((DWORD) EHPtr[0]);
		EHPtr+=2;
		if (dwSize)
		{
			Code = ExaMemAlloc( NULL, (1+dwSize)*sizeof(wchar_t), (void **) KPArray[i] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code == EXAPATCH_SUCCESS)
			{
				for (j=0;j<dwSize;j++)
				{
					(*KPArray[i])[j] = (((wchar_t)EHPtr[1]) << 8) + ((wchar_t) EHPtr[0]);
# ifndef _WIN32
					if ((*KPArray[i])[j] == L'\\')
					{
						(*KPArray[i])[j] = L'/';
					}
# endif
					EHPtr += 2;
				}
				(*KPArray[i])[dwSize] = L'\0';
			}
		}
		else
		{
			*KPArray[i] = NULL;
		}
	}
	// get dest directory
	if (Code == EXAPATCH_SUCCESS)
	{
		Code = ExaPatchFindKeyDir( &DestDir,
					nKeyType,
					dwKeyBase,
					lpKeyParm1,
					lpKeyParm2,
					lpKeyParm3);
		// make sure dest directory doesn't exist
		Code = ExaFullyQualify( *pApplyDir, &FullSource );
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = ExaFullyQualify( DestDir, &FullDest );
			if (Code == EXAPATCH_SUCCESS)
			{
				// make sure Dest isn't a subdir of source
				wchar_t * SrcPtr = FullSource;
				wchar_t * DestPtr = FullDest;

# ifdef _WIN32
				while( towupper(*SrcPtr) == towupper(*DestPtr))
# else
				while( *SrcPtr == *DestPtr)
# endif
				{
					SrcPtr++;
					DestPtr++;
				}
				if ((!*SrcPtr) && (*DestPtr == PATH_DELIMITER))
				{
					Code = EXAPATCH_INVALID_PARAMETER;
				}
			}
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			if (EXAPATCH_FILE_NOT_FOUND == ExaExists(DestDir))
			{
				// dest directory doesn't exist
				// copy files and reset apply directory
				{
					if (HeaderPtr->Comm.dwFileFlags & EXP_FILE_DESTKEY_LATE_COPY)
					{
						Code = ExaMakeDirEx( FullDest, &HeaderPtr->DKRmDir );
					}
					else
					{
						Code = rcp( FullSource, FullDest, ProgressCallBack, CallbackHandle, &HeaderPtr->DKRmDir );
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						ExaMemFree( NULL, *pApplyDir );
						*pApplyDir = DestDir;
					}
				}
			}
			else if (HeaderPtr->Comm.dwFileFlags & EXP_FILE_DESTKEY_MAY_EXIST)
			{
				// dest directory exists and we've been told that's OK
				if (HeaderPtr->Comm.dwFileFlags & EXP_FILE_DESTKEY_ALWAYS_COPY)
				{
					// force copy anyway
					if (0 == (HeaderPtr->Comm.dwFileFlags & EXP_FILE_DESTKEY_LATE_COPY))
					{
						Code = rcp( FullSource, FullDest, ProgressCallBack, CallbackHandle, NULL );
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						ExaMemFree( NULL, *pApplyDir );
						*pApplyDir = DestDir;
					}
				}
				else
				{
					// just reset apply directory and reset late copy bit
					HeaderPtr->Comm.dwFileFlags &= ~EXP_FILE_DESTKEY_LATE_COPY;
					ExaMemFree( NULL, *pApplyDir );
					*pApplyDir = DestDir;
				}
			}
			else
			{
				Code = EXAPATCH_INVALID_PARAMETER;
			}
		}
	}
	if (FullDest)
	{
		ExaMemFree( NULL, FullDest );
	}
	if (FullSource)
	{
		ExaMemFree( NULL, FullSource );
	}
	return Code;
}
static int DestKeyHandling2( ExaPatchApplyFileHeaderInfo * HeaderPtr,
							wchar_t ** pApplyDir,
							int (CALLBACK *ProgressCallBack) (DWORD ID,
								LPVOID Ptr, PLATWORD Handle),
							PLATWORD CallbackHandle )
{
	wchar_t * FullSource = NULL;
	wchar_t * FullDest = NULL;
	int Code = EXAPATCH_SUCCESS;
	if (HeaderPtr->Comm.dwFileFlags & 0x2000)
	{
		if (HeaderPtr->Comm.dwFileFlags & EXP_FILE_DESTKEY_LATE_COPY)
		{
			Code = ExaFullyQualify( HeaderPtr->DKApplyDir, &FullSource );
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = ExaFullyQualify( *pApplyDir, &FullDest );
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = rcp( FullSource, FullDest, ProgressCallBack, CallbackHandle, NULL );
				}
			}
		}
	}
	if (FullDest)
	{
		ExaMemFree( NULL, FullDest );
	}
	if (FullSource)
	{
		ExaMemFree( NULL, FullSource );
	}
	return Code;
	
}
#   endif /* EXAPATCH_REGISTRY_SUPPORT */
#  endif /* EXAPATCH_REGISTRY_SUPPORT || EXAPATCH_ARCHIVE_SUPPORT */
# if defined( EXAPATCH_ARCHIVE_SUPPORT ) || defined( EXAPATCH_LOCKDIR_SUPPORT )
/*
Archive start and subdirectory entries are handled by queueing them up - entries
are lex-sorted: magic # (incr), start/subdir
Then, the queue is processed in order.
*/
static DWORD GetLEDword( unsigned char * Ptr )
{
	DWORD retval=0;
	int i;

	for (i=3; i>=0 ; i-- )
	{
		retval = (retval << 8) + ((DWORD) Ptr[i]);
	}
	return(retval);
	
}
static SQWORD GetLESQword( unsigned char * Ptr )
{
	SQWORD retval = 0;
	int i;
	for (i=7; i >= 0 ; i-- )
	{
		retval = (retval << 8) + (SQWORD)Ptr[i];
	}
	return(retval);
}
static int GetLEwchar( unsigned char * Ptr, wchar_t ** ppWc )
{
	DWORD dwCount;
	int Code;
	DWORD i;

	dwCount = ((DWORD) Ptr[0]) + (((DWORD) Ptr[1]) << 8);
	Code = ExaMemAlloc( NULL, sizeof(wchar_t)*(dwCount + 1), (void **) ppWc); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (Code == EXAPATCH_SUCCESS)
	{
		for (i=0; i< dwCount; i++ )
		{
			(*ppWc)[i] = ((wchar_t) Ptr[2+(2*i)]) + (((wchar_t) Ptr[3+(2*i)])<<8);
		}
		(*ppWc)[dwCount] = L'\0';
	}
	return(Code);
}
# endif
# ifdef EXAPATCH_ARCHIVE_SUPPORT
static int CompareQE(PArchiveQueueEntry pQE1, PArchiveQueueEntry pQE2)
{
	if (pQE1->dwMagic != pQE2->dwMagic)
	{
		if (pQE1->dwMagic < pQE2->dwMagic)
		{
			return(-1);
		}
		else
		{
			return(1);
		}
	}
	else
	{
		if (pQE1->dwType < pQE2->dwType)
		{
			return(-1);
		}
		else if (pQE1->dwType > pQE2->dwType)
		{
			return(1);
		}
	}
	return(0);
}
static int ArchiveAddToQueue(ExaPatchApplyFileHeaderInfo * HeaderPtr, PArchiveQueueEntry pQE )
{
	PArchiveQueueEntry pThisQE;

	pThisQE = ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQHead;
	while (pThisQE && (-1 == CompareQE(pThisQE, pQE)))
	{
		pThisQE = pThisQE->pNext;
	}
	if (pThisQE)
	{
		// insert between pThisQE->pPrev and pThisQE
		pQE->pPrev = pThisQE->pPrev;
		pQE->pNext = pThisQE;
		if (pThisQE->pPrev)
		{
			pThisQE->pPrev->pNext = pQE;
		}
		else
		{
			((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQHead = pQE;
		}
		pThisQE->pPrev = pQE;
	}
	else
	{
		// insert at tail
		pQE->pPrev = ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQTail;
		pQE->pNext = NULL;
		if (pQE->pPrev)
		{
			pQE->pPrev->pNext = pQE;
		}
		else
		{
			((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQHead = pQE;
		}
		((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQTail = pQE;
	}
	return(EXAPATCH_SUCCESS);
}
static int ArchiveStart(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, ExaPatchExtendedHeader * EHBlockPtr)
{
	int Code = EXAPATCH_SUCCESS;
	if (
#   ifdef EXAPATCH_LIST_SUPPORT
			(0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   else
			(1)
#   endif
#   ifdef EXAPATCH_ONEFILE_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY))
#   endif
			)
	{
		if ((HeaderPtr->Comm.dwReserved & 1)
			&& (HeaderPtr->dwExtensionSize >= ARCHIVE_SIZE))
		{
			// actually do archive start (queue entry)
			PArchiveQueueEntry pQE = NULL;
			unsigned char * Ptr;
			unsigned char Flag;

			Code = ExaMemAlloc( NULL, sizeof( ArchiveQueueEntry ), (void **) &pQE ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code == EXAPATCH_SUCCESS)
			{
				memset( pQE, 0, sizeof( ArchiveQueueEntry ) );
				Ptr = (unsigned char *) EHBlockPtr->HeaderBlock;
				Flag = *Ptr++;
				pQE->dwMagic = GetLEDword( Ptr );
				pQE->nSubdir = -1;
				pQE->nSystem = -1;
				pQE->dwOwner = 0xffffffff;
				pQE->dwGroup = 0xffffffff;
				pQE->dwPermissions = 0xffffffff;
				if (EHBlockPtr->ID == ARCH_START2_BLOCKID_LOCAL)
				{
					pQE->dwFlags |= 1;
				}
				else if (EHBlockPtr->ID == ARCH_START3_BLOCKID_LOCAL)
				{
					pQE->dwFlags |= 2;
				}
				Ptr += 4;
				if (Flag & 1)
				{
					pQE->nSubdir = GetLESQword( Ptr );
					Ptr += 8;
				}
				if (Flag & 2)
				{
					pQE->nSystem = GetLESQword( Ptr );
					Ptr += 8;
				}
				if (Flag & 4)
				{
					pQE->dwOwner = GetLEDword( Ptr );
					Ptr += 4;
					pQE->dwGroup = GetLEDword( Ptr );
					Ptr += 4;
					pQE->dwPermissions = GetLEDword( Ptr );
					Ptr += 4;
				}
				Code = GetLEwchar( Ptr, &pQE->pName );
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = ArchiveAddToQueue( HeaderPtr, pQE );
				}
			}
			if (Code)
			{
				if (pQE && pQE->pName)
				{
					ExaMemFree( NULL, pQE->pName );
					pQE->pName = NULL;
				}
				if (pQE)
				{
					ExaMemFree( NULL, pQE );
					pQE = NULL;
				}
			}
		}
		else
		{
			// error since archive support is needed, but the structure wasn't provided for it
			return EXAPATCH_NOT_SUPPORTED;
		}
	}
	return(Code);
}
static int ArchiveSubdir(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, ExaPatchExtendedHeader * EHBlockPtr)
{
	int Code = EXAPATCH_SUCCESS;
	if (
#   ifdef EXAPATCH_LIST_SUPPORT 
			(0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   else
			(1)
#   endif
#   ifdef EXAPATCH_ONEFILE_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY))
#   endif
			)
	{
		// actually do archive subdir (queue entry)
		PArchiveQueueEntry pQE = NULL;
		unsigned char * Ptr;

		Code = ExaMemAlloc( NULL, sizeof( ArchiveQueueEntry ), (void **) &pQE ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			memset( pQE, 0, sizeof( ArchiveQueueEntry ) );
			pQE->dwType = 1;
			Ptr = (unsigned char *) EHBlockPtr->HeaderBlock;
			pQE->dwMagic = GetLEDword( Ptr );
			Ptr += 4;
			Code = GetLEwchar( Ptr, &pQE->pName );
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = ArchiveAddToQueue( HeaderPtr, pQE );
			}
		}
		if (Code)
		{
			if (pQE && pQE->pName)
			{
				ExaMemFree( NULL, pQE->pName );
				pQE->pName = NULL;
			}
			if (pQE)
			{
				ExaMemFree( NULL, pQE );
				pQE = NULL;
			}
		}
	}
	return(Code);
}
static int ArchiveEnd(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, ExaPatchExtendedHeader * EHBlockPtr,
	int (CALLBACK *ProgressCallBack)(DWORD ID, LPVOID Ptr, PLATWORD Handle), PLATWORD CallbackHandle)
{
	int Code = EXAPATCH_SUCCESS;
	if (
#   ifdef EXAPATCH_LIST_SUPPORT 
			(0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   else
			(1)
#   endif
#   ifdef EXAPATCH_ONEFILE_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY))
#   endif
			)
	{
		// actually do archive end
		unsigned char * Ptr;
		DWORD dwMagic;
		ArchiveStack * pAS;
		ArchiveStack * pAS2;
		ExaPatchApplyFileManipulation * pSaveLocalHead, * pSaveLocalTail;
		INT Done = 0;

		Ptr = (unsigned char *) EHBlockPtr->HeaderBlock;
		dwMagic = GetLEDword( Ptr );
		pAS = ((PArchiveInfo) HeaderPtr->pArchiveInfo)->pStack;
		while (pAS && (dwMagic != pAS->dwMagic))
		{
			pAS = pAS->pPrev;
		}
		if (pAS)
		{
			pAS2 = ((PArchiveInfo) HeaderPtr->pArchiveInfo)->pStack;
			while ((Code == EXAPATCH_SUCCESS) && !Done)
			{
				void * Ptr;
				//ExaDelete( pAS2->TempArchName );
# ifdef EXAPATCH_VERIFYONLY_SUPPORT
				if (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_VERIFYONLY)
				{
					ExaDelete( pAS2->TempArchName );
					rrm( pAS2->NewApplyRoot );
					if (pAS2 == pAS)
					{
						Done = -1;
					}
					HeaderPtr->Comm.dwGlobalFlags = pAS2->dwPrevGlobalFlags;
					EntryPtr->Comm.dwFileFlags = pAS2->dwPrevFileFlags;
				}
				else
# endif				
				{
					wchar_t * NameSave;

					if (pAS2->dwFlags & 1)
					{
						if (ExaCompressExact( pAS2->NewApplyRoot, pAS2->nArchType, pAS2->TempArchName ))
						{
							Code = EXAPATCH_WRITE_FAILED;
						}
					}
					else
					{
						if (ExaCompress( pAS2->NewApplyRoot, pAS2->nArchType, pAS2->TempArchName ))
						{
							Code = EXAPATCH_WRITE_FAILED;
						}
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						rrm( pAS2->NewApplyRoot );
					}
					if (pAS2 == pAS)
					{
						Done = -1;
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						HeaderPtr->Comm.dwGlobalFlags = pAS2->dwPrevGlobalFlags;
						EntryPtr->Comm.dwFileFlags = pAS2->dwPrevFileFlags;
						// move to local list and use ExaPatchDoLocal to perform archive FileManip
						// (requires that Local already be emptied before this point)
						pSaveLocalHead = HeaderPtr->LocalManipList;
						pSaveLocalTail = HeaderPtr->LocalManipTail;
						HeaderPtr->LocalManipList = HeaderPtr->LocalManipTail = pAS2->pManip;
						Code = ExaMemAlloc( NULL, sizeof(wchar_t)*(PSwcslen(pAS2->NewArchName)+1), (void **) &NameSave );
						if (Code == EXAPATCH_SUCCESS)
						{
							memmove( NameSave, pAS2->NewArchName, sizeof(wchar_t)*(PSwcslen(pAS2->NewArchName)+1));
							Code = ExaPatchDoLocal(HeaderPtr, EntryPtr, ProgressCallBack, CallbackHandle);
							HeaderPtr->LocalManipList = pSaveLocalHead;
							HeaderPtr->LocalManipTail = pSaveLocalTail;
							if ((pAS2->dwOwner != 0xffffffff) && (pAS2->dwGroup != 0xffffffff))
							{
								ExaSetOwnerGroupNum( NameSave, pAS2->dwOwner, pAS2->dwGroup );
							}
							if (pAS2->dwPermissions != 0xffffffff)
							{
								ExaPatchFileAttrib Attrib;
								Attrib.Flags = EXP_ATTRIB_ATTRIBUTE;
								Attrib.Attributes = pAS2->dwPermissions;
								ExaSetFileAttrib( NameSave, &Attrib, NULL, 0 );
							}
							ExaMemFree( NULL, NameSave );
						}
					}
				}
				if (Code == EXAPATCH_SUCCESS)
				{
					pAS2->pManip = NULL;
					pAS2->OldArchName = pAS2->TempArchName = pAS2->NewArchName = NULL;
					if (pAS2->PrevApplyDir)
					{
						ExaMemFree( NULL, HeaderPtr->ActualApplyDir );
						pAS2->NewApplyRoot = NULL;
						HeaderPtr->ActualApplyDir = pAS2->PrevApplyDir;
					}
					if (pAS2->PrevSubDir)
					{
						if (((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir)
						{
							ExaMemFree( NULL, ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir );
						}
						((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir = pAS2->PrevSubDir;
						pAS2->PrevSubDir = NULL;
					}
					Ptr = (void *)pAS2;
					pAS2 = ((PArchiveInfo) HeaderPtr->pArchiveInfo)->pStack = pAS2->pPrev;
					ExaMemFree( NULL, Ptr );
				}
			}
		}
	}
	return(Code);
}
static int ArchiveProcess(ExaPatchApplyFileHeaderInfo * HeaderPtr, ExaPatchApplyFileEntryInfo * EntryPtr, PLATWORD Handle)
{
	int Code = EXAPATCH_SUCCESS;
# ifdef EXAPATCH_TEMPDIR_SUPPORT
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle; /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
# endif
	if ( (HeaderPtr->pArchiveInfo) &&
#   ifdef EXAPATCH_LIST_SUPPORT 
			(0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_LISTONLY))
#   else
			(1)
#   endif
#   ifdef EXAPATCH_ONEFILE_SUPPORT
			&& (0 == (HeaderPtr->ApplyFlags & EXP_PATCH_APPLY_ONEFILEONLY))
#   endif
			)
	{
		// actually do archive queue processing
		PArchiveQueueEntry pQE;

		while ((Code == EXAPATCH_SUCCESS) && ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQHead)
		{
			pQE = ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQHead;
			if (pQE->dwType)
			{
				// actually do archive subdir processing
				if (((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir)
				{
					ExaMemFree( NULL, ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir );
				}
				((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir = pQE->pName;
				pQE->pName = NULL;
			}
			else
			{
				// actually do archive start processing
				// make stack entry
				ArchiveStack * pAS;

				Code = ExaMemAlloc( NULL, sizeof(ArchiveStack), (void **) &pAS ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				if (Code == EXAPATCH_SUCCESS)
				{
					memset( pAS, 0, sizeof(ArchiveStack) );
					pAS->dwMagic = pQE->dwMagic;
					pAS->dwPrevGlobalFlags = HeaderPtr->Comm.dwGlobalFlags;
					pAS->dwPrevFileFlags = EntryPtr->Comm.dwFileFlags;
					pAS->PrevApplyDir = HeaderPtr->ActualApplyDir;
					pAS->PrevSubDir = ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir;
					pAS->dwOwner = pQE->dwOwner;
					pAS->dwGroup = pQE->dwGroup;
					pAS->dwPermissions = pQE->dwPermissions;
					// locate old archive (set pAS->OldArchName/TempArchName/NewArchName)
					{
						SQWORD nSaveSubDirIndex, nSaveSystemIndex;
						DWORD dwSaveNumOldFiles, dwSaveNumNewFiles;
						wchar_t ** lpSaveOldPathArray, ** lpSaveNewPathArray;
						DWORD SearchFlags, AttribFlags;
						wchar_t * SearchPath = NULL, * HintSubDir = NULL;
						DWORD CNCFlag = 0, NumOldMoved = 0, NumIntMoved = 0;
						ExaPatchApplyFileManipulation * pSaveLocalHead, * pSaveLocalTail;
						wchar_t * RenamePtr = NULL;
						wchar_t * RenameHelper;

						nSaveSubDirIndex = EntryPtr->Comm.nSubDirIndex;
						nSaveSystemIndex = EntryPtr->Comm.nSystemIndex;
						dwSaveNumOldFiles = EntryPtr->Comm.Files.dwNumOldFiles;
						dwSaveNumNewFiles = EntryPtr->Comm.Files.dwNumNewFiles;
						lpSaveOldPathArray = EntryPtr->Comm.Files.lpOldPathArray;
						lpSaveNewPathArray = EntryPtr->Comm.Files.lpNewPathArray;
						EntryPtr->Comm.nSubDirIndex = pQE->nSubdir;
						EntryPtr->Comm.nSystemIndex = pQE->nSystem;
						EntryPtr->Comm.Files.dwNumOldFiles = EntryPtr->Comm.Files.dwNumNewFiles = 1;
						EntryPtr->Comm.Files.lpOldPathArray = EntryPtr->Comm.Files.lpNewPathArray = &pQE->pName;
						RenamePtr = PSwcschr(pQE->pName,L'/');
						if (RenamePtr)
						{
							*RenamePtr = L'\0';
							RenameHelper = RenamePtr + 1;
							EntryPtr->Comm.Files.lpNewPathArray = &RenameHelper;
						}
						pSaveLocalHead = HeaderPtr->LocalManipList;
						pSaveLocalTail = HeaderPtr->LocalManipTail;
						HeaderPtr->LocalManipList = HeaderPtr->LocalManipTail = NULL;
						Code = SetSearchParms( HeaderPtr, EntryPtr, &SearchFlags, &AttribFlags, &SearchPath, &HintSubDir, FALSE );
						if (Code == EXAPATCH_SUCCESS)
						{
							SearchFlags &= ~(EXP_SEARCH_VERIFY_CKSUM | EXP_SEARCH_VERIFY_ATTRIB);
							AttribFlags = 0;
							Code = ExaPatchFileSearch( &pAS->OldArchName, pQE->pName,SearchPath, SearchFlags, 
								NULL, NULL, NULL, NULL, NULL, PATH_DELIMITER, PATH_SEPARATOR, NULL, 0, NULL, 0 );
							if (Code == EXAPATCH_SUCCESS)
							{
								Code = SetFileLoc( &pAS->OldArchName, &pAS->TempArchName, &pAS->NewArchName, NULL, 
									HeaderPtr, EntryPtr, EntryPtr, 1, 1, &CNCFlag, 1, EXP_STATUS_CONTENTS, 
									EntryPtr->Comm.dwFileFlags, 0, &NumOldMoved, &NumIntMoved, HintSubDir, Handle );
								if (Code == EXAPATCH_SUCCESS)
								{
									pAS->pManip = HeaderPtr->LocalManipList;
								}
							}
							else if (RenamePtr)
							{
								/* see if NEW file exists - use it if so without renaming
								This is essentially a mechanism to make sure the contents
								of the NEW file are correct
								 */
								Code = ExaPatchFileSearch( &pAS->OldArchName, RenameHelper, SearchPath, SearchFlags,
										NULL, NULL, NULL, NULL, NULL, PATH_DELIMITER, PATH_SEPARATOR, NULL, 0, NULL, 0 );
								if (Code == EXAPATCH_SUCCESS)
								{
									EntryPtr->Comm.Files.lpOldPathArray = &RenameHelper;
									Code = SetFileLoc( &pAS->OldArchName, &pAS->TempArchName, &pAS->NewArchName, NULL, 
										HeaderPtr, EntryPtr, EntryPtr, 1, 1, &CNCFlag, 1, EXP_STATUS_CONTENTS, 
										EntryPtr->Comm.dwFileFlags, 0, &NumOldMoved, &NumIntMoved, HintSubDir, Handle );
									if (Code == EXAPATCH_SUCCESS)
									{
										pAS->pManip = HeaderPtr->LocalManipList;
									}
								}
							}
						}
						if (RenamePtr)
						{
							*RenamePtr = L'/';
						}
						EntryPtr->Comm.nSubDirIndex = nSaveSubDirIndex;
						EntryPtr->Comm.nSystemIndex = nSaveSystemIndex;
						EntryPtr->Comm.Files.dwNumOldFiles = dwSaveNumOldFiles;
						EntryPtr->Comm.Files.dwNumNewFiles = dwSaveNumNewFiles;
						EntryPtr->Comm.Files.lpOldPathArray = lpSaveOldPathArray;
						EntryPtr->Comm.Files.lpNewPathArray = lpSaveNewPathArray;
						HeaderPtr->LocalManipList = pSaveLocalHead;
						HeaderPtr->LocalManipTail = pSaveLocalTail;
					}
					// make temp dir (set pAS->NewApplyRoot)
					if (Code == EXAPATCH_SUCCESS)
					{
# ifdef EXAPATCH_TEMPDIR_SUPPORT
						if (TheChannel->pTempDir 
							&& (NULL == ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack))
						{
							Code = ExaEZTemp( TheChannel->pTempDir, &pAS->NewApplyRoot );
						}
						else
# endif
						{
							Code = ExaEZTemp( HeaderPtr->ActualApplyDir, &pAS->NewApplyRoot );
						}
						if (Code == EXAPATCH_SUCCESS)
						{
							Code = ExaMakeDir( pAS->NewApplyRoot );
						}
					}
					// uncompress archive and set pAS->nArchType
					if (Code == EXAPATCH_SUCCESS)
					{
						pAS->nArchType = ExaIsArchive( pAS->OldArchName );
						if (pAS->nArchType <= 0)
						{
							pAS->nArchType = 0;
							Code = EXAPATCH_INVALID_FILE_FOUND;
						}
						else
						{
							if (pQE->dwFlags & 2)
							{
								pAS->dwFlags |= 1;
								if (ExaExpandExact( pAS->OldArchName, pAS->nArchType, pAS->NewApplyRoot ))
								{
									pAS->nArchType = 0;
									Code = EXAPATCH_INVALID_FILE_FOUND;
								}
							}
							else
							{
								if (ExaExpand( pAS->OldArchName, pAS->nArchType, pAS->NewApplyRoot ))
								{
									pAS->nArchType = 0;
									Code = EXAPATCH_INVALID_FILE_FOUND;
								}
							}
						}
					}
					if ((Code == EXAPATCH_SUCCESS) && (0 == (pQE->dwFlags & 1)))
					{
						// remove metadata if it's not being preserved
						DWORD dwBufSize;
						wchar_t * Buffer;

						dwBufSize = 1+1+14+(DWORD)PSwcslen(pAS->NewApplyRoot);
						Code = ExaMemAlloc( NULL, dwBufSize*sizeof(wchar_t), (void **) &Buffer );
						if (Code == EXAPATCH_SUCCESS)
						{
							wcscpy_s( Buffer, dwBufSize, pAS->NewApplyRoot );
# ifdef _WIN32
							wcscat_s( Buffer, dwBufSize, L"\\.arch.metadata" );
# else
							wcscat_s( Buffer, dwBufSize, L"/.arch.metadata" );
# endif
							ExaDelete( Buffer );
							ExaMemFree( NULL, Buffer );
						}
					}
					// setup stack entry
					if (Code == EXAPATCH_SUCCESS)
					{
						EntryPtr->Comm.dwFileFlags &= ~(EXP_FILE_BACKUP | EXP_FILE_PATHSEARCH | EXP_FILE_SUBDIRSEARCH | EXP_FILE_IGNOREMISSING);
						HeaderPtr->Comm.dwGlobalFlags &= ~(EXP_GLOBAL_UNDO | EXP_GLOBAL_CONFIRM | EXP_GLOBAL_RESILIENT );
						HeaderPtr->ActualApplyDir = pAS->NewApplyRoot;
						pAS->pPrev = ((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack;
						((PArchiveInfo)HeaderPtr->pArchiveInfo)->pStack = pAS;
						((PArchiveInfo)HeaderPtr->pArchiveInfo)->pCurSubdir = NULL;
					}
					else
					{
						// error cleanup
						if (pAS->PrevSubDir)
						{
							pAS->PrevSubDir = NULL;
						}
						if (pAS->NewApplyRoot)
						{
							rrm( pAS->NewApplyRoot );
							ExaMemFree( NULL, pAS->NewApplyRoot );
						}
						if (pAS->TempArchName)
						{
							ExaDelete( pAS->TempArchName );
						}
						if (pAS->pManip)
						{
							ExaPatchFreeManipList( &pAS->pManip, &pAS->pManip );
							pAS->OldArchName = pAS->TempArchName = pAS->NewArchName = NULL;
						}
						else
						{
							if (pAS->NewArchName)
							{
								ExaMemFree( NULL, pAS->NewArchName );
							}
							if (pAS->OldArchName)
							{
								ExaMemFree( NULL, pAS->OldArchName );
							}
							if (pAS->TempArchName)
							{
								ExaMemFree( NULL, pAS->TempArchName );
							}
						}
						ExaMemFree( NULL, pAS );
					}
				}
			}
			((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQHead = pQE->pNext;
			if (pQE->pName)
			{
				ExaMemFree( NULL, pQE->pName );
			}
			ExaMemFree( NULL, pQE );
		}
		((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQHead = NULL;
		((PArchiveInfo)HeaderPtr->pArchiveInfo)->pAQTail = NULL;
	}
	return(Code);
}
# endif
# endif /* EXAPATCH_EH_SUPPORT */
# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
int /* ErrCode */
	EXP_DLLIMPORT
	ExaPatchApplySupplyBuffer(
		PLATWORD Handle,
		void * Buffer,
		DWORD dwBufferSize
		)
{
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle; /* MISRA C 2012 [11.4, 11.6]: casting pointer to/from integer is used to provide an opaque handle to the caller. */
	intnt Code;

	Code = AcquireChannel( Handle );
	if (0 != Code)
	{
		return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}

	TheChannel->dwCBSize = dwBufferSize;
	TheChannel->pChannelBuffer = Buffer;
	(void)ReleaseChannel(Handle); /* MISRA C 2012 [2.2]: function lacks side-effects in this compilation configuration */
	return(Code);
}

# endif
static int MarkProgress( ExaPatchApplyFileHeaderInfo * HeaderPtr,
# ifdef ATTOPATCH
		const ExaPatchApplyFileEntryInfo * EntryPtr,
# else
		ExaPatchApplyFileEntryInfo * EntryPtr,
# endif
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		DWORD dwLocalStart,
		DWORD dwLocalDelta
 )
{
	int Code = EXAPATCH_SUCCESS;
	if (ProgressCallBack)
	{
		DWORD Progress[2];

		Progress[0] = (DWORD)(((1 + EntryPtr->qwEntryNum)<<15)/HeaderPtr->Comm.qwNumEntries);
		Progress[1] = dwLocalStart + dwLocalDelta;
		Code = (*ProgressCallBack)(EXP_PATCH_PROGRESS,(void *) &Progress[0],CallbackHandle);
		if (Code == EXP_CALLBACK_ABORT)
		{
			Code = EXAPATCH_USER_CANCEL;
		}
		else
		{
			Code = EXAPATCH_SUCCESS;
		}
	}
	return(Code);
}

int /* ErrCode */
	EXP_DLLIMPORT
	ExaPatchApplySetTempDir(
		PLATWORD Handle,
		WCHAR * TempDir
		)
# ifdef EXAPATCH_TEMPDIR_SUPPORT
{
	int Code;
	ExaPatchApplyChannel * TheChannel = (ExaPatchApplyChannel *) Handle;
	WCHAR * OldDir;

	Code = AcquireChannel( Handle );
	if (0 != Code)
	{
		return(Code);
	}
	if (EXAPATCH_SUCCESS == ExaDirExists(TempDir))
	{
		OldDir = TheChannel->pTempDir;
		Code = ExaFullyQualify( TempDir, &TheChannel->pTempDir );
		if (Code == EXAPATCH_SUCCESS)
		{
			if (OldDir != NULL)
			{
				ExaMemFree( NULL, OldDir );
			}
		}
	}
	else
	{
		Code = EXAPATCH_INVALID_PARAMETER;
	}
	ReleaseChannel( Handle );
	return(Code);
}
# else
{
	return(EXAPATCH_NOT_SUPPORTED);
}
# endif
