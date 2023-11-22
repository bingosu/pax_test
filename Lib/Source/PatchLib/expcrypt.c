/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXPCRYPT.C                                                         |
|                                                                           |
|                                                                           |
|  RTPatch Server Encryption Routines                                       |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2010.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/
# define EXAPATCH_INTERNAL_CODE
# include "exaplat.h"
# include "experror.h"
# include "expfmt.h"
# include "expcrpt.h"
/*
A Short Discourse on Encryption and Endian-ness:

The QWORDs referred to here are *really* two DWORDs "First" and "Second"
 in memory.  However, each of the DWORDs really is an endianly-correct DWORD.

On the other hand, the bytes in the UsePW and CryptPW Buffers are really bytes
and need no swapping whatsoever.

This is because the TwoFish code is designed to work with either
endian-ness, BUT the "keys" are all arrays of 4 endianly-correct DWORDs, while
the "buffers" are all end-ependent (although they must be a multiple of 16 bytes
in length).

So, all we have to do is:

1) make sure that the QWORDs are input/output as pairs of DWORDs
2) in CrunchPW, make sure that the calculation of the "silly hash"
	really produces an end-ependent 16-byte array from endianly-correct wide 
	characters.  This is handled by the ADDR_WORD_XOR construct.
3) in CrunchPW, make sure that Buffer2 is not used as a key until it
	has been made endianly-correct.  This is the first FixedBuffer construct.
4) in CrunchPW, make sure that Buffer3 has been made endianly-correct
	before it is passed back.  This is the second FixedBuffer construct.

Clean and simple, n'est-ce-pas?

*/
# ifdef EXAPATCH_PASSWORD_SUPPORT
int CrunchPW( wchar_t * Password, QWORD * Hash1, QWORD * Hash2 )
{
	PSSKEY TheKey;
	unsigned char Buffer[16];
	unsigned char Buffer2[16];
	unsigned char FixedBuffer[16];
	unsigned char Buffer3[16];
	DWORD FixedKey[4] = {
		0x4349522a,
		0x4e552045,
		0x52455649,
		0x59544953
		};
	int i,j;
	int Dir=+1;
	int Count;
  int Code;
	unsigned char * Ptr = (unsigned char *) Password;

	/* first, do a silly hash */
	memset( Buffer, 0, 16 );
	for (i = 0, j=0, Count = (DWORD) (2*PSwcslen( Password ));	//NOTE: Password is limited to 2GB
		Count ;
		Count--, i += Dir, j++ )
	{
		Buffer[i] ^= Ptr[j ^ ADDR_WORD_XOR];
		if ((i == 15) && (Dir == 1))
		{
			i = 16;
			Dir = -1;
		}
		else if (( i == 0 ) && (Dir == -1))
		{
			i = -1;
			Dir = 1;
		}
	}
	/* then, SymmEncrypt the silly hash with a Fixed Key */
	Code = CryptInit( &TheKey, DIR_ENCRYPT, FixedKey );
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	Code = ExaCryptEncrypt( &TheKey, Buffer, 16, Buffer2 );
	/* endianly-fix the result */
	for (i=0; i<16 ; i++ )
	{
		FixedBuffer[i^ADDR_XOR] = Buffer2[i];
	}
	/* then, SymmEncrypt the result with itself */
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	Code = CryptInit( &TheKey, DIR_ENCRYPT, (DWORD *) FixedBuffer );
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	Code = ExaCryptEncrypt( &TheKey, Buffer2, 16, Buffer3 );
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	/* endianly-fix the result */
	for (i=0; i<16 ; i++ )
	{
		FixedBuffer[i ^ ADDR_XOR] = Buffer3[i];
	}
	memcpy( Hash1, FixedBuffer, 8 );
	memcpy( Hash2, FixedBuffer + 8, 8 );
	return(EXAPATCH_SUCCESS);
}

int UsePW( void * Buffer, QWORD * Hash1, QWORD * Hash2 )
{
	PSSKEY TheKey;
	QWORD KeyData[2];
	int Code;
	unsigned char OutBuffer[16];

	KeyData[0] = *Hash1;
	KeyData[1] = *Hash2;
	Code = CryptInit( &TheKey, DIR_DECRYPT, (DWORD *) &KeyData[0] );
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	Code = ExaCryptDecrypt( &TheKey, Buffer, 16, OutBuffer );
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	memcpy( Buffer, OutBuffer, 16 );
	return(EXAPATCH_SUCCESS);
}

