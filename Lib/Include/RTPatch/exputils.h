/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXPUTILS.H                                                         |
|                                                                           |
|                                                                           |
|  RTPatch Server Build/Apply DLL Common Utilities API Header File          |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2014.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/
# ifndef EXAPATCH_UTILS_INCLUDED

# define EXAPATCH_UTILS_INCLUDED

# include "exafeature.h"
# include "exaplat.h"
# if !defined(_WIN32) && !defined(ATTOPATCH)
# include <sys/types.h>
# ifndef NOACL
#  if !defined( __ANDROID__) || defined(ANDROID_ACL)
#  include <sys/acl.h>
#  endif
# endif
# endif


/*
 STRUCTURES
*/
typedef struct _ExaPatchFileAttrib {
DWORD Flags;
DWORD Attributes;
FILETIME ChangedTime;
FILETIME CreateTime;
QWORD Size;
} ExaPatchFileAttrib;

# ifndef ATTOPATCH
typedef struct _ExaPatchFileSecurity {
DWORD WhatToSet;
# ifndef _WIN32
uid_t Owner;
gid_t Group;
# ifndef NOACL
#  if !defined( __ANDROID__ ) || defined( ANDROID_ACL )
acl_t ACL;
#  endif
# endif
# ifdef __APPLE__
#  if !defined( __IOS__ ) || defined( IOS_TC )
DWORD Type;
DWORD Creator;
#  endif
# endif
# endif
void * pSD;
} ExaPatchFileSecurity;
# endif /* !ATTOPATCH */

typedef struct _ExaPatchStream {
unsigned int dwSize;
DWORD Type;	
HANDLE FileHandle;		//index
int (CALLBACK * Read)(HANDLE hStream, void * pBuffer, DWORD dwCount, LPDWORD pdwAmtRead);
/*
 returns an error code
                              Buffer  Size   ReturnSize
*/
int (CALLBACK * Write)(HANDLE hStream, const void * pBuffer, DWORD dwCount, LPDWORD pdwAmtWritten);
/*
                              Buffer  Size   ReturnSize
*/
int (CALLBACK * Seek)(HANDLE hStream, QWORD qwPos, DWORD dwOrigin, QWORD * pqwReturnPos);
/*
                              Pos    Origin  ReturnPos
*/
int (CALLBACK * AttribGet)(HANDLE hStream, ExaPatchFileAttrib * pAttrib);
int (CALLBACK * AttribSet)(HANDLE hStream, const ExaPatchFileAttrib * pAttrib);
int (CALLBACK * Close)(HANDLE hStream);
DWORD Flags; /* 1 -> file handle is a true duplicate */
QWORD UserStreamPos;
struct _ExaPatchStream * Link;
QWORD Size;
QWORD CompositeOrigin;
QWORD FileOrigin;
void * Buffer;
DWORD BufferSize;
QWORD BufferOrigin;
QWORD SpecialGapPos;
QWORD CurPos;
struct _ExaPatchStream * CurCpt;
} ExaPatchStream;
/* MISRA C 2012 [1.1, 1.2]: use of non-ANSI keyword when needed under Windows (CALLBACK includes __stdcall) */

typedef struct _ExaPatchFileChecksum {
unsigned char Cksum[10];
} ExaPatchFileChecksum;

/*
 CONSTANTS
 Seek Types
*/
# define EXAPATCH_SEEK_BEGIN 0
# define EXAPATCH_SEEK_CUR 1
# define EXAPATCH_SEEK_END 2

/*
 Stream Types
*/
# define EXP_STREAM_FILE 0U
# define EXP_STREAM_COMPOSITE 1U
# define EXP_STREAM_GAP 2U
# define EXP_STREAM_SPECIAL_GAP 3U
# define EXP_STREAM_USER 4U
# define EXP_STREAM_TYPE_BITS 0xfU
# define EXP_STREAM_BUFFERED 0x10U
# define EXP_STREAM_SPECIAL_BUFFERED 0x20U
# define EXP_STREAM_SB_RESERVED 0x40U
# define EXP_STREAM_SB_RSVD2 0x80U

/*
 Attrib Flags
*/
# define EXP_ATTRIB_ALL 0xf
# define EXP_ATTRIB_ATTRIBUTE	0x1
# define EXP_ATTRIB_CHANGED_DATE	0x2
# define EXP_ATTRIB_CREATE_DATE	0x4
# define EXP_ATTRIB_SIZE		0x8
# define EXP_ATTRIB_SECURITY 0x10 /* NOTE: not included in EXP_ATTRIB_ALL - this is intentional */

/*
 Non-Win32 Security Flags
*/
# define EXP_SECURITY_OWNER 0x1
# define EXP_SECURITY_GROUP 0x2
# define EXP_SECURITY_ACL 0x4
# define EXP_SECURITY_TC 0x8

