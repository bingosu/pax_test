#ifndef _ISEM_MTI_CMAC_H
#define _ISEM_MTI_CMAC_H

typedef struct _isem_cmac_ctx {
	unsigned char K1[8+1];
	unsigned char K2[8+1];
	unsigned char key[24];
} ISEM_CMAC_CTX;

void isem_MTI_xor_64(unsigned char *a, unsigned char *b, unsigned char *out);
void isem_MTI_leftshift_onebit(unsigned char *input, unsigned char *output);
void isem_MTI_generate_subkey(unsigned char *key, unsigned char *K1, unsigned char *K2);
void isem_MTI_padding(unsigned char *lastb, unsigned char *pad, int length);
void isem_MTI_CMAC(ISEM_CMAC_CTX *ctx, unsigned char *in, int inlen, unsigned char *out, int *outlen);
void isem_MTI_CMAC_deriveEncKey(ISEM_CMAC_CTX *ctx, unsigned char *enckey, int *enckey_len);
void isem_MTI_CMAC_deriveMacKey(ISEM_CMAC_CTX *ctx, unsigned char *mackey, int *mackey_len);
int isem_MTI_CMAC_init(ISEM_CMAC_CTX *ctx, unsigned char *key, int key_len);

int ISEM_MTI_CMAC_DeriveKey(unsigned char *key, int key_len, unsigned char *enckey, int *enckey_len, unsigned char *mackey, int *mackey_len);

#endif
