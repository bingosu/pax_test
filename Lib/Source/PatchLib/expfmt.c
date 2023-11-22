/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXPFMT.C                                                           |
|                                                                           |
|                                                                           |
|  RTPatch Server File Format Handling Routines                             |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2010.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/
/*lint --e{950, 9026, 9024, 904} */
/* Suppress non-ANSI, function-type macro, ## macro, early return warnings in this file (all deviations are marked) */
# define EXAPATCH_INTERNAL_CODE
# include "exaplat.h"
# include "experror.h"
# include "expfmt.h"

# if defined (_WIN32) && !defined (WINCE) && !defined(_WIN64) && !defined( ATTOPATCH )
#  define DO_INLINE_ASM /* WINCE TODO: Some CE might support this */
# endif

# ifndef ATTOPATCH
INT	GetVarIndex( ExaPatchStream * Stream, QWORD * IndexPtr )
{
# ifndef DO_INLINE_ASM
	unsigned char * Buffer = (unsigned char *) IndexPtr;
# endif
	unsigned char Temp;
	INT Code;

# ifdef DO_INLINE_ASM
  /* PLAT_BIG_ENDIAN is not implemented here */
	*IndexPtr = 0;
	Code = GetBytes( Stream, &Temp, 1);
	if (0 != Code) 
	{
		return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	/* performance: */
	/* 1 byte needs one read */
	/* 2-9 bytes needs two reads */
	/* */
	/* 1-2 byte needs no swaps */
	/* 3-5 bytes needs one swap */
	/* 6-9 bytes needs two swaps */
	/* */
	if (Temp < 0x80U)
	{
		/* one byte - no swap */
		*((unsigned char *)IndexPtr) = Temp;
		return(Code);
	}
	else if (Temp < 0xc0U)
	{
		/* two byte - no swap */
		Code = GetBytes( Stream, IndexPtr, 1 );
		*(1+(unsigned char *)IndexPtr) = Temp & 0x3fU;
		return(Code);
	}
	else if (Temp < 0xe0U)
	{
		/* three bytes - one swap */
		Code = GetBytes( Stream, 2+((unsigned char *)IndexPtr), 2 );
		*(1+(unsigned char *)IndexPtr) = Temp & 0x1fU;
		_asm {
			mov ebx,IndexPtr
			mov eax,[ebx]
			bswap eax
			mov [ebx],eax
			}
			return(Code);
	}
	else if (Temp < 0xf0U)
	{
		/* four bytes - one swap */
		Code = GetBytes( Stream, 1 + ((unsigned char *) IndexPtr), 3 );
		*((unsigned char *)IndexPtr) = Temp & 0xfU;
		_asm {
			mov ebx,IndexPtr
			mov eax,[ebx]
			bswap eax
			mov [ebx],eax
			}
			return(Code);
	}
	else if (Temp < 0xf8U)
	{
		/* five bytes - one swap */
		Code = GetBytes( Stream, IndexPtr, 4 );
		*(4+(unsigned char *)IndexPtr) = Temp & 0x7U;
		_asm {
			mov ebx,IndexPtr
			mov eax,[ebx]
			bswap eax
			mov [ebx],eax
			}
			return(Code);
	}
#  ifdef QWORD_IS_DWORD
	// not enough room to return index
  return(EXAPATCH_NOT_SUPPORTED);
  
#  else
	else if (Temp < 0xfcU)
	{
		/* six bytes - from here on, we fall through to the big double-swap */
		Code = GetBytes( Stream, 3 + ((unsigned char *)IndexPtr), 5 );
		*(2+(unsigned char *)IndexPtr) = Temp & 0x3U;
	}
	else if (Temp < 0xfeU)
	{
		/* seven bytes */
		Code = GetBytes( Stream, 2 + ((unsigned char *)IndexPtr), 6 );
		*(1+(unsigned char *)IndexPtr) = Temp & 0x1U;
	}
	else if (Temp == 0xfeU)
	{
		/* eight bytes */
		Code = GetBytes( Stream, 1 + ((unsigned char *)IndexPtr), 7 );
	}
	else
	{
		/* nine bytes */
		Code = GetBytes( Stream, IndexPtr, 8 );
	}
	_asm {
		mov ebx,IndexPtr
		mov eax,[ebx]
		bswap eax
		mov ecx,[ebx+4]
		bswap ecx
		mov [ebx+4],eax
		mov [ebx],ecx
		}
	return(Code);
#  endif
# else
#  ifdef QWORD_IS_DWORD
	memset( Buffer, 0, 4 );
#  else
	memset( Buffer, 0, 8 );
#  endif
# ifdef PLAT_BIG_ENDIAN
	/* Big Endian code is easier */
	if (Code = GetBytes( Stream, Buffer+7, 1) ) return(Code);
	if (Buffer[7] < 0x80U)
	{
		/* one byte */
		return(Code);
	}
	else if (Buffer[7] < 0xc0U)
	{
		/* two bytes */
		Buffer[6] = Buffer[7] & 0x3fU;
		Code = GetBytes( Stream, Buffer+7, 1);
	}
	else if (Buffer[7] < 0xe0U)
	{
		/* three bytes */
		Buffer[5] = Buffer[7] & 0x1fU;
		Code = GetBytes( Stream, Buffer+6, 2);
	}
	else if (Buffer[7] < 0xf0U)
	{
		/* four bytes */
		Buffer[4] = Buffer[7] & 0xfU;
		Code = GetBytes( Stream, Buffer+5, 3);
	}
	else if (Buffer[7] < 0xf8U)
	{
		/* five bytes */
		Buffer[3] = Buffer[7] & 0x7U;
		Code = GetBytes( Stream, Buffer+4, 4);
	}
#  ifdef QWORD_IS_DWORD
	// not enough room to return index
	else
	{
		Code = EXAPATCH_NOT_SUPPORTED;
	}
#  else
	else if (Buffer[7] < 0xfcU)
	{
		/* six bytes */
		Buffer[2] = Buffer[7] & 0x3U;
		Code = GetBytes( Stream, Buffer+3, 5);
	}
	else if (Buffer[7] < 0xfeU)
	{
		/* seven bytes */
		Buffer[1] = Buffer[7] & 0x1U;
		Code = GetBytes( Stream, Buffer+2, 6);
	}
	else if (Buffer[7] == 0xfeU)
	{
		/* eight bytes */
		Code = GetBytes( Stream, Buffer+1, 7);
	}
	else
	{
		/* nine bytes */
		Code = GetBytes( Stream, Buffer, 8);
	}
#  endif
# else
	if (Code = GetBytes( Stream, Buffer, 1) ) return(Code);
	/* performance is given by (r,s) where r = # reads */
	/*	s = # swaps */
	if (*Buffer < 0x80U)
	{
		/* one byte (1,0) */
		return(Code);
	}
	else if (*Buffer < 0xc0U)
	{
		/* two bytes (2,0) */
		Buffer[1] = Buffer[0] & 0x3fU;
		Code = GetBytes( Stream, Buffer, 1);
	}
	else if (*Buffer < 0xe0U)
	{
		/* three bytes (2,1) */
		Buffer[2] = Buffer[0] & 0x1fU;
		Code = GetBytes( Stream, Buffer, 2);
		Temp = Buffer[0];
		Buffer[0] = Buffer[1];
		Buffer[1] = Temp;
	}
	else if (*Buffer < 0xf0U)
	{
		/* four bytes (2,1) */
		Buffer[3] = Buffer[0] & 0xfU;
		Code = GetBytes( Stream, Buffer, 3);
		Temp = Buffer[0];
		Buffer[0] = Buffer[2];
		Buffer[2] = Temp;
	}
	else if (*Buffer < 0xf8U)
	{
		/* five bytes (2,2) */
		Buffer[4] = Buffer[0] & 0x7U;
		Code = GetBytes( Stream, Buffer, 4);
		Temp = Buffer[0];
		Buffer[0] = Buffer[3];
		Buffer[3] = Temp;
		Temp = Buffer[1];
		Buffer[1] = Buffer[2];
		Buffer[2] = Temp;
	}
#  ifdef QWORD_IS_DWORD
	else
	{
		Code = EXAPATCH_NOT_SUPPORTED;
	}
#  else
	else if (*Buffer < 0xfcU)
	{
		/* six bytes (2,2) */
		Buffer[5] = Buffer[0] & 0x3U;
		Code = GetBytes( Stream, Buffer, 5);
		Temp = Buffer[0];
		Buffer[0] = Buffer[4];
		Buffer[4] = Temp;
		Temp = Buffer[1];
		Buffer[1] = Buffer[3];
		Buffer[3] = Temp;
	}
	else if (*Buffer < 0xfeU)
	{
		/* seven bytes (2,3) */
		Buffer[6] = Buffer[0] & 0x1U;
		Code = GetBytes( Stream, Buffer, 6);
		Temp = Buffer[0];
		Buffer[0] = Buffer[5];
		Buffer[5] = Temp;
		Temp = Buffer[1];
		Buffer[1] = Buffer[4];
		Buffer[4] = Temp;
		Temp = Buffer[2];
		Buffer[2] = Buffer[3];
		Buffer[3] = Temp;
	}
	else if (*Buffer == 0xfeU)
	{
		/* eight bytes (2,3) */
		Code = GetBytes( Stream, Buffer, 7);
		Temp = Buffer[0];
		Buffer[0] = Buffer[6];
		Buffer[6] = Temp;
		Temp = Buffer[1];
		Buffer[1] = Buffer[5];
		Buffer[5] = Temp;
		Temp = Buffer[2];
		Buffer[2] = Buffer[4];
		Buffer[4] = Temp;
	}
	else
	{
		/* nine bytes (2,4) */
		Code = GetBytes( Stream, Buffer, 8);
		Temp = Buffer[0];
		Buffer[0] = Buffer[7];
		Buffer[7] = Temp;
		Temp = Buffer[1];
		Buffer[1] = Buffer[6];
		Buffer[6] = Temp;
		Temp = Buffer[2];
		Buffer[2] = Buffer[5];
		Buffer[5] = Temp;
		Temp = Buffer[3];
		Buffer[3] = Buffer[4];
		Buffer[4] = Temp;
	}
#  endif
# endif
	return(Code);
# endif
}

INT PutVarIndex( ExaPatchStream * Stream, QWORD Index )
{
# ifdef PLAT_BIG_ENDIAN
	/* Big Endian case is easier */
	DWORD Count;
	DWORD HalfIndex;
	unsigned char * Ptr;
	unsigned char Buffer[9];

#  ifdef QWORD_IS_DWORD
	memset( Buffer, 0, 4);
	memcpy( Buffer + 4, &Index, 4 );
#  else
	memcpy( Buffer, &Index, 8 );
#  endif
#  ifndef QWORD_IS_DWORD
	if (*((DWORD *) Buffer))
	{
		/* high DWORD is non-zero */
		HalfIndex = *((DWORD *)Buffer));
		if (HalfIndex < 8)
		{
			/* five bytes */
			Buffer[3] |= 0xf0U;
			Ptr = Buffer + 3;
			Count = 5;
		}
		else if (HalfIndex < 0x400U)
		{
			/* six bytes */
			Buffer[2] |= 0xf8U;
			Ptr = Buffer + 2;
			Count = 6;
		}
		else if (HalfIndex < 0x20000U)
		{
			/* seven bytes */
			Buffer[1] |= 0xfcU;
			Ptr = Buffer + 1;
			Count = 7;
		}
		else if (HalfIndex < 0x1000000U)
		{
			/* eight bytes */
			Buffer[3] |= 0xfeU;
			Ptr = Buffer;
			Count = 8;
		}
		else
		{
			/* nine bytes */
			memcpy( Buffer+1, &Index, 8 );
			Buffer[0] = 0xffU;
			Ptr = Buffer;
			Count = 9;
		}
	}
	else
#  endif	
	{
		HalfIndex = ((DWORD) Index );
		if (HalfIndex < 0x80U)
		{
			/* one byte */
			Ptr = Buffer + 7;
			Count = 1;
		}
		else if (HalfIndex < 0x4000U)
		{
			/* two bytes */
			Buffer[6] |= 0x80U;
			Ptr = Buffer + 6;
			Count = 2;
		}
		else if (HalfIndex < 0x200000U)
		{
			/* three bytes */
			Buffer[5] |= 0xc0U;
			Ptr = Buffer + 5;
			Count = 3;
		}
		else if (HalfIndex < 0x10000000U)
		{
			/* four bytes */
			Buffer[4] |= 0xe0U;
			Ptr = Buffer + 4;
			Count = 4;
		}
		else
		{
			/* five bytes */
			Buffer[3] = 0xf0U;
			Ptr = Buffer + 3;
			Count = 5;
		}
	}
# else
#  ifndef DO_INLINE_ASM
	unsigned char TempChar;
#  endif
	unsigned char Buffer[9];
	unsigned char * Ptr;
	unsigned char * SwapPtr;
#  ifdef _WIN32
	LARGE_INTEGER ll;
#  else
	struct LLL 
	{
		DWORD LowPart;
		DWORD HighPart;
	} ll;
#  endif
	DWORD Temp;
	DWORD Count;

#  ifdef QWORD_IS_DWORD
	ll.LowPart = Index;
	ll.HighPart = 0;
#  else
#  ifdef _WIN32
	ll.QuadPart = Index;
#  else
	ll.HighPart = (DWORD) (Index >> 32);
	ll.LowPart = (DWORD) (Index & (QWORD) 0xffffffffU);
#  endif

	if (ll.HighPart)
	{
		*((DWORD *)(Buffer + 5)) = ll.LowPart;
		SwapPtr = Buffer + 5;
#  ifdef DO_INLINE_ASM
		_asm {
			mov ebx,SwapPtr
			mov eax,[ebx]
			bswap eax
			mov [ebx],eax
			}
#  else
		TempChar = SwapPtr[0];
		SwapPtr[0] = SwapPtr[3];
		SwapPtr[3] = TempChar;
		TempChar = SwapPtr[1];
		SwapPtr[1] = SwapPtr[2];
		SwapPtr[2] = TempChar;				
#  endif
		Temp = ll.HighPart;
		if (Temp < 0x8U)
		{
			/* five byte */
			Buffer[4] = (unsigned char) (Temp | 0xf0U);
			Ptr = Buffer + 4;
			Count = 5;
		}
		else if (Temp < 0x400U)
		{
			/* six byte */
			*((USHRT *) (Buffer+1)) = (USHRT) (Temp | 0xf800U);
			Ptr = Buffer + 3;
			Count = 6;
		}
		else if ( Temp < 0x20000U)
		{
			/* seven byte */
			*((DWORD *) (Buffer+1)) = Temp | 0xfc0000U;
			Ptr = Buffer + 2;
			Count = 7;
		}
		else if (Temp < 0x1000000U)
		{
			/* eight byte */
			*((DWORD *) (Buffer+1)) = Temp | 0xfe000000U;
			Ptr = Buffer + 1;
			Count =8;
		}
		else
		{
			/* nine byte */
			*((DWORD *) (Buffer+1)) = Temp;
			Buffer[0] = 0xffU;
			Ptr = Buffer;
			Count = 9;
		}
		if (Count != 5)
		{
			SwapPtr = Buffer + 1;
#  ifdef DO_INLINE_ASM
			_asm {
				mov ebx,SwapPtr
				mov eax,[ebx]
				bswap eax
				mov [ebx],eax
				}
#  else
			TempChar = SwapPtr[0];
			SwapPtr[0] = SwapPtr[3];
			SwapPtr[3] = TempChar;
			TempChar = SwapPtr[1];
			SwapPtr[1] = SwapPtr[2];
			SwapPtr[2] = TempChar;				
#  endif
		}
	}
	else
#  endif	
	{
		/* generic case... */
		Temp = ll.LowPart;
		if (Temp < 0x80U)
		{
			/* one byte */
			Buffer[0] = (UCHAR) Temp;
			Ptr = Buffer;
			Count = 1;
		}
		else if (Temp < 0x4000U)
		{
			/* two byte */
			*((USHRT *) (Buffer+1)) = (USHRT) (Temp | 0x8000U);
			Ptr = Buffer+3;
			Count = 2;
		}
		else if (Temp < 0x200000U)
		{
			/* three byte */
			*((DWORD *) (Buffer+1)) = Temp | 0xc00000U;
			Ptr = Buffer + 2;
			Count = 3;
		}
		else if (Temp < 0x10000000U)
		{
			/* four byte */
			*((DWORD *) (Buffer+1)) = Temp | 0xe0000000U;
			Ptr = Buffer+1;
			Count = 4;
		}
		else
		{
			/* five byte */
			*((DWORD *) (1+Buffer)) = Temp;
			Buffer[0] = 0xf0U;
			Ptr = Buffer;
			Count = 5;
		}
		if (Count != 1)
		{
			SwapPtr = Buffer+1;
#  ifdef DO_INLINE_ASM
			_asm {
				mov ebx,SwapPtr
				mov eax,[ebx]
				bswap eax
				mov [ebx],eax
				}
#  else
			TempChar = SwapPtr[0];
			SwapPtr[0] = SwapPtr[3];
			SwapPtr[3] = TempChar;
			TempChar = SwapPtr[1];
			SwapPtr[1] = SwapPtr[2];
			SwapPtr[2] = TempChar;				
#  endif
		}
	}
# endif
	return(PutBytes(Stream, Ptr, Count));
}
INT GetVarStr( ExaPatchStream * Stream, wchar_t ** PtrPtr )
{
	//NOTE: the current VarStr routines are only capable of dealing with 16-bit characters
	//  the actual value of sizeof(wchar_t) may be larger, but we will only encode the lower 16 bits.
	//  if this ever causes anyone any *actual* grief, we'll have to deal with it then
	unsigned char * Buffer;
	unsigned char * CurNPtr;
	wchar_t * CurWPtr;
	DWORD CLength=0;
	DWORD ELength=0;
	DWORD BLength;
	unsigned char TC;
	INT Code;

	*PtrPtr = NULL;
	if (Code = GetBytes( Stream, &TC, 1) ) return(Code);
	if (TC != 0xffU)
	{
		ELength = (DWORD) TC;
	}
	else
	{
# ifdef PLAT_BIG_ENDIAN
		if (Code = GetBytes( Stream, &ELength, 2) ) return(Code);
		ELength = ((ELength & 0xff000000U) >> 24) | ((ELength & 0xff0000U) >> 8);
# else
		if (Code = GetBytes( Stream, &ELength, 2) ) return(Code);
# endif
	}
	if (ELength == 0)
	{
		/* special code for empty string */
		if (Code = ExaMemAlloc( NULL, 2, (void **) PtrPtr ) ) return(Code);
		**PtrPtr = L'\0';
		return(Code);
	}

	if (Code = GetBytes( Stream, &TC, 1) ) return(Code);
	if (TC != 0xffU)
	{
		CLength = (DWORD) TC;
	}
	else
	{
# ifdef PLAT_BIG_ENDIAN
		if (Code = GetBytes( Stream, &CLength, 2) ) return(Code);
		CLength = ((CLength & 0xff000000U) >> 24) | ((CLength & 0xff0000U) >> 8);
# else
		if (Code = GetBytes( Stream, &CLength, 2) ) return(Code);
# endif
	}
	if (Code = ExaMemAlloc( NULL, sizeof(wchar_t)*(ELength+1), (void **) PtrPtr ) ) return(Code);
	if (Code = ExaMemAlloc( NULL, CLength, (void **) &Buffer) )
	{
		ExaMemFree( NULL, *PtrPtr );
		*PtrPtr = NULL;
		return(Code);
	}
  if (Code = GetBytes( Stream, Buffer, CLength ) ) goto exit;
	CurWPtr = *PtrPtr;
	CurNPtr = Buffer;
	while (CLength && ELength)
	{
		if (*CurNPtr < 0x80U)
		{
			/* one byte */
			*(CurWPtr++) = (wchar_t) (*(CurNPtr++));
			CLength--;
			ELength--;
		}
		else if (*CurNPtr < 0xc0U)
		{
			/* two byte */
			if (CLength == 1)
			{
				Code = EXAPATCH_CORRUPT_PATCH_FILE;
				goto exit;
			}
			else
			{
				*(CurWPtr++) = (((wchar_t) (0x3fU & CurNPtr[0])) << 8) | ((wchar_t) (CurNPtr[1]));
        CurNPtr += 2;
				CLength -= 2;
				ELength--;
			}
		}
		else
		{
			/* literal escape */
		  BLength = (DWORD) (CurNPtr[0] & 0x3fU);
			if (BLength == 0)
			{
				BLength = 0x40U;
			}
			if (CLength < (1+2*BLength) || ELength < BLength)
			{
				Code = EXAPATCH_CORRUPT_PATCH_FILE;
				goto exit;
			}
			else
			{
# ifdef PLAT_BIG_ENDIAN
				CurNPtr++;
				CLength--;
				while (BLength--)
				{
					*(CurWPtr++) =  (((wchar_t)CurNPtr[1]) << 8) | ((wchar_t) CurNPtr[0]);
					CurNPtr += 2;
					ELength--;
					CLength -= 2;
				}
# else
				if (sizeof(wchar_t)==2)
				{
					memcpy( CurWPtr, CurNPtr+1, 2*BLength );
					CurWPtr += BLength;
					CurNPtr += 1+2*BLength;
					CLength -= 1+2*BLength;
					ELength -= BLength;
				}
				else
				{
					CurNPtr++;
					CLength--;
					while (BLength--)
					{
						*(CurWPtr++) = (((wchar_t)CurNPtr[1]) << 8) | ((wchar_t) CurNPtr[0]);
						CurNPtr += 2;
						ELength--;
						CLength -= 2;
					}
				}
# endif
			}
		}
	}
	if (CLength || ELength)
	{
		Code = EXAPATCH_CORRUPT_PATCH_FILE;
		goto exit;
	}
	*CurWPtr = L'\0';
exit:
	ExaMemFree( NULL, Buffer );
	if (Code)	
	{
		ExaMemFree( NULL, *PtrPtr );
		*PtrPtr = NULL;
	}
	return(Code);
}

