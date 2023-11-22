/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXPACHAN.H                                                         |
|                                                                           |
|                                                                           |
|   RTPatch Server Apply DLL Internal Channel Definition Header File        |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2010.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/

# ifndef EXAPATCH_APPLY_CHANNEL_INCLUDED

# define EXAPATCH_APPLY_CHANNEL_INCLUDED

# ifndef EXAPATCH_INTERNAL
#  define EXAPATCH_INTERNAL
# endif

# include "exaapply.h"

# define CHANNEL_SIGNATURE 0x45504143

typedef struct _ExaPatchApplyChannel {
DWORD Signature;
# ifdef _WIN32
CRITICAL_SECTION CritSect;
# else
# ifndef ATTOPATCH
pthread_mutex_t Mutex;
# endif
# endif
DWORD BusyFlag;
QWORD BufferBreak;
# ifdef EXAPATCH_CONTAINER_SUPPORT
DWORD dwPatchIndex;
# endif
# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
void * pChannelBuffer;
DWORD  dwCBSize;
# endif
# ifdef EXAPATCH_TEMPDIR_SUPPORT
wchar_t * pTempDir;
# endif
} ExaPatchApplyChannel;

# endif /* !EXAPATCH_APPLY_CHANNEL_INCLUDED */