int CryptPW( void * Buffer, QWORD * Hash1, QWORD * Hash2 )
{
	PSSKEY TheKey;
	QWORD KeyData[2];
	int Code;
	unsigned char OutBuffer[16];

	KeyData[0] = *Hash1;
	KeyData[1] = *Hash2;
	Code = CryptInit( &TheKey, DIR_ENCRYPT, (DWORD *) &KeyData[0] );
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	Code = ExaCryptEncrypt( &TheKey, Buffer, 16, OutBuffer );
	if (Code) return(EXAPATCH_ENCRYPTION_FAILURE);
	memcpy( Buffer, OutBuffer, 16 );
	return(EXAPATCH_SUCCESS);
}

/***************************************************************************
	Pocket Soft ExaPatch-specific Implementation of Counterpane 
			TWOFISH symmetric key cipher

	Authors:
		Bruce Schneier, Counterpane Systems
		Doug Whiting,	Hi/fn
		John Kelsey,	Counterpane Systems
		Chris Hall,		Counterpane Systems
		David Wagner,	UC Berkeley

	Redactor:
		Kerry Jones, Pocket Soft, Inc.

		Notes: Optimized C code, equivalent to Counterpane's FULL_KEY,
			BIG_TAB version.  AES stupidity is removed.  only 128-bit keys
			in CBC mode are supported

		In this version, very little is explained or commented
		See the astonishingly similar code in ps2fish.c for help...
			

***************************************************************************/

fullSbox MDStab;		
int			needToBuildMDS=1;		
BYTE		bigTab[4][256][256];
static 	fullSbox _sBox_;		
#define _sBox8_(N) (((BYTE *) _sBox_) + (N)*256)
#define	Fe32_(x,R) (_sBox_[0][2*_b(x,R  )] ^ _sBox_[0][2*_b(x,R+1)+1] ^ _sBox_[2][2*_b(x,R+2)] ^ _sBox_[2][2*_b(x,R+3)+1])
#define sbSet(N,i,J,v) { _sBox_[N&2][2*i+(N&1)+2*J]=MDStab[N][v]; }

static DWORD RS_MDS_Encode(DWORD k0,DWORD k1)
{
	int i,j;
	DWORD r;

	for (i=r=0;i<2;i++)
	{
		r ^= (i) ? k0 : k1;			
		for (j=0;j<4;j++)		 
			RS_rem(r);				
	}
	return r;
}


static void BuildMDS(void)
{
	int i;
	DWORD d;
	BYTE m1[2],mX[2],mY[4];
	int j,k;
	BYTE *q0 = NULL,*q1 = NULL;

	for (i=0;i<256;i++)
	{
		m1[0]=P8x8[0][i];		
		mX[0]=(BYTE) Mul_X(m1[0]);
		mY[0]=(BYTE) Mul_Y(m1[0]);

		m1[1]=P8x8[1][i];
		mX[1]=(BYTE) Mul_X(m1[1]);
		mY[1]=(BYTE) Mul_Y(m1[1]);

#undef	Mul_1				 
#undef	Mul_X
#undef	Mul_Y
#define	Mul_1	m1		 
#define	Mul_X	mX				
#define	Mul_Y	mY

#define	SetMDS(N)					\
		b0(d) = M0##N[P_##N##0];	\
		b1(d) = M1##N[P_##N##0];	\
		b2(d) = M2##N[P_##N##0];	\
		b3(d) = M3##N[P_##N##0];	\
		MDStab[N][i] = d;

		SetMDS(0);			 
		SetMDS(1);
		SetMDS(2);
		SetMDS(3);
	}
#undef	Mul_1
#undef	Mul_X
#undef	Mul_Y
#define	Mul_1	Mx_1	 
#define	Mul_X	Mx_X
#define	Mul_Y	Mx_Y
	

	for (i=0;i<4;i++)
	{
		switch (i)
		{
			case 0:	q0=p8(01); q1=p8(02);	break;
			case 1:	q0=p8(11); q1=p8(12);	break;
			case 2:	q0=p8(21); q1=p8(22);	break;
			case 3:	q0=p8(31); q1=p8(32);	break;
		}
		for (j=0;j<256;j++)
			for (k=0;k<256;k++)
				bigTab[i][j][k]=q0[q1[k]^j];
	}

	needToBuildMDS=0;
}

static void ReverseRoundSubkeys(keyInstance *key,BYTE newDir)
{
	DWORD t0,t1;
	register DWORD *r0=key->subKeys+ROUND_SUBKEYS;
	register DWORD *r1=r0 + 2*key->numRounds - 2;

	for (;r0 < r1;r0+=2,r1-=2)
	{
		t0=r0[0];			 
		t1=r0[1];
		r0[0]=r1[0];	 
		r0[1]=r1[1];
		r1[0]=t0;
		r1[1]=t1;
	}

	key->direction=newDir;
}

