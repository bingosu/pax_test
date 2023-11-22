#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libMtiCommonApi.h"

#include "ICL_log.h"
#include "ICL_inicrypto.h"

#include "isemMTI.h"
#include "isemMTI_client.h"
#include "isemMTI_dukpt.h"
#include "isemMTI_error.h"
#include "isemMTI_internal.h"

#ifndef _ISEM_CLIENT
#include "ICL_string.h"
#include "isemMTI_server.h"
#endif

ISEM_MTI_CONF_INFO conf_info;

/* 2017-02-28 INITECH Add START */
static const unsigned char DES_odd_parity_table[128] = { 1,  2,  4,  7,  8,
        11, 13, 14, 16, 19, 21, 22, 25, 26, 28, 31, 32, 35, 37, 38, 41, 42, 44,
        47, 49, 50, 52, 55, 56, 59, 61, 62, 64, 67, 69, 70, 73, 74, 76, 79, 81,
        82, 84, 87, 88, 91, 93, 94, 97, 98, 100, 103, 104, 107, 109, 110, 112,
        115, 117, 118, 121, 122, 124, 127, 128, 131, 133, 134, 137, 138, 140,
        143, 145, 146, 148, 151, 152, 155, 157, 158, 161, 162, 164, 167, 168,
        171, 173, 174, 176, 179, 181, 182, 185, 186, 188, 191, 193, 194, 196,
        199, 200, 203, 205, 206, 208, 211, 213, 214, 217, 218, 220, 223, 224,
        227, 229, 230, 233, 234, 236, 239, 241, 242, 244, 247, 248, 251, 253,
        254 };
 
void ISEM_Des_key_set_parity(unsigned char* key_data, int key_data_len)
{
        int offset = 0;
        int index  = 0;
 
        if( key_data == NULL )  return;
 
        for(index = 0; index < (key_data_len/8); index++){
                for(offset = 0; offset < 8; offset++){
                        key_data[(index*8) + offset] = DES_odd_parity_table[key_data[(index*8) + offset] / 2 ];
                }
        }
}
/* 2017-02-28 INITECH Add END */

int ISEM_MTI_Init(char *conf)
{
	int ret = ISEM_OK;

#ifndef _ISEM_CLIENT
	ICL_COM_Change_Non_Proven();
#endif

	if(conf == NULL){
#ifdef _ISEM_CLIENT
		memset(conf_info.sym_alg, 0x00, sizeof(conf_info.sym_alg));
		memset(conf_info.hash_alg, 0x00, sizeof(conf_info.hash_alg));
		//@@CACING PENTEST strcpy(conf_info.sym_alg, ISEM_SYM_ALG);
		//@@CACING PENTEST strcpy(conf_info.hash_alg, ISEM_HASH_ALG);
		strncpy(conf_info.sym_alg, ISEM_SYM_ALG,(int)strlen(ISEM_SYM_ALG));
		strncpy(conf_info.hash_alg, ISEM_HASH_ALG,(int)strlen(ISEM_HASH_ALG));
#else
		ret = ISEM_INIT_CONF_NULL;
		return ret;
#endif
	}else{
#ifndef _ISEM_CLIENT
		ret = isem_loadconf(conf);
		if(ret != ISEM_OK){
			ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_MTI_Init-isem_loadconf ret:%d", ret);
			return ret;
		}
#endif
	}
	memcpy(conf_info.sec_seed, ISEM_SEED_VALUE, ISEM_SEC_SEED_LENGTH);
	conf_info.sec_seed_len = ISEM_SEC_SEED_LENGTH;

	return ret;
}

int ISEM_MTI_Ctx_New(char type, ISEM_MTI_CTX *ctx)
{
	int ret = ISEM_OK;

	memset(ctx, 0x00, sizeof(ISEM_MTI_CTX));

	if(type == ISEM_CLIENT_CTX){
		ret = ISEM_Ctx_Client_New(ctx);
		if(ret){
			return ret;
		}
	}
	else{
#ifndef _ISEM_CLIENT
		ret = ISEM_Ctx_Server_New(ctx);
		if(ret){
			ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Ctx_Init-ISEM_Ctx_Server_New ret:%d", ret);
			return ret;
		}
#endif
	}

	return ret;
}

