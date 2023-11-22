/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXAMEM.C                                                           |
|                                                                           |
|                                                                           |
|  RTPatch Server Memory Routines                                           |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2010.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/
/*lint --e{950, 9026, 9024, 715, 818, 904} */
/* Suppress non-ANSI, function-type macro, ## macro, unused parameter, early return, warnings in this file (all deviations are marked) */
# define EXAPATCH_INTERNAL_CODE
# include "exaplat.h"
# include "experror.h"
# include "exputils.h"
/*# define MEM_TRACE */
# ifdef MEM_TRACE
# include <stdio.h>
# endif

/*
 AllocBig routines now allocate one trailing guard word <sigh>
  it's *much* easier than fixing the fact that the efficient GET3
  accesses one byte more than it should...
*/

/*# define EXAMEM_DEBUG */

# ifndef EXAMEM_DEBUG

# define EZ_EXAMEM

# ifdef EZ_EXAMEM

# ifndef _WIN32
# define EXAMEM_SIGNATURE 0xeaaccaae
# endif
# ifdef MEM_TRACE
static PLATWORD CurMemSize;
static PLATWORD MaxMemSize;
# endif
int ExaMemInit( PLATWORD InitSize, HANDLE * HandlePtr, unsigned int SerializeFlag ) /* MISRA C 2012 [2.7]: SerializeFlag and InitSize are unused with this set of compilation flags */
{
	*HandlePtr = NULL;
# ifdef MEM_TRACE
	CurMemSize = 0;
	MaxMemSize = 0;
# endif
	return(EXAPATCH_SUCCESS);
}

