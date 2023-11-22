/*
 * libRtpatch.c
 *
 *  Created on: Oct 4, 2016
 *      Author: share_pc
 */

# include "expapply.h"
# include "apSubDir.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <assert.h>

#ifdef __VERIFONE_SRC__
#include <log/liblog.h>
#endif

# include "libDebug.h"

#ifdef __INGENICO_SRC__
	#include "fs_def.h"
#else
	#include "svc.h"
#endif

extern void mtiSleep(unsigned long ulTimeout);
extern int mtiStrlen(int iMaxLength, void *szpSrc);
extern int GetFileSize(char *path, long *lSize);

#ifdef __INGENICO_SRC__
	extern int TmsMountFs(const char *disk);
	extern int TmsUnmountFs(const char *disk);
#endif

extern void zmsg(unsigned char *msg, int len);

#define USER_TRACE_ID	0xF100

int CALLBACK FeedBack(DWORD ID, LPVOID Ptr, PLATWORD Handle);
int CALLBACK MyRead (HANDLE Index, void * Buffer, DWORD Size, LPDWORD ReturnSize);
int CALLBACK MyWrite (HANDLE Index, const void * Buffer, DWORD Size, LPDWORD ReturnSize);
int CALLBACK MySeek (HANDLE Index, QWORD Position, DWORD Origin, QWORD * ReturnPosition);

int EXP_DLLIMPORT
	ExaPatchApplyDoEntryRaw(
		PLATWORD Handle,
		ExaPatchStream * FileToApply,
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
		);

DWORD GlobalProgress, LocalProgress;

# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
# define BUFFER_SIZE 200
  char Buffer[BUFFER_SIZE];
# endif

# define OLDFILE 0
# define NEWFILE 1
# define DFCFILE 2

QWORD PosArray[3];

#ifdef __VERIFONE_SRC__
  int aip;
  int bip;
  int cip;
// By Allen for Ingenico RTpatch speed 
#elif __INGENICO_SRC__ 
  S_FS_FILE * aFp;
  S_FS_FILE * bFp;
  S_FS_FILE * cFp;
#endif


static void (*cbDispMsg)(char *cpMsg);

unsigned char *g_cpWriteStream = NULL;
//@@ROH RTPATCH for using RAM--
unsigned char *g_cpReadOldStream = NULL;
unsigned char *g_cpReadPatchStream = NULL;
//--@@ROH RTPATCH for using RAM


int GetTmsPath(int idx, char *path)
{
	if(idx == OLDFILE)
		sprintf(path, "%s/%s", TMS_OLDPATH, FILE_TMS_OLDAPP);
	else if(idx == NEWFILE)
		sprintf(path, "%s/%s", TMS_UPDATEDIR, FILE_TMS_NEWAPP);
	else if(idx == DFCFILE)
		sprintf(path, "%s/%s", TMS_OLDPATH, FILE_TMS_DOWNAPP);
	else
		return -1;
	return 0;
}

//@@ROH RTPATCH for using RAM--
int FileWritetoMemory(char *file_path, int file_size, char *membuff)
{
	FILE *fpTemp = NULL;
	int i = 0;
	fpTemp = fopen(file_path, "r"); 

	if(fpTemp == NULL)                                        
	{                                                         
		dmsg("file open error [%s][r]", file_path);        
		return -1;                                       
	}

	for (i=0; i<file_size; i++)
	{
		fread(&membuff[i], 1, 1, fpTemp);
	}

	fclose(fpTemp);

	return 0;
}
//--@@ROH RTPATCH for using RAM