void ISEM_MTI_Ctx_Free(ISEM_MTI_CTX *ctx)
{
	if(ctx){
		memset(ctx, 0x00, sizeof(ISEM_MTI_CTX));
	}
}

void ISEM_MTI_Clean(void)
{
	memset(&conf_info, 0x00, sizeof(ISEM_MTI_CONF_INFO));
}

#ifndef _ISEM_CLIENT
void ISEM_Hexa_Dump(FILE *fp, unsigned char *msg, int msg_len)
{
	ICL_HexaDump(fp, msg, msg_len);
}
#else 
void ISEM_Hexa_Dump(char *outbuf, int *outbuf_len, unsigned char *msg, int msg_len)
{
	ICL_HexaDump(outbuf, outbuf_len, msg, msg_len);
}
#endif

int ISEM_Set_HashAlg(char *hash_alg)
{
	int ret = ISEM_OK;

	if(hash_alg == NULL){
		return ret;
	}

	memset(conf_info.hash_alg, 0x00, sizeof(conf_info.hash_alg));
	strncpy(conf_info.hash_alg, hash_alg,strlen(hash_alg));

	return ret;
}

int ISEM_Set_SymAlg(char *sym_alg)
{
	int ret = ISEM_OK;

	if(sym_alg == NULL){
		return ret;
	}

	memset(conf_info.sym_alg, 0x00, sizeof(conf_info.sym_alg));
	strncpy(conf_info.sym_alg, sym_alg,strlen(sym_alg));

	return ret;
}

int ISEM_MTI_Init_DUKPT(char *ksn, unsigned char *ipek)
{
	int ret = ISEM_OK;

	if(ksn == NULL || ipek == 0){
		return ISEM_ERR_IN_NULL;
	}

	memcpy(id_ksn, ksn, ISEM_KSN_LENGTH);

	ret = isem_dukpt_load_initial_key(ipek, ISEM_DUKPTKEY_LENGTH, ksn, ISEM_KSN_LENGTH);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_MTI_Init_DUKPT-isem_dukpt_load_initial_key ret:%d", ret);
		return ret;
	}

	return ret;
}

int ISEM_Dukpt_Encrypt_GenKey(ISEM_MTI_CTX *ctx)
{
	int ret = ISEM_OK;

	unsigned char ck[ISEM_KEYGEN_LENGTH+1];
	int ck_len = 0;

	unsigned char dek[ISEM_KEYGEN_LENGTH+1] = {0x00,};
	unsigned char pek[ISEM_KEYGEN_LENGTH+1] = {0x00,};
	unsigned char mack[ISEM_KEYGEN_LENGTH+1] = {0x00,};

	int enc_count = 0;

	ret = isem_dukpt_requestPINentry(ck, &ck_len,  &enc_count);
	if(ret != 0){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Dukpt_Encrypt_GenKey-isem_dukpt_requestPINentry ret:%d", ret);
		return ret;
	}
	ICL_Log(ICL_DEBUG, ISEM_FL, "ISEM_Dukpt_Encrypt_GenKey enc_count(%d)", enc_count);

	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "CurrentKey", ck, ck_len);

	ret = isem_dukpt_deriveDEK(ck, dek);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Dukpt_Encrypt_GenKey-isem_dukpt_deriveDEK ret:%d", ret);
		memset(ck, 0x00, sizeof(ck));
		return ret;
	}
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "DEK", dek, ISEM_KEY_LENGTH);
	memcpy(ctx->dek, dek, ISEM_KEY_LENGTH);

	ret = isem_dukpt_derivePEK(ck, pek);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Dukpt_Encrypt_GenKey-isem_dukpt_derivePEK ret:%d", ret);
		memset(dek, 0x00, sizeof(dek));
		memset(ck, 0x00, sizeof(ck));
		return ret;
	}
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "PEK", pek, ISEM_KEY_LENGTH);
	memcpy(ctx->pek, pek, ISEM_KEY_LENGTH);

	ret = isem_dukpt_deriveMACK(ck, mack);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Dukpt_Encrypt_GenKey-isem_dukpt_deriveMACK ret:%d", ret);
		memset(dek, 0x00, sizeof(dek));
		memset(ck, 0x00, sizeof(ck));
		memset(pek, 0x00, sizeof(pek));
		return ret;

	}
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "MACK", mack, ISEM_KEY_LENGTH);
	memcpy(ctx->mackey, mack, ISEM_KEY_LENGTH);

	memcpy(ctx->ksn, id_ksn, ISEM_KSN_LENGTH);
	isem_dukpt_setCounter(ctx->ksn, enc_count);

	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "KSN", ctx->ksn, ISEM_KSN_LENGTH);

	memset(ck, 0x00, sizeof(ck));
	memset(dek, 0x00, sizeof(dek));
	memset(pek, 0x00, sizeof(pek));
	memset(mack, 0x00, sizeof(mack));

	return ret;
}

