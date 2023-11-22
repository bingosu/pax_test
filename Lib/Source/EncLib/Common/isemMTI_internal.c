#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ICL_log.h"
#include "ICL_inicrypto.h"

#include "isemMTI.h"
#include "isemMTI_internal.h"
#include "isemMTI_error.h"
#include "isemMTI_version.h"

int i_NTOHL(int x){
#ifdef B_ENDIAN
	return x;
#else
	char buf[4];
	char tobuf[4];
	int toint;

	memcpy(buf, &x, 4);
	tobuf[0] = buf[3];
	tobuf[1] = buf[2];
	tobuf[2] = buf[1];
	tobuf[3] = buf[0];

	memcpy(&toint, tobuf, 4);
	return toint;
#endif
}

int i_HTONL(int x){
#ifdef B_ENDIAN
	return x;
#else
	return i_NTOHL(x);
#endif
}

void I2OS(unsigned char *os, int i)
{
	os[0] = (unsigned char)((i) >> 24);
	os[1] = (unsigned char)((i) >> 16);
	os[2] = (unsigned char)((i) >>  8);
	os[3] = (unsigned char)(i);
}

int isem_make_random(int rand_len, unsigned char *random)
{

	int ret = ISEM_OK;

	if((random == NULL) || (rand_len <= 0)){
		return ISEM_ERR_IN_NULL;
	}

	ret = ICL_DRBG_Get_Random(random, rand_len);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_make_random-ICL_DRBG_Get_Random ret:%d", ret);
		return ISEM_ERR_GEN_RANDOM;
	}

	return ret;
}

int isem_make_ms(ISEM_MTI_CTX *ctx, unsigned char *ms, int *ms_len)
{
	int ret = ISEM_OK;

	unsigned char *out = NULL;
	int outl = 0;
	unsigned char inbuf[ISEM_RANDOM_LENGTH+ISEM_RANDOM_LENGTH+1] = {0x00,};
	int inbuf_len = 0;

	if((ctx->cr == NULL) || (ctx->sr == NULL)){
		return ISEM_ERR_IN_NULL;
	} 

	if(ms == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	out = ms;
	memset(out, 0x00, ISEM_RANDOM_LENGTH);
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "isem_make_ms-ctx->cr", ctx->cr, ISEM_RANDOM_LENGTH);
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "isem_make_ms-ctx->sr", ctx->sr, ISEM_RANDOM_LENGTH);

	/* MS 를 Hash 하여 생성 */
	memcpy(inbuf, ctx->cr, ISEM_RANDOM_LENGTH);
	memcpy(inbuf+ISEM_RANDOM_LENGTH, ctx->sr, ISEM_RANDOM_LENGTH);
	inbuf_len = ISEM_RANDOM_LENGTH + ISEM_RANDOM_LENGTH;

	ret = ICL_HASH_Data_F((unsigned char *)inbuf, inbuf_len, out, &outl, "SHA256");
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_make_ms-ICL_HASH_Data_F ret:%d", ret);
		return ret;
	}
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "isem_make_ms-ms", out, outl);

	*ms_len = outl;

	memset(inbuf, 0x00, sizeof(inbuf));

	return ret;
}

int isem_sec_encode(unsigned char *sec, int secl, unsigned char *in, int inl, unsigned char *out, int *outl, int type)
{
	int ret = ISEM_OK;
	unsigned char *tmp = NULL;
	int tmpl = 0;
	int i = 0;
	int j = 0;

	if((sec == NULL) || (secl <= 0) || (in == NULL) || (inl <= 0)){
		return ISEM_ERR_IN_NULL;
	}

	if((out == NULL) || (outl == NULL)){
		return ISEM_ERR_OUT_NULL;
	}

	if((type == 0) || (type == 1)){
		tmpl = inl;
		tmp = out;
		memset(tmp, 0x00, tmpl);

		i = j = 0;
		for(; i < inl; i++, j++){
			if( j >= secl){
				j = 0;
			}
			tmp[i] = in[i]^sec[j];
		}
		*outl = tmpl;
	}

	return ret;
}