/*
 File Attributes
*/
# define EXP_ATTRIBUTE_READONLY 0x1
# define EXP_ATTRIBUTE_HIDDEN 0x2
# define EXP_ATTRIBUTE_SYSTEM 0x4
# define EXP_ATTRIBUTE_SECURITY 0x8
# define EXP_ATTRIBUTE_SHAREABLE 0x80 /* No longer supported */
# define EXP_ATTRIBUTE_TEMPORARY 0x100
# define EXP_ATTRIBUTE_INDEX 0x2000
# define EXP_ATTRIBUTE_WIN32_CHANGEABLE 0x2107
# define EXP_ATTRIBUTE_CHECKED 0x2157
# define EXP_ATTRIBUTE_WORLD 0x70000
# define EXP_ATTRIBUTE_WORLD_X 0x10000
# define EXP_ATTRIBUTE_WORLD_W 0x20000
# define EXP_ATTRIBUTE_WORLD_R 0x30000
# define EXP_ATTRIBUTE_GROUP 0x380000
# define EXP_ATTRIBUTE_GROUP_X 0x80000
# define EXP_ATTRIBUTE_GROUP_W 0x100000
# define EXP_ATTRIBUTE_GROUP_R 0x200000
# define EXP_ATTRIBUTE_OWNER 0x1c00000
# define EXP_ATTRIBUTE_OWNER_X 0x400000
# define EXP_ATTRIBUTE_OWNER_W 0x800000
# define EXP_ATTRIBUTE_OWNER_R 0x1000000
# define EXP_ATTRIBUTE_PERMISSIONS 0xdff0000

/*
 Checksum types
*/
# define EXP_CHECKSUM_NEW 0x80

/*
 ROUTINES

 Streams (exastream.c)
*/
int InitExaStream( ExaPatchStream * Stream );
int ReadExaStream( ExaPatchStream * Stream, void * Buffer, DWORD Count, LPDWORD lpCount );
int WriteExaStream( ExaPatchStream * Stream, const void * Buffer, DWORD Count, LPDWORD lpCount );
int SeekExaStream( ExaPatchStream * Stream, QWORD Position, DWORD Origin, QWORD * lpPos );
int CloseExaStream( ExaPatchStream * Stream );
# ifndef ATTOPATCH
int CloneExaStream( ExaPatchStream ** DestStream, const ExaPatchStream * SrcStream, DWORD DupFlag );
int FreeClonedExaStream( ExaPatchStream * Stream );
# endif
int GetExaStreamAttrib( ExaPatchStream * Stream, ExaPatchFileAttrib * Attrib );
int SetExaStreamAttrib( ExaPatchStream * Stream, const ExaPatchFileAttrib * Attrib );
int MakeExaStreamFromFileArray( ExaPatchStream * Stream, DWORD NumHandles, HANDLE * HandlePtr, QWORD * SizePtr );
int ForceExaStreamZeroes( ExaPatchStream * Stream, QWORD Pos, QWORD Size, QWORD * SpecialPosPtr );
int SetExaStreamBuffering( ExaPatchStream * Stream, DWORD Flag );
int GetNextExaStreamSpecial( const ExaPatchStream * Stream, QWORD CurPos, QWORD * OffsetPtr, QWORD * LenPtr, QWORD * PosPtr );
int ForceExaStreamBuffering( ExaPatchStream * Stream, DWORD dwBufferSize );
# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
int ExaStreamSupplyBuffer( ExaPatchStream * Stream, void * Buffer, DWORD dwBufferSize );
# endif
# ifdef EXAPATCH_ACLCACHE_SUPPORT
int ExaStreamAclCacheOn( void );
	/*
		thread-safe and uses a lock count - CacheOff must be called once for each successful call to CacheOn
	*/
int ExaStreamAclCacheOff( void );
int ExaStreamAclCacheQuery( void );
	/* 
		returns EXAPATCH_SUCCESS if cache is on, and increments lock count, so that CacheOff must be called 
		returns EXAPATCH_UNSPECIFIED_ERROR if cache is off
	*/
# endif

# define EXAPATCH_STREAM_DEFAULT_BUFSIZE 0x40000

/*
 Memory (examem.c)
*/
int ExaMemInit( PLATWORD InitSize, HANDLE * HandlePtr, unsigned int SerializeFlag );
int ExaMemAlloc( HANDLE Handle, PLATWORD Size, void * * PtrPtr );
int ExaMemFree( HANDLE Handle, void * Ptr );
PLATWORD ExaMemSize( HANDLE Handle, void * Ptr );
int ExaMemAllocBig( HANDLE Handle, PLATWORD Size, PLATWORD MaxSize, void * * PtrPtr );
int ExaMemReallocBig( HANDLE Handle, PLATWORD Size, void * * PtrPtr );
PLATWORD ExaMemSizeBig( HANDLE Handle, void * Ptr );
int ExaMemFreeBig( HANDLE Handle, void * Ptr );
int ExaMemShutDown( HANDLE Handle );

