#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "isemMTI.h"
#include "isemMTI_cmac.h"
#include "isemMTI_error.h"
#include "ICL_inicrypto.h"
#include "libDebug.h"

unsigned char isem_MTI_const_Rb[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b };
unsigned char isem_MTI_const_Zero[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void isem_MTI_xor_64(unsigned char *a, unsigned char *b, unsigned char *out)
{
	int i;

	for (i = 0; i < 8; i++) {
		out[i] = a[i] ^ b[i];
	}
}

/* AES-CMAC Generation Function */
void isem_MTI_leftshift_onebit(unsigned char *input, unsigned char *output)
{
	int i;
	unsigned char overflow = 0;

	for ( i = 7; i >= 0; i-- ) {
		output[i] = input[i] << 1;
		output[i] |= overflow;
		overflow = (input[i] & 0x80)?1:0;
	}
}

void isem_MTI_generate_subkey(unsigned char *key, unsigned char *K1, unsigned char *K2)
{   
	unsigned char L[8+1] = {0x00,};
	unsigned char tmp[8];
	int out_len;

	/* T-DES */
	ICL_SYM_Encrypt_F(key, NULL, "DES_EDE-ECB", ICL_NO_PAD, isem_MTI_const_Zero, 8, L, &out_len);

	/* Make K1 start */
	/* If MSB(L) = 0, then K1 = L << 1 */
	if ( (L[0] & 0x80) == 0 ) {

		isem_MTI_leftshift_onebit(L,K1);
	}
	/* Else K1 = ( L << 1 ) (+) Rb */
	else {

		isem_MTI_leftshift_onebit(L,tmp);
		isem_MTI_xor_64(tmp,isem_MTI_const_Rb,K1);
	}
	/* Make K1 end */

	/* Make K2 start */
	if ( (K1[0] & 0x80) == 0 ) {

		isem_MTI_leftshift_onebit(K1,K2);
	}
	else {

		isem_MTI_leftshift_onebit(K1,tmp);
		isem_MTI_xor_64(tmp,isem_MTI_const_Rb,K2);
	}
	/* Make K2 end */
}

void isem_MTI_padding(unsigned char *lastb, unsigned char *pad, int length)
{
	int j;

	/* original last block */
	for ( j=0; j<16; j++ ) {
		if ( j < length ) {
			pad[j] = lastb[j];
		}
		else
			if ( j == length ) {
				pad[j] = 0x80;
			}
			else {
				pad[j] = 0x00;
			}
	}
}

int isem_MTI_CMAC_init(ISEM_CMAC_CTX *ctx, unsigned char *key, int key_len)
{
	int ret = ISEM_OK;
	unsigned char key3[24]= {0x00,};

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(key == NULL || key_len <= 0){
		return ISEM_ERR_IN_NULL;
	}

	memcpy(key3, key, 16);
	memcpy(key3+16, key, 8);

	memset(ctx, 0x00, sizeof(ISEM_CMAC_CTX));
	memcpy(ctx->key, key3, 24);
	isem_MTI_generate_subkey(key3, ctx->K1, ctx->K2);

	return ret;
}

void isem_MTI_CMAC(ISEM_CMAC_CTX *ctx, unsigned char *in, int inlen, unsigned char *out, int *outlen)
{
	unsigned char Y[8+1] = {0x00,};
	unsigned char M_last[8+1] = {0x00,};
	unsigned char padded[8+1] = {0x00,};
	int n, i, flag = 0;

	n = (inlen+7)/8;

	if( n == 0 ){
		n = 1;
		flag = 0;
	}
	else{
		if( (inlen%8) == 0 ){
			flag = 1;
		}
		else{
			flag = 0;
		}
	}

	if(flag){
		isem_MTI_xor_64(&in[8*(n-1)], ctx->K1, M_last);
	}
	else{
		isem_MTI_padding ( &in[8*(n-1)], padded, inlen%8 );
		isem_MTI_xor_64(in, ctx->K2, M_last);
	}

	for( i = 0; i < 8; i++)
		out[i] = 0;
	for( i = 0; i < n-1; i++){
		isem_MTI_xor_64(out, &in[8*i], Y);
		ICL_SYM_Encrypt_F(ctx->key, NULL, "DES_EDE-ECB", ICL_NO_PAD, Y, 8, out, outlen);
	}

	isem_MTI_xor_64(out, M_last, Y);
	ICL_SYM_Encrypt_F(ctx->key, NULL, "DES_EDE-ECB", ICL_NO_PAD, Y, 8, out, outlen);
}

void isem_MTI_CMAC_deriveEncKey(ISEM_CMAC_CTX *ctx, unsigned char *enckey, int *enckey_len)
{
	int out_len=0;
	unsigned char first_enckey_indata  [8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };
	unsigned char second_enckey_indata [8] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };

	isem_MTI_CMAC(ctx, first_enckey_indata, 8, enckey, &out_len);
	*enckey_len = out_len;
	isem_MTI_CMAC(ctx, second_enckey_indata, 8, enckey+8, &out_len);
	*enckey_len += out_len;
}

void isem_MTI_CMAC_deriveMacKey(ISEM_CMAC_CTX *ctx, unsigned char *mackey, int *mackey_len)
{
	int out_len=0;
	unsigned char first_mackey_indata  [8] = { 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80 };
	unsigned char second_mackey_indata [8] = { 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80 };

	isem_MTI_CMAC(ctx, first_mackey_indata, 8, mackey, &out_len);
	*mackey_len = out_len;
	isem_MTI_CMAC(ctx, second_mackey_indata, 8,  mackey+8, &out_len);
	*mackey_len += out_len;
}

int ISEM_MTI_CMAC_DeriveKey(unsigned char *key, int key_len, unsigned char *enckey, int *enckey_len, unsigned char *mackey, int *mackey_len)
{
	int ret = ISEM_OK;

	ISEM_CMAC_CTX ctx;

	if(key == NULL || key_len <= 0){
		return ISEM_ERR_IN_NULL;
	}

	if(enckey == NULL || enckey_len == NULL || mackey == NULL || mackey_len == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	isem_MTI_CMAC_init(&ctx, key, key_len);

	dbuf("-@@- ISEM_MTI_CMAC_DeriveKey : ctx.K1", ctx.K1, 8+1);
	dbuf("-@@- ISEM_MTI_CMAC_DeriveKey : ctx.K2", ctx.K2, 8+1);
	if(enckey && enckey_len){
		isem_MTI_CMAC_deriveEncKey(&ctx, enckey, enckey_len);
	}

	if(mackey && mackey_len){
		isem_MTI_CMAC_deriveMacKey(&ctx, mackey, mackey_len);
	}

	return ret;
}