int ISEM_Encrypt_Data(ISEM_MTI_CTX *ctx, unsigned char *msg, int msg_len, unsigned char *out, int *out_len)
{
	int ret = ISEM_OK;

	unsigned char enc_buf[ISEM_ENC_LENGTH] = {0x00,};
	int enc_bufsize = 0;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(msg == NULL || msg_len <= 0){
		return ISEM_ERR_IN_NULL;
	}

	if(out == NULL || out_len == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	ICL_Log(ICL_DEBUG, ISEM_FL, "ISEM_Encrypt_Data start");
	dbuf("-dek-", ctx->dek, ISEM_KEY_LENGTH);
	/* Encrypt */
	ret = ICL_SYM_Encrypt_F(ctx->dek, (unsigned char *)ISEM_DEF_IV_VALUE, ctx->sym_alg, ICL_NO_PAD, msg, msg_len, enc_buf, &enc_bufsize);
	if (ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Encrypt_Data-ICL_SYM_Encrypt_F ret:%08X", ret);
		return ret;
	}

	/* KSN(10)||Edek(data) */
	memcpy(out, ctx->ksn, ISEM_KSN_LENGTH);
	memcpy(out+ISEM_KSN_LENGTH, enc_buf, enc_bufsize);
	*out_len = ISEM_KSN_LENGTH+enc_bufsize;

	memset(enc_buf, 0x00, sizeof(enc_buf));

	return  ret;

}

int ISEM_Decrypt_Data(ISEM_MTI_CTX *ctx, char *ksn, unsigned char *ct, int ct_len, unsigned char *pt, int *pt_len)
{
	int ret = ISEM_OK;
	unsigned char ck[ISEM_KEYGEN_LENGTH+1] = {0x00,};
	int ck_len = 0;
	unsigned char dek[ISEM_KEYGEN_LENGTH+1] = {0x00,};
	unsigned char mack[ISEM_KEYGEN_LENGTH+1] = {0x00,};
	int enc_count = 0;

	unsigned char ksn_tmp[ISEM_KSN_LENGTH] = {0x00,};

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(ct == NULL || ct_len <= 0){
		return ISEM_ERR_IN_NULL;
	}

	if(pt == NULL || pt_len == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	ret = isem_dukpt_requestPINentry(ck, &ck_len,  &enc_count);
	if(ret != 0){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Decrypt_Data-isem_dukpt_requestPINentry ret:%d", ret);
		return ret;
	}
	ICL_Log(ICL_DEBUG, ISEM_FL, "ISEM_Decrypt_Data enc_count(%d)", enc_count);

	ICL_Log(ICL_DEBUG, ISEM_FL, "ISEM_Decrypt_Data start");
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "CurrentKey", ck, ck_len);

	ret = isem_dukpt_deriveDEK(ck, dek);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Encrypt_Data-isem_dukpt_deriveDEK ret:%d", ret);
		memset(ck, 0x00, sizeof(ck));
		return ret;
	}
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "DEK", dek, ISEM_KEY_LENGTH);

	ret = isem_dukpt_deriveMACK(ck, mack);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Dukpt_Encrypt_GenKey-isem_dukpt_deriveMACK ret:%d", ret);
		memset(dek, 0x00, sizeof(dek));
		memset(ck, 0x00, sizeof(ck));
		return ret;
	}
	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "MACK", mack, ISEM_KEY_LENGTH);
	memcpy(ctx->mackey, mack, ISEM_KEY_LENGTH);

	memcpy(ksn_tmp, id_ksn, ISEM_KSN_LENGTH);
	isem_dukpt_setCounter((unsigned char *)ksn_tmp, enc_count);

	if(ksn != NULL){
		if(memcmp(ksn_tmp, ksn, ISEM_KSN_LENGTH)){
			/* */
		}
	}

	ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "KSN", (unsigned char *)ksn_tmp, ISEM_KSN_LENGTH);

	ret = ICL_SYM_Decrypt_F(dek, (unsigned char *)ISEM_DEF_IV_VALUE, ctx->sym_alg, ICL_NO_PAD, ct, ct_len, pt, pt_len);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Decrypt_Data-ICL_SYM_Decrypt_F ret:%08X", ret);
		memset(dek, 0x00, sizeof(dek));
		memset(ck, 0x00, sizeof(ck));
		memset(mack, 0x00, sizeof(mack));
		memset(ctx->mackey, 0x00, ISEM_KEY_LENGTH);
		return ret;
	}

	memset(ck, 0x00, sizeof(ck));
	memset(dek, 0x00, sizeof(dek));
	memset(mack, 0x00, sizeof(mack));

	return ret;
}

