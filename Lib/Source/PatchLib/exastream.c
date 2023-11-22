/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXASTREAM.C                                                        |
|                                                                           |
|                                                                           |
|  RTPatch Server Stream I/O Routines                                       |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2010.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/
/*lint --e{950, 9026, 9024, 9079, 9087, 904, 527, 818} */
/* Suppress non-ANSI, function-type macro, ## macro, pointer conversion, early return, unreachable code, 
	possible const, warnings in this file (all deviations are marked) */
# define EXAPATCH_INTERNAL_CODE
# include "exaplat.h"
# include "experror.h"
# include "exputils.h"

# ifdef _WIN32
# ifndef WINCE
# ifndef ATTOPATCH
# include <aclapi.h>
# endif
# endif
# endif
# ifdef EXAPATCH_ACLCACHE_SUPPORT
//# define ACLCACHE_DEBUG
# ifdef ACLCACHE_DEBUG
# include <stdio.h>
# endif
# endif
/* suppress some spurious warning messages for gcc - this is (of necessity) overly broad if the version is between 4.2 and 4.6 */
# ifndef _WIN32
# if (GCC_VERSION >= 40200) && (GCC_VERSION < 40600)
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
# endif
# endif

# ifndef ATTOPATCH
# define EPOCH_CHANGE (((__int64)(1000*60*60*24))*((__int64)(72+(365*300))))
# define TIME_T_EPOCH_CHANGE (((long long)(60*60*24))*((long long)(17 + (365*69))))
# endif

int InitExaStream( ExaPatchStream * Stream )
{
	Stream->Buffer = NULL;
	Stream->CurPos = 0;
	Stream->CompositeOrigin = 0;
	Stream->FileOrigin = 0;
	Stream->CurCpt = NULL;
	return (EXAPATCH_SUCCESS);
}