int ExaMemAlloc( HANDLE Handle, PLATWORD Size, void * * PtrPtr ) /* MISRA C 2012 [2.7, 8.13]: Handle is unused (and could be const) with this set of compilation flags */
{
# ifdef _WIN32
	*PtrPtr = (void *) GlobalAlloc( GMEM_FIXED, Size );
	if (*PtrPtr == NULL)
	{
		return(EXAPATCH_OUT_OF_MEMORY); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
# ifdef MEM_TRACE
	CurMemSize += Size;
	if (MaxMemSize < CurMemSize)
	{
		MaxMemSize = CurMemSize;
		printf("MM = %x\n",MaxMemSize);
	}
# endif
	return(EXAPATCH_SUCCESS);
# else
	char * Ptr;

	Ptr = malloc( Size + 2*sizeof(PLATWORD) );
	if (Ptr)
	{
		*PtrPtr = Ptr + 2*sizeof(PLATWORD);
		*((PLATWORD *) Ptr) = EXAMEM_SIGNATURE;
		*((PLATWORD *) (Ptr+sizeof(PLATWORD))) = Size;
		return(EXAPATCH_SUCCESS);
	}
	return(EXAPATCH_OUT_OF_MEMORY);
# endif
}
int ExaMemFree( HANDLE Handle, void * Ptr ) /* MISRA C 2012 [2.7, 8.13]: Handle is unused (and could be const) with this set of compilation flags */
{
# ifdef _WIN32
# ifdef MEM_TRACE
	PLATWORD FreeSize;
	FreeSize = (PLATWORD)(GlobalSize( Ptr ));
# endif
	if (NULL != GlobalFree( Ptr ))
	{
		return(EXAPATCH_INVALID_PARAMETER); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
# ifdef MEM_TRACE
	CurMemSize -= FreeSize;
# endif
	return(EXAPATCH_SUCCESS);
# else
	char * Ptr2;

	Ptr2 = ((char *) Ptr) - 2*sizeof(PLATWORD);
	if (*((PLATWORD *)Ptr2) == EXAMEM_SIGNATURE)
	{
		free( Ptr2 );
		return(EXAPATCH_SUCCESS); /* MISRA C 2012 [15.5]: return used for quick exit in success condition  */
	}
	return(EXAPATCH_INVALID_PARAMETER);
# endif
}
# ifndef ATTOPATCH
PLATWORD ExaMemSize( HANDLE Handle, void * Ptr )
{
# ifdef _WIN32
	return((unsigned)(GlobalSize(Ptr)));
# else
	char * Ptr2;

	Ptr2 = ((char *) Ptr) - 2*sizeof(PLATWORD);
	if (*((PLATWORD *) Ptr2) == EXAMEM_SIGNATURE)
	{
		return(*((PLATWORD *) (Ptr2+sizeof(PLATWORD))));
	}
	return(0);
# endif
}
int ExaMemAllocBig( HANDLE Handle, PLATWORD Size, PLATWORD MaxSize, void * * PtrPtr )
{
# ifdef _WIN32
	*PtrPtr = (void *) GlobalAlloc( GMEM_FIXED, Size+sizeof(PLATWORD) );
	if (*PtrPtr == NULL)
	{
		return(EXAPATCH_OUT_OF_MEMORY);
	}
# ifdef MEM_TRACE
	CurMemSize += Size + sizeof(PLATWORD);
	if (MaxMemSize < CurMemSize)
	{
		MaxMemSize = CurMemSize;
		printf("MM = %x\n",MaxMemSize);
	}
# endif
	return(EXAPATCH_SUCCESS);
# else
	char * Ptr;

	Ptr = malloc( Size + 3*sizeof(PLATWORD) );
	if (Ptr)
	{
		*PtrPtr = Ptr + 2*sizeof(PLATWORD);
		*((PLATWORD *) Ptr) = EXAMEM_SIGNATURE;
		*((PLATWORD *) (Ptr+sizeof(PLATWORD))) = Size;
		return(EXAPATCH_SUCCESS);
	}
	return(EXAPATCH_OUT_OF_MEMORY);
# endif
}
int ExaMemReallocBig( HANDLE Handle, PLATWORD Size, void * * PtrPtr )
{
# ifdef _WIN32
# ifdef MEM_TRACE
	PLATWORD OldSize;
	OldSize = (PLATWORD)(GlobalSize(*PtrPtr));
# endif
	*PtrPtr = GlobalReAlloc( *PtrPtr, Size+sizeof(PLATWORD), GMEM_MOVEABLE );
	if (*PtrPtr)
	{
		return(EXAPATCH_SUCCESS);
	}
# ifdef MEM_TRACE
	CurMemSize += (Size+sizeof(PLATWORD))-OldSize;
	if (MaxMemSize < CurMemSize)
	{
		MaxMemSize = CurMemSize;
		printf("MM = %x\n",MaxMemSize);
	}
# endif
	return(EXAPATCH_OUT_OF_MEMORY);
# else
	char * Ptr2;
	char * Ptr;

	Ptr2 = ((char *) *PtrPtr) - 2*sizeof(PLATWORD);
	if (*((PLATWORD *) Ptr2) != EXAMEM_SIGNATURE)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	Ptr = realloc( Ptr2, Size + 3*sizeof(PLATWORD) );
	if (Ptr)
	{
		*PtrPtr = Ptr + 2*sizeof(PLATWORD);
		*((PLATWORD *) Ptr) = EXAMEM_SIGNATURE;
		*((PLATWORD *) (Ptr+sizeof(PLATWORD))) = Size;
		return(EXAPATCH_SUCCESS);
	}
	return(EXAPATCH_OUT_OF_MEMORY);
# endif
}
PLATWORD ExaMemSizeBig( HANDLE Handle, void * Ptr )
{
# ifdef _WIN32
	return((PLATWORD)(GlobalSize(Ptr)-sizeof(PLATWORD)));
# else
	char * Ptr2;

	Ptr2 = ((char *) Ptr) - 2*sizeof(PLATWORD);
	if (*((PLATWORD *) Ptr2) == EXAMEM_SIGNATURE)
	{
		return(*((PLATWORD *) (Ptr2+sizeof(PLATWORD))));
	}
	return(0);
# endif
}
int ExaMemFreeBig( HANDLE Handle, void * Ptr )
{
# ifdef _WIN32
# ifdef MEM_TRACE
	PLATWORD FreeSize;
	FreeSize = (PLATWORD)(GlobalSize(Ptr));
# endif
	if (GlobalFree( Ptr ))
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
# ifdef MEM_TRACE
	CurMemSize -= FreeSize;
# endif
	return(EXAPATCH_SUCCESS);
# else
	char * Ptr2;

	Ptr2 = ((char *) Ptr) - 2*sizeof(PLATWORD);
	if (*((PLATWORD *)Ptr2) == EXAMEM_SIGNATURE)
	{
		free( Ptr2 );
		return(EXAPATCH_SUCCESS);
	}
	return(EXAPATCH_INVALID_PARAMETER);
# endif
}

int ExaMemShutDown( HANDLE Handle )
{
	return(EXAPATCH_SUCCESS);
}
# endif
# else
	/* The non-EZ memory handler is NOT available except under Win32 */
typedef struct _ExaMemBigBlockHdr {
	void * Base;
	PLATWORD MaxSize;
	PLATWORD CurSize;
	struct _ExaMemBigBlockHdr * Next;
	struct _ExaMemBigBlockHdr * Prev;
	} ExaMemBigBlockHdr;

typedef struct _ExaMemHeap {
	PLATWORD Signature;
	HANDLE HeapHandle;
	unsigned Serialized;
	unsigned BusyFlag;
	CRITICAL_SECTION CritSect;
	ExaMemBigBlockHdr * BigHead;
	ExaMemBigBlockHdr * BigTail;
	} ExaMemHeap;

# if defined( _WIN64 ) || defined( _LP64 )
# define EXAMEM_SIGNATURE 0xeaaccaaecaaeeaac
# else
# define EXAMEM_SIGNATURE 0xeaaccaae
# endif
static unsigned PageSize=0;
static unsigned PageMask=0;

static int AcquireHeap( ExaMemHeap * Ptr )
{
	if (Ptr->Serialized == FALSE)
		return(EXAPATCH_SUCCESS);
		
	EnterCriticalSection( &Ptr->CritSect );
	if (Ptr->BusyFlag)
	{
		LeaveCriticalSection( &Ptr->CritSect );
		return(EXAPATCH_BUSY);
	}
	Ptr->BusyFlag = TRUE;
	LeaveCriticalSection( &Ptr->CritSect );
	return(EXAPATCH_SUCCESS);


}
static int ReleaseHeap( ExaMemHeap * Ptr )
{
	if (Ptr->Serialized == FALSE)
		return(EXAPATCH_SUCCESS);
		
	EnterCriticalSection( &Ptr->CritSect );
	Ptr->BusyFlag = FALSE;
	LeaveCriticalSection( &Ptr->CritSect );
	return(EXAPATCH_SUCCESS);
	
}

int ExaMemInit( PLATWORD InitSize, HANDLE * HandlePtr, unsigned SerializeFlag )
{
	ExaMemHeap * TheHeap;
	SYSTEM_INFO TheInfo;
	if (PageSize == 0)
	{
		GetSystemInfo( &TheInfo );
		PageMask = ~(TheInfo.dwAllocationGranularity - 1);
		PageSize = TheInfo.dwAllocationGranularity;
	}
	TheHeap = GlobalAlloc( GMEM_FIXED, sizeof(ExaMemHeap) );
	if (TheHeap == NULL)
	{
		return(EXAPATCH_OUT_OF_MEMORY);
	}
	if (SerializeFlag)
		InitializeCriticalSection( &TheHeap->CritSect );
	TheHeap->HeapHandle = HeapCreate( SerializeFlag?0:HEAP_NO_SERIALIZE, InitSize, 0 );
	if (TheHeap->HeapHandle == NULL)
	{
		if (SerializeFlag)
			DeleteCriticalSection( &TheHeap->CritSect );
		GlobalFree( TheHeap );
		return(EXAPATCH_OUT_OF_MEMORY);
	}
	TheHeap->Signature = EXAMEM_SIGNATURE;
	TheHeap->Serialized = SerializeFlag;
	TheHeap->BigHead = TheHeap->BigTail = NULL;
	TheHeap->BusyFlag = FALSE;
	*HandlePtr = (HANDLE) TheHeap;
	return(EXAPATCH_SUCCESS);
}

int ExaMemAlloc( HANDLE Handle, PLATWORD Size, void * * PtrPtr )
{
	ExaMemHeap * TheHeap;
	void * Ptr;
	int Code;

	if (Handle == 0)
	{
		*PtrPtr = (void *) GlobalAlloc( GMEM_FIXED, Size );
		if (*PtrPtr == NULL)
		{
			return(EXAPATCH_OUT_OF_MEMORY);
		}
		return(EXAPATCH_SUCCESS);
	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(EXAPATCH_INVALID_PARAMETER);
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	if (Code = AcquireHeap( TheHeap ) )
	{
		return(Code);
	}

	Ptr = HeapAlloc( TheHeap->HeapHandle, TheHeap->Serialized?0:HEAP_NO_SERIALIZE, Size );
	if (Ptr == NULL)
	{
		ReleaseHeap( TheHeap );
		return(EXAPATCH_OUT_OF_MEMORY);
	}
	*PtrPtr = Ptr;
	ReleaseHeap( TheHeap );
	return(EXAPATCH_SUCCESS);
}
int ExaMemFree( HANDLE Handle, void * Ptr )
{
	ExaMemHeap * TheHeap;
	int Code;

	if (Handle == 0)
	{
		if (GlobalFree( Ptr ))
		{
			return(EXAPATCH_INVALID_PARAMETER);
		}
		return(EXAPATCH_SUCCESS);

	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(EXAPATCH_INVALID_PARAMETER);
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	if (Code = AcquireHeap( TheHeap ))
	{
		return(Code);
	}

	if (HeapFree( TheHeap->HeapHandle, TheHeap->Serialized?0:HEAP_NO_SERIALIZE, Ptr) )
	{
		ReleaseHeap( TheHeap );
		return(EXAPATCH_SUCCESS);
	}
	ReleaseHeap( TheHeap );
	return(EXAPATCH_INVALID_PARAMETER);
}
PLATWORD ExaMemSize( HANDLE Handle, void * Ptr )
{
	ExaMemHeap * TheHeap;
	int Code;
	unsigned RetVal;

	if (Handle == 0)
	{
		return(GlobalSize(Ptr));
	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(0xffffffffU);
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(0xffffffffU);
	}
	if (Code = AcquireHeap( TheHeap ))
	{
		return(0xffffffffU);
	}
	RetVal = HeapSize( TheHeap->HeapHandle, TheHeap->Serialized?0:HEAP_NO_SERIALIZE, Ptr);
	ReleaseHeap( TheHeap );
	return(RetVal);
}
int ExaMemAllocBig( HANDLE Handle, PLATWORD NomSize, PLATWORD NomMaxSize, void * * PtrPtr )
{
	unsigned Size = NomSize+sizeof(PLATWORD);
	unsigned MaxSize = NomMaxSize+sizeof(PLATWORD);
	ExaMemHeap * TheHeap;
	ExaMemBigBlockHdr * TheHdr;
	int Code;

	if (Handle == 0)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(EXAPATCH_INVALID_PARAMETER);
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	if (Code = AcquireHeap( TheHeap ))
	{
		return(Code);
	}
	TheHdr = HeapAlloc( TheHeap->HeapHandle, TheHeap->Serialized?0:HEAP_NO_SERIALIZE, sizeof(ExaMemBigBlockHdr) );
	if (TheHdr == NULL) 
	{
		ReleaseHeap( TheHeap );
		return(EXAPATCH_OUT_OF_MEMORY);
	}
	TheHdr->CurSize = (Size + PageSize - 1) & PageMask;
	TheHdr->MaxSize = (MaxSize + PageSize - 1) & PageMask;
	TheHdr->Base = VirtualAlloc( NULL, TheHdr->MaxSize, MEM_RESERVE, PAGE_READWRITE );
	if (TheHdr->Base == NULL)
	{
		HeapFree( TheHeap->HeapHandle, TheHeap->Serialized?0:HEAP_NO_SERIALIZE, TheHdr );
		ReleaseHeap( TheHeap );
		return(EXAPATCH_OUT_OF_MEMORY);
	}
	if (NULL == VirtualAlloc( TheHdr->Base, TheHdr->CurSize, MEM_COMMIT, PAGE_READWRITE) )
	{
		VirtualFree( TheHdr->Base, 0, MEM_RELEASE );
		HeapFree( TheHeap->HeapHandle, TheHeap->Serialized?0:HEAP_NO_SERIALIZE, TheHdr );
		ReleaseHeap( TheHeap );
		return(EXAPATCH_OUT_OF_MEMORY);
	}
	TheHdr->Next = TheHeap->BigHead;
	if (TheHeap->BigHead)
		TheHeap->BigHead->Prev = TheHdr;
	else
		TheHeap->BigTail = TheHdr;
	TheHdr->Prev = NULL;
	TheHeap->BigHead = TheHdr;
	*PtrPtr = TheHdr->Base;
	ReleaseHeap( TheHeap );
	return(EXAPATCH_SUCCESS);
}
int ExaMemReallocBig( HANDLE Handle, PLATWORD NomSize, void * * PtrPtr )
{
	unsigned Size = NomSize+sizeof(PLATWORD);
	ExaMemHeap * TheHeap;
	ExaMemBigBlockHdr * TheHdr;
	int Code;
	unsigned NewSize;
	void * Ptr;

	if (Handle == 0)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(EXAPATCH_INVALID_PARAMETER);
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	if (Code = AcquireHeap( TheHeap ))
	{
		return(Code);
	}
	TheHdr = TheHeap->BigHead;
	while ( TheHdr && TheHdr->Base != *PtrPtr )
	{
		TheHdr = TheHdr->Next;
	}
	if (TheHdr == NULL)
	{
		ReleaseHeap( TheHeap );
		return(EXAPATCH_INVALID_PARAMETER);
	}
	NewSize = (Size + PageSize - 1) & PageMask;
	if (NewSize == TheHdr->CurSize)
	{
		/* no change */
		ReleaseHeap( TheHeap );
		return(EXAPATCH_SUCCESS);
	}
	else if (NewSize < TheHdr->CurSize)
	{
		/* shrink */
		Code = VirtualFree( NewSize + (unsigned char *) TheHdr->Base ,
				TheHdr->CurSize - NewSize, MEM_DECOMMIT );
		if (Code)
		{
			TheHdr->CurSize = NewSize;
			ReleaseHeap( TheHeap );
			return(EXAPATCH_SUCCESS);
		}
		else
		{
			/* shrink failure - something is drastically wrong! */
			ReleaseHeap( TheHeap );
			return(EXAPATCH_INTERNAL_ERROR);
		}
	}
	else if (NewSize > TheHdr->MaxSize)
	{
		/* grow not possible */
		ReleaseHeap( TheHeap );
		return(EXAPATCH_INVALID_PARAMETER);
	}
	/* grow */
	Ptr = VirtualAlloc( TheHdr->CurSize + (unsigned char *) TheHdr->Base,
			NewSize - TheHdr->CurSize, MEM_COMMIT, PAGE_READWRITE );
	if (Ptr == NULL)
	{
		ReleaseHeap( TheHeap );
		return(EXAPATCH_OUT_OF_MEMORY);
	}
	TheHdr->CurSize = NewSize;
	ReleaseHeap( TheHeap );
	return(EXAPATCH_SUCCESS);
}
PLATWORD ExaMemSizeBig( HANDLE Handle, void * Ptr )
{
	ExaMemHeap * TheHeap;
	ExaMemBigBlockHdr * TheHdr;
	unsigned RetVal;

	if (Handle == 0)
	{
		return(0xffffffffU);
	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(0xffffffffU);
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(0xffffffffU);
	}
	if (AcquireHeap( TheHeap ))
	{
		return(0xffffffffU);
	}
	TheHdr = TheHeap->BigHead;
	while ( TheHdr && TheHdr->Base != Ptr )
	{
		TheHdr = TheHdr->Next;
	}
	if (TheHdr == NULL)
	{
		ReleaseHeap( TheHeap );
		return(0xffffffffU);
	}
	RetVal = TheHdr->CurSize;
	ReleaseHeap( TheHeap );
	return(RetVal-sizeof(PLATWORD));
}
int ExaMemFreeBig( HANDLE Handle, void * Ptr )
{
	ExaMemHeap * TheHeap;
	ExaMemBigBlockHdr * TheHdr;
	int Code;

	if (Handle == 0)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(EXAPATCH_INVALID_PARAMETER);
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	if (Code = AcquireHeap( TheHeap ) )
	{
		return(Code);
	}
	TheHdr = TheHeap->BigHead;
	while ( TheHdr && TheHdr->Base != Ptr )
	{
		TheHdr = TheHdr->Next;
	}
	if (TheHdr == NULL)
	{
		ReleaseHeap( TheHeap );
		return(EXAPATCH_INVALID_PARAMETER);
	}
	if (0 == VirtualFree( Ptr, 0, MEM_RELEASE ) )
	{
		/* this shouldn't happen */
		ReleaseHeap( TheHeap );
		return(EXAPATCH_INTERNAL_ERROR);
	}
	if (TheHdr->Prev)
		TheHdr->Prev->Next = TheHdr->Next;
	else
		TheHeap->BigHead = TheHdr->Next;
	if (TheHdr->Next)
		TheHdr->Next->Prev = TheHdr->Prev;
	else
		TheHeap->BigTail = TheHdr->Prev;
	if (0 == HeapFree( TheHeap->HeapHandle, TheHeap->Serialized?0:HEAP_NO_SERIALIZE, TheHdr ) )
	{
		/* this shouldn't happen either... */
		ReleaseHeap( TheHeap );
		return(EXAPATCH_INTERNAL_ERROR);
	}
	ReleaseHeap( TheHeap );
	return(EXAPATCH_SUCCESS);
}
int ExaMemShutDown( HANDLE Handle )
{
	ExaMemHeap * TheHeap;
	ExaMemBigBlockHdr * TheHdr;
	int Code;

	if (Handle == 0)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	TheHeap = (ExaMemHeap *) Handle;
	if (IsBadReadPtr( TheHeap, sizeof(PLATWORD) ) )
		return(EXAPATCH_INVALID_PARAMETER);
		
	if (TheHeap->Signature != EXAMEM_SIGNATURE)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}
	if (Code = AcquireHeap( TheHeap ))
	{
		return(Code);
	}
	/* free all the big blocks */
	TheHdr = TheHeap->BigHead;
	while ( TheHdr )
	{
		VirtualFree( TheHdr->Base, 0, MEM_RELEASE );
		TheHdr = TheHdr->Next;
	}
	TheHeap->Signature = 0;
	if (TheHeap->Serialized)
	{
		TheHeap->Serialized = 0;
		DeleteCriticalSection( &TheHeap->CritSect );
	}
	HeapDestroy( TheHeap->HeapHandle );
	GlobalFree( TheHeap );
	return(EXAPATCH_SUCCESS);
}
# endif

# else
# include <stdio.h>
# include <time.h>
# if defined( _WIN64 ) || defined( _LP64)
# define EXAMEM_SIGNATURE 0xeaaccaaecaaeeaac
# else
# define EXAMEM_SIGNATURE 0xeaaccaae
# endif

# define EXAMEM_VERBOSE
/* EXAMEM_DEBUG - on error, log info to examem.log */
/* if EXAMEM_VERBOSE is defined, log all calls to examem.log */

# ifdef _WIN32
static CRITICAL_SECTION ExaMemCS;
static DWORD dwExaMemInit = 0;
static DWORD dwExaMemInitDone = 0;
# endif
static FILE * LogFile = NULL;
static PLATWORD * ExaMemHead = NULL;
static PLATWORD * ExaMemTail = NULL;
static PLATWORD ExaMemNumBlocks = 0;
static PLATWORD ExaMemTotalSize = 0;
static PLATWORD ExaMemMaxSize = 0;

# ifdef _WIN32
static ExaMemCSInit( void )
{
	DWORD dwInit;

	dwInit = InterlockedExchange( &dwExaMemInit, 0xffffffffU );
	if (dwInit == 0)
	{
		// we should initialize
		InitializeCriticalSection( &ExaMemCS );
		dwExaMemInitDone = 0xffffffffU;
	}
	else
	{
		// someone else is initializing
		while (dwExaMemInitDone == 0)
		{
			Sleep(1);
		}
	}
}

# endif
void ExaMemRemove( PLATWORD * BlockPtr )
{
	if (BlockPtr[2])
	{
		((PLATWORD *)(BlockPtr[2]))[3] = BlockPtr[3];
	}
	else
	{
		ExaMemHead = (PLATWORD *)BlockPtr[3];
	}
	if (BlockPtr[3])
	{
		((PLATWORD *)(BlockPtr[3]))[2] = BlockPtr[2];
	}
	else
	{
		ExaMemTail = (PLATWORD *) BlockPtr[2];
	}
}

void ExaMemInsert( PLATWORD * BlockPtr, PLATWORD Size )
{
	BlockPtr[0] = EXAMEM_SIGNATURE;
	BlockPtr[1] = Size;
	BlockPtr[2] = (PLATWORD) ExaMemTail;	//prev
	BlockPtr[3] = 0;									//next
	if (ExaMemTail)
	{
		ExaMemTail[3] = (PLATWORD)BlockPtr;
	}
	else
	{
		ExaMemHead = BlockPtr;
	}
	ExaMemTail = BlockPtr;
}

void ExaMemLog( char * Fmt, ... )
{
	va_list ArgList;
	char Buffer[1000];
  HANDLE DummyHandle;

	va_start( ArgList, Fmt);
	vsprintf_s( Buffer, 1000, Fmt, ArgList );
# ifdef _WIN32
	EnterCriticalSection( &ExaMemCS );
# endif
	if (LogFile == NULL)
	{
		LogFile = fopen( "examem.log", "at" );
    ExaMemInit( 0, &DummyHandle, FALSE );    
	}
	fwrite( Buffer, 1, lstrlenA( Buffer ), LogFile );
# ifdef _WIN32
	LeaveCriticalSection( &ExaMemCS );
# endif
	va_end( ArgList );
}
void ExaMemDump( void )
{
	PLATWORD * Ptr;
	char * Status;
	DWORD Flag = TRUE;

# ifdef _WIN32
	if (0 == dwExaMemInitDone)
	{
		ExaMemCSInit();
	}
	EnterCriticalSection( &ExaMemCS );
# endif
	Ptr = ExaMemHead;
	ExaMemLog( "Allocated Memory (%d;0x%x):\n", ExaMemNumBlocks, ExaMemTotalSize );
	while (Ptr && Flag)
	{
		if (Ptr[0] == EXAMEM_SIGNATURE)
		{
			if (((Ptr[2]==0) && (ExaMemHead == Ptr)) 
				|| (Ptr[2] && (((PLATWORD *)Ptr[2])[3] == (PLATWORD)Ptr)))
			{
				if (((Ptr[3]==0)&&(ExaMemTail == Ptr))
				|| (Ptr[3] && (((PLATWORD *)Ptr[3])[2] == (PLATWORD)Ptr)))
				{
					Status = "OK";
				}
				else
				{
					Status = "Bad Next Ptr";
					Flag = FALSE;
				}
			}
			else
			{
				Status = "Bad Prev Ptr";
				Flag = FALSE;
			}
		}
		else
		{
			Status = "Bad Signature";
			Flag = FALSE;
		}
		ExaMemLog( "0x%x(0x%x) %s\n", Ptr+4, Ptr[1], Status );
		if (Flag == FALSE)
		{
			ExaMemLog( "  0x%x - 0x%x - 0x%x\n", Ptr[0], Ptr[2], Ptr[3] );
		}
		Ptr = (PLATWORD *) Ptr[3];
	}
# ifdef _WIN32
	LeaveCriticalSection( &ExaMemCS );
# endif
}
int ExaMemInit( PLATWORD InitSize, HANDLE * HandlePtr, unsigned SerializeFlag )
{
# ifdef _WIN32
	SYSTEMTIME ST;

	ExaMemCSInit();
	*HandlePtr = NULL;
	GetSystemTime( &ST );
	ExaMemLog( "Init: %04d.%02d.%02d %02d:%02d:%02d.%03d (0x%x)\n", 
		ST.wYear,
		ST.wMonth,
		ST.wDay,
		ST.wHour,
		ST.wMinute,
		ST.wSecond,
		ST.wMilliseconds,
		InitSize );
# else
	struct tm * TheTime;
	time_t TheTimeT;

	*HandlePtr = NULL;
	time( &TheTimeT );
	TheTime = localtime( TheTimeT );
	ExaMemLog( "Init: %04d.%02d.%02d %02d:%02d:%02d (0x%x)\n", 
		TheTime.tm_year + 1900, 
		TheTime.tm_month,
		TheTime.tm_mday,
		TheTime.tm_hour,
		TheTime.tm_min,
		TheTime.tm_sec,
		InitSize );
# endif
	return(EXAPATCH_SUCCESS);
}

int ExaMemAlloc( HANDLE Handle, PLATWORD Size, void * * PtrPtr )
{
	PLATWORD * Ptr;

# ifdef _WIN32
	if (0 == dwExaMemInitDone)
	{
		ExaMemCSInit();
	}
	Ptr = (PLATWORD *) GlobalAlloc( GMEM_FIXED, Size + 4*sizeof(PLATWORD) );
# else
	Ptr = (PLATWORD *) malloc( Size + 4*sizeof(PLATWORD) );
# endif
	if (Ptr)
	{
		*PtrPtr = (void *) (Ptr + 4);
# ifdef _WIN32
		EnterCriticalSection( &ExaMemCS );
# endif
		ExaMemInsert( Ptr, Size );
    ExaMemNumBlocks++;
    ExaMemTotalSize += Size;
    if (ExaMemMaxSize < ExaMemTotalSize)
      ExaMemMaxSize = ExaMemTotalSize;
# ifdef EXAMEM_VERBOSE
		ExaMemLog( "A [%d;0x%x]: 0x%x(0x%x)\n", ExaMemNumBlocks, ExaMemTotalSize, *PtrPtr, Size );
# endif
# ifdef _WIN32
		LeaveCriticalSection( &ExaMemCS );
# endif
		return(EXAPATCH_SUCCESS);
	}
	ExaMemLog( "Alloc Failed - 0x%x\n", Size );
	ExaMemDump();
	return(EXAPATCH_OUT_OF_MEMORY);
}
int ExaMemFree( HANDLE Handle, void * Ptr )
{
	PLATWORD * Ptr2;

# ifdef _WIN32
	if (0 == dwExaMemInitDone)
	{
		ExaMemCSInit();
	}
# endif
	Ptr2 = ((PLATWORD *) Ptr) - 4;
	if ((*Ptr2) == EXAMEM_SIGNATURE)
	{
# ifdef _WIN32
		EnterCriticalSection( &ExaMemCS );
# endif
    ExaMemNumBlocks--;
    ExaMemTotalSize -= Ptr2[1];
# ifdef EXAMEM_VERBOSE
		ExaMemLog( "F [%d;0x%x]: 0x%x(0x%x)\n", ExaMemNumBlocks, ExaMemTotalSize, Ptr, Ptr2[1] );
# endif
		ExaMemRemove( Ptr2 );
# ifdef _WIN32
		LeaveCriticalSection( &ExaMemCS );
# endif
# ifdef _WIN32
		GlobalFree( (char *)Ptr2 );
# else
		free( (char *)Ptr2 );
# endif
		return(EXAPATCH_SUCCESS);
	}
	ExaMemLog( "Invalid Free - 0x%x:0x%x\n", Ptr2+4, *Ptr2 );
	return(EXAPATCH_INVALID_PARAMETER);
}
PLATWORD ExaMemSize( HANDLE Handle, void * Ptr )
{
	PLATWORD * Ptr2;

	Ptr2 = ((PLATWORD *) Ptr) - 4;
	if ((Ptr2[0]) == EXAMEM_SIGNATURE)
	{
		return(Ptr2[1]);
	}
	return(0);
}
int ExaMemAllocBig( HANDLE Handle, PLATWORD Size, PLATWORD MaxSize, void * * PtrPtr )
{
	return(ExaMemAlloc( Handle, Size+sizeof(PLATWORD), PtrPtr) );
}
int ExaMemReallocBig( HANDLE Handle, PLATWORD NomSize, void * * PtrPtr )
{
	PLATWORD Size = NomSize+sizeof(PLATWORD);
	PLATWORD * Ptr2;
	PLATWORD * Ptr;

# ifdef _WIN32
	if (0 == dwExaMemInitDone)
	{
		ExaMemCSInit();
	}
# endif
	Ptr2 = ((PLATWORD *) *PtrPtr) - 4;
	if ((*Ptr2) != EXAMEM_SIGNATURE)
	{
		ExaMemLog( "Invalid Realloc - 0x%x:0x%x\n", Ptr2 + 4, *Ptr2 );
		return(EXAPATCH_INVALID_PARAMETER);
	}
# ifdef _WIN32
	Ptr = (PLATWORD *) GlobalReAlloc( (char *) Ptr2, Size+4*sizeof(PLATWORD), GMEM_MOVEABLE );
# else
	Ptr = (PLATWORD *) realloc( Ptr2, Size + 4*sizeof(PLATWORD) );
# endif
	if (Ptr)
	{
# ifdef _WIN32
		EnterCriticalSection( &ExaMemCS );
# endif
    ExaMemTotalSize -= Ptr[1];
    ExaMemTotalSize += Size;
    if (ExaMemMaxSize < ExaMemTotalSize)
      ExaMemMaxSize = ExaMemTotalSize;
# ifdef EXAMEM_VERBOSE
		ExaMemLog( "R: 0x%x(0x%x) -> 0x%x(0x%x)\n", Ptr2 + 4, Ptr[1], Ptr + 4, Size );
# endif
		*PtrPtr = (void *) (Ptr + 4);
		Ptr[0] = EXAMEM_SIGNATURE;
		Ptr[1] = Size;
		if (Ptr != Ptr2)
		{
			if (Ptr[2])
			{
				((PLATWORD *)Ptr[2])[3] = (PLATWORD) Ptr;
			}
			else
			{
				ExaMemHead = Ptr;
			}
			if (Ptr[3])
			{
				((PLATWORD *)Ptr[3])[2] = (PLATWORD) Ptr;
			}
			else
			{
				ExaMemTail = Ptr;
			}
		}
# ifdef _WIN32
		LeaveCriticalSection( &ExaMemCS );
# endif
		return(EXAPATCH_SUCCESS);
	}
	ExaMemLog( "Realloc Failed: 0x%x(0x%x) -> 0x%x\n", Ptr2 + 4, Ptr2[1], Size );
	ExaMemDump();
	return(EXAPATCH_OUT_OF_MEMORY);
}
PLATWORD ExaMemSizeBig( HANDLE Handle, void * Ptr )
{
	return(ExaMemSize( Handle, Ptr)-sizeof(PLATWORD));
}
int ExaMemFreeBig( HANDLE Handle, void * Ptr )
{
	return(ExaMemFree( Handle, Ptr));
}
int ExaMemShutDown( HANDLE Handle )
{
	ExaMemLog( "ShutDown called (MaxSize = 0x%x)\n", ExaMemMaxSize );
	ExaMemDump();
	return(EXAPATCH_SUCCESS);
}
# endif