int iRtPatchApply(long lNewSize, void (*cbDisplayMessage)(char *cpMsg))
{
	PLATWORD ApplyChannel;
	long lsize = 0;
	char caPath[64 + 1];
	int Code;
	//int iBsfd;
	// @@EB CODE CLEAN
#if 0
	FILE *fpTemp = NULL;
#endif

	// @@EB MALLOC MODIFY
#if 1
	ExaPatchStream * StreamArray = NULL;
#else
	ExaPatchStream * StreamArray;
#endif

	cbDispMsg = cbDisplayMessage;

	dmsg("[*] iRtPatchApply start : %ld", lNewSize);

	memset(caPath, 0x00, sizeof(caPath));
	GetTmsPath(NEWFILE, caPath);

#ifdef __INGENICO_SRC__
	FS_unlink(caPath);
	bFp = FS_open(caPath, "a");
	if (bFp == NULL)
	{
		dmsg("file open error [%s]",caPath);
		return -1;
	}
	FS_close(bFp);
	bFp = FS_open(caPath, "r+");
#else
	remove(caPath);

	bip = open(caPath, O_RDWR|O_CREAT);
	if(bip < 0)
	{
		dmsg("file open error [%s]",caPath);
		return -1;
	}
	
	g_cpWriteStream = (char*)malloc(lNewSize * sizeof(unsigned char));
	if (g_cpWriteStream == NULL)
	{
		dmsg("Failed to allocate the heap memory to byte-pointer!!");
		return -2;
	}
	
	memset(g_cpWriteStream, 0, sizeof(lNewSize));
#endif

	ApplyChannel = ExaPatchApplyOpen( );
	if (ApplyChannel == 0)
	{
		dmsg("ExaPatchApplyOpen open error [%d]", (int)ApplyChannel);
		return(-1);
	}

	StreamArray = malloc(3 * sizeof(ExaPatchStream));
	memset (&StreamArray[OLDFILE], 0, sizeof (ExaPatchStream));
	memset (&StreamArray[NEWFILE], 0, sizeof (ExaPatchStream));
	memset (&StreamArray[DFCFILE], 0, sizeof (ExaPatchStream));

	memset(caPath, 0x00, sizeof(caPath));
	GetTmsPath(OLDFILE, caPath);
	GetFileSize(caPath, &lsize);
	StreamArray[OLDFILE].Size = (QWORD)lsize;

#if __VERIFONE_SRC__

	//@@ROH RTPATCH for using RAM--
	g_cpReadOldStream = (char*)malloc(lsize * sizeof(unsigned char));
	if (g_cpReadOldStream == NULL)
	{
		dmsg("Failed to allocate the heap memory for readoldstream!!");
		return -2;
	}
	
	memset(g_cpReadOldStream, 0, sizeof(lsize));
	
	FileWritetoMemory(caPath, lsize, g_cpReadOldStream); // oldfile copy to g_cpReadOldStream
	//--@@ROH RTPATCH for using RAM

	if((aip = open(caPath, O_RDONLY)) < 0 )
	{
		dmsg("file open error [%s]",caPath);
		close(bip); //close(iBsfd);
		return -1;
	}

	
// By Allen for Ingenico RTpatch speed 
#elif __INGENICO_SRC__
	if((aFp = FS_open(caPath, "r")) == NULL )
	{
		dmsg("file open error [%s]",caPath);
		FS_close(bFp);
		return -1;
	}
#endif

	memset(caPath, 0x00, sizeof(caPath));
	GetTmsPath(DFCFILE, caPath);//PATCH
	GetFileSize(caPath, &lsize);
	StreamArray[DFCFILE].Size = (QWORD)lsize;

#if __VERIFONE_SRC__

	//@@ROH RTPATCH for using RAM--
	g_cpReadPatchStream = (char*)malloc(lsize * sizeof(unsigned char));
	if (g_cpReadPatchStream == NULL)
	{
		dmsg("Failed to allocate the heap memory for readpatchstream!!");
		return -2;
	}
	
	memset(g_cpReadPatchStream, 0, sizeof(lsize));
	
	FileWritetoMemory(caPath, lsize, g_cpReadPatchStream); // patch file copy to g_cpReadOldStream
	//--@@ROH RTPATCH for using RAM

	if((cip = open(caPath, O_RDONLY)) < 0 )
	{
		dmsg("file open error [%s]",caPath);
		close(aip); close(bip); //close(iBsfd);
		return -1;
	}
// By Allen for Ingenico RTpatch speed 
#elif __INGENICO_SRC__
	if((cFp = FS_open(caPath, "r")) == NULL)
	{
		dmsg("file open error [%s]",caPath);
		FS_close(aFp); FS_close(bFp);
		return -1;
	}
#endif

	if( StreamArray[OLDFILE].Size < 1 || StreamArray[DFCFILE].Size < 1 )
	{
#if __VERIFONE_SRC__
		close(aip); close(bip); close(cip); //close(iBsfd);
// By Allen for Ingenico RTpatch speed 
#elif __INGENICO_SRC__
		FS_close(aFp); FS_close(bFp); FS_close(cFp);
#endif
		dmsg("file size error StreamArray[OLDFILE].Size=[%d]  StreamArray[DFCFILE].Size=[%d]",(int)StreamArray[OLDFILE].Size, (int)StreamArray[DFCFILE].Size);
		free(StreamArray);	// @@EB MALLOC MODIFY
		return -1;
	}

	StreamArray[OLDFILE].Type = StreamArray[NEWFILE].Type  = StreamArray[DFCFILE].Type = EXP_STREAM_USER;
	StreamArray[OLDFILE].dwSize = StreamArray[NEWFILE].dwSize  = StreamArray[DFCFILE].dwSize =  sizeof (ExaPatchStream);
	StreamArray[OLDFILE].Read = StreamArray[DFCFILE].Read =  MyRead;
	StreamArray[OLDFILE].Write = MyWrite;
	StreamArray[OLDFILE].Seek = StreamArray[DFCFILE].Seek =  MySeek;
# ifndef EXAPATCH_SPECIALSTREAM_SUPPORT
	StreamArray[NEWFILE].Seek  = MySeek;
	StreamArray[NEWFILE].Read  = MyRead;
# endif
	StreamArray[NEWFILE].Write  = MyWrite;
	StreamArray[OLDFILE].UserStreamPos = StreamArray[NEWFILE].UserStreamPos  = StreamArray[DFCFILE].UserStreamPos =  0;
	StreamArray[OLDFILE].CompositeOrigin = StreamArray[NEWFILE].CompositeOrigin  = StreamArray[DFCFILE].CompositeOrigin =  0;
	StreamArray[OLDFILE].CurPos = StreamArray[NEWFILE].CurPos  = StreamArray[DFCFILE].CurPos =  0;
	StreamArray[OLDFILE].FileOrigin = StreamArray[NEWFILE].FileOrigin  = StreamArray[DFCFILE].FileOrigin =  0;
	StreamArray[OLDFILE].FileHandle = (HANDLE) OLDFILE;
	StreamArray[DFCFILE].FileHandle = (HANDLE) DFCFILE;
	StreamArray[NEWFILE].FileHandle = (HANDLE) NEWFILE;

	PosArray[OLDFILE] = PosArray[NEWFILE] = PosArray[DFCFILE] = 0;

# ifdef EXAPATCH_SPECIALSTREAM_SUPPORT
	Code = ExaPatchApplySupplyBuffer( ApplyChannel, (void *) Buffer, BUFFER_SIZE );
dmsg("[Function]  ExaPatchApplySupplyBuffer Ret(Code)=[%d]",Code);
	if (Code == EXAPATCH_SUCCESS)
# endif

	{
		Code = ExaPatchApplyDoEntryRaw(
			ApplyChannel,
			&StreamArray[DFCFILE],
			NULL,
			NULL,
			&StreamArray[OLDFILE],
			&StreamArray[NEWFILE],
			&FeedBack,
			0);
	}
	

	if (Code != EXAPATCH_SUCCESS)
	{
		dmsg("ExaPatchApplyDoEntryRaw failed Ret(Code)=[%d]",Code);
		free (g_cpWriteStream);
                free(StreamArray); // @@EB MALLOC MODIFY
		return -1;
	}
	else
	{
		dmsg("ExaPatchApplyDoEntryRaw success Ret(Code)=[%d]",Code);
	}
	dmsg("StreamArray[NEWFILE].Size [%ld] bytes",StreamArray[NEWFILE].Size);

#ifdef __VERIFONE_SRC__
	if (write(bip, g_cpWriteStream, lNewSize) != lNewSize) 
	{
		dmsg("Failed to write at BYTE STREAM FILE by Stream-Buffer.");
	}
	else
	{
		dmsg("success to write at BYTE STREAM FILE by Stream-Buffer.");
	}
		
	free (g_cpWriteStream);
#endif
	free (StreamArray);
	ExaPatchApplyClose( ApplyChannel );

#ifdef __VERIFONE_SRC__
		close(aip); close(bip); close(cip); //close(iBsfd);
// By Allen for Ingenico RTpatch speed 
#elif __INGENICO_SRC__
		FS_flush(bFp);
		FS_close(aFp); FS_close(bFp); FS_close(cFp);
#endif

  dmsg("EXIT: Return Code = %d", Code);

  if(Code == 0)
	  return 1;
  else
	  return -1;
}