int ReadExaStream( ExaPatchStream * Stream, void * Buffer, DWORD Count, LPDWORD lpCount )
{
	unsigned char * CurPtr = (unsigned char *) Buffer; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
	DWORD CountLeft = Count;
	DWORD ThisCount;
	DWORD ThisCountReturn = 0;
	int Code = EXAPATCH_SUCCESS;
# ifdef ATTOPATCH
	if (0U != (Stream->Type & EXP_STREAM_BUFFERED))
	{
		return (EXAPATCH_NOT_SUPPORTED); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
# else
	/* see if there is any buffered stuff we can use */
	/* currently we only check for end of previous buffer */
	/* 		overlapping beginning of this read (only case we */
	/*		currently need) */
	if (0U != (Stream->Type & EXP_STREAM_BUFFERED))
	{
		if (((Stream->BufferOrigin + (QWORD)Stream->BufferSize) > Stream->CurPos)
			&& (Stream->BufferOrigin <= Stream->CurPos))
		{
			ThisCount = (DWORD)((Stream->BufferOrigin + (QWORD)Stream->BufferSize) - Stream->CurPos);
			if (ThisCount > Count)
			{
				ThisCount = Count;
			}
			if ((CurPtr != Stream->Buffer) || (Stream->CurPos != Stream->BufferOrigin))
			{
				memmove( CurPtr, &((unsigned char *)Stream->Buffer)[Stream->CurPos - Stream->BufferOrigin], ThisCount); /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
			}
			CountLeft -= ThisCount;
			CurPtr = &CurPtr[ThisCount];
			if (0 != SeekExaStream( Stream, (QWORD) ThisCount, EXAPATCH_SEEK_CUR, NULL )) /* this shouldn't fail */
			{
					if (NULL != lpCount)
					{
						*lpCount = Count - CountLeft;
					}
					return (EXAPATCH_SEEK_FAILED); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			}
		}
	}
# endif
	ThisCount = CountLeft;
	if ((Stream->Size + Stream->CompositeOrigin) > Stream->CurPos)
	{
		if (((QWORD)ThisCount) > ((Stream->Size + Stream->CompositeOrigin) - Stream->CurPos))
		{
			ThisCount = (DWORD)((Stream->Size + Stream->CompositeOrigin) - Stream->CurPos);
		}
	}
	else
	{
		ThisCount = 0;
	}
	if (0U != ThisCount)
	{
		switch ( Stream->Type & EXP_STREAM_TYPE_BITS )
		{
			default:
				Code = EXAPATCH_NOT_SUPPORTED;
				break;
			case EXP_STREAM_FILE:
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
#  ifdef _WIN32
				Code = ReadFile( Stream->FileHandle, CurPtr, ThisCount, &ThisCountReturn, NULL );
				CurPtr += ThisCountReturn;
				Stream->CurPos += (QWORD)ThisCountReturn;
				CountLeft -= ThisCountReturn;
				if (Code)
				{
					Code = EXAPATCH_SUCCESS;
				}
				else
				{
					Code = EXAPATCH_READ_FAILED;
				}
#  else
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
				Code = read( (intnt) Stream->FileHandle, CurPtr, ThisCount );
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
			  if (Code == -1)
			  {
					Code = EXAPATCH_READ_FAILED;
			  }
				else
				{
					ThisCountReturn = Code;
					CurPtr += ThisCountReturn;
					Stream->CurPos += (QWORD) ThisCountReturn;
					CountLeft -= ThisCountReturn;
					Code = EXAPATCH_SUCCESS;
				}
#  endif
# else
				Code = EXAPATCH_NOT_SUPPORTED;
# endif
				break;

			case EXP_STREAM_COMPOSITE:
# ifdef EXAPATCH_COMPOSITESTREAM_SUPPORT
				if (Stream->CurCpt == NULL || Stream->CurPos < Stream->CurCpt->CompositeOrigin)
				{
					Stream->CurCpt = Stream->Link;
					SeekExaStream( Stream->CurCpt, 0, EXAPATCH_SEEK_BEGIN, NULL );
				}
				if (Stream->CurCpt->CompositeOrigin <= Stream->CurPos
					&& (Stream->CurCpt->CompositeOrigin + Stream->CurCpt->Size) > Stream->CurPos)
				{
					ThisCount = CountLeft;
					if (((QWORD)ThisCount) > (Stream->CurCpt->Size + Stream->CurCpt->CompositeOrigin - Stream->CurPos))
						ThisCount = (DWORD)(Stream->CurCpt->Size + Stream->CurCpt->CompositeOrigin - Stream->CurPos);
					if (Stream->CurCpt->CurPos != Stream->CurPos)
					{
						Code = SeekExaStream( Stream->CurCpt, Stream->CurPos, EXAPATCH_SEEK_BEGIN, NULL );
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = ReadExaStream( Stream->CurCpt, CurPtr, ThisCount, &ThisCountReturn );
						CountLeft -= ThisCountReturn;
						CurPtr += ThisCountReturn;
						Stream->CurPos += (QWORD)ThisCountReturn;
					}
				}
				/* at this point, we've read all we can from Stream->CurCpt */
				while ( Code == EXAPATCH_SUCCESS && CountLeft && Stream->CurCpt->Link)
				{
					Stream->CurCpt = Stream->CurCpt->Link;
					if (Stream->CurCpt->CompositeOrigin <= Stream->CurPos
						&& (Stream->CurCpt->CompositeOrigin + Stream->CurCpt->Size) > Stream->CurPos)
					{
						ThisCount = CountLeft;
						if (((QWORD) ThisCount) > Stream->CurCpt->Size)
							ThisCount = (DWORD)(Stream->CurCpt->Size);
						Code = SeekExaStream( Stream->CurCpt, Stream->CurPos, EXAPATCH_SEEK_BEGIN, NULL );
						if (Code == EXAPATCH_SUCCESS)
						{
							Code = ReadExaStream( Stream->CurCpt, CurPtr, ThisCount, &ThisCountReturn );
							CountLeft -= ThisCountReturn;
							CurPtr += ThisCountReturn;
							Stream->CurPos += (QWORD)ThisCountReturn;
						}
					}
				}
# else
				Code = EXAPATCH_NOT_SUPPORTED;
# endif
				break;

			case EXP_STREAM_USER:
				if (NULL != Stream->Read)
				{
					Code = Stream->Read(Stream->FileHandle, CurPtr, ThisCount, &ThisCountReturn );/**/
				}
				else
				{
					Code = EXAPATCH_READ_FAILED;
				}
				Stream->CurPos += (QWORD)ThisCountReturn;
				CountLeft -= ThisCount;
				break;

			case EXP_STREAM_GAP:
			case EXP_STREAM_SPECIAL_GAP:
				memset( CurPtr, 0, ThisCount );
				Stream->CurPos += (QWORD)ThisCount;
				CountLeft -= ThisCount;
				break;
		}
	}
	if (NULL != lpCount)
	{
		*lpCount = Count - CountLeft;
	}
# ifndef ATTOPATCH
	if (0U != (Stream->Type & EXP_STREAM_BUFFERED))
	{
		if (Count != CountLeft)
		{
			Stream->Buffer = Buffer;
			Stream->BufferSize = (Count - CountLeft);
			Stream->BufferOrigin = Stream->CurPos - (QWORD)Stream->BufferSize;
		}
	}
# endif
	if (EXP_STREAM_USER == (Stream->Type & EXP_STREAM_TYPE_BITS))
	{
		Stream->UserStreamPos = Stream->CurPos + Stream->FileOrigin - Stream->CompositeOrigin;
	}

	return(Code);
}

int WriteExaStream( ExaPatchStream * Stream, const void * Buffer, DWORD Count, LPDWORD lpCount )
{
	const unsigned char * CurPtr = (const unsigned char *) Buffer; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
	DWORD CountLeft = Count;
	DWORD ThisCount;
	DWORD ThisCountReturn = 0;
	int Code = EXAPATCH_SUCCESS;
	
# ifdef ATTOPATCH
	if (0U != (Stream->Type & EXP_STREAM_BUFFERED))
	{
		return (EXAPATCH_NOT_SUPPORTED); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
# endif

	ThisCount = CountLeft;
	if (0U != ThisCount)
	{

		switch ( Stream->Type & EXP_STREAM_TYPE_BITS )
		{
			default:
				Code = EXAPATCH_NOT_SUPPORTED;
				break;
			case EXP_STREAM_FILE:
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
#  ifdef _WIN32
				Code = WriteFile( Stream->FileHandle, CurPtr, ThisCount, &ThisCountReturn, NULL );
				CurPtr = &CurPtr[ThisCountReturn];
				Stream->CurPos += (QWORD) ThisCountReturn;
				CountLeft -= ThisCountReturn;
				if (Code)
				{
					Code = EXAPATCH_SUCCESS;
				}
				else
				{
					Code = GetLastError();
					Code = EXAPATCH_WRITE_FAILED;
				}
#  else
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
				Code = write( (int) Stream->FileHandle, CurPtr, ThisCount );
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
				if (Code == -1)
				{
					Code = EXAPATCH_WRITE_FAILED;
				}
				else
				{
					ThisCountReturn = Code;
					CurPtr = &CurPtr[ThisCountReturn];
					Stream->CurPos += (QWORD) ThisCountReturn;
					CountLeft -= ThisCountReturn;
					Code = EXAPATCH_SUCCESS;
				}
#  endif
				/* update the Stream Size */
				if ((Stream->CurPos-Stream->CompositeOrigin) > Stream->Size)
				{
					Stream->Size = Stream->CurPos - Stream->CompositeOrigin;
				}
# else
				Code = EXAPATCH_NOT_SUPPORTED;
# endif
				break;

			case EXP_STREAM_COMPOSITE:
# ifdef EXAPATCH_COMPOSITESTREAM_SUPPORT
				if (Stream->CurCpt == NULL || Stream->CurPos < Stream->CurCpt->CompositeOrigin)
				{
					Stream->CurCpt = Stream->Link;
					SeekExaStream( Stream->CurCpt, 0, EXAPATCH_SEEK_BEGIN, NULL );
				}
				if (Stream->CurCpt->CompositeOrigin <= Stream->CurPos
					&& (Stream->CurCpt->CompositeOrigin + Stream->CurCpt->Size) > Stream->CurPos)
				{
					ThisCount = CountLeft;
					if (Stream->CurCpt->Link 
						&& ((QWORD)ThisCount) > (Stream->CurCpt->Size + Stream->CurCpt->CompositeOrigin - Stream->CurPos))
						ThisCount = (DWORD)(Stream->CurCpt->Size + Stream->CurCpt->CompositeOrigin - Stream->CurPos);
					if (Stream->CurCpt->CurPos != Stream->CurPos)
					{
						Code = SeekExaStream( Stream->CurCpt, Stream->CurPos, EXAPATCH_SEEK_BEGIN, NULL );
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = WriteExaStream( Stream->CurCpt, CurPtr, ThisCount, &ThisCountReturn );
						CountLeft -= ThisCountReturn;
						CurPtr += ThisCountReturn;
						Stream->CurPos += (QWORD)ThisCountReturn;
					}
				}
				/* at this point, we've written all we can to Stream->CurCpt */
				while ( Code == EXAPATCH_SUCCESS && CountLeft && Stream->CurCpt->Link)
				{
					Stream->CurCpt = Stream->CurCpt->Link;
					if (Stream->CurCpt->CompositeOrigin <= Stream->CurPos
						&& (Stream->CurCpt->CompositeOrigin + Stream->CurCpt->Size) > Stream->CurPos)
					{
						ThisCount = CountLeft;
						if (Stream->CurCpt->Link && ((QWORD)ThisCount) > Stream->CurCpt->Size)
							ThisCount = (DWORD)(Stream->CurCpt->Size);
						if (Stream->CurCpt->CurPos != Stream->CurPos)
							Code = SeekExaStream( Stream->CurCpt, Stream->CurPos, EXAPATCH_SEEK_BEGIN, NULL );
						if (Code == EXAPATCH_SUCCESS)
						{
							Code = WriteExaStream( Stream->CurCpt, CurPtr, ThisCount, &ThisCountReturn );
							CountLeft -= ThisCountReturn;
							CurPtr = &CurPtr[ThisCountReturn];
							Stream->CurPos += (QWORD)ThisCountReturn;
						}
					}
				}
				/* update the Stream Size */
				if (Stream->CurPos > Stream->Size)
				{
					Stream->Size = Stream->CurPos;
				}
# else
				Code = EXAPATCH_NOT_SUPPORTED;
# endif
				break;

			case EXP_STREAM_USER:
				if ((Stream->UserStreamPos + Stream->CompositeOrigin) 
					!= (Stream->CurPos + Stream->FileOrigin))
				{
					Code = EXAPATCH_WRITE_FAILED;
				}
				else
				{
					if (NULL != Stream->Write)
					{
						Code = Stream->Write(Stream->FileHandle, CurPtr, ThisCount, &ThisCountReturn );/**/
					}
					else
					{
						Code = EXAPATCH_WRITE_FAILED;
					}
					Stream->CurPos += (QWORD)ThisCountReturn;
					CountLeft -= ThisCount;
				}
				/* update the Stream Size */
				if ((Stream->CurPos - Stream->CompositeOrigin) > Stream->Size)
				{
					Stream->Size = Stream->CurPos - Stream->CompositeOrigin;
				}
				break;

			case EXP_STREAM_GAP:
			case EXP_STREAM_SPECIAL_GAP:
				Stream->CurPos += (QWORD)ThisCount;
				CountLeft -= ThisCount;
				break;
		}
	}
	if (NULL != lpCount)
	{
		*lpCount = Count - CountLeft;
	}
# ifndef ATTOPATCH
	if ((0U != (Stream->Type & EXP_STREAM_BUFFERED))
		&& ((Stream->BufferOrigin + (QWORD)Stream->BufferSize) > (Stream->CurPos - (((QWORD)Count) - ((QWORD)CountLeft)) ) )
		&& (Stream->BufferOrigin <= Stream->CurPos ) )
	{
		/* the most recent "read" buffer overlaps this one - replace it. */
		/* Otherwise just leave it alone (writes and reads shouldn't interfere */
		/*	 with one another unless necessary) */
		if (Count != CountLeft)
		{
			Stream->Buffer = (void *)Buffer;
			Stream->BufferSize = (Count - CountLeft);
			Stream->BufferOrigin = Stream->CurPos - Stream->BufferSize;
		}
		else
		{
			Stream->Buffer = NULL;
			Stream->BufferOrigin = 0;
			Stream->BufferSize = 0;
		}
	}
# endif
	if (EXP_STREAM_USER == (Stream->Type & EXP_STREAM_TYPE_BITS))
	{
		Stream->UserStreamPos = Stream->CurPos + Stream->FileOrigin - Stream->CompositeOrigin;
	}
	return(Code);
}

int SeekExaStream( ExaPatchStream * Stream, QWORD Position, DWORD Origin, QWORD * lpPos )
{
	int Code = EXAPATCH_SUCCESS;
	QWORD PrevPos = Stream->UserStreamPos;
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
	QWORD NewPos;
# endif
	QWORD ThisSeek;
	QWORD SeekReturn;
	DWORD ReadReturn;
	// @@EB MALLOC MODIFY
#if 1
	unsigned char * DummyBuffer = NULL;
#else
	unsigned char * DummyBuffer;
#endif
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
	DWORD NoChange=FALSE;
# endif
# ifdef ATTOPATCH
	if (0U != (Stream->Type & EXP_STREAM_BUFFERED))
	{
		return (EXAPATCH_NOT_SUPPORTED); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
# endif
	switch ( Origin )
	{
		default:
			return (EXAPATCH_INVALID_PARAMETER); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			break; /* MISRA C 2012 [2.1]: unreachable code (alternative is MISRA switch syntax departure) */
		case EXAPATCH_SEEK_BEGIN:
			if (Stream->CurPos == Position)
			{
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
				NoChange = TRUE;
# endif
			}
			else
			{
				Stream->CurPos = Position;
			}
			break;

		case EXAPATCH_SEEK_CUR:
			if (Position == 0U)
			{
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
				NoChange = TRUE;
# endif
			}
			else
			{
				Stream->CurPos += Position;
			}
			break;

		case EXAPATCH_SEEK_END:
			Stream->CurPos = Stream->CompositeOrigin + Stream->Size + Position;
			break;
	}
	switch ( Stream->Type & EXP_STREAM_TYPE_BITS)
	{
		default:
			Code = EXAPATCH_NOT_SUPPORTED;
			break;
		case EXP_STREAM_FILE:
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
			if (!NoChange)
			{
				NewPos = Stream->CurPos + Stream->FileOrigin - Stream->CompositeOrigin;
# ifndef __FreeBSD__
				if (NewPos <= 0x7fffffff)
# endif
				{
#  ifdef _WIN32
					Code = SetFilePointer( Stream->FileHandle, (DWORD)NewPos, 
						NULL, FILE_BEGIN );
					if (Code == 0xffffffffU)
					{
						Code = EXAPATCH_SEEK_FAILED;
					}
					else
					{
						Code = EXAPATCH_SUCCESS;
					}
#  else
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
					Code = lseek( (int) Stream->FileHandle,
#  ifdef __FreeBSD__
							(off_t) NewPos,
#  else
							(DWORD) NewPos,
#  endif
							SEEK_SET );
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
					if (Code == -1)
					{
						Code = EXAPATCH_SEEK_FAILED;
					}
					else
					{
						Code = EXAPATCH_SUCCESS;
					}
#  endif
				}
#  ifndef __FreeBSD__
				else
				{
#   ifdef _WIN32
					LARGE_INTEGER ll;

					ll.QuadPart = NewPos;
					Code = SetFilePointer( Stream->FileHandle, ll.LowPart, &ll.HighPart, FILE_BEGIN );
					if ((Code == 0xffffffffU) && (GetLastError() != NO_ERROR) )
					{
						Code = EXAPATCH_SEEK_FAILED;
					}
					else
					{
						Code = EXAPATCH_SUCCESS;
					}
#   else
#   	ifdef SEEK64
					/* No ANSI solution for this... */
#     if GCC_VERSION > 40600
#      pragma GCC diagnostic push
#      pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#     endif
					Code = lseek64( (int) Stream->FileHandle, NewPos, SEEK_SET );
#     if GCC_VERSION > 40600
#      pragma GCC diagnostic pop
#     endif
					if (Code == -1)
					{
						Code = EXAPATCH_SEEK_FAILED;
					}
					else
					{
						Code = EXAPATCH_SUCCESS;
					}
#		 else
					Code = EXAPATCH_NOT_SUPPORTED;
#		 endif
#   endif
				}
#  endif
			}
# else
			Code = EXAPATCH_NOT_SUPPORTED;
# endif
			break;
		case EXP_STREAM_USER:
			ThisSeek = Stream->CurPos + Stream->FileOrigin - Stream->CompositeOrigin;
			if (NULL != Stream->Seek)
			{
//				dpt(); dmsg("seek index[%d] pos[%ld]",(int)Stream->FileHandle, (DWORD)PosArray[(int)Stream->FileHandle]);
				Code = Stream->Seek( Stream->FileHandle, ThisSeek, 
					EXAPATCH_SEEK_BEGIN, &SeekReturn );
//				dpt(); dmsg("seek index[%d] pos[%ld]",(int)Stream->FileHandle, (DWORD)PosArray[(int)Stream->FileHandle]);
				Stream->UserStreamPos = SeekReturn;
			}
			else
			{
				if (Stream->CurPos > PrevPos)
				{
					/* seek forward by a dummy read/write */
					if ((Stream->CurPos - PrevPos) < (QWORD) 0x40000000)
					{
						Code = ExaMemAlloc( NULL, (DWORD)(Stream->CurPos - PrevPos), (void **) &DummyBuffer ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						if (Code != EXAPATCH_SUCCESS)
						{
							Code = EXAPATCH_SEEK_FAILED;
						}
						else
						{
							if (NULL != Stream->Read)
							{
//								dpt(); dmsg("read index[%d] pos[%ld]",(int)Stream->FileHandle, (DWORD)PosArray[(int)Stream->FileHandle]);
								Code = Stream->Read( Stream->FileHandle, DummyBuffer, (DWORD)(Stream->CurPos - PrevPos), &ReadReturn );
								if (ReadReturn != (DWORD)(Stream->CurPos - PrevPos))
								{
									Stream->CurPos = ((QWORD)ReadReturn) + PrevPos + Stream->CompositeOrigin - Stream->FileOrigin;
									Stream->UserStreamPos += (QWORD)ReadReturn;
									Code = EXAPATCH_SEEK_FAILED;
								}
							}
							else
							{
								if (NULL != Stream->Write)
								{
									memset( DummyBuffer, 0, (DWORD)(Stream->CurPos - PrevPos) );
									Code = Stream->Write( Stream->FileHandle, DummyBuffer, (DWORD)(Stream->CurPos - PrevPos), &ReadReturn );/**/
									if (ReadReturn != (DWORD)(Stream->CurPos - PrevPos))
									{
										Stream->CurPos = ((QWORD)ReadReturn) + PrevPos + Stream->CompositeOrigin - Stream->FileOrigin;
										Stream->UserStreamPos += (QWORD)ReadReturn;
										Code = EXAPATCH_SEEK_FAILED;
									}
								}
								else
								{
									Code = EXAPATCH_SEEK_FAILED;
								}
							}
							(void)ExaMemFree( NULL, DummyBuffer );
						}
					}
					else
					{
						Code = EXAPATCH_SEEK_FAILED;
					}

				}
				else if (Stream->CurPos < PrevPos)
				{
# ifdef ATTOPATCH
					Code = EXAPATCH_SEEK_FAILED;
# else
					/* seek backward using the buffer */
					if (0U != (Stream->Type & EXP_STREAM_BUFFERED) )
					{
						if ((Stream->Buffer == NULL)
							|| (Stream->CurPos < Stream->BufferOrigin)
							|| (Stream->CurPos >= (Stream->BufferOrigin + Stream->BufferSize)))
						{
							Code = EXAPATCH_SEEK_FAILED;
						}
						else
						{
							Stream->UserStreamPos = Stream->CurPos;
						}
					}
# endif
				}
				else
				{
					Stream->UserStreamPos = Stream->CurPos;
				}
			}
			break;

		case EXP_STREAM_COMPOSITE:
		case EXP_STREAM_GAP:
		case EXP_STREAM_SPECIAL_GAP:
			break;
	}
	if (NULL != lpPos)
	{
		*lpPos = Stream->CurPos;
	}
	return(Code);
	
}
# ifdef EXAPATCH_ACLCACHE_SUPPORT

/* ACL caching to fix bug in Windows implementation of GetEffectiveRightsFromAcl */
#  ifdef _WIN32
static DWORD ACLCacheInitInProgress = 0;
static DWORD ACLCacheInitialized = 0;
static DWORD ACLCacheLockCount = 0;

typedef struct _ACLCLink {
	DWORD dwType; // 0 => ACL, 1 => SID, 2 => Mask
	DWORD dwMask;
	struct _ACLCLink * pNext;
	struct _ACLCLink * pPrev;
	PACL ACL; // if Mask, this is really a pointer to the ACLCLink in the ACL list
	PSID SID; // if ACL, this is really a pointer to the ACLCLink in the Mask list that is the first one associated with this ACL
			  // if Mask, this is really a pointer to the ACLCLink in the SID list
} ACLCLink;

static ACLCLink * ACLHead; // ordered list (ACL ordering)
static ACLCLink * ACLTail;
static ACLCLink * SIDHead; // ordered list (SID ordering)
static ACLCLink * SIDTail;
static ACLCLink * MaskHead; // ordered list - chron(ACL) x SID
static ACLCLink * MaskTail;
int ExaStreamAclCacheOn( void )
{
	DWORD IP;
	int Code = EXAPATCH_SUCCESS;
	
	IP = InterlockedExchange(&ACLCacheInitInProgress, 1);
	if (!IP)
	{
		/* we are the initializer */
		ACLHead = ACLTail = SIDHead = SIDTail = MaskHead = MaskTail = NULL;
		if (Code == EXAPATCH_SUCCESS)
		{
			ACLCacheLockCount = 1;
			ACLCacheInitialized = 1;
		}
		InterlockedExchange( &ACLCacheInitInProgress, 0 );
		
	}
	else
	{
		/* someone else is initializing - wait until they're done */
		while (ACLCacheInitInProgress)
		{
			Sleep(100);
		}
		InterlockedIncrement(&ACLCacheLockCount);
		if (!ACLCacheInitialized)
		{
			/* they failed - presumably we would too */
			InterlockedDecrement(&ACLCacheLockCount);
			Code = EXAPATCH_UNSPECIFIED_ERROR;
		}
	}
	return Code;
}
int ExaStreamAclCacheOff( void )
{
	DWORD LC;
	ACLCLink * MyACLHead;
	ACLCLink * MySIDHead;
	ACLCLink * MyMaskHead;
	ACLCLink * Temp;
# ifdef ACLCACHE_DEBUG
	int nNumACL=0;
	int nNumSID=0;
	int nNumMask=0;
# endif
	
	LC = InterlockedDecrement( &ACLCacheLockCount );
	if ((LC == 0) && ACLCacheInitialized)
	{
		ACLCacheInitialized = 0;
		MyACLHead = ACLHead;
		MySIDHead = SIDHead;
		MyMaskHead = MaskHead;
		ACLHead = ACLTail = SIDHead = SIDTail = MaskHead = MaskTail = NULL;
		while (MyACLHead)
		{
# ifdef ACLCACHE_DEBUG
			nNumACL++;
# endif
			Temp = MyACLHead->pNext;
			ExaMemFree( NULL, MyACLHead->ACL );
			ExaMemFree( NULL, MyACLHead );
			MyACLHead = Temp;
		}
		while (MySIDHead)
		{
# ifdef ACLCACHE_DEBUG
			nNumSID++;
# endif
			Temp = MySIDHead->pNext;
			ExaMemFree( NULL, MySIDHead->SID );
			ExaMemFree( NULL, MySIDHead );
			MySIDHead = Temp;
		}
		while (MyMaskHead)
		{
# ifdef ACLCACHE_DEBUG
			nNumMask++;
# endif
			Temp = MyMaskHead->pNext;
			ExaMemFree( NULL, MyMaskHead );
			MyMaskHead = Temp;
		}
# ifdef ACLCACHE_DEBUG
		printf("GERFA Cache stats: ACL=%d, SID=%d, Mask=%d\n", nNumACL, nNumSID, nNumMask );
# endif
	}
	return EXAPATCH_SUCCESS;
}
int ExaStreamAclCacheQuery( void )
{
	DWORD LC;
	
	LC = InterlockedIncrement( &ACLCacheLockCount );
	if ((LC == 1) || !ACLCacheInitialized)
	{
		InterlockedDecrement( &ACLCacheLockCount );
		return EXAPATCH_UNSPECIFIED_ERROR;
	}
	return EXAPATCH_SUCCESS;
}
static int CompareACL( PACL pACL1, PACL pACL2 )
{
	/*
		<0 => ACL1 < ACL2
		0 => ACL1 == ACL2
		>0 => ACL1 > ACL2
	*/
	int aceCount;
	int aceIndex;
	int c;
	
	if (pACL1->AceCount != pACL2->AceCount)
	{
		if (pACL1->AceCount < pACL2->AceCount)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	 // We have the same number of ACEs, so compare each ACE
	aceCount = pACL1->AceCount;
    for (aceIndex = 0; aceIndex != aceCount; aceIndex++)
    {
        // Get the ACEs
        PACE_HEADER ace1;
        PACE_HEADER ace2;
        GetAce(pACL1, aceIndex, (LPVOID*)&ace1);
        GetAce(pACL2, aceIndex, (LPVOID*)&ace2);
        // Compare the ACE sizes

        c = ace1->AceSize - ace2->AceSize;
        if (c)
            return c;

        // Compare the ACE content
        c = memcmp(ace1, ace2, ace1->AceSize);
        if (c)
            return c;
    }
    return 0;
}
static int CompareSID( PSID pSID1, PSID pSID2 )
{
	/*
		<0 => SID1 < SID2
		0 => SID1 == SID2
		>0 => SID1 > SID2
	*/
	unsigned char * Ptr1 = (unsigned char *) pSID1;
	unsigned char * Ptr2 = (unsigned char *) pSID2;
	
	if (Ptr1[1] != Ptr2[1])
	{
		if (Ptr1[1] < Ptr2[1])
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	return memcmp( Ptr1, Ptr2, 4*(2+Ptr1[1]) );
}
static void AddToList( ACLCLink * pLink, ACLCLink * pBefore, ACLCLink ** ppHead, ACLCLink ** ppTail )
{
	if (pBefore)
	{
		pLink->pPrev = pBefore->pPrev;
		if (pLink->pPrev)
		{
			pLink->pPrev->pNext = pLink;
		}
		else
		{
			*ppHead = pLink;
		}
		pBefore->pPrev = pLink;
		pLink->pNext = pBefore;
	}
	else
	{
		// add to end
		if (*ppTail)
		{
			(*ppTail)->pNext = pLink;
			pLink->pPrev = *ppTail;
			pLink->pNext = NULL;
			*ppTail = pLink;
		}
		else
		{
			*ppHead = pLink;
			*ppTail = pLink;
			pLink->pNext = NULL;
			pLink->pPrev = NULL;
		}
	}
}
static ACLCLink * FindACL( PACL pACL, int *Add )
{
	ACLCLink * pLink;
	ACLCLink * pLink2 = NULL;
	ACLCLink * pNewLink;
	int Code;
	// returns NULL if not found and Add == NULL
	// returns NULL if Add != NULL and out of memory
	// returns link if Add == NULL and found in list
	// returns link if Add != NULL and sets *Add to 0 if found, 1 if not (new entry)
	pLink = ACLHead;
	while (pLink)
	{
		Code = CompareACL( pACL, pLink->ACL );
		if (Code == 0)
		{
			if (Add)
			{
				*Add = 0;
			}
			return pLink;
		}
		else if (Code < 0)
		{
			if (Add)
			{
				/* add before pLink */
				pLink2 = pLink;
				pLink = NULL;
			}
		}
		if (pLink)
		{
			pLink = pLink->pNext;
		}
	}
	if (Add)
	{
		/* add before pLink2 (or at end) */
		if (EXAPATCH_SUCCESS == ExaMemAlloc( NULL, sizeof( ACLCLink ), (void **) &pNewLink ) )
		{
			memset( pNewLink, 0, sizeof(ACLCLink) );
			if (EXAPATCH_SUCCESS == ExaMemAlloc( NULL, pACL->AclSize, (void **) &pNewLink->ACL ) )
			{
				memmove( pNewLink->ACL, pACL, pACL->AclSize );
				AddToList( pNewLink, pLink2, &ACLHead, &ACLTail );
				*Add = 1;
				return pNewLink;
			}
			else
			{
				ExaMemFree( NULL, pNewLink );
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}
static ACLCLink * FindSID( PSID pSID, int *Add )
{
	// returns NULL if not found and Add == NULL
	ACLCLink * pLink;
	ACLCLink * pLink2 = NULL;
	ACLCLink * pNewLink;
	DWORD dwSidSize;
	int Code;
	// returns NULL if not found and Add == NULL
	// returns NULL if Add != NULL and out of memory
	// returns link if Add == NULL and found in list
	// returns link if Add != NULL and sets *Add to 0 if found, 1 if not (new entry)
	pLink = SIDHead;
	dwSidSize = 4*(2+((DWORD)(*(((unsigned char *)pSID)+1))));
	while (pLink)
	{
		Code = CompareSID( pSID, pLink->SID );
		if (Code == 0)
		{
			if (Add)
			{
				*Add = 0;
			}
			return pLink;
		}
		else if (Code < 0)
		{
			if (Add)
			{
				/* add before pLink */
				pLink2 = pLink;
				pLink = NULL;
			}
		}
		if (pLink)
		{
			pLink = pLink->pNext;
		}
	}
	if (Add)
	{
		/* add before pLink2 (or at end) */
		if (EXAPATCH_SUCCESS == ExaMemAlloc( NULL, sizeof( ACLCLink ), (void **) &pNewLink ) )
		{
			memset( pNewLink, 0, sizeof(ACLCLink) );
			pNewLink->dwType = 1;
			if (EXAPATCH_SUCCESS == ExaMemAlloc( NULL, dwSidSize, (void **) &pNewLink->SID ) )
			{
				memmove( pNewLink->SID, pSID, dwSidSize );
				AddToList( pNewLink, pLink2, &SIDHead, &SIDTail );
				*Add = 1;
				return pNewLink;
			}
			else
			{
				ExaMemFree( NULL, pNewLink );
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}
static ACLCLink * FindMask( PACL pACL, PSID pSID, int *Add )
{
	ACLCLink * pACLLink;
	ACLCLink * pSIDLink;
	ACLCLink * pMaskLink;
	ACLCLink * pMaskLink2 = NULL;
	ACLCLink * pNewLink;
	int Code;
	// returns NULL if not found and Add == NULL
	// returns NULL if Add != NULL and out of memory
	// returns link if Add == NULL and found in list
	// returns link if Add != NULL and sets *Add to 0 if found, 1 if not (new entry)
	pACLLink = FindACL( pACL, Add );
	if (pACLLink)
	{
		pSIDLink = FindSID( pSID, Add );
		if (pSIDLink)
		{
			// we've got links to the ACL and the SID - now run the relevant portion of the Mask list
			if (pACLLink->SID)
			{
				pMaskLink = (ACLCLink *) pACLLink->SID;
				while (pMaskLink)
				{
					if (0 == CompareACL(pACL, ((ACLCLink *)(pMaskLink->ACL))->ACL))
					{
						// ACL is OK
						Code = CompareSID( pSID, ((ACLCLink *)(pMaskLink->SID))->SID);
						if (Code == 0)
						{
							if (Add)
							{
								*Add = 0;
							}
							return pMaskLink;
						}
						else if (Code < 0)
						{
							// passed the point where this mask should be
							pMaskLink2 = pMaskLink;
							pMaskLink = NULL;
						}
					}
					else
					{
						// reached the end of the masks associated with this ACL
						pMaskLink2 = pMaskLink;
						pMaskLink = NULL;
					}
					if (pMaskLink)
					{
						pMaskLink = pMaskLink->pNext;
					}
				}
				if (Add)
				{
					if (EXAPATCH_SUCCESS == ExaMemAlloc( NULL, sizeof( ACLCLink ), (void **) &pNewLink ) )
					{
						memset( pNewLink, 0, sizeof( ACLCLink ) );
						pNewLink->dwType = 2;
						pNewLink->ACL = (PACL) pACLLink;
						pNewLink->SID = (PSID) pSIDLink;
						if (pMaskLink2 == (ACLCLink *) pACLLink->SID)
						{
							// we're inserting the new mask link before the first
							// entry associated with this ACL - update the list head
							pACLLink->SID = (PSID) pNewLink;
						}
						AddToList(pNewLink, pMaskLink2, &MaskHead, &MaskTail );
						*Add = 1;
						return pNewLink;
					}
				}
				return NULL;
			}
			else
			{
				// first mask added with this ACL
				if (Add)
				{
					if (EXAPATCH_SUCCESS == ExaMemAlloc( NULL, sizeof(ACLCLink), (void **) &pNewLink ) )
					{
						memset( pNewLink, 0, sizeof( ACLCLink ) );
						pNewLink->dwType = 2;
						pNewLink->ACL = (PACL) pACLLink;
						pNewLink->SID = (PSID) pSIDLink;
						pACLLink->SID = (PSID) pNewLink;
						AddToList( pNewLink, NULL, &MaskHead, &MaskTail );
						*Add = 1;
						return pNewLink;
					}
				}
				return NULL;
			}
		}
	}
	return NULL;
}
static int CachedGERFA( PACL pACL, TRUSTEE_A * pTR, DWORD * pAM )
{
	int Add;
	int Code=0;
	ACLCLink * pLink;
	
	if (EXAPATCH_SUCCESS == ExaStreamAclCacheQuery())
	{
		pLink = FindMask( pACL, pTR->ptstrName, &Add );
		if (pLink)
		{
			// cache entry found
			if (Add)
			{
				// new cache entry
				Code = GetEffectiveRightsFromAclA( pACL, pTR, pAM );
				pLink->dwMask = *pAM;
			}
			else
			{
				// old cache entry
				Code = 0;
				*pAM = pLink->dwMask;
			}
		}
		else
		{
			// cache failed (probably out of memory) - try to bypass cache anyway
			Code = GetEffectiveRightsFromAclA( pACL, pTR, pAM );
		}
		ExaStreamAclCacheOff();
		return Code;
	}
	else
	{
		// cache is off - use OS API
		return(GetEffectiveRightsFromAclA( pACL, pTR, pAM ) );
	}
}
#  else
/* not implemented on non-Win32 platforms */
int ExaStreamAclCacheOn( void )
{
	return EXAPATCH_NOT_SUPPORTED;
}
int ExaStreamAclCacheOff( void )
{
	return EXAPATCH_SUCCESS;
}
int ExaStreamAclCacheQuery( void )
{
	return EXAPATCH_UNSPECIFIED_ERROR;
}
#  endif
# endif
# ifndef ATTOPATCH
/*
Attributes are:
0x1 = Read Only	(non-Win32 also)
0x2 = Hidden
0x4 = System
0x8 = Security
0x10 = Directory (non-Win32 also)
0x20 = Unused
0x40 = Encrypted
0x80 = Shareable (not supported any more)
0x100 = Unused
0x200 = Sparse
0x400 = Reparse point
0x800 = Compressed
0x2000 = Index
0xdff0000 = Unix-style permissions
*/
static DWORD GetAttributeFromFileHandle( HANDLE hFile )
{
	DWORD RetVal = 0;
# ifdef _WIN32
	BY_HANDLE_FILE_INFORMATION FInfo;

	if (GetFileInformationByHandle( hFile, &FInfo ))
	{
		// Get FAT-style attribs
		RetVal = FInfo.dwFileAttributes & EXP_ATTRIBUTE_CHECKED;
		// synthesize permissions from FAT-style attribs
		RetVal |= 0x1ff0000U;	// ignore setgid/setuid
# ifndef WINCE
		if (0 == (0x80000000U & GetVersion()))
		{
	    PSID	       		WorldSID, GroupSID, OwnerSID;
	    PSECURITY_DESCRIPTOR	pSD;
	    PACL			TheACL;
	    ACL_SIZE_INFORMATION	ACLSize;
	    //DWORD			TheACEIndex;
	    SID_IDENTIFIER_AUTHORITY	SIA=SECURITY_WORLD_SID_AUTHORITY;
	    //ACCESS_ALLOWED_ACE		*TheACE;
	    DWORD			RWX;


	    if (ERROR_SUCCESS == GetSecurityInfo( hFile, 
															SE_FILE_OBJECT,
												    	OWNER_SECURITY_INFORMATION 
															| GROUP_SECURITY_INFORMATION
															| DACL_SECURITY_INFORMATION,
															&OwnerSID,
															&GroupSID,
															&TheACL,
															NULL,
															&pSD ))
			{
				if (AllocateAndInitializeSid(&SIA, 1,0,0,0,0,0,0,0,0,&WorldSID)
					&& TheACL
					&& GetAclInformation(TheACL, (PVOID) &ACLSize, sizeof(ACLSize),AclSizeInformation))
				{

	        
					/*
					 This method doesn't always work because it doesn't take group membership
					 into account - use GetEffectiveRightsFromAcl instead
					*/
					/*
					RetVal &= 0xffff;
					for (TheACEIndex = 0; TheACEIndex < ACLSize.AceCount; TheACEIndex++ )
					{
						GetAce( TheACL, TheACEIndex, &TheACE );
				    RWX = ((TheACE->Mask & 1) << 2) | (TheACE->Mask & 2)
				    	| ((TheACE->Mask & 0x20) >> 5);
				    if ( EqualSid( WorldSID, (PSID) &(TheACE->SidStart) ) )
				    {
				      if ( TheACE->Header.AceType == ACCESS_ALLOWED_ACE_TYPE )
				      {
				        RetVal |= RWX << 16;
				      }
				      else
				      {
				        RetVal &= ~(RWX << 16);
				      }
				    }
				    else if ( EqualSid( GroupSID, (PSID) &(TheACE->SidStart) ) )
				    {
				      if ( TheACE->Header.AceType == ACCESS_ALLOWED_ACE_TYPE )
				      {
				        RetVal |= RWX << 19;
				      }
				      else
				      {
				        RetVal &= ~( RWX << 19);
				      }
				    }
				    else if ( EqualSid( OwnerSID, (PSID) &(TheACE->SidStart) ) )
				    {
				      if ( TheACE->Header.AceType == ACCESS_ALLOWED_ACE_TYPE )
				      {
				        RetVal |= (RWX << 22);
				      }
				      else
				      {
				        RetVal &= ~(RWX << 22);
				      }
				    }
					}
					*/
					/*
					 here we have all the WGO SIDs and a valid non-NULL DACL
					 first, kill all the current permissions
					*/
					
					ACCESS_MASK AM;
					TRUSTEE_A TR;
					RetVal &= 0x100ffff; // make sure that the owner can always *read* the file
					TR.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
					TR.pMultipleTrustee = NULL;
					TR.TrusteeForm = TRUSTEE_IS_SID;
					TR.ptstrName = WorldSID;
					TR.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
# ifdef EXAPATCH_ACLCACHE_SUPPORT
					CachedGERFA( TheACL, &TR, &AM );
# else
					GetEffectiveRightsFromAclA( TheACL, &TR, &AM);
# endif
					RWX = ((AM & 1) << 2) | (AM & 2) | ((AM & 0x20) >> 5);
					RetVal |= (RWX << 16);
					TR.TrusteeType = TRUSTEE_IS_GROUP;
					TR.ptstrName = GroupSID;
# ifdef EXAPATCH_ACLCACHE_SUPPORT
					CachedGERFA( TheACL, &TR, &AM );
# else
					GetEffectiveRightsFromAclA( TheACL, &TR, &AM);
# endif
					RWX = ((AM & 1) << 2) | (AM & 2) | ((AM & 0x20) >> 5);
					RetVal |= (RWX << 19);
					TR.TrusteeType = TRUSTEE_IS_USER;
					TR.ptstrName = OwnerSID;
# ifdef EXAPATCH_ACLCACHE_SUPPORT
					CachedGERFA( TheACL, &TR, &AM );
# else
					GetEffectiveRightsFromAclA( TheACL, &TR, &AM);
# endif
					RWX = ((AM & 1) << 2) | (AM & 2) | ((AM & 0x20) >> 5);
					RetVal |= (RWX << 22);
					FreeSid( WorldSID );
				}
				LocalFree( pSD );
			}
		    /*
			 reasonablize the permissions
			 theoretically, this shouldn't be needed anymore, 
			   but the Department of Redundancy Department insists...
			*/
	    RetVal |= ((RetVal & (EXP_ATTRIBUTE_WORLD | EXP_ATTRIBUTE_GROUP)) << 3) | ((RetVal & EXP_ATTRIBUTE_WORLD) << 6);
		}
# endif /* !WINCE */
		if (RetVal & 1)
		{
			RetVal &= 0x124ffffU;
		}
	}
# else
	struct stat SS;
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
	if (0 == fstat((int) hFile, &SS))
	{
		RetVal = ((DWORD)(SS.st_mode & 0xdff)) << 16;
		/* synthesize FAT-style attributes from permissions */
		if (0 == (SS.st_mode & S_IWUSR))
			RetVal |= 0x1;
		if (SS.st_mode & S_IFDIR)
			RetVal |= 0x10;
	}
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
# endif
	return(RetVal);
}
int GetExaStreamAttrib( ExaPatchStream * Stream, ExaPatchFileAttrib * Attrib )
{
	INT Code = EXAPATCH_SUCCESS;
# ifdef _WIN32
  LARGE_INTEGER ll;
# else
	long long ll;
# endif

	switch ( Stream->Type & EXP_STREAM_TYPE_BITS)
	{
# ifdef _WIN32
		BY_HANDLE_FILE_INFORMATION FInfo;
# else
#  ifdef SEEK64
		struct stat64 SS;
#	 else
		struct stat SS;
#	 endif
# endif

		case EXP_STREAM_FILE:
#  ifdef _WIN32
			Code = GetFileInformationByHandle( Stream->FileHandle, &FInfo );
      if (Code)
      {
        Code = EXAPATCH_SUCCESS;
      }
      else
      {
        Code = EXAPATCH_GET_ATTRIB_FAILED;
      }
			if (EXP_ATTRIB_ATTRIBUTE & Attrib->Flags)
			{
				Attrib->Attributes = GetAttributeFromFileHandle( Stream->FileHandle );
			}
			if (EXP_ATTRIB_CHANGED_DATE & Attrib->Flags)
      {
        ll.LowPart = FInfo.ftLastWriteTime.dwLowDateTime;
        ll.HighPart = FInfo.ftLastWriteTime.dwHighDateTime;
        ll.QuadPart /= 10000; /* we use msec */
        ll.QuadPart -= EPOCH_CHANGE; /* we use Jan. 1, 1901 epoch */
				Attrib->ChangedTime.dwLowDateTime = ll.LowPart;
        Attrib->ChangedTime.dwHighDateTime = ll.HighPart;
      }
			if (EXP_ATTRIB_CREATE_DATE & Attrib->Flags)
      {
        ll.LowPart = FInfo.ftCreationTime.dwLowDateTime;
        ll.HighPart = FInfo.ftCreationTime.dwHighDateTime;
        ll.QuadPart /= 10000; /* we use msec */
        ll.QuadPart -= EPOCH_CHANGE; /* we use Jan. 1, 1901 epoch */
				Attrib->CreateTime.dwLowDateTime = ll.LowPart;
        Attrib->CreateTime.dwHighDateTime = ll.HighPart;
      }
			if (EXP_ATTRIB_SIZE & Attrib->Flags)
			{
				ll.LowPart = FInfo.nFileSizeLow;
				ll.HighPart = FInfo.nFileSizeHigh;
				Attrib->Size = ll.QuadPart;
			}
#  else
			/* Non-Win32 */
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
#   ifdef SEEK64
			/* Again, no ANSI solution for this */
			if (0 == fstat64( (INT) Stream->FileHandle, &SS ))
#   else
			if (0 == fstat( (INT) Stream->FileHandle, &SS ))
#   endif
			{
				Code = EXAPATCH_SUCCESS;
				if (EXP_ATTRIB_SIZE & Attrib->Flags)
				{
#   ifdef SEEK64
					Attrib->Size = SS.st_size;
#   else
					Attrib->Size = (QWORD) SS.st_size;
#   endif
				}
				if (EXP_ATTRIB_ATTRIBUTE & Attrib->Flags)
				{
					Attrib->Attributes = GetAttributeFromFileHandle( Stream->FileHandle );
				}
				if (EXP_ATTRIB_CREATE_DATE & Attrib->Flags)
				{
					/* Always return 0 here */
					Attrib->CreateTime.dwLowDateTime = 0;
					Attrib->CreateTime.dwHighDateTime = 0;
				}
				if (EXP_ATTRIB_CHANGED_DATE & Attrib->Flags)
				{
					/* Unix time DWORD(Jan. 1, 1970; sec) */
					/* Our time QWORD(Jan. 1, 1901; ms) */
					ll = ((long long) SS.st_mtime) + TIME_T_EPOCH_CHANGE;
					ll *= (long long) 1000;
					Attrib->ChangedTime.dwLowDateTime = (DWORD) (ll & (long long)0xffffffffU);
					Attrib->ChangedTime.dwHighDateTime = (DWORD) (ll >> 32);
				}
			}
			else
			{
				Code = EXAPATCH_GET_ATTRIB_FAILED;
			}
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
#  endif
			break;

		case EXP_STREAM_USER:
			if (Stream->AttribGet)
			{
				Code = Stream->AttribGet( Stream->FileHandle, Attrib);
			}
			else
			{
				Code = EXAPATCH_GET_ATTRIB_FAILED;
			}
			break;

		case EXP_STREAM_COMPOSITE:
		case EXP_STREAM_GAP:
		case EXP_STREAM_SPECIAL_GAP:
			if (EXP_ATTRIB_ATTRIBUTE & Attrib->Flags)
				Attrib->Attributes = 0;
			if (EXP_ATTRIB_CHANGED_DATE & Attrib->Flags)
				memset( &Attrib->ChangedTime, 0, sizeof( FILETIME ) );
			if (EXP_ATTRIB_CREATE_DATE & Attrib->Flags)
				memset( &Attrib->CreateTime, 0, sizeof( FILETIME ) );
			if (EXP_ATTRIB_SIZE & Attrib->Flags)
				Attrib->Size = Stream->Size;
			break;
	}
	return(Code);
}


/* This is really pretty useless! Maybe I should kill it... */
INT SetExaStreamAttrib( ExaPatchStream * Stream, const ExaPatchFileAttrib * Attrib )
{
	INT Code = EXAPATCH_SUCCESS;
# ifdef _WIN32
	DWORD Pos;
	QWORD QPos;
	FILETIME ChgTime;
	FILETIME CrtTime;
  LARGE_INTEGER ll;
# endif

	switch ( Stream->Type & EXP_STREAM_TYPE_BITS)
	{
		case EXP_STREAM_FILE:
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
#  ifdef _WIN32
			if ((EXP_ATTRIB_CREATE_DATE | EXP_ATTRIB_CHANGED_DATE) & Attrib->Flags)
			{
				/* set some times... */
        if (Attrib->Flags & EXP_ATTRIB_CHANGED_DATE)
        {
          ll.HighPart = Attrib->ChangedTime.dwHighDateTime;
          ll.LowPart = Attrib->ChangedTime.dwLowDateTime;
          ll.QuadPart += EPOCH_CHANGE; /* we use Jan. 1, 1901 epoch */
          ll.QuadPart *= 10000; /* we use msec units */
          ChgTime.dwHighDateTime = ll.HighPart;
          ChgTime.dwLowDateTime = ll.LowPart;
        }
        if (Attrib->Flags & EXP_ATTRIB_CREATE_DATE)
        {
          ll.HighPart = Attrib->CreateTime.dwHighDateTime;
          ll.LowPart = Attrib->CreateTime.dwLowDateTime;
          ll.QuadPart += EPOCH_CHANGE; /* we use Jan. 1, 1901 epoch */
          ll.QuadPart *= 10000; /* we use msec units */
          CrtTime.dwHighDateTime = ll.HighPart;
          CrtTime.dwLowDateTime = ll.LowPart;
        }
				//ChgTime = Attrib->ChangedTime;
				//CrtTime = Attrib->CreateTime;
				Code = SetFileTime( Stream->FileHandle, 
					(Attrib->Flags & EXP_ATTRIB_CREATE_DATE)?(&CrtTime):NULL, 
					NULL,
					(Attrib->Flags & EXP_ATTRIB_CHANGED_DATE)?(&ChgTime):NULL );
				if (Code == 0)
				{
					Code = EXAPATCH_SET_ATTRIB_FAILED;
				}
				else
				{
					Code = EXAPATCH_SUCCESS;
				}
			}
			if (Code == EXAPATCH_SUCCESS && (EXP_ATTRIB_SIZE & Attrib->Flags) )
			{
				Code = EXAPATCH_SET_ATTRIB_FAILED;
				ll.HighPart = 0;
				Pos = SetFilePointer( Stream->FileHandle, 0, &ll.HighPart, FILE_CURRENT );
				if ((Pos != 0xffffffffU) || (GetLastError() == NO_ERROR))
				{
					ll.LowPart = Pos;
					QPos = ll.QuadPart;
					ll.QuadPart = Attrib->Size;
					if ((0xffffffffU != SetFilePointer( Stream->FileHandle, ll.LowPart, &ll.HighPart, FILE_BEGIN ))
						|| (GetLastError() == NO_ERROR))
					{
						if (0U != SetEndOfFile( Stream->FileHandle ))
						{
							ll.QuadPart = QPos;
							if ((0xffffffffU != SetFilePointer( Stream->FileHandle, ll.LowPart, &ll.HighPart, FILE_BEGIN ))
								|| (GetLastError() == NO_ERROR))
							{
								Code = EXAPATCH_SUCCESS;
							}
						}
					}
				}
			}
			if (EXP_ATTRIB_ATTRIBUTE & Attrib->Flags)
			{
				/* can't set attributes on a File-type Stream! */
				Code = EXAPATCH_SET_ATTRIB_FAILED;
			}
#  else
			/* Non-Win32 */
			/*use utime, ftruncate */
			if ((EXP_ATTRIB_ATTRIBUTE | EXP_ATTRIB_CREATE_DATE) & Attrib->Flags)
			{
				/* Can't set these under non-Win32 */
				Code = EXAPATCH_SET_ATTRIB_FAILED;
			}
			if ((Code == EXAPATCH_SUCCESS) && (EXP_ATTRIB_SIZE & Attrib->Flags))
			{
				if (Attrib->Size > ((QWORD)0x7fffffff))
				{
#   ifdef SEEK64
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
					/* REALLY non-portable solution!!! */
					if (ftruncate((INT)(Stream->FileHandle), (off64_t) Attrib->Size))
					{
						Code = EXAPATCH_SET_ATTRIB_FAILED;
					}
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
#   else
					Code = EXAPATCH_SET_ATTRIB_FAILED;
#   endif
				}
				else
				{
					/* No truly portable solution here... */
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
					if (ftruncate((INT)(Stream->FileHandle) , (off_t) Attrib->Size))
					{
						Code = EXAPATCH_SET_ATTRIB_FAILED;
					}
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
				}
			}
			if ((Code == EXAPATCH_SUCCESS) && (EXP_ATTRIB_CHANGED_DATE & Attrib->Flags))
			{
				/* This isn't really portable either... */
#   ifdef FUTIME_SUPPORTED
				struct utimbuf UT;
				struct stat SS;
				unsigned long long TheTime;

				TheTime = (((unsigned long long) Attrib->ChangedTime.dwHighDateTime) << 32) 
					| ((unsigned long long) Attrib->ChangedTime.dwLowDateTime);

#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
				if (0 == fstat((INT) Stream->FileHandle, &SS))
				{
					UT.actime = SS.st_atime;
					UT.modtime = (time_t)((TheTime/((QWORD) 1000)) - TIME_T_EPOCH_CHANGE);
					if (futime((INT) Stream->FileHandle, &UT))
					{
						Code = EXAPATCH_SET_ATTRIB_FAILED;
					}
				}
				else
				{
					Code = EXAPATCH_SET_ATTRIB_FAILED;
				}
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
#   else
				Code = EXAPATCH_SET_ATTRIB_FAILED;
#   endif
			}
#  endif
# else
			Code = EXAPATCH_NOT_SUPPORTED;
# endif
			break;

		case EXP_STREAM_USER:
			if (Stream->AttribSet)
			{
				Code = Stream->AttribSet( Stream->FileHandle, Attrib);
			}
			else
			{
				Code = EXAPATCH_SET_ATTRIB_FAILED;
			}
			break;

		case EXP_STREAM_COMPOSITE:
		case EXP_STREAM_GAP:
		case EXP_STREAM_SPECIAL_GAP:
			/* can't set ANY attribs on these guys */
			Code = EXAPATCH_SET_ATTRIB_FAILED;
			break;
	}
	return(Code);
	
}
# endif /* ATTOPATCH */

int CloseExaStream( ExaPatchStream * Stream )
{
# ifdef EXAPATCH_COMPOSITESTREAM_SUPPORT
	ExaPatchStream * ThisCpt;
	ExaPatchStream * PrevCpt;
	ExaPatchStream * OtherCpt;
# endif

	if (NULL == Stream)
	{
		return(EXAPATCH_SUCCESS); /* MISRA C 2012 [15.5]: return used for quick exit in exceptional condition  */
	}
		
	switch (Stream->Type & EXP_STREAM_TYPE_BITS)
	{
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
		case EXP_STREAM_FILE:
      if (Stream->FileHandle != INVALID_HANDLE_VALUE)
      {
				ExaClose( Stream->FileHandle );
        Stream->FileHandle = INVALID_HANDLE_VALUE;
      }
			break;
# endif
		case EXP_STREAM_COMPOSITE:
# ifdef EXAPATCH_COMPOSITESTREAM_SUPPORT
			/* make sure each file is only closed once */
			ThisCpt = Stream->Link;
			while (ThisCpt)
			{
				if (((ThisCpt->Type & EXP_STREAM_TYPE_BITS) == EXP_STREAM_FILE)
					&& (ThisCpt->FileHandle != INVALID_HANDLE_VALUE))
				{
					OtherCpt = ThisCpt->Link;
					while (OtherCpt)
					{
						if (((OtherCpt->Type & EXP_STREAM_TYPE_BITS) == EXP_STREAM_FILE)
							&& (OtherCpt->FileHandle == ThisCpt->FileHandle))
						{
							OtherCpt->FileHandle = INVALID_HANDLE_VALUE;
						}
						OtherCpt = OtherCpt->Link;
					}
				}
				ThisCpt = ThisCpt->Link;
			}
			ThisCpt = Stream->Link;
			while (ThisCpt)
			{
				CloseExaStream( ThisCpt );
				PrevCpt = ThisCpt;
				ThisCpt = ThisCpt->Link;
				ExaMemFree( NULL, PrevCpt );
			}
      Stream->Link = Stream->CurCpt = NULL;
# else
			return(EXAPATCH_NOT_SUPPORTED); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
# endif
			break; /* MISRA C 2012 [2.1]: unreachable code (alternative is MISRA switch syntax departure) */

		case EXP_STREAM_USER:
			if (NULL != Stream->Close)
			{
				(void)Stream->Close(Stream->FileHandle);
        Stream->Close = NULL;
			}
			break;

		default:
			/* close on unsupported stream type reports success (presumably we couldn't have really opened it) */
			break;
	}
	return(EXAPATCH_SUCCESS);
	
}

/*
NOTE: with EXAPATCH_DEFAULTSTREAM_SUPPORT turned OFF, MakeExaStreamFromFileArray with NumHandles still works,
as do CloseExaStream and GetExaStreamAttrib - that's because they are used in the file verification routines.
Checksum will not work since that uses default stream read support
*/
# ifndef ATTOPATCH
INT MakeExaStreamFromFileArray( ExaPatchStream * Stream, DWORD NumHandles, HANDLE * HandlePtr, QWORD * SizePtr )
{
# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
	ExaPatchStream * ThisCpt;
	ExaPatchStream * PrevCpt;
	INT Code;
	DWORD i;
# endif
# ifdef _WIN32
	LARGE_INTEGER ll;
# endif

# ifdef EXAPATCH_DEFAULTSTREAM_SUPPORT
	if (NumHandles > 1)
	{
		memset( Stream, 0, sizeof( ExaPatchStream ) );
		Stream->Type = EXP_STREAM_COMPOSITE;
		PrevCpt = Stream;
		Stream->Size = 0;
		for (i=0; i<NumHandles ; i++ )
		{
			Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) &ThisCpt); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (Code != EXAPATCH_SUCCESS)
			{
				return(Code);
			}
			PrevCpt->Link = ThisCpt;
			memset( ThisCpt, 0, sizeof( ExaPatchStream ) );
			ThisCpt->FileHandle = HandlePtr[i];
			ThisCpt->Type = EXP_STREAM_FILE;
			if (SizePtr)
			{
				ThisCpt->Size = SizePtr[i];
			}
			else
			{
#  ifdef _WIN32
				ll.LowPart = GetFileSize( ThisCpt->FileHandle, &ll.HighPart );
				ThisCpt->Size = ll.QuadPart;
#  else
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
#   ifdef SEEK64
				/* Non-portable!!! */
				struct stat64 SS;

				fstat64( (INT) ThisCpt->FileHandle, &SS );
#   else
				struct stat SS;
				fstat( (INT) ThisCpt->FileHandle, &SS );
#   endif
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
				ThisCpt->Size = (QWORD) SS.st_size;
#  endif
			}
			ThisCpt->CompositeOrigin = Stream->Size;
			Stream->Size += ThisCpt->Size;
			PrevCpt = ThisCpt;
		}
	}
	else
	{
		memset( Stream, 0, sizeof( ExaPatchStream ) );
		Stream->FileHandle = *HandlePtr;
		Stream->Type = EXP_STREAM_FILE;
		if (SizePtr)
		{
			Stream->Size = *SizePtr;
		}
		else
		{
#  ifdef _WIN32
			ll.LowPart = GetFileSize( Stream->FileHandle, &ll.HighPart );
			Stream->Size = ll.QuadPart;
#  else
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#  endif
#   ifdef SEEK64
			/* Non-portable!!! */
			struct stat64 SS;

			fstat64( (INT) Stream->FileHandle, &SS );
#   else
			struct stat SS;
			fstat( (INT) Stream->FileHandle, &SS );
#   endif
#  if GCC_VERSION > 40600
#   pragma GCC diagnostic pop
#  endif
			Stream->Size = (QWORD) SS.st_size;
#  endif
		}
	}
	return(EXAPATCH_SUCCESS);
# else
	if (NumHandles > 1)
	{
		return(EXAPATCH_NOT_SUPPORTED);
	}
	else
	{
		memset( Stream, 0, sizeof( ExaPatchStream ) );
		Stream->FileHandle = *HandlePtr;
		Stream->Type = EXP_STREAM_FILE;
		if (SizePtr)
		{
			Stream->Size = *SizePtr;
		}
		else
		{
#  ifdef _WIN32
			ll.LowPart = GetFileSize( Stream->FileHandle, &ll.HighPart );
			Stream->Size = ll.QuadPart;
#  else
#   ifdef SEEK64
			/* Non-portable!!! */
			struct stat64 SS;

			fstat64( (INT) Stream->FileHandle, &SS );
#   else
			struct stat SS;
			fstat( (INT) Stream->FileHandle, &SS );
#   endif
			Stream->Size = (QWORD) SS.st_size;
#  endif
		}
	}
	return(EXAPATCH_SUCCESS);
# endif
}
# endif

# ifdef EXAPATCH_COMPOSITESTREAM_SUPPORT
INT ForceExaStreamZeroes( ExaPatchStream * Stream, QWORD Pos, QWORD Size, QWORD * SpecialPosPtr )
{
	ExaPatchStream * FirstCpt;
	ExaPatchStream * LastCpt;
	ExaPatchStream * MedCpt;
	ExaPatchStream * NextCpt;
  ExaPatchStream * PrevCpt;
	DWORD NumSkips=0;
	DWORD i;
	INT Code;

	if (Pos + Size > Stream->Size)
	{
		return(EXAPATCH_INVALID_PARAMETER);
	}

	if (EXP_STREAM_COMPOSITE != (Stream->Type & EXP_STREAM_TYPE_BITS))
	{
		/* make the stream composite if it isn't already */
		Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) &Stream->Link ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code != EXAPATCH_SUCCESS)
		  return(Code);
		memcpy( Stream->Link, Stream, sizeof( ExaPatchStream ) );
		Stream->Link->Link = NULL;
		Stream->Type = (EXP_STREAM_BUFFERED & Stream->Type) | EXP_STREAM_COMPOSITE;
		Stream->Link->Type &= EXP_STREAM_TYPE_BITS;
	}
	FirstCpt = Stream->Link;
	while ( FirstCpt && ((FirstCpt->CompositeOrigin + FirstCpt->Size) <= Pos ) )
	{
		FirstCpt = FirstCpt->Link;
	}
	LastCpt = FirstCpt;
	while ( LastCpt && ((LastCpt->CompositeOrigin + LastCpt->Size) < (Pos + Size)) )
	{
		NumSkips++;
		LastCpt = LastCpt->Link;
	}
	if (LastCpt == NULL)
	{
		return(EXAPATCH_INTERNAL_ERROR);
	}
	if (NumSkips == 0)
	{
		/* split a single component into 3 */
		Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **)&MedCpt ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) &LastCpt ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code != EXAPATCH_SUCCESS)
			return(Code);
		memcpy( LastCpt, FirstCpt, sizeof( ExaPatchStream ) );	
		memset( MedCpt, 0, sizeof( ExaPatchStream ) );
		MedCpt->Link = LastCpt;
		FirstCpt->Link = MedCpt;
		FirstCpt->Size = Pos - FirstCpt->CompositeOrigin;
		MedCpt->Type = SpecialPosPtr?EXP_STREAM_SPECIAL_GAP:EXP_STREAM_GAP;
		if (SpecialPosPtr)
			MedCpt->SpecialGapPos = *SpecialPosPtr;
		MedCpt->CompositeOrigin = Pos;
		MedCpt->Size = Size;
		LastCpt->FileOrigin += (Pos + Size - FirstCpt->CompositeOrigin);
		LastCpt->Size -= (Pos + Size - FirstCpt->CompositeOrigin);
		LastCpt->CompositeOrigin = Pos + Size;
	}
	else if (NumSkips == 1)
	{
		/* split two components into 3 */
		Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) &MedCpt ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code != EXAPATCH_SUCCESS)
			return(Code);
		memset( MedCpt, 0, sizeof( ExaPatchStream ) );
		FirstCpt->Link = MedCpt;
		MedCpt->Link = LastCpt;
		MedCpt->Type = SpecialPosPtr?EXP_STREAM_SPECIAL_GAP:EXP_STREAM_GAP;
		if (SpecialPosPtr)
			MedCpt->SpecialGapPos = *SpecialPosPtr;
		MedCpt->CompositeOrigin = Pos;
		MedCpt->Size = Size;
		FirstCpt->Size = Pos - FirstCpt->CompositeOrigin;
		LastCpt->FileOrigin += (Pos + Size - LastCpt->CompositeOrigin);
		LastCpt->Size -= (Pos + Size - LastCpt->CompositeOrigin);
		LastCpt->CompositeOrigin = Pos + Size;
	}
	else
	{
		/* no additional components necessary:  */
		/*	NumSkips-2 components will be removed from the chain */
		MedCpt = FirstCpt->Link;
		NextCpt = MedCpt->Link;
		for (i=2; i<NumSkips ; i++ )
		{
			MedCpt = NextCpt;
			NextCpt = MedCpt->Link;
			ExaMemFree( NULL, MedCpt );
		}
		MedCpt = FirstCpt->Link;
		MedCpt->Link = LastCpt;
		MedCpt->Type = SpecialPosPtr?EXP_STREAM_SPECIAL_GAP:EXP_STREAM_GAP;
		if (SpecialPosPtr)
			MedCpt->SpecialGapPos = *SpecialPosPtr;
		MedCpt->CompositeOrigin = Pos;
		MedCpt->Size = Size;
		FirstCpt->Size = Pos - FirstCpt->CompositeOrigin;
		LastCpt->FileOrigin += (Pos + Size - LastCpt->CompositeOrigin);
		LastCpt->Size -= (Pos + Size - LastCpt->CompositeOrigin);
		LastCpt->CompositeOrigin = Pos + Size;
	}
	/* remove any zero-length components from the chain */
	for (FirstCpt = Stream->Link, PrevCpt = Stream; 
		FirstCpt ; 
		FirstCpt = FirstCpt->Link )
	{
		if (FirstCpt->Size)
		{
			PrevCpt = FirstCpt;
		}
		else
		{
			PrevCpt->Link = FirstCpt->Link;
			ExaMemFree( NULL, FirstCpt );
			FirstCpt = PrevCpt;
		}
	}
	return(EXAPATCH_SUCCESS);
}
# else
int ForceExaStreamZeroes( ExaPatchStream * Stream, QWORD Pos, QWORD Size, QWORD * SpecialPosPtr )
{
	(void) Stream;
	(void) Pos;
	(void) Size;
	(void) SpecialPosPtr;
	return(EXAPATCH_NOT_SUPPORTED);
}
# endif
# ifdef ATTOPATCH
int SetExaStreamBuffering( ExaPatchStream * Stream, DWORD Flag )
{
	(void) Stream;
	(void) Flag;
	return(EXAPATCH_NOT_SUPPORTED);
}
# else
INT SetExaStreamBuffering( ExaPatchStream * Stream, DWORD Flag )
{
	if (0U != Flag)
	{
		Stream->Type |= EXP_STREAM_BUFFERED;
	}
	else
	{
		Stream->Type &= ~EXP_STREAM_BUFFERED;
	}
	return(EXAPATCH_SUCCESS);
}
# endif
# ifndef ATTOPATCH
# ifdef _WIN32
typedef DWORD (WINAPI * PQDDW)(wchar_t * lpDevice, wchar_t * lpPath, DWORD ucchMax);				// QueryDosDeviceW (kernel32)
typedef DWORD (WINAPI * PGMFNW)(HANDLE hProc, LPVOID lpv, wchar_t * lpPath , DWORD nSize);	//GetMappedFileNameW (psapi)
typedef DWORD (WINAPI * PGFPNBHW)(HANDLE hFile, wchar_t * lpPath, DWORD nPathSize, DWORD dwFlags); //GetFinalPathNameByHandleW (

static INT GetFileNameFromHandle(HANDLE hFile, wchar_t * pszFileName, DWORD dwSize) 
{
	// boy, is this ever ugly!
  INT nSuccess = FALSE;
  HANDLE hFileMap;
	HMODULE hModK32;
	HMODULE hModPS;
	PQDDW pQDDW = NULL;
	PGMFNW pGMFNW = NULL;
	PGFPNBHW pGFPNBHW = NULL;
	wchar_t RawFileName[300] = L"";
	wchar_t * DeviceBuffer;
	static DWORD dwDBSize = 1000;
	wchar_t Xlat[300];
	wchar_t * Ptr;
	const wchar_t * Ptr2;
	const wchar_t * Ptr3;

	hModK32 = LoadLibraryA("KERNEL32.DLL");
	if (NULL != hModK32)
	{
		pQDDW = (PQDDW) GetProcAddress( hModK32, "QueryDosDeviceW" ); /* MISRA C 2012 [11.1, 11.3]: necessary for Windows dynamic DLL linkage */
		pGFPNBHW = (PGFPNBHW) GetProcAddress( hModK32, "GetFinalPathNameByHandleW" ); /* MISRA C 2012 [11.1, 11.3]: necessary for Windows dynamic DLL linkage */
	}
	hModPS = LoadLibraryA("PSAPI.DLL");
	if (NULL != hModPS)
	{
		pGMFNW = (PGMFNW) GetProcAddress( hModPS, "GetMappedFileNameW" ); /* MISRA C 2012 [11.1, 11.3]: necessary for Windows dynamic DLL linkage */
	}
	if ((NULL != pQDDW) && (NULL != pGMFNW))
	{
		if (NULL != pGFPNBHW)
		{
			//This is critical because under certain circumstances in Windows 7,
			//GetMappedFileName returns an incorrect path.  However, the new
			//Vista+ call appears to work correctly.
			(void)(*pGFPNBHW)(hFile, RawFileName, 299, 2);
		}
		else
		{
			// Get the file size.
			DWORD dwFileSizeHi = 0;
			DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

			if ((0U != dwFileSizeLo) || (0U != dwFileSizeHi))
			{
				// Create a file mapping object.
				hFileMap = CreateFileMappingA(hFile, 
		                    NULL, 
		                    PAGE_READONLY,
		                    0, 
		                    1,
		                    NULL);

				if (NULL != hFileMap) 
				{
					// Create a file mapping to get the file name.
					void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

					if (NULL != pMem) 
					{
						(void)(*pGMFNW) (GetCurrentProcess(), 
		                             pMem, 
		                             RawFileName,
		                             300);
						(void)UnmapViewOfFile(pMem);
					}
					(void)CloseHandle(hFileMap);
				}
			}
		}

		/* see if we can shortcut this process... */
		(void)wcscpy_s(pszFileName,dwSize,L"\\\\?\\GLOBALROOT");
		(void)wcscat_s(pszFileName,dwSize,RawFileName);
		if (0xffffffffU == GetFileAttributesW(pszFileName)) /* INVALID_FILE_ATTRIBUTES not present in all versions of windows.h */
		{
			/*
			 for now, just handle translating DosDevice names
			 allegedly, this may fail for certain network files, but
			 	a) I've never seen it fail, and
			 	b) every network file I've tried, never gets to this point
					(the GLOBALROOT trick works)
			*/
			DeviceBuffer = GlobalAlloc( GMEM_FIXED, dwDBSize*sizeof(wchar_t) ); /* MISRA C 2012 [11.5]: necessary under Windows memory allocation scheme */
			if (NULL != DeviceBuffer)
			{
				while(0U == (*pQDDW)(NULL,DeviceBuffer,dwDBSize))
				{
					dwDBSize *= 2U;
					(void)GlobalFree( DeviceBuffer );
					DeviceBuffer = GlobalAlloc( GMEM_FIXED, dwDBSize*sizeof(wchar_t)); /* MISRA C 2012 [11.5]: necessary under Windows memory allocation scheme */
				}
				if (0U != (*pQDDW)(NULL,DeviceBuffer,dwDBSize))
				{
					Ptr = DeviceBuffer;
					while ((0 == nSuccess) && (L'\0' != *Ptr))
					{
						if (0U != (*pQDDW)(Ptr,Xlat,300))
						{
							Ptr2=Xlat;
							Ptr3=RawFileName;
							while ((L'\0' != *Ptr2) && (L'\0' != *Ptr3) && (towupper((wint_t)*Ptr2) == towupper((wint_t)*Ptr3))) /* MISRA C 2012 [1.3, 13.5]: Some MISRA checkers incorrectly flag this statement */
							{
								Ptr2++;
								Ptr3++;
							}
							if ((*Ptr2 == L'\0') && (*Ptr3 == L'\\'))
							{
								(void)wcscpy_s(pszFileName,dwSize,L"\\\\?\\");
								(void)wcscat_s(pszFileName,dwSize, Ptr);
								(void)wcscat_s(pszFileName,dwSize, Ptr3);
								nSuccess = TRUE;
							}
						}
						Ptr = &Ptr[PSwcslen(Ptr) + 1U];
					}
				}
				(void)GlobalFree( DeviceBuffer );
			}
		}
		else
		{
			nSuccess = TRUE;
		}

	}
	if (NULL != hModK32)
	{
		(void)FreeLibrary( hModK32 );
	}
	if (NULL != hModPS)
	{
		(void)FreeLibrary( hModPS );
	}
	return(nSuccess);
	
}
# endif
/* NOTE: DupFlag == TRUE is only supported under Windows, currently */
INT CloneExaStream( ExaPatchStream ** DestStream, const ExaPatchStream * SrcStream, DWORD DupFlag )
{
	const ExaPatchStream * ThisSrcCpt;
	ExaPatchStream * ThisDestCpt;
	ExaPatchStream * PrevDestCpt;
	INT Code;

	Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) DestStream ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) 
	{
		return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	memcpy( *DestStream, SrcStream, sizeof( ExaPatchStream ) );
	if ((SrcStream->Type & EXP_STREAM_TYPE_BITS) == EXP_STREAM_COMPOSITE)
	{
		(*DestStream)->CurCpt = NULL;
		PrevDestCpt = (*DestStream);
		for (ThisSrcCpt = SrcStream->Link; NULL != ThisSrcCpt ; ThisSrcCpt = ThisSrcCpt->Link )
		{
			Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) &ThisDestCpt ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (0 != Code)
			{
				PrevDestCpt->Link = NULL;
				(void)FreeClonedExaStream( *(DestStream) );
				*DestStream = NULL;
				return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			}
			memcpy( ThisDestCpt, ThisSrcCpt, sizeof( ExaPatchStream ) );
			PrevDestCpt->Link = ThisDestCpt;
			PrevDestCpt = ThisDestCpt;
		}
	}
# ifdef _WIN32
	else if ((0U != DupFlag) && ((SrcStream->Type & EXP_STREAM_TYPE_BITS) == EXP_STREAM_FILE))
	{
		wchar_t pszFileName[400];

		if (0 != GetFileNameFromHandle( SrcStream->FileHandle, pszFileName, 400 ))
		{
			(*DestStream)->FileHandle = CreateFileW( pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ); /* MISRA C 2012 [7.2]: some Windows macros lack 'U' suffixes */
			if (((*DestStream)->FileHandle) != INVALID_HANDLE_VALUE) /* MISRA C 2012 [11.6]: use of Windows INVALID_HAND_VALUE macro violates this rule. It should nevertheless be used. */
			{
				(*DestStream)->Flags |= 1U;
			}
			/*
			else
			{

				(*DestStream)->FileHandle = SrcStream->FileHandle;
			}
			*/
			/*
				We *might* want to come up with a fall-back strategy for dealing with this case.
				Originally, the GetFileNameFromHandle scheme seemed rather fragile to me, but
				especially with the GLOBALROOT modification, I really haven't seen it fail -
				any platform (Win2K+), any sort of file, local or remote.  The original sources for code
				like this on the 'net REALLY seem overly complicated (see, for example, MSDN)
				and limited (MSDN code, for example, seems to only work with local files and 
				remote files on mapped drives; the DDJ code appears to work with UNC, but at 
				a truly horrid code-cost).  In my testing (before I found the GLOBALROOT trick), 
				I never found a file that failed my more complex method (which is itself simpler
				than any alternatives I found).  And then the GLOBALROOT trick (which I find no 
				direct reference to on the 'net) is even simpler.  Maybe all the fussing was occasioned
				by 9x/NT4 issues...
				
				note: if the original handle is opened with read/write access, this code will make it silently
					fail to do what was desired.  That's probably not good...it's why I commented it out.
					Now, an invalid handle value will be placed in the resulting stream, which should cause
					I/O errors.
				*/
		}
		else
		{
			(*DestStream)->FileHandle = INVALID_HANDLE_VALUE; /* MISRA C 2012 [11.6]: use of Windows INVALID_HAND_VALUE macro violates this rule. It should nevertheless be used. */
		}
	}
	else
	{
	}
# endif
	return(EXAPATCH_SUCCESS);
}
INT FreeClonedExaStream( ExaPatchStream * Stream )
{
	ExaPatchStream * ThisCpt;
	ExaPatchStream * NextCpt;
	if ((Stream->Type & EXP_STREAM_TYPE_BITS) == EXP_STREAM_COMPOSITE)
	{
		for (ThisCpt = Stream->Link; NULL != ThisCpt ; ThisCpt = NextCpt )
		{
			NextCpt=ThisCpt->Link;
			ExaMemFree( NULL, ThisCpt );
		}
	}
# ifdef _WIN32
	else if ((0U != (Stream->Flags & 1U)) &&((Stream->Type & EXP_STREAM_TYPE_BITS) == EXP_STREAM_FILE))
	{
		(void)CloseHandle( Stream->FileHandle );
		(void)ExaMemFree( NULL, Stream );
	}
# endif
	else
	{
		ExaMemFree( NULL, Stream );
	}
	return(EXAPATCH_SUCCESS);
}
# endif /* !ATTOPATCH */
int GetNextExaStreamSpecial( const ExaPatchStream * Stream,
	QWORD CurPos, 
	QWORD * OffsetPtr, 
	QWORD * LenPtr, 
	QWORD * PosPtr )
{
	const ExaPatchStream * ThisCpt;

	*LenPtr = 0;
	*PosPtr = 0;
# ifdef QWORD_IS_DWORD
	*OffsetPtr = 0xffffffffU;
# else
	*OffsetPtr = 0xffffffffffffffffU;
# endif
	if ((Stream->Type & EXP_STREAM_TYPE_BITS) != EXP_STREAM_COMPOSITE)
	{
		return(EXAPATCH_SUCCESS); /* MISRA C 2012 [15.5]: return used for quick exit in exceptional condition  */
	}

	ThisCpt = Stream->Link;
	while ( ThisCpt && ((ThisCpt->CompositeOrigin + ThisCpt->Size) <= CurPos) )
	{
		ThisCpt = ThisCpt->Link;
	}
	if ((NULL != ThisCpt) && (ThisCpt->CompositeOrigin != CurPos))
	{
		ThisCpt = ThisCpt->Link;
	}
	if (ThisCpt == NULL)
	{
		return(EXAPATCH_SUCCESS); /* MISRA C 2012 [15.5]: return used for quick exit in exceptional condition  */
	}
	while ( (NULL != ThisCpt) && (ThisCpt->Type != EXP_STREAM_SPECIAL_GAP))
	{
		ThisCpt = ThisCpt->Link;
	}
	if (ThisCpt == NULL)
	{
		return(EXAPATCH_SUCCESS); /* MISRA C 2012 [15.5]: return used for quick exit in exceptional condition  */
	}
	*LenPtr = ThisCpt->Size;
	*OffsetPtr = ThisCpt->CompositeOrigin;
	*PosPtr = ThisCpt->SpecialGapPos;
	return(EXAPATCH_SUCCESS);
}
# ifdef EXAPATCH_CACHEDSTREAM_SUPPORT
# define EXASTREAM_READ_CACHING
typedef struct _CachedStream 
{
	DWORD dwBufferSize;
	QWORD qwPosMask;
	DWORD dwShiftCount;
	char * Buffer[2];
	QWORD BufferOrigin;
	DWORD BufferValidEnd[2];
	DWORD BufferDirtyStart[2];
	DWORD BufferDirtyEnd[2];
	QWORD qwPos;
	QWORD qwSize;
	ExaPatchStream Stream;
} CachedStream;

INT CacheBuf( CachedStream * pCS, 
	DWORD dwIndex, 
	void * Buffer, 
	DWORD dwCount, 
	LPDWORD pdwCount, 
	BOOL bRead )
{
	// move between memory and cache
	DWORD dwCacheOffset=0;
	DWORD dwMaxCacheOffset;
	DWORD dwCountInCache = dwCount;
	INT Code;


	if (dwIndex > 1)
	{
		return(EXAPATCH_INTERNAL_ERROR);
		
	}
	if ((pCS->qwPos >> pCS->dwShiftCount) == (pCS->BufferOrigin + dwIndex))
	{
		/* positions match...transfer begins at appropriate position in buffer */
		dwCacheOffset = (DWORD) (pCS->qwPos & pCS->qwPosMask);
		if (dwCount > (pCS->dwBufferSize - dwCacheOffset))
		{
			dwCountInCache = pCS->dwBufferSize - dwCacheOffset;
		}
	}
	else
	{
		/* positions don't match...assume transfer begins at start of buffer */
		if (dwCount > pCS->dwBufferSize)
		{
			dwCountInCache = pCS->dwBufferSize;
		}
	}
	dwMaxCacheOffset = dwCountInCache + dwCacheOffset;
	if (bRead && (dwMaxCacheOffset > pCS->BufferValidEnd[dwIndex]))
	{
		dwMaxCacheOffset = pCS->BufferValidEnd[dwIndex];
		if (dwCacheOffset <= dwMaxCacheOffset)
		{
			dwCountInCache = dwMaxCacheOffset - dwCacheOffset;
		}
		else
		{
			dwCountInCache = 0;
		}
	}
	if (dwCountInCache)
	{
		if (bRead)
		{
			memmove( Buffer, pCS->Buffer[dwIndex]+dwCacheOffset, dwCountInCache );
		}
		else
		{
			if (pCS->BufferValidEnd[dwIndex] < dwCacheOffset)
			{
				/*
				 here, the write starts AFTER the valid buffer head
				 fill in remainder of buffer head from file
				*/
				Code = SeekExaStream( &(pCS->Stream),
					((pCS->BufferOrigin + dwIndex)<<pCS->dwShiftCount) + pCS->BufferValidEnd[dwIndex],
					EXAPATCH_SEEK_BEGIN,
					NULL );
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = ReadExaStream( &(pCS->Stream),
						pCS->Buffer[dwIndex] + pCS->BufferValidEnd[dwIndex],
						dwCacheOffset - pCS->BufferValidEnd[dwIndex],
						NULL ); /* don't care if this read is longer than the file */
				}
				if (Code) return(Code); /* unable to validate head of buffer */
			}
			memmove( pCS->Buffer[dwIndex]+dwCacheOffset, Buffer, dwCountInCache );
			if (pCS->BufferValidEnd[dwIndex] < dwMaxCacheOffset)
			{
				pCS->BufferValidEnd[dwIndex] = dwMaxCacheOffset;
			}
			if (pCS->BufferDirtyEnd[dwIndex]==0)
			{
				pCS->BufferDirtyStart[dwIndex] = dwCacheOffset;
				pCS->BufferDirtyEnd[dwIndex] = dwMaxCacheOffset;
			}
			else
			{
				if (pCS->BufferDirtyStart[dwIndex] > dwCacheOffset)
				{
					pCS->BufferDirtyStart[dwIndex] = dwCacheOffset;
				}
				if (pCS->BufferDirtyEnd[dwIndex] < dwMaxCacheOffset)
				{
					pCS->BufferDirtyEnd[dwIndex] = dwMaxCacheOffset;
				}
			}
		}
	}


	if (pdwCount)
	{
		*pdwCount = dwCountInCache;
	}
	return(EXAPATCH_SUCCESS);
	
}