/*
 CheckSums (exacksum.c)
*/
int ExaCSCheckSum(
	unsigned char * Buffer,
	DWORD Length,
	ExaPatchFileChecksum * CSPtr,
	int Type
	);
int ExaCSUnCheckSum(
	unsigned char * Buffer,
	DWORD Length,
	QWORD Offset,
	ExaPatchFileChecksum * CSPtr
	);
/*
 FileNames (exafname.c)
*/
# ifndef ATTOPATCH
HANDLE ExaOpen( const wchar_t * FileName, DWORD dwReadOnly, DWORD dwAppend );
int ExaClose( HANDLE hFile );
int ExaDelete( const wchar_t * FileName );
/*
		 no delay
*/
int ExaRename( const wchar_t * FileName1, const wchar_t * FileName2 );
/* 
 no delay
*/
wchar_t * ExaBaseName( wchar_t * FileName, wchar_t SepChar );
int ExaFullyQualify( const wchar_t * FileName, wchar_t **RetPtr);
int ExaDirMerge( const wchar_t * DirName, const wchar_t *FileName, wchar_t **RetPtr, wchar_t SepChar );
int ExaBuildTempName( wchar_t * DirName, DWORD Num, wchar_t SepChar );
int ExaExists( const wchar_t * FileName );
int ExaFileExists( const wchar_t * FileName );
int ExaDirExists( const wchar_t * FileName );
int ExaIsSymlink( const wchar_t * FileName );
int ExaCopyFile( const wchar_t * Source, const wchar_t * Dest, DWORD dwEADSFlag, DWORD dwLinkFlag, DWORD dwOGFlag );
# ifndef ATTOPATCH
int ExaSetFileAttrib( const wchar_t * FileName, ExaPatchFileAttrib * Attrib, ExaPatchFileSecurity * pFS, DWORD dwLink );
int ExaGetFileAttrib( const wchar_t * FileName, ExaPatchFileAttrib * Attrib, ExaPatchFileSecurity * pFS, DWORD dwLink );
int ExaFreeFileSecurity( ExaPatchFileSecurity * pFS );
# endif
int ExaMakeDir( wchar_t * Directory );
int ExaMakeDirEx( wchar_t * Directory, wchar_t ** pTopDir ); /* returns top-most parent that needed to be created */
int ExaRmDir( const wchar_t * Directory );
# ifdef EXAPATCH_UPDATELINK_SUPPORT
int ExaGetLinkTarget(const wchar_t * pFileName,char * pBuffer, DWORD dwSize, DWORD * pdwSizeNeeded);
int ExaSetLinkTarget(const wchar_t * pFileName, const char * pBuffer, DWORD dwDirFlag);
# endif
# ifdef EXAPATCH_OWNERGROUP_SUPPORT
int ExaGetOwnerGroupNum(const wchar_t * pFileName, DWORD * pdwOwner, DWORD * pdwGroup);
int ExaSetOwnerGroupNum(const wchar_t * pFileName, DWORD dwOwner, DWORD dwGroup );
# endif
/* free space mechanism */
/* Typedefs */

typedef struct _MOUNTPOINT {
struct _MOUNTPOINT * pNext;
struct _MOUNTPOINT * pPrev;
struct _PHYSDRIVE * pPhys;
wchar_t * szWideName;
# ifdef _WIN32
char * szNarrowName;
# endif
} MOUNTPOINT, * PMOUNTPOINT;

typedef struct _PHYSDRIVE {
struct _PHYSDRIVE * pNext;
struct _PHYSDRIVE * pPrev;
# ifdef _WIN32
wchar_t * szWideNormalName;
char * szNarrowNormalName;
# else
dev_t st_dev;
# endif
wchar_t * szDevName;
QWORD qwNumFiles;
QWORD qwFileSizeNeeded;
QWORD qwBytesNeeded;
QWORD qwBytesPresent;
} PHYSDRIVE, * PPHYSDRIVE;

int ExaDevInit( BOOL bLimited );
int ExaDevClose( void );
int ExaDevRegNeeds( wchar_t * szPath, QWORD qwFileSize, QWORD qwNumFiles );
int ExaDevCheckNeeds( void );
PMOUNTPOINT ExaDevGetMP( void );
PPHYSDRIVE ExaDevGetPD( void );
int ExaDevCompare( wchar_t * szPath1, wchar_t * szPath2 );
# endif /* !ATTOPATCH */

# endif	/* EXAPATCH_UTILS_INCLUDED */