INT PutVarStr( ExaPatchStream * Stream, wchar_t * Ptr )
{
	/* this code is End-ependent, but as noted above, is limited to 16-bit character sets */
	unsigned char LenBuf[6];
	unsigned char * Buffer;
	DWORD CLength;
	DWORD ELength;
	DWORD CLength2;
	unsigned char * NPtr;
	unsigned char * NPtr2;
	wchar_t * WPtr;
	INT Code;
	size_t TheLen;

	if (Ptr == NULL) Ptr = L"";
	TheLen = wcslen( Ptr );
	if (TheLen > 0x7ffffffeU) return EXAPATCH_OUT_OF_MEMORY;
	ELength = (DWORD)(TheLen);
	if (ELength)
	{
		if (Code = ExaMemAlloc( NULL, 3*ELength, (void **) &Buffer) ) return(Code);
		for (NPtr = Buffer, CLength = 0, WPtr = Ptr; *WPtr ; WPtr++ )
		{
			if (*WPtr < 0x80U)
			{
				*NPtr++ = (unsigned char) *WPtr;
				CLength++;
			}
			else if (*WPtr < 0x4000U)
			{
				*NPtr++ = (unsigned char) ((0x8000U | *WPtr) >> 8);
				*NPtr++ = (unsigned char) (0xffU & *WPtr);
				CLength += 2;
			}
			else
			{

				for (CLength2 = 0, NPtr2 = NPtr + 1; (*WPtr >= 0x80U) && (CLength2 <= 0x40U) ; WPtr++, CLength2++ )
				{
					*NPtr2++ = (unsigned char)(0xffU & *WPtr);
					*NPtr2++ = (unsigned char)(*WPtr >> 8);
				}
				*NPtr = 0xc0U | ((unsigned char) (0x3fU & CLength2));
				NPtr = NPtr2;
				WPtr--;
				CLength += 1+2*CLength2;
			}
		}
		if (ELength < 0xffU)
		{
			CLength2 = 1;
			LenBuf[0] = (unsigned char)(ELength);
		}
		else if (ELength < 0xffffU)
		{
			CLength2 = 3;
			LenBuf[0] = 0xffU;
			LenBuf[1] = (unsigned char) (ELength & 0xffU);
			LenBuf[2] = (unsigned char) (ELength >> 8);
		}
		else
		{
			Code = EXAPATCH_INVALID_PARAMETER;
			goto exit;
		}
		if (CLength < 0xffU)
		{
			LenBuf[CLength2] = (unsigned char)(CLength);
			CLength2 += 1;
		}
		else if (CLength < 0xffffU)
		{
			LenBuf[CLength2++] = 0xffU;
			LenBuf[CLength2++] = (unsigned char) (CLength & 0xffU);
			LenBuf[CLength2++] = (unsigned char) (CLength >> 8);
		}
		else
		{
			Code = EXAPATCH_INVALID_PARAMETER;
			goto exit;
		}
		if (Code = PutBytes( Stream, LenBuf, CLength2) ) goto exit;
		if (Code = PutBytes( Stream, Buffer, CLength) ) goto exit;
	}
	else
	{
		/* special case */
		return(PutBytes(Stream, &ELength, 1));
	}
exit:
	ExaMemFree( NULL, Buffer );
	return(Code);


}
# endif /* ATTOPATCH */