INT CacheFile( CachedStream * pCS, 
	DWORD dwIndex, 
	BOOL bRead )
{
	/* move between file and cache */
	INT Code = EXAPATCH_SUCCESS;
	DWORD dwWritten;

	if (dwIndex > 1) return(EXAPATCH_INTERNAL_ERROR);
	
	if (bRead)
	{
		pCS->BufferDirtyEnd[dwIndex] = 0;
		Code = SeekExaStream( &(pCS->Stream),  
			(pCS->BufferOrigin + dwIndex) << pCS->dwShiftCount, 
			EXAPATCH_SEEK_BEGIN,
			NULL );
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = ReadExaStream( &(pCS->Stream),
				pCS->Buffer[dwIndex],
				pCS->dwBufferSize,
				&pCS->BufferValidEnd[dwIndex] );
		}
	}
	else if ((pCS->BufferDirtyEnd[dwIndex])
		&& (pCS->BufferDirtyEnd[dwIndex] != pCS->BufferDirtyStart[dwIndex]))
	{
		Code = SeekExaStream( &(pCS->Stream),
			((pCS->BufferOrigin + dwIndex) << pCS->dwShiftCount) + pCS->BufferDirtyStart[dwIndex],
			EXAPATCH_SEEK_BEGIN,
			NULL );
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = WriteExaStream( &(pCS->Stream),
				pCS->Buffer[dwIndex] + pCS->BufferDirtyStart[dwIndex],
				pCS->BufferDirtyEnd[dwIndex] - pCS->BufferDirtyStart[dwIndex],
				&dwWritten );
			if ((Code == EXAPATCH_SUCCESS) 
				&& (dwWritten != (pCS->BufferDirtyEnd[dwIndex] - pCS->BufferDirtyStart[dwIndex])))
			{
				Code = EXAPATCH_DISK_FULL;
			}
		}
		pCS->BufferDirtyEnd[dwIndex] = 0;
	}
	return(Code);
	
}