int ICL_HASH_KDF(unsigned char *sec_data, int sec_data_len, unsigned char *etc_data, int etc_data_len, char *hash_alg, unsigned char *out, int *out_len)
{

	int ret = ISEM_OK;
	int i = 0;
	char c[4+1] = {0x00,};
	int num = 0;
	int n = 0;
	int rem = 0;

	unsigned char indata[ISEM_BUF_LENGTH+1] = {0x00,};
	unsigned char hash_data[ISEM_HASH_LENGTH+1] = {0x00,};
	int hash_data_len = 0;

	if( (sec_data == NULL) || (sec_data_len <= 0) || (etc_data == NULL) || (etc_data_len <= 0) ){
		return ISEM_ERR_IN_NULL;
	}
	if( (out == NULL) || (out_len == NULL)){
		return ISEM_ERR_OUT_NULL;
	}
	/*
	   1) n을 [유도하고자 하는 키 길이/해쉬코드의 길이]로 놓는다.
	 */
	n = (*out_len)/ISEM_HASH_LENGTH;
	rem = (*out_len)%ISEM_HASH_LENGTH;
	if(rem) n++;

	/*
	   2) 카운터 i를 1부터 n까지 1씩 증가시키며 다음과 같은 동작을 반복한다.
	   2.1) 해쉬함수의 입력 : "카운터 i || 비밀값 || 그 밖의 정보"
	   2.2) 해쉬함수 적용
	   2.3) 출력된 해쉬코드를 해쉬코드 i로 놓는다.
	 */
	for(i = 1; i < n; i++){
		memset(indata, 0x00, sizeof(indata));
		memset(c, 0x00, sizeof(c));
		I2OS((unsigned char *)&c, i);
		memcpy(indata, c, 4);
		memcpy(indata+4, sec_data, sec_data_len);
		memcpy(indata+4+sec_data_len, etc_data, etc_data_len);

		ret = ICL_HASH_Data_F( indata, 4+sec_data_len+etc_data_len, hash_data, &hash_data_len, hash_alg);
		if(ret){
			ICL_Log(ICL_ERROR, ISEM_FL, "ICL_HASH_KDF-ICL_HASH_Data_F ret:%d", ret);
			ret = ISEM_ERR_HASH_KDF;
			memset(indata, 0x00, sizeof(indata));
			return ret;
		}
		/*
		   3) 출력한 해쉬코드들을 다음과 같이 연접하여 유도하고자 하는 키로 이용한다.
		   키 = 해쉬코드1 || 해쉬코드2 || ... || 해쉬코드n
		 */
		memcpy(out + num, hash_data, ISEM_HASH_LENGTH);
		num += ISEM_HASH_LENGTH;
	} 

	I2OS((unsigned char *)&c, i);

	memcpy(indata, c, 4);
	memcpy(indata+4, sec_data, sec_data_len);
	memcpy(indata+4+sec_data_len, etc_data, etc_data_len);

	ret = ICL_HASH_Data_F(indata, 4+sec_data_len+etc_data_len, hash_data, &hash_data_len, hash_alg);
	if(ret){
		ICL_Log(ICL_ERROR, ISEM_FL, "ICL_HASH_KDF-ICL_HASH_Data_F ret:%d", ret);
		ret = ISEM_ERR_HASH_KDF;
		memset(indata, 0x00, sizeof(indata));
		memset(c, 0x00, sizeof(c));
		return ret;
	}

	if(rem){
		memcpy(out + num, hash_data, rem);
		num += rem;
	}
	else{
		memcpy(out + num, hash_data, ISEM_HASH_LENGTH);
		num += ISEM_HASH_LENGTH;
	}
	*out_len = num;

	memset(indata, 0x00, sizeof(indata));
	memset(c, 0x00, sizeof(c));

	return ret;
}