DWORD preProcess = 0L;

int CALLBACK FeedBack( DWORD ID, LPVOID Ptr, PLATWORD Handle )
{
  DWORD * dwPtr;
  DWORD TempLocal, TempGlobal;
  switch(ID)
  {
    case EXP_PATCH_WARNING_CALLBACK:
       break;
    case EXP_PATCH_ERROR_CALLBACK:
      break;
    case EXP_PATCH_PROGRESS:
      dwPtr = (DWORD *) Ptr;

	TempGlobal = (100*dwPtr[0]) >> 7;
	TempLocal = (100*dwPtr[1]) >> 7;
      if ((TempGlobal != GlobalProgress) || (TempLocal != LocalProgress) )
      {
        GlobalProgress = TempGlobal;
        LocalProgress = TempLocal;
       }
      if(preProcess != GlobalProgress)
      {
    	  char caMsg[200] = {0,};

    	  dmsg("== GlobalProgress [%ld]%%   [%ld] [%ld]   ", GlobalProgress, (long)dwPtr[0], (long)dwPtr[1]);

    	  //sprintf(caMsg, "PATCH FILE\nAPPLY [%d %%]", (int)GlobalProgress);
    	  sprintf(caMsg, "NEW VERSION\nUPDATE..[%d %%]\nPLEASE\nWAIT", (int)GlobalProgress);

    	  cbDispMsg(caMsg);
    	  preProcess = GlobalProgress;
      }
      break;
    case EXP_PATCH_FILE_START:
        LocalProgress = 0xffffffff;
      break;
    case EXP_PATCH_FILE_FINISH:
      break;
    case EXP_PATCH_INFO:
      break;
    case EXP_PATCH_COMMENT:
      break;
    case EXP_PATCH_COPYRIGHT:
      break;
    case EXP_PATCH_INVALID_PATCH_FILE:
      break;
    case EXP_PATCH_PASSWORD:
      break;
    case EXP_PATCH_INVALID_PASSWORD:
      break;
    case EXP_PATCH_SYSTEM_CONFIRM:
      break;
    case EXP_PATCH_SYSTEM_PROMPT:
      break;
    case EXP_PATCH_DELAYED_PATCHING:
      break;
    case EXP_PATCH_SEARCHING:
      break;
    case EXP_PATCH_EXTENDED_HEADER:
      return EXP_CALLBACK_CONTINUE;
    case EXP_PATCH_FILE_MANIPULATION:
      return EXP_CALLBACK_CONTINUE;
  }

  return(EXP_CALLBACK_OK);
}