INT CALLBACK CachedRead(HANDLE hStream, void *Buffer, DWORD dwCount, LPDWORD pdwReturn)
{
	CachedStream * pCS = (CachedStream *) hStream;
	DWORD dwCache0ReadSize;
	DWORD dwReturn = 0;
	DWORD ReadSoFar;
	QWORD qw0,qw1;
	char * Ptr;
	INT Code = EXAPATCH_SUCCESS;

	Ptr = (char *) Buffer;
	qw0 = pCS->qwPos >> pCS->dwShiftCount;
	qw1 = (pCS->qwPos + dwCount) >> pCS->dwShiftCount;
	if (0 == (pCS->BufferValidEnd[0] | pCS->BufferValidEnd[1]))
	{
		// no buffer yet - fill it
		pCS->BufferOrigin = qw0;
		Code = CacheFile( pCS, 0, TRUE );
		if ((Code == EXAPATCH_SUCCESS) && (qw1 > qw0))
		{
			Code = CacheFile( pCS, 1, TRUE );
		}
	}
	if (Code == EXAPATCH_SUCCESS)
	{
		if ((qw0==qw1)&&((qw0-pCS->BufferOrigin)<= 1))
		{
			if ((qw0 != pCS->BufferOrigin)&&(pCS->BufferValidEnd[1]==0))
			{
				Code = CacheFile( pCS, 1, TRUE );
			}
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = CacheBuf( pCS, 
					(DWORD)(qw0-pCS->BufferOrigin), 
					Ptr, 
					dwCount, 
					&dwReturn, TRUE );
			}
		}
		else if ((qw0==pCS->BufferOrigin)&&(qw1 == (qw0+1)))
		{
			if (pCS->BufferValidEnd[1]==0)
			{
				Code = CacheFile( pCS, 1, TRUE );
			}
			dwCache0ReadSize = pCS->dwBufferSize - (DWORD)(pCS->qwPos & pCS->qwPosMask);
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = CacheBuf( pCS, 
					0, 
					Ptr, 
					dwCache0ReadSize, 
					&dwReturn, 
					TRUE );
				if ((Code == EXAPATCH_SUCCESS)&&(dwReturn == dwCache0ReadSize))
				{
					Code = CacheBuf( pCS, 
						1, 
						Ptr + dwCache0ReadSize, 
						dwCount - dwCache0ReadSize, 
						&dwReturn, 
						TRUE );
					dwReturn += dwCache0ReadSize;
				}
			}
		}
		else
		{
# ifdef EXASTREAM_READ_CACHING
			/* have to hit the disk anyway - see if it's a *big* or *small* read. */
			if ((qw0 != pCS->BufferOrigin) &&(qw1 > qw0))
			{
				ReadSoFar = 0;
				/* if it's a *big* read, then flush, and behave as though cache is empty */
				if (pCS->BufferDirtyEnd[0])
				{
					Code = CacheFile( pCS, 0, FALSE );
				}
				if ((Code == EXAPATCH_SUCCESS) && pCS->BufferDirtyEnd[1])
				{
					Code = CacheFile( pCS, 1, FALSE );
				}
				/* check for special cases (advancing or reversing buffer) */
				if ((qw0==(pCS->BufferOrigin+1)) && (pCS->BufferValidEnd[1]))
				{
					/* buffer advance */
					char * Temp;

					Temp = pCS->Buffer[0];
					pCS->Buffer[0] = pCS->Buffer[1];
					pCS->Buffer[1] = Temp;
					pCS->BufferValidEnd[0] = pCS->BufferValidEnd[1];
					pCS->BufferValidEnd[1] = 0;
					pCS->BufferOrigin = qw0;
					if (pCS->BufferValidEnd[0] == 0)
					{
						Code = CacheFile( pCS, 0, TRUE );
					}
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = CacheFile( pCS, 1, TRUE );
					}
				}
				else if ((qw0==(pCS->BufferOrigin-1))&&(pCS->BufferValidEnd[0]))
				{
					/* buffer reverse */
					char * Temp;

					Temp = pCS->Buffer[0];
					pCS->Buffer[0] = pCS->Buffer[1];
					pCS->Buffer[1] = Temp;
					pCS->BufferValidEnd[1] = pCS->BufferValidEnd[0];
					pCS->BufferValidEnd[0] = 0;
					pCS->BufferOrigin = qw0;
					Code = CacheFile( pCS, 0, TRUE );
				}
				else
				{
					// generic
					pCS->BufferValidEnd[0] = pCS->BufferValidEnd[1] = 0;
					pCS->BufferOrigin = qw0;
					Code = CacheFile( pCS, 0, TRUE );
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = CacheFile( pCS, 1, TRUE );
					}
				}
				dwCache0ReadSize = pCS->dwBufferSize - (DWORD)(pCS->qwPos & pCS->qwPosMask);
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = CacheBuf( pCS,
						0,
						Ptr,
						dwCache0ReadSize,
						&dwReturn,
						TRUE );
					if ((Code == EXAPATCH_SUCCESS) && (dwReturn == dwCache0ReadSize))
					{
						Code = CacheBuf( pCS,
							1,
							Ptr + dwCache0ReadSize,
							dwCount - dwCache0ReadSize,
							&dwReturn,
							TRUE );
						dwReturn += dwCache0ReadSize;
					}
					ReadSoFar = dwReturn;
				}
				if ((dwCount > ReadSoFar)&& (Code == EXAPATCH_SUCCESS))
				{
					Code = SeekExaStream( &(pCS->Stream), pCS->qwPos + ReadSoFar, EXAPATCH_SEEK_BEGIN, NULL );
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = ReadExaStream( &(pCS->Stream), Ptr + ReadSoFar, dwCount - ReadSoFar, &dwReturn );
						dwReturn += ReadSoFar;
					}
				}

			}
			else if (qw1 > qw0)
			{
				dwCache0ReadSize = pCS->dwBufferSize - (DWORD)(pCS->qwPos & pCS->qwPosMask);
				Code = CacheBuf( pCS,
					0,
					Ptr,
					dwCache0ReadSize,
					&dwReturn,
					TRUE );
				if ((Code == EXAPATCH_SUCCESS) && (dwReturn == dwCache0ReadSize))
				{
					Code = CacheBuf( pCS,
						1,
						Ptr + dwCache0ReadSize,
						dwCount - dwCache0ReadSize,
						&dwReturn,
						TRUE );
					dwReturn += dwCache0ReadSize;
				}
				ReadSoFar = dwReturn;
				if ((dwCount > ReadSoFar)&& (Code == EXAPATCH_SUCCESS))
				{
					Code = SeekExaStream( &(pCS->Stream), pCS->qwPos + ReadSoFar, EXAPATCH_SEEK_BEGIN, NULL );
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = ReadExaStream( &(pCS->Stream), Ptr + ReadSoFar, dwCount - ReadSoFar, &dwReturn );
						dwReturn += ReadSoFar;
					}
				}
			}
			else
			{
				/* if it's a *small* read, then just read outside the cache */
				Code = SeekExaStream( &(pCS->Stream), pCS->qwPos, EXAPATCH_SEEK_BEGIN, NULL );
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = ReadExaStream( &(pCS->Stream), Ptr, dwCount, &dwReturn );
				}
			}
