#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ICL_inicrypto.h"
#ifndef _ISEM_CLIENT
#include "ICL_string.h"
#include "ICL_log.h"
#endif

#include "isemMTI.h"
#include "isemMTI_client.h"
#include "isemMTI_cmac.h"
#include "isemMTI_internal.h"
#include "isemMTI_error.h"
#include "libDebug.h"

#ifdef _ISEM_CLIENT
int ISEM_Mutual_Auth_Request_MS1(ISEM_MTI_CTX *ctx, char *pubidx, unsigned char *serial, unsigned char *mod, int modl, unsigned char *exp, int expl, unsigned char *out, int *outl)
#else
int ISEM_Mutual_Auth_Request_MS1(ISEM_MTI_CTX *ctx, char *pubidx, unsigned char *serial, unsigned char *pubkey, int pubkeyl, unsigned char *out, int *outl)
#endif
{
	int ret = ISEM_OK;

	unsigned char buf[ISEM_BUF_LENGTH] = {0x00,};
	int bufl = 0;
	unsigned char hash[ISEM_HASH_LENGTH] = {0x00,};
	int hashl = 0;
	unsigned char enc[ISEM_ENC_LENGTH] = {0x00,};
	int encl = 0;
	unsigned char *penc = NULL;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

#ifdef _ISEM_CLIENT
	if(pubidx == NULL || serial == NULL || mod == NULL || modl <= 0 || exp == NULL || expl <= 0){
#else
	if(pubidx == NULL || serial == NULL || pubkey == NULL || pubkeyl <= 0){
#endif
		return ISEM_ERR_IN_NULL;
	}

	if(out == NULL || outl == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	/* CTX 구조체에 복사 */
	memset(ctx->serial, 0x00, sizeof(ctx->serial));
	memset(ctx->pubidx, 0x00, sizeof(ctx->pubidx));

	ctx->serial_len = strlen((char *)serial);
	memcpy(ctx->serial, serial, ctx->serial_len);
	ctx->pubidxl = strlen(pubidx);
	memcpy(ctx->pubidx, pubidx, ctx->pubidxl);

#ifdef _ISEM_CLIENT
	memset(ctx->mod, 0x00, sizeof(ctx->mod));
	memset(ctx->exp, 0x00, sizeof(ctx->exp));
	ctx->modl = modl;
	memcpy(ctx->mod, mod, ctx->modl);
	ctx->expl = expl;
	memcpy(ctx->exp, exp, ctx->expl);
#else
	memset(ctx->pubkey, 0x00, sizeof(ctx->pubkey));
	ctx->pubkeyl = pubkeyl;
	memcpy(ctx->pubkey, pubkey, ctx->pubkeyl);
#endif

	/* Cr, Ck 생성 */
	ret = isem_make_random(ISEM_RANDOM_LENGTH, ctx->cr);
	if(ret != ISEM_OK){
#ifdef _ISEM_CLIENT
		memset(ctx->mod, 0x00, sizeof(ctx->mod));
		memset(ctx->exp, 0x00, sizeof(ctx->exp));
#else
		memset(ctx->pubkey, 0x00, sizeof(ctx->pubkey));
#endif

		return ret;
	}
	ret = isem_make_random(ISEM_RANDOM_LENGTH, ctx->ck);
	if(ret != ISEM_OK){
#ifdef _ISEM_CLIENT
		memset(ctx->mod, 0x00, sizeof(ctx->mod));
		memset(ctx->exp, 0x00, sizeof(ctx->exp));
#else
		memset(ctx->pubkey, 0x00, sizeof(ctx->pubkey));
#endif

		return ret;
	}

	/* H(Serial||Ck) 생성 */
	memcpy(buf, ctx->serial, ISEM_SERIAL_LENGTH);
	memcpy(buf+ISEM_SERIAL_LENGTH, ctx->ck, ISEM_RANDOM_LENGTH);
	bufl = ISEM_SERIAL_LENGTH + ISEM_RANDOM_LENGTH;

	ret = ICL_HASH_Data_F(buf, bufl, hash, &hashl, ctx->hash_alg);
	if(ret != ISEM_OK){
#ifdef _ISEM_CLIENT
		memset(ctx->mod   , 0x00, sizeof(ctx->mod));
		memset(ctx->exp   , 0x00, sizeof(ctx->exp));
#else
		memset(ctx->pubkey, 0x00, sizeof(ctx->pubkey));
#endif		
		memset(ctx->serial, 0x00, ISEM_SERIAL_LENGTH);
		memset(ctx->ck    , 0x00, ISEM_RANDOM_LENGTH);
		memset(buf, 0x00, sizeof(buf));
		return ret;
	}

	/* EMpub(Serial||Ck||H(Serial||Ck)) 생성 */
	memcpy(buf+bufl, hash, ISEM_HASH_LENGTH);
	bufl += ISEM_HASH_LENGTH;

	dbuf("sha-src", buf, bufl);

#ifdef _ISEM_CLIENT
	ret = ICL_PK1_Public_Encrypt_ex_C(ctx->mod, ctx->modl, ctx->exp, ctx->expl, ICL_RSAES_PKCS1_15, buf, bufl, enc, &encl, ICL_NO_ENCODE, "SHA1");
	if(ret != ISEM_OK){
		memset(ctx->mod   , 0x00, sizeof(ctx->mod));
		memset(ctx->exp   , 0x00, sizeof(ctx->exp));		
		memset(ctx->serial, 0x00, ISEM_SERIAL_LENGTH);
		memset(ctx->ck    , 0x00, ISEM_RANDOM_LENGTH);
		memset(buf, 0x00, sizeof(buf));
		memset(hash, 0x00, sizeof(hash));
		return ret;
	}
	penc = enc;
#else
	ret = ICL_PK1_Public_Encrypt_ex(ctx->pubkey, ctx->pubkeyl, ICL_RSAES_PKCS1_15, buf, bufl, &penc, &encl, ICL_NO_ENCODE, "SHA1");
	if(ret != ISEM_OK){
		memset(ctx->pubkey, 0x00, sizeof(ctx->pubkey));
		memset(ctx->serial, 0x00, ISEM_SERIAL_LENGTH);
		memset(ctx->ck    , 0x00, ISEM_RANDOM_LENGTH);	
		memset(buf, 0x00, sizeof(buf));
		memset(hash, 0x00, sizeof(hash));
		return ret;
	}
#endif

	/* 전문 생성 */
	/* pubidx||Cr||EMpub(Serial||Ck||H(Serial||Ck)) */
	memcpy(out, ctx->pubidx, ISEM_PUBIDX_LENGTH);
	memcpy(out+ISEM_PUBIDX_LENGTH, ctx->cr, ISEM_RANDOM_LENGTH);
	memcpy(out+ISEM_PUBIDX_LENGTH+ISEM_RANDOM_LENGTH, penc, encl);

	*outl = ISEM_PUBIDX_LENGTH + ISEM_RANDOM_LENGTH + encl;

	memset(buf, 0x00, sizeof(buf));
	memset(hash, 0x00, sizeof(hash));
	memset(enc, 0x00, sizeof(enc));
#ifndef _ISEM_CLIENT
	if(penc){
		ICL_Free(penc, encl);
		penc = NULL;
	}
#endif

	return ret;
}

int ISEM_Mutual_Auth_Handle_MS2(ISEM_MTI_CTX *ctx, unsigned char *in, int inl)
{
	int ret = ISEM_OK;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(in == NULL || inl <= 0){
		return ISEM_ERR_IN_NULL;
	}

	/* Mpub 로 서명 검증 */
#ifdef _ISEM_CLIENT
	ret = ICL_PK1_Public_Verify_C(ctx->mod, ctx->modl, ctx->exp, ctx->expl, ICL_RSASSA_PKCS1_15, "SHA1", in, ISEM_RANDOM_LENGTH+ISEM_RANDOM_LENGTH, in+ISEM_RANDOM_LENGTH+ISEM_RANDOM_LENGTH, inl-(ISEM_RANDOM_LENGTH+ISEM_RANDOM_LENGTH), ICL_NO_ENCODE);
	if(ret != ISEM_OK){
		return ret;
	}
#else
	ret = ICL_PK1_Public_Verify(ctx->pubkey, ctx->pubkeyl, ICL_RSASSA_PKCS1_15, "SHA1", in, ISEM_RANDOM_LENGTH+ISEM_RANDOM_LENGTH, in+ISEM_RANDOM_LENGTH+ISEM_RANDOM_LENGTH, inl-(ISEM_RANDOM_LENGTH+ISEM_RANDOM_LENGTH), ICL_NO_ENCODE);
	if(ret != ISEM_OK){
		return ret;
	}
#endif
	/* TMS 인증 */
	if(memcmp(in, ctx->cr, ISEM_RANDOM_LENGTH)){
		return ISEM_ERR_INVALID_CLIENT_RANDOM;
	}

	/* CTX에 Sr 세팅 */
	memcpy(ctx->sr, in+ISEM_RANDOM_LENGTH, ISEM_RANDOM_LENGTH);

	return ret;
}

int ISEM_Key_Exchange_Request_KSN1(ISEM_MTI_CTX *ctx, unsigned char *out, int *outl)
{
	int ret = ISEM_OK;

	unsigned char buf[ISEM_BUF_LENGTH] = {0x00,};
	int bufl = 0;
	unsigned char hash[ISEM_HASH_LENGTH] = {0x00,};
	int hashl = 0;
	unsigned char ms[ISEM_KEYGEN_LENGTH] = {0x00,};
#ifndef _ISEM_TEST
	int msl = 0;
#endif
	unsigned char enc[ISEM_ENC_LENGTH] = {0x00,};
	int encl = 0;
	unsigned char *penc = NULL;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(out == NULL || outl == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	ret = isem_make_ms(ctx, ms, &msl);
	if(ret != ISEM_OK){
		return ret;
	}
	memcpy(ctx->ms, ms, ISEM_KEYGEN_LENGTH);

	/* H(Sr||MS) 생성 */
	memcpy(buf, ctx->sr, ISEM_RANDOM_LENGTH);
	memcpy(buf+ISEM_RANDOM_LENGTH, ctx->ms, ISEM_KEYGEN_LENGTH);
	bufl = ISEM_RANDOM_LENGTH + ISEM_KEYGEN_LENGTH;

	ret = ICL_HASH_Data_F(buf, bufl, hash, &hashl, ctx->hash_alg);
	if(ret != ISEM_OK){
		memset(ms, 0x00, sizeof(ms));
		memset(buf, 0x00, sizeof(buf));
		return ret;
	}

	/* EMpuk(Sr||MS||H(Sr||MS)) */
	memcpy(buf+bufl, hash, ISEM_HASH_LENGTH);
	bufl += ISEM_HASH_LENGTH;

	dbuf("ctx->ms", ctx->ms, ISEM_KEYGEN_LENGTH);
	dbuf("ctx->sr", ctx->sr, ISEM_RANDOM_LENGTH);
	dbuf("**** buf", buf, bufl);
#ifdef _ISEM_CLIENT
	ret = ICL_PK1_Public_Encrypt_ex_C(ctx->mod, ctx->modl, ctx->exp, ctx->expl, ICL_RSAES_PKCS1_15, buf, bufl, enc, &encl, ICL_NO_ENCODE, "SHA1");
	if(ret != ISEM_OK){
		memset(ctx->ms, 0x00, ISEM_KEYGEN_LENGTH);
		memset(hash, 0x00, sizeof(hash));
		memset(ms, 0x00, sizeof(ms));
		memset(buf, 0x00, sizeof(buf));
		return ret;
	}
	penc = enc;
#else
	ret = ICL_PK1_Public_Encrypt_ex(ctx->pubkey, ctx->pubkeyl, ICL_RSAES_PKCS1_15, buf, bufl, &penc, &encl, ICL_NO_ENCODE, "SHA1");
	if(ret != ISEM_OK){
		memset(ctx->ms, 0x00, ISEM_KEYGEN_LENGTH);
		memset(hash, 0x00, sizeof(hash));
		memset(ms, 0x00, sizeof(ms));
		memset(buf, 0x00, sizeof(buf));
		return ret;
	}
#endif

	/* 전문 리턴 */
	memcpy(out, penc, encl);
	*outl = encl;
	
	memset(buf, 0x00, sizeof(buf));
	memset(hash, 0x00, sizeof(hash));
	memset(ms, 0x00, sizeof(ms));
	memset(enc, 0x00, sizeof(enc));
#ifndef _ISEM_CLIENT
	if(penc){
		ICL_Free(penc, encl);
		penc = NULL;
	}
#endif

	return ret;
}

int ISEM_Key_Exchange_Handle_KSN2(ISEM_MTI_CTX *ctx, unsigned char *in, int inl, char *ksn)
{
	int ret = ISEM_OK;

	unsigned char enck[ISEM_KEY_LENGTH] = {0x00,};
	int enckl = 0;
	unsigned char mack[ISEM_KEY_LENGTH] = {0x00,};
	int mackl = 0;
	unsigned char gkey[ISEM_KEYGEN_LENGTH] = {0x00,};
	int gkeyl = 0;
	unsigned char hmac[ISEM_HMAC_LENGTH] = {0x00,};
	int hmacl = 0;
	unsigned char dec[ISEM_DEC_LENGTH] = {0x00,};
	int decl = 0;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(in == NULL || inl <= 0){
		return ISEM_ERR_IN_NULL;
	}

	if(ksn == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	/* MS, Ck 로 encK, macK 생성 */
	gkeyl = ISEM_KEYGEN_LENGTH;
	dbuf("-@@- ctx->ms", ctx->ms, ISEM_KEYGEN_LENGTH);
	dbuf("-@@- ctx->ck", ctx->ck, ISEM_RANDOM_LENGTH);
	ret = ICL_HASH_KDF(ctx->ms, ISEM_KEYGEN_LENGTH, ctx->ck, ISEM_RANDOM_LENGTH, ctx->hash_alg, gkey, &gkeyl);
	if(ret != ISEM_OK){
		return ret;
	}
	dbuf("-@@- gkey", gkey, gkeyl);

#if 1   /* 2017-02-28 INITECH Change */
	ret = ISEM_MTI_CMAC_DeriveKey(gkey, gkeyl, enck, &enckl, mack, &mackl);
    if(ret != ISEM_OK){
		memset(gkey, 0x00, sizeof(gkey));	
        return ret;
    }
    memcpy(ctx->enck, enck, ISEM_KEY_LENGTH);
    memcpy(ctx->mack, mack, ISEM_KEY_LENGTH);	
    dbuf("-@@- ctx->enck", ctx->enck, ISEM_KEY_LENGTH);
    dbuf("-@@- ctx->mack", ctx->mack, ISEM_KEY_LENGTH);
#else
	ISEM_Des_key_set_parity(gkey, gkeyl);

	memcpy(ctx->enck, gkey, ISEM_KEY_LENGTH);
	memcpy(ctx->mack, gkey + ISEM_KEY_LENGTH , ISEM_KEY_LENGTH);

#endif

	/* 무결성 검증 */
	ret = ICL_MAC_HMAC_F(ISEM_HMAC_SHA256, in, inl-ISEM_HMAC_LENGTH, ctx->mack, ISEM_KEY_LENGTH, hmac, &hmacl);
	if(ret != ISEM_OK){
		memset(gkey, 0x00, sizeof(gkey));
		memset(ctx->enck, 0x00, ISEM_KEY_LENGTH);
		memset(ctx->mack, 0x00, ISEM_KEY_LENGTH);
		memset(enck, 0x00, sizeof(enck));
		memset(mack, 0x00, sizeof(mack));		
		return ret;
	}
	dbuf("-@@- hmac", hmac, hmacl);

	if(memcmp(hmac, in+(inl-ISEM_HMAC_LENGTH), ISEM_HMAC_LENGTH)){
		memset(gkey, 0x00, sizeof(gkey));
		memset(ctx->enck, 0x00, ISEM_KEY_LENGTH);
		memset(ctx->mack, 0x00, ISEM_KEY_LENGTH);
		memset(hmac, 0x00, sizeof(hmac));
		memset(enck, 0x00, sizeof(enck));
		memset(mack, 0x00, sizeof(mack));
		return ISEM_ERR_HMAC_VERIFY;
	}

	/* 복호화 */
	ret = ICL_SYM_Decrypt_F(ctx->enck, (unsigned char *)ISEM_DEF_IV_VALUE, "AES128-CBC", ICL_PKCS5_PAD, in, inl-ISEM_HMAC_LENGTH, dec, &decl);
	if(ret != ISEM_OK){
		memset(gkey, 0x00, sizeof(gkey));
		memset(ctx->enck, 0x00, ISEM_KEY_LENGTH);
		memset(ctx->mack, 0x00, ISEM_KEY_LENGTH);
		memset(hmac, 0x00, sizeof(hmac));
		memset(enck, 0x00, sizeof(enck));
		memset(mack, 0x00, sizeof(mack));
		return ret;
	}
	dbuf("-@@- dec", dec, ISEM_KSN_LENGTH * 2);

	/* KSN 리턴 */
	memcpy(ksn, dec, ISEM_KSN_LENGTH * 2);
	dbuf("-@@- ksn", ksn, ISEM_KSN_LENGTH * 2);

	memset(enck, 0x00, sizeof(enck));
	memset(mack, 0x00, sizeof(mack));
	memset(gkey, 0x00, sizeof(gkey));
	memset(hmac, 0x00, sizeof(hmac));
	memset(dec, 0x00, sizeof(dec));

	return ret;
}

int ISEM_Key_Exchange_Request_IPEK1(ISEM_MTI_CTX *ctx, unsigned char *out, int *outl)
{
	int ret = ISEM_OK;
	unsigned char hmac[ISEM_HMAC_LENGTH] = {0x00,};
	int hmacl = 0;
	unsigned char enc[ISEM_BUF_LENGTH] = {0x00,};
	int encl = 0;
	unsigned char *penc = NULL;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(out == NULL || outl == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	/* Empuk(enck) 생성 */
#ifdef _ISEM_CLIENT
	ret = ICL_PK1_Public_Encrypt_ex_C(ctx->mod, ctx->modl, ctx->exp, ctx->expl, ICL_RSAES_PKCS1_15, ctx->enck, ISEM_KEY_LENGTH, enc, &encl, ICL_NO_ENCODE, "SHA1");
	if(ret != ISEM_OK){
		return ret;
	}
	penc = enc;
#else
	ret = ICL_PK1_Public_Encrypt_ex(ctx->pubkey, ctx->pubkeyl, ICL_RSAES_PKCS1_15, ctx->enck, ISEM_KEY_LENGTH, &penc, &encl, ICL_NO_ENCODE, "SHA1");
	if(ret != ISEM_OK){
		return ret;
	}
#endif

	/* HMACmack(Empuk(enck)) 생성  */
	ret = ICL_MAC_HMAC_F(ISEM_HMAC_SHA256, penc, encl, ctx->mack, ISEM_KEY_LENGTH, hmac, &hmacl);
	if(ret != ISEM_OK){
		memset(hmac, 0x00, sizeof(hmac));
		memset(enc, 0x00, sizeof(enc));		
#ifndef _ISEM_CLIENT
		if(penc){
			ICL_Free(penc, encl);
			penc = NULL;
		}
#endif		
		return ret;
	}

	/* 전문 생성 */
	/* Empuk(enck)||HMACmack(Empuk(enck)) */
	memcpy(out, penc, encl);
	memcpy(out+encl, hmac, ISEM_HMAC_LENGTH);
	*outl = encl + ISEM_HMAC_LENGTH;

	memset(hmac, 0x00, sizeof(hmac));
	memset(enc, 0x00, sizeof(enc));
#ifndef _ISEM_CLIENT
	if(penc){
		ICL_Free(penc, encl);
		penc = NULL;
	}
#endif

	return ret;
}


int ISEM_Key_Exchange_Handle_IPEK2(ISEM_MTI_CTX *ctx, unsigned char *in, int inl, unsigned char *ipek, int *ipek_len)
{
	int ret = ISEM_OK;
	unsigned char hmac[ISEM_HMAC_LENGTH] = {0x00,};
	int hmacl = 0;
	unsigned char dec[ISEM_DEC_LENGTH] = {0x00,};
	int decl = 0;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(in == NULL || inl <= 0){
		return ISEM_ERR_IN_NULL;
	}

	if(ipek == NULL || ipek_len == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	/* 무결성 검증 */
	ret = ICL_MAC_HMAC_F(ISEM_HMAC_SHA256, in, inl-ISEM_HMAC_LENGTH, ctx->mack, ISEM_KEY_LENGTH, hmac, &hmacl);
	if(ret != ISEM_OK){
		return ret;
	}
	if(memcmp(hmac, in+(inl-ISEM_HMAC_LENGTH), ISEM_HMAC_LENGTH)){
		memset(hmac, 0x00, sizeof(hmac));
		return ISEM_ERR_HMAC_VERIFY;
	}

	/* 복호화 */
	ret = ICL_SYM_Decrypt_F(ctx->enck, (unsigned char *)ISEM_DEF_IV_VALUE, ctx->sym_alg, ICL_NO_PAD, in, inl-ISEM_HMAC_LENGTH, dec, &decl);
	if(ret != ISEM_OK){
		memset(hmac, 0x00, sizeof(hmac));
		return ret;
	}

	/* IPEK 리턴 */
	memcpy(ipek, dec, ISEM_IPEK_LENGTH);
	*ipek_len = decl;

	memset(hmac, 0x00, sizeof(hmac));
	memset(dec, 0x00, sizeof(dec));

	return ret;
}

int ISEM_Ctx_Client_New(ISEM_MTI_CTX *ctx)
{
	int ret = ISEM_OK;

	if(ctx == NULL){
		return ISEM_ERR_CTX_NULL;
	}

	if(strlen(conf_info.sym_alg)>0){
		strncpy(ctx->sym_alg, conf_info.sym_alg, sizeof(ctx->sym_alg));
	}else{
		strncpy(ctx->sym_alg, ISEM_SYM_ALG, sizeof(ctx->sym_alg));
	}

	if(strlen(conf_info.hash_alg)>0){
		strncpy(ctx->hash_alg, conf_info.hash_alg, sizeof(ctx->hash_alg));
	}else{
		strncpy(ctx->hash_alg, ISEM_HASH_ALG, sizeof(ctx->hash_alg));
	}

	return ret;
}