int CALLBACK MyRead (HANDLE Index, void * Buffer, DWORD Size, LPDWORD ReturnSize)
{
#ifdef __INGENICO_SRC__
	S_FS_FILE *fp;
	int Code = -1;

#else
	int fp;
	long int lCode = -1;
#endif

	*ReturnSize = 0;

#ifdef __INGENICO_SRC__

	if((int)Index==OLDFILE)
		fp = aFp;
	else if((int)Index==NEWFILE)
		fp = bFp;
	else 	fp = cFp;

	FS_seek(fp, 0, FS_SEEKSET );

	Code = FS_seek(fp, (DWORD)PosArray[(int)Index], FS_SEEKSET);
	if(Code != FS_OK)
	{

		dpt();
		dmsg("FS_seek [%d]", Code);
		return -1;
	}
	Code = FS_read(Buffer, 1, Size, fp);

	if(Code < 1)
	{
		dpt();
		dmsg("FS_read [%d] ", Code);
		return -1;
	}
#else //VERIFONE
	

#if 1
	if((int)Index==OLDFILE) memcpy(Buffer, &g_cpReadOldStream[PosArray[(int)Index]], Size);
	else if((int)Index==NEWFILE) memcpy(Buffer, &g_cpWriteStream[PosArray[(int)Index]], Size);
	else memcpy(Buffer, &g_cpReadPatchStream[PosArray[(int)Index]], Size);
#else

	if((int)Index==OLDFILE) fp = aip;
	else if((int)Index==NEWFILE) fp = bip;
	else fp = cip;

	lseek(fp, 0, SEEK_SET );
	lCode = lseek(fp, (long int)PosArray[(int)Index], SEEK_SET );
	if(lCode != PosArray[(int)Index]) return -1;

	lCode = read(fp, Buffer, Size);
	if(lCode != Size) return -1;
#endif

#endif//VERIFONE

	PosArray[(int)Index] += Size;
	if (ReturnSize)
		*ReturnSize = Size;

	return (EXAPATCH_SUCCESS);
}

