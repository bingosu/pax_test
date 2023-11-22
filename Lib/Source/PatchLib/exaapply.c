/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXAAPPLY.C                                                         |
|                                                                           |
|                                                                           |
|  RTPatch Server Apply Main Worker Routines                                |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2010.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/
/*lint --e{950, 9026, 801, 9042, 9087, 826, 740, 904, 9024, 621, 829, 506, 774, 527, 9011, 850, 750} */
/* Suppress non-ANSI, function-type macro, goto deprecation, switch syntax, suspicious cast, early return,
	## operator, identifier clash, headerwarn, constant Boolean, unreachable code, loop structure,
	and unused macro warnings in this file (all deviations are marked) */
/*lint -esym(*,SrcPtr) */
/* Suppress spurious warnings about a variable that is referenced in an asm block (Windows only) */
# define EXAPATCH_INTERNAL_CODE
# include "exaapply.h"

//test
#ifdef __VERIFONE_SRC__
#include <log/liblog.h>
#endif

#include "libDebug.h"

/* # define TRACE */

# include <stdio.h>
# include <stdarg.h> /* MISRA C 2012 [17.1]: stdarg.h to provide variadic functions (for legacy reasons), but invocations of these are all tightly structured for safety */

# ifdef ATTOPATCH
# define BUFFER_SIZE 0x800U
# else
# define BUFFER_SIZE 0x100000U
# endif

# if defined (_WIN32) && !defined (WINCE) && !defined(_WIN64)
# define DO_INLINE_ASM
# define FASTCALL __fastcall
# else
# define FASTCALL
# endif
/* MISRA C 2012 [1.2]: non-ANSI keyword used in Win32 only to provide performance enhancement (__fastcall) */


# ifdef DUMP_ONLY
# ifdef _WIN32
HANDLE hExaApplyModule;
# endif
# endif
/* local routines forward decl */
static int DoNextBlock( ExaPatchApplyState * TheState );
static int FASTCALL GetCW( ExaPatchApplyState * TheState, const ExaPatchApplyTable * TheTable,
	DWORD * CWPtr, DWORD * CWExtra );
static int FASTCALL GetBits( ExaPatchApplyState * TheState, DWORD * TheBits, DWORD NumBits );
static int FASTCALL PeekBits( ExaPatchApplyState * TheState, DWORD * TheBits, DWORD NumBits );
static int ReadTable( ExaPatchApplyState * TheState, ExaPatchApplyTable * TheTable, DWORD TableType );
static int FreeTable( ExaPatchApplyTable * TheTable );
static int FASTCALL FillBarrel( ExaPatchApplyState * TheState );
static int DoProgress( ExaPatchApplyState * TheState );
static int FinishProgress( const ExaPatchApplyState * TheState );
static QWORD NormalizeCopyPos( QWORD CopyPos, QWORD FileSize );

/* Big messy macros...they REALLY speed things up, though...*/
/* MISRA C 2012 [Dir. 4.9]: macros allow code re-use without subroutine linkage performance penalty (important in this case) */
# define PutBytesMac(B,NB) Code=0;if((TheState->LBUsed+(NB))<BUFFER_SIZE){memcpy(&TheState->LocalBuffer[TheState->LBUsed],(B),(NB));TheState->LBUsed+=(NB);}\
else{if(0U!=TheState->LBUsed){Code=PutBytes(TheState->NewFile,TheState->LocalBuffer,TheState->LBUsed);TheState->LBUsed=0;}if((NB)<=BUFFER_SIZE){memcpy(TheState->LocalBuffer,(B),(NB));TheState->LBUsed=(NB);}\
else{Code=PutBytes(TheState->NewFile,(B),(NB));};};

# define FlushLB if(0U!=TheState->LBUsed){Code=PutBytes(TheState->NewFile,TheState->LocalBuffer,TheState->LBUsed);TheState->LBUsed=0U;}else{Code=0;};

# define GetBitsMac(TB,NB) if((0U != NB)&&((NB)<(TheState->BitsInBarrel)))\
{Code=0;(*(TB))=((1UL<<(NB))-1UL)&(TheState->Barrel>>(TheState->BitsInBarrel-(NB)));TheState->BitsInBarrel-=(NB);}\
else if (0U!=(NB)){Code=GetBits(TheState,(TB),(NB));} else {Code=0;(*(TB))=0;};
/* MISRA C 2012 [10.1]: macro includes suspicious shift that is safe because the NB argument is always an entry from a const table that never exceeds 32 */

# ifdef PLAT_BIG_ENDIAN
# define FillBarrelMac {DWORD Num;unsigned char *Ptr;\
if(TheState->BufSizeLeft>3U){Code=0;Num=(32U-TheState->BitsInBarrel)>>3;\
Ptr=(unsigned char *)&TheState->Barrel;TheState->BitsInBarrel+=Num<<3;\
TheState->BufSizeLeft-=Num;switch(Num){default: case 0:{};break;\
case 1:Ptr[0]=Ptr[1];Ptr[1]=Ptr[2];Ptr[2]=Ptr[3];Ptr[3]=*(TheState->BufferPtr++);break;\
case 2:memcpy(Ptr,Ptr+2,2);Ptr+=2;*(Ptr++)=*(TheState->BufferPtr++);\
*(Ptr++)=*(TheState->BufferPtr++);break;\
case 3:Ptr[0]=Ptr[3];Ptr++;*(Ptr++)=*(TheState->BufferPtr++);\
*(Ptr++)=*(TheState->BufferPtr++);*(Ptr++)=*(TheState->BufferPtr++);break;\
case 4:memcpy(Ptr,TheState->BufferPtr,4);TheState->BufferPtr=&TheState->BufferPtr[4];break;}\
}else{Code=FillBarrel(TheState);}};
# else
# ifdef DO_INLINE_ASM
# define FillBarrelMac {DWORD Num;unsigned char *Pt,*Pt2;\
if(TheState->BufSizeLeft>3U){Code=0;Num=(32U-TheState->BitsInBarrel)>>3;\
Pt=(unsigned char *)&TheState->Barrel;Pt2=TheState->BufferPtr;TheState->BitsInBarrel+=Num<<3;\
TheState->BufferPtr=&TheState->BufferPtr[Num];TheState->BufSizeLeft-=Num;switch(Num){default: case 0:{};break;\
case 1:_asm{_asm mov edi,Pt _asm mov esi,Pt2 _asm mov eax,[edi]\
	_asm shl eax,8 _asm mov al,[esi] _asm mov [edi],eax}break;\
case 2:_asm{_asm mov edi,Pt _asm mov esi,Pt2 _asm mov eax,[edi] _asm shl eax,16\
	_asm mov al,[esi+1] _asm mov ah,[esi] _asm mov [edi],eax}break;\
case 3:_asm{_asm mov edi,Pt _asm mov esi,Pt2 _asm mov ax,[esi+1] _asm shl eax,16\
	_asm mov ah,[esi] _asm mov al,[edi] _asm bswap eax _asm mov [edi],eax}break;\
case 4:_asm{_asm mov esi,Pt2 _asm mov edi,Pt _asm mov eax,[esi] _asm bswap eax\
	_asm mov [edi],eax}break;}}else{Code=FillBarrel(TheState);}};
# else
# define FillBarrelMac {DWORD Num;unsigned char *Ptr;\
if(TheState->BufSizeLeft>3U){Code=0;Num=(32U-TheState->BitsInBarrel)>>3;\
Ptr=(unsigned char *)&TheState->Barrel;TheState->BitsInBarrel+=Num<<3;\
TheState->BufSizeLeft-=Num;switch(Num){default: case 0:{};break;\
case 1:Ptr[3]=Ptr[2];Ptr[2]=Ptr[1];Ptr[1]=Ptr[0];*Ptr=*(TheState->BufferPtr++);break;\
case 2:memcpy(Ptr+2,Ptr,2);Ptr[1]=*(TheState->BufferPtr++);*Ptr=*(TheState->BufferPtr++);break;\
case 3:Ptr[3]=Ptr[0];Ptr[2]=*(TheState->BufferPtr++);\
Ptr[1]=*(TheState->BufferPtr++);Ptr[0]=*(TheState->BufferPtr++);break;\
case 4:Ptr[3]=*(TheState->BufferPtr++);Ptr[2]=*(TheState->BufferPtr++);\
Ptr[1]=*(TheState->BufferPtr++);Ptr[0]=*(TheState->BufferPtr++);break;}\
}else{Code=FillBarrel(TheState);}};
# endif
# endif

# ifndef _WIN32
# ifndef ATTOPATCH
# ifndef DUMP_ONLY
/* Unix seems to lack this... */
# ifdef NOT_NEEDED_ANYMORE
static int _wcsicmp( const WCHAR *, const WCHAR *);
# endif
# endif
# endif
# endif

/* local defines */
# define EXAPATCH_CHARTABLE 0x0U
# define EXAPATCH_DISPTABLE 0x1U
# define EXAPATCH_LENTABLE 0x2U
# define EXAPATCH_TABLE_TYPE 0x3U
# define EXAPATCH_ENCODING_00 0x0U
# define EXAPATCH_ENCODING_01 0x4U
# define EXAPATCH_ENCODING_10 0x8U
# define EXAPATCH_ENCODING_11 0xcU /* MISRA C 2012 [2.5]: currently unused macro - defined for expansion purposes */
# define EXAPATCH_ENCODING_BITS 0xcU
# define EXAPATCH_FLAT 0x10U

# define EXAPATCH_ACTION_COPY 0x0U
# define EXAPATCH_ACTION_WINDOW 0x1U
# define EXAPATCH_ACTION_BYTE_MOD 0x2U
# define EXAPATCH_ACTION_WORD_MOD 0x3U
# define EXAPATCH_ACTION_DWORD_MOD 0x4U

/* local routines */
static DWORD FlatCharBrk = ((DWORD)NUM_SHORT_9) << 24;
static DWORD FlatCharBrkCW[2] = {0,NUM_SHORT_9};

static DWORD FlatDispLenBrk = (NUM_SHORT_6) << 27;
static DWORD FlatDispLenBrkCW[2] = {0,NUM_SHORT_6};

static const DWORD DispBase[]={
	0x1U,
	0x2U,
	0x4U,
	0x8U,
	0x10U,0x20U,0x30U,0x40U,0x50U,0x60U,0x70U,
	0x80U,0xa0U,0xc0U,0xe0U,
	0x100U,0x140U,0x180U,0x1c0U,
	0x200U,0x300U,
	0x400U,0x600U,
	0x800U,0xc00U,
	0x1000U,0x1800U,
	0x2000U,0x3000U,
	0x4000U,0x6000U,
	0x8000U,0xc000U,
	0x10000U,0x20000U,0x30000U,
	0x40000U,0x80000U,0xc0000U,
	0x100000U,
	0x200000U,
	0x400000U,
	0x800000U,
	0x1000000U,
	0x2000000U,
	0x4000000U,
	0x8000000U,
	0x10000000U,
	0x20000000U,
	0x40000000U,
	0x80000000U};

static const unsigned char DispExtraBits[] = {
	0U,
	1U,
	2U,
	3U,
	4U,4U,4U,4U,4U,4U,4U,
	5U,5U,5U,5U,
	6U,6U,6U,6U,
	8U,8U,
	9U,9U,
	10U,10U,
	11U,11U,
	12U,12U,
	13U,13U,
	14U,14U,
	16U,16U,16U,
	18U,18U,18U,
	20U,
	21U,
	22U,
	23U,
	24U,
	25U,
	26U,
	27U,
	28U,
	29U,
	30U,
	31U
	};
static const DWORD LenBase[]={
	0x1U,0x2U,0x3U,
	0x4U,0x6U,
	0x8U,0xcU,0x10U,0x14U,0x18U,0x1cU,
	0x20U,0x28U,0x30U,0x38U,
	0x40U,0x50U,0x60U,0x70U,
	0x80U,0xc0U,
	0x100U,0x180U,
	0x200U,0x300U,
	0x400U,0x600U,
	0x800U,0xc00U,
	0x1000U,0x2000U,0x3000U,
	0x4000U,0x8000U,0xc000U,
	0x10000U,
	0x20000U,
	0x40000U,
	0x80000U,
	0x100000U,
	0x200000U,
	0x400000U,
	0x800000U,
	0x1000000U,
	0x2000000U,
	0x4000000U,
	0x8000000U,
	0x10000000U,
	0x20000000U,
	0x40000000U,
	0x80000000U};
static const unsigned char LenExtraBits[] = {
	0U,0U,0U,
	1U,1U,
	2U,2U,2U,2U,2U,2U,
	3U,3U,3U,3U,
	4U,4U,4U,4U,
	6U,6U,
	7U,7U,
	8U,8U,
	9U,9U,
	10U,10U,
	12U,12U,12U,
	14U,14U,14U,
	16U,
	17U,
	18U,
	19U,
	20U,
	21U,
	22U,
	23U,
	24U,
	25U,
	26U,
	27U,
	28U,
	29U,
	30U,
	31U
	};

# ifndef IO_REPARSE_TAG_SYMLINK
/* not present in all versions of winnt.h */
#   define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)
# endif
# ifndef IO_REPARSE_TAG_MOUNT_POINT
/* not present in all versions of winnt.h */
#   define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)
# endif


static int DoNextBlock( ExaPatchApplyState * TheState )
{
	int Code;
  DWORD BlockType;
	DWORD Dummy;
# ifndef DUMP_ONLY
	unsigned char DummyChar;
# endif
	DWORD Count;
	DWORD ThisCount;
	DWORD CodeWord;
	DWORD ExtraBits;
	DWORD BlockEOF;
	DWORD ActionType = 0;
	/*unsigned char * LocalBuffer = NULL; */
	QWORD CopyPos = 0;
	DWORD CopyLength;
	DWORD DWMod=0;
	USHRT WMod=0;
	unsigned char BMod=0;
	DWORD CurOff;
# ifndef DUMP_ONLY
	DWORD DWModBuf=0;
	USHRT WModBuf=0;
	unsigned char BModBuf=0;
  DWORD * DWBufPtr;
	DWORD BufLeft;
  QWORD PropCount;
  DWORD BufCount;
  DWORD BufSize;
  QWORD ThisProp;
  unsigned char * BBufPtr;
  USHRT * WBufPtr;
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
	unsigned char * UDWPtr;
	unsigned char * UWPtr;
# else
	UNALIGNED DWORD * UDWPtr;
	UNALIGNED USHRT * UWPtr;
# endif
	DWORD ThisLength;
	DWORD DontRead;

# endif

	if (0 != TheState->EOFEncountered)
	{
		return(EXAPATCH_UNEXPECTED_EOF); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	TheState->PBUsed = 0;
	memset( TheState->CharTable, 0, sizeof( ExaPatchApplyTable ) );
	memset( TheState->DispTable, 0, sizeof( ExaPatchApplyTable ) );
	memset( TheState->LenTable, 0, sizeof( ExaPatchApplyTable ) );
# ifndef DUMP_ONLY
	Code = SeekExaStream( TheState->NewFile, TheState->NewFilePos, EXAPATCH_SEEK_BEGIN, NULL );
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	TheState->NeedsSeek = FALSE;
	TheState->LBUsed = 0;
# endif
	TheState->PrevNegOff = 0;
	TheState->PrevPosOff = 0;
	TheState->PrevModOrigin = 0;
# ifdef DUMP_ONLY
# ifdef _WIN32
  Code = ExaPatchDump( TheState, 0, L"Block Start: Entry Offset = 0x%I64x\r\n", (TheState->PatchSize - (TheState->PatchSizeLeft + TheState->BufSizeLeft)) - (TheState->BitsInBarrel >> 3));
# else
  Code = ExaPatchDump( TheState, 0, L"Block Start: Entry Offset = 0x%llx\r\n", (TheState->PatchSize - (TheState->PatchSizeLeft + TheState->BufSizeLeft)) - (TheState->BitsInBarrel >> 3));
# endif
  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
  TheState->NumCW = TheState->NumCopy = TheState->NumWdw = TheState->NumChar
    = TheState->NumMod = TheState->NumPropMod = 0;
# endif
	Code = GetBits( TheState, &BlockType, 8 );
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	switch ( BlockType & 0xc0U )
	{
		case 0x0U:
			/* bypass encoding */
			Code = GetBits( TheState, &Dummy, (TheState->BitsInBarrel)& 7U );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1,16.1]: goto used for consistent exit processing */
			Code = GetBits( TheState, &Count, 32 );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1,16.1]: goto used for consistent exit processing */
# ifdef DUMP_ONLY
      Code = ExaPatchDump( TheState, 1, L"ENC = BYP, Count = 0x%x\r\n",Count-5 );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1,16.1]: goto used for consistent exit processing */