int ISEM_Encrypt_Data_PIN(ISEM_MTI_CTX *ctx, unsigned char *msg, int msg_len, unsigned char *out, int *out_len)
{
	int ret = ISEM_OK;

	unsigned char enc_buf[ISEM_ENC_LENGTH] = {0x00,};
	int enc_bufsize = 0;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(msg == NULL || msg_len <= 0){
		return ISEM_ERR_IN_NULL;
	}

	if(out == NULL || out_len == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	ICL_Log(ICL_DEBUG, ISEM_FL, "ISEM_Encrypt_Data_PIN start");

	/* Encrypt */
	ret = ICL_SYM_Encrypt_F(ctx->pek, (unsigned char *)ISEM_DEF_IV_VALUE, ctx->sym_alg, ICL_NO_PAD, msg, msg_len, enc_buf, &enc_bufsize);
	if (ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_Encrypt_Data_PIN-ICL_SYM_Encrypt_F ret:%08X", ret);
		return ret;
	}

	/* KSN(10)||Epek(data) */
	memcpy(out, ctx->ksn, ISEM_KSN_LENGTH);
	memcpy(out+ISEM_KSN_LENGTH, enc_buf, enc_bufsize);
	*out_len = ISEM_KSN_LENGTH+enc_bufsize;

	memset(enc_buf, 0x00, sizeof(enc_buf));

	return  ret;

}

int ISEM_CBC_MAC(ISEM_MTI_CTX *ctx, int algo_id, unsigned char *in, int inl,  unsigned char *key, unsigned char *out)
{
	int ret = ISEM_OK;

	unsigned char mackey[ISEM_KEY_LENGTH] = {0x00,};

	if(in == NULL || inl <= 0){
		return ISEM_ERR_IN_NULL;
	}

	if(out == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	if(key == NULL){
		memcpy(mackey, ctx->mackey, ISEM_KEY_LENGTH);
	}else{
		memcpy(mackey, key, ISEM_KEY_LENGTH);
	}

	ret = ICL_MAC_CBC(algo_id, in, inl, mackey, out);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "ISEM_CBC_MAC-ICL_MAC_CBC ret:%08X", ret);
		return ISEM_ERR_CBC_MAC;
	}

	memset(mackey, 0x00, sizeof(mackey));
	return ret;
}