# else
			/*
			 no significant overlap (need to hit disk anyway) - just read normally
			 don't move buffer (for now)
			 first cut is to position the buffer on *write* then use it on subsequent
			  read/write cycles
			 This goes along with the access pattern on patch apply
			*/
			Code = SeekExaStream( &(pCS->Stream), pCS->qwPos, EXAPATCH_SEEK_BEGIN, NULL );
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = ReadExaStream( &(pCS->Stream), Ptr, dwCount, &dwReturn );
			}
# endif
		}
	}

	if (pdwReturn)
		*pdwReturn = dwReturn;
	pCS->qwPos += dwReturn;
	return(Code);

}
INT CALLBACK CachedWrite(HANDLE hStream, const void *Buffer, DWORD dwCount, LPDWORD pdwReturn)
{
	CachedStream * pCS = (CachedStream *) hStream;
	DWORD dwReturn = 0;
	DWORD dwCache0WriteSize;
	QWORD qw0,qw1;
	DWORD WrittenSoFar;
	const unsigned char * Ptr;
	INT Code = EXAPATCH_SUCCESS;

	Ptr = (const unsigned char *) Buffer;
	qw0 = pCS->qwPos >> pCS->dwShiftCount;
	qw1 = (pCS->qwPos + dwCount) >> pCS->dwShiftCount;

	if (0 == (pCS->BufferValidEnd[0] | pCS->BufferValidEnd[1]))
	{
		// no buffer yet - fill it, write the rest, return
		pCS->BufferOrigin = qw0;
		pCS->BufferDirtyEnd[0] = pCS->BufferDirtyEnd[1] = 0;
		dwCache0WriteSize = pCS->dwBufferSize - (DWORD)(pCS->qwPos & pCS->qwPosMask);
		if (dwCache0WriteSize > dwCount)
			dwCache0WriteSize = dwCount;
		Code = CacheBuf( pCS, 
			0, 
			(unsigned char *)Buffer, //this is OK
			dwCache0WriteSize, 
			&dwReturn, 
			FALSE );
		if ((Code == EXAPATCH_SUCCESS) 
			&& (dwReturn == dwCache0WriteSize) 
			&& (qw1 > qw0))
		{
			Code = CacheBuf( pCS, 
				1, 
				(unsigned char *)(Ptr + dwCache0WriteSize), //this is OK
				dwCount - dwCache0WriteSize, 
				&dwReturn, 
				FALSE );
			dwReturn += dwCache0WriteSize;
		}
		WrittenSoFar = dwReturn;
		Ptr += dwReturn;
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = SeekExaStream( &(pCS->Stream),pCS->qwPos + WrittenSoFar,  EXAPATCH_SEEK_BEGIN, NULL );
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = WriteExaStream( &(pCS->Stream), Ptr, dwCount - WrittenSoFar, &dwReturn );
				dwReturn += WrittenSoFar;
			}
		}
	}
	else
	{
		// buffer is nonempty - see if we can write through it
		if ((qw0==qw1)&&((qw0-pCS->BufferOrigin)<= 1))
		{
			Code = CacheBuf( pCS, (DWORD)(qw0-pCS->BufferOrigin), (unsigned char *)Buffer, dwCount, &dwReturn, FALSE ); //this is OK
		}
		else if ((qw0==pCS->BufferOrigin)&&(qw1 == (qw0+1)))
		{
			dwCache0WriteSize = pCS->dwBufferSize - (DWORD)(pCS->qwPos & pCS->qwPosMask);
			Code = CacheBuf( pCS, 
				0, 
				(unsigned char *)Ptr, //this is OK
				dwCache0WriteSize, 
				&dwReturn, 
				FALSE );
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = CacheBuf( pCS, 
					1, 
					(unsigned char *)(Ptr + dwCache0WriteSize), //this is OK
					dwCount - dwCache0WriteSize, 
					&dwReturn, 
					FALSE );
				dwReturn += dwCache0WriteSize;
			}
		}
		else
		{
			/* have to hit the disk anyway - see if it's a *big* or *small* write. */
			if (qw1 > qw0)
			{
				/*
				 if it's a *big* write, then flush, and behave as though cache
				 is empty.
				*/
				if (pCS->BufferDirtyEnd[0])
				{
					Code = CacheFile( pCS, 0, FALSE );
				}
				if ((Code == EXAPATCH_SUCCESS) && pCS->BufferDirtyEnd[1])
				{
					Code = CacheFile( pCS, 1, FALSE );
				}
				// check for special case (advancing the buffer window)
				if ((qw0 == (pCS->BufferOrigin + 1))&&(pCS->BufferValidEnd[1]))
				{
					char * Temp;

					Temp = pCS->Buffer[0];
					pCS->Buffer[0] = pCS->Buffer[1];
					pCS->Buffer[1] = Temp;
					pCS->BufferValidEnd[0]=pCS->BufferValidEnd[1];
					pCS->BufferValidEnd[1] = 0;
					pCS->BufferOrigin = qw0;
				}
				else
				{
					pCS->BufferValidEnd[0] = pCS->BufferValidEnd[1] = 0;
					pCS->BufferOrigin = qw0;
				}
				dwCache0WriteSize = pCS->dwBufferSize - (DWORD)(pCS->qwPos & pCS->qwPosMask);
				Code = CacheBuf( pCS, 
					0, 
					(unsigned char *)Buffer, //this is OK
					dwCache0WriteSize, 
					&dwReturn, 
					FALSE );
				if ((Code == EXAPATCH_SUCCESS) 
					&& (dwReturn == dwCache0WriteSize) 
					&& (qw1 > qw0))
				{
					Code = CacheBuf( pCS, 
						1, 
						(unsigned char *)(Ptr + dwCache0WriteSize), //this is OK
						dwCount - dwCache0WriteSize, 
						&dwReturn, 
						FALSE );
					dwReturn += dwCache0WriteSize;
				}
				// EXPERIMENTAL
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = CacheFile( pCS, 0, FALSE );
					if ((Code == EXAPATCH_SUCCESS)&& (qw1 > qw0))
					{
						Code = CacheFile( pCS, 1, FALSE );
					}
				}
				// END EXPERIMENTAL
				WrittenSoFar = dwReturn;
				Ptr += dwReturn;
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = SeekExaStream( &(pCS->Stream), pCS->qwPos + WrittenSoFar, EXAPATCH_SEEK_BEGIN, NULL );
					if (Code == EXAPATCH_SUCCESS)
					{
						Code = WriteExaStream( &(pCS->Stream), Ptr, dwCount - WrittenSoFar, &dwReturn );
						dwReturn += WrittenSoFar;
					}
				}
			}
			else
			{
				/* if it's a *small* write, then just write outside the cache */
				Code = SeekExaStream( &(pCS->Stream), pCS->qwPos, EXAPATCH_SEEK_BEGIN, NULL );
				if (Code == EXAPATCH_SUCCESS)
				{
					Code = WriteExaStream( &(pCS->Stream), Ptr, dwCount, &dwReturn );
				}
			}
			
		}
	}
	if (pdwReturn)
		*pdwReturn = dwReturn;
	pCS->qwPos += dwReturn;
	if (pCS->qwSize < pCS->qwPos)
	{
		pCS->qwSize = pCS->qwPos;
	}
	return(Code);
}
INT CALLBACK CachedSeek(HANDLE hStream, QWORD qwPos, DWORD dwOrigin, QWORD * pqwReturn)
{
	CachedStream * pCS = (CachedStream *) hStream;
	INT Code = EXAPATCH_SUCCESS;

	switch (dwOrigin)
	{
		case EXAPATCH_SEEK_BEGIN:
			pCS->qwPos = qwPos;
			break;
		case EXAPATCH_SEEK_CUR:
			pCS->qwPos += qwPos;
			break;
		case EXAPATCH_SEEK_END:
			pCS->qwPos = pCS->qwSize + qwPos;
			break;
		default:
			Code = EXAPATCH_INVALID_PARAMETER;
			break;
	}
	if (pqwReturn)
	{
		*pqwReturn = pCS->qwPos;
	}
	return(Code);
	
}
INT CALLBACK CachedAttribGet(HANDLE hStream, ExaPatchFileAttrib * pAttrib)
{
	CachedStream * pCS = (CachedStream *) hStream;
	INT Code;
	Code = GetExaStreamAttrib( &pCS->Stream, pAttrib );
	return(Code);
}
INT CALLBACK CachedAttribSet(HANDLE hStream, const ExaPatchFileAttrib * pAttrib)
{
	CachedStream * pCS = (CachedStream *) hStream;
	INT Code;

	Code = SetExaStreamAttrib( &pCS->Stream, pAttrib );
	return(Code);
}
INT CALLBACK CachedClose(HANDLE hStream)
{
	CachedStream * pCS = (CachedStream *) hStream;
	INT Code = EXAPATCH_SUCCESS;

	if (pCS->BufferDirtyEnd[0])
	{
		Code = CacheFile( pCS, 0, FALSE );
	}
	if ((Code == EXAPATCH_SUCCESS)&& pCS->BufferDirtyEnd[1])
	{
		Code = CacheFile( pCS, 1, FALSE );
	}
	if (pCS->Buffer[0])
		ExaMemFree( NULL, pCS->Buffer[0] );
	if (pCS->Buffer[1])
		ExaMemFree( NULL, pCS->Buffer[1] );
	if (Code)
	{
		CloseExaStream( &(pCS->Stream) );
	}
	else
	{
		Code = CloseExaStream( &(pCS->Stream) );
	}
	ExaMemFree( NULL, pCS );
	return(Code);
}