# endif
      if (Count < 5U)
      {
        Code = EXAPATCH_CORRUPT_PATCH_FILE;
        goto exit; /* MISRA C 2012 [15.1,16.1]: goto used for consistent exit processing */
      }
      Count -= 5U;
			while (Count != 0U)
			{
				while ((Count != 0U) && (TheState->BitsInBarrel != 0U))
				{
					Dummy = TheState->Barrel >> (TheState->BitsInBarrel - 8U);
# ifdef DUMP_ONLY
          Code = ExaPatchDump( TheState, 2, L"BYP: 1 alignment character\r\n", Dummy );
# else
					DummyChar = (unsigned char) Dummy;
					PutBytesMac( &DummyChar, 1U ); /* MISRA C 2012 [2.1, 14.3]: macro will generate a constant Boolean which all modern compilers will handle correctly */
# endif
					if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
					TheState->BitsInBarrel -= 8U;
					Count--;
					TheState->NewFilePos++;
				}
				if (0U != Count)
				{
					ThisCount = Count;
					if (ThisCount > TheState->BufSizeLeft)
					{
						ThisCount = TheState->BufSizeLeft;
					}
# ifdef DUMP_ONLY
          Code = ExaPatchDump( TheState, 2, L"BYP: 0x%x chars\r\n", ThisCount );
# else
					PutBytesMac( TheState->BufferPtr, ThisCount );
# endif
					TheState->NewFilePos += ThisCount;
					if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
					TheState->BufferPtr = &TheState->BufferPtr[ThisCount];
					Count -= ThisCount;
					TheState->BufSizeLeft -= ThisCount;
					Code = FillBarrel( TheState );
					if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
					if ((0U != Count) && (TheState->BitsInBarrel == 0U))
					{
						Code = EXAPATCH_UNEXPECTED_EOF;
						goto exit; /* MISRA C 2012 [15.1,16.1]: goto used for consistent exit processing */
					}
				}
				Code = DoProgress( TheState );
				if (0 != Code ) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			}
			break;

		case 0xc0U:
			/* EOF or size marker */
			if (BlockType == 0xffU)
			{
				TheState->EOFEncountered = TRUE;
# ifdef DUMP_ONLY
        Code = ExaPatchDump( TheState, 1, L"EOF\r\n" );
        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
			}
			else
      {
			  Code = GetBits( TheState, &Dummy, (TheState->BitsInBarrel)&7U );
			  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			  Code = GetBits( TheState, &Count, 32 );
			  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			  TheState->NextMark = (TheState->PatchFile->CurPos - (((QWORD)TheState->BufSizeLeft) + (((QWORD)TheState->BitsInBarrel) >> 3))) + (QWORD)(Count);
# ifdef DUMP_ONLY
        Code = ExaPatchDump( TheState, 1, L"MARK: 0x%x\r\n", Count);
        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
			}
			break;

		case 0x40U:
		case 0x80U:
		default:
			/* flat or dynamic encoding */
			/* initialize */
			TheState->NumPDM = 0;
			TheState->OrgPDM = 0;
			TheState->NumPWM = 0;
			TheState->OrgPWM = 0;
			TheState->NumPBM = 0;
			TheState->OrgPBM = 0;
			TheState->PrevNegOff = 0;
			TheState->PrevPosOff = 0;
			/*Code = ExaMemAlloc( NULL, 0x100000, (void **) &LocalBuffer ); */
			/* if (0 != Code) {goto exit;} */

			/* setup decoding tables */
			if (0U != (BlockType & 0x80U))
			{
# ifdef DUMP_ONLY
        Code = ExaPatchDump( TheState, 1, L"ENC = DYN\r\n" );
        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
				/* read them from the patch file */
				Code = ReadTable( TheState, TheState->CharTable, EXAPATCH_CHARTABLE | ((BlockType >> 2) & EXAPATCH_ENCODING_BITS) );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				Code = ReadTable( TheState, TheState->DispTable, EXAPATCH_DISPTABLE | (BlockType & EXAPATCH_ENCODING_BITS) );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				Code = ReadTable( TheState, TheState->LenTable, EXAPATCH_LENTABLE | ( (BlockType << 2) & EXAPATCH_ENCODING_BITS) );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			}
			else
			{
# ifdef DUMP_ONLY
        Code = ExaPatchDump( TheState, 1, L"ENC = STATIC\r\n" );
        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
				/* build them statically */
 				Code = ReadTable( TheState, TheState->CharTable, EXAPATCH_CHARTABLE | EXAPATCH_FLAT );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				Code = ReadTable( TheState, TheState->DispTable, EXAPATCH_DISPTABLE | EXAPATCH_FLAT );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
				Code = ReadTable( TheState, TheState->LenTable, EXAPATCH_LENTABLE | EXAPATCH_FLAT );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
			}
			/* start decoding the encoded bits */
			BlockEOF = FALSE;
			while (0U == BlockEOF)
			{
				Code = GetCW( TheState, TheState->CharTable, &CodeWord, &ExtraBits );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */

				if (CodeWord < 0x100U)
				{
					/* character - put it into the output file */
# ifdef DUMP_ONLY
					Code = ExaPatchDump( TheState, 2, L"LIT - 0x%x\r\n", CodeWord & 0xff);
          TheState->NumChar++;
          TheState->NumCW++;
# else
					if (0U != TheState->PBUsed)
					{
						/* Flush PropBuf */
						Code = PutBytes( TheState->NewFile, TheState->PropBuffer, TheState->PBSize );
						if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
						TheState->PBUsed = 0;
					}
					if (0U != TheState->NeedsSeek)
					{
						TheState->NeedsSeek = FALSE;
						Code = SeekExaStream( TheState->NewFile, TheState->NewFilePos, EXAPATCH_SEEK_BEGIN, NULL );
						if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
					}
					DummyChar = (unsigned char) CodeWord;
					PutBytesMac( &DummyChar, 1U ); /* MISRA C 2012 [2.1, 14.3]: macro will generate a constant Boolean which all modern compilers will handle correctly */
# endif
					if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
					TheState->NewFilePos++;
				}
				else
				{
					/* non-character - do first round parsing */
					/*  (all parsing that is not solely dependent on the action type) */
					switch (CodeWord)
					{
						case CHAR_EOB:
							BlockEOF = TRUE;
# ifdef DUMP_ONLY
              {
# ifdef _WIN32
                ULARGE_INTEGER UI1, UI2, UI3, UI4;

                TheState->NumCW++;
                UI1.u.LowPart = TheState->PrevTime.dwLowDateTime;
                UI1.u.HighPart = TheState->PrevTime.dwHighDateTime;
                GetSystemTimeAsFileTime( &TheState->PrevTime );
                UI3.u.LowPart = UI2.u.LowPart = TheState->PrevTime.dwLowDateTime;
                UI3.u.HighPart = UI2.u.HighPart = TheState->PrevTime.dwHighDateTime;
                UI4.u.LowPart = TheState->StartTime.dwLowDateTime;
                UI4.u.HighPart = TheState->StartTime.dwHighDateTime;
                UI2.QuadPart -= UI1.QuadPart;
                UI2.QuadPart /= 10000;
                UI3.QuadPart -= UI4.QuadPart;
                UI3.QuadPart /= 10000;

							  /* Code = ExaPatchDump( TheState, 2, L"Block EOF [%02d:%02d:%02d.%03d - %02d:%02d:%02d.%03d]",
                  UI2.u.LowPart / 3600000,
                  (UI2.u.LowPart / 60000) % 60,
                  (UI2.u.LowPart/1000) % 60,
                  UI2.u.LowPart % 1000,
                  UI3.u.LowPart / 3600000,
                  (UI3.u.LowPart / 60000) % 60,
                  (UI3.u.LowPart/1000) % 60,
                  UI3.u.LowPart % 1000
                 ); */
	# endif
								Code = ExaPatchDump( TheState, 2, L"Block EOF " );
                Code = ExaPatchDump( TheState, 0, L"Stats = (%d,%d,%d,%d,%d,%d)\r\n",
                  TheState->NumCW, TheState->NumCopy, TheState->NumWdw, TheState->NumChar,
                  TheState->NumMod, TheState->NumPropMod );
                TheState->NumCW = TheState->NumCopy = TheState->NumWdw = TheState->NumChar
                  = TheState->NumMod = TheState->NumPropMod = 0;
							  if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
              }
# endif
							break;

						case CHAR_BYTE_MOD:
							ActionType = EXAPATCH_ACTION_BYTE_MOD;
							BMod = (unsigned char) ExtraBits;
							TheState->PrevByteMod[0xfU & TheState->OrgPBM] = BMod;
							TheState->OrgPBM = 0xfU & (TheState->OrgPBM + 1U);
							if (TheState->NumPBM < 16U)
							{
								TheState->NumPBM++;
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Byte MOD" );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_WORD_MOD:
							ActionType = EXAPATCH_ACTION_WORD_MOD;
							WMod = (USHRT) ExtraBits;
							TheState->PrevWordMod[0xffU & TheState->OrgPWM] = WMod;
							TheState->OrgPWM = 0xffU & (TheState->OrgPWM + 1U);
							if (TheState->NumPWM < 256U)
							{
								TheState->NumPWM++;
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Word MOD" );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_DWORD_MOD:
							ActionType = EXAPATCH_ACTION_DWORD_MOD;
							DWMod = ExtraBits;
							TheState->PrevDWordMod[0xffU & TheState->OrgPDM] = DWMod;
							TheState->OrgPDM = 0xffU & (TheState->OrgPDM + 1U);
							if (TheState->NumPDM < 256U)
							{
								TheState->NumPDM++;
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"DWord MOD" );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_WINDOW:
							ActionType = EXAPATCH_ACTION_WINDOW;
							break;

						case CHAR_PATCH_EVEN:
							ActionType = EXAPATCH_ACTION_COPY;
							CopyPos = TheState->NewFilePos;
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"EVEN Copy" );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_BYTE_MOD_TABLE:
							ActionType = EXAPATCH_ACTION_BYTE_MOD;
							BMod = (unsigned char)TheState->PrevByteMod[0xfU & (TheState->OrgPBM - (1U+ExtraBits))];
							TheState->PrevByteMod[0xfU & (TheState->OrgPBM - (1U+ExtraBits))] =
								TheState->PrevByteMod[0xfU & (TheState->OrgPBM - 1U)];
							TheState->PrevByteMod[0xfU & (TheState->OrgPBM - 1U)] = BMod;
							if (TheState->NumPBM <= ExtraBits)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Table Byte MOD [0x%x]", ExtraBits & 0xf );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_WORD_MOD_TABLE:
							ActionType = EXAPATCH_ACTION_WORD_MOD;
							WMod = (USHRT)TheState->PrevWordMod[0xffU & (TheState->OrgPWM - (1U+ExtraBits))];
							TheState->PrevWordMod[0xffU & (TheState->OrgPWM - (1U+ExtraBits))] =
								TheState->PrevWordMod[0xffU & (TheState->OrgPWM - 1U)];
							TheState->PrevWordMod[0xffU & (TheState->OrgPWM - 1U)] = WMod;
							if (TheState->NumPWM <= ExtraBits)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Table Word MOD [0x%x]", ExtraBits & 0xff );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_DWORD_MOD_TABLE:
							ActionType = EXAPATCH_ACTION_DWORD_MOD;
							DWMod = TheState->PrevDWordMod[0xffU & (TheState->OrgPDM - (1U+ExtraBits))];
							TheState->PrevDWordMod[0xffU & (TheState->OrgPDM - (1U+ExtraBits))] =
								TheState->PrevDWordMod[0xffU & (TheState->OrgPDM - 1U)];
							TheState->PrevDWordMod[0xffU & (TheState->OrgPDM - 1U)] = DWMod;
							if (TheState->NumPDM <= ExtraBits)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Table DWord MOD [0x%x]", ExtraBits & 0xff );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_BYTE_PREV_MOD:
							ActionType = EXAPATCH_ACTION_BYTE_MOD;
							BMod = (unsigned char)TheState->PrevByteMod[0xfU & (TheState->OrgPBM - 1U)];
							if (TheState->NumPBM == 0U)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Prev Byte MOD");
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_WORD_PREV_MOD:
							ActionType = EXAPATCH_ACTION_WORD_MOD;
							WMod = (USHRT)TheState->PrevWordMod[0xffU & (TheState->OrgPWM - 1U)];
							if (TheState->NumPWM == 0U)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Prev Word MOD");
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_DWORD_PREV_MOD:
							ActionType = EXAPATCH_ACTION_DWORD_MOD;
							DWMod = TheState->PrevDWordMod[0xffU & (TheState->OrgPDM - 1U)];
							if (TheState->NumPDM == 0U)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"Prev DWord MOD");
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_FWD_POS:
							ActionType = EXAPATCH_ACTION_COPY;
							Code = GetCW( TheState, TheState->DispTable,
								&CodeWord, &ExtraBits );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							CurOff = DispBase[ CodeWord ] + ExtraBits;
							TheState->PrevPosOff += CurOff;
							CopyPos = TheState->NewFilePos + (QWORD) (TheState->PrevPosOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
              TheState->NumCW++;
							Code = ExaPatchDump( TheState, 2, L"FWD + Copy [0x%x]", CurOff);
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_FWD_EVEN:
							if (TheState->PrevPosOff == 0U)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
							ActionType = EXAPATCH_ACTION_COPY;
							CopyPos = TheState->NewFilePos + (QWORD) (TheState->PrevPosOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"FWD 0 Copy");
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_FWD_NEG:
							if (TheState->PrevPosOff == 0U)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
							ActionType = EXAPATCH_ACTION_COPY;
							Code = GetCW( TheState, TheState->DispTable,
								&CodeWord, &ExtraBits );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							CurOff = DispBase[ CodeWord ] + ExtraBits;
							if (TheState->PrevPosOff <= CurOff)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
							TheState->PrevPosOff -= CurOff;
							CopyPos = TheState->NewFilePos + (QWORD) (TheState->PrevPosOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
              TheState->NumCW++;
							Code = ExaPatchDump( TheState, 2, L"FWD - Copy [0x%x]", CurOff);
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_BACK_POS:
							ActionType = EXAPATCH_ACTION_COPY;
							Code = GetCW( TheState, TheState->DispTable,
								&CodeWord, &ExtraBits );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							CurOff = DispBase[ CodeWord ] + ExtraBits;
							TheState->PrevNegOff += CurOff;
							CopyPos = TheState->NewFilePos - (QWORD) (TheState->PrevNegOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
              TheState->NumCW++;
							Code = ExaPatchDump( TheState, 2, L"REV + Copy [0x%x]", CurOff);
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_BACK_EVEN:
							if (TheState->PrevNegOff == 0U)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
							ActionType = EXAPATCH_ACTION_COPY;
							CopyPos = TheState->NewFilePos - (QWORD) (TheState->PrevNegOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
							Code = ExaPatchDump( TheState, 2, L"REV 0 Copy");
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_BACK_NEG:
							if (TheState->PrevNegOff == 0U)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
							ActionType = EXAPATCH_ACTION_COPY;
							Code = GetCW( TheState, TheState->DispTable,
								&CodeWord, &ExtraBits );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							CurOff = DispBase[ CodeWord ] + ExtraBits;
							if (TheState->PrevNegOff <= CurOff)
							{
								Code = EXAPATCH_CORRUPT_PATCH_FILE;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							}
							TheState->PrevNegOff -= CurOff;
							CopyPos = TheState->NewFilePos - (QWORD) (TheState->PrevNegOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
              TheState->NumCW++;
							Code = ExaPatchDump( TheState, 2, L"REV - Copy [0x%x]", CurOff);
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_FWD_ABS:
							ActionType = EXAPATCH_ACTION_COPY;
							Code = GetCW( TheState, TheState->DispTable,
								&CodeWord, &ExtraBits );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
							CurOff = DispBase[ CodeWord ] + ExtraBits;
							TheState->PrevPosOff = CurOff;
							CopyPos = TheState->NewFilePos + (QWORD) (TheState->PrevPosOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
              TheState->NumCW++;
							Code = ExaPatchDump( TheState, 2, L"FWD Copy [0x%x]", CurOff);
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;

						case CHAR_PATCH_BACK_ABS:
							ActionType = EXAPATCH_ACTION_COPY;
							Code = GetCW( TheState, TheState->DispTable,
								&CodeWord, &ExtraBits );
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */

							CurOff = DispBase[ CodeWord ] + ExtraBits;
							TheState->PrevNegOff = CurOff;
							CopyPos = TheState->NewFilePos - (QWORD) (TheState->PrevNegOff);
							CopyPos = NormalizeCopyPos( CopyPos, TheState->OldFileSize );
# ifdef DUMP_ONLY
              TheState->NumCW++;
							Code = ExaPatchDump( TheState, 2, L"REV Copy [0x%x]", CurOff);
							if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
# endif
							break;
						default:
							/* Other cases require no handling in this round */
							break;
					}
					if ((0U != TheState->PBUsed) && ((0U != BlockEOF) || (ActionType <= EXAPATCH_ACTION_WINDOW)))
					{
						/* Flush PropBuf */
						Code = PutBytes( TheState->NewFile, TheState->PropBuffer, TheState->PBSize );
						if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
						TheState->PBUsed = 0;
					}
					if (0U == BlockEOF)
					{
						/* do second-round parsing and actual action */
						switch ( ActionType )
						{
							default:
								/* any other value indicates an internal error */
								Code = EXAPATCH_INTERNAL_ERROR;
								goto exit; /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								break; /* MISRA C 2012 [2.1]: unreachable code (alternative is MISRA switch syntax departure) */
							case EXAPATCH_ACTION_COPY:
								/* set PrevModOrigin */
# ifdef TRACE
                printf("Copy from %x", CopyPos );
                printf(" to %x\n", TheState->NewFilePos );
# endif
								TheState->PrevModOrigin = TheState->NewFilePos;
                TheState->PrevModOffset = 0;
								/* read length */
								Code = GetCW( TheState, TheState->LenTable,
									&CodeWord, &ExtraBits );
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								CopyLength = LenBase[ CodeWord ] + ExtraBits;
                TheState->PrevModLength = CopyLength;
								/* do copy */
# ifdef DUMP_ONLY
                TheState->NumCW += 2;
                TheState->NumCopy++;
# ifdef _WIN32
								Code = ExaPatchDump( TheState, 0, L"-> 0x%I64x, LEN = 0x%x\r\n", CopyPos, CopyLength );
# else
								Code = ExaPatchDump( TheState, 0, L"-> 0x%llx, LEN = 0x%x\r\n", CopyPos, CopyLength );
# endif
								if (TheState->MaxOldPos < (CopyPos + CopyLength))
								{
									TheState->MaxOldPos = CopyPos + CopyLength;
								}
# else
								if (0U != TheState->NeedsSeek)
								{
									TheState->NeedsSeek = FALSE;
									Code = SeekExaStream( TheState->NewFile, TheState->NewFilePos, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								}
								Code = SeekExaStream( TheState->OldFile, CopyPos, EXAPATCH_SEEK_BEGIN, NULL );
# endif
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								TheState->NewFilePos += CopyLength;
# ifndef DUMP_ONLY
								FlushLB;
								while (0U != CopyLength)
								{
									ThisLength = BUFFER_SIZE;
									if (ThisLength > CopyLength)
									{
										ThisLength = CopyLength;
									}
									Code = GetBytes( TheState->OldFile, TheState->LocalBuffer, ThisLength );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = PutBytes( TheState->NewFile, TheState->LocalBuffer, ThisLength );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									CopyLength -= ThisLength;
								}
# endif
								Code = DoProgress( TheState );
								if (0 != Code ) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								break;

							case EXAPATCH_ACTION_WINDOW:
								/* set PrevModOrigin */
								TheState->PrevModOrigin = TheState->NewFilePos;
								/* read disp */
								Code = GetCW( TheState, TheState->DispTable,
									&CodeWord, &ExtraBits );
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								CurOff = DispBase[ CodeWord ] + ExtraBits;
								CopyPos = TheState->NewFilePos - (QWORD) CurOff;
								TheState->PrevModOffset = (QWORD) CurOff;
# ifdef TRACE
                printf("Window Copy from %x", CurOff);
                printf(" to %x\n", TheState->NewFilePos );
# endif
								/* read length */
								Code = GetCW( TheState, TheState->LenTable,
									&CodeWord, &ExtraBits );
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								CopyLength = LenBase[ CodeWord ] + ExtraBits;
								TheState->PrevModLength = CopyLength;
# ifdef DUMP_ONLY
                TheState->NumCW += 3;
                TheState->NumWdw++;
								if (TheState->MaxBackSeek < CurOff)
								{
									TheState->MaxBackSeek = CurOff;
								}
# ifdef _WIN32
								Code = ExaPatchDump( TheState, 2, L"WDW [0x%x] -> 0x%I64x, LEN = 0x%x\r\n", CurOff, TheState->NewFilePos - ((QWORD)CurOff), CopyLength );
# else
								Code = ExaPatchDump( TheState, 2, L"WDW [0x%x] -> 0x%llx, LEN = 0x%x\r\n", CurOff, TheState->NewFilePos - ((QWORD)CurOff), CopyLength );
# endif
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								TheState->NewFilePos += CopyLength;
# else
								/* do copy */
								if ((CopyLength <= BUFFER_SIZE) && (CopyLength > CurOff))
								{
									/* Use PropBuffer */
									DWORD ThisSize;

									FlushLB;
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									TheState->NeedsSeek = FALSE;
									TheState->PBUsed = 1;
									TheState->PBSize = CopyLength;
									TheState->PBOffset = 0;
									Code = SeekExaStream( TheState->NewFile, CopyPos, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = GetBytes( TheState->NewFile, TheState->PropBuffer, CurOff );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = SeekExaStream( TheState->NewFile, TheState->NewFilePos, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									BufLeft = CopyLength - CurOff;
									BBufPtr = &TheState->PropBuffer[CurOff];
									BufSize = CurOff;
									while (0U != BufLeft)
									{
										ThisSize = BufLeft;
										if (ThisSize > BufSize)
										{
											ThisSize = BufSize;
										}
										memmove( BBufPtr, TheState->PropBuffer, ThisSize );
										BufSize <<= 1;
										BufLeft -= ThisSize;
										BBufPtr = &BBufPtr[ThisSize];
									}
									TheState->NewFilePos += (QWORD) CopyLength;

								}
								else
								{
									FlushLB;
									TheState->NeedsSeek = FALSE;
									DontRead = FALSE;
									while (CopyLength != 0U)
									{
										ThisLength = BUFFER_SIZE;
										if (ThisLength > CopyLength)
										{
											ThisLength = CopyLength;
										}
										if (ThisLength > CurOff)
										{
											ThisLength = CurOff;
										}
										if (0U == DontRead)
										{
											Code = SeekExaStream( TheState->NewFile, CopyPos, EXAPATCH_SEEK_BEGIN, NULL );
											if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
											Code = GetBytes( TheState->NewFile, TheState->LocalBuffer, ThisLength );
											if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
											Code = SeekExaStream( TheState->NewFile, TheState->NewFilePos, EXAPATCH_SEEK_BEGIN, NULL );
											if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
										}
										Code = PutBytes( TheState->NewFile, TheState->LocalBuffer, ThisLength );
										if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
										if (ThisLength == CurOff)
										{
											DontRead = TRUE;
											if (CurOff <= (BUFFER_SIZE/2U))
											{
												memmove( &TheState->LocalBuffer[ThisLength], TheState->LocalBuffer, ThisLength );
												CopyPos -= (QWORD) CurOff;
												CurOff <<= 1;
											}
										}
										TheState->NewFilePos += (QWORD) ThisLength;
										CopyPos += (QWORD) ThisLength;
										CopyLength -= ThisLength;
									}
								}
# endif
								Code = DoProgress( TheState );
								if ( 0!= Code )	{goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								break;

							case EXAPATCH_ACTION_BYTE_MOD:
								if (0U == TheState->PBUsed)
								{
									FlushLB;
									TheState->NeedsSeek=TRUE;
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								}
								/* read disp */
								Code = GetCW( TheState, TheState->DispTable,
									&CodeWord, &ExtraBits );
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								CurOff = DispBase[ CodeWord ] + ExtraBits;
								TheState->PrevModOrigin += (QWORD) CurOff;
                if (TheState->PrevModLength > (QWORD) CurOff)
                {
                  TheState->PrevModLength -= (QWORD) CurOff;
                }
                else
                {
                  TheState->PrevModLength = 0;
                }
# ifdef DUMP_ONLY
                TheState->NumCW += 2;
                TheState->NumMod++;
								if (TheState->MaxBackSeek < ((DWORD)(1 + TheState->NewFilePos - TheState->PrevModOrigin)))
								{
									TheState->MaxBackSeek = (DWORD)(1 + TheState->NewFilePos - TheState->PrevModOrigin);
								}
# ifdef _WIN32
								Code = ExaPatchDump( TheState, 0, L" -> 0x%x, DISP = 0x%x (0x%I64x)",BMod, CurOff, TheState->PrevModOrigin );
# else
								Code = ExaPatchDump( TheState, 0, L" -> 0x%x, DISP = 0x%x (0x%llx)",BMod, CurOff, TheState->PrevModOrigin );
# endif
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
                if (TheState->PrevModOffset && (TheState->PrevModOffset <= TheState->PrevModLength))
								{
                  TheState->NumPropMod++;
                  if (TheState->PrevModOffset == 1)
								  {
										Code = ExaPatchDump( TheState, 0, L", PROP = CONT\r\n" );
								  }
									else
									{
										Code = ExaPatchDump( TheState, 0, L", PROP = SCATTER\r\n" );
									}

								}
								else
								{
									Code = ExaPatchDump( TheState, 0, L"\r\n" );
								}
# else
								/* perform mod */
								if (0U != TheState->PBUsed)
								{
									TheState->PBOffset += CurOff;
									TheState->PropBuffer[TheState->PBOffset - 1U] += BMod;
	                if ((0U != TheState->PrevModOffset) && (TheState->PrevModOffset <= TheState->PrevModLength))
									{
										BModBuf = TheState->PropBuffer[TheState->PBOffset - 1U];
										if (TheState->PBSize > TheState->PBOffset)
										{
											if (TheState->PrevModOffset == 1U)
											{
												/* continuous propagation */
												memset( &TheState->PropBuffer[TheState->PBOffset], (int)BModBuf, TheState->PBSize - TheState->PBOffset );
											}
											else
											{
		                    /* scattered propagation */
		                    PropCount = TheState->PrevModLength / TheState->PrevModOffset;
		                    for (ThisProp = 0U; ThisProp < PropCount; ThisProp++)
												{
													TheState->PropBuffer[TheState->PBOffset + (TheState->PrevModOffset*(ThisProp + 1U))-1U]=BModBuf;
												}
											}
										}
									}
								}
								else
								{
									Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin - 1U, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = GetBytes( TheState->NewFile, &BModBuf, 1 );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									BModBuf += BMod;
									Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin - 1U, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = PutBytes( TheState->NewFile, &BModBuf, 1 );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                /* propagate the Mod */
	                if ((0U != TheState->PrevModOffset) && (TheState->PrevModOffset <= TheState->PrevModLength))
	                {
	                  if (TheState->PrevModOffset == 1U)
	                  {
	                    /* continuous propagation */
	                    BufSize = BUFFER_SIZE;
	                    ThisLength = (DWORD)TheState->PrevModLength;
	                    CopyLength = ThisLength;
	                    if (ThisLength > BufSize)
	                    {
	                      ThisLength = BufSize;
	                    }
	                    memset( TheState->LocalBuffer, (int)BModBuf, ThisLength );
	                    while( 0U != CopyLength )
	                    {
	                      if (ThisLength > CopyLength)
	                      {
	                        ThisLength = CopyLength;
	                      }
	                      Code = PutBytes( TheState->NewFile, TheState->LocalBuffer, ThisLength );
	                      if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                      CopyLength -= ThisLength;
	                    }
	                  }
	                  else
	                  {
	                    /* scattered propagation */
	                    PropCount = TheState->PrevModLength / TheState->PrevModOffset;
	                    for (ThisProp = 0; ThisProp < PropCount; ThisProp++)
	                    {
					dmsg("*** new file pos[%ld]", (unsigned long)(TheState->PrevModOrigin + (TheState->PrevModOffset * (ThisProp+1U)) - 1U));
	                        Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin + (TheState->PrevModOffset * (ThisProp+1U)) - 1U,
	                          EXAPATCH_SEEK_BEGIN, NULL );
	                        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                        Code = PutBytes( TheState->NewFile, &BModBuf, 1 );
	                        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                    }
	                  }
	                }
								}
# endif
								break;

							case EXAPATCH_ACTION_WORD_MOD:
								/*
								 NOTE: Word Mods must be applied as little-endian
									regardless of platform
								*/
								if (0U == TheState->PBUsed)
								{
									FlushLB;
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									TheState->NeedsSeek=TRUE;
								}
								/* read disp */
								Code = GetCW( TheState, TheState->DispTable,
									&CodeWord, &ExtraBits );
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								CurOff = DispBase[ CodeWord ] + ExtraBits;
								TheState->PrevModOrigin += (QWORD) CurOff;
                if (TheState->PrevModLength > (QWORD) CurOff)
                {
                  TheState->PrevModLength -= (QWORD) CurOff;
                }
                else
                {
                  TheState->PrevModLength = 0;
                }
# ifdef DUMP_ONLY
                TheState->NumCW += 2;
                TheState->NumMod++;
								if (TheState->MaxBackSeek < ((DWORD)(2U + TheState->NewFilePos - TheState->PrevModOrigin)))
								{
									TheState->MaxBackSeek = (DWORD)(2U + TheState->NewFilePos - TheState->PrevModOrigin);
								}
# ifdef _WIN32
								Code = ExaPatchDump( TheState, 0, L" -> 0x%x, DISP = 0x%x (0x%I64x)",WMod, CurOff, TheState->PrevModOrigin );
# else
								Code = ExaPatchDump( TheState, 0, L" -> 0x%x, DISP = 0x%x (0x%llx)",WMod, CurOff, TheState->PrevModOrigin );
# endif
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
                if ((0U != TheState->PrevModOffset) && (TheState->PrevModOffset <= (TheState->PrevModLength+1)))
                {
                  TheState->NumPropMod++;
                  if (TheState->PrevModOffset <= 2)
                  {
										Code = ExaPatchDump( TheState, 0, L", PROP = CONT\r\n" );
                  }
									else
									{
										Code = ExaPatchDump( TheState, 0, L", PROP = SCATTER\r\n" );
									}
                }
								else
								{
									Code = ExaPatchDump( TheState, 0, L"\r\n" );
								}
# else
								/* perform mod */
								if (0U != TheState->PBUsed)
								{
									TheState->PBOffset += CurOff;
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
									UWPtr = (unsigned char *)&TheState->PropBuffer[TheState->PBOffset - 2U]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
									WModBuf = WORD_SWAP(WMod + AUA_W_LE_GET(UWPtr));
									AUA_W_PUT(UWPtr,WModBuf);
									UWPtr += 2;
# else
									UWPtr = (UNALIGNED USHRT *) (&TheState->PropBuffer[TheState->PBOffset - 2U]); /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
									*UWPtr = WORD_SWAP(WMod + WORD_SWAP(*UWPtr));
									WModBuf = *UWPtr;
									/* note: at this point, WModBuf is correct
									   to be assigned into place (or used as array of chars) */
									UWPtr++;
# endif
									if (TheState->PBSize > TheState->PBOffset)
									{
										if (TheState->PrevModOffset <= 2U)
										{
											/* continuous byte or word propagation */
											if (TheState->PrevModOffset == 1U)
											{
												/* continuous byte propagation */
												BModBuf = TheState->PropBuffer[TheState->PBOffset - 1U];
												memset( &TheState->PropBuffer[TheState->PBOffset], (int)BModBuf, TheState->PBSize - TheState->PBOffset );
											}
											else
											{
												/* continuous word propagation */
												BufCount = (1U + TheState->PBSize - TheState->PBOffset) >> 1;
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
												while (0U != BufCount)
												{
													AUA_W_PUT(UWPtr,WModBuf);
													UWPtr += 2;
													BufCount--;
												}
# else
												while (0U != BufCount)
												{
													*UWPtr = WModBuf;
													UWPtr++;
													BufCount--;
												}
# endif
											}
										}
										else
										{
											/* scattered propagation */
											PropCount = (TheState->PrevModLength+1U) / TheState->PrevModOffset;
	                    for (ThisProp = 0; ThisProp < PropCount; ThisProp++)
	                    {
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
												UWPtr = (unsigned char *) &TheState->PropBuffer[TheState->PBOffset + (TheState->PrevModOffset*(ThisProp+1U))-2U]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
												AUA_W_PUT(UWPtr,WModBuf);

# else
												UWPtr = (UNALIGNED USHRT *) &TheState->PropBuffer[TheState->PBOffset + (TheState->PrevModOffset*(ThisProp+1U))-2U]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
												*UWPtr = WModBuf;
# endif
	                    }
										}
									}
								}
								else
								{
									Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin - 2U, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = GetWord( TheState->NewFile, &WModBuf );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									WModBuf += WMod;
									Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin - 2U, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = PutWord( TheState->NewFile, WModBuf );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                /* propagate the Mod */
									WModBuf = WORD_SWAP(WModBuf);
									/* note: at this point, WModBuf is correct
									   to be assigned into place (or used as array of chars) */
	                if ((0U != TheState->PrevModOffset) && (TheState->PrevModOffset <= (TheState->PrevModLength+1U)))
	                {
	                  if (TheState->PrevModOffset <= 2U)
	                  {
	                    /* continuous byte or word propagation */
	                    BufSize = BUFFER_SIZE;
	                    ThisLength = (DWORD)TheState->PrevModLength;
	                    CopyLength = ThisLength;
	                    if (ThisLength > BufSize)
	                    {
	                      ThisLength = BufSize;
	                    }
	                    if (TheState->PrevModOffset == 1U)
	                    {
	                      /* byte propagation */
	                      memset( TheState->LocalBuffer, (int)(((unsigned char *)&WModBuf)[1]), ThisLength );
	                    }
	                    else
	                    {
	                      /* word propagation */
	                      WBufPtr = (USHRT*)TheState->LocalBuffer; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
	                      BufCount = (ThisLength + 1U) >> 1;
	                      while( 0U != BufCount )
	                      {
	                        *WBufPtr =WModBuf;
	                        WBufPtr++;
	                        BufCount--;
	                      }
	                    }
	                    while( 0U != CopyLength )
	                    {
	                      if (ThisLength > CopyLength)
	                      {
	                        ThisLength = CopyLength;
	                      }
	                      Code = PutBytes( TheState->NewFile, TheState->LocalBuffer, ThisLength );
	                      if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                      CopyLength -= ThisLength;
	                    }
	                  }
	                  else	                  {
	                    /* scattered propagation */
	                    PropCount = (TheState->PrevModLength+1U)/ TheState->PrevModOffset;
	                    for (ThisProp = 0; ThisProp < PropCount; ThisProp++)
	                    {
	                        Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin + (TheState->PrevModOffset * (ThisProp+1U)) - 2U,
	                          EXAPATCH_SEEK_BEGIN, NULL );
	                        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                        if ((TheState->NewFile->CurPos+2U) > TheState->NewFilePos)
	                        {
	                          Code = PutBytes( TheState->NewFile, &WModBuf, 1 );
	                        }
	                        else
	                        {
	                          Code = PutBytes( TheState->NewFile, &WModBuf, 2 );
	                        }
	                        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                    }
	                  }
	                }
								}
# endif
								break;

							case EXAPATCH_ACTION_DWORD_MOD:
								/*
								 NOTE: DWord Mods must be applied as little-endian
									regardless of platform
								*/
								if (0U == TheState->PBUsed)
								{
									FlushLB;
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									TheState->NeedsSeek=TRUE;
								}
								/* read disp */
								Code = GetCW( TheState, TheState->DispTable,
									&CodeWord, &ExtraBits );
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
								CurOff = DispBase[ CodeWord ] + ExtraBits;
								TheState->PrevModOrigin += (QWORD) CurOff;
                if (TheState->PrevModLength > (QWORD) CurOff)
                {
                  TheState->PrevModLength -= (QWORD) CurOff;
                }
                else
                {
                  TheState->PrevModLength = 0;
                }
# ifdef DUMP_ONLY
                TheState->NumCW+= 2U;
                TheState->NumMod++;
								if (TheState->MaxBackSeek < ((DWORD)(4U + TheState->NewFilePos - TheState->PrevModOrigin)))
								{
									TheState->MaxBackSeek = (DWORD)(4U + TheState->NewFilePos - TheState->PrevModOrigin);
								}
# ifdef _WIN32
								Code = ExaPatchDump( TheState, 0, L" -> 0x%x, DISP = 0x%x (0x%I64x)",DWMod, CurOff, TheState->PrevModOrigin );
# else
								Code = ExaPatchDump( TheState, 0, L" -> 0x%x, DISP = 0x%x (0x%llx)",DWMod, CurOff, TheState->PrevModOrigin );
# endif
								if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
                if (TheState->PrevModOffset && (TheState->PrevModOffset <= (TheState->PrevModLength+3U)))
								{
                  TheState->NumPropMod++;
                  if (TheState->PrevModOffset <= 4U)
								  {
										Code = ExaPatchDump( TheState, 0, L", PROP = CONT\r\n" );
								  }
									else
									{
										Code = ExaPatchDump( TheState, 0, L", PROP = SCATTER\r\n" );
									}
								}
								else
								{
									Code = ExaPatchDump( TheState, 0, L"\r\n" );
								}
# else
								/* perform mod */
								if (0U != TheState->PBUsed)
								{
									TheState->PBOffset += CurOff;
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
									UDWPtr = (unsigned char *) &TheState->PropBuffer[TheState->PBOffset - 4U]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
									DWModBuf = DWORD_SWAP(DWMod + AUA_DW_LE_GET(UDWPtr));
									AUA_DW_PUT(UDWPtr,DWModBuf);
									UDWPtr += 4;

# else
									UDWPtr = (UNALIGNED DWORD *) &TheState->PropBuffer[TheState->PBOffset - 4U]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
									*UDWPtr = DWORD_SWAP(DWMod + DWORD_SWAP(*UDWPtr));
									DWModBuf = *UDWPtr;
									/* note: at this point, DWModBuf is correct
									   to be assigned into place (or used as array of chars) */
									UDWPtr++;
# endif
									if (TheState->PBSize > TheState->PBOffset)
									{
										if (TheState->PrevModOffset <= 4U)
										{
		                  /* continuous byte, word, 3-byte, or dword propagation */
											if (TheState->PrevModOffset == 1U)
											{
												/* continuous byte propagation */
												BModBuf = TheState->PropBuffer[TheState->PBOffset - 1U];
												memset( &TheState->PropBuffer[TheState->PBOffset], (int)BModBuf, TheState->PBSize - TheState->PBOffset );
											}
											else if (TheState->PrevModOffset == 2U)
											{
												/* continuous word propagation */
												BufCount = (1U + TheState->PBSize - TheState->PBOffset) >> 1;
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
												UWPtr = UDWPtr;
												WModBuf = AUA_W_GET(UWPtr-2);
												while (0U != BufCount)
												{
													AUA_W_PUT(UWPtr,WModBuf);
													UWPtr+=2;
													BufCount--;
												}
# else
												UWPtr = (UNALIGNED USHRT *) UDWPtr; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
												WModBuf = UWPtr[-1];
												while (0U != BufCount)
												{
													*UWPtr = WModBuf;
													UWPtr++;
													BufCount--;
												}
# endif
											}
											else if (TheState->PrevModOffset == 3U)
											{
												unsigned char ThreeByte[3];

												BBufPtr = (unsigned char *) UDWPtr;
												ThreeByte[0] = BBufPtr[-3];
												ThreeByte[1] = BBufPtr[-2];
												ThreeByte[2] = BBufPtr[-1];
												BufCount = (2U + TheState->PBSize - TheState->PBOffset) / 3U;
												while (0U != BufCount)
												{
												  *BBufPtr = ThreeByte[0];
												  BBufPtr++;
												  *BBufPtr = ThreeByte[1];
												  BBufPtr++;
												  *BBufPtr = ThreeByte[2];
												  BBufPtr++;
												  BufCount--;
												}
											}
											else
											{
												/* continuous DWord propagation */
													BufCount = (3U + TheState->PBSize - TheState->PBOffset) >> 2;
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
												while (0U != BufCount)
												{
													AUA_DW_PUT(UDWPtr,DWModBuf);
													UDWPtr+=4;
													BufCount--;
												}
# else
												while (0U != BufCount)
												{
													*UDWPtr = DWModBuf;
													UDWPtr++;
													BufCount--;
												}
# endif
											}
										}
										else
										{
		                  /* scattered propagation */
		                  PropCount = (TheState->PrevModLength+3U) / TheState->PrevModOffset;
	                    for (ThisProp = 0; ThisProp < PropCount; ThisProp++)
	                    {
# ifdef EXAPATCH_AVOID_UNALIGNED_ACCESS
												UDWPtr = (unsigned char *) &TheState->PropBuffer[TheState->PBOffset + (TheState->PrevModOffset*(ThisProp+1U))-4U]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
												AUA_DW_PUT(UDWPtr,DWModBuf);
# else
												UDWPtr = (UNALIGNED DWORD *) &TheState->PropBuffer[TheState->PBOffset + (TheState->PrevModOffset*(ThisProp+1U))-4U]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
												*UDWPtr = DWModBuf;
# endif
	                    }
										}
									}
								}
								else
								{
									if (TheState->PrevModOrigin > TheState->NewFilePos)
									{
										/* occasionally, we get a DWORD mod 3 bytes from the end of the copy
											make sure this doesn't cause a problem...
										*/
										unsigned char Buf = 0U;
										Code = SeekExaStream( TheState->NewFile, TheState->NewFilePos, EXAPATCH_SEEK_BEGIN, NULL );
										if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
										Code = PutBytes( TheState->NewFile, (const void *) &Buf, 1 );
										if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									}
									Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin - 4U, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = GetDword( TheState->NewFile, &DWModBuf );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									DWModBuf += DWMod;
									Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin - 4U, EXAPATCH_SEEK_BEGIN, NULL );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									Code = PutDword( TheState->NewFile, DWModBuf );
									if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
									/* propagate the Mod */
									DWModBuf = DWORD_SWAP(DWModBuf);
									/* note: at this point, DWModBuf is correct
									   to be assigned into place (or used as array of chars) */
	                if ((0U != TheState->PrevModOffset) && (TheState->PrevModOffset <= (TheState->PrevModLength+3U)))
	                {
	                  if (TheState->PrevModOffset <= 4U)
	                  {
	                    /* continuous byte, word, 3-byte, or dword propagation */
	                    BufSize = BUFFER_SIZE;
	                    ThisLength = (DWORD)TheState->PrevModLength;
	                    CopyLength = ThisLength;
	                    if (TheState->PrevModOffset == 3U)
	                    {
	                      BufSize = 0xfffffU;
	                    }
	                    if (ThisLength > BufSize)
	                    {
	                      ThisLength = BufSize;
	                    }
	                    if (TheState->PrevModOffset == 1U)
	                    {
	                      /* byte propagation */
	                      memset( TheState->LocalBuffer, (int)((unsigned char *) &DWModBuf)[3], ThisLength );
	                    }
	                    else if (TheState->PrevModOffset == 2U)
	                    {
	                      /* word propagation */
	                      WBufPtr = (USHRT *) TheState->LocalBuffer; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
	                      WModBuf = ((USHRT *) &DWModBuf)[1]; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
	                      BufCount = (ThisLength + 1U) >> 1;
	                      while( 0U != BufCount )
	                      {
	                        *WBufPtr = WModBuf;
	                        WBufPtr++;
	                        BufCount--;
	                      }

	                    }
	                    else if (TheState->PrevModOffset == 3U)
	                    {
	                      /* 3-byte propagation */
	                      unsigned char ThreeByte[3];

	                      BBufPtr = (unsigned char *) TheState->LocalBuffer;
	                      ThreeByte[0] = ((unsigned char *) &DWModBuf)[1];
	                      ThreeByte[1] = ((unsigned char *) &DWModBuf)[2];
	                      ThreeByte[2] = ((unsigned char *) &DWModBuf)[3];
	                      BufCount = (ThisLength + 2U)/3U;
	                      while( 0U != BufCount )
	                      {
	                        *BBufPtr = ThreeByte[0];
	                        BBufPtr++;
	                        *BBufPtr = ThreeByte[1];
	                        BBufPtr++;
	                        *BBufPtr = ThreeByte[2];
	                        BBufPtr++;
	                        BufCount--;
	                      }

	                    }
	                    else
	                    {
	                      /* dword propagation */
	                      DWBufPtr = (DWORD *) TheState->LocalBuffer; /* MISRA C 2012 [11.3]: algorithmically necessary (and correct) pointer conversion */
	                      BufCount = (ThisLength + 3U) >> 2;
	                      while( 0U != BufCount )
	                      {
	                        *DWBufPtr = DWModBuf;
	                        DWBufPtr++;
	                        BufCount--;
	                      }
	                    }
	                    while( 0U != CopyLength )
	                    {
	                      if (ThisLength > CopyLength)
	                      {
	                        ThisLength = CopyLength;
	                      }
	                      Code = PutBytes( TheState->NewFile, TheState->LocalBuffer, ThisLength );
	                      if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                      CopyLength -= ThisLength;
	                    }
	                  }
	                  else
	                  {
	                    /* scattered propagation */
	                    PropCount = (TheState->PrevModLength + 3U)/ TheState->PrevModOffset;
	                    for (ThisProp = 0; ThisProp < PropCount; ThisProp++)
	                    {
	                        Code = SeekExaStream( TheState->NewFile, TheState->PrevModOrigin + (TheState->PrevModOffset * (ThisProp+1U)) - 4U,
	                          EXAPATCH_SEEK_BEGIN, NULL );
	                        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                        if ((TheState->NewFile->CurPos+4U) > TheState->NewFilePos)
	                        {
	                          Code = PutBytes( TheState->NewFile, &DWModBuf, (DWORD)(TheState->NewFilePos - TheState->NewFile->CurPos) );
	                        }
	                        else
	                        {
	                          Code = PutBytes( TheState->NewFile, &DWModBuf, 4 );
	                        }
	                        if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1, 16.1]: goto used for consistent exit processing */
	                    }
	                  }
	                }
								}
# endif
								break;
						}
					}
				}
			}
			/* here when we decoded an EOB mark */
			break;
	}
	FlushLB;
exit:
	/* cleanup */
	/*
	if (LocalBuffer)
	{
		ExaMemFree( NULL, LocalBuffer );
		LocalBuffer = NULL;
	}
	*/
	(void)FreeTable( TheState->CharTable );
	(void)FreeTable( TheState->DispTable );
	(void)FreeTable( TheState->LenTable );
	return(Code);
}

static int FASTCALL GetCW( ExaPatchApplyState * TheState, const ExaPatchApplyTable * TheTable,
	DWORD * CWPtr, DWORD * CWExtra )
{
	DWORD Temp;
	DWORD i;
  int Code;
# ifdef TRACE
  DWORD Bits;
# endif

  if (0U != TheTable->MinLength)
  {
	  if (TheState->BitsInBarrel <= TheTable->MaxLength)
	  {
		  FillBarrelMac;
		  if (0!=Code)
		  {
		  	return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			}
	  }
	  Temp = TheState->Barrel << (32U-TheState->BitsInBarrel);
	  for (i=TheTable->StartLevel; (i < TheTable->NumLevels) && (TheTable->BrkPt[i] <= Temp) ; i++ ) {};
	  if ((0U != TheTable->StartLevel) && (i == TheTable->StartLevel))
	  {
		  for (;(0U != i) && (TheTable->BrkPt[i-1U] > Temp) ; i--) {};
	  }
	  i += TheTable->MinLength;
	  if (i >= TheState->BitsInBarrel)
	  {
		  return(EXAPATCH_UNEXPECTED_EOF); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	  }
 # ifdef TRACE
    Bits = Temp >> (32U-i);
 # endif
	  Temp = (Temp >> (32U-i)) - TheTable->BrkCW[i - TheTable->MinLength];
	  TheState->BitsInBarrel -= i;
	  if (NULL != TheTable->CW)
	  {
		  Temp = (TheTable->CW)[Temp];
		}
 # ifdef TRACE
    printf("%x-", Temp);
    printf("%x", Bits);
    printf("(%d)",i);
 # endif
  }
  else
  {
     Temp = TheTable->EscCW;
# ifdef TRACE
    printf("%x-0(0)", Temp );
# endif
  }
	if (Temp == TheTable->EscCW)
	{
		Code = GetBits( TheState, &Temp, TheTable->EscSize );
		if (0 != Code)
		{
			  return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
		}
# ifdef TRACE
    printf("-%x(%i)", Temp, TheTable->EscSize );
# endif
	}
	if (NULL != CWPtr)
	{
		*CWPtr = Temp;
	}
	if (NULL != CWExtra)
	{
		GetBitsMac( CWExtra, (DWORD) TheTable->CWExtra[Temp] );
# ifdef TRACE
    if (TheTable->CWExtra[Temp])
    {
      printf("[%x",*CWExtra);
      printf("(%d)]\n", TheTable->CWExtra[Temp]);
    }
    else
    {
      printf("\n");
    }
# endif
	}
	else
	{
		GetBitsMac( &Temp, (DWORD) TheTable->CWExtra[Temp] );
	}

	return(Code);
}
static int FASTCALL PeekBits( ExaPatchApplyState * TheState, DWORD * TheBits, DWORD NumBits )
{
	DWORD Mask;
  int Code;

	if (0U != NumBits)
	{
		Mask=(1UL<<NumBits)-1UL;
		if (NumBits==32U)
		{
			Mask = 0xffffffffU;
		}
		if (NumBits <= TheState->BitsInBarrel )
		{
			*TheBits = Mask & (TheState->Barrel >> (TheState->BitsInBarrel - NumBits));
		}
		else if (NumBits <= (24U+(7U & TheState->BitsInBarrel)))
		{
			FillBarrelMac;
			if (0 != Code)
			{
				return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			}
			if (NumBits <= TheState->BitsInBarrel)
			{
				*TheBits = Mask & (TheState->Barrel >> (TheState->BitsInBarrel - NumBits));
			}
			else
			{
				return(EXAPATCH_UNEXPECTED_EOF); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
			}
		}
		else
		{
      return (EXAPATCH_UNEXPECTED_EOF); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
		}
	}
	else
	{
		*TheBits = 0;
	}
	return(EXAPATCH_SUCCESS);
}

static int FASTCALL GetBits( ExaPatchApplyState * TheState, DWORD * TheBits, DWORD NumBits )
{
	DWORD Mask;
	DWORD Temp;
	DWORD BitsLeft;
	int Code;

	if (0U != NumBits)
	{
		Mask=(1UL<<NumBits)-1UL;
		if (NumBits==32U)
		{
			Mask = 0xffffffffU;
		}
		if (NumBits <= TheState->BitsInBarrel )
		{
			*TheBits = Mask & (TheState->Barrel >> (TheState->BitsInBarrel - NumBits));
			TheState->BitsInBarrel -= NumBits;
			if (TheState->BitsInBarrel <= 16U)
			{
				FillBarrelMac;
				if (0 != Code)
				{
					return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
				}
			}
		}
		else if (NumBits <= (24U+(7U & TheState->BitsInBarrel)))
		{
			FillBarrelMac;
			if (0 != Code)
			{
			 return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
			}
			if (NumBits <= TheState->BitsInBarrel)
			{
				*TheBits = Mask & (TheState->Barrel >> (TheState->BitsInBarrel - NumBits));
				TheState->BitsInBarrel -= NumBits;
				if (TheState->BitsInBarrel <= 16U)
				{
					FillBarrelMac;
					if (0 != Code)
					{
					 return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
					}
				}
			}
			else
			{
				return(EXAPATCH_UNEXPECTED_EOF); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
			}
		}
		else
		{
			BitsLeft = NumBits - TheState->BitsInBarrel;
			Temp = (TheState->Barrel <<  BitsLeft);
			TheState->BitsInBarrel = 0;
			TheState->Barrel = 0;
			FillBarrelMac;
			if (0 != Code)
			{
			 return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
			}
			if (TheState->BitsInBarrel < BitsLeft)
			{
				return(EXAPATCH_UNEXPECTED_EOF); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
			}
			Temp |= TheState->Barrel >> (TheState->BitsInBarrel - BitsLeft);
			*TheBits = Mask & Temp;
			TheState->BitsInBarrel -= BitsLeft;
			if (TheState->BitsInBarrel <= 16U)
			{
				FillBarrelMac;
				if (0 != Code)
				{
				 return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
				}
			}
		}
	}
	else
	{
		*TheBits = 0;
	}
	return(EXAPATCH_SUCCESS);
}
static int FreeTable( ExaPatchApplyTable * TheTable )
{
	if ((NULL != TheTable->CWExtra) && (EXAPATCH_CHARTABLE == (TheTable->TableType & EXAPATCH_TABLE_TYPE)))
	{
		(void)ExaMemFree( NULL, (void *)TheTable->CWExtra );
		TheTable->CWExtra = NULL;
	}
	if (0U == (TheTable->TableType & EXAPATCH_FLAT))
	{
		if (NULL != TheTable->BrkPt)
		{
			(void)ExaMemFree( NULL, TheTable->BrkPt );
			TheTable->BrkPt = NULL;
		}
		if (NULL != TheTable->BrkCW)
		{
			(void)ExaMemFree( NULL, TheTable->BrkCW );
			TheTable->BrkCW = NULL;
		}
		if (NULL != TheTable->CW)
		{
			(void)ExaMemFree( NULL, TheTable->CW );
			TheTable->CW = NULL;
		}
	}
	memset( TheTable, 0, sizeof( ExaPatchApplyTable ) );
	return(EXAPATCH_SUCCESS);
}

static int ReadTable( ExaPatchApplyState * TheState, ExaPatchApplyTable * TheTable, DWORD TableType )
{
	int Code = EXAPATCH_SUCCESS;
	DWORD * SymArray = NULL;
	DWORD * LenArray = NULL;
	DWORD i;
	DWORD SymSize;
	DWORD EOT;
	DWORD TabSize;
	ExaPatchApplyTable TempTable;
	DWORD Sum;
	DWORD TempBrk[11];
	DWORD TempBrkCW[12];
	DWORD TempCWLen[26];
	DWORD TempCW[26];
	DWORD TempCWSym[12];
	unsigned char TempCWExtra[26];
	DWORD PrevSym;
	DWORD ThisSym;
	DWORD ExtraBits;
	DWORD NoEOTFlag;
	DWORD MaxLenLen=0;

# ifdef TRACE
  printf( "ReadTable: %x\n", TableType );
# endif
# ifdef DUMP_ONLY
  Code = ExaPatchDump( TheState, -1, L"Init Table, TYPE = 0x%x(", TableType );
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	switch ( TableType & 0x3 )
	{
		case EXAPATCH_CHARTABLE:
			Code = ExaPatchDump( TheState, 0, L"Char)\r\n" );
			break;
		case EXAPATCH_DISPTABLE:
			Code = ExaPatchDump( TheState, 0, L"Disp)\r\n" );
			break;
		case EXAPATCH_LENTABLE:
			Code = ExaPatchDump( TheState, 0, L"Len)\r\n" );
			break;
		default:
			Code = ExaPatchDump( TheState, 0, L"Unknown)\r\n" );
			break;
	}
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# endif
	TheTable->TableType = TableType;
	TheTable->BrkPt = NULL;
	TheTable->BrkCW = NULL;
	TheTable->CW = NULL;
	TheTable->CWExtra = NULL;
	/* first, handle the MinLength/MaxLength/Brk/CW members */
	if (0U != (TableType & EXAPATCH_FLAT))
	{
# ifdef DUMP_ONLY
		Code = ExaPatchDump( TheState, -2, L"Flat Table\r\n" );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# endif
		TheTable->CW = NULL;
		if (EXAPATCH_CHARTABLE == (TableType & EXAPATCH_TABLE_TYPE))
		{
			/* build a flat char table */
			TheTable->MinLength = 8;
			TheTable->MaxLength = 9;
			TheTable->BrkPt = &FlatCharBrk;
			TheTable->BrkCW = FlatCharBrkCW;
			TheTable->HintLength = 8;
			TheTable->NumLevels = 1;
			TheTable->StartLevel = 0;
		}
		else
		{
			/* build a flat len/disp table */
			TheTable->MinLength = 5;
			TheTable->MaxLength = 6;
			TheTable->BrkPt = &FlatDispLenBrk;
			TheTable->BrkCW = FlatDispLenBrkCW;
			TheTable->HintLength = 5;
			TheTable->NumLevels = 1;
			TheTable->StartLevel = 0;

		}
	}
	else if ((EXAPATCH_ENCODING_00 == (TableType & EXAPATCH_ENCODING_BITS))
			||(EXAPATCH_ENCODING_10 == (TableType & EXAPATCH_ENCODING_BITS)))
	{
		/* non-sparse encoding */
		/* */
		/* Coding: MinCWLength (1-8:3 bits) */
		/*	length of sym representing MinCWLength (3/4) [0 -> not present] */
		/*	length of sym representing MinCWLength+1 (3/4) */
		/*	... */
		/*	length of sym representing MaxCWLength (3/4) */
		/*	end of table mark (3/4 bits - all 1's) */
		/*  length of sym representing Not Present (3/4) */
		/*  length of sym representing RLC Escape (3/4) */
		/*  symbols for lengths of each symbol in table  */
		/*		(4 extra bits after each RLC escape) */

# ifdef DUMP_ONLY
		Code = ExaPatchDump( TheState, -2, L"Dense Encoding\r\n" );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# endif
		memset( TempBrk, 0, 11*4 );
		memset( TempBrkCW, 0, 12*4 );
		memset( TempCWLen, 0, 26*4 );
		memset( TempCWExtra,0,26 );
		memset( TempCWSym, 0, 12*4 );
		TempCWExtra[25] = 4;

		if (EXAPATCH_CHARTABLE == (TableType & EXAPATCH_TABLE_TYPE))
		{
			SymSize = 4;
			EOT = 0xf;
			TabSize = CHAR_TABLE_SIZE;
		}
		else
		{
			SymSize = 3;
			EOT = 7;
			TabSize = DISPLEN_TABLE_SIZE;
		}
		if (EXAPATCH_ENCODING_10 == (TableType & EXAPATCH_ENCODING_BITS))
		{
			SymSize = 4;
      EOT = 0xf;
		}
		Code = GetBits( TheState, &TheTable->MinLength, 3 );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		if (TheTable->MinLength == 0U)
		{
			TheTable->MinLength = 8U;
		}
		TempTable.MinLength = 11U;
		TempTable.MaxLength = 0U;
		for (i=TheTable->MinLength; i<=24U ; i++ )
		{
			Code = GetBits( TheState, &TempCWLen[i], SymSize );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			if (TempCWLen[i] == EOT)
			{
				TempCWLen[i] = 0;
				TheTable->MaxLength = i-1U;
				break; /* MISRA C 2012 [15.4]: break is algorithmically necessary, goto is necessary in error condition */
			}
			else
			{
        if (0U != TempCWLen[i])
        {
					if (MaxLenLen < TempCWLen[i])
					{
						MaxLenLen = TempCWLen[i];
					}
				  TempBrk[TempCWLen[i]]++;
				  if (TempCWLen[i] < TempTable.MinLength)
				  {
  					TempTable.MinLength = TempCWLen[i];
  				}
				  if (TempCWLen[i] > TempTable.MaxLength)
				  {
  					TempTable.MaxLength = TempCWLen[i];
  				}
        }
			}
		}
		if ((TheTable->MaxLength < TheTable->MinLength) || (MaxLenLen > 11U))
		{
		  Code = EXAPATCH_CORRUPT_PATCH_FILE;
		  goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		}
		Code = GetBits( TheState, &TempCWLen[24], SymSize );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = GetBits( TheState, &TempCWLen[25], SymSize );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		for (i=24U; i<26U ; i++ )
		{
			TempBrk[TempCWLen[i]]++;
			if (TempCWLen[i] < TempTable.MinLength)
			{
				TempTable.MinLength = TempCWLen[i];
			}
			if (TempCWLen[i] > TempTable.MaxLength)
			{
				TempTable.MaxLength = TempCWLen[i];
			}
		}
		TempBrk[TempTable.MinLength-1U] = 0;
		TempBrkCW[TempTable.MinLength] = 0;
		Sum = 0;
		for (i=TempTable.MinLength; i<= TempTable.MaxLength; i++ )
		{
			Sum += TempBrk[i];
			TempCWSym[i+1U] = Sum;
			TempBrkCW[i+1U] = (2U*TempBrkCW[i]) + Sum;
			TempBrk[i] = TempBrk[i-1U] + (TempBrk[i] << (32U-i));
		}
		for (i=1U; i<26U ; i++ )
		{
			if (0U != TempCWLen[i])
			{
				TempCW[TempCWSym[TempCWLen[i]]] = i;
				TempCWSym[TempCWLen[i]]++;
			}
		}
		TempTable.EscCW = 26U;
		TempTable.EscSize = 0U;
		TempTable.NumLevels = TempTable.MaxLength - TempTable.MinLength;
		TempTable.StartLevel = 0U;
		TempTable.HintLength = TempTable.MinLength;
		TempTable.BrkPt = &TempBrk[TempTable.MinLength];
		TempTable.BrkCW = &TempBrkCW[TempTable.MinLength];
		TempTable.CW = TempCW;
		TempTable.CWExtra = TempCWExtra;
		/* allocate arrays (incl. len/sym arrays) */
		Code = ExaMemAlloc( NULL, TabSize*4U, (void **)&LenArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, TabSize*4U, (void **) &TheTable->CW ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, (2U + TheTable->MaxLength - TheTable->MinLength)*4U, (void **) &SymArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, (1U + TheTable->MaxLength - TheTable->MinLength)*4U, (void **) &TheTable->BrkPt ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, (2U + TheTable->MaxLength - TheTable->MinLength)*4U, (void **) &TheTable->BrkCW ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		/* read symbol lengths and histogram them */
		memset( TheTable->BrkPt, 0, 4U*(1U + TheTable->MaxLength - TheTable->MinLength) );
		memset( TheTable->BrkCW, 0, 4U*(2U + TheTable->MaxLength - TheTable->MinLength) );
		memset( SymArray, 0, 4U*(2U + TheTable->MaxLength - TheTable->MinLength) );
		memset( LenArray, 0, TabSize*4U );
		memset( TheTable->CW, 0, TabSize*4U );
		PrevSym = 24U;
		NoEOTFlag = FALSE;
		for (i=0U; i<TabSize ; i++ )
		{
			Code = GetCW( TheState, &TempTable, &ThisSym, &ExtraBits );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			if (ThisSym == 25U)
			{
				/* RLC */
				if ((ExtraBits == 0U) && (NoEOTFlag == 0U))
				{
					/* Special Code for fill to EOT */
					if (PrevSym != 24U)
					{
						while (i < TabSize)
						{
							LenArray[i] = PrevSym;
							TheTable->BrkPt[PrevSym-TheTable->MinLength]++;
							i++; /* MISRA C 2012 [14.2]: algorithmically necessary to alter loop variable */
						}
					}
					break;
				}
				else
				{
					if (ExtraBits == 0xfU)
					{
						NoEOTFlag = TRUE;
					}
					ExtraBits++;
					while (0U != ExtraBits)
					{
						if (PrevSym != 24U)
						{
							LenArray[i] = PrevSym;
							TheTable->BrkPt[PrevSym-TheTable->MinLength]++;
						}
						i++; /* MISRA C 2012 [14.2]: algorithmically necessary to alter loop variable */
						ExtraBits--;
					}
          i--; /* MISRA C 2012 [14.2]: algorithmically necessary to alter loop variable */
				}
			}
			else if (ThisSym == 24U)
			{
				/* Not Present */
				PrevSym = ThisSym;
				NoEOTFlag = FALSE;
			}
			else
			{
				/* Actual Length */
				PrevSym = ThisSym;
				NoEOTFlag = FALSE;
				LenArray[i] = ThisSym;
				TheTable->BrkPt[ThisSym-TheTable->MinLength]++;
			}
		}
		/* convert histogram to Brk/BrkCW */
		Sum = TheTable->BrkPt[0];
		SymArray[1] = Sum;
		TheTable->BrkCW[0] = 0;
		TheTable->BrkCW[1] = Sum;
		TheTable->BrkPt[0] = TheTable->BrkPt[0] << (32U - TheTable->MinLength);
		for (i=1; i<= (TheTable->MaxLength - TheTable->MinLength) ; i++ )
		{
			Sum += TheTable->BrkPt[i];
			SymArray[i + 1U] = Sum;
			TheTable->BrkCW[i + 1U] = (2U*TheTable->BrkCW[i]) + Sum;
			TheTable->BrkPt[i] = TheTable->BrkPt[i - 1U] + (TheTable->BrkPt[i] << (32U - (i + TheTable->MinLength)));
		}
		/* build CW array */
		for (i=0U; i<TabSize ; i++ )
		{
			if (0U != LenArray[i])
			{
				TheTable->CW[SymArray[LenArray[i]-TheTable->MinLength]] = i;
				SymArray[LenArray[i]-TheTable->MinLength]++;
			}
		}
		/* free len/sym arrays */
		(void)ExaMemFree( NULL, LenArray );
		LenArray = NULL;
		(void)ExaMemFree( NULL, SymArray );
		SymArray = NULL;
	}
	else if (EXAPATCH_ENCODING_01 == (TableType & EXAPATCH_ENCODING_BITS))
	{
		/* sparse encoding */
		/*	MinCWLength (1-8:3 bits) */
		/*	sym1 (9/6) */
		/*	sym2 (9/6) */
		/*	... */
		/*	size advance (3/4) */
		/*	... */
		/* */
		/*  any symbol can be replaced by a RLC escape (3/4 + 3 count) indicating */
		/*		consecutive symbols in the same level */
		/*	EOT is calculated by the receiver "on-the-fly" when the code table is full */
		/* size advance and RLC escape are coded as */
		/* 	(1)110 and (1)111 respectively.  This is possible */
		/* 	because the maximum symbols are 0x114 and 51 respectively, */
		/*	which begin with 10 and 110 respectively */
		/* NOTE: the LONGER RLC/advance goes with the SHORTER codewords */
		/* */
    /* Special case - 000/RLC is single codeword table (ESC only)   */
		DWORD SymSize;
		DWORD AdvSize;
		DWORD RLC;
		DWORD Advance;
		DWORD TabSize;
		DWORD PrevBrk;
		DWORD CurLength;
		DWORD SpecFlg;
		DWORD CW1;
		DWORD CW2;
		DWORD CurCount;
		DWORD Sum;
		DWORD PrevCW=0;
# ifdef DUMP_ONLY
		Code = ExaPatchDump( TheState, -2, L"Sparse Encoding\r\n" );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# endif

		if (EXAPATCH_CHARTABLE == (TableType & EXAPATCH_TABLE_TYPE))
		{
			SymSize = 9U;
			AdvSize = 3U;
			Advance = 6U;
			RLC = 7U;
			TabSize = CHAR_TABLE_SIZE;
		}
		else
		{
			SymSize = 6U;
			AdvSize = 4U;
			Advance = 0xeU;
			RLC = 0xfU;
			TabSize = DISPLEN_TABLE_SIZE;
		}
		/* read MinLength */
		Code = GetBits( TheState, &TheTable->MinLength, 3U );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
    if (TheTable->MinLength == 0U)
    {
      Code = PeekBits( TheState, &CW1, AdvSize );
      if (CW1 == Advance)
      {
        Code = GetBits( TheState, &CW1, AdvSize );
        TheTable->MaxLength = 0U;
        goto tabledone; /* MISRA C 2012 [15.1]: goto used for special algorithmic purposes (coding efficiency) */
      }
      else
      {
        TheTable->MinLength = 8U;
      }
    }
		/* allocate arrays (incl. len/sym arrays) */
		Code = ExaMemAlloc( NULL, TabSize*4U, (void **) &LenArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, TabSize*4U, (void **) &TheTable->CW ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, (24U - TheTable->MinLength)*4U, (void **) &SymArray ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, (24U - TheTable->MinLength)*4U, (void **) &TheTable->BrkPt ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		Code = ExaMemAlloc( NULL, (24U - TheTable->MinLength)*4U, (void **) &TheTable->BrkCW ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		memset( TheTable->BrkPt, 0, 4U*(24U - TheTable->MinLength) );
		memset( TheTable->BrkCW, 0, 4U*(24U - TheTable->MinLength) );
		memset( SymArray, 0, 4U*(24U - TheTable->MinLength) );
		memset( LenArray, 0, TabSize*4U );
		memset( TheTable->CW, 0, TabSize*4U );
		/* read symbol lengths and histogram them */
		/* as we go, build the Brk/BrkCW array */
		PrevBrk = 0U;
		SpecFlg = TRUE;
		CurLength = TheTable->MinLength;
		CurCount = 0U;
		Sum = 0U;
		while ( (0U != (PrevBrk + (CurCount << (32U-CurLength)))) || (0U != SpecFlg) )
		{
			Code = GetBits( TheState, &CW1, AdvSize );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			if (CW1 == RLC)
			{
				Code = GetBits( TheState, &CW2, 3U );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				CW2++;
				while ( 0U != CW2-- )
				{
					PrevCW++;
					LenArray[PrevCW] = CurLength;
					CurCount++;
					Sum++;
				}

			}
			else if (CW1 == Advance)
			{
				PrevBrk += CurCount << (32U - CurLength);
				TheTable->BrkPt[CurLength - TheTable->MinLength]
  					= PrevBrk;
				SymArray[CurLength + 1U - TheTable->MinLength] = Sum;
				TheTable->BrkCW[CurLength + 1U - TheTable->MinLength] =
					(2U*TheTable->BrkCW[CurLength - TheTable->MinLength]) + Sum;
				CurLength++;
				CurCount = 0;
				SpecFlg = FALSE;
			}
			else
			{
				Code = GetBits( TheState, &CW2, (SymSize - AdvSize) );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				CW2 = CW2 | (CW1 << (SymSize - AdvSize) );
				PrevCW = CW2;
        if (CW2 >= TabSize)
        {
          Code = EXAPATCH_CORRUPT_PATCH_FILE;
          goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
        }
				LenArray[CW2] = CurLength;
				CurCount++;
				Sum++;
        SpecFlg = FALSE;
			}
		}
		SymArray[CurLength + 1U - TheTable->MinLength] = Sum;
		TheTable->BrkCW[CurLength + 1U - TheTable->MinLength] =
			(2U*TheTable->BrkCW[CurLength - TheTable->MinLength]) + Sum;
		TheTable->MaxLength = CurLength;
		/* build CW array */
		for (i=0U; i<TabSize ; i++ )
		{
			if (0U != LenArray[i])
			{
				TheTable->CW[SymArray[LenArray[i]- TheTable->MinLength]] = i;
				SymArray[LenArray[i]- TheTable->MinLength]++;
			}
		}
		/* free len/sym arrays */
		(void)ExaMemFree( NULL, LenArray );
		LenArray = NULL;
		(void)ExaMemFree( NULL, SymArray );
		SymArray = NULL;
	}
	else
	{
		/* Reserved table coding methods */
		/* not used currently */
		return(EXAPATCH_CORRUPT_PATCH_FILE); /* MISRA C 2012 [15.5]: return used for quick exit in error condition */
	}
tabledone:
	/* now, handle the CWExtra/Esc* members */
	if (EXAPATCH_CHARTABLE == (TableType & EXAPATCH_TABLE_TYPE))
	{
		// @@EB MALLOC MODIFY
#if 1
		unsigned char * TempCWExtra = NULL;
#else
		unsigned char * TempCWExtra;
#endif
		TheTable->EscCW = CHAR_TABLE_ESC;
		TheTable->EscSize = 9U;
		Code = ExaMemAlloc( NULL, CHAR_TABLE_SIZE, (void **) &TempCWExtra ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		memset( TempCWExtra, 0, CHAR_TABLE_SIZE );
		TempCWExtra[CHAR_BYTE_MOD] = 8U;
		TempCWExtra[CHAR_WORD_MOD] = 16U;
		TempCWExtra[CHAR_DWORD_MOD] = 32U;
		TempCWExtra[CHAR_BYTE_MOD_TABLE] = 4U;
		TempCWExtra[CHAR_WORD_MOD_TABLE] = 8U;
		TempCWExtra[CHAR_DWORD_MOD_TABLE] = 8U;
		TheTable->CWExtra = (const unsigned char *) TempCWExtra;
	}
	else if (EXAPATCH_DISPTABLE == (TableType & EXAPATCH_TABLE_TYPE))
	{
		TheTable->CWExtra = (const unsigned char *) DispExtraBits;
		TheTable->EscCW = DISPLEN_TABLE_ESC;
		TheTable->EscSize = 6U;
	}
	else
	{
		TheTable->CWExtra = (const unsigned char *) LenExtraBits;
		TheTable->EscCW = DISPLEN_TABLE_ESC;
		TheTable->EscSize = 6;
	}
  {
# if defined(TRACE) || defined( DUMP_ONLY )
    DWORD j;
# endif
    DWORD NumSym;
    for (i=TheTable->MinLength; i<=TheTable->MaxLength; i++)
    {
      if (i == TheTable->MinLength)
      {
        if (0U != i)
		{
        	NumSym = TheTable->BrkPt[0] >> (32U-i);
			if (NumSym == 0U)
			{
				NumSym = 0x80000000U >> (31U-i);
			}
		}
        else
        {
        	NumSym = 1;
        }
      }
      else
      {
    	  NumSym = (TheTable->BrkPt[i - TheTable->MinLength] - TheTable->BrkPt[i - (TheTable->MinLength + 1U)]) >> (32U-i);
      }

# ifdef TRACE
      if (0U == (TableType & EXAPATCH_FLAT) )
      {
        for (j=0U; j<NumSym; j++)
        {
          if (i == TheTable->MinLength)
          {
            if (0U != i)
            {
              printf("%x ",TheTable->CW[j] );
            }
            else
            {
              printf("%x ", TheTable->EscCW );
            }
          }
          else
          {
            printf("%x ",TheTable->CW[j+(TheTable->BrkPt[i-(TheTable->MinLength+1U)]>>(32U-i))-TheTable->BrkCW[i-TheTable->MinLength]]);
          }
        }
        printf("\n");
      }
# endif
# ifdef DUMP_ONLY
			if (0U == (TableType & EXAPATCH_FLAT) )
			{
				Code = ExaPatchDump( TheState, -2, L"Length %d: ", i );
				for (j=0; (j<NumSym) && (Code == 0) ; j++ )
				{
					if (i == TheTable->MinLength)
					{
            if (0U != i)
            {
  						Code = ExaPatchDump( TheState, 0, L"%x ", TheTable->CW[j] );
  					}
            else
              Code = ExaPatchDump( TheState, 0, L"%x", TheTable->EscCW );
					}
					else
					{
						Code = ExaPatchDump( TheState, 0, L"%x ",
							TheTable->CW[j+(TheTable->BrkPt[i-(TheTable->MinLength+1U)]>>(32U-i))-TheTable->BrkCW[i-TheTable->MinLength]]);
					}
				}
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				Code = ExaPatchDump( TheState, 0, L"\r\n" );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			}
# endif
    }
  }
  /* Calculate most-likely symbol length (HintLength) */
  if (0U == (TableType & EXAPATCH_FLAT))
  {
		QWORD Denom=0U, Numer=0U, Term;
		for (i=TheTable->MinLength; i<= TheTable->MaxLength ; i++ )
		{
			if (i==TheTable->MinLength)
			{
        if (0U != i)
        {
				  Term = (QWORD)(TheTable->BrkPt[0]);
				}
        else
        {
          Term = 0U;
        }
			}
			else
			{
				Term = ((QWORD)TheTable->BrkPt[i-TheTable->MinLength]) - ((QWORD)TheTable->BrkPt[(i -1U)- TheTable->MinLength]);
			}
			Denom += Term;
			Numer += Term*i;
		}
    if (0U != Denom)
    {
		  TheTable->HintLength = (DWORD) ((Numer+Denom-1U)/Denom);
    }
    else
    {
      TheTable->HintLength = TheTable->MinLength;
    }
  }
  TheTable->NumLevels = TheTable->MaxLength - TheTable->MinLength;
	TheTable->StartLevel = TheTable->HintLength - TheTable->MinLength;
exit:
	if (Code == 0)
	{
 		return(Code); /* MISRA C 2012 [15.5]: return used to bypass error processing */
 	}
	if (NULL != LenArray)
	{
		(void)ExaMemFree( NULL, LenArray );
	}
	if (NULL != SymArray)
	{
		(void)ExaMemFree( NULL, SymArray );
	}
	if (( NULL != TheTable->CWExtra) && (EXAPATCH_CHARTABLE == (TableType & EXAPATCH_TABLE_TYPE)))
	{
		(void)ExaMemFree( NULL, (void *)TheTable->CWExtra );
    TheTable->CWExtra = NULL;
	}
	if (0U == (TheTable->TableType & EXAPATCH_FLAT))
	{
		if (NULL != TheTable->BrkPt)
		{
			(void)ExaMemFree( NULL, TheTable->BrkPt );
      TheTable->BrkPt = NULL;
		}
		if (NULL != TheTable->BrkCW)
		{
			(void)ExaMemFree( NULL, TheTable->BrkCW );
      TheTable->BrkCW = NULL;
		}
		if (NULL != TheTable->CW)
		{
			(void)ExaMemFree( NULL, TheTable->CW );
      TheTable->CW = NULL;
		}
	}
	return(Code);
}

static int FASTCALL FillBarrel( ExaPatchApplyState * TheState )
{
	DWORD Count2=0;
	DWORD Count1;
	DWORD i;
	int Code;
	unsigned char *BarrelPtr = (unsigned char *) &TheState->Barrel;
# ifdef DO_INLINE_ASM
	unsigned char *SrcPtr = (unsigned char *) TheState->BufferPtr;
# endif

	Count1 = (32U-TheState->BitsInBarrel) >> 3;
	if (Count1 > TheState->BufSizeLeft)
  {
    Count2 = Count1 - TheState->BufSizeLeft;
		Count1 = TheState->BufSizeLeft;
  }
	/* Optimize this! it shouldn't have a loop! */
	/* get all available bytes from Buffer */

  if (0U != Count2)
  {
		if (0U != Count1)
		{
			for (i=0; i<4U ; i++ )
			{
				if (i < (4U-Count1) )
				{
	# ifdef PLAT_BIG_ENDIAN
					BarrelPtr[i] = BarrelPtr[Count1+i];
	# else
					BarrelPtr[3U-i] = BarrelPtr[3U-(Count1+i)];
	# endif
				}
				else
				{
	# ifdef PLAT_BIG_ENDIAN
					BarrelPtr[i] = TheState->BufferPtr[i+Count1-4U];
	# else
					BarrelPtr[3U-i] = TheState->BufferPtr[i+Count1-4U];
	# endif
				}
			}
			TheState->BitsInBarrel += (Count1 << 3);
			TheState->BufferPtr = &TheState->BufferPtr[Count1];
			TheState->BufSizeLeft -= Count1;
		}
		/* if we got here, then the buffer must be empty */
		/* refill it... */
		TheState->BufSizeLeft = TheState->BufSize;
		TheState->BufferPtr = TheState->Buffer;
		if (TheState->BufSizeLeft > TheState->PatchSizeLeft)
		{
			TheState->BufSizeLeft = (DWORD)TheState->PatchSizeLeft;
		}
		TheState->PatchSizeLeft -= TheState->BufSizeLeft;
		if (0U != TheState->BufSizeLeft)
		{
			Code = GetBytes( TheState->PatchFile, TheState->Buffer, TheState->BufSizeLeft );
			if (0 != Code)
			{
				return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
			}
		}
		/* get the rest of the bytes from the new buffer */
		if (Count2 > TheState->BufSizeLeft)
		{
			Count2 = TheState->BufSizeLeft;
		}
		if (0U != Count2)
		{
			for (i=0; i<4U ; i++ )
			{
				if (i < (4U-Count2) )
				{
# ifdef PLAT_BIG_ENDIAN
					BarrelPtr[i] = BarrelPtr[Count2+i];
# else
					BarrelPtr[3U-i] = BarrelPtr[3U-(Count2+i)];
# endif
				}
				else
				{
# ifdef PLAT_BIG_ENDIAN
					BarrelPtr[i] = TheState->BufferPtr[i+Count2-4U];
# else
					BarrelPtr[3U-i] = TheState->BufferPtr[i+Count2-4U];
# endif
				}
			}
			TheState->BitsInBarrel += (Count2 << 3);
			TheState->BufferPtr = &TheState->BufferPtr[Count2];
			TheState->BufSizeLeft -= Count2;
		}
  }
  else
  {
		switch (Count1)
		{
			default:
			case 0:
				/* No room in the barrel */
				break;

			case 1:
				/* 3 bytes old, one byte new */
# ifdef PLAT_BIG_ENDIAN
				BarrelPtr[0] = BarrelPtr[1];
				BarrelPtr[1] = BarrelPtr[2];
				BarrelPtr[2] = BarrelPtr[3];
				BarrelPtr[3] = TheState->BufferPtr[0];
# else
#  ifdef DO_INLINE_ASM
				_asm
				{
					mov edi,BarrelPtr
					mov esi,SrcPtr
					mov eax,[edi]
					shl eax,8
					mov al,[esi]
					mov [edi],eax
				}
#  else
				BarrelPtr[3] = BarrelPtr[2];
				BarrelPtr[2] = BarrelPtr[1];
				BarrelPtr[1] = BarrelPtr[0];
				BarrelPtr[0] = TheState->BufferPtr[0];
#  endif
# endif
				TheState->BitsInBarrel += 8U;
				TheState->BufferPtr++;
				TheState->BufSizeLeft--;
				break;

			case 2:
				/* 2 bytes new, 2 bytes old */
# ifdef PLAT_BIG_ENDIAN
				memcpy( BarrelPtr, BarrelPtr+2, 2);
				memcpy( BarrelPtr+2, TheState->BufferPtr, 2);
# else
#  ifdef DO_INLINE_ASM
				_asm
				{
					mov edi,BarrelPtr
					mov esi,SrcPtr
					mov eax,[edi]
					shl eax,16
					mov al,[esi+1]
					mov ah,[esi]
					mov [edi],eax
				}
#  else
				memcpy( BarrelPtr+2,BarrelPtr,2);
				BarrelPtr[1]=TheState->BufferPtr[0];
				BarrelPtr[0]=TheState->BufferPtr[1];
#  endif
# endif
				TheState->BitsInBarrel += 16U;
				TheState->BufferPtr = &TheState->BufferPtr[2];
				TheState->BufSizeLeft -= 2U;
				break;
			case 3:
				/* 3 bytes new, one byte old */
# ifdef PLAT_BIG_ENDIAN
				BarrelPtr[0]=BarrelPtr[3];
				memcpy( BarrelPtr+1,TheState->BufferPtr, 3);
# else
#  ifdef DO_INLINE_ASM
				_asm
				{
					mov edi,BarrelPtr
					mov esi,SrcPtr
					mov ax,[esi+1]
					shl eax,16
					mov ah,[esi]
					mov al,[edi]
					bswap eax
					mov [edi],eax
				}
#  else
				BarrelPtr[3]=BarrelPtr[0];
				BarrelPtr[2]=TheState->BufferPtr[0];
				BarrelPtr[1]=TheState->BufferPtr[1];
				BarrelPtr[0]=TheState->BufferPtr[2];
#  endif
# endif
				TheState->BitsInBarrel += 24U;
				TheState->BufferPtr = &TheState->BufferPtr[3];
				TheState->BufSizeLeft -= 3U;
				break;
			case 4:
				/* whole barrel is new */
# ifdef PLAT_BIG_ENDIAN
				memcpy( BarrelPtr, TheState->BufferPtr, 4);
# else
#  ifdef DO_INLINE_ASM
				_asm
				{
					mov esi,SrcPtr
					mov edi,BarrelPtr
					mov eax,[esi]
					bswap eax
					mov [edi],eax
				}
#  else
				BarrelPtr[0]=TheState->BufferPtr[3];
				BarrelPtr[1]=TheState->BufferPtr[2];
				BarrelPtr[2]=TheState->BufferPtr[1];
				BarrelPtr[3]=TheState->BufferPtr[0];
# endif
# endif
				TheState->BitsInBarrel = 32U;
				TheState->BufferPtr = &TheState->BufferPtr[4];
				TheState->BufSizeLeft -= 4U;
				break;
		}
  }

  return(EXAPATCH_SUCCESS);
}

/* routines to be called from exaawrap.c */
int ExaPatchApplyWorkInit( ExaPatchApplyState * TheState )
{
	int Code;
	/* must be set before call: */
	/* PatchFile, OldFile, OldFileSize, NewFile */
	/* PatchSize, CBPtr, CBHandle, LocalStart/Delta, GlobalStart/Delta */
	TheState->CharTable = NULL;
	TheState->DispTable = NULL;
	TheState->LenTable = NULL;
	TheState->Buffer = NULL;
	TheState->LocalBuffer = NULL;
	TheState->PropBuffer = NULL;
	TheState->LBUsed = 0U; TheState->PBUsed = 0U;
	Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyTable ), (void **) &TheState->CharTable ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyTable ), (void **) &TheState->DispTable ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyTable ), (void **) &TheState->LenTable ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	TheState->EOFEncountered = FALSE;
	TheState->PatchSizeLeft = TheState->PatchSize;
	Code = ExaMemAlloc(NULL, BUFFER_SIZE, (void **) &TheState->LocalBuffer); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# ifdef ATTOPATCH
	Code = ExaMemAlloc(NULL, BUFFER_SIZE + 4U, (void **) &TheState->PropBuffer); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# else
	Code = ExaMemAlloc(NULL, 0x100000U+4U, (void **) &TheState->PropBuffer); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# endif
	Code = ExaMemAlloc(NULL, BUFFER_SIZE, (void **) &TheState->Buffer); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	TheState->BufSize = BUFFER_SIZE;
	TheState->GlobalPrev = 0;
	TheState->LocalPrev = 0;
	TheState->BufSizeLeft =	0;
	TheState->BitsInBarrel = 0;
	TheState->Barrel = 0;
	TheState->NewFilePos = 0;
	TheState->NextMark = 0;
exit:
	if (Code == EXAPATCH_SUCCESS)
	{
		return(EXAPATCH_SUCCESS); /* MISRA C 2012 [15.5]: return used to bypass error processing  */
	}
	if (NULL != TheState->Buffer)
	{
		(void)ExaMemFree( NULL, TheState->Buffer );
		TheState->Buffer = NULL;
	}
	if (NULL != TheState->LocalBuffer)
	{
		(void)ExaMemFree( NULL, TheState->LocalBuffer );
		TheState->LocalBuffer = NULL;
	}
	if (NULL != TheState->PropBuffer)
	{
		(void)ExaMemFree( NULL, TheState->PropBuffer );
		TheState->PropBuffer = NULL;
	}
	if (NULL != TheState->CharTable)
	{
		(void)ExaMemFree( NULL, TheState->CharTable );
		TheState->CharTable = NULL;
	}
	if (NULL != TheState->DispTable)
	{
		(void)ExaMemFree( NULL, TheState->DispTable );
		TheState->DispTable = NULL;
	}
	if (NULL != TheState->LenTable)
	{
		(void)ExaMemFree( NULL, TheState->LenTable );
		TheState->LenTable = NULL;
	}
	return(Code);
}
# ifdef EXAPATCH_PASSWORD_SUPPORT
int ExaPatchApplyUsePW( ExaPatchApplyState * TheState, QWORD * Hash1, QWORD * Hash2 )
{
	int Code;

	if (TheState->PatchSizeLeft < 16)
	{
		return(EXAPATCH_SUCCESS);
	}
	TheState->BufferPtr = TheState->Buffer + (TheState->BufSize - 16);
	TheState->BufSizeLeft = 16;
	TheState->BitsInBarrel = TheState->Barrel = 0;
	Code = GetBytes( TheState->PatchFile, TheState->BufferPtr, 16 );
	if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	TheState->PatchSizeLeft -= 16;
	Code = UsePW( TheState->BufferPtr, Hash1, Hash2 );
exit:
	return(Code);
}
# endif
int ExaPatchApplyWorkRoutine( ExaPatchApplyState * TheState )
{
	int Code;

# ifdef DUMP_ONLY
#  ifdef _WIN32
  GetSystemTimeAsFileTime( &TheState->PrevTime );
#  endif
# endif

	while ( 0 == TheState->EOFEncountered )
	{
		Code = DoNextBlock( TheState );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	}
	Code = FinishProgress( TheState );
exit:
	if (NULL != TheState->Buffer)
	{
		(void)ExaMemFree( NULL, TheState->Buffer );
		TheState->Buffer = NULL;
	}
	if (NULL != TheState->LocalBuffer)
	{
		(void)ExaMemFree( NULL, TheState->LocalBuffer );
		TheState->LocalBuffer = NULL;
	}
	if (NULL != TheState->PropBuffer)
	{
		(void)ExaMemFree( NULL, TheState->PropBuffer );
		TheState->PropBuffer = NULL;
	}
	if (NULL != TheState->CharTable)
	{
		(void)ExaMemFree( NULL, TheState->CharTable );
		TheState->CharTable = NULL;
	}
	if (NULL != TheState->DispTable)
	{
		(void)ExaMemFree( NULL, TheState->DispTable );
		TheState->DispTable = NULL;
	}
	if (NULL != TheState->LenTable)
	{
		(void)ExaMemFree( NULL, TheState->LenTable );
		TheState->LenTable = NULL;
	}
	return(Code);
}

static QWORD NormalizeCopyPos( QWORD CopyPos, QWORD FileSize )
{
	QWORD RetPos = CopyPos;
	if (CopyPos >= FileSize)
	{
		RetPos = CopyPos - ((CopyPos + ((QWORD)0xffffffffU) - FileSize) & 0xffffffff00000000U);
	}
	return (RetPos );
}
# ifndef DUMP_ONLY
# ifndef ATTOPATCH
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
		DWORD SystemBase )
{
	unsigned char * BigBuf=NULL;
	HANDLE TheHandle=INVALID_HANDLE_VALUE;
	int Code = EXAPATCH_SUCCESS;
	ExaPatchStream TheStream;
	ExaPatchApplyDupListEntry * TheDup;
	ExaPatchFileAttrib RetrievedAttrib;
# ifdef EXAPATCH_CHECKSUM_SUPPORT
	ExaPatchFileChecksum RetrievedCkSum;
	QWORD SizeLeft;
	DWORD ThisSize;
	DWORD ThisSizeRead;
	QWORD ThisOffset;
  ExaPatchApplyIgnoreRegion * ThisIgnore;
	int ChecksumFlag;
# endif
	size_t TheLen;
# ifndef _WIN32
	int orKludge = FALSE;
	mode_t orKludgeMode = 0;
	char * orKludgeName = NULL;
# endif

# ifdef EXAPATCH_UPDATELINK_SUPPORT
	if (SearchFlags & EXP_SEARCH_PROCESSLINKS)
	{
		/*
		if we find a symbolic link, regard it as having zero size,
		and otherwise not checking timestamps/attributes/etc.
		*/
		if (EXAPATCH_SUCCESS == ExaIsSymlink(FullName))
		{
			if (TheAttrib && (SearchFlags & EXP_SEARCH_VERIFY_ATTRIB) && (TheAttrib->Flags & EXP_ATTRIB_SIZE) && TheAttrib->Size)
			{
				return(EXAPATCH_FILE_NOT_FOUND);
			}
			else
			{
				return(EXAPATCH_SUCCESS);
			}
		}
	}
# endif
  Code = ExaFileExists( FullName );
  if (Code) return( Code );
# ifdef EXAPATCH_LINK_SUPPORT
	if (SearchFlags & EXP_SEARCH_IGNORELINKS)
	{
		if (EXAPATCH_SUCCESS == ExaIsSymlink(FullName))
		{
			return(EXAPATCH_FILE_NOT_FOUND);
		}
	}
# endif
# ifdef EXAPATCH_SYSTEM_SUPPORT
	if ((SearchFlags & EXP_SEARCH_CONFIRMATION) && ProgressCallBack)
	{
		WCHAR * Ptrs[2];
		WCHAR * Buffer;
		WCHAR * DelimPtr;

		TheLen = PSwcslen( FullName );
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			Code = ExaMemAlloc( NULL, (DWORD)(sizeof(WCHAR)*(1+TheLen )), (void **) &Buffer); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			wcscpy_s( Buffer, TheLen+1, FullName );
			Ptrs[0] = SystemName;
			Ptrs[1] = Buffer;
			SystemBase++;
			while (SystemBase--)
			{
				DelimPtr = PSwcsrchr( Buffer, PATH_DELIMITER );
				if (DelimPtr)
				{
					*DelimPtr = L'\0';
				}
			}
			Code = (*ProgressCallBack)(EXP_PATCH_SYSTEM_CONFIRM, Ptrs, CallbackHandle);
			ExaMemFree( NULL, Buffer );
			if (Code == EXP_CALLBACK_ABORT)
			{
				Code = EXAPATCH_USER_CANCEL;
			}
			if (Code == EXP_CALLBACK_CONTINUE)
			{
				Code = EXAPATCH_FILE_NOT_FOUND;
			}
		}
		goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	}
# else
#  ifndef EXAPATCH_SILENT_UNSUPPORT
	if ((SearchFlags & EXP_SEARCH_CONFIRMATION) && ProgressCallBack)
	{
		Code = EXAPATCH_NOT_SUPPORTED;
		goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	}
#  endif
# endif
	if (DupList && *DupList && (SearchFlags & EXP_SEARCH_IGNORE_DUPS) )
	{
		for (TheDup = *DupList; TheDup ; TheDup = TheDup->NextEntry )
		{
			if (0 == ((SearchFlags & EXP_SEARCH_IGNORE_CASE)?PSFNwcscmp(FullName, TheDup->FileName):PSwcscmp(FullName, TheDup->FileName)))
			{
				Code = EXAPATCH_FILE_NOT_FOUND;
				goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			}
		}
	}
	if (SearchFlags & (EXP_SEARCH_VERIFY_ATTRIB | EXP_SEARCH_VERIFY_CKSUM | EXP_SEARCH_CHECK_OLD_DISC | EXP_SEARCH_CHECK_NEW_DISC))
	{
# ifndef _WIN32
		{
			DWORD BufSize;
			struct stat SS;

			BufSize = sizeof(WCHAR)*(PSwcslen(FullName) + 1);
			Code = ExaMemAlloc(NULL, BufSize, (void **) &orKludgeName); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			if (-1 != PSwcstombs(orKludgeName,FullName,BufSize))
			{
				if (0 == stat( orKludgeName, &SS ))
				{
					if (0 == (SS.st_mode & 0x100))
					{
						// no owner read permission - fix it
						orKludge = TRUE;
						orKludge = SS.st_mode;
						if (-1 == chmod( orKludgeName, orKludge | 0x100 ))
						{
							Code = EXAPATCH_FILE_NOT_FOUND;
						}
					}
				}
				else
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
				}
			}
			else
			{
				Code = EXAPATCH_CHARSET_ERROR;
			}
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		}
# endif
		TheHandle = ExaOpen( FullName, TRUE, FALSE );
		if (TheHandle == INVALID_HANDLE_VALUE)
		{
			Code = EXAPATCH_FILE_NOT_FOUND;
			goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		}
		memset( &TheStream, 0, sizeof( TheStream ) );
		TheStream.dwSize = sizeof( TheStream );
		Code = MakeExaStreamFromFileArray( &TheStream, 1, &TheHandle, NULL );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
	}
	if (TheAttrib && (EXP_SEARCH_VERIFY_ATTRIB & SearchFlags) )
	{
		RetrievedAttrib.Flags = TheAttrib->Flags;
		Code = GetExaStreamAttrib( &TheStream, &RetrievedAttrib );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
# ifndef _WIN32
		if (orKludge)
		{
			RetrievedAttrib.Attributes &= 0xffffff;
		}
# endif
		if (TheAttrib->Flags & EXP_ATTRIB_SIZE)
		{
			DWORD SearchFlags2 = SearchFlags & (EXP_SEARCH_ALLOWPAD | EXP_SEARCH_CHECK_OLD_DISC | EXP_SEARCH_CHECK_NEW_DISC);

			if ((SearchFlags2 == (EXP_SEARCH_ALLOWPAD | EXP_SEARCH_CHECK_OLD_DISC)) || (SearchFlags2 == (EXP_SEARCH_ALLOWPAD | EXP_SEARCH_CHECK_NEW_DISC)))
			{
				/* with ALLOWPAD, the file can be longer than TheAttrib->Size */
				if (TheAttrib->Size > RetrievedAttrib.Size)
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
			}
			else
			{
				if (TheAttrib->Size != RetrievedAttrib.Size)
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
			}
		}
		if (TheAttrib->Flags & EXP_ATTRIB_ATTRIBUTE)
		{
			if (EXP_ATTRIBUTE_CHECKED & (TheAttrib->Attributes ^ RetrievedAttrib.Attributes))
			{
				Code = EXAPATCH_FILE_NOT_FOUND;
				goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			}
		}
		//if (TheAttrib->Flags & EXP_ATTRIB_CHANGED_DATE)
		if (0)
		{
			if (EXP_SEARCH_IGNORE_TIMEZONE & SearchFlags)
			{
# ifdef _WIN32
				LARGE_INTEGER ll1, ll2;

				ll1.LowPart = TheAttrib->ChangedTime.dwLowDateTime;
				ll1.HighPart = TheAttrib->ChangedTime.dwHighDateTime;
				ll2.LowPart = RetrievedAttrib.ChangedTime.dwLowDateTime;
				ll2.HighPart = RetrievedAttrib.ChangedTime.dwHighDateTime;
				ll2.QuadPart -= ll1.QuadPart;
				if (ll2.QuadPart < 0)
					ll2.QuadPart = -ll2.QuadPart;
				/* File Not Found if difference is greater than a day OR */
				/*  not a multiple of an hour */
				if ((ll2.QuadPart > 24*60*60*1000) || (ll2.QuadPart % 60*60*1000 != 0) )
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
# else
				QWORD ll1, ll2;

# ifdef QWORD_IS_DWORD
				ll1 = TheAttrib->ChangedTime.dwLowDateTime;
				ll2 = RetrievedAttrib.ChangedTime.dwLowDateTime;
# else
				ll1 =  (((QWORD)TheAttrib->ChangedTime.dwHighDateTime) << 32)+ ((QWORD)TheAttrib->ChangedTime.dwLowDateTime);
				ll2 =  (((QWORD)RetrievedAttrib.ChangedTime.dwHighDateTime) << 32)+ ((QWORD)RetrievedAttrib.ChangedTime.dwLowDateTime);
# endif
				if (ll1 < ll2)
				{
					ll2 = ll2 - ll1;
				}
				else
				{
					ll2 = ll1 - ll2;
				}
				if ((ll2 > 24*60*60*1000) || (ll2 % 60*60*1000 != 0))
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
# endif
			}
			else
			{
				if (TheAttrib->ChangedTime.dwLowDateTime != RetrievedAttrib.ChangedTime.dwLowDateTime
					|| TheAttrib->ChangedTime.dwHighDateTime != RetrievedAttrib.ChangedTime.dwHighDateTime)
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
			}
		}
    /* Don't check this under Unix because it's not maintained - should we check Win32 either?
     it's likely to be different...
    */
    /*   (do set it correctly on the new files, though) */
# ifdef _WIN32
		if (TheAttrib->Flags & EXP_ATTRIB_CREATE_DATE)
		{
			if (EXP_SEARCH_IGNORE_TIMEZONE & SearchFlags)
			{
				LARGE_INTEGER ll1, ll2;

				ll1.LowPart = TheAttrib->CreateTime.dwLowDateTime;
				ll1.HighPart = TheAttrib->CreateTime.dwHighDateTime;
				ll2.LowPart = RetrievedAttrib.CreateTime.dwLowDateTime;
				ll2.HighPart = RetrievedAttrib.CreateTime.dwHighDateTime;
				ll2.QuadPart -= ll1.QuadPart;
				if (ll2.QuadPart < 0)
					ll2.QuadPart = -ll2.QuadPart;
				/* File Not Found if difference is greater than a day OR */
				/*  not a multiple of an hour */
				if ((ll2.QuadPart > 24*60*60*1000) || (ll2.QuadPart % 60*60*1000 != 0) )
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
			}
			else
			{
				if (TheAttrib->CreateTime.dwLowDateTime != RetrievedAttrib.CreateTime.dwLowDateTime
					|| TheAttrib->CreateTime.dwHighDateTime != RetrievedAttrib.CreateTime.dwHighDateTime)
				{
					Code = EXAPATCH_FILE_NOT_FOUND;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
			}
		}
# endif
	}
# ifdef EXAPATCH_CHECKSUM_SUPPORT
	if (TheCkSum && (EXP_SEARCH_VERIFY_CKSUM & SearchFlags) )
	{
		DWORD SearchFlags2 = SearchFlags & (EXP_SEARCH_ALLOWPAD | EXP_SEARCH_CHECK_OLD_DISC | EXP_SEARCH_CHECK_NEW_DISC);

		ChecksumFlag = TheCkSum->Cksum[0] & EXP_CHECKSUM_NEW;
		Code = ExaMemAlloc( NULL, BUFFER_SIZE, (void **) &BigBuf ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
    memset( &RetrievedCkSum, 0, 10 );
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		SizeLeft = TheStream.Size;
		if ((SearchFlags2 == (EXP_SEARCH_ALLOWPAD | EXP_SEARCH_CHECK_OLD_DISC)) || (SearchFlags2 == (EXP_SEARCH_ALLOWPAD | EXP_SEARCH_CHECK_NEW_DISC)))
		{
			/* with ALLOWPAD, just check the size specified in the TheAttrib */
			SizeLeft = TheAttrib->Size;
		}
		while (SizeLeft)
		{
			if (SizeLeft > BUFFER_SIZE)
			{
				ThisSize = BUFFER_SIZE;
			}
			else
			{
				ThisSize = (DWORD) SizeLeft;
			}
			Code = ReadExaStream( &TheStream, BigBuf, ThisSize, &ThisSizeRead );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			if (ThisSize != ThisSizeRead)
			{
				Code = EXAPATCH_UNEXPECTED_EOF;
				goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			}
			Code = ExaCSCheckSum( BigBuf, ThisSize, &RetrievedCkSum, ChecksumFlag );
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			SizeLeft -= (QWORD) ThisSize;
		}
		for (ThisIgnore = RegList; ThisIgnore ; ThisIgnore = ThisIgnore->Next )
		{
			ThisOffset = ThisIgnore->Offset;
			SizeLeft = ThisIgnore->Length;
			Code = SeekExaStream( &TheStream, ThisOffset, EXAPATCH_SEEK_BEGIN, NULL);
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			while (SizeLeft)
			{
				if (SizeLeft > BUFFER_SIZE)
				{
					ThisSize = BUFFER_SIZE;
				}
				else
				{
					ThisSize = (DWORD) SizeLeft;
				}
				Code = ReadExaStream( &TheStream, BigBuf, ThisSize, &ThisSizeRead );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				if (ThisSize != ThisSizeRead)
				{
					Code = EXAPATCH_UNEXPECTED_EOF;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
				Code = ExaCSUnCheckSum( BigBuf, ThisSize, ThisOffset, &RetrievedCkSum );
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				SizeLeft -= (QWORD) ThisSize;
				ThisOffset += (QWORD) ThisSize;
			}
		}
		if (ChecksumFlag)
		{
			RetrievedCkSum.Cksum[0] |= EXP_CHECKSUM_NEW;
		}
		if (memcmp(&RetrievedCkSum, TheCkSum, 10 ))
		{
			Code = EXAPATCH_FILE_NOT_FOUND;
			goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		}
	}
	if (TheDisc && (SearchFlags & (EXP_SEARCH_CHECK_OLD_DISC | EXP_SEARCH_CHECK_NEW_DISC)))
	{
		unsigned char Disc;

		Code = SeekExaStream( &TheStream, TheDisc->Pos, EXAPATCH_SEEK_BEGIN, NULL);
		if (EXAPATCH_SUCCESS == Code)
		{
			Code = ReadExaStream( &TheStream, &Disc, 1, &ThisSizeRead );
			if (EXAPATCH_SUCCESS == Code)
			{
				if (SearchFlags & EXP_SEARCH_CHECK_OLD_DISC)
				{
					if (Disc != TheDisc->OldByte)
					{
						Code = EXAPATCH_FILE_NOT_FOUND;
						goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
					}
				}
				else
				{
					if (Disc != TheDisc->NewByte)
					{
						Code = EXAPATCH_FILE_NOT_FOUND;
						goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
					}
				}
			}
		}
	}
# endif
exit:
# ifndef _WIN32
	if (orKludge)
	{
		// we had to elevate permissions to verify the file - set them back
		chmod( orKludgeName, orKludgeMode );
	}
	if (orKludgeName)
	{
		ExaMemFree( NULL, orKludgeName );
	}
# endif
	if (Code == EXAPATCH_SUCCESS && (SearchFlags & EXP_SEARCH_ADD_TO_DUP))
	{
		/* add it to the Dup List */
		Code = ExaMemAlloc( NULL, sizeof( ExaPatchApplyDupListEntry ), (void **) &TheDup ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXAPATCH_SUCCESS)
		{
			TheLen = PSwcslen( FullName );
			if (TheLen > 0x7ffffffe)
			{
				Code = EXAPATCH_OUT_OF_MEMORY;
			}
			else
			{
				Code = ExaMemAlloc( NULL, (DWORD)(sizeof(WCHAR)*(1+TheLen )), (void **) &TheDup->FileName ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			}
			if (Code == EXAPATCH_SUCCESS)
			{
				wcscpy_s( TheDup->FileName, TheLen+1, FullName );
				TheDup->NextEntry = *DupList;
				*DupList = TheDup;
			}
			else
			{
				ExaMemFree( NULL, TheDup );
			}
		}
	}

	if (TheHandle != INVALID_HANDLE_VALUE)
	{
		ExaClose( TheHandle );
	}
	if (BigBuf)
	{
		ExaMemFree( NULL, BigBuf );
		BigBuf = NULL;
	}
	return(Code);
}


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
	DWORD SystemBase )
{
	/* NOTES: *FullPathPtrPtr should be NULL on entry */
	/*	EXAPATCH_SUCCESS indicates correct file found */
	/*	EXAPATCH_FILE_NOT_FOUND indicates correct file not found */
	/*	any other code indicates a "genuine" error */
	/*	if correct file was not found, *FullPathPtrPtr is */
	/*		set to the first file of the correct name that was */
	/*		found (NULL if none) */
	/* Directory may be a PathSep-delimited list of directories */
	/*  directory levels are separated by PathDelim */
	/* Paths in the patch file use L'\\' (Windows convention) */
	/*  and should be "slashified" before passing them */
	/*  to this routine on Unix-style platforms */

	/*
	NOTE: we should probably make sure that this routine does
	*NOT* run into 	problems with circular mount points.  Most
	search routines will, so sysadmins usually avoid them, but
	it's really not *that* hard to fix - will require an additional
	parameter to this routine (which will be NULL when called from
	the outside)
	*/
	WCHAR * Ptr1,*Ptr2;
	WCHAR * NameBuf = NULL;
	char * NNameBuf = NULL;
	DWORD BufRem;
	int DirFlag;
	int Found;
	int Code = EXAPATCH_SUCCESS;
	int Code2;
	HANDLE SearchHandle;
	int FirstTime = TRUE;
	int SecondTime = FALSE;
	DWORD FirstSearchFlags = SearchFlags;
	DWORD SecondSearchFlags = SearchFlags;
	size_t TheLen;
# ifdef _WIN32
#  ifndef WINCE
	WIN32_FIND_DATAA FindDataA;
#  endif
	WIN32_FIND_DATAW FindDataW;
# else
	struct stat SS;
	DWORD Size;
	struct dirent * pDE = NULL;
	char * NNameBuf2 = NULL;
# endif

	if (PSwcschr(Directory, PathSep))
	{
# if defined(EXAPATCH_SYSTEM_SUPPORT) || defined(EXAPATCH_ADDWITHFILE_SUPPORT) || defined(EXAPATCH_SUBDIRSEARCH_SUPPORT)
		if (0 == (SearchFlags & EXP_SEARCH_FULL_RECURSE))
		{
			if (SearchFlags & EXP_SEARCH_SPECIAL_RECURSE)
			{
				/*
				Make sure that the first directory is searched non-recursively
				second is searched recursively
				others non-recursively
				*/
				FirstSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				SecondSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				SearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				SecondSearchFlags |= EXP_SEARCH_RECURSE;
			}
			else if (SearchFlags & EXP_SEARCH_RECURSE)
			{
				/*
				Make sure that the first directory is searched recursively
				others non-recursively
				*/
				FirstSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				SecondSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				SearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				FirstSearchFlags |= EXP_SEARCH_RECURSE;
			}
			else
			{
				/*
				No recursion
				*/
				FirstSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				SecondSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
				SearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
			}
		}
		else
		{
			SearchFlags |= EXP_SEARCH_RECURSE;
			FirstSearchFlags |= EXP_SEARCH_RECURSE;
			SecondSearchFlags |= EXP_SEARCH_RECURSE;
		}
# else
		FirstSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
		SecondSearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
		SearchFlags &= ~(EXP_SEARCH_RECURSE | EXP_SEARCH_SPECIAL_RECURSE);
# endif
		Ptr1 = Directory;
		while (Ptr2 = PSwcschr(Ptr1, PathSep))
		{
			*Ptr2 = L'\0';
			if (SearchFlags & EXP_SEARCH_CALLBACK && ProgressCallBack)
			{
				Code = (*ProgressCallBack)(EXP_PATCH_SEARCHING, NULL, CallbackHandle);
				if (Code)
				{
					return(EXAPATCH_USER_CANCEL);
				}
			}
			Code = ExaPatchFileSearch( FullPathPtrPtr,
					FileName,
					Ptr1,
					FirstTime?FirstSearchFlags:(SecondTime?SecondSearchFlags:SearchFlags),
					TheAttrib,
					TheCkSum,
					RegList,
					DupList,
					TheDisc,
					PathDelim,
					PathSep,
					ProgressCallBack,
					CallbackHandle,
					SystemName,
					SystemBase );
			*Ptr2 = PathSep;
			if (Code != EXAPATCH_FILE_NOT_FOUND)
			{
				return(Code);
			}
			Ptr1 = Ptr2 + 1;
			if (FirstTime)
			{
				FirstTime = FALSE;
				SecondTime = TRUE;
			}
			else
			{
				if (SecondTime)
				{
					SecondTime = FALSE;
				}
			}
		}
    /* special Last-time-through code */
		if (SearchFlags & EXP_SEARCH_CALLBACK && ProgressCallBack)
		{
			Code = (*ProgressCallBack)(EXP_PATCH_SEARCHING, NULL, CallbackHandle);
			if (Code)
			{
				return(EXAPATCH_USER_CANCEL);
			}
		}
		return ExaPatchFileSearch( FullPathPtrPtr,
				FileName,
				Ptr1,
				FirstTime?FirstSearchFlags:(SecondTime?SecondSearchFlags:SearchFlags),
				TheAttrib,
				TheCkSum,
				RegList,
				DupList,
				TheDisc,
				PathDelim,
				PathSep,
				ProgressCallBack,
				CallbackHandle,
				SystemName,
				SystemBase );
	}
	else
	{
		/* first search the given directory non-recursively */
		/* */
		if (SearchFlags & (EXP_SEARCH_SPECIAL_RECURSE | EXP_SEARCH_FULL_RECURSE))
		{
			SearchFlags |= EXP_SEARCH_RECURSE;
		}
		Code = ExaMemAlloc( NULL, sizeof(WCHAR)*MAX_PATH, (void **) &NameBuf ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		wcscpy_s( NameBuf, MAX_PATH, Directory );
		BufRem = (DWORD)(PSwcslen( NameBuf ));
		Ptr1 = NameBuf + BufRem;
		if (Ptr1[-1] != PathDelim)
		{
			*Ptr1++ = PathDelim;
		}
		BufRem = (DWORD)((NameBuf + MAX_PATH) - Ptr1);
		wcscpy_s( Ptr1, BufRem, FileName );

		Code = VerifyFile( NameBuf,
			SearchFlags,
			TheAttrib,
			TheCkSum,
			RegList,
			DupList,
			 TheDisc,
			ProgressCallBack,
			CallbackHandle,
			SystemName,
			SystemBase );
		if (Code == EXAPATCH_SUCCESS ||
			((NULL == *FullPathPtrPtr)&&(EXAPATCH_SUCCESS == ExaFileExists(NameBuf))))
		{
			if (*FullPathPtrPtr)
				ExaMemFree( NULL, *FullPathPtrPtr );
			TheLen = PSwcslen(NameBuf);
			if (TheLen > 0x7ffffffe)
			{
				Code2 = EXAPATCH_OUT_OF_MEMORY;
			}
			else
			{
				Code2 = ExaMemAlloc( NULL, (DWORD)(sizeof(WCHAR)*(1 + TheLen)), (void **) FullPathPtrPtr ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			}
			if (Code2)
			{
				Code = Code2;
				goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			}
			wcscpy_s( *FullPathPtrPtr, TheLen+1, NameBuf );
		}
		if (Code == EXAPATCH_SUCCESS)
		{
			goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		}
		if (Code != EXAPATCH_FILE_NOT_FOUND)
		{
			goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
		}
# if defined(EXAPATCH_SYSTEM_SUPPORT) || defined(EXAPATCH_ADDWITHFILE_SUPPORT) || defined(EXAPATCH_SUBDIRSEARCH_SUPPORT)
		if (EXP_SEARCH_RECURSE & SearchFlags)
		{
			if (SearchFlags & EXP_SEARCH_CALLBACK && ProgressCallBack)
			{
				Code = (*ProgressCallBack)(EXP_PATCH_SEARCHING, NULL, CallbackHandle);
				if (Code)
				{
					Code = EXAPATCH_USER_CANCEL;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
			}
#  ifdef _WIN32
			Ptr1[0] = L'*';
			Ptr1[1] = L'\0';
#   ifndef WINCE
			if (GetVersion() & 0x80000000U)
			{
				/* fake Unicode */
				Code = ExaMemAlloc( NULL, MAX_PATH, (void **) &NNameBuf ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				if (0 == WideCharToMultiByte( CP_ACP, 0, NameBuf, -1, NNameBuf, MAX_PATH, NULL, NULL) )
				{
					Code = EXAPATCH_CHARSET_ERROR;
					goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
				}
				SearchHandle = FindFirstFileA( NNameBuf, &FindDataA );
				if (SearchHandle != INVALID_HANDLE_VALUE)
				{
					if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, FindDataA.cFileName, -1, Ptr1, BufRem))
					{
						Code = EXAPATCH_CHARSET_ERROR;
						FindClose( SearchHandle );
						goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
					}
				}
			}
			else
#   endif /* WINCE */
			{
				/* real Unicode */
				SearchHandle = FindFirstFileW( NameBuf, &FindDataW );
				if (SearchHandle != INVALID_HANDLE_VALUE)
				{
					wcscpy_s( Ptr1, BufRem, FindDataW.cFileName );
				}
			}
			if (SearchHandle != INVALID_HANDLE_VALUE)
			{
				Found = TRUE;
				while (Found)
				{
					/* we have a file - see if it's a subdir */
					DirFlag = FALSE;
#   ifndef WINCE
					if (GetVersion() & 0x80000000U)
					{
						/* fake Unicode */
						if (strcmp(FindDataA.cFileName, ".")
							&& strcmp(FindDataA.cFileName, "..")
							&& (FindDataA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
						{
#    ifdef EXAPATCH_LINK_SUPPORT
							if ((0 == (SearchFlags & (EXP_SEARCH_IGNORELINKS | EXP_SEARCH_PROCESSLINKS)))
								|| (0 == (FindDataA.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
								|| ((FindDataA.dwReserved0 != IO_REPARSE_TAG_SYMLINK)
									&&(FindDataA.dwReserved0 != IO_REPARSE_TAG_MOUNT_POINT)))
#    endif
							{
								DirFlag = TRUE;
								if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, FindDataA.cFileName, -1, Ptr1, BufRem))
								{
									Code = EXAPATCH_CHARSET_ERROR;
									FindClose( SearchHandle );
									goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
								}
							}
						}
					}
					else
#   endif /* WINCE */
					{
						/* real Unicode */
						if (PSwcscmp(FindDataW.cFileName, L".")
							&& PSwcscmp(FindDataW.cFileName, L"..")
							&& (FindDataW.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
						{
#   ifdef EXAPATCH_LINK_SUPPORT
							if ((0 == (SearchFlags & (EXP_SEARCH_IGNORELINKS | EXP_SEARCH_PROCESSLINKS)))
								|| (0 == (FindDataW.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
								|| ((FindDataW.dwReserved0 != IO_REPARSE_TAG_SYMLINK)
									&&(FindDataW.dwReserved0 != IO_REPARSE_TAG_MOUNT_POINT)))
#   endif
							{
								DirFlag = TRUE;
								wcscpy_s( Ptr1, BufRem, FindDataW.cFileName );
							}
						}
					}
					if (DirFlag)
					{
						/* it's a subdir - search it */
						Code = ExaPatchFileSearch( FullPathPtrPtr,
							FileName,
							NameBuf,
							SearchFlags,
							TheAttrib,
							TheCkSum,
							RegList,
							DupList,
							TheDisc,
							PathDelim,
							PathSep,
							ProgressCallBack,
							CallbackHandle,
							SystemName,
							SystemBase );
						if (Code == EXAPATCH_SUCCESS)
						{
							FindClose( SearchHandle );
							goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
						}
						if (Code != EXAPATCH_FILE_NOT_FOUND)
						{
							FindClose( SearchHandle );
							goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
						}
					}
#   ifndef WINCE
					if (GetVersion() & 0x80000000U)
					{
						/* fake Unicode */
						if (0 == FindNextFileA( SearchHandle, &FindDataA) )
						{
							Found = FALSE;
						}
					}
					else
#   endif /* WINCE */
					{
						/* real Unicode */
						if (0 == FindNextFileW( SearchHandle, &FindDataW) )
						{
							Found = FALSE;
						}
					}
				}
				FindClose( SearchHandle );
			}
#  else
		  Ptr1[-1] = L'\0';
			Size = sizeof(WCHAR)*PSwcslen(NameBuf) + 1;
			Code = ExaMemAlloc( NULL, Size, (void **) &NNameBuf ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			if (0 != Code) {goto exit;} /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			if (-1 == PSwcstombs( NNameBuf, NameBuf, Size) )
			{
				Code = EXAPATCH_CHARSET_ERROR;
				goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
			}
			SearchHandle = (HANDLE) opendir(NNameBuf);
			if (SearchHandle)
			{
				pDE = readdir( (DIR *) SearchHandle );
			}
			if (SearchHandle && pDE)
			{
				int SkipFlag = FALSE;

				Ptr1[-1] = PathDelim;
				if (-1 == PSmbstowcs( Ptr1, pDE->d_name, BufRem))
				{
					SkipFlag = TRUE;
				}
				Found = TRUE;
				while (Found)
				{
					/* we have a file - see if it's a subdir */
					DirFlag = FALSE;
					if (!SkipFlag)
					{
						if (strcmp(pDE->d_name, ".")
							&& strcmp(pDE->d_name, "..") )
						{
							Size = sizeof(WCHAR)*PSwcslen( NameBuf ) + 1;
							Code = ExaMemAlloc( NULL, Size, (void **) &NNameBuf2 ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
							if (Code)
							{
								closedir( (DIR *) SearchHandle );
								goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
							}
							if (-1 == PSwcstombs( NNameBuf2, NameBuf, Size ))
							{
								Code = EXAPATCH_CHARSET_ERROR;
								closedir( (DIR *) SearchHandle );
								ExaMemFree( NULL, NNameBuf2 );
								goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
							}
							if ((0 == stat(NNameBuf2, &SS)) && (SS.st_mode & S_IFDIR))
							{
	#   ifdef EXAPATCH_LINK_SUPPORT
								if (SearchFlags & (EXP_SEARCH_IGNORELINKS | EXP_SEARCH_PROCESSLINKS))
									lstat(NNameBuf2, &SS);
								if ((0 == (SearchFlags & (EXP_SEARCH_IGNORELINKS | EXP_SEARCH_PROCESSLINKS)))
									|| (S_IFLNK != (SS.st_mode & S_IFMT)))
	#   endif
								{
									DirFlag = TRUE;
								}
							}
							ExaMemFree( NULL, NNameBuf2 );
						}
					}
					if (DirFlag)
					{
						/* it's a subdir - search it */
						Code = ExaPatchFileSearch( FullPathPtrPtr,
							FileName,
							NameBuf,
							SearchFlags,
							TheAttrib,
							TheCkSum,
							RegList,
							DupList,
              TheDisc,
							PathDelim,
							PathSep,
							ProgressCallBack,
							CallbackHandle,
							SystemName,
							SystemBase );
						if (Code == EXAPATCH_SUCCESS)
						{
							closedir( (DIR *) SearchHandle );
							goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
						}
						if (Code != EXAPATCH_FILE_NOT_FOUND)
						{
							closedir( (DIR *) SearchHandle );
							goto exit; /* MISRA C 2012 [15.1]: goto used for consistent exit processing */
						}
					}
					SkipFlag = FALSE;
					pDE = readdir( (DIR *) SearchHandle );
					if (pDE )
					{
						if (-1 == PSmbstowcs( Ptr1, pDE->d_name, BufRem))
						{
							SkipFlag = TRUE;
						}
					}
					else
					{
						Found = FALSE;
					}
				}
				closedir( (DIR *) SearchHandle );
			}
#  endif
		}
# endif
		Code = EXAPATCH_FILE_NOT_FOUND;
	}
exit:
	if (NameBuf)
	{
		ExaMemFree( NULL, NameBuf );
		NameBuf = NULL;
	}
	if (NNameBuf)
	{
		ExaMemFree( NULL, NNameBuf );
		NNameBuf = NULL;
	}
	return(Code);
}
# ifdef EXAPATCH_HOOK_SUPPORT
static int DoCmd( WCHAR * Cmd )
{
# ifdef _WIN32
	/*
	if first character is '&' then a direct (GUI) spawn is done
	if not, then the command is passed to a (minimized) shell
	if stdin is a valid handle, then the handles are passed on to
	the child process.

	in either case, a synchronous spawn is done.
	*/
	WCHAR CmdLine[260];
	int CheckRetVal = 0;
	DWORD RetCode;
# ifndef WINCE
	int ConsoleFlag;
	int AmpFlag;
	WCHAR ComSpec[260];
# else
  WCHAR AppName[ 260 ];
# endif

  if (*Cmd == L'?')
  {
	  CheckRetVal = TRUE;
	  Cmd++;
  }
# ifdef WINCE
  if (TRUE)
# else
	if ((*Cmd == L'&') || GetEnvironmentVariableW( L"COMSPEC", ComSpec, 260 ) )
# endif
	{
# ifndef WINCE
    STARTUPINFOW StartUp;
# endif
    PROCESS_INFORMATION Process;

# ifdef WINCE
    if (*Cmd == L'&')
    {
      wcscpy( CmdLine, Cmd+1 );
    }
    else
    {
      wcscpy( CmdLine, Cmd );
    }

/****************************************************************************
    StartUp.cb = sizeof( STARTUPINFO );
    StartUp.lpReserved = NULL;
    StartUp.lpDesktop = NULL;
    StartUp.lpTitle = NULL;
    StartUp.dwFlags = 0;
    StartUp.cbReserved2 = 0;
    StartUp.lpReserved2 = NULL;

		The name of the module to execute must be specified by the
		lpApplicationName parameter. Windows CE does not support passing
		NULL for lpApplicationName.

		The following parameters are not supported and require the following settings:

			lpProcessAttributes must be NULL
			lpThreadAttributes must be NULL
			bInheritHandles must be FALSE
			lpEnvironment must be NULL
			lpCurrentDirectory must be NULL
			lpStartupInfo must be NULL

    if (!(CreateProcessW( NULL, WCmdLine, NULL, NULL,
    	FALSE, 0, NULL, NULL, &StartUp, &Process) ) )
    {
      return(-1);
    }
***************************************************************************/

		/*
		   This assumes a command of the form:
			 "'application name with spaces' [param1] [param2] ..."
			 or
			 "application_with_no_spaces [param1] [param2] ..."
		*/

		/* First token is the Application Name, the rest is the command line */
		{
			WCHAR * Ptr;
			if (*CmdLine == L'\'')
      {
			  wcscpy(AppName, CmdLine + 1);
        Ptr = PSwcschr(AppName, L'\'');
        if (!Ptr)
        {
          Ptr = PSwcschr(AppName, L' ');
        }
      }
			else
      {
				wcscpy(AppName, CmdLine);
        Ptr = PSwcschr(AppName, L' ');
      }
      if (Ptr)
      {
        *Ptr = L'\0';
  			while (*++Ptr == L' ');
			  wcscpy(CmdLine, Ptr);
      }
      else
      {
        *CmdLine = L'\0';
      }
		}
		if (!(CreateProcessW(AppName, CmdLine, NULL, NULL,
												 FALSE, 0, NULL, NULL, NULL, &Process)))
		{
			return (EXAPATCH_SPAWN_FAILED);
		}

# else /* WINCE */

	if ( GetStdHandle( STD_INPUT_HANDLE ) != INVALID_HANDLE_VALUE)
	{
		ConsoleFlag = TRUE;
	}
	else
	{
		ConsoleFlag = FALSE;
	}
	if (*Cmd == L'&')
	{
		AmpFlag = TRUE;
		wcscpy_s( CmdLine, 260, Cmd+1 );
	}
	else
	{
		AmpFlag = FALSE;
		wcscpy_s( CmdLine, 260, ComSpec );
		wcscat_s( CmdLine, 260, L" /C " );
		wcscat_s( CmdLine, 260, Cmd );
	}
	StartUp.cb = sizeof( STARTUPINFOW );
	StartUp.lpReserved = NULL;
	StartUp.lpDesktop = NULL;
    StartUp.lpTitle = NULL;
    StartUp.dwFlags = STARTF_USESHOWWINDOW;
    StartUp.wShowWindow = AmpFlag ? SW_SHOWNORMAL : SW_MINIMIZE;
    StartUp.cbReserved2 = 0;
    StartUp.lpReserved2 = NULL;
    if (!(CreateProcessW( AmpFlag ? NULL : ComSpec, CmdLine, NULL, NULL,
    	ConsoleFlag, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartUp, &Process) ) )
    {
      return( EXAPATCH_SPAWN_FAILED );
    }

    CloseHandle( Process.hThread );
    WaitForSingleObject( Process.hProcess, INFINITE );
	GetExitCodeProcess(Process.hProcess, &RetCode);
    CloseHandle( Process.hProcess );
	return(CheckRetVal?(RetCode?EXAPATCH_HOOK_ABORT:EXAPATCH_SUCCESS):EXAPATCH_SUCCESS);
# endif /* WINCE */
	}
	return(EXAPATCH_SPAWN_FAILED);
# else
	/* just use 'system' for now since it's
	unclear what would be best (hook support in non-Win32 is new) */
	char * NBuf;
	int Code = EXAPATCH_SUCCESS;
	DWORD Size;
	int CheckRetVal = 0;
	int RetVal = 0;
	if (*Cmd == L'?')
	{
		CheckRetVal = TRUE;
		Cmd++;
	}

	Size = sizeof(WCHAR)*PSwcslen( Cmd ) + 1;
	NBuf = malloc( Size );
	if (NBuf)
	{
		if (-1 != PSwcstombs( NBuf, Cmd, Size) )
		{
			if (-1 == (RetVal = system( NBuf ) ))
			{
				Code = EXAPATCH_SPAWN_FAILED;
			}
		}
		else
		{
			Code = EXAPATCH_CHARSET_ERROR;
		}
		free( NBuf );
	}
	else
	{
		Code = EXAPATCH_OUT_OF_MEMORY;
	}
	if (CheckRetVal && RetVal && (Code == EXAPATCH_SUCCESS))
	{
		Code = EXAPATCH_HOOK_ABORT;
	}
	return(Code);
# endif
}

int ExaPatchDoHook( WCHAR * Hook,
	WCHAR * FileName,
	WCHAR * UpdDir )
{
	WCHAR CmdBuffer[2048];
	WCHAR * CmdIn;
	WCHAR * CmdOut;
	size_t TheLen=2048;
# ifdef _WIN32
# ifndef WINCE
	WCHAR OldDir[260];
# endif
# else
	char * OldDir = NULL;
# endif
	int Code = EXAPATCH_SUCCESS;

	if (Hook == NULL || *Hook == L'\0')
	{
		return(EXAPATCH_SUCCESS);
	}

	if (FileName && *FileName)
	{
# ifdef _WIN32
# ifndef WINCE
		SetEnvironmentVariableW( L"CURPATCH", FileName );
# endif
# else
		char * NBuf;
		DWORD Size;

		Size = sizeof(WCHAR)*PSwcslen( FileName ) + 1;
		NBuf = malloc( Size );
		if (NBuf)
		{
			if (-1 != PSwcstombs(NBuf, FileName, Size) )
			{
				/* POSIX, not strict ANSI */
				setenv( "CURPATCH", NBuf, TRUE );
			}
			else
			{
				Code = EXAPATCH_CHARSET_ERROR;
			}
			free( NBuf );
		}
		else
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
# endif
	}

	for (CmdIn = Hook, CmdOut = CmdBuffer; *CmdIn ; CmdIn++ )
	{
		if (*CmdIn == L'$')
		{
			if (CmdIn[1] == L'$')
			{
				CmdIn++;
				*CmdOut++ = L'$';
				TheLen--;
			}
			else
			{
				wcscpy_s( CmdOut, TheLen, FileName );
				CmdOut += PSwcslen( FileName );
				TheLen -= PSwcslen( FileName );
			}
		}
		else
		{
			*CmdOut++ = *CmdIn;
			TheLen--;
		}
	}

	*CmdOut = L'\0';
	if (UpdDir && *UpdDir)
	{
# ifdef _WIN32
# ifndef WINCE
		GetCurrentDirectoryW( 260, OldDir );
		SetCurrentDirectoryW( UpdDir );
# endif
# else
	  char * NBuf;
		DWORD Size;
		size_t ODSize = 200;
		char * ODBuf = NULL;

		/* NOTE: uses extension of POSIX getcwd spec */
		Code = ExaMemAlloc( NULL, ODSize, (void **) &ODBuf ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		while (!Code && ODBuf != OldDir)
		{
			OldDir = getcwd( ODBuf ,ODSize );
			if (NULL == OldDir)
			{
				ExaMemFree( NULL, ODBuf );
				if (errno == ERANGE)
				{
					ODSize *= 2;
					Code = ExaMemAlloc( NULL, ODSize, (void **) &ODBuf ); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				}
				else
				{
					Code = EXAPATCH_UNSPECIFIED_ERROR;
				}
			}
		}
		if (!Code)
		{
			Size = sizeof(WCHAR)*PSwcslen( UpdDir ) + 1;
			NBuf = malloc( Size );
			if (NBuf)
			{
				if (-1 != PSwcstombs(NBuf, UpdDir, Size))
				{
					if (chdir( NBuf ))
					{
						Code = 	EXAPATCH_SPAWN_FAILED;
					}
				}
				else
				{
					Code = EXAPATCH_CHARSET_ERROR;
				}
				free( NBuf );
			}
			else
			{
				Code = EXAPATCH_OUT_OF_MEMORY;
			}
		}
# endif
	}
	if (Code == EXAPATCH_SUCCESS)
	{
		if (CmdBuffer[0] == L'|'
			|| CmdBuffer[0] == L'<'
			|| CmdBuffer[0] == L'>'
			|| CmdBuffer[0] == L':'	)
		{
			WCHAR CmdSep;
			WCHAR * ThisCmd;
			WCHAR * NextCmd;

			CmdSep = CmdBuffer[0];
			ThisCmd = CmdBuffer + 1;
			while ((Code == EXAPATCH_SUCCESS) && (NextCmd = PSwcschr( ThisCmd, CmdSep) ) )
			{
				*NextCmd = L'\0';
				NextCmd++;
				if (PSwcslen( ThisCmd ) )
				{
					Code = DoCmd( ThisCmd );
				}
				ThisCmd = NextCmd;
			}
			if ((Code == EXAPATCH_SUCCESS) && PSwcslen( ThisCmd ) )
			{
				Code = DoCmd( ThisCmd );
			}
		}
		else
		{
			Code = DoCmd( CmdBuffer );
		}
	}
	/* reset environment and current directory */
# ifdef _WIN32
# ifndef WINCE
	if (UpdDir && *UpdDir)
	{
		SetCurrentDirectoryW( OldDir );
	}
	if (FileName && *FileName)
	{
		SetEnvironmentVariableW( L"CURPATCH", NULL );
	}
# endif
# else
	if (OldDir)
	{
		if (chdir( OldDir ))
		{
			Code = EXAPATCH_SPAWN_FAILED;
		}
		ExaMemFree( NULL, OldDir );
	}
	if (FileName && *FileName)
	{
		/* POSIX, not strict ANSI */
		unsetenv( "CURPATCH" );
	}
# endif
	return(Code);
}
# endif
# endif
# endif

static int DoProgress( ExaPatchApplyState * TheState )
{
	DWORD Progress[2];
	DWORD PartialProgress;
	int Code;

	if (TheState->PatchSize < 0x1000000000000U)
	{
// By Allen for Ingenico RTpatch speed 
//		PartialProgress = (DWORD)(((TheState->PatchSize - (TheState->PatchSizeLeft + TheState->BufSizeLeft)) << 16) / TheState->PatchSize);
		PartialProgress = (DWORD)(((TheState->PatchSize - (TheState->PatchSizeLeft + TheState->BufSizeLeft)) << 8) / TheState->PatchSize);
	}
	else
	{
// By Allen for Ingenico RTpatch speed 
//		PartialProgress = (DWORD)((TheState->PatchSize - (TheState->PatchSizeLeft + TheState->BufSizeLeft)) / (TheState->PatchSize >> 16));
		PartialProgress = (DWORD)((TheState->PatchSize - (TheState->PatchSizeLeft + TheState->BufSizeLeft)) / (TheState->PatchSize >> 8));
	}
	Progress[0] = ((TheState->GlobalDelta * PartialProgress) >> 16) + TheState->GlobalStart;
	Progress[1] = ((TheState->LocalDelta * PartialProgress) >> 16) + TheState->LocalStart;
	if ((NULL != TheState->CBPtr) && ((Progress[0] != TheState->GlobalPrev) || (Progress[1] != TheState->LocalPrev) ) )
	{
		TheState->GlobalPrev = Progress[0];
		TheState->LocalPrev = Progress[1];
		Code = (*TheState->CBPtr)(EXP_PATCH_PROGRESS, (void *) &Progress[0], TheState->CBHandle); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXP_CALLBACK_ABORT)
		{
			return(EXAPATCH_USER_CANCEL); /* MISRA C 2012 [15.5]: return used for quick exit in case of user cancel */
		}
	}
	return(EXAPATCH_SUCCESS);
}


static int FinishProgress( const ExaPatchApplyState * TheState )
{
	DWORD Progress[2];
  int Code;

	if (NULL != TheState->CBPtr)
	{
// By Allen for Ingenico RTpatch speed 
//		Progress[0] = TheState->GlobalStart + TheState->GlobalDelta;
//		Progress[1] = TheState->LocalStart + TheState->LocalDelta;
		Progress[0] = ((TheState->GlobalStart + TheState->GlobalDelta) >> 8);
		Progress[1] = ((TheState->LocalStart + TheState->LocalDelta) >> 8);
		Code = (*TheState->CBPtr)(EXP_PATCH_PROGRESS, (void *) &Progress[0], TheState->CBHandle); /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
		if (Code == EXP_CALLBACK_ABORT)
		{
			return(EXAPATCH_USER_CANCEL); /* MISRA C 2012 [15.5]: return used for quick exit in case of user cancel */
		}
	}
	return(EXAPATCH_SUCCESS);
}

# ifndef ATTOPATCH
static char PrefixArray[]="XKMGTPEZY";
# define NUM_PREFIXES 8
int ExaPrettify( QWORD NumBytes, DWORD * pNum, char * pPrefix, DWORD UpOrDown )
{
	DWORD Index;
	QWORD Pow1000;
	DWORD Done = FALSE;

	for (Index = 1, Pow1000 = 1000; (!Done) && (Index <= NUM_PREFIXES) ; Index++, Pow1000 *= 1000 )
	{
		if ((NumBytes/Pow1000) < 1000)
		{
			Done = TRUE;
			*pNum = (DWORD)((NumBytes + (UpOrDown?(Pow1000-1):0))/Pow1000);
			*pPrefix = PrefixArray[Index];
		}
	}
	return(EXAPATCH_SUCCESS);
}
int ExaEZTemp( WCHAR * pDirectory, WCHAR ** ppFileName )
{
	INT Code = EXAPATCH_SUCCESS;
	if (EXAPATCH_SUCCESS == (Code = ExaDirMerge(pDirectory, L"expaaaaa", ppFileName, PATH_DELIMITER)))
	{
		(*ppFileName)[PSwcslen(*ppFileName)-5] = L'\0';
		Code = ExaBuildTempName( *ppFileName, 4, PATH_DELIMITER );
		if (Code)
		{
			ExaMemFree( NULL, *ppFileName );
			*ppFileName = NULL;
		}
	}
	return(Code);
}
# endif

# ifndef ATTOPATCH
# ifndef _WIN32
/* Unix seems to lack these */
# ifndef __ANDROID__
# undef swprintf_s
static void swprintf_s( WCHAR *, size_t, WCHAR *, ...);
# endif
# undef vswprintf_s
static void vswprintf_s( WCHAR *, size_t, WCHAR *, va_list );
# ifndef __ANDROID__
#  undef swprintf1
#  define swprintf1 swprintf_s
# endif
# endif
void ExaPatchError(
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * Entry,
		DWORD ErrorNumber,
		... )
{
	va_list ArgList;
	PLATWORD Ptrs[3];
	WCHAR Buffer[2048] = WC_CONST("");
	WCHAR * Ptr;
	WCHAR * Ptr2;
	WCHAR ErrorID[20];
	DWORD Done = FALSE;
	DWORD ErrorStringNum = 0;
	size_t TheLen;

	swprintf1( ErrorID, 19, WC_CONST("err%04d"), ErrorNumber );
	Ptrs[0] = (PLATWORD) (&ErrorID[0]);
	Ptrs[1] = (PLATWORD) (&Buffer[0]);
	Ptrs[2] = (PLATWORD) ErrorNumber;
	va_start( ArgList, ErrorNumber);
	switch (ErrorNumber)
	{
		default:
		case EXAPATCH_SUCCESS:
			Done = TRUE;
			break;
		case EXAPATCH_UNSPECIFIED_ERROR:
			ErrorStringNum = EXP_APPLY_STRING_UNSPECIFIED_ERROR;
			break;
		case EXAPATCH_OUT_OF_MEMORY:
			ErrorStringNum = EXP_APPLY_STRING_OUT_OF_MEMORY;
			break;
		case EXAPATCH_READ_FAILED:
		case EXAPATCH_WRITE_FAILED:
		case EXAPATCH_DISK_FULL:
		case EXAPATCH_SEEK_FAILED:
		case EXAPATCH_OPEN_FAILED:
    case EXAPATCH_RENAME_FAILED:
		case EXAPATCH_SET_ATTRIB_FAILED:
		case EXAPATCH_GET_ATTRIB_FAILED:
		case EXAPATCH_ADS_ERROR:
		case EXAPATCH_EA_ERROR:
		case EXAPATCH_FILE_MANIPULATION_FAILED:
			Done = TRUE;
			Ptr = ExaGetString( EXP_APPLY_STRING_IO_FAILED );
			if (Ptr)
			{
				switch (ErrorNumber)
				{
          case EXAPATCH_RENAME_FAILED:
            ErrorStringNum = EXP_APPLY_STRING_RENAME;
            break;
					case EXAPATCH_READ_FAILED:
						ErrorStringNum = EXP_APPLY_STRING_READ;
						break;
					case EXAPATCH_WRITE_FAILED:
						ErrorStringNum = EXP_APPLY_STRING_WRITE;
						break;
					case EXAPATCH_DISK_FULL:
						ErrorStringNum = EXP_APPLY_STRING_DISK_FULL;
						break;
					case EXAPATCH_SEEK_FAILED:
						ErrorStringNum = EXP_APPLY_STRING_SEEK;
						break;
					case EXAPATCH_OPEN_FAILED:
						ErrorStringNum = EXP_APPLY_STRING_OPEN;
						break;
					case EXAPATCH_SET_ATTRIB_FAILED:
						ErrorStringNum = EXP_APPLY_STRING_SET_ATTRIB;
						break;
					case EXAPATCH_GET_ATTRIB_FAILED:
						ErrorStringNum = EXP_APPLY_STRING_GET_ATTRIB;
						break;
					case EXAPATCH_FILE_MANIPULATION_FAILED:
						ErrorStringNum = EXP_APPLY_STRING_FILE_MANIPULATION;
						break;
					case EXAPATCH_ADS_ERROR:
						ErrorStringNum = EXP_APPLY_STRING_ADS;
						break;
					case EXAPATCH_EA_ERROR:
						ErrorStringNum = EXP_APPLY_STRING_EA;
						break;

				}
				Ptr2 = ExaGetString( ErrorStringNum );
				if (Ptr2)
				{
					swprintf1( Buffer, 2047, Ptr, Ptr2 );
				}
			}
			break;
		case EXAPATCH_INVALID_PARAMETER:
			ErrorStringNum = EXP_APPLY_STRING_INVALID_PARAMETER;
			break;
		case EXAPATCH_INTERNAL_ERROR:
			ErrorStringNum = EXP_APPLY_STRING_INTERNAL_ERROR;
			break;
		case EXAPATCH_INVALID_HANDLE:
			ErrorStringNum = EXP_APPLY_STRING_INVALID_HANDLE;
			break;
		case EXAPATCH_BUSY:
			ErrorStringNum = EXP_APPLY_STRING_BUSY;
			break;
		case EXAPATCH_RESOURCES_UNAVAILABLE:
			ErrorStringNum = EXP_APPLY_STRING_RESOURCES_UNAVAILABLE;
			break;
		case EXAPATCH_INVALID_PASSWORD:
			ErrorStringNum = EXP_APPLY_STRING_INVALID_PASSWORD;
			break;
		case EXAPATCH_UNEXPECTED_EOF:
			ErrorStringNum = EXP_APPLY_STRING_UNEXPECTED_EOF;
			break;
		case EXAPATCH_USER_CANCEL:
			ErrorStringNum = EXP_APPLY_STRING_USER_CANCEL;
			break;
		case EXAPATCH_CHARSET_ERROR:
			ErrorStringNum = EXP_APPLY_STRING_CHARSET_ERROR;
			break;
		case EXAPATCH_INVALID_FILE_LIST:
			ErrorStringNum = EXP_APPLY_STRING_INVALID_FILE_LIST;
			break;
		case EXAPATCH_CORRUPT_PATCH_FILE:
			ErrorStringNum = EXP_APPLY_STRING_CORRUPT_PATCH_FILE;
			break;
		case EXAPATCH_INVALID_PATCH_FILE:
			ErrorStringNum = EXP_APPLY_STRING_INVALID_PATCH_FILE;
			break;
		case EXAPATCH_ENTRY_NOT_FOUND:
			ErrorStringNum = EXP_APPLY_STRING_ENTRY_NOT_FOUND;
			break;
		case EXAPATCH_ENCRYPTION_FAILURE:
			ErrorStringNum = EXP_APPLY_STRING_ENCRYPTION_FAILURE;
			break;
		case EXAPATCH_NOT_HANDLED_BY_USER:
			ErrorStringNum = EXP_APPLY_STRING_NOT_HANDLED_BY_USER;
			break;
		case EXAPATCH_FILE_NOT_FOUND:
			ErrorStringNum = EXP_APPLY_STRING_FILE_NOT_FOUND;
			break;
		case EXAPATCH_SPAWN_FAILED:
			ErrorStringNum = EXP_APPLY_STRING_SPAWN_FAILED;
			break;
		case EXAPATCH_CANT_BUILD_TEMPFILE:
			ErrorStringNum = EXP_APPLY_STRING_CANT_BUILD_TEMPFILE;
			break;
		case EXAPATCH_ALREADY_PATCHED:
			ErrorStringNum = EXP_APPLY_STRING_ALREADY_PATCHED;
			break;
		case EXAPATCH_INVALID_FILE_FOUND:
			ErrorStringNum = EXP_APPLY_STRING_INVALID_FILE_FOUND;
			break;
		case EXAPATCH_NOT_SUPPORTED:
			ErrorStringNum = EXP_APPLY_STRING_NOT_SUPPORTED;
			break;
		case EXAPATCH_DEVICE_MAPPING_FAILED:
			ErrorStringNum = EXP_APPLY_STRING_DEVICE_MAPPING_FAILED;
			break;
		case EXAPATCH_MANDATORY_EH_NOT_HANDLED:
			ErrorStringNum = EXP_APPLY_STRING_EH_NOT_HANDLED;
			break;
		case EXAPATCH_INSUFFICIENT_DISK_SPACE:
			Done = TRUE;
			Ptr = ExaGetString( EXP_APPLY_STRING_INSUFFICIENT_DISK_SPACE );
			if (Ptr)
			{
				if (HeaderPtr->ErrorBuffer)
				{
					swprintf1( Buffer, 2047, Ptr, HeaderPtr->ErrorBuffer );
					ExaMemFree( NULL, HeaderPtr->ErrorBuffer );
					HeaderPtr->ErrorBuffer = NULL;
				}
				else
				{
					swprintf1( Buffer, 2047, Ptr, "" );
				}
			}
			break;
		case EXAPATCH_UNABLE_TO_CREATE_DIRECTORY:
			ErrorStringNum = EXP_APPLY_STRING_UNABLE_TO_CREATE_DIRECTORY;
			break;
		case EXAPATCH_UNABLE_TO_REMOVE_DIRECTORY:
			ErrorStringNum = EXP_APPLY_STRING_UNABLE_TO_REMOVE_DIRECTORY;
			break;
		case EXAPATCH_POST_PATCH_VERIFY_FAILED:
			ErrorStringNum = EXP_APPLY_STRING_POST_PATCH_VERIFY_FAILED;
			break;
		case EXAPATCH_HOOK_ABORT:
			ErrorStringNum = EXP_APPLY_STRING_HOOK_ABORT;
			break;
	}
	if (!Done)
	{
		Ptr = ExaGetString( ErrorStringNum );
		if (Ptr)
		{
			wcscpy_s( Buffer, 2048, Ptr );
		}
	}
        if (Buffer[0] && ProgressCallBack && HeaderPtr && (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE))
	{
		(*ProgressCallBack)( EXP_PATCH_ERROR_CALLBACK, Ptrs, CallbackHandle );
	}
	if (HeaderPtr && HeaderPtr->ErrorFile)
	{
		DWORD BufferSize;
		char * NBuf;
		WCHAR EntryBuffer[2048] = WC_CONST("");
		if (Entry->Comm.lpEntryName)
		{
			Ptr = ExaGetString( EXP_APPLY_STRING_ENTRY_STR );
			if (Ptr)
			{
				wcscpy_s( EntryBuffer, 2048, Ptr );
				wcscat_s( EntryBuffer, 2048, Entry->Comm.lpEntryName );
			}
		}

		HeaderPtr->dwBW |= EXP_BW_ERRORFILE_USED;
		TheLen = (sizeof(WCHAR)*PSwcslen( Buffer )) + (sizeof(WCHAR)*PSwcslen( EntryBuffer )) + PSwcslen( ErrorID ) + 12;
		if (TheLen < 0x7ffffffe)
		{
			BufferSize = (DWORD) TheLen;
			if (0 == ExaMemAlloc( NULL, BufferSize, (void **) &NBuf)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			{
# ifdef _WIN32
				sprintf2( NBuf, BufferSize, "%ls\r\n%ls: %ls\r\n", EntryBuffer, ErrorID, Buffer );
# else
#  ifdef __ANDROID__
				{
					char * NBuf1;
					DWORD BufSize1;
					char * NBuf2;
					DWORD BufSize2;
					char * NBuf3;
					DWORD BufSize3;

					BufSize1 = PSwcslen(ErrorID)+1;
					BufSize2 = PSwcslen(Buffer)+1;
					BufSize3 = PSwcslen(EntryBuffer)+1;
					if (0 == ExaMemAlloc(NULL, BufSize1, (void **) &NBuf1)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					{
						if (0 == ExaMemAlloc(NULL, BufSize2, (void **) &NBuf2)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						{
							if (0 == ExaMemAlloc(NULL, BufSize3, (void **) &NBuf3)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
							{
								PSwcstombs( NBuf1, ErrorID, BufSize1 );
								PSwcstombs( NBuf2, Buffer, BufSize2 );
								PSwcstombs( NBuf3, EntryBuffer, BufSize3 );
								sprintf(NBuf, "%s\n%s: %s\n", NBuf3, NBuf1, NBuf2 );
								ExaMemFree( NULL, NBuf3 );
							}
							ExaMemFree( NULL, NBuf2 );
						}
						ExaMemFree( NULL, NBuf1 );
					}
				}
#  else
				sprintf( NBuf, "%ls\n%ls: %ls\n", EntryBuffer, ErrorID, Buffer );
#  endif
# endif
				WriteExaStream( HeaderPtr->ErrorFile, NBuf, (DWORD) strlen( NBuf ), &BufferSize );
				ExaMemFree( NULL, NBuf );
			}

		}
	}
	va_end( ArgList );
}
int ExaPatchWarning(
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		ExaPatchApplyFileHeaderInfo * HeaderPtr,
		ExaPatchApplyFileEntryInfo * Entry,
		DWORD WarningNumber,
		... )
{
	va_list ArgList;
	PLATWORD Ptrs[3];
	WCHAR Buffer[2048] = WC_CONST("");
	WCHAR * Ptr;
	WCHAR WarningID[20];
	int Code = EXAPATCH_SUCCESS;
	size_t TheLen;


	swprintf1( WarningID, 19, WC_CONST("wrn%04d"), WarningNumber );
	Ptrs[0] = (PLATWORD) (&WarningID[0]);
	Ptrs[1] = (PLATWORD) (&Buffer[0]);
	Ptrs[2] = (PLATWORD) WarningNumber;

	va_start( ArgList, WarningNumber);
	switch (WarningNumber)
	{
		case EXAPATCH_WARNING_FILE_NOT_FOUND:
			Ptr = ExaGetString( EXP_APPLY_STRING_WARNING_FILE_NOT_FOUND );
			if (Ptr)
				wcscpy_s( Buffer, 2048, Ptr );
			break;
		case EXAPATCH_WARNING_FILE_ALREADY_PATCHED:
			Ptr = ExaGetString( EXP_APPLY_STRING_WARNING_FILE_ALREADY_PATCHED );
			if (Ptr)
				wcscpy_s( Buffer, 2048, Ptr );
			break;
	}
	if ((L'\0' != Buffer[0])
		&& (NULL != ProgressCallBack)
		&& (NULL != HeaderPtr)
		&& (0U != (HeaderPtr->Comm.dwGlobalFlags & EXP_GLOBAL_MESSAGE))
		&& (0U == (Entry->Comm.dwFileFlags & EXP_FILE_NOWARNINGS)))
	{
		Code = (*ProgressCallBack)( EXP_PATCH_WARNING_CALLBACK, Ptrs, CallbackHandle );
	}
	if (HeaderPtr && HeaderPtr->ErrorFile && (HeaderPtr->ErrorFile->FileHandle != INVALID_HANDLE_VALUE))
	{
		DWORD BufferSize;
		char * NBuf;

		HeaderPtr->dwBW |= EXP_BW_ERRORFILE_USED;
		TheLen = (sizeof(WCHAR)*PSwcslen( Buffer )) + PSwcslen( WarningID ) + 10;
		if (TheLen < 0x7ffffffe)
		{
			BufferSize = (DWORD) TheLen;
			if (0 == ExaMemAlloc( NULL, BufferSize, (void **) &NBuf)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			{
# ifdef _WIN32
				sprintf2( NBuf, BufferSize, "%ls: %ls\r\n", WarningID, Buffer );
# else
#  ifdef __ANDROID__
				{
					char * NBuf1;
					DWORD BufSize1;
					char * NBuf2;
					DWORD BufSize2;

					BufSize1 = PSwcslen(WarningID)+1;
					BufSize2 = PSwcslen(Buffer)+1;
					if (0 == ExaMemAlloc(NULL, BufSize1, (void **) &NBuf1)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
					{
						if (0 == ExaMemAlloc(NULL, BufSize2, (void **) &NBuf2)) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
						{
							PSwcstombs( NBuf1, WarningID, BufSize1 );
							PSwcstombs( NBuf2, Buffer, BufSize2 );
							sprintf(NBuf, "%s: %s\n", NBuf1, NBuf2 );
							ExaMemFree( NULL, NBuf2 );
						}
						ExaMemFree( NULL, NBuf1 );
					}
				}
#  else
				sprintf( NBuf, "%ls: %ls\n", WarningID, Buffer );
#  endif
# endif
				WriteExaStream( HeaderPtr->ErrorFile, NBuf, (DWORD) strlen( NBuf ), &BufferSize );
				ExaMemFree( NULL, NBuf );
			}
		}
	}
	va_end( ArgList );
	return(Code);
}
int ExaPatchInfo(
		int (CALLBACK *ProgressCallBack)(DWORD ID,
			LPVOID Ptr, PLATWORD Handle),
		PLATWORD CallbackHandle,
		DWORD StringNumber,
		... )
{
	va_list ArgList;
	WCHAR Buffer[2048] = WC_CONST("");
	WCHAR * Ptr;
	int Code = EXP_CALLBACK_OK;

	va_start( ArgList, StringNumber);
	Ptr = ExaGetString( StringNumber );
	if (Ptr)
	{
		vswprintf_s( Buffer, 2048, Ptr, ArgList );
		if (Buffer[0] && ProgressCallBack)
		{
			Code = (*ProgressCallBack)( EXP_PATCH_INFO, Buffer, CallbackHandle );
		}
	}
	va_end( ArgList );
	return(Code);
}
# endif /* !ATTOPATCH */
# ifdef DUMP_ONLY
#  ifdef DUMP_MEANS_VERIFY
int ExaPatchDump( ExaPatchApplyState * StatePtr,
		int FmtFlag,
    WCHAR * FmtString,
    ... )
{
	return(EXP_CALLBACK_OK);
}
#  else
int ExaPatchDump( ExaPatchApplyState * StatePtr,
		int FmtFlag,
    WCHAR * FmtString,
    ... )
{
	va_list ArgList;
	WCHAR Buffer[2048]=L"";
	WCHAR Prefix[100]=L"";
	WCHAR * Ptr;
	char NBuffer[4096]="";
	int Code = EXP_CALLBACK_OK;
	DWORD BufferSize;
	int i;
	size_t TheLen;

	va_start( ArgList, FmtString );
	vswprintf_s( Buffer, 2048, FmtString, ArgList );
	if (FmtFlag > 0)
	{
		Ptr = Prefix;
		TheLen = 100;
		for (i=0; i<(FmtFlag-1) ; i++ )
		{
			wcscpy_s(Ptr, TheLen, L"  ");
			TheLen -= PSwcslen( Ptr );
			Ptr += PSwcslen( Ptr );
		}
# ifdef _WIN32
		swprintf1( Ptr, TheLen, L"[0x%I64x]: ", StatePtr->NewFilePos );
# else
		swprintf1( Ptr, TheLen, L"[0x%llx]: ", StatePtr->NewFilePos );
# endif
	}
	else if (FmtFlag < 0)
	{
		Ptr = Prefix;
		TheLen = 100;
		for (i=0; i<(-FmtFlag); i++ )
		{
			wcscpy_s(Ptr, TheLen, L"  ");
			TheLen -= PSwcslen( Ptr );
			Ptr += PSwcslen(Ptr);
		}
	}
	sprintf2( NBuffer, 4096,"%ls%ls", Prefix, Buffer );
	WriteExaStream( StatePtr->HeaderPtr->ErrorFile, NBuffer, (DWORD)strlen( NBuffer ), &BufferSize );
	va_end( ArgList );
	return(Code);
}
#  endif
# endif

# ifndef ATTOPATCH
# ifdef _WIN32
static DWORD StringsInitialized = FALSE;
static DWORD StringsInitInProgress = FALSE;
static DWORD StringsCompressed = FALSE;
CRITICAL_SECTION StringsCS;
static char * StringCompressedPtr;
static DWORD StringCompressedSize;
static WCHAR * ExaStringArray[EXP_APPLY_NUM_STRINGS];
# else
static WCHAR * ExaStringArray[EXP_APPLY_NUM_STRINGS];
# endif
WCHAR * ExaGetString( DWORD StringNum )
{
	if (StringNum > (sizeof(ExaStringArray))/(sizeof(WCHAR *)))
	{
		return(NULL);
	}
# ifdef _WIN32
	while (InterlockedExchange( &StringsInitInProgress, TRUE))
	{
		Sleep( 100 );
	}
	if (StringsInitialized == FALSE)
	{
		HRSRC hTemp;

		InitializeCriticalSection( &StringsCS );
		memset( ExaStringArray, 0, sizeof( ExaStringArray ) );
		if (hTemp = FindResource(hExaApplyModule, (LPCTSTR)(0x604a), (LPCTSTR)(0x5032)))
		{
			StringsCompressed = TRUE;
			StringCompressedPtr = LockResource( LoadResource( hExaApplyModule, hTemp ) );
			StringCompressedSize = *((DWORD *) StringCompressedPtr);
			StringCompressedPtr += 4;
		}
		StringsInitialized = TRUE;
	}
	StringsInitInProgress = FALSE;
	EnterCriticalSection( &StringsCS );
	if (NULL == ExaStringArray[StringNum-1])
	{
		/* haven't read this one yet */
		if (StringsCompressed)
		{
			/*
			Format for Compressed String Table:
			<DWORD Flags>	(bit 0 = 1 => 32-bit PLATWORD)
										(bit 0 = 0 => 16-bit PLATWORD)
			<DWORD # Dir Blocks>
			followed by Dir Blocks
			followed by Compressed Strings
							Dir Block:
			<DWORD ID>
			<PLATWORD Count>		(always odd if using 16-bit PLATWORDs)
			<PLATWORD Pointer0><PLATWORD Pointer1>...<PLATWORD Pointern>
			*/
			char * ThisCS;
			DWORD Flags;
			USHRT * pWord;
			DWORD * pDword;
			DWORD NumDBs;
			DWORD Found = FALSE;
			DWORD ELength;
			DWORD CLength;
			DWORD BLength;
			WCHAR * OutPtr;

			pDword = (DWORD *) StringCompressedPtr;
			Flags = *pDword++;
			NumDBs = *pDword++;
			pWord = (USHRT *) pDword;
			while (NumDBs-- && !Found)
			{
				if (Flags & 1)
				{
					if ((StringNum >= *pDword) && (StringNum - *pDword) < pDword[1])
					{
						Found = TRUE;
						ThisCS = StringCompressedPtr + pDword[2+(StringNum - *pDword)];
					}
					else
					{
						pDword += pDword[1] + 2;
					}
				}
				else
				{
					if ((StringNum >= *pDword) && (StringNum - *pDword) < (UINT)(pWord[2]))
					{
						Found = TRUE;
						ThisCS = StringCompressedPtr + pWord[3+(StringNum - *pDword)];
					}
					else
					{
						pWord += pWord[2] + 3;
						pDword = (DWORD *) pWord;
					}
				}
			}
			if (Found)
			{
				/* Uncompress string */
				if (*ThisCS == 0xff)
				{
					ThisCS++;
					ELength = (DWORD) *((USHRT *) ThisCS);
					ThisCS += 2;
				}
				else
				{
					ELength = (DWORD) *ThisCS;
					ThisCS++;
				}
				if (0 == ExaMemAlloc( NULL, sizeof(WCHAR)*(1+ELength), (void **) ExaStringArray[StringNum-1] )) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				{
					if (ELength)
					{
						if (*ThisCS == 0xff)
						{
							ThisCS++;
							CLength = (DWORD) *((USHRT *) ThisCS);
							ThisCS += 2;
						}
						else
						{
							CLength = (DWORD) *ThisCS;
							ThisCS++;
						}
						OutPtr = ExaStringArray[StringNum-1];
						while (CLength && ELength)
						{
							if (*ThisCS < 0x80)
							{
								*OutPtr++ = (WCHAR) (*ThisCS++);
								CLength--;
								ELength--;
							}
							else if (*ThisCS < 0xc0)
							{
								if (CLength == 1)
								{
									CLength = 0;
									ELength = 0;
									ExaMemFree( NULL, ExaStringArray[StringNum-1] );
									ExaStringArray[StringNum-1] = NULL;
								}
								else
								{
									*OutPtr++ = *((WCHAR *) ThisCS);
									ThisCS += 2;
									CLength -= 2;
									ELength--;
								}
							}
							else
							{
								BLength = (DWORD) (*ThisCS & 0x3f);
								if (BLength == 0)
								{
									BLength = 0x40;
								}
								if (CLength < (1+sizeof(WCHAR)*BLength) || ELength < BLength)
								{
									CLength = 0;
									ELength = 0;
									ExaMemFree( NULL, ExaStringArray[StringNum-1] );
									ExaStringArray[StringNum-1] = NULL;
								}
								else
								{
									memcpy( OutPtr, ThisCS + 1, sizeof(WCHAR)*BLength );
									OutPtr += BLength;
									ThisCS += 1 + 2*BLength;
									CLength -= 1 + 2*BLength;
									ELength -= BLength;
								}
							}
						}
						if (CLength || ELength)
						{
							ExaMemFree( NULL, ExaStringArray[StringNum-1] );
							ExaStringArray[StringNum-1] = NULL;
						}
						else
						{
							*OutPtr = L'\0';
						}
					}
					else
					{
						/* Special code for empty string */
						ExaStringArray[StringNum-1][0] = L'\0';
					}
				}
			}
		}
		else
		{
			/* Uncompressed strings */
			WCHAR BigBuffer[2048];
			DWORD BufSize;
			size_t TheLen;

# ifndef WINCE
			if (0x80000000U & GetVersion())
			{
				char NBuf[2048];

				LoadStringA( hExaApplyModule, StringNum, NBuf, 2048 );
				MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, NBuf, -1, BigBuffer, 2048 );
			}
			else
# endif
			{
				LoadStringW( hExaApplyModule, StringNum, BigBuffer, 2048 );
			}
			TheLen = PSwcslen( BigBuffer );
			if (TheLen <= 0x7ffffffe)
			{
				BufSize = (DWORD) (sizeof(WCHAR)*(1 + TheLen));
				if (0 == ExaMemAlloc(NULL, BufSize, (void **) &ExaStringArray[StringNum-1])) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
				{
					wcscpy_s( ExaStringArray[StringNum-1], TheLen+1, BigBuffer );
				}
			}
		}
	}
	LeaveCriticalSection( &StringsCS );
	return(ExaStringArray[StringNum-1]);

# else
	return(ExaStringArray[StringNum-1]);
# endif
}
int ExaGetNarrowString( DWORD StringNum, char ** ppString )
{
	WCHAR * Ptr;
	int Code = EXAPATCH_SUCCESS;
	DWORD BufSize;
	size_t TheLen;

	Ptr = ExaGetString( StringNum );
	if (Ptr)
	{
		TheLen = PSwcslen( Ptr );
		if (TheLen > 0x7ffffffe)
		{
			Code = EXAPATCH_OUT_OF_MEMORY;
		}
		else
		{
			BufSize = (DWORD) (sizeof(WCHAR)*TheLen + 1);
			if (EXAPATCH_SUCCESS == (Code = ExaMemAlloc( NULL, BufSize, (void **) ppString ))) /* MISRA C 2012 [11.3]: necessary for clean subroutine linkage */
			{
# ifdef _WIN32
				WideCharToMultiByte( CP_ACP, 0, Ptr, -1, *ppString, BufSize, NULL, NULL );
# else
				if (-1 == PSwcstombs( *ppString, Ptr, BufSize))
				{
					Code = EXAPATCH_CHARSET_ERROR;
					ExaMemFree( NULL, *ppString );
					*ppString = NULL;
				}
# endif
			}
		}
	}
	else
	{
		Code = EXAPATCH_INVALID_PARAMETER;
	}
	return(Code);
}
# ifndef _WIN32
# ifndef DUMP_ONLY
# ifndef ATTOPATCH
# ifdef NOT_NEEDED_ANYMORE
static int _wcsicmp( const WCHAR *Str1, const WCHAR *Str2)
{
	WCHAR * Str1U;
	WCHAR * Str2U;
	const WCHAR * In;
	WCHAR * Out;
	int Result;
	Str1U = malloc( sizeof(WCHAR)*(1+PSwcslen( Str1 ) ) );
	Str2U = malloc( sizeof(WCHAR)*(1+PSwcslen( Str2 ) ) );
	for (In = Str1, Out = Str1U; *In; )
	{
		*Out++ = PStowlower( *In++ );
	}
	*Out = L'\0';
	for (In = Str2, Out = Str2U; *In ; )
	{
		*Out++ = PStowlower( *In++ );
	}
	Result = PSwcscmp( Str1U, Str2U );
	free( Str1U );
	free( Str2U );
	return(Result);
}
# endif
# endif
# endif

# ifndef __ANDROID__
static void swprintf_s( WCHAR *WideOut, size_t Count, WCHAR *WideFmt, ...)
{
	va_list ArgList;

	va_start( ArgList, WideFmt );
	vswprintf_s( WideOut, Count, WideFmt, ArgList );
	va_end( ArgList );
}
# endif
static void vswprintf_s( WCHAR *WideOut, size_t Count, WCHAR *WideFmt, va_list ArgList )
{
	char OutBuf[2048];
	char FmtBuf[2048];
	PSwcstombs( FmtBuf, WideFmt, 2048 );
	vsprintf_s( OutBuf, 2048, FmtBuf, ArgList );
	PSmbstowcs( WideOut, OutBuf, Count );
}
/* Include the actual strings */
# define NON_WIN32
# include "exaapstr.str"
# endif

# endif /* !ATTOPATCH */