//20161103
# ifdef PLAT_BIG_ENDIAN
void swab(char *to, char *from, int cnt)
{
	int i;

	for(i=0; i<cnt; i++)
	{
		to[i] = from[cnt-i-1];
	}
	return;
}
#endif

int PutWord( ExaPatchStream * Stream, USHRT Amount )
{
# ifdef PLAT_BIG_ENDIAN
	USHRT Temp;
	swab((char *)&Temp,(char *)&Amount,2);
	return(PutBytes(Stream, &Temp, 2));
# else
	USHRT Temp = Amount;
	return(PutBytes(Stream, &Temp, 2));
# endif
	
}
int PutDword( ExaPatchStream * Stream, DWORD Amount )
{
# ifdef PLAT_BIG_ENDIAN
	char SwapBuf[4];
	char * Ptr = (char *) &Amount;

	SwapBuf[0] = Ptr[3];
	SwapBuf[1] = Ptr[2];
	SwapBuf[2] = Ptr[1];
	SwapBuf[3] = Ptr[0];
	
	return(PutBytes(Stream, SwapBuf, 4));
# else
	DWORD Temp = Amount;
	return(PutBytes(Stream, &Temp, 4));
# endif
}
int PutQword( ExaPatchStream * Stream, QWORD Amount )
{
# ifdef PLAT_BIG_ENDIAN
	char SwapBuf[8];
	char * Ptr = (char *) &Amount;

#  ifdef QWORD_IS_DWORD
	SwapBuf[4] = SwapBuf[5] = SwapBuf[6] = SwapBuf[7] = 0;
	SwapBuf[0] = Ptr[3];
	SwapBuf[1] = Ptr[2];
	SwapBuf[2] = Ptr[1];
	SwapBuf[3] = Ptr[0];
#  else
	SwapBuf[0] = Ptr[7];
	SwapBuf[1] = Ptr[6];
	SwapBuf[2] = Ptr[5];
	SwapBuf[3] = Ptr[4];
	SwapBuf[4] = Ptr[3];
	SwapBuf[5] = Ptr[2];
	SwapBuf[6] = Ptr[1];
	SwapBuf[7] = Ptr[0];
#  endif
	
	return(PutBytes(Stream, SwapBuf, 8));
# else
#  ifdef QWORD_IS_DWORD
	DWORD Temp[2];
	Temp[0] = Amount;
	Temp[1] = 0;
	return(PutBytes(Stream, &Temp[0], 8));
#  else
	QWORD Temp = Amount;
	return(PutBytes(Stream, &Temp, 8));
#  endif
# endif
}

