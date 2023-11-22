#ifndef _ISEM_MTI_INTERNAL_H
#define _ISEM_MTI_INTERNAL_H

extern ISEM_MTI_CONF_INFO conf_info;

int i_NTOHL(int x);
int i_HTONL(int x);
void I2OS(unsigned char *os, int i);
int isem_make_random(int rand_len, unsigned char *random);
int isem_make_ms(ISEM_MTI_CTX *ctx, unsigned char *ms, int *ms_len);
int isem_sec_encode(unsigned char *sec, int secl, unsigned char *in, int inl, unsigned char *out, int *outl, int type);
int ICL_HASH_KDF(unsigned char *sec_data, int sec_data_len, unsigned char *etc_data, int etc_data_len, char *hash_alg, unsigned char *out, int *out_len);

int isem_loadconf(char *conf_path);
int ICL_HASH_Data_F(unsigned char *in, int in_len, unsigned char *hash_data, int *hash_len, char *hash_alg);
int ICL_MAC_HMAC_F(int algo_id, unsigned char *org_data, int org_data_len, unsigned char *key, int key_len, unsigned char *hmac, int *hmac_len);

#endif
