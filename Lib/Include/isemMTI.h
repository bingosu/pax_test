#ifndef _ISEM_MTI_H
#define _ISEM_MTI_H

#define ISEM_FL				__FILE__,__LINE__	/**< macro for log */
#define ISEM_LOG_FILE_PREFIX			"isemLT"		/**< prefix name of log filename */
#define DEFAULT_LOG_OUT			0x46			/**< F(file output) */
#define DEFAULT_LOG_PATH		"./"			/**< default log path */
#define DEFAULT_LOG_LEVEL		ICL_OFF			/**< default log level */
#define ISEM_DEF_IV_VALUE		"INISAFESEMFORC!!"
#define ISEM_SEED_VALUE			"1!42@6#3$7%09^&*58INISAFESEMSEED"

#define ISEM_SYM_ALG			"DES_EDE_2KEY-ECB"
#define ISEM_HASH_ALG			"SHA256"
#define ISEM_HMAC_SHA1			0x15000100
#define ISEM_HMAC_SHA256		0x15000300

#define ISEM_CLIENT_CTX			'C'
#define ISEM_SERVER_CTX			'S'

#define ISEM_SEC_SEED_LENGTH		32
#define ISEM_SERIAL_LENGTH		16

#define ISEM_BUF_LENGTH			2048

#define ISEM_ALG_SIZE			20
#define ISEM_HASH_LENGTH		32
#define ISEM_HMAC_LENGTH		32

#define ISEM_KEYGEN_LENGTH		32
#define ISEM_KEY_LENGTH			16
#define ISEM_RANDOM_LENGTH		20

#define ISEM_KSN_LENGTH			10
#define ISEM_IPEK_LENGTH		16

#define ISEM_PUBIDX_LENGTH		10
#define ISEM_PUBKEY_LENGTH		1024
#define ISEM_MODULE_LENGTH		512
#define ISEM_EXPONENT_LENGTH		16

#define ISEM_SIGN_LENGTH		2048
#define ISEM_ENC_LENGTH			512
#define ISEM_DEC_LENGTH			256

#define ISEM_DUKPT_LRC_POS		16
#define ISEM_DUKPTKEY_LENGTH		16
#define ISEM_FK_TABLE_COUNT		21

/* Secure 인코딩 설정 */
#define ISEM_SEC_ENCODE			0
#define ISEM_SEC_DECODE			1

#define ISEM_DES                         0x02000100

#define ISEM_CBC_MAC_AL1                 0x00100000
#define ISEM_CBC_MAC_AL2                 0x00200000
#define ISEM_CBC_MAC_AL3                 0x00300000

#define ISEM_CBC_MAC_PAD_MASK            0x0000000F
#define ISEM_CBC_MAC_PAD1                0x00000001
#define ISEM_CBC_MAC_PAD2                0x00000002

/* 환경 파일 구조체 */
typedef struct isem_mti_conf_info_st{
	char sym_alg[ISEM_ALG_SIZE];
	char hash_alg[ISEM_ALG_SIZE];

	unsigned char sec_seed[ISEM_SEC_SEED_LENGTH];
	int sec_seed_len;
}ISEM_MTI_CONF_INFO;

/* ISEM CTX 구조체 */
typedef struct isem_mti_ctx_st{
	char sym_alg[ISEM_ALG_SIZE];
	char hash_alg[ISEM_ALG_SIZE];

	unsigned char serial[ISEM_SERIAL_LENGTH];
	int serial_len;
	unsigned char pubkey[ISEM_PUBKEY_LENGTH];
	int pubkeyl;
	unsigned char mod[ISEM_MODULE_LENGTH];
	int modl;
	unsigned char exp[ISEM_EXPONENT_LENGTH];
	int expl;
	char pubidx[ISEM_PUBIDX_LENGTH];
	int pubidxl;

	unsigned char cr[ISEM_RANDOM_LENGTH];
	unsigned char sr[ISEM_RANDOM_LENGTH];
	unsigned char ms[ISEM_KEYGEN_LENGTH];
	unsigned char ck[ISEM_RANDOM_LENGTH];
	unsigned char enck[ISEM_KEY_LENGTH];
	unsigned char mack[ISEM_KEY_LENGTH];

	unsigned char dek[ISEM_KEY_LENGTH];
	unsigned char pek[ISEM_KEY_LENGTH];
	unsigned char mackey[ISEM_KEY_LENGTH];
	unsigned char ksn[ISEM_KSN_LENGTH];
}ISEM_MTI_CTX;
/* 2017-02-28 INITECH ADD START */
void ISEM_Des_key_set_parity(unsigned char* key_data, int key_data_len);
/* 2017-02-28 INITECH ADD END */


int ISEM_MTI_Init(char *conf);
int ISEM_MTI_Ctx_New(char type, ISEM_MTI_CTX *ctx);
void ISEM_MTI_Ctx_Free(ISEM_MTI_CTX *ctx);
void ISEM_MTI_Clean(void);

int ISEM_Set_HashAlg(char *hash_alg);
int ISEM_Set_SymAlg(char *sym_alg);

int ISEM_MTI_Init_DUKPT(char *ksn, unsigned char *ipek);
int ISEM_Dukpt_Encrypt_GenKey(ISEM_MTI_CTX *ctx);
int ISEM_Encrypt_Data(ISEM_MTI_CTX *ctx, unsigned char *msg, int msg_len, unsigned char *out, int *out_len);
int ISEM_Encrypt_Data_PIN(ISEM_MTI_CTX *ctx, unsigned char *msg, int msg_len, unsigned char *out, int *out_len);
int ISEM_Decrypt_Data(ISEM_MTI_CTX *ctx, char *ksn, unsigned char *ct, int ct_len, unsigned char *pt, int *pt_len);

int ISEM_CBC_MAC(ISEM_MTI_CTX *ctx, int algo_id, unsigned char *in, int inl,  unsigned char *key, unsigned char *out);

#ifndef _ISEM_CLIENT
void ISEM_Hexa_Dump(FILE *fp, unsigned char *msg, int msg_len);
#else
void ISEM_Hexa_Dump(char *outbuf, int *outbuf_len, unsigned char *msg, int msg_len);
#endif

#endif