int GetWord( ExaPatchStream * Stream, USHRT * AmtPtr )
{
# ifdef PLAT_BIG_ENDIAN
	USHRT Temp;
	INT Code;

	Code = GetBytes(Stream, &Temp, 2);
	if (Code == EXAPATCH_SUCCESS)
	{
		swab( (char *)AmtPtr, (char *) &Temp, 2 );
	}
	return(Code);
	
# else
	return(GetBytes(Stream, AmtPtr, 2));
# endif
}
int GetDword( ExaPatchStream * Stream, DWORD * AmtPtr )
{
# ifdef PLAT_BIG_ENDIAN
	DWORD Temp;
	INT Code;
	char * Ptr = (char *)AmtPtr;
	char * Ptr2 = (char *)&Temp;

	Code = GetBytes(Stream, &Temp, 4);
	if (Code == EXAPATCH_SUCCESS)
	{
		Ptr[0] = Ptr2[3];
		Ptr[1] = Ptr2[2];
		Ptr[2] = Ptr2[1];
		Ptr[3] = Ptr2[0];
	}
	return(Code);

# else
	return(GetBytes(Stream, AmtPtr, 4));
# endif
}
int GetQword( ExaPatchStream * Stream, QWORD * AmtPtr )
{
# ifdef PLAT_BIG_ENDIAN
	QWORD Temp;
	INT Code;
	char * Ptr = (char *)AmtPtr;
	char * Ptr2 = (char *)&Temp;

#  ifdef QWORD_IS_DWORD
	DWORD Temp2;
	Code = GetBytes(Stream,&Temp, 4);
	GetBytes(Stream, &Temp2, 4);
	if (Code == EXAPATCH_SUCCESS)
	{
		Ptr[0] = Ptr2[3];
		Ptr[1] = Ptr2[2];
		Ptr[2] = Ptr2[1];
		Ptr[3] = Ptr2[0];
	}
#  else
	Code = GetBytes(Stream, &Temp, 8);
	if (Code == EXAPATCH_SUCCESS)
	{
		Ptr[0] = Ptr2[7];
		Ptr[1] = Ptr2[6];
		Ptr[2] = Ptr2[5];
		Ptr[3] = Ptr2[4];
		Ptr[4] = Ptr2[3];
		Ptr[5] = Ptr2[2];
		Ptr[6] = Ptr2[1];
		Ptr[7] = Ptr2[0];
	}
#  endif
	return(Code);

# else
#  ifdef QWORD_IS_DWORD
	DWORD Temp;
	int Code = GetBytes(Stream, AmtPtr, 4);
	GetBytes(Stream,&Temp,4);
	return(Code);
#  else
	return(GetBytes(Stream, AmtPtr, 8));
#  endif
# endif
}

int GetBytes( ExaPatchStream * Stream, void * Ptr, DWORD Size )
{
	DWORD SizeRead;
	int Code;

	Code = ReadExaStream( Stream, Ptr, Size, &SizeRead );
	if (0 != Code)
	{
	 return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	if (SizeRead != Size)
	{
		return(EXAPATCH_UNEXPECTED_EOF); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	return(EXAPATCH_SUCCESS);
}

int PutBytes( ExaPatchStream * Stream, const void * Ptr, DWORD Size )
{
	DWORD SizeWritten;
	int Code;

	Code = WriteExaStream( Stream, Ptr, Size, &SizeWritten );
	if (0 != Code)
	{
	 return(Code); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	if (SizeWritten != Size)
	{
		return(EXAPATCH_DISK_FULL); /* MISRA C 2012 [15.5]: return used for quick exit in error condition  */
	}
	return(EXAPATCH_SUCCESS);
}