INT ForceExaStreamBuffering( ExaPatchStream * Stream, DWORD dwBufferSize )
{
	INT Code;
	CachedStream * pCS;

	if (!Stream) return EXAPATCH_SUCCESS;
  if (dwBufferSize == 0)
  {
		dwBufferSize = EXAPATCH_STREAM_DEFAULT_BUFSIZE;
  }
	Code = ExaMemAlloc(NULL, sizeof( CachedStream ), (void **) &pCS ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (Code == EXAPATCH_SUCCESS)
	{
		memset( pCS, 0, sizeof( CachedStream ) );
		memmove( &pCS->Stream, Stream, sizeof( ExaPatchStream ) );
		pCS->qwSize = Stream->Size;
		pCS->qwPos = Stream->CurPos;
		Stream->Type = EXP_STREAM_USER;
		Stream->FileHandle = (HANDLE) pCS;
		Stream->Read = CachedRead;
		Stream->Write = CachedWrite;
		Stream->Seek = CachedSeek;
		Stream->AttribGet = CachedAttribGet;
		Stream->AttribSet = CachedAttribSet;
		Stream->Close = CachedClose;
		if ((dwBufferSize == 0xffffffffU) && (pCS->qwSize > 0x40000000U))
		{
			dwBufferSize = 0x40000000U;
		}
		if (dwBufferSize != 0xffffffffU)
		{
			dwBufferSize = dwBufferSize >> 12;

			pCS->dwBufferSize = 0x400U;
			pCS->dwShiftCount = 10U;
			while (dwBufferSize)
			{
				dwBufferSize = dwBufferSize >> 1;
				pCS->dwBufferSize = pCS->dwBufferSize << 1;
				(pCS->dwShiftCount)++;
			}
			pCS->qwPosMask = ((QWORD) pCS->dwBufferSize) - 1;
		}
		else
		{
			pCS->dwBufferSize = (DWORD) pCS->qwSize;
			pCS->dwShiftCount = 31;
# ifdef QWORD_IS_DWORD
			pCS->qwPosMask = 0xffffffffU;
# else
			pCS->qwPosMask = 0xffffffffffffffffU;
# endif
		}
		Code = ExaMemAlloc( NULL, pCS->dwBufferSize, (void **) &pCS->Buffer[0] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if ((Code == EXAPATCH_SUCCESS) && (pCS->dwShiftCount != 31U))
		{
			Code = ExaMemAlloc( NULL, pCS->dwBufferSize, (void **) &pCS->Buffer[1] ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (0 != Code)
			{
				(void)ExaMemFree( NULL, pCS->Buffer[0] );
			}
		}
		if (0 != Code)
		{
			memmove( Stream, &pCS->Stream, sizeof( ExaPatchStream ) );
			(void)ExaMemFree( NULL, pCS );
		}
	}
	return(Code);
	
}
# else
int ForceExaStreamBuffering( ExaPatchStream * Stream, DWORD dwBufferSize )
{
	(void) Stream;
	(void) dwBufferSize;
	return(EXAPATCH_NOT_SUPPORTED);
}

# endif
# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
static INT SBFlush( ExaPatchStream * Stream, DWORD dwFull )
{
	INT Code = EXAPATCH_SUCCESS;
	DWORD dwSize = (DWORD)(Stream->Size - Stream->BufferOrigin);
	DWORD dwSizeWritten;

	if (0U != (Stream->Type & EXP_STREAM_SB_RSVD2))
	{
		Code = SeekExaStream( Stream->Link, Stream->BufferOrigin - Stream->BufferSize, EXAPATCH_SEEK_BEGIN, NULL );
		if (Code == EXAPATCH_SUCCESS)
		{
			Code = WriteExaStream( Stream->Link,
				(0U != (Stream->Type & EXP_STREAM_SB_RESERVED))?Stream->Buffer:(&((unsigned char *)Stream->Buffer)[Stream->BufferSize]), /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */
				Stream->BufferSize, &dwSizeWritten );
			if ((Code == EXAPATCH_SUCCESS) && (Stream->BufferSize != dwSizeWritten))
			{
				Code = EXAPATCH_DISK_FULL;
			}
		}
	}
	if (0U != dwFull)
	{
		if ((0U != dwSize) && (Code == EXAPATCH_SUCCESS))
		{
			Code = SeekExaStream( Stream->Link, Stream->BufferOrigin, EXAPATCH_SEEK_BEGIN, NULL );
			if (Code == EXAPATCH_SUCCESS)
			{
				Code = WriteExaStream( Stream->Link, 
					(0U != (Stream->Type & EXP_STREAM_SB_RESERVED))?(&((UCHAR *)Stream->Buffer)[Stream->BufferSize]):Stream->Buffer,  /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */ 
					dwSize, &dwSizeWritten );
				if ((Code == EXAPATCH_SUCCESS) && (dwSize != dwSizeWritten))
				{
					Code = EXAPATCH_DISK_FULL;
				}
			}
		}
		Stream->Type &= ~EXP_STREAM_SB_RSVD2;
		Stream->BufferOrigin = Stream->Size;
		/* NOTE: after a full flush, buffer is positioned at the EOF and the 
		buffer contents are undefined */
	}
	else
	{
		Stream->Type ^= EXP_STREAM_SB_RESERVED;
		Stream->Type |= EXP_STREAM_SB_RSVD2;
		Stream->BufferOrigin += Stream->BufferSize;
	}
	return(Code);
}
static INT CALLBACK SBRead( HANDLE Handle, void * Buffer, DWORD Count, LPDWORD lpCount )
{
	/* MISRA C 2012 [8.13]: some MISRA checkers complain about Handle not being "const" */
	const ExaPatchStream * Stream = (ExaPatchStream *) Handle; /* MISRA C 2012 [11.3, 11.5]: casting pointer to/from "void *" is used to provide an opaque handle to the caller. */
	DWORD dwThisCount = 0;
	DWORD dwCountRead = 0;
	DWORD dwModCount = Count;
	QWORD qwThisOrigin = Stream->UserStreamPos;
	INT Code = EXAPATCH_SUCCESS;
	unsigned char * ThisBuffer = (unsigned char *)Buffer; /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */
	const unsigned char * Buf1 = (unsigned char *)Stream->Buffer; /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */
	const unsigned char * Buf2 = &Buf1[Stream->BufferSize];
	const unsigned char * Ptr;

	if (qwThisOrigin < ((0U != (Stream->Type & EXP_STREAM_SB_RSVD2))?(Stream->BufferOrigin - Stream->BufferSize):Stream->BufferOrigin))
	{
		/* read from earlier position than beginning of buffer */
		Code = EXAPATCH_INVALID_PARAMETER;
	}
	else if (qwThisOrigin < Stream->Size)
	{
		if ((qwThisOrigin + dwModCount) > Stream->Size)
		{
			/* this is OK because all of this code is skipped if read starts beyond EOF */
			dwModCount = (DWORD)(Stream->Size - qwThisOrigin);
		}
		while ((0U != dwModCount) && (Code == EXAPATCH_SUCCESS))
		{
			if (qwThisOrigin < Stream->BufferOrigin)
			{
				if (0U == (Stream->Type & EXP_STREAM_SB_RSVD2))
				{
					Code = EXAPATCH_INVALID_PARAMETER;
				}
				else
				{
					/*
					read from first half of buffer
					*/
					dwThisCount = (DWORD)(Stream->BufferOrigin - qwThisOrigin);
					if (dwThisCount > dwModCount)
					{
						dwThisCount = dwModCount;
					}
					Ptr = (0U !=(Stream->Type & EXP_STREAM_SB_RESERVED))?Buf1:Buf2; /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */
					memmove( ThisBuffer, &Ptr[Stream->BufferSize + qwThisOrigin - Stream->BufferOrigin], dwThisCount );
				}
			}
			else
			{
				/* read from second half of buffer */
				dwThisCount = (DWORD)(Stream->BufferOrigin + Stream->BufferSize - qwThisOrigin);
				if (dwThisCount > dwModCount)
				{
					dwThisCount = dwModCount;
				}
				Ptr = (0U != (Stream->Type & EXP_STREAM_SB_RESERVED))?Buf2:Buf1; /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */
				memmove(ThisBuffer, &Ptr[qwThisOrigin - Stream->BufferOrigin], dwThisCount );
			}
			ThisBuffer = &ThisBuffer[dwThisCount];
			qwThisOrigin += dwThisCount;
			dwCountRead += dwThisCount;
			dwModCount -= dwThisCount;
		}
	}
	else
	{
		/* no action necessary here - no bytes read */
	}
	if (NULL != lpCount)
	{
		*lpCount = dwCountRead;
	}
	return(Code);
}
static INT CALLBACK SBWrite( HANDLE Handle, const void * Buffer, DWORD Count, LPDWORD lpCount )
{
	/* MISRA C 2012 [8.13]: some MISRA checkers complain about Buffer not being "const" */
	ExaPatchStream * Stream = (ExaPatchStream *) Handle; /* MISRA C 2012 [11.3, 11.5]: casting pointer to/from "void *" is used to provide an opaque handle to the caller. */
	DWORD dwThisCount = 0;
	DWORD dwCountWritten = 0;
	QWORD qwThisOrigin = Stream->UserStreamPos;
	INT Code = EXAPATCH_SUCCESS;
	const unsigned char * ThisBuffer = (const unsigned char *)Buffer; /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */
	unsigned char * Ptr;
	unsigned char * Buf1 = (unsigned char *)Stream->Buffer; /* MISRA C 2012 [11.5]: algorithmically necessary (and correct) pointer conversion */
	unsigned char * Buf2 = &Buf1[Stream->BufferSize];


	if (qwThisOrigin < ((0U != (Stream->Type & EXP_STREAM_SB_RSVD2))?(Stream->BufferOrigin - Stream->BufferSize):Stream->BufferOrigin))
	{
		/* write at earlier position than beginning of buffer */
		Code = EXAPATCH_INVALID_PARAMETER;
	}
	else if (qwThisOrigin >= (Stream->BufferOrigin + Stream->BufferSize))
	{
		/* write at later position than end of buffer - reposition */
		if (qwThisOrigin >= (Stream->BufferOrigin + (((QWORD)Stream->BufferSize) << 1)))
		{
			/* no portion of the buffer is retained - optimize by skipping over all intervening writes */
			Code = SBFlush( Stream, 1 );
			Stream->BufferOrigin = qwThisOrigin;
		}
		else
		{
			/* flush first half of buffer, retain second half */
			Code = SBFlush( Stream, 0 );
		}
	}
	else
	{
		/* no corrective action necessary here */
	}
	while ((0U != Count) && (Code == EXAPATCH_SUCCESS))
	{
		if (qwThisOrigin < Stream->BufferOrigin)
		{
			if (0U == (Stream->Type & EXP_STREAM_SB_RSVD2))
			{
				Code = EXAPATCH_INVALID_PARAMETER;
			}
			else
			{
				/*
				write into first half of buffer
				*/
				dwThisCount = (DWORD)(Stream->BufferOrigin - qwThisOrigin);
				if (dwThisCount > Count)
				{
					dwThisCount = Count;
				}
				Ptr = (0U != (Stream->Type & EXP_STREAM_SB_RESERVED))?Buf1:Buf2;
				memmove( &Ptr[((QWORD)Stream->BufferSize) + qwThisOrigin - Stream->BufferOrigin], ThisBuffer, dwThisCount );
			}
		}
		else if (qwThisOrigin >= (Stream->BufferOrigin + Stream->BufferSize))
		{
			/*
			write beyond second half of buffer - flush
			this should only happen between buffer fills; any
			multiple flushes to reposition should have been
			handled above
			*/
			dwThisCount = 0;
			Code = SBFlush( Stream, 0 );
		}
		else
		{
			/* write into second half of buffer */
			dwThisCount = (DWORD)(Stream->BufferOrigin + Stream->BufferSize - qwThisOrigin);
			if (dwThisCount > Count)
			{
				dwThisCount = Count;
			}
			Ptr = (0U != (Stream->Type & EXP_STREAM_SB_RESERVED))?Buf2:Buf1;
			memmove(&Ptr[qwThisOrigin - Stream->BufferOrigin], ThisBuffer, dwThisCount );
			if (Stream->Size < (qwThisOrigin + dwThisCount))
			{
				/* update Size since it is used in SBFlush */
				Stream->Size = qwThisOrigin + dwThisCount;
			}
		}
		ThisBuffer = &ThisBuffer[dwThisCount];
		qwThisOrigin += dwThisCount;
		dwCountWritten += dwThisCount;
		Count -= dwThisCount;
	}
	if (NULL != lpCount)
	{
		*lpCount = dwCountWritten;
	}
	return(Code);
}
static INT CALLBACK SBSeek( HANDLE Handle, QWORD Position, DWORD Origin, QWORD * lpPos )
{
	/*
		Note that Origin is always EXAPATCH_SEEK_BEGIN for user stream seeks. If this ever changes, code like that in CachedSeek (which is unnecessary currently)
		should be added here.
	*/
	(void) Origin; /* not needed in this particular context */
	(void) Handle; /* not needed in this particular context */
	if (NULL != lpPos)
	{
		*lpPos = Position;
	}
	return(EXAPATCH_SUCCESS);
}
static INT CALLBACK SBGet( HANDLE Handle, ExaPatchFileAttrib * Attrib )
{
# ifdef ATTOPATCH
	(void)Handle;
	(void) Attrib;
	return (EXAPATCH_NOT_SUPPORTED);
# else
	ExaPatchStream * Stream = (ExaPatchStream *) Handle;
	return(GetExaStreamAttrib(Stream->Link, Attrib));
# endif
}
static INT CALLBACK SBSet( HANDLE Handle, const ExaPatchFileAttrib * Attrib )
{
# ifdef ATTOPATCH
	(void)Handle;
	(void)Attrib;
	return (EXAPATCH_NOT_SUPPORTED);
# else
	ExaPatchStream * Stream = (ExaPatchStream *) Handle;
	return(SetExaStreamAttrib(Stream->Link, Attrib));
# endif
}
static INT CALLBACK SBClose( HANDLE Handle )
{
	ExaPatchStream * Stream = (ExaPatchStream *) Handle; /* MISRA C 2012 [11.3, 11.5]: casting pointer to/from "void *" is used to provide an opaque handle to the caller. */
	INT Code;
	INT Code2;
	ExaPatchStream * LowerLayer = Stream->Link;

	Code = SBFlush( Stream, 1 );
	memcpy( Stream, Stream->Link, sizeof( ExaPatchStream ) );
	(void)ExaMemFree( NULL, LowerLayer );
	Code2 = CloseExaStream( Stream );
	if (0 != Code)
	{
	 return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	return(Code2);
	/*
	NOTE: returns error if either the flush fails or the close on the base stream fails.
	The base stream WILL be closed, even if the flush fails
	*/
}
INT ExaStreamSupplyBuffer( ExaPatchStream * Stream, void * Buffer, DWORD dwBufferSize )
{
	/*
	The supplied stream is made into a layered stream - the top layer has the
	EXP_STREAM_SPECIAL_BUFFERED bit set, and represents the stream subsequently.
	The lower layer (newly allocated) represents the actual stream originally supplied
	by the caller. 

	The supplied buffer is split in half. One half is being constructed by writes,
	and the other is used for satisfying reads. If EXP_STREAM_SB_RESERVED is 0, the
	first half of the buffer is being written (second half is read) and if it is 1,
	the second half of the buffer is being written (first half is read). The BufferSize
	member in the ExaPatchStream is half of the supplied buffer size (offset between halves).
	EXP_STREAM_SB_RSVD2 is set once both halves of the buffer are dirty.

	The top level layer (with EXP_STREAM_SPECIAL_BUFFERED set) is made into a user stream,
	with Read, Write, Seek, Get/Set and Close routines. BufferSize gives the size of each
	buffer, BufferOrigin gives the origin of the buffer being written, Size marks the end
	of the dirty portion of the buffer.


	Read uses both buffers to satisfy the request and returns failure if the region requested
	is not in the buffer - a read will NEVER be issued on the base stream. Write writes into
	the buffer. At the point where the buffer fills, the base stream will be used to write the
	first half of the buffer, and the buffer halves will be swapped. Seek is essentially a no-op, 
	get/set invoke on the base stream, and close writes the unfinished buffer (both halves, based 
	on top layer Size), then closes the base stream, copies the base stream back to the top level 
	stream and frees the memory for the base stream.



	Note that the base stream itself may make use of buffering, USER streams, etc. Furthermore,
	note that the memory supplied here is not freed by us at any point, nor is a pointer retained
	beyond the point of close. If the buffer supplied to this routine is dynamically allocated,
	this will cause a memory leak if the supplied buffer is not freed by the caller after the 
	stream is closed.

	May also be used on a special buffered stream to remove the buffering. May NOT be used to
	replace/resize the buffering since the new buffer will not necessarily contain enough data
	to satisfy subsequent reads. Note also that, if data has been written to the stream before
	special buffering is activated, it will not be readable.
	*/

	INT Code = EXAPATCH_INVALID_PARAMETER;
	ExaPatchStream * LowerLayer;

	if ((NULL == Stream->Link) && (0U == (Stream->Type & ~EXP_STREAM_TYPE_BITS)))
	{
		Code = ExaMemAlloc( NULL, sizeof( ExaPatchStream ), (void **) &Stream->Link ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code != EXAPATCH_SUCCESS)
		{
			return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
		}
		memcpy( Stream->Link, Stream, sizeof( ExaPatchStream ) );
		Stream->Link->Link = NULL;
		Stream->Type = EXP_STREAM_SPECIAL_BUFFERED | EXP_STREAM_USER;
		Stream->Buffer = Buffer;
		Stream->BufferSize = dwBufferSize >> 1;
		Stream->BufferOrigin = Stream->Size;
		Stream->Read = SBRead;
		Stream->Write = SBWrite;
		Stream->Seek = SBSeek;
		Stream->AttribGet = SBGet;
		Stream->AttribSet = SBSet;
		Stream->Close = SBClose;
		Stream->FileHandle = (HANDLE) Stream; /* MISRA C 2012 [11.3, 11.5]: casting pointer to/from "void *" is used to provide an opaque handle to the caller. */
	}
	else if ((0U != (Stream->Type & EXP_STREAM_SPECIAL_BUFFERED)) && (NULL == Buffer))
	{
		LowerLayer = Stream->Link;
		Code = SBFlush( Stream, 1 );
		memcpy( Stream, Stream->Link, sizeof( ExaPatchStream ) );
		(void)ExaMemFree( NULL, LowerLayer );
	}
	else
	{
		/* no processing needed in this case - Code is already set correctly */
	}
	return(Code);
	
}
# else
int ExaStreamSupplyBuffer( ExaPatchStream * Stream, void * Buffer, DWORD dwBufferSize )
{
	return(EXAPATCH_NOT_SUPPORTED);
}
# endif