static void Xor256(void *dst,void *src,BYTE b)
{
	register DWORD	x=b*0x01010101u;
	register DWORD *d=(DWORD *)dst;
	register DWORD *s=(DWORD *)src;
#define X_8(N)	{ d[N]=s[N] ^ x; d[N+1]=s[N+1] ^ x; }
#define X_32(N)	{ X_8(N); X_8(N+2); X_8(N+4); X_8(N+6); }
	X_32(0 ); X_32( 8); X_32(16); X_32(24);
	d+=32;
	s+=32;
	X_32(0 ); X_32( 8); X_32(16); X_32(24);
}

static int CryptInit( LPPSSKEY pKey, BYTE direction, DWORD * pKeyData )
{
	int		i,j;
	int		subkeyCnt;
	DWORD	A=0,B=0,q;
	DWORD	sKey[MAX_KEY_BITS/64],k32e[MAX_KEY_BITS/64],k32o[MAX_KEY_BITS/64];
	BYTE	L0[256];

	pKey->TheKey.direction = direction;
	pKey->TheKey.keyLen = BLOCK_SIZE;
	pKey->TheKey.numRounds =  ROUNDS_128;
	memset( pKey->TheKey.key32,0,sizeof(pKey->TheKey.key32));
	memcpy( pKey->TheKey.key32,pKeyData,16 );

	if (needToBuildMDS)
		BuildMDS();
#define	F32(res,x,k32)	\
	{											\
	DWORD t=x;						\
	res=	MDStab[0][p8(01)[p8(02)[b0(t)] ^ b0(k32[1])] ^ b0(k32[0])] ^	\
				MDStab[1][p8(11)[p8(12)[b1(t)] ^ b1(k32[1])] ^ b1(k32[0])] ^	\
				MDStab[2][p8(21)[p8(22)[b2(t)] ^ b2(k32[1])] ^ b2(k32[0])] ^	\
				MDStab[3][p8(31)[p8(32)[b3(t)] ^ b3(k32[1])] ^ b3(k32[0])] ;	\
	}
	subkeyCnt = ROUND_SUBKEYS + 2*pKey->TheKey.numRounds;
	for (i=0,j=1;i<2;i++,j--)
	{						 
		k32e[i]=pKey->TheKey.key32[2*i  ];
		k32o[i]=pKey->TheKey.key32[2*i+1];
		sKey[j]=pKey->TheKey.sboxKeys[j]=RS_MDS_Encode(k32e[i],k32o[i]);
	}

	for (i=q=0;i<subkeyCnt/2;i++,q+=SK_STEP)	
	{						 
		F32(A,q        ,k32e); 
		F32(B,q+SK_BUMP,k32o); 
		B = ROL(B,8);
		pKey->TheKey.subKeys[2*i  ] = A+B;
		B = A + 2*B;
		pKey->TheKey.subKeys[2*i+1] = ROL(B,SK_ROTL);
	}
	#define	one128(N,J)	sbSet(N,i,J,L0[i+J])
	#define	sb128(N) {						\
		BYTE *qq=bigTab[N][b##N(sKey[1])];	\
		Xor256(L0,qq,b##N(sKey[0]));		\
		for (i=0;i<256;i+=2) { one128(N,0); one128(N,1); } }
	sb128(0); sb128(1); sb128(2); sb128(3);


	if (direction == DIR_ENCRYPT)	
		ReverseRoundSubkeys(&pKey->TheKey,DIR_ENCRYPT);
	memcpy( pKey->TheCipher.iv32, pKeyData, 16 );
	pKey->TheCipher.iv32[0] ^= 0x19590212; 
	pKey->TheCipher.iv32[1] ^= 0x19600121;
	pKey->TheCipher.iv32[2] ^= 0x19820622;
	pKey->TheCipher.iv32[3] ^= 0x19860102;
	return(0);
}

static int ExaCryptEncrypt( LPPSSKEY pKey, BYTE *input, DWORD Len, BYTE *outBuffer )
{
	DWORD   n;							 
	DWORD x[BLOCK_SIZE/32];	 
	DWORD t0,t1;						 
	int	  rounds=pKey->TheKey.numRounds;	

	DWORD sk[TOTAL_SUBKEYS];
	DWORD IV[BLOCK_SIZE/32];


	if (pKey->TheKey.direction != DIR_ENCRYPT)
		ReverseRoundSubkeys(&pKey->TheKey,DIR_ENCRYPT);

	memcpy(sk,pKey->TheKey.subKeys,sizeof(DWORD)*(ROUND_SUBKEYS+2*rounds));
	BlockCopy(IV,pKey->TheCipher.iv32)

	for (n=0;n<Len;n+=BLOCK_SIZE,input+=BLOCK_SIZE/8,outBuffer+=BLOCK_SIZE/8)
	{
#define	LoadBlockE(N)  x[N]=Bswap(((DWORD *)input)[N]) ^ sk[INPUT_WHITEN+N] ^ IV[N]
		LoadBlockE(0);	LoadBlockE(1);	LoadBlockE(2);	LoadBlockE(3);
#define	EncryptRound(K,R,id)	\
			t0	   = Fe32##id(x[K  ],0);					\
			t1	   = Fe32##id(x[K^1],3);					\
			x[K^3] = ROL(x[K^3],1);							\
			x[K^2]^= t0 +   t1 + sk[ROUND_SUBKEYS+2*(R)  ];	\
			x[K^3]^= t0 + 2*t1 + sk[ROUND_SUBKEYS+2*(R)+1];	\
			x[K^2] = ROR(x[K^2],1);
#define		Encrypt2(R,id)	{ EncryptRound(0,R+1,id); EncryptRound(2,R,id); }

		Encrypt2(14,_);
		Encrypt2(12,_);
		Encrypt2(10,_);
		Encrypt2( 8,_);
		Encrypt2( 6,_);
		Encrypt2( 4,_);
		Encrypt2( 2,_);
		Encrypt2( 0,_);

#if LittleEndian
#define	StoreBlockE(N)	((DWORD *)outBuffer)[N]=x[N^2] ^ sk[OUTPUT_WHITEN+N]
#else
#define	StoreBlockE(N)	{ t0=x[N^2] ^ sk[OUTPUT_WHITEN+N]; ((DWORD *)outBuffer)[N]=Bswap(t0); }
#endif
		StoreBlockE(0);	StoreBlockE(1);	StoreBlockE(2);	StoreBlockE(3);
		IV[0]=Bswap(((DWORD *)outBuffer)[0]);
		IV[1]=Bswap(((DWORD *)outBuffer)[1]);
		IV[2]=Bswap(((DWORD *)outBuffer)[2]);
		IV[3]=Bswap(((DWORD *)outBuffer)[3]);
	}

	BlockCopy(pKey->TheCipher.iv32,IV);

	return 0;
}

static int ExaCryptDecrypt( LPPSSKEY pKey, BYTE *input, DWORD Len, BYTE *outBuffer )
{
	DWORD n;							 
	DWORD x[BLOCK_SIZE/32];
	DWORD t0,t1;					 
	int	  rounds=pKey->TheKey.numRounds;

	DWORD sk[TOTAL_SUBKEYS];
	DWORD IV[BLOCK_SIZE/32];

	if (pKey->TheKey.direction != DIR_DECRYPT)
		ReverseRoundSubkeys(&pKey->TheKey,DIR_DECRYPT);
	memcpy(sk,pKey->TheKey.subKeys,sizeof(DWORD)*(ROUND_SUBKEYS+2*rounds));
	BlockCopy(IV,pKey->TheCipher.iv32)

	for (n=0;n<Len;n+=BLOCK_SIZE,input+=BLOCK_SIZE/8,outBuffer+=BLOCK_SIZE/8)
	{
#define LoadBlockD(N) x[N^2]=Bswap(((DWORD *)input)[N]) ^ sk[OUTPUT_WHITEN+N]
		LoadBlockD(0);	LoadBlockD(1);	LoadBlockD(2);	LoadBlockD(3);

#define	DecryptRound(K,R,id)								\
			t0	   = Fe32##id(x[K  ],0);					\
			t1	   = Fe32##id(x[K^1],3);					\
			x[K^2] = ROL (x[K^2],1);							\
			x[K^2]^= t0 +   t1 + sk[ROUND_SUBKEYS+2*(R)  ];	\
			x[K^3]^= t0 + 2*t1 + sk[ROUND_SUBKEYS+2*(R)+1];	\
			x[K^3] = ROR (x[K^3],1);

#define		Decrypt2(R,id)	{ DecryptRound(2,R+1,id); DecryptRound(0,R,id); }

		Decrypt2(14,_);
		Decrypt2(12,_);
		Decrypt2(10,_);
		Decrypt2( 8,_);
		Decrypt2( 6,_);
		Decrypt2( 4,_);
		Decrypt2( 2,_);
		Decrypt2( 0,_);
#define	StoreBlockD(N)	x[N]   ^= sk[INPUT_WHITEN+N] ^ IV[N];	\
						IV[N]   = Bswap(((DWORD *)input)[N]);	\
						((DWORD *)outBuffer)[N] = Bswap(x[N]);
		StoreBlockD(0);	StoreBlockD(1);	StoreBlockD(2);	StoreBlockD(3);
#undef  StoreBlockD
	}
	BlockCopy(pKey->TheCipher.iv32,IV)

	return 0;
}
# endif