int CALLBACK MyWrite (HANDLE Index, const void * Buffer, DWORD Size, LPDWORD ReturnSize)
{
#ifdef __INGENICO_SRC__
	int Code = -1;
	S_FS_FILE *fp;
// By Allen for Ingenico RTpatch speed 
//	char aPath[64 + 1];
#else
	int fp;
	long int lCode = -1;
#endif

	*ReturnSize = 0;

#ifdef __INGENICO_SRC__

	if((int)Index==NEWFILE)
		fp = bFp;
	else
	{
		dmsg("Index error [%d]", (int)Index);
		return -1;
	}

	FS_seek(fp, 0, FS_SEEKSET );

	Code = FS_seek(fp, (DWORD)PosArray[(int)Index], FS_SEEKSET);
	if(Code != FS_OK)
	{
		dpt();
		dmsg("FS_seek [%d]", Code);
		return -1;
	}
	Code = FS_write(Buffer, 1, Size, fp);
	if(Code < Size)
	{
		dpt();
		dmsg("FS_write [%d]", Code);
		return -1;
	}

#else

#if 1
	memcpy(&g_cpWriteStream[PosArray[(int)Index]], Buffer, Size);

#else

	if((int)Index==NEWFILE) fp = bip;
	else return -1;

	lseek(fp, 0, SEEK_SET );
	lCode = lseek(fp, (long int)PosArray[(int)Index], SEEK_SET );
	if(lCode != (long int)PosArray[(int)Index]) return -1;
	
	lCode = write(fp, Buffer, Size);
	if(lCode != Size) return -1;
#endif
#endif

	PosArray[(int)Index] += Size;
	if (ReturnSize)
		*ReturnSize = Size;

// By Allen for Ingenico RTpatch speed 
#ifdef __INGENICO_SRC__
//	FS_close(fp);
	FS_flush(fp);
#endif

	return (EXAPATCH_SUCCESS);
}

int CALLBACK MySeek (HANDLE Index, QWORD Position, DWORD Origin, QWORD * ReturnPosition)
{

	*ReturnPosition = 0;
	PosArray[(int)Index] = Position;
	if (ReturnPosition)
		*ReturnPosition = Position;
	return (EXAPATCH_SUCCESS);
}
