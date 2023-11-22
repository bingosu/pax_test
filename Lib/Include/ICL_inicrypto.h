/*
  ICL_inicrypto.h
  librsadlx

  Created by DaeHyun Kim on 2015. 2. 28..
  Copyright (c) 2015??INITECH. All rights reserved.
*/

#ifndef librsadlx_ICL_inicrypto_h
#define librsadlx_ICL_inicrypto_h

#if defined(_WIN32) && defined(_USRDLL)
#   if !defined(ISEM_DLL)
#       define ISEM_DLL
#   endif
#endif

#if defined(_WIN32) && defined(ISEM_DLL)
#   if defined(ISEM_EXPORTS)
#       define ISEM_API __declspec(dllexport)
#   else
#       define ISEM_API __declspec(dllimport)    
#   endif
#elif defined(_WIN32)
#   if defined(ISEM_EXPORTS)
#       define ISEM_API __declspec(dllexport)
#   else
#       define ISEM_API __declspec(dllimport)
#   endif
#else
#   if defined(__GNUC__) && __GNUC__ >= 4
#       define ISEM_API __attribute__((visibility("default")))
#   else
#       define ISEM_API 
#   endif
#endif

#if !defined(ISEM_API)
#   define ISEM_API 
#endif

#define ICL_OK  0

/* these define values have to same with inicrypto's define value */

#define ICL_NO_PAD				0x00
#define ICL_RSAES_PKCS1_15			0x20	/*!< RSA encryption PKCS1 v1.5 ENCODE*/
#define ICL_RSAES_OAEP_20			0x08	/*!< RSA encryption OAEP v2.0 ENCODE*/
#define ICL_RSAES_OAEP_21			0x10	/*!< RSA encryption OAEP v2.1 ENCODE*/
#define ICL_RSASSA_PKCS1_15		0x01	/*!< RSA signature PKCS1 v1.5 ENCODE*/
#define ICL_RSASSA_PSS			0x02	/*!< RSA signature PSS ENCODE*/
#define ICL_PKCS5_PAD				0x01

#define ICL_NO_ENCODE				0x10	/*!< No encoding flag */
#define ICL_B64_ENCODE			0x00	/*!< Base64 encoding flag */
#define ICL_B64_LF_ENCODE			0x01	/*!< Base64 encoding with 'insert linefeed' flag */

typedef struct {
	int line;
	char name[128];
	char value[512];
} st_CONFIG;

int ICL_PK5_PBKDF2(unsigned char *in_passwd, int in_passwd_len, unsigned char *in_salt, int in_salt_len, int in_iter, char *in_hash_alg, unsigned char *out_key, int in_key_len);
void ICL_Free(void *p, int len);
void ICL_HexaDump(char *outbuf, int *outbuf_len,  unsigned char *content, int len);

ISEM_API int ICL_DRBG_Get_Random(unsigned char *random, int rand_len);
int ICL_SYM_Encrypt_F(unsigned char *key, unsigned char *iv, char *alg, int pad_mode, unsigned char *in, int in_len, unsigned char *out, int *out_len);
int ICL_SYM_Decrypt_F(unsigned char *key, unsigned char *iv, char *alg, int pad_mode, unsigned char *in, int in_len, unsigned char *out, int *out_len);
int ICL_HASH_Data_F(unsigned char *in_data, int in_data_len, unsigned char *hash_data, int *hash_len, char *hash_alg);
int ICL_MAC_HMAC_F(int algo_id, unsigned char *input, int inputlen, unsigned char *key, int keylen, unsigned char *output, int *outlen);
char *ICL_Get_Error_Msg(int error_code);
ISEM_API int ICL_PK1_Public_Encrypt_ex_F(unsigned char *pubk_str, int pubk_len, char pad_mode, unsigned char *in, int in_len, unsigned char *out, int *out_len, char encode, char *hash_algo);
ISEM_API int ICL_PK1_Private_Decrypt_ex_F(unsigned char *prik_str, int prik_len, char pad_mode, unsigned char *in, int in_len, unsigned char *out, int *out_len, char encode, char *hash_algo);

int ICL_PK1_Private_Sign_F(unsigned char *priv_str, int priv_len, char pad_mode, char *hash_alg, unsigned char *in, int in_len, unsigned char *out, int *out_len, char encode);
int ICL_PK1_Public_Verify_F(unsigned char *pubk_str, int pubk_len, char pad_mode, char *hash_alg, unsigned char *msg, int msg_len, unsigned char *sign, int sign_len, char encode);

int ICL_PK1_Public_Verify_C(unsigned char *modulus, int modulus_len,unsigned char* exponent, int exponent_len, char pad_mode, char *hash_alg, unsigned char *msg, int msg_len, unsigned char *sign, int sign_len, char encode);

int ICL_PK1_Public_Encrypt_ex_C(unsigned char *modulus, int modulus_len,unsigned char* exponent, int exponent_len, char pad_mode, unsigned char *in, int in_len, unsigned char *out, int *out_len, char encode, char *hash_algo);

int ICL_MAC_CBC(int block_algo_id, unsigned char *in, int inLen,  unsigned char *key, unsigned char *out);
#endif
